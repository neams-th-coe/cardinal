import csv
import numpy as np
import glob
import os

Twall = []
Tbulk = []
flux = []

flux_files = glob.glob('solid_out_flux_axial_*')
flux_file = max(flux_files, key=os.path.getctime)
with open(flux_file) as f:
  reader = csv.reader(f, delimiter=',')
  next(f)
  for row in reader:
    flux.append(float(row[0]))

wall_files = glob.glob('solid_out_nek0_wall_*')
wall_file = max(wall_files, key=os.path.getctime)
with open(wall_file) as f:
  reader = csv.reader(f, delimiter=',')
  next(f)
  for row in reader:
    Twall.append(float(row[0]))

bulk_files = glob.glob('solid_out_nek0_bulk_*')
bulk_file = max(bulk_files, key=os.path.getctime)
with open(bulk_file) as f:
  reader = csv.reader(f, delimiter=',')
  next(f)
  for row in reader:
    Tbulk.append(float(row[0]))

h = []
for i in range(len(Twall)):
  h.append(flux[i] / (Twall[i] - Tbulk[i]))

print('The average heat transfer coefficient is (W/m^2K): ', np.mean(h))
