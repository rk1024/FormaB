require 'fileutils'
require 'getoptlong'
require 'io/console'
require 'set'

class LineWriter
  class BaseFormatter
    def initialize(arr)
      @arr = arr
      @sep = 0
    end

    def trim
      @sep.times { @arr.pop }
      @sep = 0
    end

    def <<(val)
      str = val.to_s
      str.rstrip!
      relay str, is_first:@arr.length == 0
    end

    def relay(str, is_first:)
      if str.strip.length == 0
        @sep += 1
      else
        @sep = 0
      end

      @arr << str

      self
    end

    def sep(spacing, and_merge: true, unless_first: true)
      return self if unless_first and @arr.length == 0

      spacing -= @sep if and_merge

      spacing.times { @arr << "" }

      @sep += spacing

      self
    end
  end

  class Formatter
    attr_reader :spacing, :indent

    def initialize(with_spacing: 0, with_indent: "", child_of: nil)
      @spacing = with_spacing
      @indent = with_indent
      @parent = child_of
      @first = true
    end

    def <<(val)
      sep @spacing

      str = val.to_s
      str.rstrip!

      relay str, is_first: @first
    end

    def relay(str, is_first:)
      sep @spacing if is_first

      str.prepend(@indent) if str.length > 0

      @first = false if @first

      @parent.relay str, is_first: is_first

      self
    end

    def sep(spacing = 1, and_merge: true, unless_first: true)
      return self if unless_first and @first

      @parent.sep spacing, and_merge: and_merge, unless_first: unless_first

      self
    end

    def group
      sep @spacing

      @first = true
    end

    def close; @parent = nil end
  end

  def initialize()
    @lines = []
    @base_fmt = BaseFormatter.new(@lines)
    @stack = [@base_fmt]
  end

  def self.lines(with_spacing: 0, with_indent: "")
    l = LineWriter.new()

    l.fmt with_spacing: with_spacing, with_indent: with_indent do
      yield l
    end

    l.to_s
  end

  def curr; @stack.last end

  def fmt(with_spacing: 0, with_indent: "")
    @stack.push Formatter.new(
      with_spacing: with_spacing,
      with_indent: with_indent,
      child_of: curr
    )

    yield

    curr.close
    @stack.pop
  end

  def group; curr.group; self end

  def trim; @base_fmt.trim; self end

  def <<(val) curr << val; self end

  def sep(spacing = 1, and_merge: true, unless_first: true)
    curr.sep spacing, and_merge: and_merge, unless_first: unless_first
    self
  end

  def to_s; "#{@lines.join("\n")}\n" end
end

class Vis
  def initialize(l, vis)
    @l = l.curr
    @curr = vis
  end

  def <<(vis)
    if @curr != vis
      @l.sep
      @l << "#{vis.to_s}:"

      @curr = vis
    end

    self
  end
end

class ASTGen
  @@nodes = {}

  class Node
    class Union
      attr_reader :members, :member_types

      def initialize(member_types)
        @members = {}
        @member_types = member_types
      end

      def []=(name, type)
        raise "Duplicate name #{name}" if @member_types.has_key?(name)

        @members[name] = type
        @member_types[name] = type
      end

      def let(type, name); self.[]=(name, type); end
    end

    attr_reader :froz_dep_types

    protected :froz_dep_types

    def initialize(name)
      @name = name
      @types = Set.new
      @unions = []
      @members = {}
      @member_types = {}
      @ctors = {}
      @format = {}
    end

    def union(&block)
      u = Union.new(@member_types)
      @unions << u

      u.instance_eval(&block)

      self
    end

    def []=(name, type)
      raise "Duplicate name #{name}" if @member_types.has_key?(name)

      @members[name] = type
      @member_types[name] = type
    end

    def let(type, name); self.[]=(name, type); end

    def ctor(type, *members)
      if type.is_a? Array
        type.flat_map.each do |e|
          ctor(e, *members)
        end

        return
      end

      raise "Type #{type} already registered" if @types.include? type

      members = members.clone.freeze

      @ctors[members] = [] if !@ctors.has_key?(members)

      @types << type
      @ctors[members] << type

      self
    end

    def fmt(type, str)
      split = str.split(/:([a-zA-Z][a-zA-Z0-9]*)/)

      arr = []

      split.each_with_index do |e, i|
        if i % 2 == 0 then
          arr << e
        else
          arr << e.to_sym
        end
      end

      type = [type] unless type.is_a? Array

      @format[type] = arr
    end

    def freeze
      @froz_name = "Forma#{@name}"
      @froz_memb_types = {}
      @froz_type_membs = {}
      @froz_dep_types = Set.new(@member_types.values)

      @froz_dep_types.delete(@name)

      @ctors.each do |key, val|
        key.each do |e|
          @froz_memb_types[e] = [] if !@froz_memb_types.has_key?(e)

          val.each{|e2| @froz_memb_types[e] << e2 }
        end
      end

      membs = Set.new (@member_types.map{|key, | key})

      @ctors.each do |key, val|
        membs -= key

        @froz_type_membs[val] = key
      end

      super
    end

    def emit_head(nodes, out, indent: "")
      raise "Cannot emit non-frozen node!" unless frozen?

      out << (LineWriter.lines do |l|
        l << "#pragma once"

        l.sep

        l << "#include \"ast/astBase.hpp\""

        l.sep

        l << "namespace frma {"
        l.group

        @froz_dep_types.each do |e|
          l << "class Forma#{e.to_s};"
        end

        l.sep

        l << "class #{@froz_name} : public FormaAST {"
        l.group

        vis = Vis.new(l, :private)

        l.fmt with_indent: "  " do
          if @types.length > 1
            vis << :public
            l << "enum Type {"
            l.fmt with_indent: "  " do
              @types.each do |e|
                l << "#{e.to_s},"
              end
            end
            l.trim << "};"
          end

          @unions.each do |e|
            l.sep

            vis << :private
            l << "union {"

            l.fmt with_indent: "  " do
              e.members.each do |key, val|
                l << "Forma#{val.to_s} *m_#{key.to_s};"
              end
            end

            l.trim << "};"
          end

          l.sep

          @members.each do |key, val|
            vis << :private
            l << "Forma#{val.to_s} *m_#{key} = nullptr;"
          end

          l.sep

          if @types.length > 1
            vis << :private
            l << "Type m_type;"
          end

          l.sep

          @ctors.map do |key, val|
            s = @froz_name.clone << "("

            if val.length > 1
              s << "Type"
              s << ", " if key.length > 0
            end

            s << key.map do |e|
              "Forma#{@member_types[e].to_s} *"
            end.join(", ") << ");"
          end.each do |e|
            vis << :public
            l << e
          end

          l.sep

          vis << :public
          l << "virtual ~#{@froz_name}() override;"

          l.sep

          vis << :public
          l << "virtual void print(std::ostream &) const override;"

          l.sep

          @member_types.each do |key, val|
            vis << :public
            l << "const Forma#{val.to_s} *#{key.to_s}() const;"
          end

          l.sep

          nodes.select{|key, val| val.froz_dep_types.include?(@name) }
            .each do |key, val|
              l << "friend class Forma#{key.to_s};"
            end
        end

        l.trim << "};"
        l.trim << "}"
      end) << "\n"
    end

    def emit_body(nodes, out, indent: "")
      raise "Cannot emit non-frozen node!" unless frozen?

      pre = "#{@froz_name}::"
      qname = pre.clone << @froz_name

      out << (LineWriter.lines do |l|
        l << "#include \"ast/#{ASTGen.file_name(@name)}.hpp\""

        l.sep

        @froz_dep_types.each do |e|
          l << "#include \"ast/#{ASTGen.file_name(e)}.hpp\""
        end

        l.sep << "namespace frma {"
        l.group

        @ctors.each do |key, val|
          s = qname.clone << "("

          if val.length > 1
            s << "Type type"
            s << ", " if key.length > 0
          end

          s << key.map{|e| "Forma#{@member_types[e].to_s} *#{e.to_s}"}
            .join(", ") << ")"

          delg = key.length > 0 || val.length > 1 || @types.length > 1

          s << " {" unless delg

          l.sep << s
          l.group unless delg

          l.fmt with_indent: "  " do
            if delg
              s = ": "

              s << key.map{|e| "m_#{e.to_s}(#{e.to_s})"}.join(", ")

              if @types.length > 1
                s << ", " if key.length > 0

                s << "m_type(" << (val.length > 1 ? "type" : val[0].to_s) << ")"
              end

              s << " {"
              l << s
              l.group
            end

            key.each do |e|
              l << "m_#{e.to_s}->m_rooted = true;"
            end
          end
          l.trim << "}"
        end

        l.sep

        l << "#{pre}~#{@froz_name}() {"
        l.fmt with_indent: "  " do
          if @types.length > 1
            l << "switch (m_type) {"
            c = l.curr
            l.fmt with_indent: "  " do
              @froz_type_membs.each do |key, val|
                key.each do |e|
                  c << "case #{e.to_s}:"
                end

                val.each do |e|
                  l << "if (m_#{e.to_s}) delete m_#{e.to_s};"
                end

                l << "break;"
              end
            end
            l.trim << "}"
          else
            @member_types.each do |key, |
              l << "if (m_#{key.to_s}) delete m_#{key.to_s};"
            end
          end
        end
        l.trim << "}"

        l.sep

        l << "void #{pre}print(std::ostream &os) const {"
        l.fmt with_indent: "  " do
          emit_fmt = lambda do |f|
            raise "Missing format specification for #{@name}!" unless f
            f.each do |e|
              case e
              when String
                l << "os << \"#{e}\";" unless e.length == 0
              when Symbol
                l << "m_#{e.to_s}->print(os);"
              end
            end
          end

          if @types.length > 1
            l << "switch (m_type) {"
            c = l.curr
            l.fmt with_indent: "  " do
              @format.each do |key, val|
                key.each do |e|
                  c << "case #{e.to_s}:"
                end

                emit_fmt.call(val)
                l << "break;"
              end
            end
            l.trim << "}"
          else
            emit_fmt.call(@format[[@types.first]])
          end
        end
        l.trim << "}"

        @member_types.each do |key, val|
          l.sep << "const Forma#{val.to_s} *#{pre}#{key.to_s}() const {"
          l.fmt with_indent: "  " do
            if @types.length > 1 && @froz_memb_types[key]
              mtypes = @froz_memb_types[key]

              if mtypes.length > 1
                l << "switch (m_type) {"
                c = l.curr
                l.fmt with_indent: "  " do
                  mtypes.each do |e|
                    c << "case #{e.to_s}:"
                  end

                  l << "return m_#{key.to_s};"

                  c << "default:"
                  l << "return nullptr;"
                end
                l.trim << "}"
              else
                l << "if (m_type == #{mtypes[0]})"
                l.fmt with_indent: "  " do
                  l << "return m_#{key.to_s};"
                end

                l.sep << "return nullptr;"
              end
            else
              l << "return m_#{key.to_s};"
            end
          end
          l.trim << "}"
        end

        l.trim << "}"
      end) << "\n"
    end
  end

  def self.begin(&block)
    self.class_eval(&block)

    self
  end

  def self.node(name, &block)
    raise "Duplicate node name #{name}" if @@nodes.has_key?(name)

    n = Node.new(name)
    @@nodes[name] = n

    n.instance_eval(&block)

    self
  end

  def self.freeze
    @@nodes.each{|key, val| val.freeze}
  end

  def self.file_name(name)
    name[0].downcase << name[1..-1]
  end

  def self.file_path(path, name)
    File.join("#{path}", "#{file_name(name)}")
  end

  def self.emit(path, data)
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
        l << "#include \"ast/#{file_name(key.to_s)}.hpp\""
      end
    end)

    self
  end

  def self.emit_flex(path, data)
    File.open("")
  end

  def self.list(s, path, data, impl:)
    list = @@nodes.map do |key, val|
      "#{file_path(path, key.to_s)}.#{impl ? "hpp" : "cpp"}"
    end

    if impl
      list << file_path(path, "ast.hpp")
    else
      list << data[:flexOut] << data[:bisonOut]
    end

    $stderr << list.inspect << "\n"

    s << list.join(" ") << "\n"
  end

  def self.run
    opts = GetoptLong.new(
      ["--list", "-l", GetoptLong::NO_ARGUMENT],
      ["--implicit", "-i", GetoptLong::NO_ARGUMENT],
      ["--flex", "-f", GetoptLong::REQUIRED_ARGUMENT],
      ["--bison", "-b", GetoptLong::REQUIRED_ARGUMENT],
    )

    list = impl = false

    data = {
      :flexIn => nil,
      :flexOut => nil,
      :bisonIn => nil,
      :bisonOut => nil,
    }

    opts.each do |opt, arg|
      case opt
      when "--list"
        list = true

      when "--implicit"
        list = true
        impl = true

      when "--flex"
        data[:flexIn], data[:flexOut] = arg.split(":")

      when "--bison"
        data[:bisonIn], data[:bisonOut] = arg.split(":")
      end
    end

    raise "Missing flex input" unless data[:flexIn]
    raise "Missing flex output" unless data[:flexOut]
    raise "Missing bison input" unless data[:bisonIn]
    raise "Missing bison output" unless data[:bisonOut]

    raise "Missing AST directory path" if ARGV.length == 0

    dir = ARGV.shift

    if list then self.list($stdout, dir, data, impl: impl)
    else self.emit(dir, data) end
  end
end

ASTGen.begin do
  node :Prims do
    let :Prims, :prims
    let :Prim, :prim

    ctor :Prim, :prim
    ctor :Prims, :prims, :prim

    fmt :Prim, ":prim"
    fmt :Prims, ":prims\u00B7:prim"
  end

  node :Prim do
    union do
      let :Token, :token
      let :Group, :group
      let :RawBlk, :rawblk
      let :MetaBlk, :metablk
    end

    toks = [
      :Identifier,
      :PPDirective,
      :Number,
      :Operator,
      :SQLiteral,
      :DQLiteral
    ]

    ctor :Group, :group
    ctor :RawBlk, :rawblk
    ctor :MetaBlk, :metablk
    ctor toks, :token

    fmt :Group, ":group"
    fmt :RawBlk, ":rawblk"
    fmt :MetaBlk, ":metablk"
    fmt toks, ":token"
  end

  node :RawBlk do
    let :Token, :id
    let :Token, :body

    ctor :RawBlk, :id, :body

    fmt :RawBlk, "@!:id{:body@}"
  end

  node :MetaBlk do
    ctor :MetaBlk

    fmt :MetaBlk, "@!meta{...@}"
  end

  node :Group do
    let :Prims, :prims

    ctor :PGroup, :prims
    ctor :KGroup, :prims
    ctor :CGroup, :prims

    fmt :PGroup, "(\u00B7:prims\u00B7)"
    fmt :KGroup, "[\u00B7:prims\u00B7]"
    fmt :CGroup, "{\u00B7:prims\u00B7}"
  end



  node :MDecls do
    let :MDecls, :decls
    let :MDecl, :decl

    ctor :Decl, :decl
    ctor :Decls, :decls, :decl
  end

  node :MDecl do
    union do
      let :MSyntaxExt, :ext
      let :MLetDecl, :let
    end

    ctor :SyntaxExt, :ext
    ctor :LetDecl, :let
  end

  node :MSyntaxExt do
    let :Prims, :prims
    let :Token, :id
    let :MRecord, :rec

    ctor :SyntaxExt, :prims, :id, :rec

    fmt :SyntaxExt, "syntax(:prims)\\nlet :id = :rec"
  end

  node :MLetDecl do
    let :MLetLHS, :lhs
    let :MLetRHS, :rhs

    ctor :LetDecl, :lhs, :rhs

    fmt :LetDecl, ""
  end

  node :MLetLHS do
    ctor :LetLHS

    fmt :LetLHS, ""
  end

  node :MLetRHS do
    ctor :LetRHS

    fmt :LetRHS, ""
  end

  node :MRecord do
    ctor :Record

    fmt :Record, ""
  end
end.freeze

ASTGen.run()
