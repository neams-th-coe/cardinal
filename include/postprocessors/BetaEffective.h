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

#include "GeneralPostprocessor.h"

#include "OpenMCBase.h"

class BetaEffective : public GeneralPostprocessor, public OpenMCBase
{
public:
  static InputParameters validParams();

  BetaEffective(const InputParameters & parameters);

  virtual void initialize() override {}
  virtual void execute() override {}

  virtual Real getValue() const override;

protected:
  /// The value of the kinetics parameter to output.
  const statistics::OutputEnum _output;

  const enum class BetaTypeEnum {
    Sum = 0,
    D_1 = 1,
    D_2 = 2,
    D_3 = 3,
    D_4 = 4,
    D_5 = 5,
    D_6 = 6
  } _beta_type;
};
