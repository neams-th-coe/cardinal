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

with open('openmc_out_avg_q_0005.csv') as f:
    reader = csv.reader(f)
    next(reader) # skip header
    for row in reader:
        avg_q.append(float(row[0]) / 1e6)

with open('openmc_out_stdev_0005.csv') as f:
    reader = csv.reader(f)
    next(reader) # skip header
    for row in reader:
        q_stdev.append(float(row[0]) / 1e6)

with open('openmc_out_solid0_fuel_axial_avg_0005.csv') as f:
    reader = csv.reader(f)
    next(reader) # skip header
    for row in reader:
        T_fuel.append(float(row[0]))

with open('openmc_out_solid0_block_axial_avg_0005.csv') as f:
    reader = csv.reader(f)
    next(reader) # skip header
    for row in reader:
        T_block.append(float(row[0]))

fig, ax = plt.subplots()
plt.errorbar(z, avg_q, yerr=q_stdev, fmt='.k', capsize=3.0, marker = 'o', color = 'k')
plt.xlabel('Axial Position (m)')
plt.ylabel('Radially-Averaged Power (MW/m$^3$)')
plt.grid()
plt.savefig('q_axial.pdf', bbox_inches='tight')
plt.close()

fig, ax = plt.subplots()
plt.plot(z, T_fuel, marker = 'o', color = 'r', label='Fuel Compact')
plt.plot(z, T_block, marker = 'o', color = 'b', label='Graphite Block')
plt.xlabel('Axial Position (m)')
plt.ylabel('Radially-Averaged Temperature (K)')
plt.grid()
plt.savefig('T_axial.pdf', bbox_inches='tight')
plt.close()
