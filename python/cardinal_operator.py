import os
import copy
from numbers import Integral
import requests
import subprocess
import time

from uncertainties import ufloat

import openmc
import openmc.lib

# openmc
import openmc.deplete
from openmc.deplete import CoupledOperator
from openmc.deplete.abc import OperatorResult

# moose
import pyhit


# perhaps we should require that this is part of the user's environment?
CARDINAL_PATH = '/home/pshriwise/soft/cardinal/cardinal-opt'


class CardinalOperator(CoupledOperator):

    def __init__(self, *args, **kwargs):
        self._cardinal_pipe = None
        self.cardinal_log = 'cardinal.out'

        super().__init__(*args, **kwargs)
        self.moose_base_url = 'http://localhost'
        self.port = 5800
        self.output = True
        self.verbose = False
        self._threads = None

    def __del__(self):
        # Ensure the Cardinal process is terminated
        # when this object is deleted
        self.stop_cardinal()

    def log_message(self, message):
        if self.output:
            print(f'[CardinalOperator]: {message}')

    @staticmethod
    def build_material_userobject(node, mat):
        print(mat.nuclides)
        mat_info = {'type': 'OpenMCNuclideDensities',
                    'material_id': f'{mat.id}',
                    'names': f"'{' '.join(mat.nuclides)}'",
                    'densities': f"'{' '.join([str(d) for d in mat.densities.flat])}'"}
        node.append(f'openmc_mat{mat.id}', **mat_info)

    @staticmethod
    def build_tally_userobject(node, id):
        tally_info = {'type': 'OpenMCTallyNuclides',
                      'tally_id': f'{id}',
                      'names': "''"}
        node.append(f'openmc_tally{id}', **tally_info)

    @property
    def threads(self):
        return self._threads

    @threads.setter
    def threads(self, value):
        if not isinstance(value, Integral):
            raise ValueError('Threads must be an integer')
        self._threads = value

    @property
    def port(self):
        return self._port

    @port.setter
    def port(self, value):
        if not isinstance(value, Integral):
            raise ValueError('Property \'port\' must be an integer')
        self._port = value

    @property
    def moose_url(self):
        return f'{self.moose_base_url}:{self.port}'

    def start_cardinal(self):
        """Starts the Cardinal background process in server mode
        """
        root = pyhit.Node()
        userobjs = root.append('UserObjects')

        # material user objects
        for mat in openmc.lib.materials.values():
            self.build_material_userobject(userobjs, mat)

        for tally in openmc.lib.tallies.values():
            self.build_tally_userobject(userobjs, tally.id)

        # add sever information
        server_info = {'type': 'WebServerControl',
                        'execute_on': '"TIMESTEP_BEGIN TIMESTEP_END"',
                        'port': str(self.port)}
        controls = root.append('Controls')
        webserver = controls.append('webserver', **server_info)

        pyhit.write('server.i', root)

        print('[CardinalOperator]: Starting Cardinal')
        log = open(self.cardinal_log, 'w')
        command = ['mpiexec', '-n', str(openmc.deplete.comm.Get_size()), CARDINAL_PATH, '-i', 'server.i', 'openmc.i']
        if self.threads is not None:
            command += [f'--n-threads={self.threads}']

        self._cardinal_pipe = subprocess.Popen(command,
                                               stdout=log,
                                               stderr=log,
                                               env=os.environ)

    def stop_cardinal(self):
        """Stops the Cardinal background process"""
        if self._cardinal_pipe is not None:
            self._cardinal_pipe.terminate()
            self._cardinal_pipe = None

    def moose_wait(self, exec_flag=None):
        """Waits for the moose webserver and returns once the WebServerControl
        is waiting for input

        Parameters
        ----------
        exec_flag : str
            A specific exec flag to wait on (optional)
        """

        print(f'[CardinalOperator]: Waiting for cardinal to be at {exec_flag}')

        # How often we want to poll MOOSE for its availability
        poll_time = 0.1
        # The max errors we want to let pass before giving up
        # We should only get connection errors when moose is setting up; when it's
        # actually running the web server should respond
        max_errors = 1000
        # The number of errors we've reached; for tracking exits on many errors
        current_errors = 0

        # Poll until we're available
        while True:
            time.sleep(poll_time)

            try:
                r = requests.get(f'{self.moose_url}/waiting')
            except requests.exceptions.ConnectionError:
                current_errors = current_errors + 1
                if current_errors == max_errors:
                    raise
                continue
            except:
                raise

            res = r.json()

            assert 'waiting' in res
            if res['waiting']:
                print(f'[CardinalOperator]: Cardinal is waiting at {res["execute_on_flag"]}')

                assert 'execute_on_flag' in res
                if exec_flag is not None and res['execute_on_flag'] != exec_flag:
                    raise Exception('Unexpected execute on flag')

                return

    def moose_waiting_flag(self):
        """Gets the current EXECUTE_ON flag that MOOSE is waiting on

        This should only be called when moose is actually waiting (see moose_wait())

        Returns
        -------
        string
            The EXECUTE_ON flag that MOOSE is currently at
        """
        r = requests.get(f'{self.moose_url}/waiting')
        r.raise_for_status()

        res = r.json()
        assert 'waiting' in res
        assert 'execute_on_flag' in res

        if not res['waiting']:
            raise Exception('MOOSE is not waiting')

        return res['execute_on_flag']

    def moose_post(self, path, data):
        """Sends JSON information to the MOOSE webserver

        Parameters
        ----------
        path : str
            The object path to send data to; typically UserObjects/<name>
        data : dict
            The data to send
        """
        print(f'[CardinalOperator]: Sending {data} to "{path}"')
        r = requests.post(f'{self.moose_url}/{path}', json=data)
        if r.headers.get('content-type') == 'application/json' and 'error' in r.json():
            print(f'HTTP error: "{r.json()["error"]}"')
        r.raise_for_status()

    def moose_continue(self):
        """Tells the MOOSE WebServerControl to continue the solve
        """
        print(f'[CardinalOperator]: Telling cardinal to continue')
        r = requests.get(f'{self.moose_url}/continue')
        r.raise_for_status()

    def run_cardinal(self):
        """Execute a Cardinal run and wait for it to finish
        """
        # Wait for cardinal to be a the beginning of a solve
        self.moose_wait('TIMESTEP_BEGIN')

        # Start the solve
        self.moose_continue()

        # Wait for cardinal to finish the solve
        self.moose_wait('TIMESTEP_END')

        return f'statepoint.{self.model.settings.batches}.h5'

    def update_cardinal_materials(self):
        """Pass material compositions from the local in-memory OpenMC model to
        Cardinal's OpenMC model
        """
        # Wait for cardinal to be at any flag. On the first solve, we'll be
        # sitting at TIMESTEP_BEGIN. But after any other solves, we'll be
        # at TIMESTEP_END from the previous timestep
        self.moose_wait()

        # If we're at TIMESTEP_END, we need to continue on
        if self.moose_waiting_flag() == 'TIMESTEP_END':
            self.moose_continue()

        # Wait for cardinal to be ready at the beginning of a timestep
        self.moose_wait('TIMESTEP_BEGIN')

        print('[CardinalOperator]: Updating cardinal materials')
        for m in openmc.lib.materials.values():
            uo_path = f'UserObjects/openmc_mat{m.id}'
            print(f'[CardinalOperator]: Updating material {m.id} via {uo_path}')

            names_json = {'name': f'{uo_path}/names', 'value': m.nuclides}
            self.moose_post('set/controllable', names_json)

            densities_json = {'name': f'{uo_path}/densities', 'value': list(m.densities.flat)}
            self.moose_post('set/controllable', densities_json)

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
        if self._cardinal_pipe is None:
            self.start_cardinal()

        # Wait for cardinal to be at any flag
        self.moose_wait()

        # If we're at timestep end, keep going
        if self.moose_waiting_flag() == 'TIMESTEP_END':
            self.moose_continue()

        # Wait for cardinal to be ready at the beginning of a timestep
        self.moose_wait('TIMESTEP_BEGIN')

        print('[CardinalOperator]: Updating cardinal tally nuclides')
        for t in openmc.lib.tally.tallies.values():
            uo_path = f'UserObjects/openmc_tally{t.id}'
            print(f'[CardinalOperator]: Updating tally {t.id} via {uo_path}')

            nuclides = [n for n in t.nuclides]
            nuclides_json = {'name': f'{uo_path}/names', 'value': nuclides}
            self.moose_post('set/controllable', nuclides_json)
            break

    def load_cardinal_results(self, sp_file):
        """Load results from the statepoint file generated by the Cardinal
        execution
        """
        with openmc.lib.quiet_dll(False):
            openmc.lib.simulation_init()
            openmc.lib.statepoint_load(sp_file)
            openmc.lib.simulation_finalize()
        print('Statepoint loaded')

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

        openmc.lib.tallies.export_to_xml()
        # Ensure that tally data is written to the statepoint file
        for t in openmc.lib.tally.tallies.values():
            t.writable = True

        if self._cardinal_pipe is None:
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
        if self._cardinal_pipe is None:
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