# MetaForma Grammar Notes

## Statement and Expression

The two basic units of the grammar are `Statement` and `Expression`.


* `Statement` shall be one of the following:
  * A simple expression, followed by a semicolon
  * A simple expression that does not need a semicolon
  * A complex expression, using `Statement` recursively
* `Expression` shall be one of the following:
  * A simple expression
  * A complex expression, using `Expression` recursively

The types of simple and compound expressions comprising `Statement` and
`Expression` should be the same &mdash; the only reason `Statement` and
`Expression` are separate is to allow or disallow the use of semicolons in
various parts of the grammar.

### Complex Statements and Expressions

The compound form of `Statement`, referred to as `Statements`, is designed to be
similar to a C-like statement list.  Because `Statement` is already
semicolon-terminated where necessary, `Statements` is simply defined as:

```
Statements:
  Statements Statement
  Statement
```

The compound form of `Expression`, referred to as `Expressions`, is a
comma-separated list of expressions, with an optional trailing comma (which is
useful for cases where each item is on its own line).  `Expressions` is defined
as:

```
ExpressionsPart:
  Expressions "," Expression
  Expression

Expressions:
  ExpressionsPart ","
  ExpressionsPart
```

Both of these compound forms by themselves match a minimum of one statement or
expression.  For cases where an empty list should also be matched, the following
optional forms are defined:

```
StatementsOpt:
  Statements
  %empty

ExpressionsOpt:
  Expressions
  ","
  %empty
```

## Types of Expression

### Value Expressions

* `Literal`

### Complex Value Expressions

* `Member`
* `Call`
* `Index`

#### `Member`

```
MemberLHS:
  Identifier

MemberRHS:
  Identifier

MemberExpression:
  MemberLHS "." MemberRHS
```

#### `Call`

```
CallRHS:
  "(" ArgumentListOpt ")"

CallExpression:
  MemberLHS CallRHS
```

#### `Index`

```
IndexRHS:
  "[" ArgumentList "]"

IndexExpression:
  MemberLHS IndexRHS
```

### Value Assignment

```
AssignLHS:
  MemberExpression
  CallExpression
  IndexExpression

AssignRHS:
  RValue
  AssignExpression

RValue: ...good question.

AssignOp - any of:
  "=" "||=" "&&=" "|=" "&=" "^=" "+=" "-=" "*=" "/=" "%="

AssignExpression:
  AssignLHS AssignOp AssignRHS
```

###  Chaining Calls/Assignments

* `Cascade`
  * `Member`: ex. `foo..bar`
  * `Call`: ex. `foo..()`
  * `Index`: ex. `foo..[0]`
  * `Assignment`: ex. `foo..bar = 5..() = 5..[0] = 5`

```
CascadeLHS:
  MemberLHS
  CascadeExpression

CascadeRHS:
  MemberRHS
  CallRHS
  IndexRHS

CascadeExpression:
  CascadeLHS ".." CascadeRHS
```

### Conditional Expressions

* `If` / `Unless`
* `IfElse` / `UnlessElse`

```
LetCondition:
  LetExpression ";" Expression

Condition:
  Expression
  LetCondition

ParenCondition:
  "(" Condition ")"

IfExpression:
  "if" ParenCondition Expression

IfElseExpression:
  "if" ParenCondition Expression "else" Expression

UnlessExpression:
  "unless" ParenCondition Expression

UnlessElseExpression:
  "unless" ParenCondition Expression "else" Expression
```

### Loop Expressions

* `Loop`
* `While` / `Until`
* `DoWhile` / `DoUntil`
* `For`

```
LoopExpression:
  "loop" Statement

WhilePreamble:
  "while" ParenCondition

UntilPreamble:
  "until" ParenCondition

ForPreamble:
  "for" "(" LetCondition ";" Expression ")"

DoPreamble:
  "do"
  "do" "(" LetExpression ")"

DoExpression:
  DoPreamble Expression

WhileExpression:
  WhilePreamble Expression

DoWhileExpression:
  DoExpression WhilePreamble

UntilExpression:
  UntilPreamble Expression

DoUntilExpression:
  DoExpression UntilPreamble

ForExpression:
  ForPreamble Expression
```

### Error Handling

* `Try`
* `Catch`
* `Else`
  * *`try...else try` chaining?*
* `Finally`

### Pattern Matching

* `Switch`

### Object Expressions

* `Struct`
* `Function`
* `Record`
