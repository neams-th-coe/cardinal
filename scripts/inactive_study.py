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

ap.add_argument('-n-threads', dest='n_threads', type=int,
                default=multiprocessing.cpu_count(), help='Number of threads to run Cardinal with')
ap.add_argument('-i', dest='script_name', type=str, required=True,
                help='Name of the OpenMC python script to run (without .py extension)')
ap.add_argument('-input', dest='input_file', type=str, required=True,
                help='Name of the Cardinal input file to run')
ap.add_argument('--method', dest = 'method', choices =['all','half','window'], required=True,
                help = "The type of way to detect steady state. Options are all, half, or window")
opts, rem_args = ap.parse_known_args()
if(opts.method == "window"):
    ap.add_argument('--window_length', dest = 'window_length',required = True, type = int,
                    help =" The window length must be specified if window is the detectio method")
args = ap.parse_args()
# variable to be used in logic for which way to detect steady state
method = args.method
if(args.method == "window"):
    window_length = args.window_length

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

entropy_range = max_entropy - min_entropy
for i in range(len(n_layers)):
    nl = n_layers[i]
    fig, ax = plt.subplots()
    ax.plot(entropy[i][:n_inactive][:], label='{0:.0f} layers'.format(nl), color=colors[i])
    ax.set_xlabel('Inactive Batch')
    ax.set_ylabel('Shannon Entropy')
    ax.set_ylim([min_entropy - 0.05 * entropy_range, max_entropy + 0.05 * entropy_range])
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
    plt.legend(loc='lower right')
    plt.savefig('inactive_study/' + args.script_name + '_k' + str(nl) + '.pdf', bbox_inches='tight')
    plt.close()

# loop over each layer (index i) and report inactive batch (index j) that satisfies
# convergence criteria for the selected method (window,half, or all)
for i in range(len(n_layers)):
  nl = n_layers[i]
  if(method == "window"):
      # detect when entropy of current batch is within window (mean +/- std of window)
      for j in range(window_length,len(entropy[i])):
          window = entropy[i][(j-window_length):j]
          window_mean = np.average(window)
          window_dev = np.std(window)
          window_low = window_mean - window_dev
          window_high = window_mean + window_dev
          # print("window bounds: ", window_low , " to ", window_high )
          if(window_high - entropy[0][j] > 0 and entropy[0][j]-window_low > 0):
              # if entropy[j] is less than window_high and greater than
              # window_low, then it is within the window and we've succeeded
              print("For layer", nl, ", batch", j, "produced a value within one "
                    "standard deviation of the window mean. It is recommended to do "
                    "at least this many inactive cycles for this layer.")
              break
          else:
              continue
  elif(method == "half"):
      #  Accordinig to Brown, the below is done in MCNP5:
      # find the first cycle when Hsrc is within one standard deviation of its average for the last half of cycles
      # compute on the fly the average of the eigenvalue (last half batches) +/- the std dev of the last half cycles
      # and see if the current k is within this interval
      for j in range(1,len(entropy[i])):
          last_half_idx = int(np.floor(j/2))
          last_half_vals = entropy[0][last_half_idx:j]
          stdev = np.std(last_half_vals)
          mean = np.mean(last_half_vals)
          low =  mean - stdev
          high = mean + stdev
          if(high - entropy[0][j] > 0 and entropy[0][j]-low > 0):
              print("For layer", nl, ", batch", j, "produced a value within one "
                    "standard deviation of the mean for the last half of entropy values. "
                    "It is recommended to do at least this many inactive cycles for this layer.")
              break
          else:
              continue
  else:
      # use all batches as data points
      for j in range(1,len(entropy[i])):
          entropy_slice = entropy[i][0:j]
          stdev = np.std(entropy_slice)
          mean = np.mean(entropy_slice)
          low =  mean - stdev
          high = mean + stdev
          if(high - entropy[i][j] > 0 and entropy[i][j]-low > 0):
              print("For layer", nl, ", batch", j, "produced a value within one "
                    "standard deviation of the mean of the last", j ,"entropy values. "
                    "It is recommended to do at least this many inactive cycles for this layer.")
              break
          else:
              continue