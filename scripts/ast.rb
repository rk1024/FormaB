require_relative 'astgen/astgen'

ASTGen.run do
  node :Primaries do
    let :Primaries, :prims
    let :Primary, :prim

    ctor :Empty, fmt: []
    ctor :Primaries, :prims, :prim, fmt: [:prims, " ", :prim]
    ctor :Primary, :prim, fmt: :prim

    dtor "if (!($$->rooted() || tag->prims == $$)) delete $$;"

    symbol do
      rule :Primaries, :prims, :prim
      rule :Primary, :prim
    end

    symbol :PrimariesOpt do
      rule :Empty
      rule :self, [:Primaries, :self]
    end
  end

  node :Primary do
    union do
      let :Token, :token
      let :Group, :group
      let :RawBlock, :rawblk
      let :MetaBlock, :metablk
    end

    toks = [
      :Identifier,
      :PPDirective,
      :Number,
      :Operator,
      :SQLiteral,
      :DQLiteral,
    ]

    ctor :Group, :group, fmt: :group
    ctor :RawBlock, :rawblk, fmt: :rawblk
    ctor :MetaBlock, :metablk, fmt: :metablk
    ctor toks, :token, fmt: :token

    symbol do
      toks.each{|t| rule t, [t, :token] }
      rule :Group, :group
      rule :RawBlock, :rawblk
      rule :MetaBlock, :metablk
    end
  end

  node :RawBlock do
    let :Token, :id
    let :Token, :body

    ctor :RawBlock, :id, :body, fmt: ["@!", :id, "{", :body, "@}"]

    symbol do
      rule :RawBlock, [:RawBlockID, :id], [:RawBlockBody, :body]
    end
  end

  node :MetaBlock do
    let :MetaSemiExpression, :expr

    ctor :MetaBlock, :expr, fmt: ["@!meta{\n", :expr, "\n@}"]

    symbol do
      rule :MetaBlock, [:MetaBlockStart], [:MetaSemiExpressionOpt, :expr], [:MetaBlockEnd]
    end
  end

  node :Group do
    let :Primaries, :prims

    ctor [:PGroup, :KGroup, :CGroup], :prims

    fmt :PGroup, "(", :prims, ")"
    fmt :KGroup, "{", :prims, "}"
    fmt :CGroup, "[", :prims, "]"

    symbol do
      rule :PGroup, "(", :prims, ")"
      rule :KGroup, "{", :prims, "}"
      rule :CGroup, "[", :prims, "]"
    end
  end

  try_parts = [:Try, :NonTry].freeze
  open_parts = [:Open, :Closed].freeze

  do_exprs = lambda do |&block|
    try_parts.product(open_parts).each do |parts|
      block.call(:"#{parts.join}", *parts)
    end
  end

  node :MetaCommaExpression do
    let :MetaCommaExpression, :comma
    let :MetaExpression, :expr

    ctor :Empty, fmt: []
    ctor :Exprs, :comma, :expr, fmt: [:comma, ", ", :expr]
    ctor :Expr, :expr, fmt: :expr

    symbol :MetaCommaExpressionOpt do
      rule :Empty, [:CommaOpt]
      rule :self, [:MetaCommaExpression, :self]
    end

    symbol do
      rule :self, [:MetaCommaExpressionPart, :self], [:CommaOpt]
    end

    symbol :MetaCommaExpressionPart do
      rule :Exprs, me(:comma), ",", :expr
      rule :Expr, :expr
    end
  end

  node :MetaSemiExpression do
    let :MetaSemiExpression, :semi
    let :MetaSemiExpressionPart, :expr

    ctor :Empty, fmt: []
    ctor :Expressions, :semi, :expr, fmt: [:semi, "\n", :expr]
    ctor :Expression, :expr, fmt: :expr

    symbol :MetaSemiExpressionOpt do
      rule :Empty
      rule :self, [:MetaSemiExpression, :self]
    end

    symbol do
      rule :Expressions, me(:semi), [:MetaSemiExpressionOptPart, :expr]
      rule :Expression, [:MetaSemiExpressionOptPart, :expr]
    end
  end

  node :MetaSemiExpressionPart do
    let :MetaExpression, :expr

    ctor :Empty, fmt: [";"]
    ctor [:Semi, :NonSemi], :expr

    fmt :Semi, :expr, ";"
    fmt :NonSemi, :expr

    symbol :MetaSemiExpressionOptPart do
      rule :Empty, ";"
      rule :self, [:MetaSemiExpressionPart, :self]
    end

    symbol do
      do_exprs.call{|p, *| rule :self, [:"Meta#{p}SemiExpressionPart", :self] }
    end



    try_parts.each do |try|
      symbol :"Meta#{try}SemiExpressionOptPart" do
        open_parts.each{|o| rule :self, [:"Meta#{try}#{o}SemiExpression#{:Opt if try == :NonTry && o == :Closed}Part", :self] }
      end
    end

    do_exprs.call do |part, try, open|
      symbol :"Meta#{part}SemiExpressionOptPart" do
        rule :Empty, ";"
        rule :self, [:"Meta#{part}SemiExpressionPart", :self]
      end

      symbol :"Meta#{part}SemiExpressionPart" do
        rule :Semi, [:"Meta#{part}SemiExpressionUnit", :expr], ";" unless try == :Try || open == :Open
        rule :NonSemi, [:"Meta#{part}NonSemiExpressionUnit", :expr]
      end
    end
  end

  node :MetaExpression do
    union do
      let :MetaStructExpression, :strukt
      let :MetaFunctionExpression, :func
      let :MetaCondExpression, :cond
      let :MetaLoopExpression, :loop
      let :MetaTryExpression, :tri
      let :MetaBlockExpression, :block
      let :MetaLetExpression, :let
      let :MetaAssignExpression, :assign
      let :MetaKeywordExpression, :keyword
      let :MetaInfixExpression, :infix
    end

    ctor :Struct, :strukt, fmt: :strukt
    ctor :Func, :func, fmt: :func
    ctor :Cond, :cond, fmt: :cond
    ctor :Loop, :loop, fmt: :loop
    ctor :Try, :tri, fmt: :tri
    ctor :Block, :block, fmt: :block
    ctor :Let, :let, fmt: :let
    ctor :Assign, :assign, fmt: :assign
    ctor :Keyword, :keyword, fmt: :keyword
    ctor :Infix, :infix, fmt: :infix

    do_exprs.call do |part, try, open|
      symbol :"Meta#{part}SemiExpressionUnit" do
        case open
          when :Closed
            if try == :NonTry
              rule :Func, [:MetaRecordExpression, :func]
              rule :Loop, [:MetaDoWhileExpression, :loop]
              rule :Assign, :assign
              rule :Infix, :infix
            end
        end
      end unless try == :Try || open == :Open

      symbol :"Meta#{part}NonSemiExpressionUnit" do
        rule :Func, [:"Meta#{part}ArrowFuncExpression", :func]
        rule :Cond, [:"Meta#{part}IfElseExpression", :cond]
        rule :Cond, [:"Meta#{part}UnlessElseExpression", :cond]
        rule :Loop, [:"Meta#{part}LoopExpression", :loop]
        rule :Loop, [:"Meta#{part}WhileExpression", :loop]
        rule :Loop, [:"Meta#{part}ForExpression", :loop]
        rule :Try, [:"Meta#{open}TryExpression", :tri] unless try == :NonTry
        rule :Let, [:"Meta#{part}LetExpression", :let]
        rule :Keyword, [:"Meta#{part}KeywordExpression", :keyword]

        case open
          when :Open
            unless try == :Try # Not if try == :NonTry, in keeping with the above
              rule :Cond, [:"Meta#{part}IfExpression", :cond]
              rule :Cond, [:"Meta#{part}UnlessExpression", :cond]
            end
          when :Closed
            if try == :NonTry
              rule :Struct, [:MetaStructExpression, :strukt]
              rule :Func, [:MetaBlockFuncExpression, :func]
              rule :Block, :block
            end
        end
      end

      symbol :"Meta#{part}Expression" do
        rule :self, [:"Meta#{part}SemiExpressionUnit", :self] unless try == :Try || open == :Open
        rule :self, [:"Meta#{part}NonSemiExpressionUnit", :self]
      end
    end

    symbol :MetaExpression do
      do_exprs.call{|p, *| rule :self, [:"Meta#{p}Expression", :self] }
    end
  end

  node :MetaStructExpression do
    let :MetaStructParts, :parts

    ctor :Struct, :parts, fmt: ["struct {\n", :parts, "\n}"]

    symbol do
      rule :Struct, "struct", "{", [:MetaStructPartsOpt, :parts], "}"
    end
  end

  node :MetaStructParts do
    let :MetaStructParts, :parts
    let :MetaStructPart, :part

    ctor :Empty, fmt: []
    ctor :Parts, :parts, :part, fmt: [:parts, "\n", :part]
    ctor :Part, :part, fmt: :part

    symbol :MetaStructPartsOpt do
      rule :Empty
      rule :self, [:MetaStructParts, :self]
    end

    symbol do
      rule :Parts, :parts, :part
      rule :Part, :part
    end
  end

  node :MetaStructPart do
    let :MetaStructMember, :memb

    # ctor [:Semi, :NonSemi], :memb
    ctor :Semi, :memb

    fmt :Semi, :memb, ";"
    # fmt :NonSemi, :memb

    symbol do
      rule :Semi, [:MetaStructSemiMember, :memb], ";"
      # rule :NonSemi, [:MetaStructNonSemiMember, :memb], ";"
    end
  end

  node :MetaStructMember do
    let :Token, :typen
    let :Token, :name

    ctor :Member, :typen, :name, fmt: [:typen, " ", :name]

    symbol :MetaStructSemiMember do
      rule :Member, "let", [:Identifier, :typen], [:Identifier, :name]
    end
  end

  node :MetaFunctionExpression do
    union do
      let :MetaSemiExpressionPart, :semi
      let :MetaBlockExpression, :block
    end

    let :MetaFunctionArguments, :args

    ctor :ArrowFunc, :args, :semi, fmt: ["function (", :args, ") => ", :semi]
    ctor :BlockFunc, :args, :block, fmt: ["function (", :args, ") ", :block]
    ctor :Record, :args, fmt: ["record (", :args, ")"]

    do_exprs.call do |part, *|
      symbol :"Meta#{part}ArrowFuncExpression" do
        rule :ArrowFunc, "function", "(", [:MetaFunctionArgumentsOpt, :args], ")", "=>", [:"Meta#{part}SemiExpressionPart", :semi]
      end
    end

    symbol :MetaBlockFuncExpression do
      rule :BlockFunc, "function", "(", [:MetaFunctionArgumentsOpt, :args], ")", :block
    end

    symbol :MetaRecordExpression do
      rule :Record, "record", "(", [:MetaFunctionArgumentsOpt, :args], ")"
    end
  end

  node :MetaFunctionArguments do
    let :MetaFunctionArguments, :args
    let :MetaFunctionArgument, :arg

    ctor :Empty, fmt: []
    ctor :Arguments, :args, :arg, fmt: [:args, ", ", :arg]
    ctor :Argument, :arg, fmt: :arg

    symbol :MetaFunctionArgumentsOpt do
      rule :Empty
      rule :self, [:MetaFunctionArguments, :self]
    end

    symbol do
      rule :Arguments, :args, ",", :arg
      rule :Argument, :arg
    end
  end

  node :MetaFunctionArgument do
    let :Token, :typen
    let :Token, :id

    ctor :Typed, :typen, :id, fmt: [:typen, " ", :id]

    symbol do
      rule :Typed, [:Identifier, :typen], [:Identifier, :id]
    end
  end

  node :MetaCondExpression do
    let :MetaCondition, :cond
    let :MetaSemiExpressionPart, :then
    let :MetaSemiExpressionPart, :otherwise

    ctor [:Unless, :If], :cond, :then, fmt: ["if (", :cond, ") ", :then]
    ctor [:UnlessElse, :IfElse], :cond, :then, :otherwise, fmt: ["if (", :cond, ") ", :then, "\nelse ", :otherwise]

    [
      [:If, "if"],
      [:Unless, "unless"],
    ].each do |name, tok|
      do_exprs.call do |part, try, open|
        symbol :"Meta#{part}#{name}Expression" do
          rule name, tok, "(", :cond, ")", [:"Meta#{try}SemiExpression#{:Opt if try == :NonTry}Part", :then]
        end if try != :Try && open == :Open

        symbol :"Meta#{part}#{name}ElseExpression" do
          rule :"#{name}Else", tok, "(", :cond, ")", [:"Meta#{try}ClosedSemiExpression#{:Opt if try == :NonTry}Part", :then], "else", [:"Meta#{part}SemiExpression#{:Opt if try == :NonTry && open == :Closed}Part", :otherwise]
        end
      end
    end
  end

  node :MetaLoopExpression do
    union do
      let :MetaCondition, :cond
      let :MetaForPreamble, :pream
    end

    let :MetaSemiExpressionPart, :body

    ctor :Loop, :body, fmt: ["loop ", :body]
    ctor [:While, :DoWhile], :cond, :body
    ctor :For, :pream, :body, fmt: ["for (", :pream, ") ", :body]

    fmt :While, "while (", :cond, ") ", :body
    fmt :DoWhile, "do ", :body, " while (", :cond, ")"

    do_exprs.call do |part, try, open|
      body = [:"Meta#{part}SemiExpression#{:Opt if try == :NonTry && open == :Closed}Part", :body].freeze

      symbol :"Meta#{part}LoopExpression" do
        rule :Loop, "loop", body
      end

      symbol :"Meta#{part}WhileExpression" do
        rule :While, "while", "(", :cond, ")", body
      end

      symbol :"Meta#{part}ForExpression" do
        rule :For, "for", "(", :pream, ")", body
      end
    end

    symbol :MetaDoWhileExpression do
      rule :DoWhile, "do", [:MetaSemiExpressionOptPart, :body], "while", "(", :cond, ")"
    end
  end

  node :MetaTryExpression do
    let :MetaSemiExpressionPart, :tri
    let :MetaCatchExpression, :katch
    let :MetaFinallyExpression, :finally

    ctor :TryCatch, :tri, :katch, fmt: ["try ", :tri, " ", :katch]
    ctor :TryFinally, :tri, :finally, fmt: ["try ", :tri, " ", :finally]
    ctor :TryCatchFinally, :tri, :katch, :finally, fmt: ["try ", :tri, " ", :katch, " ", :finally]

    open_parts.each do |part|
      tri = [ "try", [:MetaNonTrySemiExpressionOptPart, :tri]]
      katch = [:"Meta#{part}CatchExpression", :katch]
      finally = [:"Meta#{part}FinallyExpression", :finally]

      symbol :"Meta#{part}TryExpression" do
        case part
          when :Open
            rule :TryCatch, *tri, katch
            rule :TryFinally, *tri, finally
          when :Closed
            rule :TryCatchFinally, *tri, katch, finally
        end
      end
    end
  end

  node :MetaCatchExpression do
    let :MetaCatchExpression, :katch
    let :MetaSemiExpressionPart, :body

    ctor :Catch, :body, fmt: ["catch ", :body]

    open_parts.each do |part|
      symbol :"Meta#{part}CatchExpression" do
        rule :Catch, "catch", [:"MetaNonTry#{part}SemiExpressionOptPart", :body]
      end
    end
  end

  node :MetaFinallyExpression do
    let :MetaSemiExpressionPart, :body

    ctor :Finally, :body, fmt: ["finally ", :body]

    open_parts.each do |part|
      symbol :"Meta#{part}FinallyExpression" do
        rule :Finally, "finally", [:"MetaNonTry#{part}SemiExpressionOptPart", :body]
      end
    end
  end

  node :MetaCondition do
    let :MetaLetExpression, :let
    let :MetaExpression, :expr

    ctor :Expr, :expr, fmt: :expr
    ctor :Let, :let, :expr, fmt: [:let, " ", :expr]

    symbol do
      rule :Expr, :expr
      rule :Let, :let, :expr
    end
  end

  node :MetaForPreamble do
    let :MetaLetExpression, :let
    let :MetaSemiExpressionPart, :semi
    let :MetaExpression, :expr

    ctor :Preamble, :let, :semi, :expr, fmt: [:let, " ", :semi, " ", :expr]

    symbol do
      rule :Preamble, :let, :semi, :expr
    end
  end

  node :MetaBlockExpression do
    let :MetaSemiExpression, :expr

    ctor :Block, :expr, fmt: ["{\n", :expr, "\n}"]

    symbol do
      rule :Block, "{", [:MetaSemiExpressionOpt, :expr], "}"
    end
  end

  node :MetaLetExpression do
    let :Token, :id
    let :MetaSemiExpressionPart, :expr

    ctor :Let, :id, :expr, fmt: ["let ", :id, " = ", :expr]

    symbol :MetaLetExpression do
      rule :Let, "let", [:Identifier, :id], "=", :expr
    end

    do_exprs.call do |part, *|
      symbol :"Meta#{part}LetExpression" do
        rule :Let, "let", [:Identifier, :id], "=", [:"Meta#{part}SemiExpressionPart", :expr]
      end
    end
  end

  node :MetaAssignExpression do
    union do
      let :MetaMemberExpression, :memb
      let :MetaInfixExpression, :infix
    end

    let :MetaAssignExpression, :assign

    ctor [
      :Assign,
      :LogOr,
      :LogAnd,
      :BitOr,
      :BitAnd,
      :BitXor,
      :Add,
      :Sub,
      :Mul,
      :Div,
      :Mod,
    ], :memb, :assign
    ctor :Infix, :infix, fmt: :infix

    fmt :Assign, :memb, " = ", :assign
    fmt :LogOr, :memb, " ||= ", :assign
    fmt :LogAnd, :memb, " &&= ", :assign
    fmt :BitOr, :memb, " |= ", :assign
    fmt :BitAnd, :memb, " &= ", :assign
    fmt :BitXor, :memb, " ^= ", :assign
    fmt :Add, :memb, " += ", :assign
    fmt :Sub, :memb, " -= ", :assign
    fmt :Mul, :memb, " *= ", :assign
    fmt :Div, :memb, " /= ", :assign
    fmt :Mod, :memb, " %= ", :assign

    symbol do
      rule :Assign, :memb, "=", defer(:assign)
      rule :LogOr, :memb, "||=", defer(:assign)
      rule :LogAnd, :memb, "&&=", defer(:assign)
      rule :BitOr, :memb, "|=", defer(:assign)
      rule :BitAnd, :memb, "&=", defer(:assign)
      rule :BitXor, :memb, "^=", defer(:assign)
      rule :Add, :memb, "+=", defer(:assign)
      rule :Sub, :memb, "-=", defer(:assign)
      rule :Mul, :memb, "*=", defer(:assign)
      rule :Div, :memb, "/=", defer(:assign)
      rule :Mod, :memb, "%=", defer(:assign)
    end

    chain :MetaAssignRHS, with_rule: false do
      rule :Infix, :infix
      rule :self, [:MetaAssignExpression, :self]
    end
  end

  node :MetaKeywordExpression do
    let :MetaSemiExpressionPart, :expr

    ctor [
      :Break,
      :Next,
      :Return,
      :Yield,
    ], :expr

    fmt :Break, "break", :expr
    fmt :Next, "next", :expr
    fmt :Return, "return", :expr
    fmt :Yield, "yield", :expr

    do_exprs.call do |part, try, open|
      symbol :"Meta#{part}KeywordExpression" do
        semi = [:"Meta#{part}SemiExpression#{:Opt if try == :NonTry && open == :Closed}Part", :expr]
        rule :Break, "break", semi
        rule :Next, "next", semi
        rule :Return, "return", semi
        rule :Yield, "yield", semi
      end
    end
  end

  node :MetaInfixExpression do
    union do
      let :MetaInfixExpression, :infixr
      let :MetaUnaryExpression, :unary
    end

    let :MetaInfixExpression, :infixl

    ctor [
      :Disjunct,
      :Conjunct,
      :Equal,
      :NotEqual,
      :Greater,
      :Less,
      :GreaterEq,
      :LessEq,
      :BitOr,
      :BitAnd,
      :BitXor,
      :Add,
      :Sub,
      :Mul,
      :Div,
    ], :infixl, :infixr
    ctor :Mod, :infixl, :unary
    ctor :Unary, :unary, fmt: :unary

    fmt :Conjunct, :infixl, " || ", :infixr
    fmt :Disjunct, :infixl, " && ", :infixr
    fmt :Equal, :infixl, " == ", :infixr
    fmt :NotEqual, :infixl, " != ", :infixr
    fmt :Greater, :infixl, " > ", :infixr
    fmt :Less, :infixl, " < ", :infixr
    fmt :GreaterEq, :infixl, " >= ", :infixr
    fmt :LessEq, :infixl, " <= ", :infixr
    fmt :BitOr, :infixl, " | ", :infixr
    fmt :BitAnd, :infixl, " & ", :infixr
    fmt :BitXor, :infixl, " ^ ", :infixr
    fmt :Add, :infixl, " + ", :infixr
    fmt :Sub, :infixl, " - ", :infixr
    fmt :Mul, :infixl, " * ", :infixr
    fmt :Div, :infixl, " / ", :infixr
    fmt :Mod, :infixl, " % ", :unary

    symbol do end

    chain :MetaDisjunctExpression do
      rule :Disjunct, me(:infixl), "||", defer(:infixr)
    end

    chain :MetaConjunctExpression do
      rule :Conjunct, me(:infixl), "&&", defer(:infixr)
    end

    chain :MetaComparisonExpression do
      [
        [:Equal, "=="],
        [:NotEqual, "!="],
        [:Greater, ">"],
        [:Less, "<"],
        [:GreaterEq, ">="],
        [:LessEq, "<="],
      ].each do |name, op|
        rule name, me(:infixl), op, defer(:infixr)
      end
    end

    chain :MetaBitwiseOrExpression do
      rule :BitOr, me(:infixl), "|", defer(:infixr)
    end

    chain :MetaBitwiseAndExpression do
      rule :BitAnd, me(:infixl), "&", defer(:infixr)
    end

    chain :MetaBitwiseXorExpression do
      rule :BitXor, me(:infixl), "^", defer(:infixr)
    end

    chain :MetaAddExpression do
      rule :Add, me(:infixl), "+", defer(:infixr)
    end

    chain :MetaSubExpression do
      rule :Sub, me(:infixl), "-", defer(:infixr)
    end

    chain :MetaMulExpression do
      rule :Mul, me(:infixl), "*", defer(:infixr)
    end

    chain :MetaDivExpression do
      rule :Div, me(:infixl), "/", defer(:infixr)
    end

    chain :MetaModExpression do
      rule :Mod, me(:infixl), "%", :unary
      rule :Unary, :unary
    end
  end

  node :MetaUnaryExpression do
    union do
      let :MetaUnaryExpression, :unary
      let :MetaCallExpression, :call
    end

    ctor [
      :LogNot,
      :BitNot,
      :Pos,
      :Neg,
      :Inc,
      :Dec,
    ], :unary
    ctor :Call, :call, fmt: :call

    fmt :LogNot, "!", :unary
    fmt :BitNot, "~", :unary
    fmt :Pos, "+", :unary
    fmt :Neg, "-", :unary
    fmt :Inc, "++", :unary
    fmt :Dec, "--", :unary

    symbol do
      rule :LogNot, "!", :unary
      rule :BitNot, "~", :unary
      rule :Pos, "+", :unary
      rule :Neg, "-", :unary
      rule :Inc, "++", :unary
      rule :Dec, "--", :unary
      rule :Call, :call
    end
  end

  node :MetaCallExpression do
    union do
      let :MetaCallExpression, :call
      let :MetaMemberExpression, :memb
    end

    let :MetaCommaExpression, :args

    ctor :Call, :call, :args, fmt: [:call, "(", :args, ")"]
    ctor :Member, :memb, fmt: :memb

    symbol do
      rule :Call, :call, "(", [:MetaCommaExpressionOpt, :args], ")"
      rule :Member, :memb
    end
  end

  node :MetaMemberExpression do
    union do
      let :MetaCallExpression, :expr
      let :MetaPrimary, :prim
    end

    union do
      let :Token, :memb
      let :MetaCommaExpression, :index
    end

    ctor :Member, :expr, :memb, fmt: [:expr, ".", :memb]
    ctor :Index, :expr, :index, fmt: [:expr, "[", :index, "]"]
    ctor :Primary, :prim, fmt: :prim

    symbol do
      rule :Member, :expr, ".", [:Identifier, :memb]
      rule :Index, :expr, "[", :index, "]"
      rule :Primary, :prim
    end
  end

  node :MetaPrimary do
    union do
      let :MetaLiteral, :literal
      let :Token, :id
      let :MetaExpression, :expr
    end

    ctor :Literal, :literal, fmt: :literal
    ctor :Identifier, :id, fmt: :id
    ctor :Parens, :expr, fmt: ["(", :expr, ")"]

    symbol do
      rule :Literal, :literal
      rule :Identifier, [:Identifier, :id]
      rule :Parens, "(", :expr, ")"
    end
  end

  node :MetaLiteral do
    let :Token, :tok

    toks = [
      :Number,
      :SQLiteral,
      :DQLiteral,
    ]

    ctor toks, :tok, fmt: :tok
    ctor [:Nil, :True, :False]

    fmt :Nil, "nil"
    fmt :True, "true"
    fmt :False, "false"

    symbol do
      toks.each{|t| rule t, [t, :tok] }
      rule :Nil, "nil"
      rule :True, "true"
      rule :False, "false"
    end
  end
end