#! coding:utf-8

"""Create a cube mesh with 8 HEX8 cells.

"""

# Generic import
import gmsh
import numpy as np

filename = "test.msh"
length = 0.1  # [m]
n_points = 5

# Initialize gmsh
gmsh.initialize()
gmsh.option.setNumber("Mesh.MshFileVersion", 2.2)
gmsh.option.setNumber("Mesh.SaveAll", 0)
gmsh.option.setNumber("Mesh.RecombinationAlgorithm", 3)

# Model
model = gmsh.model
model.add("test")

# Points
p1 = model.geo.addPoint(0., 0., 0.)
p2 = model.geo.addPoint(length, 0., 0.)
p3 = model.geo.addPoint(length, length, 0.)
p4 = model.geo.addPoint(0., length, 0.)

# Lines
l1 = model.geo.addLine(p1, p2)
l2 = model.geo.addLine(p2, p3)
l3 = model.geo.addLine(p3, p4)
l4 = model.geo.addLine(p4, p1)

# Planes (with curve loops)
cl1 = model.geo.addCurveLoop([l1, l2, l3, l4])
ps1 = model.geo.addPlaneSurface([cl1])

# Transfinite curve
model.geo.mesh.setTransfiniteCurve(1, n_points, "Progression", 1.0)
model.geo.mesh.setTransfiniteCurve(2, n_points, "Progression", 1.0)
model.geo.mesh.setTransfiniteCurve(3, n_points, "Progression", 1.0)
model.geo.mesh.setTransfiniteCurve(4, n_points, "Progression", 1.0)

# Transfinite surface
model.geo.mesh.setTransfiniteSurface(1)

model.geo.synchronize()

# Extrusion
extrude_vector = [0., 0., length]
model.geo.extrude([(2, 1)], *extrude_vector, numElements=[n_points - 1], recombine=True)

model.geo.synchronize()

# Physical groups
model.addPhysicalGroup(2, [13], name="inlet")
model.addPhysicalGroup(2, [21], name="outlet")
model.addPhysicalGroup(2, [17, 25, 26, 1], name="walls")
model.addPhysicalGroup(3, [1], name="test")

# Generate mesh
for s in gmsh.model.getEntities():
    gmsh.model.mesh.setRecombine(s[0], s[1])

gmsh.model.mesh.generate(3)
gmsh.model.mesh.setOrder(2)

gmsh.write(filename)
#gmsh.fltk.run()  # Runtime visualization
gmsh.finalize()
