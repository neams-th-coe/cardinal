import openmc

geom = openmc.Geometry.from_xml()
vol_calc = openmc.VolumeCalculation.from_hdf5('volume_1.h5')
geom.add_volume_information(vol_calc)

uco = geom.get_materials_by_name('Uranium')[0]
mass = sum(uco.get_mass(nuc) for nuc in ['U234', 'U235', 'U236', 'U238'])
print(f'U mass = {mass} g')
