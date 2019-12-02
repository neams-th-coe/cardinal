//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NearestPointReceiverTransfer.h"

#include "NearestPointReceiver.h"

// MOOSE includes
#include "MooseTypes.h"
#include "FEProblem.h"
#include "MultiApp.h"

// libMesh
#include "libmesh/meshfree_interpolation.h"
#include "libmesh/system.h"

registerMooseObject("MooseApp", NearestPointReceiverTransfer);

template <>
InputParameters
validParams<NearestPointReceiverTransfer>()
{
  InputParameters params = validParams<MultiAppTransfer>();

  params.addRequiredParam<UserObjectName>(
      "from_uo",
      "The name of the UserObject in the Master to transfer the value from.");

  params.addRequiredParam<UserObjectName>(
      "to_uo",
      "The name of the NearestPointReceiver in the MultiApp to transfer the value to. ");

  return params;
}

NearestPointReceiverTransfer::NearestPointReceiverTransfer(const InputParameters & parameters)
  : MultiAppTransfer(parameters),
    _from_uo_name(getParam<UserObjectName>("from_uo")),
    _to_uo_name(getParam<UserObjectName>("to_uo"))
{
  if (_direction == FROM_MULTIAPP)
    paramError("direction", "NearestPointReceiverTransfer doesn't yet support transferring _from_ a MultiApp");
}

void
NearestPointReceiverTransfer::execute()
{
  switch (_direction)
  {
    case TO_MULTIAPP:
    {
      FEProblemBase & from_problem = _multi_app->problemBase();

      auto & from_uo = from_problem.getUserObjectBase(_from_uo_name);

      // Declared here so we don't have to keep reallocating it.
      std::vector<Real> values;

      for (unsigned int i = 0; i < _multi_app->numGlobalApps(); i++)
      {
        if (_multi_app->hasLocalApp(i))
        {
          values.clear();

          auto & receiver = _multi_app->appProblemBase(i).getUserObjectTempl<NearestPointReceiver>(_to_uo_name);
          const auto & points = receiver.positions();

          values.reserve(points.size());

          for (const auto & point : points)
            values.emplace_back(from_uo.spatialValue(point));

          receiver.setValues(values);
        }
      }

      break;
    }
    case FROM_MULTIAPP:
    {
      mooseError("Not Implemented");
    }
  }

  _console << "Finished PostprocessorTransfer " << name() << std::endl;
}
