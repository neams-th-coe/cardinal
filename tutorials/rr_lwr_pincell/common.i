# Some properties that can be modified to change the model.
## The radius of a fuel pin (cm).
R_FUEL = 0.4095

## The thickness of the fuel-clad gap (cm).
T_F_C_GAP = 0.0085

## The thickness of the Zr fuel pin cladding (cm).
T_ZR_CLAD = 0.057

## The pitch of a single lattice element (cm).
PITCH = 1.26

## The height of the fuel pin (cm). The pin geometry will
## range from 0 to HEIGHT.
HEIGHT = 200.0

## The number of axial layers in the multiphysics model.
AXIAL_LAYERS = 100

## The power of the pincell (W). Equal to 3411e6 / (273 * 264)
P_PIN = 66945.25

## The inlet temperature (K).
T_INLET  = 573.0

## Initial density of the coolant (kg/m^3) as used for computing
## multi-group cross sections.
RHO_INIT = 723.219601768

## The mass flowrate into the pincell (kg/s).
M_DOT = 0.875

## The outlet pressure (Pa).
P_OUTLET = 15.51e6

## Thermal conductivity of the fuel, cladding, and gap (W/m/K).
K_FUEL = 5.0
K_CLAD = 50.0
K_GAS = 1.0
