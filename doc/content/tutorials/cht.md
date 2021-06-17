# Conjugate Heat Transfer Coupling of nekRS and MOOSE Heat Conduction

This tutorial describes how to use nekRS to solve for fluid flow and heat transfer
coupled to a solid described by the MOOSE heat conduction module. Two examples are
provided - a bare 7-pin [!ac](SFR) fuel bundle and a section of a reflector
block in a [!ac](PB-FHR).

## Tutorial 1A
  id=tutorial1a

This tutorial describes how to use Cardinal to perform conjugate heat transfer
calculations using nekRS and the MOOSE heat conduction module for a bare
7-pin [!ac](SFR) bundle. nekRS solves for the fluid phase, while MOOSE solves for
the solid phase.

## Tutorial 1B
  id=tutorial1b

This tutorial provides a second description of how to use Cardinal to perform
conjugate heat transfer calculations with MOOSE. This tutorial shows similar capabilities
as in [#tutorial1a], but the application is performed to bypass flow in the reflector
block of a [!ac](PB-FHR). This tutorial was developed as part of the
[!ac](VTB), a [!ac](NRIC) initiative aimed at facilitating the use of advanced modeling
and simulation tools. This tutorial is quite comprehensive, and is available
[here](https://mooseframework.inl.gov/virtual_test_bed/pbfhr/reflector.html).
All input files associated with this tutorial are hosted publicly in the
[!ac](VTB) repository [here](https://github.com/idaholab/virtual_test_bed).

