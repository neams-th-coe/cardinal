#!/usr/bin/env python3

from enum import Enum
import openmc

class Scenario(Enum):
    ALL_VACUUM = 1
    ALL_REFLECTIVE = 2
    MIX_BCS = 3
    INTERNAL_TRANSMISSION = 4
    INTERNAL_REFLECTIVE = 5

SCENARIO = Scenario.MIX_BCS # update MIX_BCS here with a different scenario from above


def boundary_type(name):
    if name == "interface":
        if SCENARIO == Scenario.INTERNAL_REFLECTIVE:
            return "reflective"

        return "transmission"

    if SCENARIO == Scenario.ALL_VACUUM:
        return "vacuum"

    if SCENARIO == Scenario.ALL_REFLECTIVE:
        return "reflective"

    if SCENARIO == Scenario.MIX_BCS:
        return {
            "left": "vacuum",
            "right": "vacuum",
            "bottom": "reflective",
            "top": "reflective",
            "front": "reflective",
            "back": "reflective",
        }[name]

    if SCENARIO in (
        Scenario.INTERNAL_TRANSMISSION,
        Scenario.INTERNAL_REFLECTIVE,
    ):
        return {
            "left": "reflective",
            "right": "reflective",
            "bottom": "reflective",
            "top": "vacuum",
            "front": "reflective",
            "back": "reflective",
        }[name]

    raise ValueError(f"Unhandled scenario: {SCENARIO}")

material_1 = openmc.Material(material_id=1)
material_1.depletable = True
material_1.set_density("g/cc", 11.0)
material_1.add_nuclide("U235", 0.0072, "ao")
material_1.add_nuclide("U238", 0.9928, "ao")

material_2 = openmc.Material(material_id=2)
material_2.depletable = True
material_2.set_density("g/cc", 11.0)
material_2.add_nuclide("U235", 0.0072, "ao")
material_2.add_nuclide("U238", 0.9928, "ao")

materials = openmc.Materials([material_1, material_2])

settings = openmc.Settings()
settings.run_mode = "eigenvalue"
settings.particles = 1000
settings.batches = 100
settings.inactive = 50

settings.source = openmc.IndependentSource(
    space=openmc.stats.Box(
        [-12.5, -12.5, -12.5],
        [87.5, 37.5, 12.5],
    ),
    strength=1.0,
)

settings.temperature = {
    "default": 500.0,
    "method": "interpolation",
    "range": (294.0, 3000.0),
    "tolerance": 1000.0,
}

x_min = -12.5
x_max = 87.5
y_min = -12.5
y_interface = 12.5
y_max = 37.5
z_min = -12.5
z_max = 12.5

left = openmc.XPlane(
    surface_id=1,
    x0=x_min,
    boundary_type=boundary_type("left"),
)
right = openmc.XPlane(
    surface_id=2,
    x0=x_max,
    boundary_type=boundary_type("right"),
)
bottom = openmc.YPlane(
    surface_id=3,
    y0=y_min,
    boundary_type=boundary_type("bottom"),
)
top = openmc.YPlane(
    surface_id=4,
    y0=y_max,
    boundary_type=boundary_type("top"),
)
front = openmc.ZPlane(
    surface_id=5,
    z0=z_min,
    boundary_type=boundary_type("front"),
)
back = openmc.ZPlane(
    surface_id=6,
    z0=z_max,
    boundary_type=boundary_type("back"),
)

interface = openmc.YPlane(
    surface_id=7,
    y0=y_interface,
    boundary_type=boundary_type("interface"),
)

outer_region = (
    +left
    & -right
    & +bottom
    & -top
    & +front
    & -back
)

cell_1 = openmc.Cell(
    cell_id=1,
    name="block_1",
    fill=material_1,
    region=outer_region & +interface,
)
cell_1.temperature = 500.0

cell_2 = openmc.Cell(
    cell_id=2,
    name="block_2",
    fill=material_2,
    region=outer_region & -interface,
)
cell_2.temperature = 500.0

geometry = openmc.Geometry([cell_1, cell_2])

model = openmc.Model(
    geometry=geometry,
    materials=materials,
    settings=settings,
)
model.export_to_model_xml("model.xml")

print(f"Wrote model.xml for {SCENARIO.name}")
