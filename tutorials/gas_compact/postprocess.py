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

# This script is used to generate various plots in the z-direction of
# radially-averaged quantities using MOOSE vector postprocessor CSV output.

import matplotlib
import matplotlib.pyplot as plt
import numpy as np
import csv

n_openmc_layers = 30

matplotlib.rcParams.update({'font.size': 14})

avg_q = []
q_stdev = []
T_fuel = []
T_block = []

h = 1.6
dz = h / n_openmc_layers
z = np.linspace(dz / 2.0, h - dz / 2.0, n_openmc_layers)

with open('openmc_out_avg_q_0004.csv') as f:
    reader = csv.reader(f)
    next(reader) # skip header
    for row in reader:
        avg_q.append(float(row[0]) / 1e6)

with open('openmc_out_stdev_0004.csv') as f:
    reader = csv.reader(f)
    next(reader) # skip header
    for row in reader:
        q_stdev.append(float(row[0]) / 1e6)

with open('openmc_out_solid0_fuel_axial_avg_0004.csv') as f:
    reader = csv.reader(f)
    next(reader) # skip header
    for row in reader:
        T_fuel.append(float(row[0]))

with open('openmc_out_solid0_block_axial_avg_0004.csv') as f:
    reader = csv.reader(f)
    next(reader) # skip header
    for row in reader:
        T_block.append(float(row[0]))

fig, ax = plt.subplots()
plt.plot(z, avg_q, marker='o', markersize=3, color = 'k', linestyle = '-')
plt.xlabel('Axial Position (m)')
plt.ylabel('Radially-Averaged Power (MW/m$^3$)')
plt.grid()
plt.savefig('q_axial.png', bbox_inches='tight')
plt.close()

fig, ax = plt.subplots()
plt.plot(z, T_fuel, marker = 'o', color = 'r', label='Fuel Compact', markersize=3)
plt.plot(z, T_block, marker = 'o', color = 'b', label='Graphite Block', markersize=3)
plt.xlabel('Axial Position (m)')
plt.ylabel('Radially-Averaged Temperature (K)')
plt.grid()
plt.legend(loc='lower right')
plt.savefig('T_axial.png', bbox_inches='tight')
plt.close()
