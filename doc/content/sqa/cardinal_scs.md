!template load file=sqa/app_scs.md.template app=Cardinal category=cardinal

## Clang Format

!style halign=left
Like MOOSE, Cardinal uses `clang-format` with a customized
[config file](https://github.com/neams-th-coe/cardinal/blob/devel/.clang-format)
for code formatting. If you have clang installed, you can run

```
git clang-format [<branch>]
```

to automatically format code changed between your currently checked-out branch
and `<branch>` (if left out, it defaults to the `HEAD` commit). If you don't do
this before submitting your code, don't worry! The continuous integration
testing system, [CIVET](https://civet.inl.gov), that is triggered when
you submit a pull request, will check your code and provide information on the
changes needed to conform to the code style (if any).

## Cardinal Code Standards

!style halign=left
Cardinal follows the MOOSE code standards for all development. For information on file guidelines,
naming conventions, example code, doxygen documentation, and other tips, please see
[the MOOSE standard](sqa/framework_scs.md).
