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

# This script plots various quantities from a NekRS logfile (the console
# output, which many Nek users are familiar with piping to a separate
# file). These plots can be used to assess the stability of a NekRS solve,
# and assess overall performance in terms of the number of iterations,
# time/step, and more.
#
# First, run your NekRS case (or Cardinal-wrapped NekRS case) with the output
# piped to a file, like either of the following:
#
# nrsmpi fluid 32 > logfile
#
# cardinal-opt -i moose.i > logfile
#
# Then, run this script with "-i" passing the name of the logfile to parse.

import matplotlib
matplotlib.use('Agg')

from argparse import ArgumentParser
import matplotlib.pyplot as plt
import numpy as np
import re

matplotlib.rcParams.update({'font.size': 14})
colors = ['firebrick', 'orangered', 'darkorange', 'goldenrod', 'forestgreen', \
          'lightseagreen', 'steelblue', 'slateblue']

program_description = ("Script for plotting quantities from NekRS's console output")
ap = ArgumentParser(description=program_description)

ap.add_argument('-i', dest='logfile_name', type=str, default="logfile",
                help='Name of the NekRS logfile')

args = ap.parse_args()

CFL = []
UVW = []
P = []
eTimeStep = []
dt = []
t = []

with open(args.logfile_name, 'r') as f:
  lines = f.readlines()
  for line in lines:
    if (line.startswith('step=')):
      # get the time
      match = re.search('t=(.*)dt', line)
      if (match):
        t.append(float(match.group(1).strip()))

      # get the time step size
      match = re.search('dt=(.*)C', line)
      if (match):
        dt.append(float(match.group(1).strip()))

      # get the CFL number
      match = re.search('C=(.*)', line)
      if (match):
        CFL.append(float(match.group(1).strip().split(" ", 1)[0]))

      match = re.search('UVW:(.*)', line)
      if (match):
        UVW.append(int(match.group(1).strip().split(" ", 1)[0]))

      match = re.search('P:(.*)', line)
      if (match):
        P.append(int(match.group(1).strip().split(" ", 1)[0]))

      match = re.search('eTimeStep=(.*)', line)
      if (match):
        eTimeStep.append(float(match.group(1).strip().split("s", 1)[0]))

      match = re.search('elapsedStep=(.*)', line)
      if (match):
        eTimeStep.append(float(match.group(1).strip().split("s", 1)[0]))

if (len(dt) == 0):
  raise ValueError("Cannot parse " + args.logfile_name + " time step because " + \
    "no NekRS time steps have been run!");

print('Average dt:        ', np.average(dt))
plt.plot(t, dt, color='k')
plt.xlabel('Time')
plt.ylabel('Time Step Size')
plt.grid()
plt.savefig('dt.pdf', bbox_inches="tight")
plt.close()

print('Average CFL:       ', np.average(CFL))
plt.plot(t, CFL, color='k')
plt.xlabel('Time')
plt.ylabel('Courant Number')
plt.grid()
plt.savefig('cfl.pdf', bbox_inches="tight")
plt.close()

if (len(UVW) > 0):
  print('\nAverage UVW:       ', np.average(UVW))
  print('  Percent > 200:   ', sum(i > 200 for i in UVW) / len(UVW) * 100.0)
  plt.plot(t, UVW, color='k')
  plt.xlabel('Time')
  plt.ylabel('Number of Velocity Iterations (-)')
  plt.grid()
  plt.savefig('uvw.pdf', bbox_inches="tight")
  plt.close()

if (len(P) > 0):
  print('\nAverage P:         ', np.average(P))
  print('  Percent > 200:   ', sum(i > 200 for i in P) / len(P) * 100.0)
  plt.plot(t, P, color='k')
  plt.xlabel('Time')
  plt.ylabel('Number of Pressure Iterations (-)')
  plt.grid()
  plt.savefig('p.pdf', bbox_inches="tight")
  plt.close()

print('\nAverage eTimeStep: ', np.average(eTimeStep))
print('Total runtime:     ', np.sum(eTimeStep))
print('End time:          ', np.sum(dt))
print('runtime / second:  ', np.sum(eTimeStep) / np.sum(dt))
plt.plot(t, eTimeStep, color='k')
plt.xlabel('Time')
plt.ylabel('Elapsed Time per Step')
plt.grid()
plt.savefig('eTime.pdf', bbox_inches="tight")
plt.close()

