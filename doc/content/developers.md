# Developers Guide

This page describes a few
select repository maintenance instructions for Cardinal developers.

## Updating the Submodules

This section describes how to update the various submodule dependencies
in Cardinal. All submodules in Cardinal point to the main repository remotes for
each application *except* for the NekRS submnodule. For NekRS, we require special
build instructions to allow the same HYPRE installation as used in MOOSE to be used
with NekRS. Updating the other submodules is routine and does not require extra
explanation; therefore, these instructions only apply to updating the NekRS submodule.

The NekRS submodule points to the `cardinal` branch on our
[NekRS fork](https://github.com/neams-th-coe/NekRS). Assuming you have a separate repository checkout
of NekRS in your filesystem, first merge the latest master branch into a new NekRS feature update branch.

```
$ pwd
  /home/anovak/NekRS

$ git co -b cardinal-update-5-21
$ git merge origin/master
```

You will need to correct merge conflicts, which should just be related to retaining
the special HYPRE instructions in `config/hypre.cmake` and `CMakeLists.txt`.
After you have resolved the merge conflicts, open a pull request into the `cardinal` branch
on the [NekRS fork](https://github.com/neams-th-coe/NekRS).

Once the submodule itself is updated, then update the commit pointed to by the NekRS submodule
in Cardinal. During this step, we need to set the values of some NekRS CMake variables that we
don't necessarily set up during Cardinal's build process. In the Cardinal `Makefile`,
update the `NEKRS_VERSION`, `NEKRS_SUBVERSION`, and `GITCOMMITHASH` that are used to
print the NekRS version at the start of the NekRS initialization. The two
NekRS version macros can be found in the `build/nekrs/CMakeCache.txt` file that is created
after compiling Cardinal. Set `NEKRS_VERSION` to the value of `CMAKE_PROJECT_VERSION_MAJOR`
and set `NEKRS_SUBVERSION` to the value of `CMAKE_PROJECT_VERSION_MINOR`. Finally, set
`GITCOMMITHASH` to the
of the main NekRS repository that you just updated to. All three of `NEKRS_VERSION`, `NEKRS_SUBVERSION`,
and `GITCOMMITHASH` are added to `CXXFLAGS` in Cardinal's `Makefile`.

## Updating the Website

This website is built using [MooseDocs](https://mooseframework.inl.gov/python/MooseDocs/index.html),
an extension of Markdown with many additional features for MOOSE applications.

To update this website, first you will need to compile Cardinal (the Cardinal executable
is required for some application syntax detection in MooseDocs). Please follow the
instructions on the [Getting Started](start.md) page.

Next, generate the static HTML by running the following
from within the `doc` directory:

```
$ ./moosedocs.py build
```

This should print to the screen a summary of the steps taken when compiling
the documentation. If there are no errors or warnings, the last line printed
to the screen should display

```
CRITICAL:0 ERROR:0 WARNING:0
```

If there are any errors or warnings, please correct them before proceeding further.
When running `./moosedocs.py build`, one of the messages printed to the screen
will show something similar to

```
MooseDocs.build (MainProcess): Cleaning destination /home/anovak/.local/share/moose/site
```

This message indicates where the static HTML is placed. Navigate to
this directory, and then copy all content to the `cardinal` public HTML directory
in the [!ac](GCE) computing environment.

```
$ scp -r * <user>@homes.cels.anl.gov:/nfs/pub_html/gce/projects/cardinal
```


where `<user>` is your [!ac](GCE) account username. In order to have
permissions to access the `/nfs/pub_html/gce/projects/cardinal` directory,
you will first need access to the `Cardinal` [!ac](GCE) project
and Unix group. To request access, please visit the
[GCE help page](https://virtualhelpdesk.cels.anl.gov/docs/linux/projects-and-unix-groups/).
To get access to the Cardinal [!ac](GCE) group, please contact the
[Cardinal development team](contact.md).
For additional help, contact `help@cels.anl.gov`.
