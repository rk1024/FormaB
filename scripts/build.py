#!/usr/bin/python

from __future__ import absolute_import
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

  afl = False # Compile with afl-fuzz instrumentation
  debug = True or afl # Compile debuggable binaries
  diagnostic = False # Add extra diagnostic warnings
  sanitize = True and not afl # Compile with sanitizers enabled
  optimize = False # Perform optimizations
  dashOFast = False and not afl # Use -Ofast

  astgenFlags = [
    "-f$rootdir/src/scanner.in.lpp:$builddir/scanner.lpp",
    "-b$rootdir/src/parser.in.ypp:$builddir/parser.ypp",
    "-t$rootdir/src/ast/token.in.hpp:$builddir/ast/token.hpp",
  ]

  flexflags = [
    "--ecs",
    "--full",
  ]

  bisonflags = [
    "--warnings=all",
    "--report=all",
  ]

  re2cflags = [
    "--empty-class error",
    "-W",
    "-Werror-undefined-control-flow",
  ]

  cxxflags = [
    "-fcolor-diagnostics",
    "-std=c++14",
    "-Wall",
    "-Wconversion",
    "-Wdeprecated",
    "-Wextra",
    "-Wimplicit",
    "-Wimplicit-fallthrough",
    "-Winvalid-noreturn",
    "-Wmissing-noreturn",
    "-Wmissing-prototypes",
    "-Wmissing-variable-declarations",
    "-Wnewline-eof",
    "-Wshadow",
    "-Wno-logical-op-parentheses",
    "-Wno-shorten-64-to-32",
    "-Wno-sign-compare",
    "-Wno-sign-conversion",
    "-Wtautological-compare",
    "-Wthread-safety",
    "-Wunreachable-code-aggressive",
    "-Wunused",
    "-Werror=old-style-cast",
    "-Werror=return-type",
  ]

  ldflags = []

  def addPkgs(*args):
    cflags = pkcflags(*args)
    cxxflags.extend(cflags)
    cxxflags.extend([re.sub("^-I", "-isystem ", flag) for flag in cflags])
    ldflags.extend(pklibs(*args))

  if diagnostic:
    cxxflags.extend([
      "-fdiagnostics-show-template-tree",
      "-Wswitch-bool",
      "-Wswitch-enum",
      "-Wweak-template-vtables",
      "-Wweak-vtables",

      # Anti-spam measures:
      "-fno-caret-diagnostics",
      "-fno-diagnostics-fixit-info",
    ])
  else:
    cxxflags.extend([
      # Disabled by diagnostic as anti-spam measure:
      "-fno-elide-type",
      "-ftemplate-backtrace-limit=0",
    ])

  if sanitize:
    sanflags = ["-fsanitize=%s" % (san) for san in [
      "address",
      "undefined",
    ]]

    cxxflags.extend(sanflags)
    ldflags.extend(sanflags)

  if debug:
    cxxflags.extend([
      "-g",
      "-O0",
      "-D_GLIBCXX_DEBUG",
      "-D_GLIBCXX_DEBUG_PEDANTIC",
      "-D_LIBCPP_DEBUG",
      "-DDEBUG",
      "-D_DEBUG",
    ])

    astgenFlags.extend([
      "-v",
    ])
  else:
    cxxflags.extend([
      "-DNDEBUG",
      "-D_NDEBUG",
    ])

  if optimize:
    cxxflags.extend([
      "-Ofast" if dashOFast else "-O3",
    ])

  astgenFlagsInternal = [
    flag.replace("$rootdir", ".").replace("$builddir", "build")
    for flag in astgenFlags
  ]

  build.set(
    srcdir = build.path("src"),
    bindir = build.path("bin"),
    ruby = "ruby",
    flex = "flex",
    bison = "scripts/run-bison.sh bison",
    re2c = "$rootdir/tools/re2c/re2c/re2c",
    cxx = "afl-clang++" if afl else "clang++",
    astgenFlags = " ".join(astgenFlags),
    flexflags = " ".join(flexflags),
    bisonflags = " ".join(bisonflags),
    re2cflags = " ".join(re2cflags),
    cxxflags = " ".join(cxxflags),
    ldflags = " ".join(ldflags),
  )

  build.rule("ruby").set(
    command = "$ruby $rubyflags $flags $in $args",
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

  build.rule(
    "re2c", targets = (".cpp"), deps = (".re")
  ).set(
    command = "$re2c $re2cflags $flags -o $out $in",
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

  build.edge(build.path_b("scanner.cpp"), build.path_b("scanner.lpp")).set(
    description = "flex scanner.lpp",
  )

  build.edge(([build.path_b("parser.cpp")], build.paths_b(
    *["parser.hpp", "parser.output", "location.hh", "position.hh", "stack.hh"]
  )), build.path_b("parser.ypp")).set(
    description = "bison parser.ypp",
  )

  build.edge("formab", "phony", "$bindir/formab", True).set(
    description = "BUILD formab",
  )

  # Explicit dependencies created by ASTGen
  astSources = [
    path.relpath(src.strip(), "build")
    for src in str(
      sp.check_output(
        flatten(["ruby", "scripts/ast.rb", "-l", "build/ast"],
                astgenFlagsInternal)
      )
    ).split(" ")
  ]

  # Implicit dependencies created by ASTGen
  astImplSources = [
    path.relpath(src.strip(), "build")
    for src in str(
      sp.check_output(
        flatten(["ruby", "scripts/ast.rb", "-i", "build/ast"],
                astgenFlagsInternal)
      )
    ).split(" ")
  ]

  # "binary": (
  #   [source files in src/ directory],
  #   [.re files in src/ directory],
  #   [ {
  #     <source file in build/> |
  #     ([source files in build/], [cxx flags])
  #   } ],
  #   [extra .o files],
  #   [include path]
  # )
  sources = {
    "formab": (
      #src/...
      flatten([
        "formab.cpp",
        "ast/astBase.cpp",
        "ast/token.cpp",
      ], *[
        rglob("src/{}".format(folder), "*.cpp", rel = "src/")
        for folder in [
          "intermedia",
          "util",
        ]
      ]),
      #src/... .re
      flatten(
        *[
          rglob("src/{}".format(folder), "*.re", rel = "src/")
          for folder in [
            "intermedia",
            "util",
          ]
        ]
      ),
      #build/...
      flatten(
        [
          "scanner.cpp",
          ([
            "parser.cpp",
          ], [
            "-Wno-deprecated",
            "-Wno-weak-vtables",
          ]),
        ],
        fnmatch.filter(astSources, "**/*.cpp"),
      ),
      #objs
      [],
      #include
      "",
    ),
  }

  l.debug(sources)

  build.edge(
    (build.paths_b(*astSources), build.paths_b(*astImplSources)),
    "ruby",
    ([
      build.path("scripts/ast.rb"),
    ], flatten(
      [
        "$srcdir/parser.in.ypp",
        "$srcdir/scanner.in.lpp",
        "$srcdir/ast/token.in.hpp",
      ],
      build.paths(*rglob("scripts/astgen", "*.rb")),
    )),
  ).set(
    description = "ASTGen scripts/ast.rb",
    args = "$astgenFlags $rootdir/build/ast",
    restat = "true",
  )

  for out, (ins, re_ins, b_ins, objs, p) in sources.items():
    _p = p
    p = p.split("/") if len(p) else []

    includes = "-I{} -I{}{}".format(
      path.join("$builddir", *p),
      path.join("$srcdir", *p), " -I$builddir -I$srcdir" if len(p) else ""
    )

    for r in re_ins:
      build.edge(build.path_b("{}.cpp".format(r)), "$srcdir/{}".format(r)).set(
        description = "re2c {}".format(r),
      )

      build.edge(
        build.path_b("{}.o".format(r)), build.path_b("{}.cpp".format(r))
      ).set(
        description = "compile {}.cpp".format(r),
        flags = "{}{}".format(
          includes, " -I$srcdir/{}".format(path.dirname(r))
          if len(path.dirname(r)) else ""
        )
      )

    for n in ins:
      build.edge(
        build.path_b("{}.o".format(path.splitext(n)[0])),
        "$srcdir/{}".format(n)
      ).set(
        description = "compile {}".format(n),
        flags = includes,
      )

    b_files = []

    for b in b_ins:
      if isinstance(b, tuple):
        files, flags = b
        b_files.extend(files)
        for f in files:
          build.edge(
            build.path_b("{}.o".format(path.splitext(f)[0])), build.path_b(f)
          ).set(
            description = "compile {}".format(f),
            flags = "{} {}".format(" ".join(flags), includes),
          )
      else:
        b_files.append(b)
        build.edge(
          build.path_b("{}.o".format(path.splitext(b)[0])), build.path_b(b)
        ).set(
          description = "compile {}".format(b),
          flags = includes,
        )

    build.edge(
      path.join("$bindir", out),
      build.paths_b(
        *flatten(
          ["{}.o".format(path.splitext(n)[0]) for n in it.chain(ins, b_files)],
          ["{}.o".format(n) for n in it.chain(re_ins)],
          objs,
        )
      )
    ).set(
      description = "link {}".format(path.basename(out)),
    )

  return build.run(".", "build", *sys.argv[1:])


sys.exit(main())
