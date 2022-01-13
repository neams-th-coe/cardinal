import openmc

# This file defines the materials used in the various Radiant OpenMC models
# to ensure consistency. A number of specifications that are only used to set up
# these materials (and are not used in any other files) are defined here.

enrichment = 0.155        # U-235 enrichment (weight percent)
enrichment_234 = 2e-3     # U-234 enrichment (weight percent)
kernel_density = 10820    # fissile kernel density (kg/m3)
buffer_density = 1050     # buffer density (kg/m3)
PyC_density = 1900        # PyC density (kg/m3)
SiC_density = 3203        # SiC density (kg/m3)
matrix_density = 1700     # graphite matrix density (kg/m3)

# ----- uranium oxycarbide fuel ----- #
m_fuel = openmc.Material(name='fuel')
mass_234 = openmc.data.atomic_mass('U234')
mass_235 = openmc.data.atomic_mass('U235')
mass_238 = openmc.data.atomic_mass('U238')

n_234 = enrichment_234 / mass_234
n_235 = enrichment / mass_235
n_238 = (1.0 - enrichment - enrichment_234) / mass_238
total_n = n_234 + n_235 + n_238

m_fuel.add_nuclide('U234', n_234 / total_n)
m_fuel.add_nuclide('U235', n_235 / total_n)
m_fuel.add_nuclide('U238', n_238 / total_n)
m_fuel.add_element('C'   , 1.50)
m_fuel.add_element('O'   , 0.50)
m_fuel.set_density('kg/m3', kernel_density)

# ----- graphite buffer ----- #
m_graphite_c_buffer = openmc.Material(name='buffer')
m_graphite_c_buffer.add_element('C', 1.0)
m_graphite_c_buffer.add_s_alpha_beta('c_Graphite')
m_graphite_c_buffer.set_density('kg/m3', buffer_density)

# ----- pyrolitic carbon ----- #
m_graphite_pyc = openmc.Material(name='pyc')
m_graphite_pyc.add_element('C', 1.0)
m_graphite_pyc.add_s_alpha_beta('c_Graphite')
m_graphite_pyc.set_density('kg/m3', PyC_density)

# ----- silicon carbide ----- #
m_sic = openmc.Material(name='sic')
m_sic.add_element('C' , 1.0)
m_sic.add_element('Si', 1.0)
m_sic.set_density('kg/m3', SiC_density)

# ----- matrix graphite ----- #
m_graphite_matrix = openmc.Material(name='graphite moderator')
m_graphite_matrix.add_element('C', 1.0)
m_graphite_matrix.add_s_alpha_beta('c_Graphite')
m_graphite_matrix.set_density('kg/m3', matrix_density)

# ----- reflector graphite ----- #
m_graphite_reflector = openmc.Material(name='graphite reflector')
m_graphite_reflector.add_element('C', 1.0)
m_graphite_reflector.add_s_alpha_beta('c_Graphite')
m_graphite_reflector.set_density('kg/m3', matrix_density)

# ----- helium coolant ----- #
m_coolant = openmc.Material(name='Helium coolant')
m_coolant.add_element('He', 1.0, 'ao')
# we don't set density here because we'll set it as a function of temperature
