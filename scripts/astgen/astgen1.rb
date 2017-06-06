require_relative 'diag'
require_relative 'linewriter'
require_relative 'node'
require_relative 'token'

require 'fileutils'
require 'getoptlong'
require 'set'

module ASTGen
  class ASTList
    def initialize(nodes, d)
      @d = d
      @nodes = nodes
    end

    def node(name)
      raise @d.fatal_r("duplicate node name #{name.inspect}") if @nodes.has_key?(name)

      @nodes[name] = :node

      self
    end
  end

  @@tokens = {}
  @@nodes = {}
  @@symbols = {}
  @@errored = false
  @@quiet = false
  @@verbose = false

  def self.quiet?; @@quiet end
  def self.verbose?; @@verbose end

  def self.error!; @@errored = true end

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
        e.backtrace[1..-1].map{|s| " " * 8 + "from " << s << "\n"}.join
      error!
    end
  end

  private_class_method def self.token(name, *args, **kwargs)
    raise @@d.fatal_r("Duplicate token name #{name.inspect}") if @@tokens.has_key?(name)

    t = nil

    run_diag do
      t = Token.new(name, *args, **kwargs)
      @@tokens[name] = t
    end

    t
  end

  def self.node(name, &block)
    raise @@d.fatal_r("duplicate node name #{name.inspect}") if @@symbols.has_key?(name)

    run_diag do
      n = Node.new(name, @@symbols)
      @@symbols[name] = n
      @@nodes[name] = n

      n.instance_eval(&block)
    end
  end

  private_class_method def self.freeze
    @@tokens.each{|_, val| val.freeze }
    @@nodes.each{|_, val| val.freeze }
  end

  def self.camel_name(name)
    name[0].downcase << name[1..-1]
  end

  def self.file_path(path, name)
    File.join("#{path}", "#{camel_name(name)}")
  end

  private_class_method def self.scan_flex(path, data)
    @@d.pos("scan_flex") do
      i = 0

      File.foreach(data[:flexIn]) do |line|
        i += 1

        pos = lambda { "at #{data[:flexIn]}:#{i}" }

        case line
          when /^\s*%astgen-token\s+(\S*)\s+(.*)\s*$/
            s = StringScanner.new($2)
            t = nil
            sym = $1.to_sym
            contents = nil

            err = catch :err do
              if s.scan(/"/)
                contents = s.scan(/([^\\"]|\\.)*/)

                throw :err, "'\"'" unless s.scan(/"/)

                t = token(sym, str: contents)
              elsif s.scan(/\//)
                contents = s.scan(/[^\/]*/)

                throw :err, "'/'" unless s.scan(/\//)

                t = token(sym, pat: contents)

                s.scan(/\s+/)

                if s.scan(/"/)
                  t.str = s.scan(/([^\\"]|\\.)*/).gsub(/\\(.)/, "\\1")

                  throw :err, "'\"'" unless s.scan(/"/)
                end
              else
                throw :err, "'\"' or '/'"
              end

              s.scan(/\s+/)

              if s.scan(/\{/)
                action = catch :err do
                  braces = 1
                  a = ""

                  while braces > 0
                    r = s.scan_until(/[{}]/)
                    throw :err unless r

                    a << r[0..-1 - s.matched_size]

                    case s.matched
                      when "{"
                        braces += 1

                      when "}"
                        braces -= 1
                    end

                    a << s.match if braces > 0
                  end

                  a
                end

                t.action = action.strip! if action
                s.scan(/\s+/)
              end

              while s.scan(/\w+/)
                case s.matched
                  when "capture"
                    if s.scan(/\s+buf/)
                      if s.scan(/\s+end/)
                        t.capt = :buf_end
                      else
                        t.capt = :buf
                      end
                    else
                      t.capt = :text
                    end
                  else
                    @@d.error("unexpected token flag '#{s.matched}'")
                end

                s.scan(/\s+/)
              end

              nil
            end

            @@d.error("unexpected #{s.eos? ? "end of line" : "'#{s.rest}'"} (expecting #{err}) #{pos.call}") if err

          when /^\s*%astgen-token\b(?!-)/
            @@d.error("invalid syntax for %astgen-token #{pos.call}")
        end
      end
    end
  end

  private_class_method def self.emit(path, data)
    mtime = File.mtime($0)
    mmtime = Time.new(0)

    $".select{|e| e != $0 && File.exists?(e) }.each do |e|
      m = File.mtime(e)
      mmtime = m if m > mmtime
    end

    mtime = mmtime if mmtime > mtime

    emit_ast(path, data) if ast_outs(path, data, outs: true, impl: true)
      .any?{|e| !File.exists?(e) || mtime > File.mtime(e) }

    emit_flex(path, data) if !File.exists?(data[:flexOut]) || [File.mtime(data[:flexIn]), mmtime].max > File.mtime(data[:flexOut])
    emit_bison(path, data) if !File.exists?(data[:bisonOut]) || [File.mtime(data[:bisonIn]), File.mtime(data[:flexIn]), mtime].max > File.mtime(data[:bisonOut])
    emit_token(path, data) if !File.exists?(data[:tokenOut]) || [File.mtime(data[:tokenIn]), mtime].max > File.mtime(data[:tokenOut])
  end

  private_class_method def self.list_order()
    order = [@@nodes.first[1]]
    order_set = Set.new(order.map{|e| e.name })

    i = 0
    while i < order.length do
      order[i].froz_depends.reverse.each do |e|
        order.insert(i + 1, @@nodes[e]) if order_set.add?(e)
      end
      i += 1
    end

    order.each do |e|
      @@d.info(e.name.inspect)
    end
  end

  private_class_method def self.emit_ast(path, data)
    FileUtils.mkdir_p(path) unless File.directory?(path)

    @@nodes.each do |key, val|
      fname = file_path(path, key.to_s)

      File.open("#{fname}.hpp", "w") do |f|
        val.emit_head(@@nodes, f)
        f.close
      end

      File.open("#{fname}.cpp", "w") do |f|
        val.emit_body(@@nodes, f)
        f.close
      end
    end

    f = File.open(file_path(path, "ast.hpp"), "w")

    f << (LineWriter.lines do |l|
      @@nodes.each do |key, val|
        l << "#include \"ast/#{camel_name(key.to_s)}.hpp\""
      end
    end) << "\n"
  end

  private_class_method def self.emit_flex(path, data)
    @@d.pos("scan_flex") do
      File.open(data[:flexOut], "w") do |o|
        File.foreach(data[:flexIn]) do |line|
          case line
            when /^(\s*)%astgen-token-rules\s*$/
              o << (LineWriter.lines do |l|
                @@tokens.each {|_, val| val.emit_flex_head(l) }
              end) << "\n"
            when /^(\s*)%astgen-token\s+([a-zA-Z][a-zA-Z0-9]*)/
              o << (LineWriter.lines with_indent: $1 do |l|
                sym = $2.to_sym
                @@tokens[sym].emit_flex_body(l)
              end) << "\n"
            else
              o << line if o
          end
        end
      end
    end
  end

  private_class_method def self.emit_bison(path, data)
    File.open(data[:bisonOut], "w") do |o|
      File.foreach(data[:bisonIn]) do |line|
        case line
          when /^\s*%astgen-token-defs\s*$/
            o << (LineWriter.lines do |l|
              @@tokens.each do |_, val|
                val.emit_bison_def(l)
              end
            end) << "\n"
          when /^\s*%astgen-union\s*$/
            o << (LineWriter.lines do |l|
              l << "%union {"
              l.fmt with_indent: "  " do
                @@nodes.each do |key, val|
                  l << "frma::#{val.froz_name} *_#{camel_name(key)};"
                end
              end
              l.trim << "}"
            end) << "\n"
          when /^\s*%astgen-dtors\s*$/
            o << (LineWriter.lines do |l|
              @@nodes.each do |key, val|
                s = val.bison_dtor || "if (!$$->rooted()) delete $$;"
                l << "%destructor { #{s} } <_#{camel_name(key)}>"
              end
            end) << "\n"
          when /^\s*%astgen-types\s*$/
            o << (LineWriter.lines do |l|
              @@nodes.each do |key, val|
                l << "%type <_#{camel_name(key)}> #{key}"
              end

              @@symbols.select{|k, v| v.is_a?(Symbol) }.each do |key, val|
                l << "%type <_#{camel_name(val)}> #{key}"
              end
            end) << "\n"
          when /^\s*%astgen-syntax\s*$/
            o << (LineWriter.lines do |l|
              @@nodes.each do |_, val|
                l.sep
                val.emit_bison(@@nodes, l)
              end
            end) << "\n"
          else
            o << line
        end
      end
    end
  end

  private_class_method def self.emit_token(path, data)
    File.open(data[:tokenOut], "w") do |o|
      File.foreach(data[:tokenIn]) do |line|
        case line
          when /^\s*#pragma\s+astgen\s+friends\s*$/
            o << (LineWriter.lines with_indent: "  " do |l|
            @@nodes.select{|key, val| val.froz_dep_types.include?(:Token) }
              .each do |key, val|
                l << "friend class Forma#{key.to_s};"
              end
            end) << "\n"
          else
            o << line
        end
      end
    end
  end

  private_class_method def self.ast_outs(path, data, outs:, impl:)
    list = []

    @@nodes.each do |key, val|
      p = file_path(path, key.to_s)

      list << "#{p}.cpp" if outs
      list << "#{p}.hpp" if impl
    end

    list << file_path(path, "ast.hpp") if impl

    list
  end

  private_class_method def self.list(s, path, data, impl:)
    list = ast_outs(path, data, outs: !impl, impl: impl)

    if impl
      list << data[:tokenOut]
    else
      list << data[:flexOut] << data[:bisonOut]
    end

    s << list.join(" ") << "\n"
  end

  public_class_method def self.run(&block)
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

      raise "Missing flex input" unless data[:flexIn] || do_list || do_order
      raise "Missing flex output" unless data[:flexOut] || do_list || do_order
      raise "Missing bison input" unless data[:bisonIn] || do_list || do_order
      raise "Missing bison output" unless data[:bisonOut] || do_list || do_order
      raise "Missing token input header" unless data[:tokenIn] || do_list || do_order
      raise "Missing token output header" unless data[:tokenOut] || do_list || do_order

      raise "Missing AST directory path" unless ARGV.length > 0 || do_order

      dir = ARGV.shift unless do_order

      @@quiet = true if do_list

      run_diag do
        if do_list
          ASTList.new(@@nodes, @@d).instance_eval(&block)
        else
          class_eval(&block)
        end
      end

      catch :stop do
        throw :stop if @@errored

        if do_order
          run_diag { freeze() }
          throw :stop if @@errored

          run_diag { list_order() }
          throw :stop
        end

        run_diag { scan_flex(dir, data) }
        throw :stop if @@errored

        run_diag { freeze() }
        throw :stop if @@errored

        run_diag do
          if do_list
            list($stdout, dir, data, impl: impl)
          else
            emit(dir, data)
          end
        end
      end

      raise "One or more errors have occurred." if @@errored
    end
  end
end
