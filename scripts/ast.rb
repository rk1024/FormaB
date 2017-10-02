require_relative 'astgen/astgen'

module ASTGen
  Node.namespace = "frma"
  Node.class_prefix = "F"
  Node.class_base = "frma::FormaAST"
  Node.class_token = "frma::FToken"

  Node.header_dir = "ast"
  Node.header_base = "ast/astBase.hpp"
  Node.header_token = "ast/token.hpp"

  Node.name_prefixes = {
    prae: "P",
  }

  Node.name_suffixes = {
    expression: "X",
    literal: "L",
    statement: "S",
    declaration: "D",
  }

  Node.name_abbrevs = [
    ["Argument", "Arg"],
    ["Declaration", "Decl"],
    ["Expression", "Expr"],
    ["Function", "Func"],
    ["Message", "Msg"],
    ["Parameter", "Param"],
    ["Primary", "Prim", "Primaries"],
    ["Statement", "Stmt"],
  ]
end

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
      let :PraeBlock, :praeblk
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
    ctor :PraeBlock, :praeblk, fmt: :praeblk
    ctor toks, :tok, fmt: :tok

    symbol do
      toks.each{|t| rule t, [t, :tok] }
      rule :Group, :group
      rule :RawBlock, :rawblk
      rule :PraeBlock, :praeblk
    end
  end

  node :RawBlock do
    let :Token, :id
    let :Token, :body

    ctor :RawBlock, :id, :body, fmt: ["@!", :id, "{", :body, "@}"]
    ctor :Error, :id, fmt: ["@!", :id, "{ <ERROR> @}"]

    symbol do
      rule :RawBlock, [:RawBlockID, :id], [:RawBlockBody, :body]
      rule :Error, [:RawBlockID, :id], [:error], action: "yyerrok;"
    end
  end

  node :PraeBlock do
    let :PraeStatements, :stmts

    ctor :PraeBlock, :stmts, fmt: ["@!prae{\n", :stmts, "\n@}"]
    ctor :Error, fmt: "@!prae{ <ERROR> @}"

    symbol do
      rule :PraeBlock, [:PraeBlockStart], [:PraeStatementsOpt, :stmts], [:PraeBlockEnd]
      rule :Error, [:PraeBlockStart], [:error], [:PraeBlockEnd], action: "yyerrok;"
    end
  end

  node :Group do
    let :Primaries, :prims

    ctor [:PGroup, :KGroup, :CGroup], :prims
    ctor [:PError, :KError, :CError]

    fmt :PGroup, "(", :prims, ")"
    fmt :KGroup, "{ ", :prims, " }"
    fmt :CGroup, "[", :prims, "]"

    fmt :PError, "( <ERROR> )"
    fmt :KError, "{ <ERROR> }"
    fmt :CError, "[ <ERROR> ]"

    symbol do
      rule :PGroup, "(", :prims, ")"
      rule :KGroup, "{", :prims, "}"
      rule :CGroup, "[", :prims, "]"

      rule :PError, "(", [:error], ")", action: "yyerrok;"
      rule :KError, "{", [:error], "}", action: "yyerrok;"
      rule :CError, "[", [:error], "]", action: "yyerrok;"
    end
  end

  open_parts = [:Open, :Closed].freeze

  expr_parts = [
    open_parts,
  ].freeze

  do_exprs = lambda do |&block|
    expr_parts.reduce([nil]) {|c, p| c.product(p) }.each do |n, *parts|
      abort unless n == nil
      block.(:"#{parts.join}", *parts)
    end
  end

  node :PraeStatements do
    let :PraeStatements, :stmts
    let :PraeStatement, :stmt

    ctor :Empty, fmt: []
    ctor :Statements, :stmts, :stmt, fmt: [:stmts, "\n", :stmt]
    ctor :Statement, :stmt, fmt: :stmt

    symbol :PraeStatementsPart do
      rule :Statements, [:PraeStatementsPart, :stmts], :stmt
      rule :Statement, :stmt
    end

    symbol do
      rule :self, [:PraeStatementsPart, :self]
    end

    symbol :PraeStatementsOpt do
      rule :Empty
      rule :self, [:PraeStatements, :self]
    end
  end

  node :PraeExpressions do
    let :PraeExpressions, :exprs
    let :PraeExpression, :expr

    ctor :Empty, fmt: []
    ctor :Exprs, :exprs, :expr, fmt: [:exprs, ", ", :expr]
    ctor :Expr, :expr, fmt: :expr

    symbol :PraeExpressionsOpt do
      rule :Empty, [:CommaOpt]
      rule :self, [:PraeExpressions, :self]
    end

    symbol do
      rule :self, [:PraeExpressionsPart, :self], [:CommaOpt]
    end

    symbol :PraeExpressionsPart do
      rule :Exprs, me(:exprs), ",", :expr
      rule :Expr, :expr
    end
  end

  node :PraeStatement do
    union do
      let :PraeDeclaration, :decl
      let :PraeExpression, :expr
      let :PraeKeywordStatement, :kw
      let :PraeBindStatement, :bind
      let :PraeAssignStatement, :assign
      let :PraeControlStatement, :ctl
    end

    ctor [:SemiDecl, :NonSemiDecl], :decl
    ctor [:SemiExpr, :NonSemiExpr], :expr
    ctor :Keyword, :kw, fmt: [:kw, ";"]
    ctor :Bind, :bind, fmt: [:bind, ";"]
    ctor :Assign, :assign, fmt: [:assign, ";"]
    ctor :Control, :ctl, fmt: :ctl
    ctor :Error, fmt: "<ERROR> ;"

    fmt :SemiDecl, :decl, ";"
    fmt :NonSemiDecl, :decl
    fmt :SemiExpr, :expr, ";"
    fmt :NonSemiExpr, :expr

    do_exprs.() do |parts, open|
      symbol :"Prae#{parts}PureStatement" do
        rule :SemiExpr, [:"Prae#{parts}SemiExpression", :expr], ";"
        rule :NonSemiExpr, [:"Prae#{parts}NonSemiExpression", :expr]

        case open
          when :Closed
            rule :Error, [:error], ";", action: "yyerrok;"
        end
      end

      symbol :"Prae#{parts}DeclStatement" do
        rule :SemiDecl, [:"Prae#{parts}SemiDeclaration", :decl], ";"
        rule :NonSemiDecl, [:"Prae#{parts}NonSemiDeclaration", :decl]
      end

      symbol :"Prae#{parts}Statement" do
        rule :self, [:"Prae#{parts}PureStatement", :self]
        rule :self, [:"Prae#{parts}DeclStatement", :self]
        rule :Keyword, [:"Prae#{parts}KeywordStatement", :kw], ";"
        rule :Bind, [:"Prae#{parts}BindStatement", :bind], ";"
        rule :Control, [:"Prae#{parts}ControlStatement", :ctl]

        case open
          when :Closed
            rule :Assign, :assign, ";"
        end
      end
    end

    symbol do
      do_exprs.(){|p| rule :self, [:"Prae#{p}Statement", :self] }
    end
  end

  node :PraeDeclaration do
    union do
      let :PraeMessageDeclaration, :msg
      let :PraeTypeDeclaration, :type
    end

    ctor :Message, :msg, fmt: :msg
    ctor :Type, :type, fmt: :type

    do_exprs.() do |parts, open|
      symbol :"Prae#{parts}SemiDeclaration" do
        rule :Message, [:"Prae#{parts}MessageDeclaration", :msg]

        case open
          when :Closed
            rule :Type, :type
        end
      end

      symbol :"Prae#{parts}NonSemiDeclaration" do end
    end
  end

  node :PraeMessageDeclaration do
    let :PraeDeclPatternSpec, :target
    let :PraeMessageDeclSelector, :sel
    let :PraeExpression, :body

    ctor :Message, :target, :sel, :body, fmt: ["let ", :target, " [", :sel, "] => ", :body]

    do_exprs.() do |parts|
      symbol :"Prae#{parts}MessageDeclaration" do
        rule :Message, "let", :target, "[", :sel, "]", "=>", [:"Prae#{parts}Expression", :body]
      end
    end
  end

  node :PraeMessageDeclSelector do
    union do
      let :Token, :tok
      let :PraeMessageDeclKeywords, :kws
    end

    ctor :Unary, :tok, fmt: :tok
    ctor :Keyword, :kws, fmt: :kws

    symbol do
      rule :Unary, [:Identifier, :tok]
      rule :Keyword, :kws
    end
  end

  node :PraeMessageDeclKeywords do
    let :PraeMessageDeclKeywords, :kws
    let :PraeMessageDeclKeyword, :kw

    ctor :Keywords, :kws, :kw, fmt: [:kws, " ", :kw]
    ctor :Keyword, :kw, fmt: :kw

    symbol do
      rule :Keywords, :kws, :kw
      rule :Keyword, :kw
    end
  end

  node :PraeMessageDeclKeyword do
    let :Token, :key
    let :PraeDeclPatternSpec, :pat

    ctor :Keyword, :key, :pat, fmt: [:key, " ", :pat]

    symbol do
      rule :Keyword, [:AtomKeyword, :key], :pat
    end
  end

  node :PraeDeclPatternSpec do
    let :Token, :id
    let :PraeDeclPattern, :pat

    ctor :AnonAny, fmt: "_"
    ctor :NamedAny, :id, fmt: :id
    ctor :AnonPattern, :pat, fmt: ""
    ctor :NamedPattern, :pat, :id, fmt: [" ", :id]

    symbol do
      rule :self, [:PraeAnonDeclPatternSpec, :self]
      rule :self, [:PraeNamedDeclPatternSpec, :self]
    end

    symbol :PraeAnonDeclPatternSpec do
      rule :AnonAny, "_"
      rule :AnonPattern, :pat, "_"
    end

    symbol :PraeNamedDeclPatternSpec do
      rule :NamedAny, [:Identifier, :id]
      rule :NamedPattern, :pat, [:Identifier, :id]
    end
  end

  node :PraeDeclPattern do
    let :Token, :tok

    ctor :Type, :tok, fmt: :tok

    symbol do
      rule :Type, [:Identifier, :tok]
    end
  end

  node :PraeTypeDeclaration do
    let :PraeTypeDeclId, :id

    union do
      let :PraeStructStatements, :strukt
      let :PraeVariantStatements, :variant
    end

    ctor :Struct, :id, :strukt, fmt: ["let ", :id, " => struct {\n", :strukt, "\n}"]
    ctor :Variant, :id, :variant, fmt: ["let ", :id, " => variant {\n", :variant, "\n}"]
    ctor :Interface, :id, fmt: ["let ", :id, " => interface {\n", "\n}"]

    symbol do
      rule :Struct, "let", :id, "=>", "struct", "{", [:PraeStructStatementsOpt, :strukt], "}"
      rule :Variant, "let", :id, "=>", "variant", "{", [:PraeVariantStatementsOpt, :variant], "}"
      rule :Interface, "let", :id, "=>", "interface", "{", "}"
    end
  end

  node :PraeTypeDeclId do
    let :Token, :id
    let :PraeMessageDeclSelector, :sel

    ctor :Simple, :id, fmt: :id
    ctor :Template, :id, :sel, fmt: [:id, "(", :sel, ")"]

    symbol do
      rule :Simple, [:Identifier, :id]
      rule :Template, [:Identifier, :id], "(", :sel, ")"
    end
  end

  node :PraeStructStatements do
    let :PraeStructStatements, :stmts
    let :PraeStructStatement, :stmt

    ctor :Empty, fmt: []
    ctor :Statements, :stmts, :stmt, fmt: [:stmts, "\n", :stmt]
    ctor :Statement, :stmt, fmt: :stmt

    symbol :PraeStructStatementsOpt do
      rule :Empty
      rule :self, [:PraeStructStatements, :self]
    end

    symbol do
      rule :Statements, :stmts, :stmt
      rule :Statement, :stmt
    end
  end

  node :PraeStructStatement do
    let :Token, :type
    let :Token, :id

    ctor :Member, :type, :id, fmt: ["let ", :type, " ", :id, ";"]

    symbol do
      rule :Member, "let", [:Identifier, :type], [:Identifier, :id], ";"
    end
  end

  node :PraeVariantStatements do
    let :PraeVariantStatements, :stmts
    let :PraeVariantStatement, :stmt

    ctor :Empty, fmt: []
    ctor :Statements, :stmts, :stmt, fmt: [:stmts, "\n", :stmt]
    ctor :Statement, :stmt, fmt: :stmt

    symbol :PraeVariantStatementsOpt do
      rule :Empty
      rule :self, [:PraeVariantStatements, :self]
    end

    symbol do
      rule :Statements, :stmts, :stmt
      rule :Statement, :stmt
    end
  end

  node :PraeVariantStatement do
    let :Token, :type
    let :Token, :id

    ctor :Type, :type, :id, fmt: ["let ", :type, " => ", :id, ";"]
    ctor :VoidType, :type, fmt: ["let ", :type, " => void;"]

    symbol do
      rule :Type, "let", [:Identifier, :type], "=>", [:Identifier, :id], ";"
      rule :VoidType, "let", [:Identifier, :type], "=>", "void", ";"
    end
  end

  node :PraeKeywordStatement do
    let :PraeExpression, :expr

    toks = {
      Break: "break",
      Next: "next",
      Return: "return",
      Yield: "yield",
    }

    single = [:Return, :Yield]

    unary = [:Break, :Next]

    ctor single, :expr
    ctor unary

    single.each{|a| fmt a, "#{toks[a]} ", :expr }
    unary.each{|a| fmt a, toks[a] }

    do_exprs.() do |parts, open|
      symbol :"Prae#{parts}KeywordStatement" do
        single.each{|a| rule a, toks[a], [:"Prae#{parts}Expression", :expr] }

        case open
          when :Closed
            unary.each{|a| rule a, toks[a] }
        end
      end
    end
  end

  node :PraeBindStatement do
    let :PraeBindings, :binds

    ctor [:Let, :Var], :binds

    fmt :Let, "let ", :binds
    fmt :Var, "var ", :binds

    do_exprs.() do |parts|
      symbol :"Prae#{parts}LetStatement" do
        rule :Let, "let", [:"Prae#{parts}Bindings", :binds]
      end

      symbol :"Prae#{parts}BindStatement" do
        rule :self, [:"Prae#{parts}LetStatement", :self]
        rule :Var, "var", [:"Prae#{parts}Bindings", :binds]
      end
    end

    symbol :PraeLetStatement do
      do_exprs.(){|p| rule :self, [:"Prae#{p}LetStatement", :self] }
    end

    symbol do
      do_exprs.(){|p| rule :self, [:"Prae#{p}BindStatement", :self] }
    end
  end

  node :PraeBindings do
    let :PraeBindings, :binds
    let :PraeBinding, :bind

    ctor :Bindings, :binds, :bind, fmt: [:binds, ", ", :bind]
    ctor :Binding, :bind, fmt: :bind

    do_exprs.() do |parts|
      symbol :"Prae#{parts}BindingsPart" do
        rule :Bindings, [:"Prae#{parts}BindingsPart", :binds], ",", [:"Prae#{parts}Binding", :bind]
        rule :Binding, [:"Prae#{parts}Binding", :bind]
      end

      symbol :"Prae#{parts}Bindings" do
        rule :self, [:"Prae#{parts}BindingsPart", :self], [:CommaOpt]
      end
    end
  end

  node :PraeBinding do
    let :Token, :id
    let :PraeExpression, :expr

    ctor :Binding, :id, :expr, fmt: [:id, " = ", :expr]

    do_exprs.() do |parts|
      symbol :"Prae#{parts}Binding" do
        rule :Binding, [:Identifier, :id], "=", [:"Prae#{parts}Expression", :expr]
      end
    end
  end

  node :PraeAssignStatement do
    let :PraeMemberExpression, :memb
    let :PraeAssignValue, :value

    ops = [
      [:Assign, ""],
      [:Con, "||"],
      [:Dis, "&&"],
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

  node :PraeAssignValue do
    union do
      let :PraeAssignStatement, :assign
      let :PraeInfixExpression, :infix
    end

    ctor :Assign, :assign, fmt: :assign
    ctor :Infix, :infix, fmt: :infix

    symbol do
      rule :Assign, :assign
      rule :Infix, :infix
    end
  end

  node :PraeExpression do
    union do
      let :PraeFunctionExpression, :func
      let :PraeInfixExpression, :infix
      let :PraeControlExpression, :ctl
    end

    ctor :Function, :func, fmt: :func
    ctor :Infix, :infix, fmt: :infix
    ctor :Control, :ctl, fmt: :ctl

    do_exprs.() do |parts, open|
      symbol :"Prae#{parts}SemiExpression" do
        case open
          when :Open
          when :Closed
            rule :Infix, :infix
        end

        rule :Function, [:"Prae#{parts}FunctionExpression", :func]
      end

      symbol :"Prae#{parts}NonSemiExpression" do end

      symbol :"Prae#{parts}PureExpression" do
        rule :self, [:"Prae#{parts}SemiExpression", :self]
        rule :self, [:"Prae#{parts}NonSemiExpression", :self]
      end

      symbol :"Prae#{parts}Expression" do
        rule :self, [:"Prae#{parts}PureExpression", :self]
        rule :Control, [:"Prae#{parts}ControlExpression", :ctl]
      end
    end

    symbol do
      do_exprs.(){|p| rule :self, [:"Prae#{p}Expression", :self] }
    end
  end

  node :PraeFunctionExpression do
    let :PraeFunctionParameters, :params
    let :PraeExpression, :expr

    ctor :Function, :params, :expr, fmt: [:params, " => ", :expr]

    do_exprs.() do |parts|
      symbol :"Prae#{parts}FunctionExpression" do
        rule :Function, :params, "=>", [:"Prae#{parts}Expression", :expr]
      end
    end
  end

  node :PraeFunctionParameters do
    let :PraeFunctionParameters, :params
    let :PraeFunctionParameter, :param

    ctor :List, :params, fmt: ["(", :params, ")"]
    ctor :Empty, fmt: []
    ctor :Error, fmt: "( <ERROR> )"
    ctor :Parameters, :params, :param, fmt: [:params, ", ", :param]
    ctor :Parameter, :param, fmt: :param

    symbol :PraeFunctionParametersPart do
      rule :Parameters, me(:params), ",", :param
      rule :Parameter, :param
    end

    symbol :PraeFunctionParametersPartOpt do
      rule :Empty, [:CommaOpt]
      rule :self, [:PraeFunctionParametersPart, :self], [:CommaOpt]
    end

    symbol do
      rule :List, "(", [:PraeFunctionParametersPartOpt, :params], ")"
      rule :Error, "(", [:error], ")", action: "yyerrok;"
    end
  end

  node :PraeFunctionParameter do
    let :Token, :id
    let :PraeDeclPatternSpec, :pat

    ctor :Parameter, :id, :pat, fmt: [:id, " ", :pat]

    symbol do
      rule :Parameter, [:AtomKeyword, :id], :pat
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
    node :"PraeControl#{expr}" do
      let :PraeParenExpression, :cond

      let :"Prae#{expr}", :then
      let :"Prae#{expr}", :otherwise

      types.each do |name, tok|
        ctor name, :cond, :then, fmt: ["#{tok} ", :cond, " ", :then] if do_single

        ctor :"#{name}#{:Else if do_single}", :cond, :then, :otherwise, fmt: ["#{tok} ", :cond, " ", :then, " else ", :otherwise]
      end

      do_exprs.() do |parts, open|
        symbol :"Prae#{parts}Control#{expr}" do
          types.each do |name, tok|
            if open == :Open && do_single
              open_parts.each{|o| rule name, tok.freeze, :cond, [:"Prae#{parts.to_s.gsub(open.to_s, o.to_s)}#{expr}", :then] }
            end

            rule :"#{name}#{:Else if do_single}", tok.freeze, :cond, [:"Prae#{parts.to_s.gsub(open.to_s, "Closed")}#{expr}", :then], "else", [:"Prae#{parts}#{expr}", :otherwise]
          end
        end
      end
    end
  end

  node :PraeInfixExpression do
    union do
      let :PraeInfixExpression, :infixr
      let :PraeUnaryExpression, :unary
    end

    let :PraeInfixExpression, :infixl

    ctor [
      :Dis,
      :Con,
      :Eql,
      :Neq,
      :Grt,
      :Lss,
      :Geq,
      :Leq,
      :Add,
      :Sub,
      :Mul,
      :Div,
    ], :infixl, :infixr
    ctor :Mod, :infixl, :unary
    ctor :Unary, :unary, fmt: :unary

    fmt :Con, :infixl, " || ", :infixr
    fmt :Dis, :infixl, " && ", :infixr
    fmt :Eql, :infixl, " == ", :infixr
    fmt :Neq, :infixl, " != ", :infixr
    fmt :Grt, :infixl, " > ", :infixr
    fmt :Lss, :infixl, " < ", :infixr
    fmt :Geq, :infixl, " >= ", :infixr
    fmt :Leq, :infixl, " <= ", :infixr
    fmt :Add, :infixl, " + ", :infixr
    fmt :Sub, :infixl, " - ", :infixr
    fmt :Mul, :infixl, " * ", :infixr
    fmt :Div, :infixl, " / ", :infixr
    fmt :Mod, :infixl, " % ", :unary

    symbol do end # Does something because of chaining (i.e. DON'T REMOVE THIS)

    chain :PraeDisjunctExpression do
      rule :Dis, me(:infixl), "||", defer(:infixr)
    end

    chain :PraeConjunctExpression do
      rule :Con, me(:infixl), "&&", defer(:infixr)
    end

    chain :PraeComparisonExpression do
      [
        [:Eql, "=="],
        [:Neq, "!="],
        [:Grt, ">"],
        [:Lss, "<"],
        [:Geq, ">="],
        [:Leq, "<="],
      ].each do |name, op|
        rule name, me(:infixl), op, defer(:infixr)
      end
    end

    chain :PraeAddExpression do
      rule :Add, me(:infixl), "+", defer(:infixr)
    end

    chain :PraeSubExpression do
      rule :Sub, me(:infixl), "-", defer(:infixr)
    end

    chain :PraeMulExpression do
      rule :Mul, me(:infixl), "*", defer(:infixr)
    end

    chain :PraeDivExpression do
      rule :Div, me(:infixl), "/", defer(:infixr)
    end

    chain :PraeModExpression do
      rule :Mod, me(:infixl), "%", :unary
      rule :Unary, :unary
    end
  end

  node :PraeUnaryExpression do
    union do
      let :PraeUnaryExpression, :unary
      let :PraeMemberExpression, :memb
    end

    ctor [
      :Inv,
      :Pos,
      :Neg,
      :Inc,
      :Dec,
    ], :unary
    ctor :Member, :memb, fmt: :memb

    fmt :Inv, "!", :unary
    fmt :Pos, "+", :unary
    fmt :Neg, "-", :unary
    fmt :Inc, "++", :unary
    fmt :Dec, "--", :unary

    symbol do
      rule :Inv, "!", :unary
      rule :Pos, "+", :unary
      rule :Neg, "-", :unary
      rule :Inc, "++", :unary
      rule :Dec, "--", :unary
      rule :Member, :memb
    end
  end

  node :PraeMemberExpression do
    union do
      let :PraePrimaryExpression, :prim
      let :PraeMemberExpression, :base
    end
    let :Token, :memb

    ctor :Primary, :prim, fmt: :prim
    ctor :Member, :base, :memb, fmt: [:base, ".", :memb]

    symbol do
      rule :Primary, :prim
      rule :Member, :base, ".", [:Identifier, :memb]
    end
  end

  node :PraePrimaryExpression do
    union do
      let :Token, :tok
      let :PraeNumericLiteral, :numeric
      let :PraeBooleanLiteral, :boolean
      let :PraeNullLiteral, :null
      let :PraeParenExpression, :paren
      let :PraeBlockExpression, :block
      let :PraeMessageExpression, :message
    end

    toks = [
      :Identifier,
      :SQLiteral,
      :DQLiteral
    ]

    ctor toks, :tok, fmt: :tok
    ctor :Numeric, :numeric, fmt: :numeric
    ctor :Boolean, :boolean, fmt: :boolean
    ctor :Null, :null, fmt: :null
    ctor :Parens, :paren, fmt: :paren
    ctor :Block, :block, fmt: :block
    ctor :Message, :message, fmt: :message

    symbol :PraeNonIdentifierPrimaryExpression do
      toks.select{|t| t != :Identifier }.each{|t| rule t, [t, :tok] }
      rule :Numeric, :numeric
      rule :Boolean, :boolean
      rule :Null, :null
      rule :Parens, :paren
      rule :Block, :block
      rule :Message, :message
    end

    symbol do
      rule :Identifier, [:Identifier, :tok]
      rule :self, [:PraeNonIdentifierPrimaryExpression, :self]
    end
  end

  node :PraeNumericLiteral do
    let :Token, :tok

    ctor [:Hex, :Dec, :Oct, :Bin, :Float], :tok, fmt: :tok

    symbol do
      rule :Hex, [:PraeHexLiteral, :tok]
      rule :Dec, [:PraeDecLiteral, :tok]
      rule :Oct, [:PraeOctLiteral, :tok]
      rule :Bin, [:PraeBinLiteral, :tok]
      rule :Float, [:PraeFloatLiteral, :tok]
    end
  end

  node :PraeBooleanLiteral do
    ctor :True, fmt: "true"
    ctor :False, fmt: "false"

    symbol do
      rule :True, "true"
      rule :False, "false"
    end
  end

  node :PraeNullLiteral do
    ctor :Nil, fmt: "nil"
    ctor :Void, fmt: "void"

    symbol do
      rule :Nil, "nil"
      rule :Void, "void"
    end
  end

  node :PraeParenExpression do
    union do
      let :PraeParenExpression, :paren
      let :PraeExpression, :expr
      let :PraeExpressions, :exprs
    end

    let :PraeBindStatement, :bind

    ctor :Paren, :paren, fmt: ["(", :paren, ")"]
    ctor :Error, fmt: "( <ERROR> )"

    ctor :Where, :bind, :expr, fmt: [:bind, "; ", :expr]
    ctor :Tuple, :exprs, fmt: :exprs

    symbol :PraeParenExpressionBody do
      rule :Tuple, :exprs
      rule :Where, :bind, ";", :expr
    end

    symbol do
      rule :Paren, "(", [:PraeParenExpressionBody, :paren], ")"
      rule :Error, "(", [:error], ")", action: "yyerrok;"
    end
  end

  node :PraeBlockExpression do
    let :PraeStatements, :stmts

    ctor :Block, :stmts, fmt: ["{\n", :stmts, "\n}"]
    ctor :Error, fmt: "{ <ERROR> }"

    symbol do
      rule :Block, "{", [:PraeStatementsOpt, :stmts], "}"
      rule :Error, "{", [:error], "}", action: "yyerrok;"
    end
  end

  node :PraeMessageExpression do
    let :PraePrimaryExpression, :expr
    let :PraeMessageSelector, :sel

    ctor [:Message, :Curry], :expr, :sel
    ctor :Coerce, :expr, fmt: [:expr, "!"]
    ctor :Error, :expr, fmt: [:expr, "[ <ERROR> ]"]

    fmt :Message, :expr, " [", :sel, "]"
    fmt :Curry, :expr, " ?[", :sel, "]"

    symbol do
      rule :Message, :expr, "[", :sel, "]"
      rule :Curry, :expr, "?", "[", :sel, "]"
      rule :Coerce, :expr, "!"
      rule :Error, :expr, "[", [:error], "]", action: "yyerrok;"
    end
  end

  node :PraeMessageSelector do
    union do
      let :Token, :tok
      let :PraeMessageKeywords, :kws
    end

    ctor :Unary, :tok, fmt: :tok
    ctor :Keyword, :kws, fmt: :kws

    symbol do
      rule :Unary, [:Identifier, :tok]
      rule :Keyword, :kws
    end
  end

  node :PraeMessageKeywords do
    let :PraeMessageKeywords, :kws
    let :PraeMessageKeyword, :kw

    ctor :Keywords, :kws, :kw, fmt: [:kws, " ", :kw]
    ctor :Keyword, :kw, fmt: :kw

    symbol do
      rule :Keywords, :kws, :kw
      rule :Keyword, :kw
    end
  end

  node :PraeMessageKeyword do
    let :Token, :id
    let :PraeExpression, :expr

    ctor :Keyword, :id, :expr, fmt: [:id, " ", :expr]

    symbol do
      rule :Keyword, [:AtomKeyword, :id], :expr
    end
  end
end