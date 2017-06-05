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
      rule :Expressions, me(:semi), :expr
      rule :Expression, :expr
    end
  end

  node :MetaSemiExpressionPart do
    let :MetaExpression, :expr

    ctor [:Semi, :NonSemi], :expr

    fmt :Semi, :expr, ";"
    fmt :NonSemi, :expr

    symbol do
      [:Open, :Closed].each{|p| rule :self, [:"Meta#{p}SemiExpressionPart", :self] }
    end

    [:Open, :Closed].each do |part|
      symbol :"Meta#{part}SemiExpressionPart" do
        rule :Semi, [:"Meta#{part}SemiExpressionUnit", :expr], ";" unless part == :Open
        rule :NonSemi, [:"Meta#{part}NonSemiExpressionUnit", :expr]
      end
    end
  end

  node :MetaExpression do
    union do
      let :MetaCondExpression, :cond
      let :MetaLoopExpression, :loop
      let :MetaBlockExpression, :block
      let :MetaLetExpression, :let
      let :MetaInfixExpression, :infix
    end

    ctor :Cond, :cond, fmt: :cond
    ctor :Loop, :loop, fmt: :loop
    ctor :Block, :block, fmt: :block
    ctor :Let, :let, fmt: :let
    ctor :Infix, :infix, fmt: :infix

    [:Open, :Closed].each do |part|
      symbol :"Meta#{part}SemiExpressionUnit" do
        case part
          when :Closed
            rule :Loop, [:MetaClosedDoWhileExpression, :loop]
            rule :Infix, :infix
        end
      end unless part == :Open

      symbol :"Meta#{part}NonSemiExpressionUnit" do
        rule :Cond, [:"Meta#{part}IfElseExpression", :cond]
        rule :Loop, [:"Meta#{part}LoopExpression", :loop]
        rule :Loop, [:"Meta#{part}WhileExpression", :loop]
        rule :Loop, [:"Meta#{part}ForExpression", :loop]
        rule :Let, [:"Meta#{part}LetExpression", :let]

        case part
          when :Open
            rule :Cond, [:MetaOpenIfExpression, :cond]
          when :Closed
            rule :Block, :block
        end
      end
    end

    symbol do
      rule :Block, :block
      rule :Let, :let
      rule :Infix, :infix
    end
  end

  node :MetaCondExpression do
    let :MetaCondition, :cond
    let :MetaSemiExpressionPart, :then
    let :MetaSemiExpressionPart, :otherwise

    ctor :If, :cond, :then, fmt: ["if (", :cond, ") ", :then]
    ctor :IfElse, :cond, :then, :otherwise, fmt: ["if (", :cond, ") ", :then, "\nelse ", :otherwise]

    symbol :MetaOpenIfExpression do
      rule :If, "if", "(", :cond, ")", :then
    end

    symbol :MetaOpenIfElseExpression do
      rule :IfElse, "if", "(", :cond, ")", [:MetaClosedSemiExpressionPart, :then], "else", [:MetaOpenSemiExpressionPart, :otherwise]
    end

    symbol :MetaClosedIfElseExpression do
      rule :IfElse, "if", "(", :cond, ")", [:MetaClosedSemiExpressionPart, :then], "else", [:MetaClosedSemiExpressionPart, :otherwise]
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

    [:Open, :Closed].each do |part|
      body = [:"Meta#{part}SemiExpressionPart", :body].freeze

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

    symbol :MetaClosedDoWhileExpression do
      rule :DoWhile, "do", [:MetaClosedSemiExpressionPart, :body], "while", "(", :cond, ")"
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

    [:Open, :Closed].each do |part|
      symbol :"Meta#{part}LetExpression" do
        rule :Let, "let", [:Identifier, :id], "=", [:"Meta#{part}SemiExpressionPart", :expr]
      end
    end
  end

  node :MetaInfixExpression do
    union do
      let :MetaInfixExpression, :infixr
      let :MetaUnaryExpression, :unary
    end

    let :MetaInfixExpression, :infixl

    ctor [:Add, :Sub, :Mul, :Div], :infixl, :infixr
    ctor :Mod, :infixl, :unary
    ctor :Unary, :unary, fmt: :unary

    fmt :Add, :infixl, " + ", :infixr
    fmt :Sub, :infixl, " - ", :infixr
    fmt :Mul, :infixl, " * ", :infixr
    fmt :Div, :infixl, " / ", :infixr
    fmt :Mod, :infixl, " % ", :unary

    symbol do end

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
      :Pos,
      :Neg,
      :Inc,
      :Dec
    ], :unary
    ctor :Call, :call, fmt: :call

    fmt :Pos, "+", :unary
    fmt :Neg, "-", :unary
    fmt :Inc, "++", :unary
    fmt :Dec, "--", :unary

    symbol do
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

    symbol do
      toks.each{|t| rule t, [t, :tok] }
    end
  end
end