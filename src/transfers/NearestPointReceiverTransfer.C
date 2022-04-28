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

InputParameters
NearestPointReceiverTransfer::validParams()
{
  InputParameters params = MultiAppTransfer::validParams();

  params.addRequiredParam<UserObjectName>("from_uo",
                                          "The name of the UserObject to transfer the value from.");

  params.addRequiredParam<UserObjectName>(
      "to_uo", "The name of the NearestPointReceiver to transfer the value to. ");

  return params;
}

NearestPointReceiverTransfer::NearestPointReceiverTransfer(const InputParameters & parameters)
  : MultiAppTransfer(parameters),
    _from_uo_name(getParam<UserObjectName>("from_uo")),
    _to_uo_name(getParam<UserObjectName>("to_uo"))
{
}

void
NearestPointReceiverTransfer::execute()
{
  std::vector<Real> values;

  switch (_direction)
  {
    case TO_MULTIAPP:
    {
      FEProblemBase & from_problem = getToMultiApp()->problemBase();
      auto & from_uo = from_problem.getUserObjectBase(_from_uo_name);

      for (unsigned int i = 0; i < getToMultiApp()->numGlobalApps(); i++)
      {
        if (getToMultiApp()->hasLocalApp(i))
        {
          values.clear();

          auto & receiver = getToMultiApp()->appProblemBase(i).getUserObject<NearestPointReceiver>(_to_uo_name);
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
      FEProblemBase & to_problem = getFromMultiApp()->problemBase();
      auto & receiver = to_problem.getUserObject<NearestPointReceiver>(_to_uo_name);

      for (unsigned int i = 0; i < getFromMultiApp()->numGlobalApps(); ++i)
      {
        if (getFromMultiApp()->hasLocalApp(i))
        {
          values.clear();

          auto & from_uo = getFromMultiApp()->appProblemBase(i).getUserObjectBase(_from_uo_name);
          const auto & points = receiver.positions();

          values.reserve(points.size());

          for (const auto & point : points)
            values.emplace_back(from_uo.spatialValue(point));

          receiver.setValues(values);
        }
      }
    }
  }

  _console << "Finished NearestPointReceiverTransfer " << name() << std::endl;
}
