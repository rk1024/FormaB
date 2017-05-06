require_relative 'astgen/astgen'

require 'strscan'

ASTGen.run do
  node :Primaries do
    let :Primaries, :prims
    let :Primary, :prim

    ctor :Empty
    ctor :Primary, :prim
    ctor :Primaries, :prims, :prim

    ydtor "if (!($$->rooted() || tag->prims == $$)) delete $$;"

    syntax :Primary, :prim
    syntax :Primaries, :prims, :prim

    fmt :Empty
    fmt :Primary, ":prim"
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

    ctor :Group, :group
    ctor :RawBlock, :rawblk
    ctor :MetaBlock, :metablk
    ctor toks, :token

    syntax :Group, :group
    syntax :RawBlock, :rawblk
    syntax :MetaBlock, :metablk
    toks.each do |e|
      syntax e, [e, :token]
    end

    fmt :Group, ":group"
    fmt :RawBlock, ":rawblk"
    fmt :MetaBlock, ":metablk"
    fmt toks, ":token"
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

    fmt :MetaBlock, "@!meta{\\n:decls\\n@}"
  end

  node :Group do
    let :Primaries, :prims

    ctor :PGroup, :prims
    ctor :KGroup, :prims
    ctor :CGroup, :prims

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

    ctor :Empty
    ctor :Declaration, :decl
    ctor :Declarations, :decls, :decl

    syntax :Declaration, :decl
    syntax :Declarations, :decls, :decl

    fmt :Empty
    fmt :Declaration, ":decl"
    fmt :Declarations, ":decls :decl"

    symbol :MetaDeclarationsOpt do
      syntax :Empty
      syntax :self, :self
    end
  end

  node :MetaDeclaration do
    union do
      let :MetaSyntaxExt, :ext
      let :MetaLetDecl, :let
    end

    ctor :SyntaxExt, :ext
    ctor :LetDecl, :let

    syntax :SyntaxExt, :ext
    syntax :LetDecl, :let

    fmt :SyntaxExt, ":ext"
    fmt :LetDecl, ":let"
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

    ctor :Function, :func
    ctor :SyntaxExt, :ext
    ctor :Record, :rec

    syntax :Function, :func
    syntax :SyntaxExt, :ext
    syntax :Record, :rec

    fmt :Function, ":func"
    fmt :SyntaxExt, ":ext"
    fmt :Record, ":rec"
  end

  node :MetaFunction do
    let :MetaFuncArgs, :args
    let :MetaFuncBody, :body

    ctor :Function, :args, :body

    syntax :Function, "function", :args, :body

    fmt :Function, "function :args :body"
  end

  node :MetaSyntaxExt do
    let :MetaSyntaxExtArgs, :args
    let :MetaFuncBody, :body

    ctor :SyntaxExt, :args, :body

    syntax :SyntaxExt, "function", :args, :body

    fmt :SyntaxExt, "function :args :body"
  end

  node :MetaRecord do
    let :MetaFuncArgs, :args

    ctor :Record, :args

    syntax :Record, "record", :args

    fmt :Record, "record:args"
  end

  node :MetaFuncArgs do
    let :MetaFuncArgList, :args

    ctor :FuncArgs, :args

    syntax :FuncArgs, "(", :args, [:CommaOpt], ")"

    fmt :FuncArgs, "(:args)"
  end

  node :MetaSyntaxExtArgs do
    let :MetaSyntaxExtArgList, :args

    ctor :FuncArgs, :args

    syntax :FuncArgs, "(", :args, [:CommaOpt], ")"

    fmt :FuncArgs, "(:args))"
  end

  node :MetaFuncArgList do
    let :MetaFuncArgList, :args
    let :MetaFuncArg, :arg

    ctor :Argument, :arg
    ctor :Arguments, :args, :arg

    syntax :Argument, :arg
    syntax :Arguments, :args, ",", :arg

    fmt :Argument, ":arg"
    fmt :Arguments, ":args, :arg"
  end

  node :MetaSyntaxExtArgList do
    let :MetaSyntaxExtArgList, :args
    let :MetaSyntaxExtArg, :arg

    ctor :Argument, :arg
    ctor :Arguments, :args, :arg

    syntax :Argument, :arg
    syntax :Arguments, :args, ",", :arg

    fmt :Argument, ":arg"
    fmt :Arguments, ":args, :arg"
  end

  node :MetaFuncArg do
    let :Token, :type
    let :Token, :name

    ctor :Argument, :type, :name

    syntax :Argument, [:Identifier, :type], [:Identifier, :name]

    fmt :Argument, ":type :name"
  end

  node :MetaSyntaxExtArg do
    union do
      let :MetaSyntaxExtNamedArg, :namedArg
    end

    ctor :NamedArgument, :namedArg

    syntax :NamedArgument, "{", :namedArg, "}"

    fmt :NamedArgument, "{ :namedArg }"
  end

  node :MetaSyntaxExtNamedArg do
    let :Token, :type
    let :Token, :name

    ctor :Argument, :type, :name

    syntax :Argument, [:Identifier, :type], [:Identifier, :name]

    fmt :Argument, ":type :name"
  end

  node :MetaFuncBody do
    union do
      let :MetaFuncExprBody, :expr
      let :MetaFuncStmtBody, :stmts
    end

    ctor :Expression, :expr
    ctor :Statements, :stmts

    syntax :Expression, :expr
    syntax :Statements, :stmts

    fmt :Expression, ":expr"
    fmt :Statements, ":stmts"
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

    fmt :StmtBody, ":stmts"
  end

  node :MetaStatements do
    let :MetaStatements, :stmts
    let :MetaStatement, :stmt

    ctor :Statement, :stmt
    ctor :Statements, :stmts, :stmt

    syntax :Statement, :stmt
    syntax :Statements, :stmts, :stmt

    fmt :Statement, ":stmt"
    fmt :Statements, ":stmts :stmt"
  end

  node :MetaStatement do
    union do
      let :MetaLetStatement, :let
      let :MetaExpression, :expr
    end

    ctor :Empty
    ctor :LetStatement, :let
    ctor :Expression, :expr

    syntax :Empty, ";"
    syntax :LetStatement, :let, ";"
    syntax :Expression, :expr, ";"

    fmt :Empty
    fmt :LetStatement, ":let"
    fmt :Expression, ":expr"
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

    ctor :Infix, :infix

    syntax :Infix, :infix

    fmt :Infix, ":infix"
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

    ctor :Prefix, :prefix

    syntax :self, [:MetaAddExpression, :self]

    fmt :Prefix, ":prefix"

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
    let :MetaPostfixExpression, :postfix

    ctor :Postfix, :postfix

    syntax :Postfix, :postfix

    fmt :Postfix, ":postfix"
  end

  node :MetaPostfixExpression do
    let :MetaPrimary, :prim

    ctor :Primary, :prim

    syntax :Primary, :prim

    fmt :Primary, ":prim"
  end

  node :MetaPrimary do
    union do
      let :Token, :token
    end

    toks = [
      :Number
    ]

    ctor toks, :token

    toks.each do |e|
      syntax e, [e, :token]
    end

    fmt toks, ":token"
  end
end
