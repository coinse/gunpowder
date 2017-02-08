# Contributing to CAVM

## Code Convention

### Python
We strictly follow the format of yapf(https://github.com/google/yapf) and pylint(https://www.pylint.org/).  
Before making the commit, don't forget to run followings.

```sh
$ yapf -i <changed files>
$ pylint <changed files>
```

### C++
We strictly follow the `clang-format`(https://clang.llvm.org/docs/ClangFormat.html) and Google style guide(https://github.com/google/styleguide/tree/gh-pages/cpplint).  
Before making the commit, don't forget to run followings.

```sh
$ clang-format -style=Google -i <changed files>
$ cpplint <changed files>
```
