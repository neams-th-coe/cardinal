#!/usr/bin/env python3
import os, shlex, sys
from MooseControl import MooseControl

# All of these imports are awful and we'll figure
# out a better way to do it before merging
try:
   import openmc
except:
    openmc_contrib = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..', '..', 'contrib', 'openmc'))
    if os.path.exists(openmc_contrib):
        sys.path.append(openmc_contrib)
    import openmc


import openmc.lib
import openmc.deplete
from openmc.deplete import CoupledOperator

try:
    from cardinal_operator import CardinalOperator
except:
    cardinal_contrib = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..', '..', 'python'))
    if os.path.exists(cardinal_contrib):
        sys.path.append(cardinal_contrib)
    from cardinal_operator import CardinalOperator

if __name__ == '__main__':
    # Get the command we should run
    RUNAPP_COMMAND = os.environ.get('RUNAPP_COMMAND')
    if RUNAPP_COMMAND is None:
        sys.exit('Missing expected command variable RUNAPP_COMMAND')

    # We need to automate the removal of this state because
    # it'll definitely break things if we don't
    files = ['tallies.out', 'tallies.xml', 'statepoint.40.h5', 'summary.h5']
    for file in files:
        if os.path.exists(file):
            os.remove(file)

    model = openmc.Model.from_xml()

    operator = CardinalOperator(model, 'chain_simple.xml')
    operator.cardinal_cmd = RUNAPP_COMMAND

    timesteps = [0.1] * 2
    timestep_units = 'd'
    power = 174  # Wth (pincell)

    integrator = openmc.deplete.PredictorIntegrator(operator,
                                                    timesteps,
                                                    power,
                                                    timestep_units=timestep_units)
    integrator.integrate()
