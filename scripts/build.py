import fnmatch
import glob
import itertools as it
import logging as l
import os
import subprocess as sp
import sys
from os import path

import NinjaSnek.configure as conf

l.basicConfig(level = l.INFO)


def pkgConfig(*args):
  pinf = ["pkg-config"]
  pinf.extend(args)

  p = sp.Popen(pinf, stdout = sp.PIPE)

  o, _ = p.communicate()

  r = p.wait()

  if r: raise RuntimeError("pkg-config exited with code {}.".format(r))

  return [flag.strip() for flag in str(o).split(" ")]


def pkcflags(*args):
  return pkgConfig("--cflags", *args)


def pklibs(*args):
  return pkgConfig("--libs", *args)


def flatten(*args):
  return list(it.chain(*args))


def rglob(base, pattern, rel = None):
  def _rglob(base, pattern):
    return [
      path.join(root, f)
      for root, _, files in os.walk(base)
      for f in fnmatch.filter(files, pattern)
    ]

  if rel is None: return _rglob(base, pattern)

  return [path.relpath(f, rel) for f in _rglob(base, pattern)]


def main():
  build = conf.Build()

  build.useRepo("https://www.github.com/rookie1024/ninja")

  debug = True

  cxxflags = [
    "-fcolor-diagnostics",
    "-fno-elide-type",
    "-std=c++14",
    "-Wall",
    "-Wconversion",
    "-Wdeprecated",
    "-Wextra",
    "-Wimplicit",
    "-Winvalid-noreturn",
    "-Wmissing-noreturn",
    "-Wmissing-prototypes",
    "-Wmissing-variable-declarations",
    "-Wnewline-eof",
    "-Wshadow",
    "-Wno-shorten-64-to-32",
    "-Wno-sign-compare",
    "-Wno-sign-conversion",
    "-Wthread-safety",
    "-Wunreachable-code-aggressive",
    "-Wunused",
    "-Werror=old-style-cast",
    "-Werror=return-type",
  ]

  ldflags = []

  flexflags = [
    "--ecs",
    "--full",
  ]

  bisonflags = [
    "--warnings=all",
    "--report=all",
  ]

  astgenFlags = [
    "-f$rootdir/src/scanner.in.lpp:$builddir/scanner.lpp",
    "-b$rootdir/src/parser.in.ypp:$builddir/parser.ypp",
    "-t$rootdir/src/ast/token.in.hpp:$builddir/ast/token.hpp",
  ]

  astgenTestFlags = [
    "-f$rootdir/src/parse-test/flex-test.in.lpp:$builddir/parse-test/flex-test.lpp",
    "-b$rootdir/src/parse-test/bison-test.in.ypp:$builddir/parse-test/bison-test.ypp",
    "-t$rootdir/src/ast/token.in.hpp:$builddir/parse-test/ast/token.hpp",
  ]

  def addPkgs(*args):
    cxxflags.extend(pkcflags(*args))
    ldflags.extend(pklibs(*args))

  if debug:
    sanflags = ["-fsanitize=%s" % (san) for san in [
      "address",
      "undefined",
    ]]

    cxxflags.extend(sanflags)
    ldflags.extend(sanflags)

    cxxflags.extend([
      "-g",
      "-O0",
      "-D_GLIBCXX_DEBUG",
      "-D_LIBCPP_DEBUG",
      "-DDEBUG",
      "-D_DEBUG",
    ])

    astgenFlags.extend([
      "-v",
    ])
  else:
    cxxflags.extend([
      "-Ofast",
      "-DNDEBUG",
      "-D_NDEBUG",
    ])

  astgenFlagsInternal = [
    flag.replace("$rootdir", ".").replace("$builddir", "build")
    for flag in astgenFlags
  ]

  astgenTestFlagsInternal = [
    flag.replace("$rootdir", ".").replace("$builddir", "build")
    for flag in astgenTestFlags
  ]

  build.set(
    srcdir = build.path("src"),
    bindir = build.path("bin"),
    cxx = "clang++",
    flex = "flex",
    bison = "bison",
    ruby = "ruby",
    cxxflags = " ".join(cxxflags),
    ldflags = " ".join(ldflags),
    flexflags = " ".join(flexflags),
    bisonflags = " ".join(bisonflags),
    astgenFlags = " ".join(astgenFlags),
    astgenTestFlags = " ".join(astgenTestFlags),
  )

  build.rule(
    "cxx", targets = (".o"), deps = (".cpp")
  ).set(
    command = "$cxx $cxxflags $flags -MMD -MF $out.d -c -o $out $in",
    deps = "gcc",
    depfile = "$out.d",
    restat = "true"
  )

  build.rule(
    "link", targets = (""), deps = (".o")
  ).set(
    command = "$cxx $ldflags $flags -o $out $in",
  )

  build.rule(
    "flex", targets = (".cpp"), deps = (".lpp")
  ).set(
    command = "$flex $flexflags $flags -o $out $in",
  )

  build.rule(
    "bison", targets = (".cpp"), deps = (".ypp")
  ).set(
    command = "$bison $bisonflags $flags -o $out $in",
  )

  build.rule("ruby").set(
    command = "$ruby $rubyflags $flags $in $args",
  )

  build.edges(
    (build.path_b("scanner.cpp"), build.path_b("scanner.lpp")),
    (([build.path_b("parser.cpp")], build.paths_b(
      *[
        "parser.hpp", "parser.output", "location.hh", "position.hh",
        "stack.hh"
      ]
    )), build.path_b("parser.ypp")),
    (build.path_b("parse-test/flex-test.cpp"), "$builddir/parse-test/flex-test.lpp"),
    (([build.path_b("parse-test/bison-test.cpp")], build.paths_b(
      *[
        "parse-test/{}".format(s)
        for s in [
          "bison-test.hpp", "bison-test.output", "location.hh", "position.hh",
          "stack.hh"
        ]
      ]
    )), "$builddir/parse-test/bison-test.ypp"),
    ("formab", "phony", "$bindir/formab", True),
    ("parse-test", "phony", "$bindir/parse-test", True),
  )

  build.util("ast-order", "ruby", build.path("scripts/ast.rb")).set(args = "-r")

  astSources = [
    path.relpath(src.strip(), "build")
    for src in str(
      sp.check_output(
        flatten(["ruby", "scripts/ast.rb", "-l", "build/ast"],
                astgenFlagsInternal)
      )
    ).split(" ")
  ]

  astTestSources = [
    path.relpath(src.strip(), "build")
    for src in str(
      sp.check_output(
        flatten(["ruby", "scripts/ast-test.rb", "-l", "build/parse-test/ast"],
                astgenTestFlagsInternal)
      )
    ).split(" ")
  ]

  astImplSources = [
    path.relpath(src.strip(), "build")
    for src in str(
      sp.check_output(
        flatten(["ruby", "scripts/ast.rb", "-i", "build/ast"],
                astgenFlagsInternal)
      )
    ).split(" ")
  ]

  astTestImplSources = [
    path.relpath(src.strip(), "build")
    for src in str(
      sp.check_output(
        flatten(["ruby", "scripts/ast-test.rb", "-i", "build/parse-test/ast"],
                astgenTestFlagsInternal)
      )
    ).split(" ")
  ]

  sources = {
    "$bindir/formab": ([], flatten(
      ["scanner.cpp", "parser.cpp"], "",
      fnmatch.filter(astSources, "**/*.cpp"),
    ), ["formab.o", "ast/token.o"], []),
    "$bindir/parse-test": ([], flatten(
      ["parse-test/flex-test.cpp", "parse-test/bison-test.cpp"],
      fnmatch.filter(astTestSources, "**/*.cpp"),
    ), ["parse-test/parseTest.o", "parse-test/ast/token.o"], ["parse-test"])
  }

  for args, p in [
    ((
      build.path_b("formab.o"),
      (["$srcdir/formab.cpp"], ["$builddir/parser.hpp"])
    ), []),
    ((
      build.path_b("parse-test/parseTest.o"),
      (["$srcdir/parse-test/parseTest.cpp"], ["$builddir/parse-test/bison-test.hpp"])
    ), ["parse-test"]),
    ((
      build.path_b("ast/token.o"),
      (["$srcdir/ast/token.cpp"], ["$builddir/ast/token.hpp"])
    ), []),
    ((
      build.path_b("parse-test/ast/token.o"),
      (["$srcdir/ast/token.cpp"], ["$builddir/parse-test/ast/token.hpp"])
    ), ["parse-test"]),
  ]:
    includes = "-I{} -I{}{}".format(path.join("$builddir", *p), path.join("$srcdir", *p), " -I$builddir -I$srcdir" if len(p) else "")
    build.edge(*args).set(flags = includes)

  build.edge((build.paths_b(*astSources), build.paths_b(*astImplSources)),
             "ruby", ([build.path("scripts/ast.rb")], flatten([
               "$srcdir/parser.in.ypp",
               "$srcdir/scanner.in.lpp",
               "$srcdir/ast/token.in.hpp",
             ], build.paths(*rglob("scripts/astgen", "*.rb"))))).set(
               description = "ASTGen",
               args = "$astgenFlags $rootdir/build/ast",
               restat = "true",
             )

  build.edge((build.paths_b(*astTestSources), build.paths_b(*astTestImplSources)),
             "ruby", ([build.path("scripts/ast-test.rb")], flatten([
               "$srcdir/parse-test/bison-test.in.ypp",
               "$srcdir/parse-test/flex-test.in.lpp",
               "$srcdir/ast/token.in.hpp",
             ], build.paths(*rglob("scripts/astgen", "*.rb"))))).set(
               description = "ASTGen",
               args = "$astgenTestFlags $rootdir/build/parse-test/ast",
               restat = "true",
             )

  for out, (ins, b_ins, objs, p) in sources.iteritems():
    includes = "-I{} -I{}{}".format(path.join("$builddir", *p), path.join("$srcdir", *p), " -I$builddir -I$srcdir" if len(p) else "")

    for n in ins:
      build.edge(
        build.path_b("{}.o".format(path.splitext(n)[0])),
        "$srcdir/{}".format(n)
      ).set(flags = includes)

    for b in b_ins:
      build.edge(
        build.path_b("{}.o".format(path.splitext(b)[0])), build.path_b(b)
      ).set(flags = includes)

    build.edge(
      out,
      build.paths_b(
        *[
          "{}.o".format(path.splitext(n)[0])
          for n in it.chain(ins, b_ins, objs)
        ]
      )
    )

  return build.run(".", "build", *sys.argv[1:])


sys.exit(main())
