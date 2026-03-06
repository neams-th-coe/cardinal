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

#--------------------------------------------------------------------------------------------------------------------------#
# This is based on the C5G7 reactor physics benchmark problem extension case as described in:                              #
# "Benchmark on Deterministic Transport Calculations Without Spatial Homogenisation: MOX Fuel Assembly 3-D Extension Case" #
# [NEA/NSC/DOC(2005)16]                                                                                                    #
# https://www.oecd-nea.org/upload/docs/application/pdf/2019-12/nsc-doc2005-16.pdf                                          #
#                                                                                                                          #
# The original C5G7 benchmark is defined with multi-group cross sections. To account for                                   #
# continuous energy spectral effects, we chose to use the material properties provided in:                                 #
# "Proposal for a Second Stage of the Benchmark on Power Distributions Within Assemblies"                                  #
# [NEA/NSC/DOC(96)2]                                                                                                       #
# https://www.oecd-nea.org/upload/docs/application/pdf/2020-01/nsc-doc96-02-rev2.pdf                                       #
#--------------------------------------------------------------------------------------------------------------------------#

import common as specs

import openmc
import numpy as np
import copy
from argparse import ArgumentParser

# A helper function which discretizes the source regions in a pincell. This is performed
# with equal volume subdivision to ensure each region has the same likelyhood of getting
# hit by a ray (relative to each other region).
def build_sr_pin(pin_fill, num_rings, num_sectors, fuel_or_surf) -> openmc.Universe:
  avg_volume = np.pi * (fuel_or_surf.r**2) / num_rings
  radii = np.zeros(num_rings + 1)
  angles = [2 * j * openmc.pi / num_sectors for j in range(num_sectors)]
  radii[0] = 0.0

  # Calculate radii for equal-volume fuel rings.
  for i in range(1, num_rings + 1):
    radii[i] = np.sqrt(radii[i-1]**2 + avg_volume / np.pi)

  print('Equal volume slices:', avg_volume)
  print('Fuel radii:', radii)

  radial_surf = [openmc.ZCylinder(x0=0, y0=0, r=surf_r) for surf_r in radii[:-1]]
  radial_surf.append(fuel_or_surf)
  azimuthal_surf = [openmc.Plane(a=-openmc.sin(angle), b=openmc.cos(angle), c=0, d=0) for angle in angles]

  pincell_base = openmc.Universe()
  cell_id = 0
  if num_sectors < 2:
    # In the case where there is only a single azimuthal region, we just subdivide in r.
    for i in range(num_rings):
      r_region = -radial_surf[i+1]
      if radial_surf[i].r > 0.0:
        r_region &= +radial_surf[i]

      cell = openmc.Cell(fill=pin_fill, region=r_region, name=f'Fuel cell {cell_id}')
      pincell_base.add_cell(cell)
      cell_id += 1
  else:
    # In the case where there are multiple azimuthal segments, we subdivide in r and theta.
    for i in range(num_rings):
      r_region = -radial_surf[i+1]
      if radial_surf[i].r > 0.0:
        r_region &= +radial_surf[i]
      for j in range(num_sectors):
        azimuthal_region = +azimuthal_surf[j] & -azimuthal_surf[(j+1) % num_sectors]
        cell = openmc.Cell(fill=pin_fill, region=r_region & azimuthal_region, name=f'Fuel cell {cell_id}')
        pincell_base.add_cell(cell)
        cell_id += 1
  return pincell_base

def main() -> None:
  ap = ArgumentParser()
  ap.add_argument('-r', dest='rr', action='store_true',
                  help='If the random ray model should be run.')
  ap.add_argument('--linear', dest='linear', action='store_true',
                  help='If the random ray model should use linear source regions.')
  args = ap.parse_args()
  run_random_ray = args.rr
  use_linear_source = args.linear

  ## Fuel region: UO2 at ~1% enriched.
  uo2_comp = 1.0e24 * np.array([8.65e-4, 2.225e-2, 4.622e-2])
  uo2_frac = uo2_comp / np.sum(uo2_comp)
  uo2 = openmc.Material(name = 'UO2 Fuel')
  uo2.add_nuclide('U235', uo2_frac[0], percent_type = 'ao')
  uo2.add_nuclide('U238', uo2_frac[1], percent_type = 'ao')
  uo2.add_element('O', uo2_frac[2], percent_type = 'ao')
  uo2.set_density('atom/cm3', np.sum(uo2_comp))

  ## Cladding, pure Zr metal.
  zr = openmc.Material(name = 'Zr Cladding')
  zr.add_element('Zr', 1.0, percent_type = 'ao')
  zr.set_density('atom/cm3', 1.0e24 * 4.30e-2)

  ## Moderator and coolant, boronated water.
  h2o_comp = 3.0 * 1.0e24 * np.array([3.35e-2, 2.78e-5])
  h2o_frac = h2o_comp / np.sum(h2o_comp)
  h2o = openmc.Material(name = 'H2O Moderator')
  h2o.add_element('H', 2.0 * h2o_frac[0], percent_type = 'ao')
  h2o.add_element('O', h2o_frac[0], percent_type = 'ao')
  h2o.add_element('B', h2o_frac[1], percent_type = 'ao')
  h2o.set_density('atom/cm3', np.sum(h2o_comp))
  h2o.add_s_alpha_beta('c_H_in_H2O')
  print('Water mass density:', h2o.get_mass_density(), 'g/cm3')

  # Build the pincell geometry.
  # Some useful surfaces.
  fuel_pin_or = openmc.ZCylinder(r = specs.R_FUEL)
  fuel_gap_or = openmc.ZCylinder(r = specs.R_FUEL + specs.T_F_C_GAP)
  fuel_zr_or = openmc.ZCylinder(r = specs.R_FUEL + specs.T_F_C_GAP + specs.T_ZR_CLAD)
  fuel_bb = openmc.model.RectangularPrism(width = specs.PITCH, height = specs.PITCH, boundary_type = 'reflective')
  pin_bot = openmc.ZPlane(z0 = -0.5 * specs.HEIGHT, boundary_type = 'vacuum')
  pin_top = openmc.ZPlane(z0 = 0.5 * specs.HEIGHT, boundary_type = 'vacuum')

  ## Create cells for the cladding, gap, and water.
  gap_cell = openmc.Cell(fill = None, region = +fuel_pin_or & -fuel_gap_or, name = 'Gap Cell')
  clad_cell = openmc.Cell(fill = zr, region = +fuel_gap_or & -fuel_zr_or, name = 'Clad Cell')
  mod_cell = openmc.Cell(fill = h2o, region = +fuel_zr_or, name = 'Moderator Cell')

  ## Use the helper function to build a universe filled with subdivided
  ## fuel source regions.
  fuel_pin_uni = build_sr_pin(uo2, 3, 4, fuel_pin_or)
  ## Add the other cells to this universe.
  fuel_pin_uni.add_cells([gap_cell, clad_cell, mod_cell])

  ## Place the pin in a lattice for more efficient transport.
  one_d_lattice = openmc.RectLattice(name = 'Lattice')
  one_d_lattice.pitch = (specs.PITCH, specs.PITCH, specs.HEIGHT / specs.AXIAL_LAYERS)
  one_d_lattice.lower_left = (-0.5 * specs.PITCH, -0.5 * specs.PITCH, -0.5 * specs.HEIGHT)
  one_d_lattice.universes = [[[fuel_pin_uni]]] * specs.AXIAL_LAYERS
  lattice_cell = openmc.Cell(fill = one_d_lattice, region = -fuel_bb & +pin_bot & -pin_top)
  root_uni = openmc.Universe(cells=[lattice_cell])

  # The model container.
  ce_model = openmc.Model()
  ce_model.materials = openmc.Materials([uo2, zr, h2o])
  ce_model.geometry = openmc.Geometry(root=root_uni)
  src_ll = (-0.5 * specs.PITCH, -0.5 * specs.PITCH, -0.5 * specs.HEIGHT)
  src_ur = ( 0.5 * specs.PITCH,  0.5 * specs.PITCH,  0.5 * specs.HEIGHT)

  # Simulation settings.
  ce_model.settings.source.append(openmc.IndependentSource(space = openmc.stats.Box(lower_left = src_ll, upper_right = src_ur)))
  ce_model.settings.batches = 100
  ce_model.settings.inactive = 10
  ce_model.settings.particles = 1000
  ce_model.settings.temperature['method'] = 'interpolation'
  ce_model.settings.temperature['tolerance'] = 5000.0
  ce_model.settings.temperature['range'] = (294.0, 3000.0)
  ce_model.settings.temperature['multipole'] = True
  ce_model.settings.temperature['default'] = 0.5 * (573.0 + 623.0)

  ## Write the continuous-energy model if not running random ray.
  if not run_random_ray:
    print('Writing CE Monte Carlo model')
    ce_model.export_to_model_xml()
    return
  else:
    print('Writing MG random ray model')

  ## Otherwise, run autoconvert.
  rr_model = copy.deepcopy(ce_model)
  rr_model.convert_to_multigroup(
    method="material_wise",
    groups='CASMO-2',
    nparticles=5000,
    overwrite_mgxs_library=False,
    mgxs_path="mgxs.h5",
    correction="P0",
    temperatures = [294, 600, 900, 1200, 1500, 1800, 2100, 2400]
  )
  rr_model.convert_to_random_ray()

  ## Create a mesh to do cell-under-voxel decomposition of the moderator
  sr_depth_voxels = 10
  sr_mesh = openmc.RegularMesh().from_domain(rr_model.geometry)
  sr_mesh.dimension = (sr_depth_voxels, sr_depth_voxels, 1)
  ## Apply the source region mesh.
  rr_model.settings.random_ray['source_region_meshes'] = [(sr_mesh, [mod_cell])]

  ## Set the source region type and the transport stabilization.
  rr_model.settings.random_ray['diagonal_stabilization_rho'] = 1.0
  if use_linear_source:
    rr_model.settings.random_ray['source_shape'] = 'linear'
    print('- Using linear source regions')
  else:
    rr_model.settings.random_ray['source_shape'] = 'flat'
    print('- Using flat source regions')

  ## Write the final random ray model.
  rr_model.export_to_model_xml()

if __name__ == "__main__":
  main()
