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
      @no_elide = Set.new
      @nodes = LooseHash.new
    end

    def export(symbol) @d.pos("export #{symbol.inspect}") { @no_elide << symbol } end

    def node(name, &block)
      @d.pos("node #{name.inspect}") do
        @nodes[name] = _node(name, &block)
      end
    end

    private def ast_outs(data, outs:, impl:)
      list = []

      @nodes.each do |name, nodes|
        next unless !nodes.empty?

        p = File.join(data[:astDir], ASTGen.camel_name(name.to_s))

        list << "#{p}.hpp" if impl
      end

      list << File.join(data[:astDir], "ast.cpp") if outs
      list << File.join(data[:astDir], "ast.hpp") if impl

      list << File.join(data[:astDir], "walker.cpp") if outs
      list << File.join(data[:astDir], "walker.hpp") if impl

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
        node_uses = LooseHash.new
        syms.each{|s, n| node_uses.addn(s.name, s.node, n: n) }
        node_uses = node_uses.counted[name]

        @d.error("duplicate symbol #{@d.hl(name)} used #{uses} times in " <<
          (node_uses.length > 1 ? "#{node_uses.length} nodes: " : "node ") <<
          node_uses.map do |sym, count|
            s = @d.hl(sym)
            s << " (#{count} times)" if count > 1 && node_uses.length > 1
            next s
          end.join(", "))
      end
      symbol_list.select do |name, sym|
        !verify(sym.resolved?) { @d.error("unresolved defer() in symbol #{@d.hl(name)}") }
      end.each{|n, _| symbol_list.make_error(n) }

      nodes = ErrorableHash.new(
        node_list.map{|m, b| [m, b.make_node(symbol_list)] }.to_h,
        node_list.errors
      )
      symbols = ErrorableHash.new(
        symbol_list.map{|n, b| [n, b.make_symbol(nodes, symbol_list)] }.to_h,
        symbol_list.errors
      )

      used_syms = Set[*@no_elide]
      q = [*used_syms]

      until q.empty?
        symbols[q.shift].used_syms.each{|s| q.push(s) if symbols.isnt_error?(s) && used_syms.add?(s) }
      end

      symbols.each_valid_key.select{|s| !used_syms.include?(s) }.each do |sym|
        @d.info_v("eliding unused symbol #{@d.hl(sym)}")
        symbols.make_error(sym)
      end

      pass = 1

      loop do
        list = symbols.select do |name, sym|
          sym.syntax.empty? || sym.syntax.all? do |syms, alts|
            syms.any? do |sym2|
              symbols.is_error?(case sym2
                when Symbol; sym.node.members[sym2]
                when Array; sym2[0] if (1...2) === sym2.length
              end)
            end
          end
        end

        break if list.empty?

        list.each do |sym, _|
          @d.info_v("eliding empty symbol #{@d.hl(sym)}#{" (pass #{pass})" if pass > 1}")
          symbols.make_error(sym)
        end

        symbols.each_value{|s| s.prune_syntax(symbols) }

        pass += 1
      end

      elided_aliases = Set.new

      symbols.each do |name, sym|
        aliases = sym.alias_for(symbols)

        if aliases
          @d.info_v("symbol #{@d.hl(name)} aliases #{@d.hl(aliases)}")
          elided_aliases << name
        end
      end

      symbols.each_value{|s| s.expand_aliases(symbols) }

      used_syms = Set[*@no_elide]
      q = [*used_syms]

      until q.empty?
        symbols[q.shift].used_syms.each{|s| q.push(s) if symbols.isnt_error?(s) && used_syms.add?(s) }
      end

      symbols.each_valid_key.select{|s| !used_syms.include?(s) }.each do |sym|
        @d.info_v("eliding symbol #{@d.hl(sym)}") unless elided_aliases.include?(sym)
        symbols.make_error(sym)
      end

      used_nodes = Set.new

      symbols.each_value{|s| used_nodes << s.node.name }

      nodes.each{|m, n| n.unused = true unless used_nodes.include?(m) }

      tokens = Token.scan(data[:flexIn], @d)

      [nodes, symbols, tokens]
    end
  end

  @@errored = false
  @@quiet = false
  @@verbose = false

  def self.error!; @@errored = true end
  def self.error?; @@errored end
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

      raise "Missing AST directory path" unless ARGV.any? || do_order

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
            end

            File.open(File.join(data[:astDir], "ast.cpp"), "w") do |f|
              f << LineWriter.lines do |l|
                Node.emit_body_start(l)

                nodes.each do |name, node|
                  node.emit_body(nodes, l)
                end

                Node.emit_body_end(l)
              end<< "\n"
            end

            nodes.each_error do |name|
              File.open(file_path(data[:astDir], name, "hpp"), "w") {|f| f << "// Node header omitted\n" }
            end

            File.open(File.join(data[:astDir], "ast.hpp"), "w") do |f|
              nodes.each do |name, node|
                f << LineWriter.lines do |l|
                  node.emit_include(l)
                end << "\n"
              end
            end unless @@errored

            File.open(File.join(data[:astDir], "walker.hpp"), "w") do |f|
              f << LineWriter.lines do |l|
                l << "#pragma once"

                l.sep

                l << "#include \"#{Node.header_name(:Ast)}\""

                l.sep

                Node.emit_ns_start(l)

                l << "class #{Node.class_name(:Walker)} {"
                l.group

                vis = Vis.new(l, :private)

                l.fmt with_indent: "  " do
                  l << "bool m_walk = true;"

                  vis << :protected

                  nodes.each do |_, node|
                    l << "virtual void visit(const #{node.just_class_name} *);"
                  end

                  l.sep << "inline void noWalk() { m_walk = false; }"

                  vis << :public

                  nodes.each do |_, node|
                    l << "void walk(const #{node.just_class_name} *, bool = false);"
                  end

                  l.sep
                end

                l.trim << "};"

                Node.emit_ns_end(l)
              end << "\n"
            end

            File.open(File.join(data[:astDir], "walker.cpp"), "w") do |f|
              f << LineWriter.lines do |l|
                l << "#include \"#{Node.header_name(:Walker)}\""

                l.sep

                Node.emit_ns_start(l)

                nodes.each do |_, node|
                  l << "void #{Node.qual_name(:Walker, "visit")}(const #{node.just_class_name} *) {}"
                end

                nodes.each do |_, node|
                  l.sep << "void #{Node.qual_name(:Walker, "walk")}(const #{node.just_class_name} *node, bool skip) {"

                  l.fmt with_indent: "  " do
                    l << "if (!skip) visit(node);"

                    l.sep << "if (m_walk) {"
                    l.group

                    l.fmt with_indent: "  " do
                      if node.use_alts?
                        l.sep << "switch (node->#{Node.AltMembName}()) {"
                        l.group
                        c = l.curr

                        l.fmt with_indent: "  " do
                          node.ctor_alts.each do |sig_, alts|
                            alts.each do |alt|
                              c << "case #{node.qual_name(node.enum_name(alt))}:"
                            end

                            sig = sig_.select{|a| node.members[a] != :Token }

                            if sig.length < 2
                              l.peek << " #{[*sig.map{|a| "walk(node->#{a}());" }, "break;"].join(" ")}"
                            else
                              sig.each do |arg|
                                l << "walk(node->#{arg}());"
                              end
                              l << "break;"
                            end
                          end
                        end

                        l.trim << "}"
                      else
                        node.ctors.each do |sig, _|
                          sig.select{|a| node.members[a] != :Token }.each do |arg|
                            l << "walk(node->#{arg}());"
                          end
                        end
                      end
                    end
                    l.trim << "}"

                    l << "else m_walk = true;"
                  end
                  l << "}"
                end

                Node.emit_ns_end(l)
              end << "\n"
            end

            File.open(data[:tokenOut], "w") do |f|
              File.foreach(data[:tokenIn]) do |line|
                case line
                  when /^\s*#pragma\s+astgen\s+friends\s+forward\s*"([^"]*)"\s*$/
                    f << LineWriter.lines(with_indent: $1) do |l|
                      Node.emit_friends_forward(nodes, :Token, l)
                    end << "\n"
                  when /^\s*#pragma\s+astgen\s+friends\s*"([^"]*)"\s*$/
                    f << LineWriter.lines(with_indent: $1) do |l|
                      Node.emit_friends(nodes, :Token, true, l)
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
                          node.emit_bison_part(:union, l) unless node.unused
                        end

                        Token.emit_bison_part(:union, l)
                      end
                      l.trim << "}"
                    end << "\n"
                  when /^(\s*)%astgen-dtors\s*$/
                    f << LineWriter.lines(with_indent: $1) do |l|
                      nodes.each do |name, node|
                        node.emit_bison_part(:dtor, l) unless node.unused
                      end

                      Token.emit_bison_part(:dtor, l)
                    end << "\n"
                  when /^(\s*)%astgen-types\s*$/
                    f << LineWriter.lines(with_indent: $1) do |l|
                      symbols.each do |name, symbol|
                        symbol.emit_bison_part(:type, l) unless symbol.alias_for
                      end
                    end << "\n"
                  when /^(\s*)%astgen-rules\s*$/
                    f << LineWriter.lines(with_indent: $1) do |l|
                      symbols.each do |name, symbol|
                        symbol.emit_bison_part(:rule, l) unless symbol.alias_for
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