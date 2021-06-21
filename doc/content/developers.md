# Developers Guide

Cardinal is hosted on GitHub, and we welcome all contributors.

## Contribution Process

All changes made to the Cardinal repository must be made through
a pull request. All pull requests should be independently reviewed,
 except for routine submodule updates that do not
require subsequent code changes in Cardinal itself.

## Updating the Submodules

This section describes how to update the various submodule dependencies
in Cardinal. These instructions are meant only for developers.

Cardinal uses submodules for its dependencies - MOOSE, OpenMC, nekRS, and SAM. All dependencies
except for nekRS point to the main repository remotes for each application. However, in order to
use the same HYPRE installation as used by MOOSE within nekRS, we maintain a separate
[nekRS fork](https://github.com/neams-th-coe/nekRS). Another purpose of this separate fork is
to allow CIVET's recipes to work with nekRS's dependencies. Specifically, nekRS uses
[git subtrees](https://www.atlassian.com/git/tutorials/git-subtree) for its dependencies. However,
Nek5000 contains a [git submodule](https://git-scm.com/book/en/v2/Git-Tools-Submodules),
for the `examples` repository containing input file examples.
When CIVET attemps to recursively fetch all nested submodules in Cardinal, the presence of a
submodule inside of a subtree does not work nicely with git's submodule commands. Therefore, our
nekRS fork has deleted the `examples` submodule within nekRS's Nek5000 subtree (this submodule
is not needed anyways, since Cardinal focuses exclusively on nekRS coupling).

Updating the MOOSE, OpenMC, and SAM submodules is routine and does not
require extra explanation; therefore, these instructions only apply to updating the
nekRS submodule.

The nekRS submodule points to the `cardinal` branch on the
[nekRS fork](https://github.com/neams-th-coe/nekRS). Assuming you have a separate repository checkout
of nekRS in your filesystem, first merge the latest master branch into a new nekRS feature update branch.

```
$ pwd
  /home/anovak/nekRS

$ git co -b cardinal-update-5-21
$ git merge origin/master
```

You will need to correct merge conflicts, which should mostly be keeping the
`Deleted by us` changes related to removing the `examples` submodule and retaining
the special HYPRE instructions in `config/hypre.cmake`.
After you have resolved the merge conflicts, open a pull request into the `cardinal` branch
on the [nekRS fork](https://github.com/neams-th-coe/nekRS).

Once the submodule itself is updated, then update the commit pointed to by the nekRS submodule
in Cardinal. During this step, we need to set the values of some nekRS CMake variables that we
don't necessarily set up during Cardinal's build process. In the Cardinal `Makefile`,
update the `NEKRS_VERSION`, `NEKRS_SUBVERSION`, and `GITCOMMITHASH` that are used to
print the nekRS version at the start of the nekRS initialization. The two
nekRS version macros can be found in the `build/nekrs/CMakeCache.txt` file that is created
after compiling Cardinal. Set `NEKRS_VERSION` to the value of `CMAKE_PROJECT_VERSION_MAJOR`
and set `NEKRS_SUBVERSION` to the value of `CMAKE_PROJECT_VERSION_MINOR`. Finally, set
`GITCOMMITHASH` to the [git hash](https://github.com/neams-th-coe/nekRS/commits/cardinal)
of the nekRS fork that you just updated to. All three of `NEKRS_VERSION`, `NEKRS_SUBVERSION`,
and `GITCOMMITHASH` are added to `CXXFLAGS`.
