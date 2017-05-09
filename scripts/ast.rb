require_relative 'astgen/astgen'

require 'strscan'

ASTGen.run do
  node :Primaries do
    let :Primaries, :prims
    let :Primary, :prim

    empty :Empty, with_syntax: false
    single :Primary, :prim
    ctor :Primaries, :prims, :prim

    ydtor "if (!($$->rooted() || tag->prims == $$)) delete $$;"

    syntax :Primaries, :prims, :prim

    fmt :Primaries, ":prims :prim"

    symbol :PrimariesOpt do
      syntax :Empty
      syntax :self, :self
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

    single :Group, :group
    single :RawBlock, :rawblk
    single :MetaBlock, :metablk
    single toks, :token, with_syntax: false

    toks.each do |e|
      syntax e, [e, :token]
    end
  end

  node :RawBlock do
    let :Token, :id
    let :Token, :body

    ctor :RawBlock, :id, :body

    syntax :RawBlock, [:RawBlockID, :id], [:RawBlockBody, :body]

    fmt :RawBlock, "@!:id{:body@}"
  end

  node :MetaBlock do
    let :MetaDeclarations, :decls

    ctor :MetaBlock, :decls

    syntax :MetaBlock, [:MetaBlockStart], [:MetaDeclarationsOpt, :decls], [:MetaBlockEnd]

    fmt :MetaBlock, "@!meta{\n:decls\n@}"
  end

  node :Group do
    let :Primaries, :prims

    ctor [:PGroup, :KGroup, :CGroup], :prims

    syntax :PGroup, "(", [:PrimariesOpt, :prims], ")"
    syntax :KGroup, "[", [:PrimariesOpt, :prims], "]"
    syntax :CGroup, "{", [:PrimariesOpt, :prims], "}"

    fmt :PGroup, "(:prims)"
    fmt :KGroup, "[:prims]"
    fmt :CGroup, "{:prims}"
  end



  node :MetaDeclarations do
    let :MetaDeclarations, :decls
    let :MetaDeclaration, :decl

    empty :Empty, with_syntax: false
    single :Declaration, :decl
    ctor :Declarations, :decls, :decl

    syntax :Declarations, :decls, :decl

    fmt :Declarations, ":decls\n\n:decl"

    symbol :MetaDeclarationsOpt do
      syntax :Empty
      syntax :self, :self
    end
  end

  node :MetaDeclaration do
    union do
      let :MetaLetDecl, :let
    end

    single :LetDecl, :let
  end

  node :MetaLetDecl do
    let :Token, :id
    let :MetaLetRHS, :rhs

    ctor :LetDecl, :id, :rhs

    syntax :LetDecl, [:MetaLetLHS, :id], :rhs

    fmt :LetDecl, "let :id = :rhs"
  end

  node :MetaLetRHS do
    union do
      let :MetaFunction, :func
      let :MetaSyntaxExt, :ext
      let :MetaRecord, :rec
    end

    single :Function, :func
    single :SyntaxExt, :ext
    single :Record, :rec
  end

  node :MetaFunction do
    let :MetaFuncArgList, :args
    let :MetaFuncBody, :body

    ctor :Function, :args, :body

    syntax :Function, "function", :args, :body

    fmt :Function, "function:args :body"
  end

  node :MetaSyntaxExt do
    let :MetaSyntaxExtArgList, :args
    let :MetaFuncBody, :body

    ctor :SyntaxExt, :args, :body

    syntax :SyntaxExt, "function", :args, :body

    fmt :SyntaxExt, "function:args :body"
  end

  node :MetaRecord do
    let :MetaFuncArgList, :args

    ctor :Record, :args

    syntax :Record, "record", :args, ";"

    fmt :Record, "record:args;"
  end

  node :MetaFuncArgList do
    let :MetaFuncArgs, :args

    ctor :FuncArgs, :args

    syntax :FuncArgs, "(", :args, [:CommaOpt], ")"

    fmt :FuncArgs, "(:args)"
  end

  node :MetaSyntaxExtArgList do
    let :MetaSyntaxExtArgs, :args

    ctor :FuncArgs, :args

    syntax :FuncArgs, "(", [:SAStart], :args, [:SAEnd], ")"

    fmt :FuncArgs, "(`:args`)"
  end

  node :MetaFuncArgs do
    let :MetaFuncArgs, :args
    let :MetaFuncArg, :arg

    single :Argument, :arg
    ctor :Arguments, :args, :arg

    syntax :Arguments, :args, ",", :arg

    fmt :Arguments, ":args, :arg"
  end

  node :MetaSyntaxExtArgs do
    let :MetaSyntaxExtArgs, :args
    let :MetaSyntaxExtArg, :arg

    single :Argument, :arg
    ctor :Arguments, :args, :arg

    syntax :Arguments, :args, :arg

    fmt :Arguments, ":args :arg"
  end

  node :MetaFuncArg do
    let :Token, :typen
    let :Token, :name

    ctor :Argument, :typen, :name

    syntax :Argument, [:Identifier, :typen], [:Identifier, :name]

    fmt :Argument, ":typen :name"
  end

  node :MetaSyntaxExtArg do
    union do
      let :Token, :tok
      let :MetaSyntaxExtGroup, :group
    end

    toks = [
      :Identifier,
      :Operator,
      :Number,
    ]

    ctor :NamedArgument, :tok
    single toks, :tok, with_syntax: false
    single :Group, :group

    syntax :NamedArgument, "{", [:Identifier, :tok], "}"

    toks.each do |tok|
      syntax tok, [tok, :tok]
    end

    fmt :NamedArgument, "{:tok}"
  end

  node :MetaSyntaxExtGroup do
    let :MetaSyntaxExtArgs, :args

    ctor [:PGroup, :KGroup, :CGroup], :args

    syntax :PGroup, "(", :args, ")"
    syntax :KGroup, "[", :args, "]"
    syntax :CGroup, "{{", :args, "}}"

    fmt :PGroup, "(:args)"
    fmt :KGroup, "[:args]"
    fmt :CGroup, "{{:args}}"
  end

  node :MetaFuncBody do
    union do
      let :MetaFuncExprBody, :expr
      let :MetaFuncStmtBody, :stmts
    end

    single :Expression, :expr
    single :Statements, :stmts
  end

  node :MetaFuncExprBody do
    let :MetaExpression, :expr

    ctor :ExprBody, :expr

    syntax :ExprBody, "=>", :expr, ";"

    fmt :ExprBody, "=> :expr;"
  end

  node :MetaFuncStmtBody do
    let :MetaStatements, :stmts

    ctor :StmtBody, :stmts

    syntax :StmtBody, "{", :stmts, "}"

    fmt :StmtBody, "{\n:stmts\n}"
  end

  node :MetaStatements do
    let :MetaStatements, :stmts
    let :MetaStatement, :stmt

    single :Statement, :stmt
    ctor :Statements, :stmts, :stmt

    syntax :Statements, :stmts, :stmt

    fmt :Statements, ":stmts :stmt"
  end

  node :MetaStatement do
    union do
      let :MetaLetStatement, :let
      let :MetaExpression, :expr
    end

    empty :Empty, with_syntax: false
    ctor :LetStatement, :let
    ctor :Expression, :expr

    syntax :Empty, ";"
    syntax :LetStatement, :let, ";"
    syntax :Expression, :expr, ";"

    fmt :LetStatement, ":let;"
    fmt :Expression, ":expr;"
  end

  node :MetaLetStatement do
    let :Token, :id
    let :MetaExpression, :expr

    ctor :LetStatement, :id, :expr

    syntax :LetStatement, [:MetaLetLHS, :id], :expr

    fmt :LetStatement, "let :id = :expr"
  end

  node :MetaExpression do
    union do
      let :MetaInfixExpression, :infix
    end

    single :Infix, :infix
  end

  node :MetaInfixExpression do
    union do
      let :MetaInfixExpression, :infixr
      let :MetaPrefixExpression, :prefix
    end
    let :MetaInfixExpression, :infixl

    left = {
      Add: ["+", "(:infixl + :infixr)", :MetaSubExpression],
      Sub: ["-", "(:infixl - :infixr)", :MetaMulExpression],
      Mul: ["*", "(:infixl * :infixr)", :MetaDivExpression],
      Div: ["/", "(:infixl / :infixr)", :MetaModExpression],
    }

    last = {
      Mod: ["%", "(:infixl % :prefix)"],
    }

    ctor left.map{|k, _| k }, :infixl, :infixr
    ctor last.map{|k, _| k }, :infixl, :prefix

    single :Prefix, :prefix, with_syntax: false

    syntax :self, [:MetaAddExpression, :self]

    left.each do |key, val|
      sym = "Meta#{key}Expression".to_sym

      fmt key, val[1]

      symbol sym do
        syntax key, [sym, :infixl], val[0], [val[2], :infixr]
        syntax :self, [val[2], :self]
      end
    end

    last.each do |key, val|
      sym = "Meta#{key}Expression".to_sym

      fmt key, val[1]

      symbol sym do
        syntax key, [sym, :infixl], val[0], :prefix
        syntax :Prefix, :prefix
      end
    end
  end

  node :MetaPrefixExpression do
    union do
      let :MetaPrefixExpression, :prefix
      let :MetaPostfixExpression, :postfix
    end

    single :Postfix, :postfix
  end

  node :MetaPostfixExpression do
    let :MetaPrimary, :prim

    single :Primary, :prim
  end

  node :MetaPrimary do
    union do
      let :Token, :token
    end

    toks = [
      :Number
    ]

    single toks, :token, with_syntax: false

    toks.each do |e|
      syntax e, [e, :token]
    end
  end
end
