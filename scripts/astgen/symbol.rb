module ASTGen
  class ASymbol
    module FormatBuilder
      attr_reader :format

      private def init_fmt
        @format = LooseHash.new
      end

      def fmt(alt, *syms)
        if alt.is_a?(Array)
          alt.each{|a| fmt(a, *syms) }
          return
        end

        @d.pos("fmt #{alt.inspect}") do
          if [
            verify(alt.is_a?(Symbol)) { @d.error("invalid alternative name #{@d.hl(alt)}") },
            verify(syms.is_a?(Array)) { @d.error("invalid format specifier #{@d.hl(syms)}") } &&
            syms.reduce(true) do |curr, sym|
              next false unless verify([Symbol, String].any?{|t| sym.is_a?(t) }) { @d.error("invalid format specifier #{@d.hl(sym)}") }
              curr
            end,
          ].all?
            @format[alt] = syms.clone.freeze
          end
        end
      end
    end

    class SymbolBuilder
      include FormatBuilder

      attr_reader :name, :node, :syntax

      def initialize(name, node, d)
        init_fmt
        @d = d.fork
        @name = name
        @node = node
        @syntax = LooseHash.new
        @defer_handle = nil
        @defer_to = nil
      end

      def resolved?; !@defer_handle || @defer_to end

      def resolve(name) @defer_to = name end

      def me(name) [@name, name] end

      def defer(name)
        @defer_handle = Object.new.freeze unless @defer_handle
        [@defer_handle, name]
      end

      def rule(alt, *syms, fmt: nil)
        @d.pos("rule #{[alt.inspect, *syms.map{|s| s.inspect }].join(", ")}") do
          if [
            verify(alt.is_a?(Symbol)) { @d.error("invalid alternative name #{@d.hl(alt)}") },
            syms.reduce(true) do |curr, sym|
              next false unless verify(case sym
                when Symbol, String, @defer_handle; true
                when Array; (1..2) === sym.length
                else false
              end) { @d.error("invalid symbol spec #{@d.hl(sym)}") }
              curr
            end,
          ].all?
            syms = syms.clone.freeze
            @syntax[syms] = alt

            self.fmt(alt, *fmt) if fmt
          end
        end
      end

      def make_symbol(nodes, symbols)
        node = nodes[@node]

        syntax = LooseHash.new
        @syntax.counted.each do |syms, alts|
          out = syms.map do |sym|
            if sym.is_a?(Array) && sym.length == 2 && sym[0] == @defer_handle
              next [@defer_to, *sym[1..-1]]
            end

            sym
          end

          alts.each{|a, n| syntax.addn(out, a, n: n) }
        end

        syntax = syntax.flatten do |syms, alts, uses|
          @d.error("syntax #{syms.map{|e| @d.hl(e) }.join(", ")} defined #{uses} times for " <<
            (alts.length > 1 ? "#{alts.length} alternatives: " : "") <<
            alts.map do |alt, count|
              s = @d.hl(alt)
              s << " (#{count} times)" if count > 1 && alts.length > 1
              next s
            end.join(", "))
        end
        syntax.select do |syms, alt|
          unused = Set.new(node.alt_ctors.fetch(alt, []))
          @d.pos("rule #{[alt.inspect, *syms.map{|s| s.inspect }].join(", ")}") do
            ctor_self = alt == :self
            ctor_ok = ctor_self || !node.alt_ctors.is_error?(alt)
            result = !(verify(ctor_self || node.alt_ctors.has_key?(alt)) { @d.error("unknown alternative #{@d.hl(alt)}") } &&
            syms.reduce(true) do |curr, sym|
              next false unless case sym
                when Symbol
                  unused.delete(sym)
                  ctor_ok && verify(ctor_self ? sym == :self : node.alt_ctors[alt].include?(sym)) { @d.error("member #{@d.hl(sym)} not in constructor #{@d.hl(alt)}") }
                when Array
                  case sym.length
                    when 1
                      (type,) = sym

                      type == :Token || !symbols.has_key?(type) || (
                        !symbols.is_error?(type)
                      )
                    when 2
                      (type, name) = sym
                      unused.delete(name)

                      [
                        ctor_ok && verify(ctor_self ? name == :self : node.alt_ctors[alt].include?(name)) { @d.error("member #{@d.hl(name)} not in constructor #{@d.hl(alt)}") },
                        type == :Token || !symbols.has_key?(type) || (
                          !symbols.is_error?(type) &&
                          !node.members.is_error?(name) &&
                          verify(ctor_self ? symbols[type].node == node.name : symbols[type].node == node.members[name]) do
                            @d.error("symbol #{@d.hl(type)} (of type #{@d.hl(symbols[type].node)}) incompatible with " <<
                              if ctor_self
                                "the type of the current node"
                              else
                                "type #{@d.hl(node.members[name])} of member #{@d.hl(name)}"
                              end)
                          end
                        )
                      ].all?
                  end
                when String
                  true
              end

              curr
            end && unused.empty?)

            @d.error("#{unused.length} unused #{unused.length == 1 ? "member" : "members"}: #{unused.map{|e| @d.hl(e) }.join(", ")}") unless unused.empty?

            result
          end
        end.each{|s, _| syntax.make_error(s) }

        s = ASymbol.new(@name, node, @d)

        s.syntax = syntax

        s
      end

      def inspect; "<#{self.class.name}:0x#{object_id.to_s(16)} #{@name.inspect} syntax: #{@syntax.inspect}, format: #{@format.inspect}>" end

      def diag_no_inspect; [:@d] end
    end

    def self.build(name, node, d, &block)
      b = SymbolBuilder.new(name, node, d)
      b.instance_eval(&block)
      b
    end

    attr_accessor :name, :node, :syntax, :format

    def initialize(name, node, d)
      @d = d.fork
      @name = name
      @node = node
      @syntax = {}
    end

    def emit_bison_part(part, l)
      case part
        when :type
          l << "%type <#{@node.bison_name}> #{@name}"
        when :rule
          l.sep << "#{@name}:"
          l.peek << " %empty" if @syntax.empty?

          l.fmt with_indent: "  " do
            @syntax.each_with_index do |(syms, alt), i|
              l.peek << " |" if i > 0

              l << "#{syms.length == 0 ? "%empty" : syms.map do |sym|
                case sym
                  when String; sym.inspect
                  when Symbol; "#{sym == :self ? @name : @node.members[sym]}[#{sym}]"
                  when Array
                    case sym.length
                      when 1; sym[0]
                      when 2; "#{sym[0]}[#{sym[1]}]"
                    end
                end
              end.join(" ")} { $$ = #{alt == :self ? "$self" :
                lambda do
                  ctor = @node.alt_ctors[alt]
                  syms = @node.ctors[ctor]
                  "new #{@node.qual_class_name}(#{[
                    *("#{@node.qual_class_name}::#{alt}" if syms.length > 1 || syms.each_value.any?{|v| v.length > 1 }),
                    *("#{@node.qual_class_name}::#{@node.sym_name(@name)}" if syms.each_key.any?{|k| k && k.length > 1 }),
                    *@node.alt_ctors[alt].map{|a| "$#{a}" },
                  ].join(", ")})"
                end.call}; }"
            end
          end

          l.peek << ";"
      end
    end

    def diag_no_inspect; [:@d, :@node] end
  end
end