import os
import copy
import logging
from numbers import Integral
import shlex

from uncertainties import ufloat

import openmc
import openmc.lib

# openmc
import openmc.deplete
from openmc.deplete import CoupledOperator
from openmc.deplete.abc import OperatorResult

# moose
from MooseControl import MooseControl

logger = logging.getLogger('CardinalOperator')

class CardinalOperator(CoupledOperator):

    def __init__(self, *args, **kwargs):
        self._control = None
        self.cardinal_cmd = None
        super().__init__(*args, **kwargs)

    def __del__(self):
        # Ensure the Cardinal process is terminated
        # when this object is deleted
        self.stop_cardinal()

    @staticmethod
    def material_userobject_args(mat):
        uo_path = f'UserObjects/openmc_mat{mat.id}'
        return [f'{uo_path}/type=OpenMCNuclideDensities',
                f'{uo_path}/material_id={mat.id}',
                f'{uo_path}/names="{" ".join(mat.nuclides)}"',
                f'{uo_path}/densities="{" ".join([str(d) for d in mat.densities.flat])}"']

    @staticmethod
    def filter_userobject_args(id, domain_type, domain_ids):
        filter_path = f'UserObjects/openmc_filter{id}'
        return [f'{filter_path}/type=OpenMCDomainFilterEditor',
                f'{filter_path}/create_filter=true',
                f'{filter_path}/filter_id="{id}"',
                f'{filter_path}/filter_type="{domain_type}"',
                f'{filter_path}/bins="{" ".join([str(d) for d in domain_ids])}"']

    @staticmethod
    def tally_userobject_args(id):
        tally_path = f'UserObjects/openmc_tally{id}'
        return [f'{tally_path}/type=OpenMCTallyEditor',
                f'{tally_path}/create_tally=true',
                f'{tally_path}/tally_id={id}',
                f'{tally_path}/scores=""',
                f'{tally_path}/nuclides=""',
                f'{tally_path}/filter_ids=""',
                f'{tally_path}/multiply_density=false']

    def start_cardinal(self):
        """Builds and starts the MooseControl object that runs cardinal
        """
        if self.cardinal_cmd is None:
            raise Exception('cardinal_cmd was not set')
        cardinal_cmd = shlex.split(self.cardinal_cmd)

        # add server information
        control_name = 'web_server'
        control_path = f'Controls/{control_name}'

        cardinal_cmd += [f'{control_path}/type=WebServerControl',
                         f'{control_path}/execute_on="TIMESTEP_BEGIN TIMESTEP_END"']
        # add material user objects
        for mat in openmc.lib.materials.values():
            cardinal_cmd += self.material_userobject_args(mat)
        # add tally user objects
        for tally in openmc.lib.tallies.values():
            cardinal_cmd += self.tally_userobject_args(tally.id)

        for filter in openmc.lib.filters.values():
            if isinstance(filter, (openmc.lib.CellFilter, openmc.lib.MaterialFilter, openmc.lib.UniverseFilter)):
                bins = [bin.id for bin in filter.bins]
            elif isinstance(filter, openmc.lib.MeshFilter):
                bins = [filter.mesh.id]
            else:
                continue
            cardinal_cmd += self.filter_userobject_args(filter.id, filter.filter_type, bins)

        with open('server.i', 'w') as fh:
            fh.write('\n'.join(cardinal_cmd))

        self._control = MooseControl(moose_command=cardinal_cmd,
                                     moose_control_name=control_name)
        self._control.initialize()

    def stop_cardinal(self):
        """Stops the Cardinal background process"""
        if self._control is not None:
            self._control.kill()

    def run_cardinal(self):
        """Execute a Cardinal run and wait for it to finish
        """
        logger.info('Running cardinal')

        # Wait for cardinal to be a the beginning of a solve
        self._control.wait('TIMESTEP_BEGIN')

        # Start the solve
        self._control.setContinue()

        # Wait for cardinal to finish the solve
        self._control.wait('TIMESTEP_END')

        return f'statepoint.{self.model.settings.batches}.h5'

    def update_cardinal_materials(self):
        """Pass material compositions from the local in-memory OpenMC model to
        Cardinal's OpenMC model
        """
        logger.info('Waiting to update carindal materials')

        # Wait for cardinal to be at any flag. On the first solve, we'll be
        # sitting at TIMESTEP_BEGIN. But after any other solves, we'll be
        # at TIMESTEP_END from the previous timestep
        current_flag = self._control.wait()

        # If we're at TIMESTEP_END, we need to continue on
        if current_flag == 'TIMESTEP_END':
            self._control.setContinue()

        # Wait for cardinal to be ready at the beginning of a timestep
        self._control.wait('TIMESTEP_BEGIN')

        logger.info('Updating cardinal materials')
        for m in openmc.lib.materials.values():
            uo_path = f'UserObjects/openmc_mat{m.id}'
            logger.info(f'Updating material {m.id} via {uo_path}')

            names_path = f'{uo_path}/names'
            self._control.setControllableVectorString(names_path, m.nuclides)

            densities_path = f'{uo_path}/densities'
            densities = list(m.densities.flat)
            self._control.setControllableVectorReal(densities_path, densities)

            break # hack for only sending the first

    def _update_materials_and_nuclides(self, vec):
        """Update the number density, material compositions, and nuclide
        lists in helper objects

        Parameters
        ----------
        vec : list of numpy.ndarray
            Total atoms.

        """
        # Update the number densities regardless of the source rate
        self.number.set_density(vec)
        self._update_materials()

        # Prevent OpenMC from complaining about re-creating tallies
        openmc.reset_auto_ids()

        # Update tally nuclides data in preparation for transport solve
        nuclides = self._get_reaction_nuclides()
        print(f"Nuclides: {nuclides}")
        self._rate_helper.nuclides = nuclides
        self._normalization_helper.nuclides = nuclides
        self._yield_helper.update_tally_nuclides(nuclides)

    def update_cardinal_tallies(self):
        """Update the set of nuclides in the depletion tallies in the Cardinal
        OpenMC model
        """
        logger.info('Waiting to update cardinal tally nuclides')

        if self._control is None:
            self.start_cardinal()

        # Wait for cardinal to be at any flag
        current_flag = self._control.wait()

        # If we're at timestep end, keep going
        if current_flag == 'TIMESTEP_END':
            self._control.setContinue()

        # Wait for cardinal to be ready at the beginning of a timestep
        self._control.wait('TIMESTEP_BEGIN')

        logger.info('Updating cardinal tally nuclides')
        for t in openmc.lib.tally.tallies.values():
            uo_path = f'UserObjects/openmc_tally{t.id}'
            logger.info(f'Updating tally {t.id} via {uo_path}')
            nuclides = [n for n in t.nuclides]
            names_path = f'{uo_path}/nuclides'
            self._control.setControllableVectorString(names_path, nuclides)

            scores = [s for s in t.scores]
            self._control.setControllableVectorString(f'{uo_path}/scores', scores)

            filter_ids = [f.id for f in t.filters]
            self._control.setControllableVectorString(f'{uo_path}/filter_ids', filter_ids)

            break

    def load_cardinal_results(self, sp_file):
        """Load results from the statepoint file generated by the Cardinal
        execution
        """
        with openmc.lib.quiet_dll(False):
            openmc.lib.simulation_init()
            openmc.lib.statepoint_load(sp_file)
            openmc.lib.simulation_finalize()
        logger.info('Statepoint loaded')

    def initial_condition(self):
        """Performs setup, stars the Cardinal problem (with a server enabled) and returns initial condition.

        Parameters
        ----------
        materials : list of openmc.lib.Material
            list of materials

        Returns
        -------
        list of numpy.ndarray
            Total density for initial conditions.

        """

        n = super().initial_condition()
        self._update_materials_and_nuclides(n)

        # Ensure that tally data is written to the statepoint file
        for t in openmc.lib.tally.tallies.values():
            t.writable = True

        if self._control is None:
            self.start_cardinal()

        return n

    def __call__(self, vec, source_rate):
        """Runs a simulation.

        Simulation will abort under the following circumstances:

            1) No energy is computed using OpenMC tallies.

        Parameters
        ----------
        vec : list of numpy.ndarray
            Total atoms to be used in function.
        source_rate : float
            Power in [W] or source rate in [neutron/sec]

        Returns
        -------
        openmc.deplete.OperatorResult
            Eigenvalue and reaction rates resulting from transport operator

        """
        # Reset results in OpenMC
        openmc.lib.reset()

        # start Cardinal if it hasn't been started already
        if self._control is None:
            self.start_cardinal()

        self._update_materials_and_nuclides(vec)

        # Update materials in Cardinal
        self.update_cardinal_materials()

        # If the source rate is zero, return zero reaction rates without running
        # a transport solve
        if source_rate == 0.0:
            rates = self.reaction_rates.copy()
            rates.fill(0.0)
            return OperatorResult(ufloat(0.0, 0.0), rates)

        # EXECUTE CARDINAL
        self.update_cardinal_tallies()
        sp_file = self.run_cardinal()

        # LOAD RESULTS FROM STATEPOINT FILE GENERATED BY CARDINAL
        self.load_cardinal_results(sp_file)

        # Extract results
        rates = self._calculate_reaction_rates(source_rate)

        # Get k and uncertainty
        keff = ufloat(*openmc.lib.keff())

        op_result = OperatorResult(keff, rates)

        return copy.deepcopy(op_result)
