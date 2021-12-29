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

# This script runs the OpenMC wrapping for a number of cell layer discretizations
# in order to determine how many layers are needed to converge both the power
# distribution and k. Your OpenMC model should include triggers for k and/or
# the tallies in order to control when the OpenMC simulation should terminate.
# This script should be run with:
#
# python layers_study.py -i <script_name> -input <file_name> [-n-threads <n_threads>]
#
# - the script used to create the OpenMC model is named <script_name>.py;
#   this script MUST accept '-n' to set the number of layers; please consult
#   the tutorials for examples if you're unsure of how to do this
# - the Cardinal input file to run is named <file_name>
# - by default, the number of threads is taken as the maximum on your system;
#   otherwise, you can set it by providing -n-threads <n_threads>
#
# This script will create plots of the maximum power, minimum power, radially-
# averaged power, and k as a function of the number of layers in the
# layers_study/ directory.
#
# NOTE: Your input file must also contain a number of postprocessors and user
# objects to generate these plots. Please consult the documentation for this
# script to determine which objects you need in your input file:
# https://cardinal.cels.anl.gov/tutorials/convergence.html

# Whether to use saved statepoint files to just generate plots, skipping transport solves
use_saved_statepoints = False

# Number of cell layers to consider
n_layers = [5, 10, 15, 25, 50, 75]

# Number of inactive batches to run
n_inactive = 25

# Minimum number of active batches to run
n_active = 5

# Maximum number of active batches to run
n_max_active = 1000

# Interval with which to check the triggers
batch_interval = 20

# Height of the domain, to be used for plotting the averaged heat source
h = 1.6

# ------------------------------------------------------------------------------ #

from argparse import ArgumentParser
import multiprocessing
import matplotlib
import matplotlib.pyplot as plt
import pandas
import numpy as np
import openmc
import csv
import sys
import os

matplotlib.rcParams.update({'font.size': 14})
colors = ['firebrick', 'orangered', 'darkorange', 'goldenrod', 'forestgreen', \
          'lightseagreen', 'steelblue', 'slateblue']

program_description = ("Script for determining required number of cell divisions "
                       "for an OpenMC model")
ap = ArgumentParser(description=program_description)

ap.add_argument('-i', dest='script_name', type=str,
                help='Name of the OpenMC python script to run (without .py extension)')
ap.add_argument('-n-threads', dest='n_threads', type=int,
                default=multiprocessing.cpu_count(), help='Number of threads to run Cardinal with')
ap.add_argument('-input', dest='input_file', type=str,
                help='Name of the Cardinal input file to run')

args = ap.parse_args()

input_file = args.input_file
script_name = args.script_name + ".py"
file_path = os.path.realpath(__file__)
file_dir, file_name = os.path.split(file_path)
exec_dir, file_name = os.path.split(file_dir)

# methods to look for, in order of preference
methods = ['opt', 'devel', 'oprof', 'dbg']
exec_name = ''
for i in methods:
  if (os.path.exists(exec_dir + "/cardinal-" + i)):
    exec_name = exec_dir + "/cardinal-" + i
    break

if (exec_name == ''):
  raise ValueError("No Cardinal executable was found!")

# create directory to write plots if not already existing
if (not os.path.exists(os.getcwd() + '/layers_study')):
  os.makedirs(os.getcwd() + '/layers_study')

nl = len(n_layers)

k = np.empty([nl])
k_std_dev = np.empty([nl])
max_power = np.empty([nl])
min_power = np.empty([nl])
max_power_std_dev = np.empty([nl])
min_power_std_dev = np.empty([nl])
q = np.empty([nl, max(n_layers)])

for i in range(nl):
    n = n_layers[i]
    file_base = "layers_study/openmc_" + str(n) + "_out"
    output_csv = file_base + ".csv"

    if ((not use_saved_statepoints) or (not os.path.exists(output_csv))):
        # Generate a new set of OpenMC XML files
        os.system("python " + script_name + " -n " + str(n))

        # Run Cardinal
        os.system(exec_name + " -i " + input_file + \
            " UserObjects/average_power_axial/num_layers=" + str(n) + \
            " Problem/inactive_batches=" + str(n_inactive) + \
            " Problem/batches=" + str(n_active + n_inactive) + \
            " Problem/max_batches=" + str(n_max_active + n_inactive) + \
            " Problem/batch_interval=" + str(batch_interval) + \
            " MultiApps/active='' Transfers/active='' Executioner/num_steps=1" + \
            " Outputs/file_base=" + file_base + \
            " --n-threads=" + str(args.n_threads))

    # Read the postprocessor values from the CSV output
    data = pandas.read_csv(output_csv)
    k[i] = data.at[1, 'k']
    k_std_dev[i] = data.at[1, 'k_std_dev']
    max_power[i] = data.at[1, 'max_power']
    min_power[i] = data.at[1, 'min_power']
    max_power_std_dev[i] = data.at[1, 'proxy_max_power_std_dev']
    min_power_std_dev[i] = data.at[1, 'proxy_min_power_std_dev']

# Print the percent change in max/min powers
for i in range(1, nl):
    n = n_layers[i]
    print('Layers: ' + str(n) + ':')
    print('\tPercent change in max power: ' + str(abs(max_power[i] - max_power[i - 1]) / max_power[i - 1] * 100))
    print('\tPercent change in min power: ' + str(abs(min_power[i] - min_power[i - 1]) / min_power[i - 1] * 100))

fig, ax = plt.subplots()
plt.errorbar(n_layers, max_power, yerr=max_power_std_dev, fmt='.r', capsize=3.0, marker='o', linestyle='-', color='r')
ax.set_xlabel('Axial Layers')
ax.set_ylabel('Maximum Power (W/m$^3$)')
plt.grid()
plt.savefig('layers_study/' + args.script_name + '_max_power_layers.pdf', bbox_inches='tight')
plt.close()

fig, ax = plt.subplots()
plt.errorbar(n_layers, min_power, yerr=min_power_std_dev, fmt='.b', capsize=3.0, marker='o', linestyle='-', color='b')
ax.set_xlabel('Axial Layers')
ax.set_ylabel('Minimum Power (W/m$^3$)')
plt.grid()
plt.savefig('layers_study/' + args.script_name + '_min_power_layers.pdf', bbox_inches='tight')
plt.close()

fig, ax = plt.subplots()
plt.errorbar(n_layers, k, yerr=k_std_dev, fmt='.k', capsize=3.0, marker='o', color='k', linestyle = '-')
plt.xlabel('Axial Layers')
plt.ylabel('$k$')
plt.grid()
plt.savefig('layers_study/' + args.script_name + '_k_layers.pdf', bbox_inches='tight')
plt.close()

fig, ax = plt.subplots()
for l in range(nl):
    i = n_layers[l]
    power_file = "layers_study/openmc_" + str(i) + "_out_power_avg_0001.csv"

    q = []
    with open(power_file) as f:
        reader = csv.reader(f)
        next(reader) # skip header
        for row in reader:
            q.append(float(row[0]))

    dz = h / i
    z = np.linspace(dz / 2.0, h - dz / 2.0, i)
    plt.plot(z, q, label='{} layers'.format(i), color=colors[l], marker='o', markersize=2.0)

plt.xlabel('Axial Position (m)')
plt.ylabel('Power (W/m$^3$)')
plt.grid()
plt.legend(loc='lower center', ncol=2)
plt.savefig('layers_study/' + args.script_name + '_layers_q.pdf', bbox_inches='tight')
plt.close()
