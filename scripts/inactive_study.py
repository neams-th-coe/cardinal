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

# This script runs the OpenMC wrapping for a number of axial layer discretizations
# in order to determine how many inactive cycles are needed to converge both the
# Shannon entropy and k. This script is run with:
#
# python inactive_study.py -i <script_name> -input <file_name> [-n-threads <n_threads>]
#
# - the script used to create the OpenMC model is named <script_name>.py;
#   this script MUST accept '-s' as an argument to add a Shannon entropy mesh
#   AND '-n' to set the number of layers; please consult the tutorials for
#   examples if you're unsure of how to do this
# - the Cardinal input file to run is named <file_name>
# - by default, the number of threads is taken as the maximum on your system;
#   otherwise, you can set it by providing -n-threads <n_threads>
#
# This script will create plots named <script_name>_k_<layers>.pdf and
# <script_name>_entropy_<layers>.pdf in a sub-directory named inactive_study/.

# Whether to use saved statepoint files to just generate plots, skipping transport solves
use_saved_statepoints = False

# Number of cell layers to consider
n_layers = [5, 10]

# Number of inactive batches to run
n_inactive = 100

# Number of inactive batches to average over to draw average-k line
averaging_batches = 100

# ------------------------------------------------------------------------------ #

from argparse import ArgumentParser
import multiprocessing
import matplotlib
import matplotlib.pyplot as plt
import numpy as np
import openmc
import sys
import os

matplotlib.rcParams.update({'font.size': 14})
colors = ['firebrick', 'orangered', 'darkorange', 'goldenrod', 'forestgreen', \
          'lightseagreen', 'steelblue', 'slateblue']

program_description = ("Script for determining required number of inactive batches "
                       "for an OpenMC model divided into a number of layers")
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

# Number of active batches to run (a small number, inconsequential for this case)
n_active = 10

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
if (not os.path.exists(os.getcwd() + '/inactive_study')):
  os.makedirs(os.getcwd() + '/inactive_study')

entropy = []
k = []
k_generation_avg = []
n_batches = n_inactive + n_active

max_entropy = sys.float_info.min
min_entropy = sys.float_info.max
max_k = sys.float_info.min
min_k = sys.float_info.max

for n in n_layers:
    new_sp_filename = 'inactive_study/statepoint_' + str(n) + '_layers.h5'

    if ((not use_saved_statepoints) or (not os.path.exists(new_sp_filename))):
        # Generate a new set of OpenMC XML files
        os.system("python " + script_name + " -s -n " + str(n))

        # Run Cardinal
        os.system(exec_name + " -i " + input_file + \
            " Problem/inactive_batches=" + str(n_inactive) + \
            " Problem/batches=" + str(n_batches) + \
            " Problem/max_batches=" + str(n_batches) + \
            " MultiApps/active='' Transfers/active='' Executioner/num_steps=1" + \
            " --n-threads=" + str(args.n_threads))

        # Copy the statepoint to a separate file for later plotting
        os.system('cp statepoint.' + str(n_batches) + '.h5 ' + new_sp_filename)

    with openmc.StatePoint(new_sp_filename) as sp:
        entropy.append(sp.entropy)
        k.append(sp.k_generation)
        max_entropy = max(np.max(sp.entropy[:n_inactive][:]), max_entropy)
        min_entropy = min(np.min(sp.entropy[:n_inactive][:]), min_entropy)
        max_k = max(np.max(sp.k_generation[:n_inactive][:]), max_k)
        min_k = min(np.min(sp.k_generation[:n_inactive][:]), min_k)

        averaging_k = sp.k_generation[(n_inactive - averaging_batches):n_inactive]
        k_generation_avg.append(sum(averaging_k) / len(averaging_k))

for i in range(len(n_layers)):
    nl = n_layers[i]
    fig, ax = plt.subplots()
    ax.plot(entropy[i][:n_inactive][:], label='{0:.0f} layers'.format(nl), color=colors[i])
    ax.set_xlabel('Inactive Batch')
    ax.set_ylabel('Shannon Entropy')
    ax.set_ylim([min_entropy, max_entropy])
    plt.grid()
    plt.legend(loc='upper right')
    plt.savefig('inactive_study/' + args.script_name + '_entropy' + str(nl) + '.pdf', bbox_inches='tight')
    plt.close()

for i in range(len(n_layers)):
    nl = n_layers[i]
    fig, ax = plt.subplots()
    ax.plot(k[i][:n_inactive][:], label='{0:.0f} layers'.format(nl), color=colors[i])
    plt.axhline(y=k_generation_avg[i], color='k', linestyle='-')

    ax.set_xlabel('Inactive Batch')
    ax.set_ylabel('$k$')
    ax.set_ylim([min_k, max_k])
    plt.grid()
    plt.legend(loc='upper right')
    plt.savefig('inactive_study/' + args.script_name + '_k' + str(nl) + '.pdf', bbox_inches='tight')
    plt.close()
