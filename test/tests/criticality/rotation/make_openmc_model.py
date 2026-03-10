import openmc
import numpy as np
from argparse import ArgumentParser
import common_input as specs


model = openmc.Model()
# materials
fuel = openmc.Material(name="fuel")
fuel.add_element("U", 1.0, enrichment=19.95)
fuel.set_density("g/cm3", 17.3)

water = openmc.Material(name="water")
water.add_elements_from_formula("H2O")
water.set_density("g/cm3", 1.0)

model.materials = openmc.Materials([fuel, water])

# geometry
z_min = openmc.ZPlane(z0=0.0, boundary_type="vacuum")
z_max = openmc.ZPlane(z0=specs.height, boundary_type="vacuum")
x_divider = openmc.XPlane(x0=0.0)  # at rotation=zero

cyl_inner = openmc.ZCylinder(r=specs.r_inner)
cyl_outer = openmc.ZCylinder(r=specs.r_outer, boundary_type="vacuum")

inner_fuel_cell = openmc.Cell(
    region=+z_min & -z_max & -x_divider & -cyl_inner, fill=fuel
)
inner_mod_cell = openmc.Cell(
    region=+z_min & -z_max & +x_divider & -cyl_inner, fill=water
)
outer_fuel_cell = openmc.Cell(
    region=+z_min & -z_max & -x_divider & +cyl_inner & -cyl_outer, fill=fuel
)
outer_mod_cell = openmc.Cell(
    region=+z_min & -z_max & +x_divider & +cyl_inner & -cyl_outer, fill=water
)

inner_cyl_univ = openmc.Universe(cells=[inner_fuel_cell, inner_mod_cell])
inner_cyl_cell = openmc.Cell(region=-cyl_inner & +z_min & -z_max, fill=inner_cyl_univ)
model.geometry = openmc.Geometry([inner_cyl_cell, outer_fuel_cell, outer_mod_cell])

# build settings
settings = openmc.Settings()
settings.batches = 50
settings.inactive = 10
settings.particles = 10000

# Create an initial uniform spatial source distribution over the whole geometry, the first generation will fix it to be only fissionable sites
radii = openmc.stats.Uniform(0.0, specs.r_outer)
angles = openmc.stats.Uniform(0, 2 * np.pi)
heights = openmc.stats.Uniform(0.0, specs.height)
cylindrical_source = openmc.stats.CylindricalIndependent(r=radii, phi=angles, z=heights)
settings.source = openmc.IndependentSource(space=cylindrical_source)
settings.output = {"summary": False}
settings.temperature = {
    "default": 294.0,
    "method": "nearest",
    "range": (294.0, 1600.0),
}
model.settings = settings


def rotate_inner_cyl(rotation):
    inner_cyl_cell.rotation = (0.0, 0.0, rotation)  # rotate about z axis


def build_parser():
    ap = ArgumentParser()
    ap.add_argument(
        "-r",
        "--rotation",
        dest="rotation",
        type=float,
        default=0.0,
        help="The rotation in degrees to rotate the inner cylinder",
    )
    ap.add_argument(
        "-s",
        "--search",
        dest="search",
        action="store_true",
        help="Whether to run a criticality search on the built model. If false, model is built only.",
    )
    return ap.parse_args()


if __name__ == "__main__":
    args = build_parser()
    rotate_inner_cyl(args.rotation)
    model.export_to_model_xml()
    if args.search:
        result = model.keff_search(
            rotate_inner_cyl, x0=0, x1=180, output=True, k_tol=1e-3, sigma_final=1e-3
        )
    else:
        model.run()
