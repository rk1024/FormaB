module ASTGen
  class Token
    attr_writer :action, :capt

    def initialize(name, str: nil, pat: nil, capt: nil, action: nil)
      @d = ASTGen.diag
      @name = name
      @d.push("token #{@name.inspect}")
      @str = str if str
      @pat = pat if pat
      @capt = capt if capt
      @action = action if action
    end

    def froz_validate
      unless @str.nil? || @pat.nil?
        @d.error("too many selectors defined")
      end

      unless @capt.nil? || [:text, :buf, :buf_end].include?(@capt)
        @d.error("invalid capture type #{@capt.inspect}")
      end

      if @capt && @pat.nil?
        @d.error("capture specified for non-pattern token")
      end
    end

    def freeze
      super

      froz_validate
    end

    def emit_flex_head(l)
      raise "Cannot emit non-frozen token!" unless frozen?

      if @str
        l << "#{@name} #{@str.inspect}"
      end
    end

    def emit_flex_body(l)
      raise "Cannot emit non-frozen token!" unless frozen?

      if @str
        l << "{#{@name}}"
      end

      if @pat
        l << @pat
      end

      l.peek << " {"

      l.peek << " #{@action}" if @action

      if @capt
        l.peek << " yylval->_tok = new frma::FormaToken(" << case @capt
          when :text; "yytext"
          when :buf, :buf_end; "yyg->yyextra_r->buf().str()"
        end << ");"

        if @capt == :buf_end
          l.peek << " yyg->yyextra_r->bufEnd();"
        end
      end

      l.peek << " return tk::#{@name};"

      l.peek << " }"
    end

    def emit_bison_def(l)
      l << "%token"

      l.peek << " <_tok>" if @capt

      l.peek << " #{@name}"

      l.peek << " #{@str.inspect}" if @str

    end
  end
end