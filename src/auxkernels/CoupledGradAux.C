#include "CoupledGradAux.h"

template<>
InputParameters validParams<CoupledGradAux>()
{
  InputParameters params = validParams<AuxKernel>();

  params.addRequiredCoupledVar("coupled", "Coupled gradient for calculation");
  params.addParam<MaterialPropertyName>("diffusion_coefficient_name",
                                        "thermal_conductivity",
                                        "Property name of the diffusivity (Default: thermal_conductivity)");
  return params;
}

CoupledGradAux::CoupledGradAux(const InputParameters & parameters) :
    AuxKernel(parameters),
    _coupled(coupled("coupled")),
    _coupled_grad(coupledGradient("coupled")),
    _diffusion_coefficient(getMaterialProperty<Real>("diffusion_coefficient_name"))
{
}

CoupledGradAux::~CoupledGradAux()
{
}

Real
CoupledGradAux::computeValue()
{
  return _coupled_grad[_qp].norm() * _diffusion_coefficient[_qp];
}
