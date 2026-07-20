# Some properties that can be modified to change the model.
## The radius of a fuel pin (cm).
R_FUEL = 0.4095

## The thickness of the fuel-clad gap (cm).
T_F_C_GAP = 0.0085

## The thickness of the Zr fuel pin cladding (cm).
T_ZR_CLAD = 0.057

## The pitch of a single lattice element (cm).
PITCH = 1.26

## The height of the fuel pin (cm). The pin geometry will range from -HEIGHT/2 to HEIGHT/2
HEIGHT = 200.0

## The number of axial layers in the multiphysics model.
AXIAL_LAYERS = 100

## The power of the pincell (W). Equal to 3000e6 / (273 * 264)
P_PIN = 41625.0

## The inlet and outlet temperature of the coolant, alongside the average of the
## inlet/outlet temperature (K).
T_INLET  = 573.0
T_OUTLET = 623.0
T_AVG    = 598

## The convective heat transfer coefficient (W/m2/K)
CONV_HT_COEFF = 1000.0

## Thermal conductivity of the fuel, cladding, and gap (W/m/K).
K_FUEL = 5.0
K_CLAD = 50.0
K_GAS = 1.0
