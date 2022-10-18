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
# Optional settings:

# What quantities to plot for temporal convergence. options:
# Vx, Vy, Vz, P, T, S<nn>, dP. The 'dP' option is only valid for periodic
# flows, as it will extract the bulk pressure drop using information for
# the constant flow rate solver
temporal_plots = ['Vx', 'Vy', 'Vz', 'S01', 'S02', 'dP']

# Length of domain in non-dimensional units, to use for temporal plots
flow_through_length = 144.3761398582499

# Names to use for the scalars for plotting
scalar_names = ["$T$", "$k$", "$\\tau$"]

# Colors to use for plotting
colors = ['firebrick', 'darkorange', 'forestgreen', 'steelblue', 'slateblue', 'black']
if (len(colors) < len(temporal_plots)):
  raise ValueError("Length of 'colors' array must be at least as long as the length " + \
    "of 'temporal_plots'! Please edit plot_nek_logfile.py to specify more plot colors.")

import matplotlib
matplotlib.use('Agg')

from argparse import ArgumentParser
from colorama import Fore
import matplotlib.pyplot as plt
import numpy as np
import re

matplotlib.rcParams.update({'font.size': 14})

program_description = ("Script for plotting quantities from NekRS's console output")
ap = ArgumentParser(description=program_description)

ap.add_argument('-i', dest='logfile_name', type=str, default="logfile",
                help='Name of the NekRS logfile')

args = ap.parse_args()

# Get various quantities indicating the performance (runtime, number of iterations)
# of the solution
CFL       = []
UVW       = []
P         = []
eTimeStep = []
dt        = []
t         = []

# Also get quantities that can be used as one measure of whether a pseudo-steady
# solution has been obtained
max_Vx      = []
max_Vy      = []
max_Vz      = []
max_P       = []
max_scalars = []
min_Vx      = []
min_Vy      = []
min_Vz      = []
min_P       = []
min_scalars = []
dP          = []
n_scalars   = 0
n_lines_in_write_checkpoint = 2
casename    = ""

first_fld_file = True
constant_flow_rate = False

# do some initial parsing to find information like number of scalars
i = 0
l = -1
step = 0
fld_file_time = []
lines_with_scale_before_write_checkpoint = []

first_shift = True
with open(args.logfile_name, 'r') as f:
  lines = f.readlines()
  for line in lines:
    l += 1

    # get the number of scalars
    if (line.startswith('key: NUMBER OF SCALARS,')):
      match = re.search('value: (.*)', line)
      if (match):
        n_scalars = int(match.group(1).strip())
        n_lines_in_write_checkpoint += n_scalars
        max_scalars = [ [] for i in range(n_scalars)]
        min_scalars = [ [] for i in range(n_scalars)]

    # get the casename
    if (line.startswith('key: CASENAME,')):
      match = re.search('value: (.*)', line)
      if (match):
        casename = match.group(1).strip()

    if (line.startswith('step=')):
      # get the time
      match = re.search('t= (.*) dt=', line)
      if (match):
        step = float(match.group(1).strip())

    # get the line numbers that have write checkpoint info
    if (line.startswith(' min/max:')):
      if (first_fld_file):
        first_fld_file = False
        continue

      split = line.split()

      if (i == 0):
        if (first_shift):
          lines_with_scale_before_write_checkpoint.append(l - 6)
          first_shift = False
        else:
          lines_with_scale_before_write_checkpoint.append(l - 5)

        fld_file_time.append(step)
        min_Vx.append(float(split[1]))
        max_Vx.append(float(split[2]))
        min_Vy.append(float(split[3]))
        max_Vy.append(float(split[4]))
        min_Vz.append(float(split[5]))
        max_Vz.append(float(split[6]))

      if (i == 1):
        min_P.append(float(split[1]))
        max_P.append(float(split[2]))

      for s in range(n_scalars):
        if (i == 2 + s):
          min_scalars[s].append(float(split[1]))
          max_scalars[s].append(float(split[2]))

      if (i == n_lines_in_write_checkpoint - 1):
        i = 0
      else:
        i += 1

# get the scale
l = -1
k = 0
with open(args.logfile_name, 'r') as f:
  lines = f.readlines()
  for line in lines:
    l += 1

    if (k < len(lines_with_scale_before_write_checkpoint) and l == lines_with_scale_before_write_checkpoint[k]):
      k += 1

      match = re.search('scale(.*)', line)
      if (match):
        dP.append(float(match.group(1).strip()))

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

      match = re.search('scale (.*)', line)
      if (match):
        scale.append(float(match.group(1).strip()))

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
print('Flow-thru times:   ', np.sum(dt) / flow_through_length)
print('runtime / second:  ', np.sum(eTimeStep) / np.sum(dt))
plt.plot(t, eTimeStep, color='k')
plt.xlabel('Time')
plt.ylabel('Elapsed Time per Step')
plt.grid()
plt.savefig('eTime.pdf', bbox_inches="tight")
plt.close()

n_fld_files = len(max_Vx)

for i in range(len(fld_file_time)):
  fld_file_time[i] /= flow_through_length
  fld_file_time[i] = np.round(fld_file_time[i], 2)

ms = 4
lw = 2

color_idx = 0
# Make plots of absolute magnitudes of various temporal quantities
if ('Vx' in temporal_plots and len(max_Vx) > 1):
  plt.plot(fld_file_time, max_Vx, marker='o', markersize=ms, linewidth=lw, color=colors[color_idx])
  color_idx += 1
  plt.xlabel('Flow-Through Times (-)')
  plt.ylabel('Maximum $V_x$')
  plt.grid()
  plt.savefig('max_Vx.pdf', bbox_inches="tight")
  plt.close()

if ('Vy' in temporal_plots and len(max_Vy) > 1):
  plt.plot(fld_file_time, max_Vy, marker='o', markersize=ms, linewidth=lw, color=colors[color_idx])
  color_idx += 1
  plt.xlabel('Flow-Through Times (-)')
  plt.ylabel('Maximum $V_y$')
  plt.grid()
  plt.savefig('max_Vy.pdf', bbox_inches="tight")
  plt.close()

if ('Vz' in temporal_plots and len(max_Vz) > 1):
  plt.plot(fld_file_time, max_Vz, marker='o', markersize=ms, linewidth=lw, color=colors[color_idx])
  color_idx += 1
  plt.xlabel('Flow-Through Times (-)')
  plt.ylabel('Maximum $V_z$')
  plt.grid()
  plt.savefig('max_Vz.pdf', bbox_inches="tight")
  plt.close()

if ('P' in temporal_plots and len(max_P) > 1):
  plt.plot(fld_file_time, max_P, marker='o', markersize=ms, linewidth=lw, color=colors[color_idx])
  color_idx += 1
  plt.xlabel('Flow-Through Times (-)')
  plt.ylabel('Maximum $P$')
  plt.grid()
  plt.savefig('max_P.pdf', bbox_inches="tight")
  plt.close()

for j in range(n_scalars):
  if ('S0' + str(j) in temporal_plots and len(max_scalars[j]) > 1):
    plt.plot(fld_file_time, max_scalars[j], marker='o', markersize=ms, linewidth=lw, color=colors[color_idx])
    color_idx += 1
    plt.xlabel('Flow-Through Times (-)')
    plt.ylabel('Maximum ' + scalar_names[j])
    plt.grid()
    plt.savefig('max_S' + str(j) + '.pdf', bbox_inches="tight")
    plt.close()

if ('dP' in temporal_plots and len(dP) > 1):
  plt.plot(fld_file_time, dP, marker='o', markersize=ms, linewidth=lw, color=colors[color_idx])
  color_idx += 1
  plt.xlabel('Flow-Through Times (-)')
  plt.ylabel('$\Delta P/\Delta L$')
  plt.grid()
  plt.savefig('dPdL.pdf', bbox_inches="tight")
  plt.close()

def rel_diff(a, b):
  """Compute relative difference between two fields, returning zero
     if both are zero."""
  tol = 1e-8
  if (abs(a) < tol and abs(b) < tol):
    return 0.0;
  else:
    return abs(a - b) / b

has_plot = False
color_idx = 0

# Make plots of temporal convergence
print('')
if ('Vx' in temporal_plots and len(max_Vx) > 1):
  has_plot = True
  rel_diff_max_Vx = []
  for i in range(n_fld_files - 1):
    rel_diff_max_Vx.append(rel_diff(max_Vx[i + 1], max_Vx[i]))
  plt.semilogy(fld_file_time[1:], rel_diff_max_Vx, marker='o', markersize=ms, linewidth=lw, color=colors[color_idx], label='Max $V_x$')
  color_idx += 1
  print(Fore.RED + '\nPercent change in maximum Vx:  ', rel_diff_max_Vx[-1] * 100.0)
  print(Fore.GREEN + '  maximum Vx:                  ', max_Vx[-1])

if ('Vy' in temporal_plots and len(max_Vy) > 1):
  has_plot = True
  rel_diff_max_Vy = []
  for i in range(n_fld_files - 1):
    rel_diff_max_Vy.append(rel_diff(max_Vy[i + 1], max_Vy[i]))
  plt.semilogy(fld_file_time[1:], rel_diff_max_Vy, marker='o', markersize=ms, linewidth=lw, color=colors[color_idx], label='Max $V_y$')
  color_idx += 1
  print(Fore.RED + '\nPercent change in maximum Vy:  ', rel_diff_max_Vy[-1] * 100.0)
  print(Fore.GREEN + '  maximum Vy:                  ', max_Vy[-1])

if ('Vz' in temporal_plots and len(max_Vz) > 1):
  has_plot = True
  rel_diff_max_Vz = []
  for i in range(n_fld_files - 1):
    rel_diff_max_Vz.append(rel_diff(max_Vz[i + 1], max_Vz[i]))
  plt.semilogy(fld_file_time[1:], rel_diff_max_Vz, marker='o', markersize=ms, linewidth=lw, color=colors[color_idx], label='Max $V_z$')
  color_idx += 1
  print(Fore.RED + '\nPercent change in maximum Vz:  ', rel_diff_max_Vz[-1] * 100.0)
  print(Fore.GREEN + '  maximum Vz:                  ', max_Vz[-1])

if ('P' in temporal_plots and len(max_P) > 1):
  has_plot = True
  rel_diff_max_P = []
  for i in range(n_fld_files - 1):
    rel_diff_max_P.append(rel_diff(max_P[i + 1], max_P[i]))
  plt.semilogy(fld_file_time[1:], rel_diff_max_P, marker='o', markersize=ms, linewidth=lw, color=colors[color_idx], label='Max $P$')
  color_idx += 1
  print(Fore.RED + '\nPercent change in maximum P:   ', rel_diff_max_P[-1] * 100.0)
  print(Fore.GREEN + '  maximum P:                   ', max_P[-1])

for j in range(n_scalars):
  if ('S0' + str(j) in temporal_plots and len(max_scalars[j]) > 1):
    has_plot = True
    rel_diff_max_S = []
    for i in range(n_fld_files - 1):
      rel_diff_max_S.append(rel_diff(max_scalars[j][i + 1], max_scalars[j][i]))
    plt.semilogy(fld_file_time[1:], rel_diff_max_S, marker='o', markersize=ms, linewidth=lw, color=colors[color_idx], label='Max ' + scalar_names[j])
    color_idx += 1
    print(Fore.RED + '\nPercent change in maximum S0' + str(j) + ': ', rel_diff_max_S[-1] * 100.0)
    print(Fore.GREEN + '  maximum S0' + str(j) + ':                 ', max_scalars[j][-1])

if ('dP' in temporal_plots and len(dP) > 1):
  has_plot = True
  rel_diff_dP = []
  for i in range(n_fld_files - 1):
    rel_diff_dP.append(rel_diff(dP[i + 1], dP[i]))
  plt.semilogy(fld_file_time[1:], rel_diff_dP, marker='o', markersize=ms, linewidth=lw, color=colors[color_idx], label='$\Delta P/\Delta L$')
  color_idx += 1
  print(Fore.RED + '\nPercent change in dP/dL:       ', rel_diff_dP[-1] * 100.0)
  print(Fore.GREEN + '  dP/dL:                       ', dP[-1])

if (has_plot):
  plt.xlabel('Flow-Through Times (-)')
  plt.ylabel('Relative Difference')
  plt.legend(ncol= int(len(temporal_plots) / 2), handletextpad=0.2)
  plt.grid(which="both")
  plt.savefig('temporal.pdf', bbox_inches="tight")
  plt.close()
