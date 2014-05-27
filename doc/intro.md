# Introduction


## clang-format

   clang-format -style=file -i src/*


## clang-modernize

    git ls-files '*.cc'| xargs -I{} clang-modernize -summary {}


## include-what-you-use

Warning: after applying the patches, look through them by hand and confirm successfull compilation; the tool is not correct in all cases and may include some extension headers, depending on stdlib used! Remove those by hand! In other situations you may have to exchange the forward declarations with real includes, e.g. if you're using a std::unique_ptr<T> a forward declared T does not suffice!

Note: not usable with cmake generated makefiles, see: https://code.google.com/p/include-what-you-use/issues/detail?id=87

    for file in src/* ; do include-what-you-use -std=c++11 $file 2>> /tmp/iwyu.out ; done

Some distributions do not include the fix_includes.py script:

    wget https://include-what-you-use.googlecode.com/svn/tags/clang_3.4/fix_includes.py
    python2 fix_includes.py < /tmp/iwyu.out --comment

Warning: apply clang-format on the fixes, too!

Note: for ease of use you may install fix_includes.py to /usr/local/bin/, adapt the shebang to python2 and chmod a+x it.
