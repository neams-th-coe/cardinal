#! python

import openmc

# create an object for the DAGMC model
dagmc_universe = openmc.DAGMCUniverse('krusty_fuel.h5m')

model = openmc.Model.from_model_xml('original_model.xml')

fuel_cell = model.geometry.get_cells_by_fill_name('U-7.65Mo Fuel')[0]

print("Inserting the CAD model into the following cell:")
print(fuel_cell)

fuel_cell.fill = dagmc_universe

model.export_to_model_xml()
