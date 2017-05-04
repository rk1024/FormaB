require_relative 'diag'
require_relative 'linewriter'
require_relative 'node'

require 'getoptlong'
require 'set'

module ASTGen
  @@nodes = {}
  @@symbols = {}
  @@errored = false
  @@quiet = false

  def self.quiet?; @@quiet end

  def self.error!; @@errored = @@errored || @@quiet end

  def self.diag
    Diagnostics.new(
      on_error: lambda { self.error! },
      quiet: lambda { self.quiet? },
    )
  end

  def self.node(name, &block)
    raise "Duplicate node name #{name}" if @@nodes.has_key?(name)

    begin
      n = Node.new(name, @@symbols)
      @@nodes[name] = n

      n.instance_eval(&block)
    rescue => e
      $stderr << e.backtrace[0] << ":#{e.to_s} (#{e.class})\n" <<
        e.backtrace[1..-1].map{|e| " " * 8 + "from " << e.to_s << "\n"}.join
      error!
    end

    self
  end

  private_class_method def self.freeze
    @@nodes.each{|key, val| val.freeze}
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

      f = File.open("#{fname}.hpp", "w")
      val.emit_head(@@nodes, f)
      f.close

      f = File.open("#{fname}.cpp", "w")
      val.emit_body(@@nodes, f)
      f.close
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

  private_class_method def self.emit_flex(path, data)
    o = File.open(data[:flexOut], "w")
    File.open(data[:flexIn], "r").each do |l|
      o << l
    end.close
    o.close
  end

  private_class_method def self.emit_bison(path, data)
    o = File.open(data[:bisonOut], "w")
    File.open(data[:bisonIn], "r").each do |line|
      case line
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
    end.close
    o.close
  end

  private_class_method def self.emit_token(path, data)
    o = File.open(data[:tokenOut], "w")
    File.open(data[:tokenIn], "r").each do |line|
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
    end.close
    o.close
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

    class_eval(&block)

    freeze

    raise "One or more errors have occurred." if @@errored

    if do_list then list($stdout, dir, data, impl: impl)
    else emit(dir, data) end
  end
end
