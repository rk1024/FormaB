# Forma Roadmap

* [Functions and messages](#functions-and-messages)
* [Function call syntax](#functions-call-syntax)
* [Currying, pattern matching, and overloading](#currying-pattern-matching-and-overloading)
* [Intermedia](#intermedia)
* [Syntax extensions](#syntax-extensions)

## <a name="functions-and-messages"></a> Functions and Messages

By using keywords, Smalltalk-style messages are a far more accessible interface to subroutines than functions.  Consider the following code in Smalltalk:

```smalltalk
  "cat" replace: "c" with: "s"
```

This is far more intuitive than the same code as it may appear in another language (here, I chose Ruby):

```ruby
  "cat".gsub("c", "s")
```

However, the way Smalltalk handles objects because of messages is fundamentally different from other languages in that there is no way to represent a function by itself, as there is no way to "call" an object - objects may only be passed messages, and passing messages is the only way to run subroutines:

```smalltalk
  modf of: 5 and: 3
```

This contrasts greatly with other languages, where a standalone function can be called the same way a member can be invoked on an object:

```ruby
  modf(5, 3)
```

As the above example shows, when using messages functions can only be represented by themselves as objects which accept a single kind of message.

## <a name="function-call-syntax"></a> Function Call Syntax

Here are some different syntaxes for invoking a member of (or passing a method to) an object:

*(`Identifier` is a single word and `Keyword` is an `Identifier` followed immediately by a colon)*

**C-style delimiters:**

```.
  Expression "(" Arguments ")"
```

**Delimiter-free:**

```.
  Expression Arguments
```

**ObjC-style delimiters:**

```.
  "[" Expression Arguments "]"
```

**Modified ObjC-style delimiters:**

```.
  "[" Expression Arguments [ "|" Arguments ] * "]"
```

### Chaining



### Technical Considerations

<!-- As far as technical considerations are concerned, the first and third forms are easy to implement without causing any grammatical conflicts, due to the fact that each has a delimiter on at least one end of the expression.  However, the Objective-C form -->

## <a name="currying-pattern-matching-and-overloading"></a> Currying, Pattern Matching, and Overloading

## <a name="intermedia"></a> Intermedia

## <a name="syntax-extensions"></a> Syntax Extensions
