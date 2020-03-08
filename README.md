Zpr'(h
======
**Zpr'(h** [ʈʃpʁɪk] is an esoteric programming language designed and implemented by Jonathan Frech in the tenth calendar week of 2020. It focuses on byte-based syntactic symbol manipulation, keeping its core semantics to a minimum.


Overview
--------
**Zpr'(h** only knows of two key character sequences (`|>` and `<|`), parting characters (represented as bytes using standard ASCII) not pertaining to these into three classes; separators (`\x00`, `\x10`, `\x20`), parentheses (`(`, `)`) and all other characters.  
For convenience, character sequences starting with a semicolon and ending in a newline (`;...\n`) are treated as comments and ignored; a backslash followed by a newline (`\\\n`) continues the line it is on. Furthermore, all whitespace is de-duplicated.

A **Zpr'(h** source file consists of _rule definitions_ (`pat |> bdy`) and _inclusions_ (`<| stdlib/number-theory.zpr`). The latter includes any rules defined in a source file verbatim into the current source file. When it is executed, the initial _program text_ "`main`" is iteratively matched against all defined rules, prioritizing the rule which matches the earliest character and has been defined the earliest. Matching continues until no match can be performed anymore. This final program text residue is outputted to `stdout`.


Rules
-----
Modulo line continuation and de-duplicated whitespace, a rule has the form `pat |> bdy`, where its _pattern_ `pat` defines all characters that must match -- with `.point` defining a _point_, i.e. matching a parenthesized character sequence which can be referenced in its _body_ `bdy`. A rule's effect when chosen to be matched is replacing its pattern with its body in the current program text.


Examples
--------

### Basic matching
Using `./Zprh --watch-complete <source.zpr>`, one can view the entire matching process, i.e. the **Zpr'(h** program

    (! false)   |> true
    (prime? 55) |> false
    main        |> (! (prime? 55))

is interpreted (`make CFLAGS=-DMONOCHROME && ./Zprh --watch-complete stdlib/_examples/matching.zpr`) as

    [watch 0] main
    [watch 1] (! (prime? 55))
    [watch 2] (! false)
    [watch 3] true

and produces an output of `true`.

### Peano arithmetic
However, without any points, a program is destined to be of rather static nature. The following example introduces some _semantics_, that is a notion of _natural numbers_ using the Peano system; successors of numbers paired with a zero number.

Symbolically, a natural number will be _encoded_ by the byte sequence `(S (S (S ... (S ()) ... )))`. Using said encoding, one can define a symbol for _addition_, _multiplication_ and the _factorial_ of a natural number:

    (.n + ())     |> n
    (.n + (S .m)) |> ((S n) + m)

    (.n * ())     |> ()
    (.n * (S .m)) |> ((n * m) + n)

    (() !)     |> (S ())
    ((S .n) !) |> ((S n) * (n !))

    main |> ((S (S (S (S ())))) !)

Executing the above (`./Zprh stdlib/_examples/peano.zpr`), `stdout` receives

    (S (S (S (S (S (S (S (S (S (S (S (S (S (S (S (S (S (S (S (S (S (S (S (S ()))))))))))))))))))))))))

which is `24` encoded using the above scheme. For better readability, the flag `--de-peano` matches byte sequences that look like they belong to an encoded natural number into its decimal form; `./Zprh --de-peano stdlib/_examples/peano.zpr` produces the output `24`.

### Basic number theory
Further well-known operators and functions are implemented in `stdlib/arithmetic.zpr` and `stdlib/number-theory.zpr`. Importing these, one can for example calculate the first sixteen primes using **Zpr'(h**:

    <| ../number-theory.zpr

    main |> (take 16 primes)

produces (`./Zprh --de-peano stdlib/_examples/basic-number-theory.zpr`) the following output:

    (' 2 (' 3 (' 5 (' 7 (' 11 (' 13 (' 17 (' 19 (' 23 (' 29 (' 31 (' 37 (' 41 (' 43 (' 47 (' 53 0))))))))))))))))
