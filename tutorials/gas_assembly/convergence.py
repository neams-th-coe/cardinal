import numpy as np
import matplotlib
import matplotlib.pyplot as plt
import csv

plt.rcParams.update({'font.size': 14})

# absolute tolerance in temperature to consider convergence
temperature_tolerance = 2.0

# plot the convergence quantities as a function of iteration
n_its = 10

k = []
k_error = []
max_Tfuel = []
max_Tblock = []
max_Tfluid = []

with open('openmc_out.csv') as f:
  reader = csv.reader(f)
  next(reader) # skip header and "initial" output before any time-stepping
  next(reader)
  for row in reader:
    k.append(float(row[2]))
    k_error.append(float(row[3]))
    max_Tfluid.append(float(row[4]))

with open('openmc_out_bison0.csv') as f:
  reader = csv.reader(f)
  next(reader) # skip header and "initial" output before any time-stepping
  next(reader)
  for row in reader:
    max_Tfuel.append(float(row[3]))
    max_Tblock.append(float(row[2]))

for i in range(len(k) - 1):
  difference = np.abs(k[i + 1] - k[i])
  if (difference < k_error[i]):
    print('k converged at iteration ', i + 1)

iterations = np.arange(1, n_its + 1, 1)

iteration_ticks = np.arange(1, n_its + 1, 1)
openmc_labels = []
for i in range(n_its):
  openmc_labels.append(str(i + 1) + 'B')

plt.errorbar(iterations, k, yerr=k_error, fmt='.k', capsize=3.0, marker='o', color='k', linestyle = '-')
plt.xlabel('Picard Iteration')
plt.ylabel('$k$')
plt.grid()
plt.xticks(iteration_ticks, openmc_labels)
plt.savefig('k.png', bbox_inches="tight")
plt.close()

fig, ax = plt.subplots()
iteration_ticks = np.arange(1, n_its + 1, 1)
solid_labels = []
for i in range(n_its):
  solid_labels.append(str(i + 1) + 'A')

for i in range(n_its - 1):
  difference = np.abs(max_Tfuel[i + 1] - max_Tfuel[i])
  if (difference < temperature_tolerance):
    print('Tfuel converged at iteration ', i + 1)
for i in range(n_its - 1):
  difference = np.abs(max_Tblock[i + 1] - max_Tblock[i])
  if (difference < temperature_tolerance):
    print('Tblock converged at iteration ', i + 1)

ax.plot(iterations, max_Tfuel[0:(n_its)], marker = 'o', color='k', label='Fuel Compact')
ax.plot(iterations, max_Tblock[0:(n_its)], marker = 'o', color='r', label='Graphite Block')

ax.grid()
ax.set_xlabel('Picard Iteration')
ax.set_ylabel('Maximum Temperature (K)', color='k')
plt.xticks(iteration_ticks, solid_labels)
ax.legend(loc='upper right', framealpha=1.0, fontsize=12)
plt.savefig('solid_temperatures.png', bbox_inches="tight")
plt.close()

iteration_ticks = np.arange(1, n_its + 1, 1)
fluid_labels = []
for i in range(len(max_Tfluid)):
  fluid_labels.append(str(i + 1) + 'C')

for i in range(len(max_Tfluid) - 1):
  difference = np.abs(max_Tfluid[i + 1] - max_Tfluid[i])
  if (difference < temperature_tolerance):
    print('Tfluid converged at iteration ', i + 1)

plt.plot(iterations, max_Tfluid[0:(n_its)], marker='o', color='k')
plt.grid()
plt.xlabel('Picard Iteration')
plt.ylabel('Maximum Fluid Temperature (K)')
plt.xticks(iteration_ticks, fluid_labels)
plt.savefig('fluid_temperatures.png', bbox_inches="tight")
