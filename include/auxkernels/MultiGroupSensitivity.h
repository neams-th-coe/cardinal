#pragma once

#include "AuxKernel.h"
#include "Function.h"
#include "CardinalEnums.h"

#ifdef ENABLE_OPENMC_COUPLING
#include "openmc/mgxs_interface.h"
#include "openmc/reaction.h"
#include "openmc/nuclide.h"
#endif

class MultigroupSensitivityAux : public AuxKernel
{
public:
  static InputParameters validParams();
  MultigroupSensitivityAux(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

private:
  unsigned int _num_groups;
  unsigned int _num_materials;
  unsigned int _from_mat;
  unsigned int _to_mat;

  std::vector<const VariableValue *> _forward_fluxes;
  std::vector<const VariableValue *> _adjoint_fluxes;

  std::vector<std::vector<Real>> _sigma_t;
  std::vector<std::vector<Real>> _sigma_s;
  std::vector<std::vector<std::vector<Real>>> _sigma_r;

  std::vector<std::string> _response_channels;
  std::vector<Real> _energy_bnds;
  bool _load_transport;

  std::vector<const Function *> _sigma_t_funcs;
  std::vector<const Function *> _sigma_s_funcs;
  std::vector<std::vector<const Function *>> _sigma_r_funcs;

  std::vector<Real> resolveGroupBoundaries();
  void computeExplicitXS();
  void computeExplicitResponseXS();

#ifdef ENABLE_OPENMC_COUPLING
  std::vector<int32_t> _openmc_mat_indices;
  std::vector<std::string> _response_nuclides;
  std::vector<int> _response_mts;
  Real _temperature;

  void loadOpenMGMGXS();
  void collapseCEResponseXS(unsigned int channel, unsigned int mat,
                            std::vector<Real> & result);
#endif
};
