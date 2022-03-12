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

#pragma once

// MOOSE includes
#include "NearestPointBase.h"
#include "ElementAverageValue.h"
#include "ElementVariableVectorPostprocessor.h"
/**
 * Given a list of points this object computes the variable integral
 * closest to each one of those points.
 */
class CardinalNearestPointAverage
  : public NearestPointBase<ElementAverageValue, ElementVariableVectorPostprocessor>
{
public:
  CardinalNearestPointAverage(const InputParameters & parameters);

  static InputParameters validParams();

  virtual Real spatialValue(const Point & point) const override;

  Real userObjectValue(unsigned int i) const;

  unsigned int nearestPointIndex(const Point & point) const;

  virtual void finalize() override;

protected:
  VectorPostprocessorValue & _np_post_processor_values;
};
