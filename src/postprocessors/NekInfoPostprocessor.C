/********************************************************************/
/*                  SOFTWARE COPYRIGHT NOTIFICATION                 */
/*                             Cardinal                             */
/*                                                                  */
/*                  (c) 2021 UChicago Argonne, LLC                  */
/*                        ALL RIGHTS RESERVED                       */
/*                                                                  */
/*                 Prepared by UChicago Argonne, LLC                */
/*               Under Contract No. DE-AC02-06CH11357               */
/*                With the U. S. Department of Energy               */
/*                                                                  */
/*             Prepared by Battelle Energy Alliance, LLC            */
/*               Under Contract No. DE-AC07-05ID14517               */
/*                With the U. S. Department of Energy               */
/*                                                                  */
/*                 See LICENSE for full restrictions                */
/********************************************************************/

#ifdef ENABLE_NEK_COUPLING

#include "NekInfoPostprocessor.h"
#include "SubProblem.h"
#include "MooseMesh.h"

#include "cfl.hpp"

registerMooseObject("CardinalApp", NekInfoPostprocessor);

InputParameters
NekInfoPostprocessor::validParams()
{
  InputParameters params = NekPostprocessor::validParams();

  MooseEnum test_type("n_iter_velocity n_iter_pressure n_iter_temperature "
                      "n_iter_scalar01 n_iter_scalar02 n_iter_scalar03 cfl tstep "
                      "p0th dp0thdt "
                      "solver_velocity solver_temperature solver_scalar01 "
                      "solver_scalar02 solver_scalar03 "
                      "xmin xmax ymin ymax zmin zmax "
                      );
  params.addRequiredParam<MooseEnum>("test_type",
                                     test_type,
                                     "The type of info to fetch from results; "
                                     "this is used to toggle between many different tests to avoid "
                                     "creating tons of source files.");

  params.addClassDescription("Extract properties of the NekRS results");
  return params;
}

NekInfoPostprocessor::NekInfoPostprocessor(const InputParameters & parameters)
  : NekPostprocessor(parameters), _test_type(getParam<MooseEnum>("test_type"))
{
  // Check if the NekRS simulation has the required scalar
  // Temperature
  if ((_test_type == "n_iter_temperature") or (_test_type == "solver_temperature"))
  {
    if (!nekrs::hasTemperatureVariable())
      mooseError("This postprocessor cannot be used in NekRS problems "
                 "without a temperature variable!");
    if (platform->options.compareArgs("SCALAR00 SOLVER", "NONE"))
      mooseWarning("Temperature solver is deactivated.\n");
    if (platform->options.compareArgs("CVODE", "TRUE"))
      mooseError("This postprocessor does not work with CVODE yet!");
  }
  // Scalar01
  else if ((_test_type == "n_iter_scalar01") or (_test_type == "solver_scalar01"))
  {
    if (!nekrs::hasScalarVariable(1))
      mooseError("This postprocessor cannot be used in NekRS problems "
                 "without a Scalar01 variable!");
    if (platform->options.compareArgs("SCALAR01 SOLVER", "NONE"))
      mooseWarning("Scalar01 solver is deactivated.\n");
    if (platform->options.compareArgs("CVODE", "TRUE"))
      mooseError("This postprocessor does not work with CVODE yet!");
  }
  // Scalar02
  else if ((_test_type == "n_iter_scalar02") or (_test_type == "solver_scalar02"))
  {
    if (!nekrs::hasScalarVariable(2))
      mooseError("This postprocessor cannot be used in NekRS problems "
                 "without a Scalar02 variable!");
    if (platform->options.compareArgs("SCALAR02 SOLVER", "NONE"))
      mooseWarning("Scalar02 solver is deactivated.\n");
    if (platform->options.compareArgs("CVODE", "TRUE"))
      mooseError("This postprocessor does not work with CVODE yet!");
  }
  // Scalar03
  else if ((_test_type == "n_iter_scalar03") or (_test_type == "solver_scalar03"))
  {
    if (!nekrs::hasScalarVariable(3))
      mooseError("This postprocessor cannot be used in NekRS problems "
                 "without a Scalar03 variable!");
    if (platform->options.compareArgs("SCALAR03 SOLVER", "NONE"))
      mooseWarning("Scalar03 solver is deactivated.\n");
    if (platform->options.compareArgs("CVODE", "TRUE"))
      mooseError("This postprocessor does not work with CVODE yet!");
  }
}

Real
NekInfoPostprocessor::getValue() const
{
  nrs_t * nrs = (nrs_t *)nekrs::nrsPtr();
  
  //double _numCVODERHS = 0;
  
  // Extract the desired information
  // Velocity iterations
  if (_test_type == "n_iter_velocity") {
    if (platform->options.compareArgs("VELOCITY BLOCK SOLVER", "TRUE"))
      return nrs->uvwSolver->Niter;
    else if (nrs->uSolver)
      return nrs->uSolver->Niter;
    else
      return -1;
  }
  
  // Pressure iterations
  else if (_test_type == "n_iter_pressure")
    return nrs->pSolver ? nrs->pSolver->Niter : -1;
  
  // Temperature iterations
  else if (_test_type == "n_iter_temperature") {
    if (platform->options.compareArgs("SCALAR00 SOLVER", "CVODE")) {
      return -1;
    }
    else if (platform->options.compareArgs("SCALAR00 SOLVER", "NONE"))
      return -1;
    else
      return nrs->cds->solver[0]->Niter;
  }
  
  // Scalar 01 iterations
  else if (_test_type == "n_iter_scalar01") {
    if (platform->options.compareArgs("SCALAR01 SOLVER", "CVODE")) {
      return -1;
    }
    else if (platform->options.compareArgs("SCALAR01 SOLVER", "NONE"))
      return -1;
    else
      return nrs->cds->solver[1]->Niter;
  }
  
  // Scalar 02 iterations
  else if (_test_type == "n_iter_scalar02") {
    if (platform->options.compareArgs("SCALAR02 SOLVER", "CVODE")) {
      return -1;
    }
    else if (platform->options.compareArgs("SCALAR02 SOLVER", "NONE"))
      return -1;
    else
      return nrs->cds->solver[2]->Niter;
  }
  
  // Scalar 03 iterations
  else if (_test_type == "n_iter_scalar03") {
    if (platform->options.compareArgs("SCALAR03 SOLVER", "CVODE")) {
      return -1;
    }
    else if (platform->options.compareArgs("SCALAR03 SOLVER", "NONE"))
      return -1;
    else
      return nrs->cds->solver[3]->Niter;
  }
  
  // Time step
  else if (_test_type == "tstep")
    return nrs->tstep;
  
  // CFL number
  else if (_test_type == "cfl")
    return computeCFL(nrs);
  
  // Termodynamic pressure
  else if (_test_type == "p0th")
    return nrs->p0th[0];
  
  // Termodynamic pressure time derivative
  else if (_test_type == "dp0thdt")
    return nrs->dp0thdt;
  
  // Velocity solver
  else if (_test_type == "solver_velocity"){
    if (platform->options.compareArgs("VELOCITY SOLVER", "NONE"))
      return 0;
    else
      return 1;
  }
  
  // Temperature solver
  else if (_test_type == "solver_temperature"){
    if (platform->options.compareArgs("SCALAR00 SOLVER", "NONE"))
      return 0;
    else
      return 1;
  }
  
  // Scalar01 solver
  else if (_test_type == "solver_scalar01"){
    if (platform->options.compareArgs("SCALAR01 SOLVER", "NONE"))
      return 0;
    else
      return 1;
  }
  
  // Scalar02 solver
  else if (_test_type == "solver_scalar02"){
    if (platform->options.compareArgs("SCALAR02 SOLVER", "NONE"))
      return 0;
    else
      return 1;
  }
  
  // Scalar03 solver
  else if (_test_type == "solver_scalar03"){
    if (platform->options.compareArgs("SCALAR03 SOLVER", "NONE"))
      return 0;
    else
      return 1;
  }
  
  // Mesh - X minimum value
  else if (_test_type == "xmin") {
    auto x = nrs->_mesh->x;
    dfloat minValue = x[0];
    int size = nrs->_mesh->Np * nrs->_mesh->Nelements;
    for (int i = 0; i < size; i++) {
        if (x[i] < minValue) {
            minValue = x[i];
        }
    }
    return minValue;
  }
  // Mesh - x maximum value
  else if (_test_type == "xmax") {
    auto x = nrs->_mesh->x;
    dfloat maxValue = x[0];
    int size = nrs->_mesh->Np * nrs->_mesh->Nelements;
    for (int i = 0; i < size; i++) {
        if (x[i] > maxValue) {
            maxValue = x[i];
        }
    }
    return maxValue;
  }
  // Mesh - Y minimum value
  else if (_test_type == "ymin") {
    auto y = nrs->_mesh->y;
    dfloat minValue = y[0];
    int size = nrs->_mesh->Np * nrs->_mesh->Nelements;
    for (int i = 0; i < size; i++) {
        if (y[i] < minValue) {
            minValue = y[i];
        }
    }
    return minValue;
  }
  // Mesh - Y maximum value
  else if (_test_type == "ymax") {
    auto y = nrs->_mesh->y;
    dfloat maxValue = y[0];
    int size = nrs->_mesh->Np * nrs->_mesh->Nelements;
    for (int i = 0; i < size; i++) {
        if (y[i] > maxValue) {
            maxValue = y[i];
        }
    }
    return maxValue;
  }
  // Mesh - Z minimum value
  else if (_test_type == "ymin") {
    auto z = nrs->_mesh->z;
    dfloat minValue = z[0];
    int size = nrs->_mesh->Np * nrs->_mesh->Nelements;
    for (int i = 0; i < size; i++) {
        if (z[i] < minValue) {
            minValue = z[i];
        }
    }
    return minValue;
  }
  // Mesh - Z maximum value
  else if (_test_type == "ymin") {
    auto z = nrs->_mesh->z;
    dfloat maxValue = z[0];
    int size = nrs->_mesh->Np * nrs->_mesh->Nelements;
    for (int i = 0; i < size; i++) {
        if (z[i] > maxValue) {
            maxValue = z[i];
        }
    }
    return maxValue;
  }
  
  else
    mooseError("Unhandled 'test_type' enum!");
}

#endif
