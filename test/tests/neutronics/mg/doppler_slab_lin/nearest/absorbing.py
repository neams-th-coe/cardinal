import os
from argparse import ArgumentParser
import numpy as np
import openmc
import openmc.mgxs

# The overall model container.
doppler_slab_model = openmc.Model()

# Parameters specified in the benchmark document
sigma0 = 4.0     # barns
alpha = -0.0001  # 1 / cm-K
N0 = 0.025       # atom/barn-cm
T0 = 293.6       # Kelvin

# User settings
dT = 5.0        # Temperature spacing we want in our data set
Tmin = T0        # Minimum temperature for data set
Tmax = 1200.0    # Maximum temperature for data set
height = 200.0   #Length in x direction
###############################################################
# Axial subdivisions.
ap = ArgumentParser()
ap.add_argument('-n', dest='n_axial', type=int, default=20,
                help='Number of axial cell divisions')
args = ap.parse_args()
N = args.n_axial
###############################################################

temps = list(np.arange(np.floor(T0), np.ceil(Tmax), dT))

# Generate the multigroup cross section data set
groups = openmc.mgxs.EnergyGroups(np.logspace(1, 20, 2))
abs_xsdata = openmc.XSdata('abs', groups, temperatures=temps)
abs_xsdata.order = 0

void_xsdata = openmc.XSdata('void', groups, temperatures=temps)
void_xsdata.order = 0

# The scattering matrix is ordered with incoming groups as rows and outgoing groups as columns
# (i.e., below the diagonal is up-scattering).
scatter_matrix = \
    [[[0.0 ]]]
scatter_matrix = np.array(scatter_matrix)
scatter_matrix = np.rollaxis(scatter_matrix, 0, 3)

for i in range(len(temps)):
  E = N0*sigma0 *(1+(alpha/(sigma0*N0))*(temps[i]-T0))
  if E < 0.0:
     E = 0.0
  abs_xsdata.set_total([E], temperature=temps[i])
  abs_xsdata.set_absorption([E], temperature=temps[i])
  abs_xsdata.set_scatter_matrix(scatter_matrix, temperature=temps[i])

  void_xsdata.set_total([0.0], temperature=temps[i])
  void_xsdata.set_absorption([0.0], temperature=temps[i])
  void_xsdata.set_scatter_matrix(scatter_matrix, temperature=temps[i])


# Initialize the library
mg_cross_sections_file = openmc.MGXSLibrary(groups)

# Add the absorber data to it
mg_cross_sections_file.add_xsdata(abs_xsdata)
mg_cross_sections_file.add_xsdata(void_xsdata)

# And write to disk
mg_cross_sections_file.export_to_hdf5('mgxs.h5')

# For every cross section data set in the library, assign an openmc.Macroscopic object to a material
materials = {}
for xs in ['abs', 'void']:
    materials[xs] = openmc.Material(name=xs)
    materials[xs].set_density('macro', 1.)
    materials[xs].add_macroscopic(xs)

# Instantiate a Materials collection, register all Materials, and export to XML
doppler_slab_model.materials += materials.values()

# Set the location of the cross sections file to our pre-written set
doppler_slab_model.materials.cross_sections = 'mgxs.h5'

# geometry
#number of axial coords
###############################################
axial_coords = np.linspace(0.0, height, N + 1)
###############################################
y_plane1 = openmc.YPlane(0, boundary_type = 'vacuum')
y_plane2 = openmc.YPlane(2.0, boundary_type = 'vacuum')
z_plane1 = openmc.ZPlane(0, boundary_type = 'vacuum')
z_plane2 = openmc.ZPlane(2.0, boundary_type = 'vacuum')

container = +y_plane1 & -y_plane2 & +z_plane1 & -z_plane2

#x planes
l_plane = openmc.XPlane(x0=-1.0, boundary_type = 'vacuum')
x_surfaces = [openmc.XPlane(x0=coord) for coord in axial_coords]
x_surfaces[-1].boundary_type = 'vacuum'

abs_cells = []
for i in range(N):
   layer = +x_surfaces[i] & -x_surfaces[i + 1]
   cell = openmc.Cell(fill = materials['abs'], region = layer & container)
   cell.temperature = temps[min(i, len(temps) - 1)]
   abs_cells.append(cell)

# A cell where particles spawn.
src_cell = openmc.Cell(fill = materials['void'], region = +l_plane & -x_surfaces[0] & container)
abs_cells.append(src_cell)

root = openmc.Universe(name = 'root')
root.add_cells(abs_cells)
doppler_slab_model.geometry = openmc.Geometry(root)

# settings
doppler_slab_model.settings.energy_mode = 'multi-group'
doppler_slab_model.settings.run_mode = 'fixed source'

# source in one side
source = openmc.IndependentSource(space=openmc.stats.Box((-1.0, 0.0, 0.0), (0.0, 2.0, 2.0)),
                                  angle=openmc.stats.Monodirectional(reference_uvw = [1.0, 0.0, 0.0]))
doppler_slab_model.settings.source = [source]
doppler_slab_model.settings.temperature = {
   'default': 300.0,
   'method': 'nearest',
   'tolerance': 10.0,
   'range': (Tmin, Tmax)
}

doppler_slab_model.settings.batches = 1000
doppler_slab_model.settings.inactive = 0
doppler_slab_model.settings.particles = 5000

# Dump the full model to disk.
doppler_slab_model.export_to_model_xml()
