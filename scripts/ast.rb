require_relative 'astgen/astgen'

ASTGen.run do
  export :PrimariesOpt

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
      let :Token, :tok
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
    ctor toks, :tok, fmt: :tok

    symbol do
      toks.each{|t| rule t, [t, :tok] }
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
    let :MetaStatements, :expr

    ctor :MetaBlock, :expr, fmt: ["@!meta{\n", :expr, "\n@}"]

    symbol do
      rule :MetaBlock, [:MetaBlockStart], [:MetaStatementsOpt, :expr], [:MetaBlockEnd]
    end
  end

  node :Group do
    let :Primaries, :prims

    ctor [:PGroup, :KGroup, :CGroup], :prims

    fmt :PGroup, "(", :prims, ")"
    fmt :KGroup, "{ ", :prims, " }"
    fmt :CGroup, "[", :prims, "]"

    symbol do
      rule :PGroup, "(", :prims, ")"
      rule :KGroup, "{", :prims, "}"
      rule :CGroup, "[", :prims, "]"
    end
  end

  open_parts = [:Open, :Closed].freeze

  expr_parts = [
    open_parts,
  ].freeze

  do_exprs = lambda do |&block|
    expr_parts.reduce([nil]) {|c, p| c.product(p) }.each do |n, *parts|
      abort unless n == nil
      block.call(:"#{parts.join}", *parts)
    end
  end

  node :MetaStatements do
    let :MetaStatements, :stmts
    let :MetaStatement, :stmt

    ctor :Empty, fmt: []
    ctor :Statements, :stmts, :stmt, fmt: [:stmts, "\n", :stmt]
    ctor :Statement, :stmt, fmt: :stmt

    symbol do
      rule :Statements, :stmts, :stmt
      rule :Statement, :stmt
    end

    symbol :MetaStatementsOpt do
      rule :Empty
      rule :self, [:MetaStatements, :self]
    end
  end

  node :MetaExpressions do
    let :MetaExpressions, :exprs
    let :MetaExpression, :expr

    ctor :Empty, fmt: []
    ctor :Exprs, :exprs, :expr, fmt: [:exprs, ", ", :expr]
    ctor :Expr, :expr, fmt: :expr

    symbol :MetaExpressionsOpt do
      rule :Empty, [:CommaOpt]
      rule :self, [:MetaExpressions, :self]
    end

    symbol do
      rule :self, [:MetaExpressionsPart, :self], [:CommaOpt]
    end

    symbol :MetaExpressionsPart do
      rule :Exprs, me(:exprs), ",", :expr
      rule :Expr, :expr
    end
  end

  node :MetaStatement do
    union do
      let :MetaExpression, :expr
      let :MetaBindStatement, :bind
      let :MetaAssignStatement, :assign
      let :MetaControlStatement, :ctl
    end

    ctor [:SemiExpr, :NonSemiExpr], :expr
    ctor :Bind, :bind, fmt: [:bind, ";"]
    ctor :Assign, :assign, fmt: [:assign, ";"]
    ctor :Control, :ctl, fmt: :ctl

    fmt :SemiExpr, :expr, ";"
    fmt :NonSemiExpr, :expr

    do_exprs.call do |parts, open|
      symbol :"Meta#{parts}PureStatement" do
        rule :SemiExpr, [:"Meta#{parts}SemiExpression", :expr], ";"
        rule :NonSemiExpr, [:"Meta#{parts}NonSemiExpression", :expr]
      end

      symbol :"Meta#{parts}Statement" do
        rule :self, [:"Meta#{parts}PureStatement", :self]
        rule :Bind, [:"Meta#{parts}BindStatement", :bind], ";"
        rule :Control, [:"Meta#{parts}ControlStatement", :ctl]

        case open
          when :Closed
            rule :Assign, :assign, ";"
        end
      end
    end

    symbol do
      do_exprs.call{|p| rule :self, [:"Meta#{p}Statement", :self] }
    end
  end

  node :MetaBindStatement do
    let :MetaBindings, :binds

    ctor [:Let, :Var], :binds

    fmt :Let, "let ", :binds
    fmt :Var, "var ", :binds

    do_exprs.call do |parts|
      symbol :"Meta#{parts}LetStatement" do
        rule :Let, "let", [:"Meta#{parts}Bindings", :binds]
        # rule :Let, "let", [:Identifier, :id], "=", [:"Meta#{parts}Expression", :expr]
      end

      symbol :"Meta#{parts}BindStatement" do
        rule :self, [:"Meta#{parts}LetStatement", :self]
        rule :Var, "var", [:"Meta#{parts}Bindings", :binds]
        # rule :Var, "var", [:Identifier, :id], "=", [:"Meta#{parts}Expression", :expr]
      end
    end

    symbol :MetaLetStatement do
      do_exprs.call{|p| rule :self, [:"Meta#{p}LetStatement", :self] }
    end

    symbol do
      do_exprs.call{|p| rule :self, [:"Meta#{p}BindStatement", :self] }
    end
  end

  node :MetaBindings do
    let :MetaBindings, :binds
    let :MetaBinding, :bind

    ctor :Bindings, :binds, :bind, fmt: [:binds, ", ", :bind]
    ctor :Binding, :bind, fmt: :bind

    do_exprs.call do |parts|
      symbol :"Meta#{parts}BindingsPart" do
        rule :Bindings, [:"Meta#{parts}BindingsPart", :binds], ",", [:"Meta#{parts}Binding", :bind]
        rule :Binding, [:"Meta#{parts}Binding", :bind]
      end

      symbol :"Meta#{parts}Bindings" do
        rule :self, [:"Meta#{parts}BindingsPart", :self], [:CommaOpt]
      end
    end
  end

  node :MetaBinding do
    let :Token, :id
    let :MetaExpression, :expr

    ctor :Binding, :id, :expr, fmt: [:id, " = ", :expr]

    do_exprs.call do |parts|
      symbol :"Meta#{parts}Binding" do
        rule :Binding, [:Identifier, :id], "=", [:"Meta#{parts}Expression", :expr]
      end
    end
  end

  node :MetaAssignStatement do
    let :MetaMemberExpression, :memb
    let :MetaAssignValue, :value

    ops = [
      [:Assign, ""],
      [:LogOr, "||"],
      [:LogAnd, "&&"],
      [:Add, "+"],
      [:Sub, "-"],
      [:Mul, "*"],
      [:Div, "/"],
      [:Mod, "%"],
    ]

    ctor ops.map{|a, _| a }, :memb, :value

    ops.each do |alt, op|
      fmt alt, :memb, " #{op}= ", :value
    end

    symbol do
      ops.each do |alt, op|
        rule alt, :memb, "#{op}=", :value
      end
    end
  end

  node :MetaAssignValue do
    union do
      let :MetaAssignStatement, :assign
      let :MetaInfixExpression, :infix
    end

    ctor :Assign, :assign, fmt: :assign
    ctor :Infix, :infix, fmt: :infix

    symbol do
      rule :Assign, :assign
      rule :Infix, :infix
    end
  end

  node :MetaExpression do
    union do
      let :MetaFunctionExpression, :func
      let :MetaInfixExpression, :infix
      let :MetaControlExpression, :ctl
    end

    ctor :Function, :func, fmt: :func
    ctor :Infix, :infix, fmt: :infix
    ctor :Control, :ctl, fmt: :ctl

    do_exprs.call do |parts, open|
      symbol :"Meta#{parts}SemiExpression" do
        case open
          when :Open
          when :Closed
            rule :Infix, :infix
        end

        rule :Function, [:"Meta#{parts}FunctionExpression", :func]
      end

      symbol :"Meta#{parts}NonSemiExpression" do end

      symbol :"Meta#{parts}PureExpression" do
        rule :self, [:"Meta#{parts}SemiExpression", :self]
        rule :self, [:"Meta#{parts}NonSemiExpression", :self]
      end

      symbol :"Meta#{parts}Expression" do
        rule :self, [:"Meta#{parts}PureExpression", :self]
        rule :Control, [:"Meta#{parts}ControlExpression", :ctl]
      end
    end

    symbol do
      do_exprs.call{|p| rule :self, [:"Meta#{p}Expression", :self] }
    end
  end

  node :MetaFunctionExpression do
    let :MetaFunctionParameters, :params
    let :MetaExpression, :expr

    ctor :Function, :params, :expr, fmt: [:params, " => ", :expr]

    do_exprs.call do |parts|
      symbol :"Meta#{parts}FunctionExpression" do
        rule :Function, :params, "=>", [:"Meta#{parts}Expression", :expr]
      end
    end
  end

  node :MetaFunctionParameters do
    let :MetaFunctionParameters, :params
    let :MetaFunctionParameter, :param

    ctor :List, :params, fmt: ["(", :params, ")"]
    ctor :Empty, fmt: []
    ctor :Parameters, :params, :param, fmt: [:params, ", ", :param]
    ctor :Parameter, :param, fmt: :param

    symbol :MetaFunctionParametersPart do
      rule :Parameters, me(:params), ",", :param
      rule :Parameter, :param
    end

    symbol :MetaFunctionParametersPartOpt do
      rule :Empty, [:CommaOpt]
      rule :self, [:MetaFunctionParametersPart, :self], [:CommaOpt]
    end

    symbol do
      rule :List, "(", [:MetaFunctionParametersPartOpt, :params], ")"
    end
  end

  node :MetaFunctionParameter do
    let :Token, :id

    ctor :Parameter, :id, fmt: :id

    symbol do
      rule :Parameter, [:AtomKeyword, :id]
    end
  end

  [
    [
      :Statement,
      true,
      [
        [:If, "if"],
        [:Unless, "unless"],
        [:While, "while"],
        [:Until, "until"],
      ],
    ],
    [
      :Expression,
      false,
      [
        [:If, "if"],
        [:Unless, "unless"],
      ]
    ],
  ].each do |expr, do_single, types|
    node :"MetaControl#{expr}" do
      let :MetaParenExpression, :cond

      let :"Meta#{expr}", :then
      let :"Meta#{expr}", :otherwise

      types.each do |name, tok|
        ctor name, :cond, :then, fmt: ["#{tok} ", :cond, " ", :then] if do_single

        ctor :"#{name}#{:Else if do_single}", :cond, :then, :otherwise, fmt: ["#{tok} ", :cond, " ", :then, " else ", :otherwise]
      end

      do_exprs.call do |parts, open|
        symbol :"Meta#{parts}Control#{expr}" do
          types.each do |name, tok|
            if open == :Open && do_single
              open_parts.each{|o| rule name, tok.freeze, :cond, [:"Meta#{parts.to_s.gsub(open.to_s, o.to_s)}#{expr}", :then] }
            end

            rule :"#{name}#{:Else if do_single}", tok.freeze, :cond, [:"Meta#{parts.to_s.gsub(open.to_s, "Closed")}#{expr}", :then], "else", [:"Meta#{parts}#{expr}", :otherwise]
          end
        end
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
    fmt :Add, :infixl, " + ", :infixr
    fmt :Sub, :infixl, " - ", :infixr
    fmt :Mul, :infixl, " * ", :infixr
    fmt :Div, :infixl, " / ", :infixr
    fmt :Mod, :infixl, " % ", :unary

    symbol do end # Does something because of chaining (i.e. DON'T REMOVE THIS)

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
      let :MetaMemberExpression, :memb
    end

    ctor [
      :LogNot,
      :Pos,
      :Neg,
      :Inc,
      :Dec,
    ], :unary
    ctor :Member, :memb, fmt: :memb

    fmt :LogNot, "!", :unary
    fmt :Pos, "+", :unary
    fmt :Neg, "-", :unary
    fmt :Inc, "++", :unary
    fmt :Dec, "--", :unary

    symbol do
      rule :LogNot, "!", :unary
      rule :Pos, "+", :unary
      rule :Neg, "-", :unary
      rule :Inc, "++", :unary
      rule :Dec, "--", :unary
      rule :Member, :memb
    end
  end

  node :MetaMemberExpression do
    union do
      let :MetaPrimaryExpression, :prim
      let :MetaMemberExpression, :base
    end
    let :Token, :memb

    ctor :Primary, :prim, fmt: :prim
    ctor :Member, :base, :memb, fmt: [:base, ".", :memb]

    symbol do
      rule :Primary, :prim
      rule :Member, :base, ".", [:Identifier, :memb]
    end
  end

  node :MetaPrimaryExpression do
    union do
      let :Token, :tok
      let :MetaBooleanExpression, :boolean
      let :MetaParenExpression, :paren
      let :MetaBlockExpression, :block
      let :MetaMessageExpression, :message
    end

    toks = [
      :Identifier,
      :Number,
      :SQLiteral,
      :DQLiteral
    ]

    ctor toks, :tok, fmt: :tok
    ctor :Boolean, :boolean, fmt: :boolean
    ctor :Parens, :paren, fmt: :paren
    ctor :Block, :block, fmt: :block
    ctor :Message, :message, fmt: :message

    symbol :MetaNonIdentifierPrimaryExpression do
      toks.select{|t| t != :Identifier }.each{|t| rule t, [t, :tok] }
      rule :Boolean, :boolean
      rule :Parens, :paren
      rule :Block, :block
      rule :Message, :message
    end

    symbol do
      rule :Identifier, [:Identifier, :tok]
      rule :self, [:MetaNonIdentifierPrimaryExpression, :self]
    end
  end

  node :MetaBooleanExpression do
    ctor :True, fmt: "true"
    ctor :False, fmt: "false"

    symbol do
      rule :True, "true"
      rule :False, "false"
    end
  end

  node :MetaParenExpression do
    union do
      let :MetaParenExpression, :paren
      let :MetaExpression, :expr
      let :MetaExpressions, :exprs
    end

    let :MetaBindStatement, :bind

    ctor :Paren, :paren, fmt: ["(", :paren, ")"]

    ctor :Where, :bind, :expr, fmt: [:bind, "; ", :expr]
    ctor :Tuple, :exprs, fmt: :exprs

    symbol :MetaParenExpressionBody do
      rule :Tuple, :exprs
      rule :Where, :bind, ";", :expr
    end

    symbol do
      rule :Paren, "(", [:MetaParenExpressionBody, :paren], ")"
    end
  end

  node :MetaBlockExpression do
    let :MetaStatements, :stmts

    ctor :Block, :stmts, fmt: ["{\n", :stmts, "\n}"]

    symbol do
      rule :Block, "{", [:MetaStatementsOpt, :stmts], "}"
    end
  end

  node :MetaMessageExpression do
    let :MetaExpression, :expr
    let :MetaMessageSelectors, :sels

    ctor :Message, :expr, :sels, fmt: ["[", :expr, " ", :sels, "]"]

    symbol do
      rule :Message, "[", :expr, :sels, "]"
    end
  end

  node :MetaMessageSelectors do
    let :MetaMessageSelectors, :sels
    let :MetaMessageSelector, :sel

    ctor :Empty, fmt: []
    ctor :Selectors, :sels, :sel, fmt: [:sels, " | ", :sel]
    ctor :Selector, :sel, fmt: :sel

    symbol :MetaMessageSelectorsOpt do
      rule :Empty, [:PipeOpt]
      rule :self, [:MetaMessageSelectors, :self]
    end

    symbol do
      rule :self, [:MetaMessageSelectorsPart, :self], [:PipeOpt]
    end

    symbol :MetaMessageSelectorsPart do
      rule :Selectors, [:MetaMessageSelectorsPart, :sels], "|", :sel
      rule :Selector, :sel
    end
  end

  node :MetaMessageSelector do
    union do
      let :Token, :tok
      let :MetaMessageKeywords, :kws
    end

    ctor :Unary, :tok, fmt: :tok
    ctor :Keyword, :kws, fmt: :kws

    symbol do
      rule :Unary, [:Identifier, :tok]
      rule :Keyword, :kws
    end
  end

  node :MetaMessageKeywords do
    let :MetaMessageKeywords, :kws
    let :MetaMessageKeyword, :kw

    ctor :Keywords, :kws, :kw, fmt: [:kws, " ", :kw]
    ctor :Keyword, :kw, fmt: :kw

    symbol do
      rule :Keywords, :kws, :kw
      rule :Keyword, :kw
    end
  end

  node :MetaMessageKeyword do
    let :Token, :id
    let :MetaExpression, :expr

    ctor :Keyword, :id, :expr, fmt: [:id, " ", :expr]

    symbol do
      rule :Keyword, [:AtomKeyword, :id], :expr
    end
  end
end