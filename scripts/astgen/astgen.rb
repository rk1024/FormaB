require_relative 'diag'
require_relative 'linewriter'
require_relative 'node'
require_relative 'token'

require 'getoptlong'
require 'set'

module ASTGen
  class ASTList
    def initialize(nodes, d)
      @d = d
      @nodes = nodes
    end

    def node(name)
      raise @d.fatal_r("Duplicate node name #{name}") if @nodes.has_key?(name)

      @nodes[name] = true

      self
    end
  end

  @@tokens = {}
  @@nodes = {}
  @@symbols = {}
  @@errored = false
  @@quiet = false

  def self.quiet?; @@quiet end

  def self.error!; @@errored = true end

  def self.diag
    Diagnostics.new(
      on_error: lambda { self.error! },
      quiet: lambda { self.quiet? },
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
        e.backtrace[1..-1].map{|e| " " * 8 + "from " << e.to_s << "\n"}.join
      error!
    end
  end

  private_class_method def self.token(name, *args, **kwargs)
    raise @@d.fatal_r("Duplicate token name #{name}") if @@tokens.has_key?(name)

    t = nil

    run_diag do
      t = Token.new(name, *args, **kwargs)
      @@tokens[name] = t
    end

    t
  end

  def self.node(name, &block)
    raise @@d.fatal_r("Duplicate node name #{name}") if @@nodes.has_key?(name)

    run_diag do
      n = Node.new(name, @@symbols)
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

  private_class_method def self.emit(path, data)
    @@nodes.each do |key, val|
      FileUtils.mkdir_p(path) unless File.directory?(path)

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

    emit_flex(path, data)
    emit_bison(path, data)
    emit_token(path, data)

    self
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
                contents = s.scan(/[^"]*/)

                throw :err, "'\"'" unless s.scan(/"/)

                t = token(sym, str: contents)
              elsif s.scan(/\//)
                contents = s.scan(/[^\/]*/)

                throw :err, "'/'" unless s.scan(/\//)

                t = token(sym, pat: contents)
              else
                throw :err, "'\"' or '/'"
              end

              s.scan(/\s+/)

              if t
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
              end

              "end of line" if s.eos?

              nil
            end

            @@d.error("unexpected #{s.eos? ? "end of line" : "'#{s.rest}'"} (expecting #{err}) #{pos.call}") if err
          when /^\s*%astgen-token\b(?!-)/
            @@d.error("invalid syntax for %astgen-token #{pos.call}")
        end
      end
    end
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

              @@symbols.each do |key, val|
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

  private_class_method def self.list(s, path, data, impl:)
    list = @@nodes.map do |key, val|
      "#{file_path(path, key.to_s)}.#{impl ? "hpp" : "cpp"}"
    end

    if impl
      list << file_path(path, "ast.hpp") << data[:tokenOut]
    else
      list << data[:flexOut] << data[:bisonOut]
    end

    s << list.join(" ") << "\n"
  end

  public_class_method def self.run(&block)
    @@d.pos("ASTGen.run") do
      opts = GetoptLong.new(
        ["--list", "-l", GetoptLong::NO_ARGUMENT],
        ["--implicit", "-i", GetoptLong::NO_ARGUMENT],
        ["--flex", "-f", GetoptLong::REQUIRED_ARGUMENT],
        ["--bison", "-b", GetoptLong::REQUIRED_ARGUMENT],
        ["--token", "-t", GetoptLong::REQUIRED_ARGUMENT],
      )

      do_list = impl = false

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
          when "--list"
            do_list = true

          when "--implicit"
            do_list = true
            impl = true

          when "--flex"
            data[:flexIn], data[:flexOut] = arg.split(":")

          when "--bison"
            data[:bisonIn], data[:bisonOut] = arg.split(":")

          when "--token"
            data[:tokenIn], data[:tokenOut] = arg.split(":")
        end
      end

      raise "Missing flex input" unless data[:flexIn]
      raise "Missing flex output" unless data[:flexOut]
      raise "Missing bison input" unless data[:bisonIn]
      raise "Missing bison output" unless data[:bisonOut]
      raise "Missing token input header" unless data[:tokenIn]
      raise "Missing token output header" unless data[:tokenOut]

      raise "Missing AST directory path" if ARGV.length == 0

      dir = ARGV.shift

      @@quiet = true if do_list

      run_diag do
        if do_list
          ASTList.new(@@nodes, @@d).instance_eval(&block)
        else
          class_eval(&block)
        end
      end

      run_diag { scan_flex(dir, data) } unless do_list || @@errored

      freeze

      raise "One or more errors have occurred." if @@errored

      if do_list then run_diag { list($stdout, dir, data, impl: impl) }
      else run_diag { emit(dir, data) } end

      raise "One or more internal errors have occurred." if @@errored
    end
  end
end
