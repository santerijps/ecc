# ´ecc´ - Easy Compiler & Compressor

`ecc` is an easy to use utility that makes compiling and running C programs easy, while also providing executable compression.

## Usage

```txt

Usage: ecc [generic options] [build options] [...c source files]

This program depends on:
  - Compilers:   gcc (and optionally, clang)
  - Compressor:  upx

Generic options:
  --help, -h             Show app usage.
  --version, -v          Show app version.

Build options:
  --compiler, -c         Which compiler to use. (Default: gcc, supports clang as well)
  --out, -o              Output executable name. (Default: out.exe)
  --std                  Which C standard version to use. (Default: c2x)
  --define, -D           Define a macro statement at compile time.
  --include, -I          Add include path.
  --libs, -L             Add lib path. Found DLLs are automatically included in the compilation.
  --quiet, -q            Only print errors.
  --strict               Compilation fails in case of warnings. (Warnings are treated as errors).
  --fast                 Compiles with the -Ofast GCC flag. (Default: -Oz)
  --compress             Compresses the compiled executable with upx.
  --run, -r              Runs the executable after compiling.

Examples:
  Compile current project:       ecc
  Compile a specific file(s):    ecc main.c lib.c
  Compile file and run:          ecc test.c -r
  Compile and run with args:     ecc sum.c -r -- 1 2 3
  Compile with options:          ecc --compiler clang -o app.exe
  Compile with defines:          ecc -D 'x=1' --define 'NAME="Alice"'

```

## Feature ideas

### Add `--root`/`-R` option

This is to avoid situations like:

```sh
ecc src/main.c -I src -L src
```

... where all the source files, includes and DLLs are in the `src` directory. Currently, we must specify the `src` dir three times. The new flag would enable the user to simply provide a "root" directory, from which the files will be looked up.

### Add code formatiing with `clang-format`

Self explanatory. This would add another dependency, but for my personal use it's ok. Code could optionally be formatted on every compilation.
