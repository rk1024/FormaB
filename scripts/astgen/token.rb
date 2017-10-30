##########################################################################
#
# FormaB - the bootstrap Forma compiler (token.rb)
# Copyright (C) 2017-2017 Ryan Schroeder, Colin Unger
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

require_relative 'loosehash'

require 'strscan'

module ASTGen
  class Token
    def self.scan(file, d)
      tokens = LooseHash.new

      File.foreach(file).each_with_index do |line, index|
        d.pos("#{file}:#{index + 1}") do
          case line
            when /^\s*%astgen-token\s+(\S+)\s*(.*)\s*$/
              s = StringScanner.new($2)
              tok = Token.new($1.to_sym, d)

              err = catch :err do
                if s.scan(/"/)
                  tok.string = s.scan(/([^\\"]|\\.)*/)

                  throw :err, "'\"'" unless s.scan(/"/)
                elsif s.scan(/\//)
                  tok.pattern = s.scan(/[^\/]*/)

                  throw :err, "'/'" unless s.scan(/\//)

                  s.scan(/\s+/)

                  if s.scan(/"/)
                    tok.string = s.scan(/([^\\"]|\\.)*/).gsub(/\\(.)/, "\\1")

                    throw :err, "'\"'" unless s.scan(/"/)
                  end
                else
                  throw :err, "'\"' or '/'"
                end

                s.scan(/\s+/)

                if s.scan(/\{/)
                  braces = 1
                  action = ""

                  while braces > 0
                    part = s.scan_until(/[{}]/)
                    throw :err unless part

                    action << part[0..-1 - s.matched_size]

                    case s.matched
                      when "{"; braces += 1
                      when "}"; braces -= 1
                    end

                    action << s.match if braces > 0
                  end

                  tok.action = action.strip! if action
                  s.scan(/\s+/)
                end

                while s.scan(/\w+/)
                  case s.matched
                    when "capture"
                      if tok.capture || !tok.pattern
                        d.error("bad capture flag")
                        throw :err, nil
                      end
                      tok.capture = :text

                    when "buf"
                      unless tok.capture == :text
                        d.error("bad buffer-capture flag")
                        throw :err, nil
                      end
                      tok.capture = :buf

                    when "end"
                      unless tok.capture == :buf
                        d.error("bad end-buffer flag")
                        throw :err, nil
                      end
                      tok.capture = :buf_end
                  end

                  s.scan(/\s+/)
                end

                tokens[tok.name] = tok

                nil
              end

              d.error("unexpected #{s.eos ? "end of line" : s.rest.inspect}, expecting #{err}") if err

            when /^\s*%astgen-token\b(?!-)/
              d.error("invalid syntax for %astgen-token")
          end
        end
      end

      tokens = tokens.flatten do |name, toks|
        d.error("duplicate token name #{d.hl(name)}")
      end

      tokens
    end

    attr_accessor :name, :string, :pattern, :action, :capture

    def initialize(name, d)
      @d = d.fork
      @name = name
      @string = nil
      @pattern = nil
      @action = nil
      @capture = nil
    end

    def self.class_name; Node.class_name(:Token) end
    def self.just_class_name; Node.just_class_name(:Token) end
    def self.qual_class_name; Node.qual_class_name(:Token) end
    def self.bison_name; Node.bison_name(:Token) end

    def emit_flex_part(part, l)
      case part
        when :def
          l << "#{@name} #{@string.inspect}" if @string && !@pattern

        when :rule
          if @pattern
            l << @pattern
          elsif @string
            l << "{#{@name}}"
          end

          l.peek << " {"

          l.peek << " #{@action}" if @action

          if @capture
            l.peek << " yylval->#{Token.bison_name} = new #{Token.qual_class_name}(" << case @capture
              when :text; "yytext"
              when :buf, :buf_end; "yyg->yyextra_r->buf().str()"
            end << ", *yylloc);"

            if @capture == :buf_end
              l.peek << " yyg->yyextra_r->bufEnd();"
            end
          end

          l.peek << " return tk::#{@name}; }"
      end
    end

    def self.emit_bison_part(part, l)
      case part
        when :union
          l << "#{Token.qual_class_name} *#{Token.bison_name};"
        when :dtor
          l << "%destructor { if (!$$->rooted()) delete $$; } <#{Token.bison_name}>"
      end
    end

    def emit_bison_part(part, l)
      case part
        when :def
          l << "%token"
          l.peek << " <#{Token.bison_name}>" if @capture
          l.peek << " #{@name}"

          if @string
            l.peek << " #{if @pattern
              "#{@name} [BISON SUCKS] (#{@string})"
            else
              @string
            end.inspect}"
          end
      end
    end

    def diag_no_inspect
      ret = [:@d]
      ret << :@string unless @string
      ret << :@pattern unless @pattern
      ret << :@capture unless @capture
      ret
    end
  end
end