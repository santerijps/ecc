#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#ifndef VERSION
#define VERSION "1.0.0"
#endif

#define MAX_SOURCE_FILE_COUNT   64
#define MAX_DEFINE_COUNT        32
#define MAX_INCLUDE_COUNT       16
#define MAX_LIB_COUNT           16

int main(int argc, char **argv) {

#pragma region Option Declarations & Initializations
  char *out_file = "out.exe";
  char *source_files[MAX_SOURCE_FILE_COUNT] = {0};
  unsigned char source_file_count = 0;

  // -- ...args
  char *run_args[32] = {0};
  unsigned char run_arg_count = 0;
  bool read_run_args = false;

  // --define, -D
  char *defines[MAX_DEFINE_COUNT] = {0};
  unsigned char define_count = 0;

  // --include, -I
  char *includes[MAX_INCLUDE_COUNT] = {0};
  unsigned char include_count = 0;

  // --libs, -L
  char *libs[MAX_LIB_COUNT] = {"."};
  unsigned char lib_count = 1;

  char *compiler = "gcc";
  char *standard = "c2x";
  bool quiet = false;
  bool strict = false;
  bool fast = false;
  bool compress = false;
  bool run = false;
#pragma endregion

#pragma region Read Command-Line Arguments
    for (int i = 1; i < argc; i++) {
      if (read_run_args) {
        run_args[run_arg_count] = argv[i];
        run_arg_count += 1;
        continue;
      }
      if (!strcmp(argv[i], "--help") || !strcmp(argv[i], "-h")) {
        fprintf(
          stderr,
          "\n"
          "Usage: ecc [generic options] [build options] [...c source files]\n"
          "\n"
          "This program depends on:\n"
          "  - Compilers:   gcc (and optionally, clang)\n"
          "  - Compressor:  upx\n"
          "\n"
          "Generic options:\n"
          "  --help, -h             Show app usage.\n"
          "  --version, -v          Show app version.\n"
          "\n"
          "Build options:\n"
          "  --compiler, -c         Which compiler to use. (Default: gcc, supports clang as well)\n"
          "  --out, -o              Output executable name. (Default: out.exe)\n"
          "  --std                  Which C standard version to use. (Default: c2x)\n"
          "  --define, -D           Define a macro statement at compile time.\n"
          "  --include, -I          Add include path.\n"
          "  --libs, -L             Add lib path. Found DLLs are automatically included in the compilation.\n"
          "  --quiet, -q            Only print errors.\n"
          "  --strict               Compilation fails in case of warnings. (Warnings are treated as errors).\n"
          "  --fast                 Compiles with the -Ofast GCC flag. (Default: -Oz)\n"
          "  --compress             Compresses the compiled executable with upx.\n"
          "  --run, -r              Runs the executable after compiling.\n"
          "\n"
          "Examples:\n"
          "  Compile current project:       ecc\n"
          "  Compile a specific file(s):    ecc main.c lib.c\n"
          "  Compile file and run:          ecc test.c -r\n"
          "  Compile and run with args:     ecc sum.c -r -- 1 2 3\n"
          "  Compile with options:          ecc --compiler clang -o app.exe\n"
          "  Compile with defines:          ecc -D 'x=1' --define 'NAME=\"Alice\"'\n"
          "\n"
        );
        return 0;
      }
      if (!strcmp(argv[i], "--version") || !strcmp(argv[i], "-v")) {
        fprintf(stderr, VERSION "\n");
        return 0;
      } else if (!strcmp(argv[i], "--quiet") || !strcmp(argv[i], "-q")) {
        quiet = true;
      } else if ((!strcmp(argv[i], "--out") || !strcmp(argv[i], "-o")) && i + 1 < argc) {
        i += 1;
        strcpy(out_file, argv[i]);
      } else if (!strcmp(argv[i], "--strict")) {
        strict = true;
      } else if (!strcmp(argv[i], "--run") || !strcmp(argv[i], "-r")) {
        run = true;
      } else if ((!strcmp(argv[i], "--compiler") || !strcmp(argv[i], "-c")) && i + 1 < argc) {
        i += 1;
        compiler = argv[i];
      } else if ((!strcmp(argv[i], "--define") || !strcmp(argv[i], "-D")) && i + 1 < argc) {
        i += 1;
        if (define_count < MAX_DEFINE_COUNT) {
          defines[define_count] = argv[i];
          define_count += 1;
        } else {
          fprintf(stderr, "Ignoring include in compilation: %s\n", argv[i]);
        }
      } else if ((!strcmp(argv[i], "--include") || !strcmp(argv[i], "-I")) && i + 1 < argc) {
        i += 1;
        if (include_count < MAX_INCLUDE_COUNT) {
          includes[include_count] = argv[i];
          include_count += 1;
        } else {
          fprintf(stderr, "Ignoring include in compilation: %s\n", argv[i]);
        }
      } else if ((!strcmp(argv[i], "--libs") || !strcmp(argv[i], "-L")) && i + 1 < argc) {
        i += 1;
        if (lib_count < MAX_LIB_COUNT) {
          libs[lib_count] = argv[i];
          lib_count += 1;
        } else {
          fprintf(stderr, "Ignoring lib in compilation: %s\n", argv[i]);
        }
      } else if (!strcmp(argv[i], "--std") && i + 1 < argc) {
        i += 1;
        standard = argv[i];
      } else if (!strcmp(argv[i], "--fast")) {
        fast = true;
      } else if (!strcmp(argv[i], "--compress")) {
        compress = true;
      } else if (strlen(argv[i]) == 2 && argv[i][0] == '-' && argv[i][1] == '-') {
        read_run_args = true;
      } else if (strlen(argv[i]) > 1 && argv[i][0] == '-') {
        fprintf(stderr, "Error: Invalid argument: %s\n", argv[i]);
        return 1;
      } else {
        if (source_file_count < MAX_SOURCE_FILE_COUNT) {
          source_files[source_file_count] = argv[i];
          source_file_count += 1;
        } else {
          fprintf(stderr, "Ignoring file in compilation: %s\n", argv[i]);
        }
      }
    }
#pragma endregion

#pragma region Ensure some source file is used
    if (source_file_count == 0) {
      source_files[0] = "*.c";
      source_file_count = 1;
    }
#pragma endregion

  { // Build the Compilation Command & Execute it
    char defines_joined[512] = {0};
    for (int i = 0; i < define_count; i++) {
      char buffer[128] = {0};
      sprintf(buffer, "-D%s ", defines[i]);
      strcat(defines_joined, buffer);
    }
    char includes_joined[512] = {0};
    for (int i = 0; i < include_count; i++) {
      char buffer[128] = {0};
      sprintf(buffer, "-I%s ", includes[i]);
      strcat(includes_joined, buffer);
    }
    char libs_joined[512] = {0};
    for (int i = 0; i < lib_count; i++) {
      char buffer[128] = {0};
      sprintf(buffer, "-L%s ", libs[i]);
      strcat(libs_joined, buffer);
    }
    char dlls_joined[512] = {0};
    for (int i = 0; i < lib_count; i++) {
      WIN32_FIND_DATA data;
      char search[128] = {0};
      sprintf(search, "%s/*.dll", libs[i]);
      HANDLE hFind = FindFirstFile(search, &data);
      if (hFind != INVALID_HANDLE_VALUE) {
        do {
          char dll_name[64] = {0};
          unsigned char dll_name_i = 0;
          while (data.cFileName[dll_name_i] != '.' && data.cFileName[dll_name_i] != '\0') {
            dll_name[dll_name_i] = data.cFileName[dll_name_i];
            dll_name_i += 1;
          }
          char buffer[128] = {0};
          sprintf(buffer, "-l%s ", dll_name);
          strcat(dlls_joined, buffer);
        } while (FindNextFile(hFind, &data));
        FindClose(hFind);
      }
    }
    char source_files_joined[512] = {0};
    for (int i = 0; i < source_file_count; i++) {
      strcat(source_files_joined, source_files[i]);
      strcat(source_files_joined, " ");
    }
    char command[512] = {0};
    sprintf(
      command,
      "%s -O%s -s -fno-ident -fno-asynchronous-unwind-tables %s %s -std=%s -o %s %s %s %s %s %s",
      compiler,
      fast ? "fast" : "z",
      !quiet ? "-Wall -Wextra" : "",
      strict ? "-Werror" : "",
      standard,
      out_file,
      defines_joined,
      includes_joined,
      libs_joined,
      dlls_joined,
      source_files_joined
    );
    if (!quiet) {
      printf("\n=> COMPILE    ");
      puts(command);
      printf("=> OUTPUT     %s\n", out_file);
      if (!run && !compress) {
        putchar('\n');
      }
    }
    if (system(command) != 0) {
      putchar('\n');
      return 1;
    }
  }

#pragma region Build the Compression Command & Exectute it
    if (compress) {
      char command[256] = {0};
      sprintf(
        command,
        "upx --best --lzma --ultra-brute -f --compress-icons=3 --8-bit --no-reloc --no-align %s >nul 2>&1",
        out_file
      );
      if (!quiet) {
        printf("=> COMPRESS   ");
        puts(command);
        if (!run) {
          putchar('\n');
        }
      }
      system(command);
    }
#pragma endregion

#pragma region Run the Compiled Program if the User So Desires
    if (run) {
      char run_args_joined[512] = {0};
      for (int i = 0; i < run_arg_count; i++) {
        strcat(run_args_joined, run_args[i]);
        strcat(run_args_joined, " ");
      }
      char buffer[256] = {0};
      sprintf(
        buffer,
        "%s %s",
        out_file,
        run_args_joined
      );
      if (!quiet) {
        printf("=> RUN        %s\n\n", buffer);
        int ret = system(buffer);
        printf("\n=> EXIT       %s (%d)\n\n", ret == 0 ? "OK" : "ERROR", ret);
      } else {
        system(buffer);
      }
    } else if (run_arg_count > 0) {
      fprintf(
        stderr,
        "Error: You passed some args for the compiled program but didn't actually run it!\n"
        "       Use the --run/-r option to run the program after compiling it.\n\n"
      );
      return 1;
    }
#pragma endregion

  return 0;
}
