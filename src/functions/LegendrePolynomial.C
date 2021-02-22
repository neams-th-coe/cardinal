#include "LegendrePolynomial.h"

template<>
InputParameters validParams<LegendrePolynomial>()
{
  InputParameters params = validParams<Function>();
  params.addRequiredParam<std::vector<Real> >("l_geom_norm","Lengths needed for Legendre polynomial normalization (min, max)");
  params.addParam<bool>("dbg", false, "Print debug output");
  return params;
}

LegendrePolynomial::LegendrePolynomial(const InputParameters & parameters) :
    Function(parameters),
    _geom_norm(parameters.get<std::vector<Real> >("l_geom_norm")),
    _dbg(parameters.get<bool>("dbg"))
{
  _dz = _geom_norm[1] - _geom_norm[0];
}

LegendrePolynomial::~LegendrePolynomial()
{
}

Real
LegendrePolynomial::value(Real /*t*/, const Point & /*p*/) const
{
  mooseWarning("value() in LegendrePolynomial should not be used");
  return 0.0;
}

Real
LegendrePolynomial::getPolynomialValue(Real /*t*/, Real p, int n)
{
  Real z;          // Normalized position
  Real plm2 = 0.0; // L-2 Legendre polynomial value
  Real plm1 = 0.0; // L-1 Legendre polynomial value
  Real plm = 0.0;  // L   Legendre polynomial value

  z = 2.0 * (p - _geom_norm[0])/(_dz) - 1.0;

  if(_dbg)
  {
    Moose::out<<"point: " << p << std::endl;
    Moose::out<<"dz = "<< _dz << std::endl;
    Moose::out<<"_geom_norm[0] = "<<_geom_norm[0]<<std::endl;
    Moose::out<<"Normalized z = "<<z<<std::endl;
  }
  // 0th order
  if (n == 0) // O order
    return 1.0 / sqrt(2.0);
  if (n == 1) // 1 order
    return sqrt(3.0 / 2.0) * z;
  else
  {
    plm2 = 1.0;
    plm1 = z;
    for(int ii=2; ii <= n; ii++)
    {
      plm = z*(2.0*ii - 1.0)/ii * plm1 - (ii - 1.0)/(ii)*plm2;
      plm2 = plm1;
      plm1 = plm;
    }
    if(_dbg)
      Moose::out<<"Legendre total value  = "<<(plm * (2.0 * n + 1.0) / _dz)<<std::endl;
    return (plm * sqrt((2.0 * n + 1.0) / 2.0));
  }
}
