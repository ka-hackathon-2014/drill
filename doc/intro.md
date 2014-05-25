# Introduction


## clang-format

   clang-format -style=file -i src/*


## clang-modernize

    git ls-files '*.cc'| xargs -I{} clang-modernize -summary {}


## include-what-you-use

    cxx=include-what-you-use file > iwyu.out
    fix_includes.py < iwyu.out
