require_relative 'diag'
require_relative 'linewriter'
require_relative 'vis'

require 'set'

module ASTGen
  class Node
    class Union
      attr_reader :members

      def initialize(member_types)
        @members = {}
        @member_types = member_types
      end

      def []=(name, type)
        raise "duplicate name #{name}" if @member_types.has_key?(name)

        @members[name] = type
        @member_types[name] = type
      end

      def let(type, name) self.[]=(name, type) end
    end

    class Syntaxes
      attr_reader :syntax

      def initialize(d, types)
        @d = d
        @types = types
        @syntax = {}
      end

      def [](*key) @syntax[*key] end

      def add(type, *syms, **defs)
        @d.pos("syntax #{type.inspect}") do
          raise @d.fatal_r("type #{type.inspect} not found") unless type == :self || @types.include?(type)
          # raise @d.fatal_r("syntax for #{type} already registered") if @syntax.has_key? type

          syms = syms.clone.freeze

          @syntax[type] = [] if !@syntax.has_key?(type)

          @syntax[type] << Syntax.new(*syms, **defs)
        end
      end

      def each(&block) @syntax.each(&block) end
      def each_with_index(&block) @syntax.each_with_index(&block) end
      def flat_map(&block) @syntax.flat_map(&block) end
      def has_key?(key) @syntax.has_key?(key) end
      def length; @syntax.length end
      def select(&block) @syntax.select(&block) end

      def validate(froz_type_ctors)
        each do |key, val|
          @d.pos("syntax #{key.inspect}") do
            val.each do |e|
              names = Set.new(case key
                when :self
                  [:self]
                else
                  froz_type_ctors[key][1..-1]
              end)
              rm = Set.new

              e.syms.each do |e2|
                name = case e2
                  when String
                    next
                  when Symbol
                    e2
                  when Array
                    err = catch :invalid do
                      throw :invalid, "incorrect number of arguments" unless (1..2) === e2.length
                      throw :invalid, "invalid symbol name" unless e2[0].is_a?(Symbol)
                      throw :invalid, "invalid parameter name" unless e2[1].is_a?(Symbol) || e2.length < 2
                      nil
                    end

                    @d.error("bad symbol specifier #{e2.inspect}: #{err}") if err

                    e2[1]
                  else
                    @d.error("bad syntax specifier #{e2.inspect}")
                    nil
                end

                catch :e do
                  throw :e unless name
                  throw :e, @d.error("parameter name #{name.inspect} already used") unless rm.add?(name)
                  throw :e, @d.error("unknown parameter name #{name.inspect}") unless names.delete?(name)
                end
              end

              e.defs.each do |key2, val2|
                name = case key2
                  when Symbol
                    key2
                  else
                    @d.error("bad default specifier #{key2}")
                    nil
                end

                catch :e do
                  throw :e unless name
                  throw :e, @d.error("parameter name #{name.inspect} already used") unless rm.add?(name)
                  throw :e, @d.error("unknown parameter name #{name.inspect}") unless names.delete?(name)
                end

                case val2
                  when String
                  else
                    @d.error("bad default value #{val2}")
                end
              end

              @d.error("#{names.length} unused #{names.length == 1 ? "parameter" : "parameters"} in syntax: #{names.map{|e| e.inspect }.join(", ")}") unless names.length == 0
            end
          end
        end
      end
    end

    class Syntax
      attr_reader :syms, :defs

      def initialize(*syms, **defs)
        @syms = syms
        @defs = defs
      end
    end

    class Symbl
      attr_reader :syntaxes
      protected :syntaxes

      def initialize(d, types)
        @syntaxes = Syntaxes.new(d, types)
      end

      def syntax(*syms, **defs) @syntaxes.add(*syms, **defs) end
    end

    attr_reader :bison_dtor, :froz_name, :froz_dep_types

    def initialize(name, symbols)
      @d = ASTGen.diag
      @name = name
      @d.push("node #{@name.inspect}")
      @symbol_names = symbols
      @types = Set.new
      @unions = []
      @members = {}
      @member_types = {}
      @ctors = {}
      @syntax = Syntaxes.new(@d, @types)
      @symbols = {}
      @format = {}
      @bison_dtor = nil
    end

    def union(&block)
      @d.pos("union") do
        u = Union.new(@member_types)
        @unions << u

        u.instance_eval(&block)
      end
    end

    def symbol(name, &block)
      @d.pos("symbol #{name.inspect}") do
        raise @d.fatal_r("symbol #{name} already registered for node #{@symbol_names[name].inspect}") if @symbol_names.has_key? name

        @symbol_names[name] = @name

        s = Symbl.new(@d, @types)
        @symbols[name] = s

        s.instance_eval(&block)
      end
    end

    def []=(name, type)
      raise @d.fatal_r("duplicate name #{name}") if @member_types.has_key?(name)

      @members[name] = type
      @member_types[name] = type
    end

    def let(type, name) self.[]=(name, type) end

    def ctor(type, *members)
      if type.is_a? Array
        type.flat_map.each {|e| ctor(e, *members) }

        return
      end

      @d.pos("ctor #{type.inspect}") do
        raise @d.fatal_r("type #{type} already registered") if @types.include? type

        members = members.clone.freeze

        members.each do |e|
          raise @d.fatal_r("unknown member name #{e.inspect}") unless @member_types.has_key? e
        end

        @ctors[members] = [] if !@ctors.has_key?(members)

        @types << type
        @ctors[members] << type
      end
    end

    def empty(type, with_syntax: true, with_fmt: true)
      @d.pos("empty #{type.inspect}") do
        ctor type
        syntax type if with_syntax
        fmt type if with_fmt
      end
    end

    def single(type, member, with_syntax: true, with_fmt: true)
      @d.pos("single #{type.inspect}") do
        ctor type, member
        syntax type, member if with_syntax
        fmt type, member.inspect if with_fmt
      end
    end

    def ydtor(str)
      @bison_dtor = str
    end

    def syntax(type, *syms, **defs) @syntax.add(type, *syms, **defs) end

    def fmt(type, str = nil)
      if type.is_a? Array
        type.flat_map.each do |e|
          fmt(e, str)
        end

        return
      end

      @d.pos("fmt #{type.inspect}") do
        @d.error("type #{type} not found") unless @types.include? type

        arr = []

        if str
          split = str.split(/:([a-zA-Z][a-zA-Z0-9]*)/)

          split.each_with_index do |e, i|
            if i % 2 == 0 then
              arr << e
            else
              arr << e.to_sym
            end
          end
        end

        arr.freeze

        @format[arr] = [] if !@format.has_key?(arr)

        raise @d.fatal_r("format for #{type} already registered") if @format[arr].include? type

        @format[arr] << type
      end
    end

    def froz_validate
      @member_types.each do |key, _|
        @d.error("use of reserved member name :type") if key == :type
      end

      types = @types.clone

      @syntax.each{|key, _| types.delete? key }

      @symbols.each do |_, val|
        val.send(:syntaxes).each{|key, _| types.delete? key }
      end

      @d.warn("syntax undefined for #{types.length} #{types.length == 1 ? "type" : "types"}: #{[*types].map{|e| e.inspect }.join(", ")}") unless types.length == 0

      types = @types.clone

      @format.each{|_, val| val.each{|e| types.delete? e } }

      @d.warn("format undefined for #{types.length} #{types.length == 1 ? "type" : "types"}: #{[*types].map{|e| e.inspect }.join(", ")}") unless types.length == 0

      @syntax.validate(@froz_type_ctors)

      @symbols.each do |key, val|
        @d.pos("symbol #{key.inspect}") do
          val.send(:syntaxes).validate(@froz_type_ctors)
        end
      end

      @format.each do |key, val|
        val.each do |e|
          @d.pos("fmt #{e.inspect}") do
            names = Set.new(@froz_type_ctors[e][1..-1])
            rm = Set.new

            key.each do |e2|
              name = case e2
                when String
                  next
                when Symbol
                  e2
                else
                  @d.error("bad format specifier #{e2.inspect}")
                  nil
              end

              catch :e do
                throw :e unless name
                throw :e, @d.warn("parameter name #{name.inspect} already used") unless rm.add?(name)
                throw :e, @d.error("unknown parameter name #{name.inspect}") unless names.delete?(name)
              end
            end

            @d.warn("#{names.length} unused #{names.length == 1 ? "parameter" : "parameters"} in format: #{names.map{|e| e.inspect }.join(", ")}") unless names.length == 0
          end
        end
      end
    end

    private :froz_validate

    def freeze
      @froz_name = "Forma#{@name}"
      @froz_memb_types = {}
      @froz_memb_order = []
      @froz_type_ctors = {}
      @froz_dep_types = Set.new(@member_types.values)

      @froz_dep_types.delete(@name)

      @unions.each do |e|
        @froz_memb_order.concat(e.members.map{|k, v| k })
      end

      @froz_memb_order.concat(@members.map{|k, v| k })

      @ctors.each do |key, val|
        val.each do |e|
          @froz_type_ctors[e] = [val.length > 1] + key
        end

        key.each do |e|
          @froz_memb_types[e] = [] if !@froz_memb_types.has_key?(e)

          val.each{|e2| @froz_memb_types[e] << e2 }
        end
      end

      super

      froz_validate
    end

    def emit_head(nodes, out)
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

    def emit_body(nodes, out)
      raise "Cannot emit non-frozen node!" unless frozen?

      pre = "#{@froz_name}::"
      qname = pre.clone << @froz_name

      out << (LineWriter.lines do |l|
        l << "#include <cassert>"

        l.sep << "#include \"ast/#{ASTGen.camel_name(@name)}.hpp\""

        l.sep

        @froz_dep_types.each do |e|
          l << "#include \"ast/#{ASTGen.camel_name(e)}.hpp\""
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

              skey = Set.new(key)

              s << @froz_memb_order.select{|e| skey.include? e }.map{|e| "m_#{e.to_s}(#{e.to_s})"}.join(", ")

              if @types.length > 1
                s << ", " if key.length > 0

                s << "m_type(" << (val.length > 1 ? "type" : val[0].to_s) << ")"
              end

              s << " {"
              l << s
              l.group
            end

            key.each do |e|
              l << "assert(m_#{e.to_s});"
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
              @ctors.each do |key, val|
                val.each do |e|
                  c << "case #{e.to_s}:"
                end

                key.each do |e|
                  l << "if (m_#{e.to_s}) delete m_#{e.to_s};"
                end

                l << "break;"
              end
            end
            l.trim << "}"
          else
            @ctors.each do |key, _|
              key.each do |e|
                l << "if (m_#{e.to_s}) delete m_#{e.to_s};"
              end
            end
          end
        end
        l.trim << "}"

        l.sep

        any_fmt = @format.any?{|k, _| k && !k.empty? }

        l << "void #{pre}print(std::ostream &"

        l.peek << "os" if any_fmt

        l.peek << ") const {"
        if any_fmt
          l.fmt with_indent: "  " do
            emit_fmt = lambda do |f|
              f.each do |e|
                case e
                  when String
                    l << "os << #{e.inspect};" unless e.length == 0
                  when Symbol
                    l << "m_#{e.to_s}->print(os);"
                end
              end
            end

            if @types.length > 1
              l << "switch (m_type) {"
              c = l.curr
              l.fmt with_indent: "  " do
                @format.select{|k, _| k }.each do |key, val|
                  val.each do |e|
                    c << "case #{e.to_s}:"
                  end

                  emit_fmt.call(key)
                  l << "break;"
                end
              end

              if @format.any?{|k, _| !k || k.empty? }
                l << "default: break;"
              end
              l.trim << "}"
            else
              emit_fmt.call(@format.keys.first) if @format.keys.first
            end
          end
          l.trim << "}"
        else
          l.peek << "}"
        end

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

    def emit_bison(nodes, l)
      raise "Cannot emit non-frozen node!" unless frozen?

      def emit_symbol(name, syntax, l)
        l.sep << "#{name}:"

        empty = true

        l.fmt with_indent: "  " do
          syntax.each_with_index do |pair, i|
            key, val = pair

            val.each_with_index do |e, j|
              s = e.syms.map do |e2|
                case e2
                  when String
                    "#{e2.inspect}"
                  when Symbol
                    case key
                      when :self
                        "#{e2 == :self ? @name : @member_types[e2].to_s}[#{e2.to_s}]"
                      else
                        "#{@member_types[e2].to_s}[#{e2.to_s}]"
                    end
                  when Array
                    case e2.length
                      when 1
                        e2[0]
                      when 2
                        "#{e2[0]}[#{e2[1].to_s}]"
                    end
                end
              end.join(" ")

              s << "%empty" if e.syms.length == 0

              s << " { $$ = "

              case key
                when :self
                  s << (e.defs.has_key?(:self) ? e.defs[:self] : "$self")
                else
                  s << "new #{@froz_name}("

                  ctor = @froz_type_ctors[key]

                  if ctor[0]
                    s << "#{@froz_name}::#{key.to_s}"
                    s << ", " if ctor.length > 1
                  end

                  s << ctor[1..-1].map do |e2|
                    e.defs.has_key?(e2) ? e.defs[e2] : "$#{e2.to_s}"
                  end.join(", ") << ")"
              end

              s << "; }"
              s << " |" unless i == syntax.length - 1 && j == val.length - 1

              l << s
              empty = false
            end
          end
        end

        l.peek << " %empty" if empty

        l.peek << ";"
      end

      l.group

      emit_symbol(@name, @syntax, l)

      @symbols.each do |key, val|
        emit_symbol(key, val.send(:syntaxes), l)
      end
    end
  end
end