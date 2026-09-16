#!/usr/bin/env python
#********************************************************************/
#*                  SOFTWARE COPYRIGHT NOTIFICATION                 */
#*                             Cardinal                             */
#*                                                                  */
#*                  (c) 2021 UChicago Argonne, LLC                  */
#*                        ALL RIGHTS RESERVED                       */
#*                                                                  */
#*                 Prepared by UChicago Argonne, LLC                */
#*               Under Contract No. DE-AC02-06CH11357               */
#*                With the U. S. Department of Energy               */
#*                                                                  */
#*             Prepared by Battelle Energy Alliance, LLC            */
#*               Under Contract No. DE-AC07-05ID14517               */
#*                With the U. S. Department of Energy               */
#*                                                                  */
#*                 See LICENSE for full restrictions                */
#********************************************************************/

import sys
import os
import subprocess

# Locate MOOSE directory
MOOSE_DIR = os.getenv('MOOSE_DIR', os.path.abspath(os.path.join(os.path.dirname(__name__), '..', 'contrib', 'moose')))
if not os.path.exists(MOOSE_DIR):
    MOOSE_DIR = os.path.abspath(os.path.join(os.path.dirname(__name__), '..', 'moose'))
if not os.path.exists(MOOSE_DIR):
    MOOSE_DIR = os.path.abspath(os.path.join(os.path.dirname(__name__), '..', '..', 'moose'))
if not os.path.exists(MOOSE_DIR):
    raise Exception('Failed to locate MOOSE, specify the MOOSE_DIR environment variable.')
os.environ['MOOSE_DIR'] = MOOSE_DIR

large_media = os.path.abspath(os.path.join(MOOSE_DIR, "large_media"))
if (not os.path.exists(large_media) or len(os.listdir(large_media)) == 0):
  raise Exception("To build Cardinal's documentation, MOOSE's large_media submodule must be checked out. Try the following first:\n\ncd cardinal/contrib/moose\ngit submodule update --init large_media")

# Append MOOSE python directory
MOOSE_PYTHON_DIR = os.path.join(MOOSE_DIR, 'python')
if MOOSE_PYTHON_DIR not in sys.path:
    sys.path.append(MOOSE_PYTHON_DIR)

from MooseDocs import main

# MooseDocs' own __init__ (just imported) has now set ROOT_DIR -- the git
# checkout root, which SQA/git-based extensions need to find real history
# in. appsyntax's own executable search wants a *different* answer for a
# CMake-superbuild build, though: cardinal-opt lives in a separate,
# out-of-source build directory (never inside ROOT_DIR, and never itself
# a git repo, since that build tree is a plain mirror), so appsyntax's own
# `executable:` config setting has its own variable to default from
# ROOT_DIR (the normal, plain-Makefile-build case, where the executable
# really does end up alongside the checkout) without forcing every other
# git-dependent extension to also look somewhere that isn't a checkout.
os.environ.setdefault("CARDINAL_EXECUTABLE_DIR", os.environ["ROOT_DIR"])

if __name__ == '__main__':
    os.chdir("..")

    # Doxygen's own OUTPUT_DIRECTORY (doc/content/doxygen/Doxyfile) is a
    # relative path, resolved against doxygen's cwd (ROOT_DIR, just above)
    # -- always correct for the plain Makefile build, where the checkout
    # itself is the build directory, but a write into the checkout itself
    # for a CMake-superbuild build, whose own build tree is a separate,
    # disposable mirror everything else builds into instead. Rather than
    # editing the checked-in Doxyfile to know about that (its OUTPUT_
    # DIRECTORY is meaningful on its own, read directly by any contributor
    # who runs doxygen by hand), override it by piping the real, unmodified
    # Doxyfile's content through `doxygen -` (reads its config from stdin)
    # with one extra assignment appended -- doxygen's config parser is a
    # flat top-to-bottom pass where the last assignment to a given
    # single-valued tag wins, so this doesn't require understanding or
    # duplicating anything else already in the file. Defaults right back to
    # the original relative path, so the plain build's behavior (and the
    # doc/content/doxygen/html*-based .gitignore entry) is unchanged.
    with open("doc/content/doxygen/Doxyfile") as f:
        doxyfile = f.read()
    doxygen_output_dir = os.environ.setdefault(
        "CARDINAL_DOXYGEN_OUTPUT_DIR", "doc/content/doxygen"
    )
    doxyfile += "\nOUTPUT_DIRECTORY = {}\n".format(doxygen_output_dir)
    subprocess.run(["doxygen", "-"], input=doxyfile, text=True)

    os.chdir("doc")
    sys.exit(main.run())
