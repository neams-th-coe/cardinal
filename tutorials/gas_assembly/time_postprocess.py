# This script is used to generate various plots in the z-direction of
# radially-averaged quantities using MOOSE vector postprocessor CSV output.

import matplotlib
import matplotlib.pyplot as plt
import numpy as np
import csv

n_plot_layers = 50

matplotlib.rcParams.update({'font.size': 14})
ms = 3

fig, ax = plt.subplots()

n_time_steps = 10
for time_step in range(1, n_time_steps):
  avg_q = []
  T_fuel = []
  T_block = []
  T_fluid = []
  T_fluid_wall = []
  pressure = []
  velocity = []

  h = 6.343
  dz = h / n_plot_layers
  z = np.linspace(dz / 2.0, h - dz / 2.0, n_plot_layers)

  with open('openmc_out_power_avg_000' + str(time_step) + '.csv') as f:
      reader = csv.reader(f)
      next(reader) # skip header
      for row in reader:
          avg_q.append(float(row[0]) / 1e6)

  ax.plot(z, avg_q, label='$i=${0}'.format(time_step), markersize=ms, marker='o')

ax.grid()
ax.set_xlabel('Axial Position (m)')
ax.set_ylabel('Power (MW)')
ax.legend(loc='lower right', ncol=2)
plt.savefig('q_iteration.png', bbox_inches='tight')
plt.close()
