require_relative 'astgen/astgen'

ASTGen.run do
  node :Primaries do
    let :Primaries, :prims
    let :Primary, :prim

    ctor :Empty, fmt: []
    ctor :Primaries, :prims, :prim, fmt: [:prims, " ", :prim]
    ctor :Primary, :prim, fmt: :prim

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
    let :MetaStatements, :stmts

    ctor :MetaBlock, :stmts, fmt: ["@!meta{\n", :stmts, "\n@}"]

    symbol do
      rule :MetaBlock, [:MetaBlockStart], [:MetaStatementsOpt, :stmts], [:MetaBlockEnd]
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



  node :MetaStatements do
    let :MetaStatements, :stmts
    let :MetaStatement, :stmt

    ctor :Empty, fmt: []
    ctor :Statement, :stmt, fmt: :stmt
    ctor :Statements, :stmts, :stmt, fmt: [:stmts, "\n", :stmt]

    symbol do
      rule :Statements, :stmts, :stmt
      rule :Statement, :stmt
    end

    symbol :MetaStatementsOpt do
      rule :Empty
      rule :self, :self
    end
  end

  node :MetaStatement do
    union do
      let :MetaExpression, :expr
      let :MetaLetStatement, :let
    end

    ctor :Expression, :expr, fmt: :expr
    ctor :Let, :let, fmt: :let

    symbol do
      rule :Expression, :expr
      rule :Let, :let
    end
  end

  node :MetaExpression do
    let :MetaPrimary, :prim

    ctor :Primary, :prim, fmt: :prim

    symbol do
      rule :Primary, :prim
    end
  end

  node :MetaInfixExpression do
    let :MetaInfixExpression, :infixl

    union do
      let :MetaInfixExpression, :infixr
      let :MetaPrefixExpression, :prefix
    end

    ctor [

    ], :infixl, :infixr
    ctor :Mod, :infixl, :prefix

    
  end

  node :MetaPrimary do
    symbol do

    end
  end

  node :MetaLetStatement do
    let :Token, :id
    let :MetaExpression, :expr

    ctor :Let, :id, :expr, fmt: ["let ", :id, " = ", :expr]

    symbol do
      rule :Let, "let", [:Identifier, :id], "=", :expr
    end
  end
end