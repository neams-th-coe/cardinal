################################################################################
# KRUSTY 01 - 01/18/2024
# Geometry and materials taken from:
# [1] David I. Poston, Marc A. Gibson, Thomas Godfroy & Patrick R. McClure (2020) 
#     KRUSTY Reactor Design, Nuclear Technology, 206:sup1, S13-S30, 
#     DOI: 10.1080/00295450.2020.1725382
# [2] Patrick R. McClure, David I. Poston, Steven D. Clement, Louis Restrepo, 
#     Robert Miller & Manny Negrete (2020) KRUSTY Experiment: Reactivity Insertion
#     Accident Analysis, Nuclear Technology, 206:sup1, S43-S55, 
#     DOI: 10.1080/00295450.2020.1722544
#
################################################################################
import openmc
import numpy as np
import math

################################################################################
model = openmc.Model()
## Defining Materials

n_234 = 0.010
n_235 = 0.931
n_236 = 0.005
n_238 = 0.054

UMo = openmc.Material(name='U-7.65Mo Fuel')
UMo.set_density('g/cm3', 17.39) #edited to conserve mass (was 17.34)
UMo.add_nuclide('U234', 92.35*n_234)
UMo.add_nuclide('U235', 92.35*n_235)
UMo.add_nuclide('U236', 92.35*n_236)
UMo.add_nuclide('U238', 92.35*n_238)
UMo.add_element('Mo', 7.65)
UMo.add_element('C', 300*1.0e-6) #C in ppm

B4C = openmc.Material(name='B4C rod')
B4C.set_density('g/cm3', 2.15 )
B4C.add_element('C', 1)
B4C.add_nuclide('B10', 0.96*4)
B4C.add_nuclide('B11', 0.04*4)

Haynes230 = openmc.Material(name='Haynes230')
Haynes230.set_density('g/cm3', 8.97)
Haynes230.add_element('Ni', 65.7)
Haynes230.add_element('Cr', 24)
Haynes230.add_element('W', 15)
Haynes230.add_element('Mo', 3)
Haynes230.add_element('Mn', 1)
Haynes230.add_element('Si', 0.75)
Haynes230.add_element('C', 0.09)

MLI = openmc.Material(name='Mo Foil')
MLI.set_density('g/cm3', 10.2)
MLI.add_element('Mo', 1.0)

Cu = openmc.Material(name='Cu Foil')
Cu.set_density('g/cm3', 8.9 )
Cu.add_element('Cu', 1.0 )

Ni = openmc.Material(name='Ni heat pipe wick')
Ni.set_density('g/cm3', 8.9 )
Ni.add_element('Ni', 1.0)

HP_fld = openmc.Material(name='heat pipe fluid')
HP_fld.set_density('g/cm3', 0.1941) #to conserve 15 gm of Na (UPDATE!)
HP_fld.add_element('Na', 1.0)

SS = openmc.Material(name='SS')
SS.set_density('g/cm3', 7.99)
SS.add_element('C',  0.001900)
SS.add_element('Si', 0.010048)
SS.add_element('P',  0.000413)
SS.add_element('S',  0.000260)
SS.add_element('Cr', 0.181986)
SS.add_element('Mn', 0.010274)
SS.add_element('Fe', 0.666812)
SS.add_element('Ni', 0.113803)
SS.add_element('Mo', 0.014504)

helium = openmc.Material(name='Helium for gap')
helium.set_density('g/cm3', 0.001598)
helium.add_element('He', 2.4044e-4)

BeO = openmc.Material(name='BeO reflector')
BeO.set_density('g/cm3', 2.85 )
BeO.add_element('Be', 1)
BeO.add_element('O', 1)
#BeO.add_s_alpha_beta('c_Be')

SS316 = openmc.Material(name='Shield')
SS316.set_density('g/cm3', 7.99)
SS316.add_element('C',  0.001900)
SS316.add_element('Si', 0.010048)
SS316.add_element('P',  0.000413)
SS316.add_element('S',  0.000260)
SS316.add_element('Cr', 0.181986)
SS316.add_element('Mn', 0.010274)
SS316.add_element('Fe', 0.666812)
SS316.add_element('Ni', 0.113803)
SS316.add_element('Mo', 0.014504)

model.materials = openmc.Materials([UMo, B4C, Haynes230, MLI, Cu, Ni, SS, helium, BeO, SS316, HP_fld])


################################################################################
## Defining Geometry
# Parameters

Fuel_OD = 11.0
Fuel_ID = 4.0
Core_length = 25.0
Reflector_height = 12.49 # Assumed
Reflector_ring_bottom = 9 # Assumed
Reflector_ring_height = 28.8925 # 11.375 inch - from KRUSTY component-critical experiments

Clamps_OD = 12.13
Clamps_th = 0.318


MLI_th = (25.4e-6 + 101.6e-6)*10*100

Vac_can_OD = 13.3
Vac_can_length = 52.5
Vac_can_th = 0.305

SS_sleeve_th = 0.089

gap_BeO = 0.1

BeO_rings_ID = 14.5
BeO_rings_OD = 38.1
BeO_rings_th = 2.54

BeO_axref_height = 10.16

SS_shield_length = 63.1
SS_shield_ID = 41.0
SS_shield_OD = 101.9

HP_OD = 1.27
HP_th = 0.089

Cu_foil_th = 25e-6 *5*100 # ASSUMED

Ni_wick_th = 0.05 # ASSUMED

##########################################
# Surfaces

fuel_or = openmc.ZCylinder(r=Fuel_OD/2)
fuel_ir = openmc.ZCylinder(r=Fuel_ID/2)

clamps_or = openmc.ZCylinder(r=Clamps_OD/2)
clamps_ir = openmc.ZCylinder(r=Clamps_OD/2 - Clamps_th)

vaccan_or = openmc.ZCylinder(r=Vac_can_OD/2)
vaccan_ir = openmc.ZCylinder(r=Vac_can_OD/2 - Vac_can_th)
MLI_ir    = openmc.ZCylinder(r=Vac_can_OD/2 - Vac_can_th - MLI_th)

reflec_ir = openmc.ZCylinder(r=BeO_rings_ID/2)
sleeve_or = openmc.ZCylinder(r=BeO_rings_ID/2 - gap_BeO)
sleeve_ir = openmc.ZCylinder(r=BeO_rings_ID/2 - gap_BeO - SS_sleeve_th)

reflec_or = openmc.ZCylinder(r=BeO_rings_OD/2)
shield_ir = openmc.ZCylinder(r=SS_shield_ID/2)
shield_or = openmc.ZCylinder(r=SS_shield_OD/2, boundary_type='vacuum')

HP_0_foil_or = openmc.ZCylinder(x0=0.0, y0=5.175, r=HP_OD/2 + Cu_foil_th)
HP_0_or      = openmc.ZCylinder(x0=0.0, y0=5.175, r=HP_OD/2)
HP_0_ir      = openmc.ZCylinder(x0=0.0, y0=5.175, r=HP_OD/2 - HP_th)
HP_0_wick_ir = openmc.ZCylinder(x0=0.0, y0=5.175, r=HP_OD/2 - HP_th - Ni_wick_th)

HP_45_foil_or = openmc.ZCylinder(x0=3.659, y0=3.659, r=HP_OD/2 + Cu_foil_th)
HP_45_or      = openmc.ZCylinder(x0=3.659, y0=3.659, r=HP_OD/2)
HP_45_ir      = openmc.ZCylinder(x0=3.659, y0=3.659, r=HP_OD/2 - HP_th)
HP_45_wick_ir = openmc.ZCylinder(x0=3.659, y0=3.659, r=HP_OD/2 - HP_th - Ni_wick_th)

HP_90_foil_or = openmc.ZCylinder(x0=5.175, y0=0.0, r=HP_OD/2 + Cu_foil_th)
HP_90_or      = openmc.ZCylinder(x0=5.175, y0=0.0, r=HP_OD/2)
HP_90_ir      = openmc.ZCylinder(x0=5.175, y0=0.0, r=HP_OD/2 - HP_th)
HP_90_wick_ir = openmc.ZCylinder(x0=5.175, y0=0.0, r=HP_OD/2 - HP_th - Ni_wick_th)

HP_135_foil_or = openmc.ZCylinder(x0=3.659, y0=-3.659, r=HP_OD/2 + Cu_foil_th)
HP_135_or      = openmc.ZCylinder(x0=3.659, y0=-3.659, r=HP_OD/2)
HP_135_ir      = openmc.ZCylinder(x0=3.659, y0=-3.659, r=HP_OD/2 - HP_th)
HP_135_wick_ir = openmc.ZCylinder(x0=3.659, y0=-3.659, r=HP_OD/2 - HP_th - Ni_wick_th)

HP_180_foil_or = openmc.ZCylinder(x0=0.0, y0=-5.175, r=HP_OD/2 + Cu_foil_th)
HP_180_or      = openmc.ZCylinder(x0=0.0, y0=-5.175, r=HP_OD/2)
HP_180_ir      = openmc.ZCylinder(x0=0.0, y0=-5.175, r=HP_OD/2 - HP_th)
HP_180_wick_ir = openmc.ZCylinder(x0=0.0, y0=-5.175, r=HP_OD/2 - HP_th - Ni_wick_th)

HP_225_foil_or = openmc.ZCylinder(x0=-3.659, y0=-3.659, r=HP_OD/2 + Cu_foil_th)
HP_225_or      = openmc.ZCylinder(x0=-3.659, y0=-3.659, r=HP_OD/2)
HP_225_ir      = openmc.ZCylinder(x0=-3.659, y0=-3.659, r=HP_OD/2 - HP_th)
HP_225_wick_ir = openmc.ZCylinder(x0=-3.659, y0=-3.659, r=HP_OD/2 - HP_th - Ni_wick_th)

HP_270_foil_or = openmc.ZCylinder(x0=-5.175, y0=0.0, r=HP_OD/2 + Cu_foil_th)
HP_270_or      = openmc.ZCylinder(x0=-5.175, y0=0.0, r=HP_OD/2)
HP_270_ir      = openmc.ZCylinder(x0=-5.175, y0=0.0, r=HP_OD/2 - HP_th)
HP_270_wick_ir = openmc.ZCylinder(x0=-5.175, y0=0.0, r=HP_OD/2 - HP_th - Ni_wick_th)

HP_315_foil_or = openmc.ZCylinder(x0=-3.659, y0=3.659, r=HP_OD/2 + Cu_foil_th)
HP_315_or      = openmc.ZCylinder(x0=-3.659, y0=3.659, r=HP_OD/2)
HP_315_ir      = openmc.ZCylinder(x0=-3.659, y0=3.659, r=HP_OD/2 - HP_th)
HP_315_wick_ir = openmc.ZCylinder(x0=-3.659, y0=3.659, r=HP_OD/2 - HP_th - Ni_wick_th)

bottom = openmc.ZPlane(z0=0, boundary_type='vacuum')
core_bottom = openmc.ZPlane(z0=Reflector_height)
core_top = openmc.ZPlane(z0=Core_length+Reflector_height)
ref_top = openmc.ZPlane(z0=Core_length+2*Reflector_height)
reflector_ring_top = openmc.ZPlane(z0=Reflector_ring_bottom+Reflector_ring_height)
reflector_ring_bottom = openmc.ZPlane(z0=Reflector_ring_bottom)
top = openmc.ZPlane(z0=50, boundary_type='vacuum')

naxd  = 24  # number of core axial divisions
hcell = Core_length/naxd
ax=[]
for c in range(naxd+1):
    ax.append(openmc.ZPlane(z0=(Reflector_height+c*hcell)))
    
nrngs = 24 # number of radial rings
rcell = (Fuel_OD/2 - Fuel_ID/2) / nrngs
rx=[]
for j in range(nrngs+1):
    rx.append(openmc.ZCylinder(r=Fuel_ID/2 +j*rcell))

##########################################
# Cells

fuel_cells = []

for c in range(naxd):
    for j in range(nrngs):
        fuel_cells.append(openmc.Cell(name='fuel a{} r{}'.format(c+1, j+1),fill=UMo,region=-rx[j+1] & +rx[j]& +HP_0_foil_or & \
                    +HP_45_foil_or & +HP_90_foil_or & +HP_135_foil_or & +HP_180_foil_or & \
                    +HP_225_foil_or & +HP_270_foil_or & +HP_315_foil_or & - ax[c+1] & + ax[c]))

B4C_region = openmc.Cell(name='B4C_region',region=-fuel_ir & +bottom & -top)

empty_top = openmc.Cell(name='empty_top', region=-fuel_or & +fuel_ir& +HP_0_foil_or & \
                    +HP_45_foil_or & +HP_90_foil_or & +HP_135_foil_or & +HP_180_foil_or & \
                    +HP_225_foil_or & +HP_270_foil_or & +HP_315_foil_or & \
                    +ref_top & -top)
                    
reflector_top = openmc.Cell(name='reflector_top',fill=BeO,region=-fuel_or & +fuel_ir& +HP_0_foil_or & \
                    +HP_45_foil_or & +HP_90_foil_or & +HP_135_foil_or & +HP_180_foil_or & \
                    +HP_225_foil_or & +HP_270_foil_or & +HP_315_foil_or & \
                    +core_top & -ref_top)
                    
reflector_bottom = openmc.Cell(name='reflector_bottom',fill=BeO,region=-fuel_or & +fuel_ir& +HP_0_foil_or & \
                    +HP_45_foil_or & +HP_90_foil_or & +HP_135_foil_or & +HP_180_foil_or & \
                    +HP_225_foil_or & +HP_270_foil_or & +HP_315_foil_or & \
                    -core_bottom & +bottom)
                    
HP0_fld = openmc.Cell(name='HP0_fld', fill = HP_fld, region = -HP_0_wick_ir & +bottom & -top)    
HP0_wck = openmc.Cell(name='HP0_wck', fill = Ni, region = +HP_0_wick_ir & -HP_0_ir & +bottom & -top) 
HP0_wall = openmc.Cell(name='HP0_wall', fill = SS, region = +HP_0_ir & -HP_0_or & +bottom & -top) 
HP0_foil = openmc.Cell(name='HP0_foil', fill = Cu, region = +HP_0_or & -HP_0_foil_or & +bottom & -top) 

HP45_fld = openmc.Cell(name='HP45_fld', fill = HP_fld, region = -HP_45_wick_ir & +bottom & -top)    
HP45_wck = openmc.Cell(name='HP45_wck', fill = Ni, region = +HP_45_wick_ir & -HP_45_ir & +bottom & -top) 
HP45_wall = openmc.Cell(name='HP45_wall', fill = SS, region = +HP_45_ir & -HP_45_or & +bottom & -top) 
HP45_foil = openmc.Cell(name='HP45_foil', fill = Cu, region = +HP_45_or & -HP_45_foil_or & +bottom & -top)

HP90_fld = openmc.Cell(name='HP90_fld', fill = HP_fld, region = -HP_90_wick_ir & +bottom & -top)    
HP90_wck = openmc.Cell(name='HP90_wck', fill = Ni, region = +HP_90_wick_ir & -HP_90_ir & +bottom & -top) 
HP90_wall = openmc.Cell(name='HP90_wall', fill = SS, region = +HP_90_ir & -HP_90_or & +bottom & -top) 
HP90_foil = openmc.Cell(name='HP90_foil', fill = Cu, region = +HP_90_or & -HP_90_foil_or & +bottom & -top)

HP135_fld = openmc.Cell(name='HP135_fld', fill = HP_fld, region = -HP_135_wick_ir & +bottom & -top)    
HP135_wck = openmc.Cell(name='HP135_wck', fill = Ni, region = +HP_135_wick_ir & -HP_135_ir & +bottom & -top) 
HP135_wall = openmc.Cell(name='HP135_wall', fill = SS, region = +HP_135_ir & -HP_135_or & +bottom & -top) 
HP135_foil = openmc.Cell(name='HP135_foil', fill = Cu, region = +HP_135_or & -HP_135_foil_or & +bottom & -top)

HP180_fld = openmc.Cell(name='HP180_fld', fill = HP_fld, region = -HP_180_wick_ir & +bottom & -top)    
HP180_wck = openmc.Cell(name='HP180_wck', fill = Ni, region = +HP_180_wick_ir & -HP_180_ir & +bottom & -top) 
HP180_wall = openmc.Cell(name='HP180_wall', fill = SS, region = +HP_180_ir & -HP_180_or & +bottom & -top) 
HP180_foil = openmc.Cell(name='HP180_foil', fill = Cu, region = +HP_180_or & -HP_180_foil_or & +bottom & -top)

HP225_fld = openmc.Cell(name='HP225_fld', fill = HP_fld, region = -HP_225_wick_ir & +bottom & -top)    
HP225_wck = openmc.Cell(name='HP225_wck', fill = Ni, region = +HP_225_wick_ir & -HP_225_ir & +bottom & -top) 
HP225_wall = openmc.Cell(name='HP225_wall', fill = SS, region = +HP_225_ir & -HP_225_or & +bottom & -top) 
HP225_foil = openmc.Cell(name='HP225_foil', fill = Cu, region = +HP_225_or & -HP_225_foil_or & +bottom & -top)

HP270_fld = openmc.Cell(name='HP270_fld', fill = HP_fld, region = -HP_270_wick_ir & +bottom & -top)    
HP270_wck = openmc.Cell(name='HP270_wck', fill = Ni, region = +HP_270_wick_ir & -HP_270_ir & +bottom & -top) 
HP270_wall = openmc.Cell(name='HP270_wall', fill = SS, region = +HP_270_ir & -HP_270_or & +bottom & -top) 
HP270_foil = openmc.Cell(name='HP270_foil', fill = Cu, region = +HP_270_or & -HP_270_foil_or & +bottom & -top)

HP315_fld = openmc.Cell(name='HP315_fld', fill = HP_fld, region = -HP_315_wick_ir & +bottom & -top)    
HP315_wck = openmc.Cell(name='HP315_wck', fill = Ni, region = +HP_315_wick_ir & -HP_315_ir & +bottom & -top) 
HP315_wall = openmc.Cell(name='HP315_wall', fill = SS, region = +HP_315_ir & -HP_315_or & +bottom & -top) 
HP315_foil = openmc.Cell(name='HP315_foil', fill = Cu, region = +HP_315_or & -HP_315_foil_or & +bottom & -top)

gap_clamp  = openmc.Cell(name='gap_clamp', fill = helium , region = +fuel_or &-clamps_ir & +HP_0_foil_or & +HP_45_foil_or \
                         & +HP_90_foil_or & +HP_135_foil_or & +HP_180_foil_or & +HP_225_foil_or & +HP_270_foil_or & +HP_315_foil_or \
                         & +bottom & -top)

ring_clamp = openmc.Cell(name='ring_clamp', fill = Haynes230 , region = -clamps_or &+clamps_ir & +HP_0_foil_or & +HP_45_foil_or \
                         & +HP_90_foil_or & +HP_135_foil_or & +HP_180_foil_or & +HP_225_foil_or & +HP_270_foil_or & +HP_315_foil_or \
                         & +bottom & -top)

gap_vacuumcan = openmc.Cell(name='gap_vacuumcan', fill = helium, region = -MLI_ir &+clamps_or & +bottom & -top)

MLI_region = openmc.Cell(name='MLI_region', fill = MLI, region = -vaccan_ir &+MLI_ir & +bottom & -top)

vacuum_can = openmc.Cell(name='vacuum_can', fill = SS, region = -vaccan_or &+vaccan_ir & +bottom & -top)

gap_sleeve = openmc.Cell(name='gap_sleeve', fill = helium, region = +vaccan_or & -sleeve_ir & +bottom & -top)

sleeve = openmc.Cell(name='sleeve', fill = SS, region = +sleeve_ir & -sleeve_or & +bottom & -top)

gap_reflector = openmc.Cell(name='gap_reflector', fill = helium, region = +sleeve_or & -reflec_ir & +bottom & -top)

reflector = openmc.Cell(name='reflector', fill = BeO, region = +reflec_ir & -reflec_or & +reflector_ring_bottom & -reflector_ring_top)
reflector_ring_top = openmc.Cell(name='reflector_ring_top', fill = helium, region = +reflec_ir & -reflec_or & +reflector_ring_top & -top)
reflector_ring_bottom = openmc.Cell(name='reflector_ring_bottom', fill = SS316, region = +reflec_ir & -reflec_or & +bottom & -reflector_ring_bottom)

gap_shield = openmc.Cell(name='gap_shield', fill = helium, region = +reflec_or & -shield_ir & +bottom & -top)

shield = openmc.Cell(name='gap_shield', fill = SS316, region = +shield_ir & -shield_or & +bottom & -top)


root_univ = openmc.Universe(cells=[HP0_foil, HP0_wall, HP0_wck, HP0_fld, HP45_foil, HP45_wall, HP45_wck, HP45_fld \
                                   , HP90_foil, HP90_wall, HP90_wck, HP90_fld, HP135_foil, HP135_wall, HP135_wck, HP135_fld \
                                   , HP180_foil, HP180_wall, HP180_wck, HP180_fld, HP225_foil, HP225_wall, HP225_wck, HP225_fld \
                                   , HP270_foil, HP270_wall, HP270_wck, HP270_fld, HP315_foil, HP315_wall, HP315_wck, HP315_fld \
                                   , ring_clamp, gap_clamp, vacuum_can, MLI_region, gap_vacuumcan, gap_sleeve, sleeve, gap_reflector \
                                   , reflector, reflector_ring_top, reflector_ring_bottom, gap_shield, shield, B4C_region, reflector_top \
                                   , reflector_bottom, empty_top])
root_univ.add_cells(fuel_cells)                                   
model.geometry = openmc.Geometry(root_univ)


################################################################################
## Defining Settings



model.settings.source = openmc.IndependentSource(space=openmc.stats.Box([-Fuel_OD/2, -Fuel_OD/2, 20],
                                                       [ Fuel_OD/2,  Fuel_OD/2, 45]))

model.settings.batches = 150
model.settings.inactive = 50
model.settings.particles = 100000

model.settings.temperature = {'default': 800.0,
                        'method': 'interpolation',
                        'range': (294.0, 3000.0),
                        'tolerance': 1000.0}

model.export_to_xml()

################################################################################
## Plotting

p1 = openmc.Plot()
p1.filename = 'plot1'
p1.basis = 'xy'
p1.origin= (0,0,30)
p1.width = (15.0, 15.0)
p1.pixels = (4000, 4000)
p1.color_by = 'material'

p2 = openmc.Plot()
p2.filename = 'plot2'
p2.basis = 'xz'
p2.origin= (0,0,25)
p2.width = (60.0, 60.0)
p2.pixels = (10000, 10000)
p2.color_by = 'cell'

plots = openmc.Plots([p1, p2])
plots.export_to_xml()
