#!/bin/sh
find . -name '*.py' \
-o -name '*.java' \
-o -iname '*.[CH]' \
-o -name '*.c' \
-o -name '*.h' \
> cscope.files

# -b: just build
# -q: create inverted index
cscope -b -q

# ctags - simply run the following
ctags -R *
