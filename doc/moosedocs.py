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
    os.system("doxygen doc/content/doxygen/Doxyfile")
    os.chdir("doc")
    sys.exit(main.run())
