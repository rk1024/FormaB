##########################################################################
#
# FormaB - the bootstrap Forma compiler (symbol.rb)
# Copyright (C) 2017-2018 Ryan Schroeder, Colin Unger
#
# FormaB is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as
# published by the Free Software Foundation, either version 3 of the
# License, or (at your option) any later version.
#
# FormaB is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Affero General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with FormaB.  If not, see <https://www.gnu.org/licenses/>.
#
##########################################################################

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

      attr_reader :name, :node, :syntax, :actions

      def initialize(name, node, d)
        init_fmt
        @d = d.fork
        @name = name
        @node = node
        @syntax = LooseHash.new
        @actions = LooseHash.new
        @defer_handle = nil
        @defer_to = nil
      end

      def resolved?; !@defer_handle || @defer_to end
      # def has_syntax?(nodes, symbols)
      #   node = nodes[@node]

      #   @syntax.counted.any? do |syms, _|
      #     syms.all? do |s|
      #       s = case s
      #         when Symbol; node.members[s]
      #         when Array
      #           case s.length
      #             when 1, 2; s[0]
      #             else nil
      #           end
      #         else nil
      #       end
      #       !symbols.has_key?(s) || verify(symbols.isnt_error?(s) && symbols[s].has_syntax?(nodes, symbols)) { @d.debug("symbol #{@d.hl(s)} not matchable") if Symbol === s }
      #     end
      #   end
      # end

      def resolve(name) @defer_to = name end

      def me(name) [@name, name] end

      def defer(name)
        @defer_handle = Object.new.freeze unless @defer_handle
        [@defer_handle, name]
      end

      def rule(alt, *syms, fmt: nil, action: nil)
        @d.pos("rule #{[alt.inspect, *syms.map{|s| s.inspect }].join(", ")}") do
          if [
            verify(alt.is_a?(Symbol)) { @d.error("invalid alternative name #{@d.hl(alt)}") },
            verify(action.nil? || action.is_a?(String)) { @d.error("invalid action #{@d.hl(action)}") },
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
            self.action(alt, action) if action
          end
        end
      end

      def action(alt, action) @actions[alt] = action end

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
                          (name == :self ? ctor_self : node.members.isnt_error?(name)) &&
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

        actions = @actions.flatten do |alt, acts, uses|
          @d.error("action defined for syntax #{alt} #{uses} times")
        end

        s = ASymbol.new(@name, node, @d)

        s.syntax = syntax
        s.actions = actions

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

    attr_accessor :name, :node, :syntax, :format, :actions

    def initialize(name, node, d)
      @d = d.fork
      @name = name
      @node = node
      @syntax = ErrorableHash.new({}, Set.new)
      @actions = ErrorableHash.new({}, Set.new)
    end

    def alias_for(symbols = nil, exclude = Set.new)
      ignore = exclude + [@name]

      return nil unless @syntax.length == 1

      (syms, alt) = @syntax.first
      return nil unless alt == :self && syms.length == 1

      sym = syms.first
      return nil unless Array === sym && sym.length == 2 && sym[1] == :self

      return false if ignore.include?(sym[0])

      if symbols && symbols.isnt_error?(sym[0])
        aliases = symbols[sym[0]].alias_for(symbols, ignore)

        return false if aliases == false # Propagate false but not nil

        return aliases if aliases
      end

      return sym[0]
    end

    def symbol_from_spec(spec)
      case spec
        when Symbol; return @node.members[spec] unless @node.members.is_error?(spec)
        when Array; return spec[0] if (1..2) === spec.length
      end
      nil
    end

    def prune_syntax(symbols)
      @syntax.select do |syms, _|
        syms.any?{|s| symbols.is_error?(symbol_from_spec(s)) }
      end.each{|s, _| @syntax.make_error(s) }
    end

    def expand_aliases(symbols)
      @syntax.each_key do |syms|
        syms.map! do |sym|
          name = symbol_from_spec(sym)
          aliases = symbols.isnt_error?(name) ? symbols[name].alias_for(symbols) : nil

          case sym
            when Symbol; sym = [aliases, sym] if aliases
            when Array
              if (1..2) === sym.length && aliases
                sym = [*sym]
                sym[0] = aliases
              end
          end

          sym
        end
      end

      @syntax.rehash do |key, val|
        @d.warn("rule #{@d.hl(key)} causes conflicts")
      end
    end

    def used_syms
      Set[*@syntax.each_key.flat_map{|ss| ss.map{|s| symbol_from_spec(s) }.select{|s| s } }]
    end

    def emit_bison_part(part, l)
      case part
        when :type
          l << "%type <#{@node.bison_name}> #{@name}"
        when :rule
          l.sep << "#{@name}:"
          if @syntax.empty?
            l.peek << " %empty"
            @d.warn("emitting empty syntax; this shouldn't happen")
          end

          l.fmt with_indent: "  " do
            @d.p(@syntax, long: true) if @name === :PraeExpression
            @syntax.each_with_index.select{|(_, a), _| !@actions.is_error?(a) }.each do |(syms, alt), i|
              @d.p([syms, alt], long: true) if @name === :PraeExpression
              l.peek << " |" if i > 0

            l << "#{syms.none? ? "%empty" : syms.map do |sym|
                case sym
                  when String; sym.inspect
                  when Symbol; "#{sym == :self ? @name : @node.members[sym]}[#{sym}]"
                  when Array
                    case sym.length
                      when 1; sym[0]
                      when 2; "#{sym[0]}[#{sym[1]}]"
                    end
                end
              end.join(" ")} { #{"do { #{@actions[alt]} } while (false); " if @actions[alt]}$$ = #{alt == :self ? "$self" :
                begin
                  ctor = @node.alt_ctors[alt]
                  syms = @node.ctors[ctor]
                  "new #{@node.qual_class_name}(#{[
                    *("#{@node.qual_class_name}::#{alt}" if syms.length > 1 || syms.each_value.any?{|v| v.length > 1 }),
                    *("#{@node.qual_class_name}::#{@node.sym_name(@name)}" if syms.each_key.any?{|k| k && k.length > 1 }),
                    *@node.alt_ctors[alt].map{|a| "$#{a}" },
                    "@$",
                  ].join(", ")})"
                end}; }"
            end
          end

          l.peek << ";"
      end
    end

    def diag_no_inspect; [:@d, :@node] end
  end
end