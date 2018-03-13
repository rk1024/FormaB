##########################################################################
#
# FormaB - the bootstrap Forma compiler (node.rb)
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

require_relative 'diag'
require_relative 'verify'
require_relative 'linewriter'
require_relative 'loosehash'
require_relative 'symbol'
require_relative 'vis'

require 'set'

module ASTGen
  class Node
    class ObjBuilder
      attr_reader :members, :member_order

      def initialize(d)
        @d = d.fork
        @members = LooseHash.new
        @member_order = []
      end

      def let(type, name)
        @d.pos("let #{type.inspect}, #{name.inspect}") do
          if [
            verify(type.is_a?(Symbol)) { @d.error("invalid member type #{@d.hl(type)}") },
            verify(name.is_a?(Symbol)) { @d.error("invalid member name #{@d.hl(name)}") },
          ].all?
            @member_order << name unless @members.has_key?(name)
            @members[name] = type
          end
        end
      end

      def diag_no_inspect; [:@d] end
    end

    class UnionBuilder < ObjBuilder
      def inspect; "<#{self.class.name}:0x#{object_id.to_s(16)} members: #{@members.inspect}>" end
    end

    class NodeBuilder < ObjBuilder
      include ASymbol::FormatBuilder

      attr_reader :name, :extra_decls, :base_classes, :unions, :ctors, :dtor, :symbols

      def initialize(name, d)
        super(d)
        init_fmt
        @name = name
        @base_classes = nil
        @extra_decls = []
        @unions = []
        @ctors = LooseHash.new
        @dtor = nil
        @symbols = LooseHash.new
        @last_symbol = nil
      end

      def inherits(*classes) @base_classes = classes end

      def declare(decl) @extra_decls << decl end

      def union(&block)
        @d.pos("union") do
          b = UnionBuilder.new(@d)
          b.instance_eval(&block)
          @unions << b
        end
      end

      def ctor(alts, *sig, fmt: nil)
        [*alts].each do |alt|
          @d.pos("ctor #{[alt.inspect, *sig.map{|e| e.inspect }].join(", ")}") do
            if [
              verify(alt.is_a?(Symbol)) { @d.error("invalid alternative name #{@d.hl(alt)}") },
              sig.reduce(true) do |curr, arg|
                next false unless verify(arg.is_a?(Symbol)) { @d.error("invalid member name #{@d.hl(arg)}") }
                curr
              end,
            ].all?
              sig = sig.clone.freeze
              @ctors[alt] = sig

              self.fmt(alt, *fmt) if fmt
            end
          end
        end
      end

      def dtor(str)
        @d.pos("dtor #{str.inspect}") do
          if [
            verify(str.is_a?(String)) { @d.error("invalid destructor") },
            verify(!@dtor) { @d.error("duplicate destructor declaration") },
          ].all?
            @dtor = str
          end
        end
      end

      def symbol(name = @name, &block)
        @d.pos("symbol #{name.inspect}") do
          if [
            verify(name.is_a?(Symbol)) { @d.error("invalid symbol name #{@d.hl(name)}") },
          ].all?
            @symbols[name] = @last_symbol = ASymbol.build(name, @name, @d, &block)
          end
        end
      end

      def chain(name = @name, with_rule: true, &block)
        @d.pos("chain #{name.inspect}") do
          if [
            verify(name.is_a?(Symbol)) { @d.error("invalid symbol name #{@d.hl(name)}") },
          ].all?
            @symbols[name] = curr_symbol = ASymbol.build(name, @name, @d, &block)
          end

          if @last_symbol
            @last_symbol.rule :self, @last_symbol.defer(:self) if with_rule
            @last_symbol.resolve(name)
          else
            @d.error("no symbol available to chain to")
          end

          @last_symbol = curr_symbol
        end
      end

      def make_node(symbols)
        members = @members.clone
        member_order = []

        @unions.each do |union|
          members.merge!(union.members)
          member_order.concat(union.member_order)
        end

        member_order.concat(@member_order)

        members = members.flatten do |name, types, uses|
          @d.error("duplicate member #{@d.hl(name)} declared #{uses} times as " <<
            (types.length > 1 ? "#{types.length} types: " : "type ") <<
            types.map do |type, count|
              s = @d.hl(type)
              s << " (#{count} times)" if count > 1 && types.length > 1
              next s
            end.join(", "))
        end

        member_order.delete_if{|m| members.is_error?(m) }

        dep_types = Set.new(members.values)
        dep_types.delete(@name)

        struct_members = @members.flatten

        unions = @unions.map{|u| u.members.flatten }

        alt_ctors = @ctors.flatten do |alt, sigs, uses|
          @d.error("constructor for #{@d.hl(alt)} defined #{uses} times with " <<
            (sigs.length > 1 ? "#{sigs.length} signatures: " : "signature ") <<
            sigs.map do |sig, count|
              s = sig.map{|e2| @d.hl(e2) }.join(", ")
              s << " (#{count} times)" if count > 1 && sigs.length > 1
              next s
            end.join("; "))
        end
        unused = Set.new
        alt_ctors.select do |alt, sig|
          @d.pos("ctor #{[alt.inspect, *sig.map{|e| e.inspect }].join(", ")}") do
            [
              sig.any?{|a| members.is_error?(a) },
              sig.reduce(false) do |curr2, arg|
                next true unless verify(members.has_key?(arg)) { @d.error("unknown member #{@d.hl(arg)}") }
                curr2
              end,
              !verify(@symbols.any?{|_, s| s.syntax.has_value?(alt) }) { unused << alt },
            ].any?
          end
        end.each{|a, _| alt_ctors.make_error(a) }
        @d.warn("#{unused.length} unused #{unused.length == 1 ? "alternative" : "alternatives"}: #{unused.map{|e| @d.hl(e) }.join(", ")}") unless unused.empty?

        unused.clear
        members.select do |name, type|
          !verify(alt_ctors.each_value.any?{|s| s.include?(name) }) { unused << name }
        end.each{|m, _| members.make_error(m) }
        @d.warn("#{unused.length} unused #{unused.length == 1 ? "member" : "members"}: #{unused.map{|e| @d.hl(e) }.join(", ")}") unless unused.empty?

        alt_formats = LooseHash.new

        @format.counted.each do |alt, formats|
          formats.each do |fmt, count|
            alt_formats[alt] = LooseHash.new.addn(nil, fmt, n: count)
          end
        end

        @symbols.flatten.each do |name, sym|
          sym.format.counted.each do |alt, formats|
            formats.each do |fmt, count|
              alt_formats[alt] = LooseHash.new.addn(name, fmt, n: count)
            end
          end
        end

        alt_formats = alt_formats.flatten
        alt_formats.map do |alt, formats|
          [
            alt,
            formats.flatten do |sym, fmts, uses|
              @d.error("format for #{@d.hl(alt)} " <<
                (sym ? "in symbol #{@d.hl(sym)} " : "") <<
                "defined #{uses} times " <<
                (fmts.length > 1 ? "in #{fmts.length} ways: " : "as ") <<
                fmts.map do |fmt, count|
                  s = fmt.map{|e2| @d.hl(e2) }.join(", ")
                  s << " (#{count} times)" if count > 1 && fmts.length > 1
                  next s
                end.join("; "))
            end
          ]
        end.each{|k, v| alt_formats[k] = v }
        alt_formats.select do |alt, formats|
          alt_ctors.is_error?(alt)
        end.each{|k, _| alt_formats.make_error(k) }

        ctor_alts = LooseHash.new(alt_ctors).invert.dedup

        ctors = LooseHash.new
        alt_ctors.each do |alt, sig|
          catch :break do
            alt_formats.fetch(alt) do
              ctors[sig] = LooseHash.new.addn(nil, alt)
              throw :break
            end.each_key do |sym|
              ctors[sig] = LooseHash.new.addn(sym, alt)
            end
          end
        end
        ctors = ctors.dedup
        ctors.map{|a, s| [a, s.invert_grouped.dedup.invert] }.each{|a, s| ctors[a] = s }

        member_alts = LooseHash.new
        alt_ctors.each do |alt, sig|
          sig.each do |arg|
            member_alts[arg] = alt
          end
        end
        member_alts = member_alts.dedup

        format = LooseHash.new(alt_formats).invert.dedup

        undef_formats = Set.new
        alt_ctors.each_key do |alt|
          undef_formats << alt unless alt_formats.has_key?(alt)
        end
        @d.warn("format undefined for #{undef_formats.length} #{undef_formats.length == 1 ? "alternative" : "alternatives"}: #{undef_formats.map{|e| @d.hl(e) }.join(", ")}") unless undef_formats.empty?

        format_syms = Set.new

        alt_formats.each_value{|f| format_syms.merge(f.each_key) if f.length > 1 }

        n = Node.new(@name, @d)

        n.members = members
        n.base_classes = @base_classes
        n.extra_decls = @extra_decls
        n.member_order = member_order
        n.member_alts = member_alts
        n.dep_types = dep_types
        n.struct_members = struct_members
        n.unions = unions
        n.alt_ctors = alt_ctors
        n.ctor_alts = ctor_alts
        n.ctors = ctors
        n.dtor = @dtor ? @dtor.clone : nil
        n.alt_formats = alt_formats
        n.format = format
        n.format_syms = format_syms

        n
      end

      def inspect; "<#{self.class.name}:0x#{object_id.to_s(16)} #{@name.inspect} base_classes: #{@base_classes.inspect}, members: #{@members.inspect}, unions: #{@unions.inspect}, ctors: #{@ctors.inspect}, fmt: #{@fmt.inspect}, symbols: #{@symbols.inspect}>" end
    end

    def self.build(name, d, &block)
      b = NodeBuilder.new(name, d)
      b.instance_eval(&block)
      b
    end

    attr_accessor(
      :name,
      :base_classes,
      :extra_decls,
      :members,
      :member_order,
      :member_alts,
      :dep_types,
      :struct_members,
      :unions,
      :alt_ctors,
      :ctor_alts,
      :ctors,
      :dtor,
      :alt_formats,
      :format,
      :format_syms,
      :unused,
    )

    def initialize(name, d)
      @d = d.fork
      @name = name
      @base_classes = nil
      @extra_decls = []
      @members = {}
      @member_order = []
      @member_alts = {}
      @dep_types = Set.new
      @struct_members = {}
      @unions = {}
      @alt_ctors = {}
      @ctor_alts = {}
      @ctors = {}
      @dtor = nil
      @alt_formats = {}
      @format = {}
      @format_syms = []
      @unused = false
    end

    @@addl_includes = []
    @@namespace = []
    @@namespace_token = []
    @@class_name_prefix = ""
    @@class_base = ""
    @@class_token = "Token"
    @@header_dir = ""
    @@header_base = "node.hpp"
    @@header_token = "token.hpp"
    @@AltEnumName = "Alt"
    @@AltMembName = "alt"
    @@SymEnumName = "Sym"
    @@SymMembName = "sym"
    @@SymNoneName = "_None"

    def self.AltEnumName; @@AltEnumName end
    def self.AltMembName; @@AltMembName end
    def self.SymEnumName; @@SymEnumName end
    def self.SymMembName; @@SymMembName end
    def self.SymNoneName; @@SymNoneName end

    @@node_name_prefixes = []
    @@node_name_suffixes = []
    @@node_name_abbrs = []

    def self.split_namespace(value) value.to_s.gsub(/^::/, "").split("::") end

    def self.add_include(value) @@addl_includes << value.to_s end

    def self.namespace; @@namespace.join("::") end
    def self.namespace=(value) @@namespace = split_namespace(value) end

    def self.class_prefix; @@class_name_prefix end
    def self.class_prefix=(value) @@class_name_prefix = value end

    def self.class_base; @@class_base end
    def self.class_base=(value) @@class_base = value end

    def self.class_token; "#{@@namespace_token.join("::")}::#{@@class_token}" end

    def self.class_token=(value)
      (*@@namespace_token, @@class_token) = split_namespace(value)
    end

    def self.header_dir; @@header_dir end
    def self.header_dir=(value) @@header_dir = value end

    def self.header_base; @@header_base end
    def self.header_base=(value) @@header_base = value end

    def self.header_token; @@header_token end
    def self.header_token=(value) @@header_token = value end

    def self.name_prefixes=(value) @@node_name_prefixes = value.map{|p, s| [s, /^#{p.to_s.downcase}/i]} end
    def self.name_suffixes=(value) @@node_name_suffixes = value.map{|p, s| [s, /(?<!^)#{p.to_s.downcase}$/i]} end
    def self.name_abbrevs=(value)
      @@node_name_abbrs = value.map do |from, to, from_plur, to_plur|
        [
          /#{from}(?!\p{Ll})/,
          to,
          /#{from_plur || "#{from}s"}(?!\p{Ll})/,
          to_plur || "#{to}s",
        ]
      end
    end

    def self.just_class_token
      return @@class_token if @@namespace.any? && @@namespace == @@namespace_token
      class_token
    end

    def self.class_name(name)
      name = name.to_s
      return just_class_token if name == "Token"

      unprefixed = @@node_name_prefixes.reduce(name) {|s, (_, r)| s.gsub(r, "") }

      "#{@@class_name_prefix}#{[
        *@@node_name_prefixes.select{|_, r| name =~ r}.map{|p, _| p },
        *@@node_name_suffixes.select{|_, r| unprefixed =~ r}.map{|p, _| p },
      ].join}#{@@node_name_abbrs.reduce(
        @@node_name_suffixes.reduce(unprefixed) {|s, (_, r)| s.gsub(r, "") }
      ) {|s, (f, t, fp, tp)| s.gsub(f, t).gsub(fp, tp) }}"
    end

    def self.just_class_name(name) @@namespace.none? ? qual_class_name(name) : class_name(name) end

    def self.qual_class_name(name)
      name = name.to_s
      return class_token if name == "Token"
      "#{@@namespace.join("::")}::#{class_name(name)}"
    end

    def self.qual_name(klass, name) "#{class_name(klass)}::#{name}" end

    def self.header_name(name)
      name = name.to_s
      return @@header_token if name == "Token"
      "#{@@header_dir}/#{ASTGen.camel_name(name)}.hpp"
    end

    def self.field_name(name) "m_#{name}" end
    def self.bison_name(name) "_#{ASTGen.camel_name(name)}" end

    def self.emit_friends_forward(nodes, node, l)
      @@namespace.each{|n| (l << "namespace #{n} {").group }

      nodes.select{|_, n| n.depends?(node) }.each {|_, n| l << "class #{n.class_name};" }

      @@namespace.length.times{ l.trim << "}" }
    end

    def self.emit_friends(nodes, node, qual, l)
      nodes.select{|_, n| n.depends?(node) }.each do |_, _node|
        l << "friend class #{qual ? _node.qual_class_name : _node.class_name};"
      end
    end

    def self.emit_ns_start(l)
      @@namespace.each{|n| (l << "namespace #{n} {").group }
    end

    def self.emit_ns_end(l)
      @@namespace.length.times{ l.trim << "}" }
    end

    def self.emit_body_start(l)
      l << "#include <cassert>"

      l.sep

      l << "#include \"#{@@header_base}\""
      l << "#include \"#{header_name(:Token)}\""
      l << "#include \"#{header_name(:Ast)}\""

      l.sep

      emit_ns_start(l)
    end

    def self.emit_body_end(l)
      emit_ns_end(l)
    end

    def self.sym_name(sym)
      return "#{@@SymEnumName}::#{sym ? sym.to_s : @@SymNoneName}"
    end

    def class_name; Node.class_name(@name) end
    def just_class_name; Node.just_class_name(@name) end
    def qual_class_name; Node.qual_class_name(@name) end
    def qual_scoped_name; Node.qual_scoped_name(@name) end
    def header_name; Node.header_name(@name) end
    def bison_name; Node.bison_name(@name) end
    def qual_name(name) Node.qual_name(@name, name) end

    def enum_name(name)
      name = name.to_s
      name == class_name ? "_#{name}" : name
    end

    def sym_name(sym) Node.sym_name(format_syms.include?(sym) ? sym : nil) end

    def depends?(node)
      @dep_types.include?(case node
        when Symbol; node
        when Node; node.name
      end)
    end

    def printable?; @format.any?{|f, _| !f.empty? } end

    def use_alts?; @alt_ctors.length > 1 end
    def use_syms?; @format_syms.length > 1 end

    def use_default_value?(name)
      @member_alts[name].length < @alt_ctors.length
    end

    def emit_include(l)
      l << "#include \"#{header_name}\""
    end

    def emit_head(nodes)
      LineWriter.lines do |l|
        l << "#pragma once"

        l.sep

        l << "#include \"#{@@header_base}\""
        @@addl_includes.each{|i| l << "#include #{i.inspect}" }

        l.sep

        token_ns_separate = @@namespace_token != @@namespace

        if token_ns_separate
          @@namespace_token.each{|n| (l << "namespace #{n} {").group }

          l << "class #{@@class_token};"

          @@namespace_token.length.times{ l.trim << "}" }
          l.sep
        end

        Node.emit_ns_start(l)

        @dep_types.select{|t| !nodes.is_error?(t) }.each do |type|
          l << "class #{Node.class_name(type)};" unless token_ns_separate && type.to_s == "Token"
        end

        l.sep

        l << "class #{class_name} : #{(@base_classes || [@@class_base]).map{|c| "public #{c}" }.join(", ")} {"
        l.group

        vis = Vis.new(l, :private)

        l.fmt with_indent: "  " do
          if use_alts?
            vis << :public
            l << "enum #{@@AltEnumName} {"
            l.fmt with_indent: "  " do
              @alt_ctors.each do |alt, _|
                l << "#{enum_name(alt)},"
              end
            end
            l.trim << "};"
          end

          if use_syms?
            l.sep
            vis << :public
            l << "enum class #{@@SymEnumName} {"
            l.fmt with_indent: "  " do
              @format_syms.each do |sym|
                l << "#{enum_name(case sym
                  when nil; @@SymNoneName
                  else sym
                end)},"
              end
            end
            l.trim << "};"
          end

          @unions.each do |union|
            l.sep

            vis << :private
            l << "union {"
            l.fmt with_indent: "  " do
              union.select{|_, v| !nodes.is_error?(v) }.each do |key, val|
                l << "#{Node.just_class_name(val)} *#{Node.field_name(key)};"
              end
            end
            l.trim << "};"
          end

          l.sep

          @struct_members.select{|_, v| !nodes.is_error?(v) }.each do |key, val|
            vis << :private
            l << "#{Node.just_class_name(val)} *#{Node.field_name(key)};"
          end

          l.sep

          if use_alts?
            vis << :private
            l << "#{@@AltEnumName} #{Node.field_name(@@AltMembName)};"
          end

          if use_syms?
            vis << :private
            l << "#{@@SymEnumName} #{Node.field_name(@@SymMembName)};"
          end

          l.sep

          @ctors.select{|s, _| s.none?{|a| nodes.is_error?(@members[a]) } }.each do |sig, syms|
            memb_args = sig.map{|a| "#{Node.just_class_name(@members[a])} *" }

            emit_ctor = lambda do |args|

              vis << :public
              l << "#{class_name}(#{[*args, *memb_args, "const location &"].join(", ")});"
            end

            if syms.length <= 1 && syms.all?{|k, v| (!k || k.length <= 1) && v.length <= 1 }
              emit_ctor.([]) if syms.length <= 1 && syms.all?{|k, v| (!k || k.length <= 1) && v.length <= 1 }
            else
              emit_ctor.([
                "#{@@AltEnumName}",
                "#{@@SymEnumName}",
              ]) if syms.any?{|k, v| k && k.length > 1 && v.length > 1 }

              emit_ctor.([
                "#{@@SymEnumName}",
              ]) if syms.any?{|k, v| k && k.length > 1 && v.length <= 1 }

              emit_ctor.([
                "#{@@AltEnumName}",
              ]) if syms.any?{|k, v| !k || k.length <= 1 }
            end

          end

          l.sep

          vis << :public
          l << "virtual ~#{class_name}() override;"

          l.sep

          vis << :public
          l << "virtual void print(std::ostream &) const override;"

          l.sep

          @members.each do |name, type|
            vis << :public
            l << "const #{Node.just_class_name(type)} *#{name}() const;"
          end

          if use_alts?
            vis << :public
            l << "#{@@AltEnumName} #{@@AltMembName}() const;"

            l.sep
          end

          if use_syms?
            vis << :public
            l << "#{@@SymEnumName} #{@@SymMembName}() const;"

            l.sep
          end

          @extra_decls.each do |decl|
            vis << :public
            l << decl << ";"
            l.sep
          end

          l.sep

          Node.emit_friends(nodes, self, false, l)
        end
        l.trim << "};"

        Node.emit_ns_end(l)
      end << "\n"
    end

    def emit_body(nodes, l)
      @ctors.each do |sig, syms|
        sigset = Set.new(sig)
        memb_inits = @member_order.select{|m| sigset.include?(m) }
          .map{|m| "#{Node.field_name(m)}(#{m})" }

        emit_ctor = lambda do |args, inits|
          l.sep << "#{qual_name(class_name)}(#{[
            *args,
            *sig.map{|a| "#{Node.just_class_name(@members[a])} *#{a}" },
            "const location &loc"
          ].join(", ")})"
          l.fmt with_indent: "  " do
            l << ": #{[
              "FormaAST(loc)",
              *memb_inits,
              *inits,
            ].join(", ")} {"

            sig.each{|a| l << "assert(#{Node.field_name(a)});" }

            l.sep

            sig.each{|a| l << "#{Node.field_name(a)}->m_rooted = true;" }

            yield if block_given?
          end
          l.trim << "}"
        end

        if syms.length <= 1 && syms.all? {|k, v| (!k || k.length <= 1) && v.length <= 1 }
          emit_ctor.([], [
            *("#{Node.field_name(@@AltMembName)}(#{syms.first[1].first})" if use_alts?),
            *("#{Node.field_name(@@SymMembName)}(#{sym_name(syms.first[0].first)})" if use_syms?),
          ]) unless syms.empty?
        else
          items = syms.select{|k, v| k && k.length > 1 && v.length > 1 }
          emit_ctor.([
            "#{@@AltEnumName} #{@@AltMembName}",
            "#{@@SymEnumName} #{@@SymMembName}",
          ], [
            "#{Node.field_name(@@AltMembName)}(#{@@AltMembName})",
            "#{Node.field_name(@@SymMembName)}(#{@@SymMembName})",
          ]) unless items.empty?

          items = syms.select{|k, v| k && k.length > 1 && v.length <= 1 }
          emit_ctor.([
            "#{@@SymEnumName} #{@@SymMembName}",
          ], [
            "#{Node.field_name(@@SymMembName)}(#{@@SymMembName})",
          ]) do
            l << "switch(#{Node.field_name(@@SymMembName)}) {"
            c = l.curr
            l.fmt with_indent: "  " do
              items.each do |syms, alts|
                syms.each{|s| c << "case #{sym_name(s)}:" }

                l << "#{Node.field_name(@@AltMembName)} = #{alts.first};"
                l << "break;"
              end
            end
            l.trim << "}"
          end unless items.empty?

          items = syms.select{|k, v| !k || k.length <= 1 }
          emit_ctor.([
            "#{@@AltEnumName} #{@@AltMembName}",
          ], [
            "#{Node.field_name(@@AltMembName)}(#{@@AltMembName})",
          ]) do
            l << "switch(#{Node.field_name(@@AltMembName)}) {"
            l.group
            c = l.curr
            l.fmt with_indent: "  " do
              items.each do |syms, alts|
                alts.each{|s| c << "case #{s}:" }

                l << "#{Node.field_name(@@SymMembName)} = #{sym_name(syms.first)};"
                l << "break;"
              end

            end
            l.trim << "}"
          end unless items.empty?
        end
      end

      l.sep

      l << "#{qual_name("~#{class_name}")}() {"
      l.group
      l.fmt with_indent: "  " do
        if use_alts?
          l << "switch (#{Node.field_name(@@AltMembName)}) {"
          c = l.curr
          l.fmt with_indent: "  " do
            @ctor_alts.each do |sig, alts|
              alts.each do |alt|
                c << "case #{enum_name(alt)}:"
              end

              sig.each do |arg|
                l << "delete #{Node.field_name(arg)};"
              end

              l << "break;"
            end
          end
          l.trim << "}"
        else
          # This handles both cases of @ctors.length = 0..1
          @ctors.each do |sig, _|
            sig.each do |arg|
              l << "delete #{Node.field_name(arg)};"
            end
          end
        end
      end
      l.trim << "}"

      l.sep

      l << "void #{qual_name("print")}(std::ostream &"
      l.peek << "os" if printable?
      l.peek << ") const {"

      if printable?
        l.fmt with_indent: "  " do
          emit_format = lambda do |format|
            format.each do |spec|
              case spec
                when String
                  l << "os << #{spec.inspect};" unless spec.length == 0
                when Symbol
                  l << "#{Node.field_name(spec)}->print(os);"
              end
            end
          end

          if use_alts?
            l << "switch (#{Node.field_name(@@AltMembName)}) {"
            c = l.curr
            l.fmt with_indent: "  " do
              @format.each do |formats, alts|
                alts.each do |alt|
                  c << "case #{enum_name(alt)}:"
                end

                if formats.length > 1
                  l << "switch (#{Node.field_name(@@SymMembName)}) {"
                  c = l.curr
                  l.fmt with_indent: "  " do
                    formats.each do |sym, fmt|
                      c << "case #{sym_name(sym)}:"

                      emit_format.(fmt)
                    end

                    c << "default: break;" if formats.length < format_syms.length
                  end
                  l.trim << "}"
                else
                  formats.each_value{|f| emit_format.(f) }
                end
                l << "break;"
              end
            end
            l.trim << "}"
          else
            @format.each_key do |formats|
              formats.each_value{|f| emit_format.(f) }
            end
          end
        end
        l.trim << "}"
      else
        l.peek << "}"
      end

      @members.each do |name, type|
        l.sep << "const #{Node.just_class_name(type)} *#{qual_name(name)}() const {"
        l.fmt with_indent: "  " do
          if use_alts? && use_default_value?(name)
            if @member_alts[name].length > 1
              l << "switch (#{Node.field_name(@@AltMembName)}) {"
              c = l.curr
              l.fmt with_indent: "  " do
                @member_alts[name].each do |alt|
                  c << "case #{enum_name(alt)}:"
                end

                l.peek << " return #{Node.field_name(name)};"

                c << "default: return nullptr;"
              end
              l.trim << "}"
            else
              l << "if (#{Node.field_name(@@AltMembName)} == #{@member_alts[name].first})"
              l.fmt with_indent: "  " do
                l << "return #{Node.field_name(name)};"
              end
              l << "else return nullptr;"
            end
          else
            l << "return #{Node.field_name(name)};"
          end
        end
        l.trim << "}"
      end

      if use_alts?
        l.sep << "#{qual_name(@@AltEnumName)} #{qual_name(@@AltMembName)}() const {"

        l.fmt with_indent: "  " do
          l << "return #{Node.field_name(@@AltMembName)};"
        end
        l.trim << "}"
      end

      if use_syms?
        l.sep << "#{qual_name(@@SymEnumName)} #{qual_name(@@SymMembName)}() const {"

        l.fmt with_indent: "  " do
          l << "return #{Node.field_name(@@SymMembName)};"
        end
        l.trim << "}"
      end
    end

    def emit_bison_part(part, l)
      case part
        when :union
          l << "#{qual_class_name} *#{bison_name};"
        when :dtor
          l << "%destructor { #{@dtor ? @dtor : "if (!$$->rooted()) delete $$;"} } <#{bison_name}>"
      end
    end

    def diag_no_inspect; [:@d] end
  end
end