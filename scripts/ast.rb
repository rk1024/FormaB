require_relative 'astgen/astgen'

module ASTGen
  Node.namespace = "fps"
  Node.class_prefix = "F"
  Node.class_base = "fps::FASTBase"
  Node.class_token = "fps::FToken"
  Node.loc_type = "fdi::FLocation"

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
  export :Inputs

  node :Inputs do
    let :Inputs, :inputs
    let :Input, :input

    ctor :Inputs, :inputs, :input, fmt: [:inputs, :input]
    ctor :Input, :input, fmt: :input

    symbol do
      rule :Inputs, :inputs, :input
      rule :Input, :input
    end
  end

  node :Input do
    let :Token, :tok
    let :PraeBlock, :praeBlk

    ctor :Fragment, :tok, fmt: :tok
    ctor :PraeBlock, :praeBlk, fmt: :praeBlk

    symbol do
      rule :Fragment, [:Fragment, :tok]
      rule :PraeBlock, :praeBlk
    end
  end

  node :PraeBlock do
    let :PraeDeclarations, :decls

    ctor :Block, :decls, fmt: ["@!prae{\n", :decls, "\n@}"]

    symbol do
      rule :Block, [:PraeStart], [:PraeDeclarationsOpt, :decls], [:PraeEnd]
    end
  end

  ##### Declarations #####

  node :PraeDeclarations do
    let :PraeDeclarations, :decls
    let :PraeDeclaration, :decl

    ctor :Empty, fmt: []
    ctor :Decls, :decls, :decl, fmt: [:decls, "\n", :decl]
    ctor :Decl, :decl, fmt: :decl

    symbol do
      rule :Decls, :decls, :decl
      rule :Decl, :decl
    end

    symbol :PraeDeclarationsOpt do
      rule :Empty
      rule :self, [:PraeDeclarations, :self]
    end
  end

  node :PraeDeclaration do
    let :PraeDeclarationBody, :body

    ctor :Decl, :body, fmt: ["let ", :body, ";"]

    symbol do
      rule :Decl, "let", :body, ";"
    end
  end

  node :PraeDeclarationBody do
    let :PraeAssignDeclaration, :assign
    let :PraeSyntaxDeclaration, :syntax

    ctor :Assign, :assign, fmt: :assign
    ctor :Syntax, :syntax, fmt: :syntax

    symbol do
      rule :Assign, :assign
      rule :Syntax, :syntax
    end
  end

  node :PraeAssignDeclaration do
    let :Token, :name
    let :PraeExpression, :value

    ctor :Const, :name, :value, fmt: [:name, " = ", :value]

    symbol do
      rule :Const, [:PIdent, :name], "=", :value
    end
  end

  node :PraeSyntaxDeclaration do
    ctor :Syntax, fmt: []

    symbol do
      rule :Syntax
    end
  end

  ##### Expressions #####

  node :PraeExpression do
    let :PraeInfixExpression, :infix

    ctor :Infix, :infix, fmt: :infix

    symbol do
      rule :Infix, :infix
    end
  end

  node :PraeInfixExpression do
    let :PraeInfixExpression, :infixl

    union do
      let :PraeInfixExpression, :infixr
      let :PraeUnaryExpression, :unary
    end

    ctor [
      :Add,
      :Sub,
      :Mul,
      :Div,
    ], :infixl, :infixr
    ctor :Mod, :infixl, :unary
    ctor :Unary, :unary, fmt: :unary

    fmt :Add, :infixl, " + ", :infixr
    fmt :Sub, :infixl, " - ", :infixr
    fmt :Mul, :infixl, " * ", :infixr
    fmt :Div, :infixl, " / ", :infixr
    fmt :Mod, :infixl, " % ", :unary

    symbol do end # Chaining root (DO NOT REMOVE)

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
    let :PraePrimaryExpression, :prim

    ctor :Prim, :prim, fmt: :prim

    symbol do
      rule :Prim, :prim
    end
  end

  node :PraePrimaryExpression do
    let :Token, :tok

    ctor [:Ident, :Number], :tok, fmt: :tok

    symbol do
      rule :Ident, [:PIdent, :tok]
      rule :Number, [:PNumber, :tok]
    end
  end
end