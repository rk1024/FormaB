% http://cs.union.edu/~striegnk/courses/nlp-with-prolog/html/leftcorner_recognizer.pl

% http://cs.union.edu/~striegnk/courses/nlp-with-prolog/html/node53.html

:- op(700, xfx, --->).

lc_recognize(Cat, [Word | StringIn], StringOut) :-
  lex(Word, WCat),
  complete(Cat, WCat, StringIn, StringOut).

complete(Cat, Cat, String, String).

complete(Cat, SubCat, StringIn, StringOut) :-
  LHS ---> [SubCat | Cats],
  matches(Cats, StringIn, String1),
  complete(Cat, LHS, String1, StringOut).

matches([], String, String).

matches([Cat | Cats], StringIn, StringOut) :-
  lc_recognize(Cat, StringIn, String1),
  matches(Cats, String1, StringOut).

lc_recognize(String) :- lc_recognize(s, String, []).
