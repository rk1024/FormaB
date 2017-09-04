# Forma Intermedia

(Latin for *intermediate form*)

Intermedia is the IR for the Forma compiler.  It serves as both the representation Praeforma is compiled to and the base structure used by Forma's syntax extensions.

## Variables

Intermedia uses SSA variable assignment.  This means that all variables are constant, and rather than reassigning a variable, a new iteration of the variable is bound and assigned.

## Blocks

Blocks are used to scope variables.  A block consists of a `BlockStart` opcode (represented as `{`), the block contents, and finally a `BlockEnd` opcode (represented as `}`).

```.
{
  ...
}
```

## Message Passing