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

#ifdef ENABLE_OPENMC_COUPLING

#include "OpenMCVolumeCalculation.h"
#include "OpenMCCellAverageProblem.h"
#include "UserErrorChecking.h"
#include "DisplacedProblem.h"

registerMooseObject("CardinalApp", OpenMCVolumeCalculation);

InputParameters
OpenMCVolumeCalculation::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params += OpenMCBase::validParams();
  params.addParam<Point>("lower_left", "Lower left of the bounding box inside of which to "
    "compute volumes. If not specified, this will default to the lower left of the [Mesh] "
    "(which will NOT capture any OpenMC geometry that lies outside the [Mesh] extents.");
  params.addParam<Point>("upper_right", "Upper right of the bounding box inside of which to "
    "compute volumes. If not specified, this will default to the upper right of the [Mesh] "
    "(which will NOT capture any OpenMC geometry that lies outside the [Mesh] extents.");
  params.addRequiredRangeCheckedParam<unsigned int>("n_samples", "n_samples > 0",
    "Number of samples to use for the stochastic volume calculation");
  params.addRangeCheckedParam<Real>("trigger_threshold", "trigger_threshold > 0",
    "Trigger threshold to decide when to terminate the volume calculation");

  MooseEnum trigger("rel_err none", "none");
  params.addParam<MooseEnum>("trigger", getTallyTriggerEnum(),
    "Type of trigger to apply to decide when to terminate the volume calculation");
  params.addClassDescription("Stochastic volume calculation of the OpenMC cells which map to MOOSE");
  return params;
}

OpenMCVolumeCalculation::OpenMCVolumeCalculation(const InputParameters & parameters)
  : GeneralUserObject(parameters),
    OpenMCBase(this, parameters),
    _n_samples(getParam<unsigned int>("n_samples")),
    _trigger(getParam<MooseEnum>("trigger"))
{
  if (_trigger == "none")
    checkUnusedParam(parameters, "trigger_threshold", "not specifying any triggers");
  else if (_trigger == "rel_err")
  {
    checkRequiredParam(parameters, "trigger_threshold", "using a trigger");
    _trigger_threshold = getParam<Real>("trigger_threshold");
  }
  else
    mooseError("Unhandled trigger enum in OpenMCCellVolumeCalculation!");

  _scaling = _openmc_problem->scaling();

  if (isParamValid("lower_left"))
    _lower_left = getParam<Point>("lower_left");
  if (isParamValid("upper_right"))
    _upper_right = getParam<Point>("upper_right");
}
openmc::Position
OpenMCVolumeCalculation::position(const Point & pt) const
{
  openmc::Position p {pt(0), pt(1), pt(2)};
  return p;
}

void
OpenMCVolumeCalculation::resetVolumeCalculation()
{
  auto idx = openmc::model::volume_calcs.begin() + _calc_index;
  openmc::model::volume_calcs.erase(idx);
}

void
OpenMCVolumeCalculation::initializeVolumeCalculation()
{

  BoundingBox box = MeshTools::create_bounding_box(_openmc_problem->getMooseMesh().getMesh());
  if (_fe_problem.getDisplacedProblem() != nullptr)
    _fe_problem.getDisplacedProblem()->updateMesh();

  if (!isParamValid("lower_left"))
    _lower_left = box.min();
  if (!isParamValid("upper_right"))
    _upper_right = box.max();
  if (_lower_left >= _upper_right)
    paramError("upper_right",
               "The 'upper_right' (",
               _upper_right(0),
               ", ",
               _upper_right(1),
               ", ",
               _upper_right(2),
               ") "
               "must be greater than the 'lower_left' (",
               _lower_left(0),
               ", ",
               _lower_left(1),
               ", ",
               _lower_left(2),
               ")!");

  _volume_calc.reset(new openmc::VolumeCalculation());
  _volume_calc->domain_type_ = openmc::VolumeCalculation::TallyDomain::CELL;
  _volume_calc->lower_left_ = position(_lower_left * _scaling);
  _volume_calc->upper_right_ = position(_upper_right * _scaling);
  _volume_calc->n_samples_ = _n_samples;

  if (_trigger == "rel_err")
  {
    _volume_calc->threshold_ = _trigger_threshold;
    _volume_calc->trigger_type_ = openmc::TriggerMetric::relative_error;
  }

  auto cell_to_elem = _openmc_problem->cellToElem();

  std::set<int> ids;
  _index_to_calc_index.clear();

  int i = 0;
  for (const auto & c : cell_to_elem)
  {
    auto index = c.first.first;
    auto id = openmc::model::cells[index]->id_;

    ids.insert(id);
    if (!_index_to_calc_index.count(c.first.first))
      _index_to_calc_index[c.first.first] = i++;
  }

  std::vector<int> domain_ids(ids.begin(), ids.end());
  _volume_calc->domain_ids_ = domain_ids;
  openmc::model::volume_calcs.push_back(*_volume_calc);
  _calc_index = openmc::model::volume_calcs.size();
}

void
OpenMCVolumeCalculation::computeVolumes()
{
  _console << "Running stochastic volume calculation... ";
  _results = _volume_calc->execute();
  _console << "done" << std::endl;
}

void
OpenMCVolumeCalculation::cellVolume(const unsigned int & index, Real & volume, Real & std_dev) const
{
  auto calc_index = _index_to_calc_index.at(index);
  auto n_instances = openmc::model::cells[index]->n_instances_;
  if (n_instances > 1)
    mooseDoOnce(mooseWarning(
        "OpenMC's stochastic volume calculation cannot individually measure volumes of cell "
        "INSTANCES. We assume that no cell instances are clipped by other cells (e.g. they are all "
        "identical) so that the volume of an individual instance is equal to the cell volume "
        "divided by number of instances. For most cases, this is correct - but if you have any "
        "instances which only partially exist in the geometry, this will give INCORRECT volumes."));

  // means add
  volume = _results[calc_index].volume[0] / (_scaling * _scaling * _scaling) / n_instances;

  // but standard deviations only add as the variances
  std_dev = _results[calc_index].volume[1] / (_scaling * _scaling * _scaling) / std::sqrt(n_instances);
}

#endif
