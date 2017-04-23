import glob
import itertools as it
import logging as l
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


def main():
  build = conf.Build()

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
      "-O1",
      "-D_GLIBCXX_DEBUG",
      "-D_LIBCPP_DEBUG",
      "-DDEBUG",
      "-D_DEBUG",
    ])
  else:
    cxxflags.extend([
      "-Ofast",
      "-DNDEBUG",
      "-D_NDEBUG",
    ])

  l.debug(
    "CXX Flags: {}".format(
      " ".join(
        "'{}'".format(str(flag)) if " " in str(flag) else str(flag)
        for flag in cxxflags
      )
    )
  )
  l.debug(
    "LD  Flags: {}".format(
      " ".join(
        "'{}'".format(str(flag)) if " " in str(flag) else str(flag)
        for flag in ldflags
      )
    )
  )

  build.set(
    cxx = "clang++",
    flex = "flex",
    bison = "bison",
    cxxflags = " ".join(cxxflags),
    ldflags = " ".join(ldflags),
    flexflags = " ".join(flexflags),
    bisonflags = " ".join(bisonflags),
    srcdir = build.path("src"),
    bindir = build.path("bin"),
  )

  build.rule(
    "cxx", targets = (".o"), deps = (".cpp")
  ).set(
    command = "$cxx $cxxflags $flags -MMD -MF $out.d -c -o $out $in",
    deps = "gcc",
    depfile = "$out.d",
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

  build.edges(
    (build.path_b("flex-test.cpp"), "$srcdir/flex-test.lpp"),
    (([build.path_b("bison-test.cpp")], [build.path_b("bison-test.hpp")]),
     "$srcdir/bison-test.ypp"),
    ("parse-test", "phony", "$bindir/parse-test"),
  )

  sources = {
    "$bindir/parse-test": (
      list(it.chain(
        ["main.cpp"],
        [path.relpath(p, "src") for p in glob.iglob("src/ast/*.cpp")],
      )), ["flex-test.cpp", "bison-test.cpp"],
    )
  }

  for out, (ins, b_ins) in sources.iteritems():
    for n in ins:
      build.edge(
        build.path_b("{}.o".format(path.splitext(n)[0])),
        "$srcdir/{}".format(n)
      ).set(flags = " ".join(["-I$builddir -I$srcdir"]))

    for b in b_ins:
      build.edge(
        build.path_b("{}.o".format(path.splitext(b)[0])), build.path_b(b)
      ).set(flags = " ".join(["-I$builddir -I$srcdir"]))

    build.edge(
      out,
      build.paths_b(
        *["{}.o".format(path.splitext(n)[0]) for n in it.chain(ins, b_ins)]
      )
    )

  return build.run("", "build", *sys.argv[1:])


sys.exit(main())
