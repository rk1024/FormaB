import fnmatch
import glob
import itertools as it
import logging as l
import os
import subprocess as sp
import sys
from os import path

import NinjaSnek.configure as conf

l.basicConfig(level = l.DEBUG)


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
    "-f$rootdir/src/flex-test.in.lpp:$builddir/flex-test.lpp",
    "-b$rootdir/src/bison-test.in.ypp:$builddir/bison-test.ypp",
    "-t$rootdir/src/ast/token.in.hpp:$builddir/ast/token.hpp",
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
    (build.path_b("flex-test.cpp"), "$builddir/flex-test.lpp"),
    (([build.path_b("bison-test.cpp")], build.paths_b(
      *[
        "bison-test.hpp", "bison-test.output", "location.hh", "position.hh",
        "stack.hh"
      ]
    )), "$builddir/bison-test.ypp"),
    ("parse-test", "phony", "$bindir/parse-test"),
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

  astImplSources = [
    path.relpath(src.strip(), "build")
    for src in str(
      sp.check_output(
        flatten(["ruby", "scripts/ast.rb", "-i", "build/ast"],
                astgenFlagsInternal)
      )
    ).split(" ")
  ]

  sources = {
    "$bindir/parse-test": ([], flatten(
      ["flex-test.cpp", "bison-test.cpp"],
      fnmatch.filter(astSources, "**/*.cpp"),
    ), ["parseTest.o", "ast/token.o"])
  }

  for args in [
    (
      build.path_b("parseTest.o"),
      (["$srcdir/parseTest.cpp"], ["$builddir/bison-test.hpp"])
    ),
    (
      build.path_b("ast/token.o"),
      (["$srcdir/ast/token.cpp"], ["$builddir/ast/token.hpp"])
    ),
  ]:
    build.edge(*args).set(flags = "-I$builddir -I$srcdir")

  build.edge((build.paths_b(*astSources), build.paths_b(*astImplSources)),
             "ruby", ([build.path("scripts/ast.rb")], flatten([
               "$srcdir/bison-test.in.ypp",
               "$srcdir/flex-test.in.lpp",
               "$srcdir/ast/token.in.hpp",
             ], build.paths(*rglob("scripts/astgen", "*.rb"))))).set(
               description = "ASTGen",
               args = "$astgenFlags $rootdir/build/ast",
               restat = "true",
             )

  for out, (ins, b_ins, objs) in sources.iteritems():
    for n in ins:
      build.edge(
        build.path_b("{}.o".format(path.splitext(n)[0])),
        "$srcdir/{}".format(n)
      ).set(flags = "-I$builddir -I$srcdir")

    for b in b_ins:
      build.edge(
        build.path_b("{}.o".format(path.splitext(b)[0])), build.path_b(b)
      ).set(flags = "-I$builddir -I$srcdir")

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
