require_relative 'diag'
require_relative 'loosehash'
require_relative 'node'
require_relative 'token'

require 'getoptlong'

module ASTGen
  class ASTBuilder
    attr_accessor :nodes

    def initialize(d)
      @d = d.fork
      @nodes = LooseHash.new
    end

    def node(name, &block)
      @d.pos("node #{name.inspect}") do
        @nodes[name] = _node(name, &block)
      end
    end

    private def ast_outs(data, outs:, impl:)
      list = []

      @nodes.each do |name, nodes|
        next unless nodes.length > 0

        p = File.join(data[:astDir], ASTGen.camel_name(name.to_s))

        list << "#{p}.cpp" if outs
        list << "#{p}.hpp" if impl
      end

      list << File.join(data[:astDir], "ast.hpp") if impl

      list
    end
  end

  class ListBuilder < ASTBuilder
    def initialize(d)
      super
    end

    private def _node(name, &block) name end

    def make_list(data, impl)
      list = ast_outs(data, outs: !impl, impl: impl)

      if impl
        list << data[:tokenOut]
      else
        list << data[:flexOut] << data[:bisonOut]
      end

      $stdout << list.join(" ") << "\n"
    end
  end

  class FullBuilder < ASTBuilder
    def initialize(d)
      super
    end

    private def _node(name, &block) Node.build(name, @d, &block) end

    def make_ast(data)
      node_list = @nodes.flatten do |name, nodes, uses|
        @d.error("duplicate node name #{@d.hl(name)} used #{uses} times")
      end

      symbol_list = LooseHash.new
      @nodes.counted.each_value do |nodes|
        nodes.each do |node, count|
          node.symbols.counted.each do |name, syms|
            syms.each{|s, n| symbol_list.addn(name, s, n: count * n) }
          end
        end
      end

      symbol_list = symbol_list.flatten do |name, syms, uses|
        nodes = syms.map{|s, n| [s.node, n] }.to_h

        @d.error("duplicate symbol #{@d.hl(name)} used #{uses} times in " <<
          (nodes.length > 1 ? "#{nodes.length} nodes: " : "node ") <<
          nodes.map do |sym, count|
            s = @d.hl(sym)
            s << " (#{count} times)" if count > 1 && nodes.length > 1
            next s
          end.join(", "))
      end
      symbol_list.select do |name, sym|
        !verify(sym.resolved?) { @d.error("unresolved defer() in symbol #{@d.hl(name)}") }
      end.each{|n, _| symbol_list.make_error(n) }

      nodes = node_list.map{|m, b| [m, b.make_node(symbol_list)] }.to_h
      symbols = symbol_list.map{|n, b| [n, b.make_symbol(nodes, symbol_list)] }.to_h
      tokens = Token.scan(data[:flexIn], @d)

      [nodes, symbols, tokens]
    end
  end

  @@errored = false
  @@quiet = false
  @@verbose = false

  def self.error!; @@errored = true end
  def self.quiet?; @@quiet end
  def self.verbose?; @@verbose end

  def self.diag
    Diagnostics.new(
      on_error: lambda { self.error! },
      quiet: lambda { self.quiet? },
      verbose: lambda { self.verbose? },
    )
  end

  @@d = diag

  private_class_method def self.run_diag
    begin
      [true, yield]
    rescue Diagnostics::DiagnosticError
      [false, nil]
    rescue => e
      $stderr << e.backtrace[0] << ":#{e.to_s} (#{e.class})\n" <<
        e.backtrace[1..-1].map{|e2| " " * 8 + "from " << e2.to_s << "\n"}.join
      error!
    end
  end

  def self.camel_name(name)
    name[0].downcase << name[1..-1]
  end

  def self.file_path(path, name, ext = nil)
    s = File.join(path, camel_name(name))
    s << ".#{ext}" if ext
    s
  end

  def self.run(&block)
    @@d.pos("ASTGen.run") do
      opts = GetoptLong.new(
        ["--verbose", "-v", GetoptLong::NO_ARGUMENT],
        ["--quiet", "-q", GetoptLong::NO_ARGUMENT],
        ["--list", "-l", GetoptLong::NO_ARGUMENT],
        ["--implicit", "-i", GetoptLong::NO_ARGUMENT],
        ["--order", "-r", GetoptLong::NO_ARGUMENT],
        ["--flex", "-f", GetoptLong::REQUIRED_ARGUMENT],
        ["--bison", "-b", GetoptLong::REQUIRED_ARGUMENT],
        ["--token", "-t", GetoptLong::REQUIRED_ARGUMENT],
      )

      do_list = impl = false
      do_order = false

      data = {
        :flexIn => nil,
        :flexOut => nil,
        :bisonIn => nil,
        :bisonOut => nil,
        :tokenIn => nil,
        :tokenOut => nil,
      }

      opts.each do |opt, arg|
        case opt
          when "--verbose"
            @@verbose = true

          when "--quiet"
            @@quiet = true

          when "--list"
            do_list = true

          when "--implicit"
            do_list = true
            impl = true

          when "--order"
            do_order = true

          when "--flex"
            data[:flexIn], data[:flexOut] = arg.split(":")

          when "--bison"
            data[:bisonIn], data[:bisonOut] = arg.split(":")

          when "--token"
            data[:tokenIn], data[:tokenOut] = arg.split(":")
        end
      end

      raise "Cannot specify --order and --list/--implicit together" if do_list && do_order

      no_files = do_list || do_order

      raise "Missing flex input" unless data[:flexIn] || no_files
      raise "Missing flex output" unless data[:flexOut] || no_files
      raise "Missing bison input" unless data[:bisonIn] || no_files
      raise "Missing bison output" unless data[:bisonOut] || no_files
      raise "Missing token input header" unless data[:tokenIn] || no_files
      raise "Missing token output header" unless data[:tokenOut] || no_files

      raise "Missing AST directory path" unless ARGV.length > 0 || do_order

      data[:astDir] = ARGV.shift unless do_order

      @@quiet = true if do_list

      b = nil

      run_diag do
        b = if do_list
          ListBuilder.new(@@d)
        else
          FullBuilder.new(@@d)
        end
        b.instance_eval(&block)
      end

      catch :stop do
        if do_order
          run_diag { list_order(b) }
          throw :stop
        end

        run_diag do
          if do_list
            b.make_list(data, impl)
          else
            (nodes, symbols, tokens) = b.make_ast(data)

            # @@d.debug("Nodes:")
            # @@d.p(nodes, long: true)
            # @@d.debug("Symbols:")
            # @@d.p(symbols, long: true)
            # @@d.p("Tokens:")
            # @@d.p(tokens, long: true)

            nodes.each do |name, node|
              File.open(file_path(data[:astDir], name, "hpp"), "w") do |f|
                f << node.emit_head(nodes) << "\n"
              end

              File.open(file_path(data[:astDir], name, "cpp"), "w") do |f|
                f << node.emit_body(nodes) << "\n"
              end
            end

            File.open(File.join(data[:astDir], "ast.hpp"), "w") do |f|
              nodes.each do |name, node|
                f << LineWriter.lines do |l|
                  node.emit_include(l)
                end << "\n"
              end
            end unless @@errored

            File.open(data[:tokenOut], "w") do |f|
              File.foreach(data[:tokenIn]) do |line|
                case line
                  when /^\s*#pragma\s+astgen\s+friends\s*\(([^\)]*)\)\s*$/
                    f << LineWriter.lines(with_indent: $1) do |l|
                      Node.emit_friends(nodes, :Token, l)
                    end << "\n"
                  else
                    f << line
                end
              end
            end

            File.open(data[:flexOut], "w") do |f|
              File.foreach(data[:flexIn]) do |line|
                case line
                  when /^(\s*)%astgen-token-defs\s*$/
                    f << LineWriter.lines(with_indent: $1) do |l|
                      tokens.each do |name, tok|
                        tok.emit_flex_part(:def, l)
                      end
                    end << "\n"
                  when /^(\s*)%astgen-token\s+(\S+).*$/
                    name = $2.to_sym
                    f << LineWriter.lines(with_indent: $1) do |l|
                      tokens[name].emit_flex_part(:rule, l)
                    end unless !tokens.isnt_error?(name)
                    f << "\n"
                  else
                    f << line
                end
              end
            end

            File.open(data[:bisonOut], "w") do |f|
              File.foreach(data[:bisonIn]) do |line|
                case line
                  when /^(\s*)%astgen-token-defs\s*$/
                    f << LineWriter.lines(with_indent: $1) do |l|
                      tokens.each do |name, tok|
                        tok.emit_bison_part(:def, l)
                      end
                    end << "\n"
                  when /^(\s*)%astgen-union\s*$/
                    f << LineWriter.lines(with_indent: $1) do |l|
                      l << "%union {"
                      l.group.fmt with_indent: "  " do
                        nodes.each do |name, node|
                          node.emit_bison_part(:union, l)
                        end

                        Token.emit_bison_part(:union, l)
                      end
                      l.trim << "}"
                    end << "\n"
                  when /^(\s*)%astgen-dtors\s*$/
                    f << LineWriter.lines(with_indent: $1) do |l|
                      nodes.each do |name, node|
                        node.emit_bison_part(:dtor, l)
                      end

                      Token.emit_bison_part(:dtor, l)
                    end << "\n"
                  when /^(\s*)%astgen-types\s*$/
                    f << LineWriter.lines(with_indent: $1) do |l|
                      symbols.each do |name, symbol|
                        symbol.emit_bison_part(:type, l)
                      end
                    end << "\n"
                  when /^(\s*)%astgen-rules\s*$/
                    f << LineWriter.lines(with_indent: $1) do |l|
                      symbols.each do |name, symbol|
                        symbol.emit_bison_part(:rule, l)
                      end
                    end << "\n"
                  else
                    f << line
                end
              end
            end
          end
        end
      end

      exit(1) if @@errored
    end
  end
end