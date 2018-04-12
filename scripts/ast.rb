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

    ctor :Fragment, :tok, fmt: [:tok]

    symbol do
      rule :Fragment, [:Fragment, :tok]
    end
  end
end