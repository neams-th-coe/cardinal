import openmc
import openmc.mgxs as mgxs
import numpy as np

L = 100.0
half_yz = 1.0
void_l = 10.0

xmin_i = openmc.XPlane(x0=-L / 2)
xmax_i = openmc.XPlane(x0=L / 2)
ymin_i = openmc.YPlane(y0=-half_yz)
ymax_i = openmc.YPlane(y0=half_yz)
zmin_i = openmc.ZPlane(z0=-half_yz)
zmax_i = openmc.ZPlane(z0=half_yz)

xmin_o = openmc.XPlane(x0=-L / 2 - void_l, boundary_type="vacuum")
xmax_o = openmc.XPlane(x0=L / 2 + void_l, boundary_type="vacuum")
ymin_o = openmc.YPlane(y0=-half_yz - void_l, boundary_type="vacuum")
ymax_o = openmc.YPlane(y0=half_yz + void_l, boundary_type="vacuum")
zmin_o = openmc.ZPlane(z0=-half_yz - void_l, boundary_type="vacuum")
zmax_o = openmc.ZPlane(z0=half_yz + void_l, boundary_type="vacuum")

slab_region = +xmin_i & -xmax_i & +ymin_i & -ymax_i & +zmin_i & -zmax_i
outer_region = +xmin_o & -xmax_o & +ymin_o & -ymax_o & +zmin_o & -zmax_o
void_region = outer_region & ~slab_region

slab = openmc.Material(1, "slab")
slab.set_density("macro", 1.0)
slab.add_macroscopic("slab_xs")

slab_cell = openmc.Cell(region=slab_region, fill=slab)
void_cell = openmc.Cell(region=void_region)
geometry = openmc.Geometry(openmc.Universe(cells=[slab_cell, void_cell]))

groups = mgxs.EnergyGroups(group_edges=[0.0, 20.0e6])
temps = np.array([293.0, 393.0])

xs = openmc.XSdata(
    "slab_xs", energy_groups=groups, temperatures=temps, num_delayed_groups=0
)
xs.order = 0

Sig_t_293 = 0.50
Sig_s_293 = 0.45
Sig_f_293 = 0.05
nu = 2.5
kappa = 200.0e6

for T in temps:
    scale = 293.0 / T
    Sig_t = Sig_t_293 * scale
    Sig_s = Sig_s_293 * scale
    Sig_f = Sig_f_293 * scale

    xs.set_total(np.array([Sig_t]), temperature=T)
    xs.set_scatter_matrix(np.array([[[Sig_s]]]), temperature=T)
    xs.set_absorption(np.array([Sig_f]), temperature=T)
    xs.set_fission(np.array([Sig_f]), temperature=T)
    xs.set_nu_fission(np.array([nu * Sig_f]), temperature=T)
    xs.set_kappa_fission(np.array([kappa * Sig_f]), temperature=T)

lib = openmc.MGXSLibrary(groups)
lib.add_xsdata(xs)
lib.export_to_hdf5("one_group.h5")

model = openmc.Model()
model.geometry = geometry
model.materials = openmc.Materials([slab])
model.materials.cross_sections = "one_group.h5"

model.settings.energy_mode = "multi-group"
model.settings.batches = 20
model.settings.inactive = 10
model.settings.particles = 1000
model.settings.source = openmc.IndependentSource(
    space=openmc.stats.Box((-L / 2, -half_yz, -half_yz), (L / 2, half_yz, half_yz))
)
model.settings.temperature = {
    "default": 293,
    "method": "nearest",
    "range": (float(temps.min()), float(temps.max())),
}

model.export_to_model_xml()
