# diffdir

diffdir compares files in two directory trees and writes the contained filenames to three files in the working directory, `a_only`, and `b_only`, `common`, depending on whether a file exists only on the "a"/left side, the "b"/right side, or has identical contents on both sides. Directories are considered common if they exist on both sides, regardless if they contain the same set of files. If files with the same name exist on both sides but have different content, each is considered to belong only to their respective side. If a directory only exists on one side, it gets listed as being on only one side, but its contents are not visited or listed in the output files.

A binary to be used on the command-line, `diffdir`, is provided, as well as a static library, `libdiffdir.a`.

# Prerequisites

diffdir has only been built on Linux with GNU make and gcc 14.2.1 using `-std=c99`. To reduce the project scope, portability was not a major concern during development.

# Installation

Run:

    make && sudo make install

# Usage

To compare two directories, give them as arguments to diffdir, and then view the resulting `common`, `a_only`, and `b_only` files.

    diffdir <dir_a> <dir_b>

# Linking

To link libdiffdir into your application, install using the above steps and then use `pkg-config` to get the required compiler and linker flags. For example, if using make:

    CFLAGS += `pkg-config --cflags libdiffdir`
    LDFLAGS += `pkg-config --libs libdiffdir`

Depending on your system configuration, it may be necessary to specify the `pkg-config` metatdata installation location, `/usr/local/pkgconfig/` by default, using `PKG_CONFIG_PATH` or the `--with-path` option.
