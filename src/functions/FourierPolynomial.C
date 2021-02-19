#include "FourierPolynomial.h"

template<>
InputParameters validParams<FourierPolynomial>()
{
  InputParameters params = validParams<Function>();
  params.addParam<bool>("dbg", "Print debug output");
  return params;
}

FourierPolynomial::FourierPolynomial(const InputParameters & parameters) :
    Function(parameters),
    _dbg(parameters.get<bool>("dbg"))
{
}

FourierPolynomial::~FourierPolynomial()
{
}

Real
FourierPolynomial::value(Real /*t*/, const Point & /*p*/) const
{
  mooseWarning("value() in FourierPolynomial should not be used");
  return 0.0;
}

Real
FourierPolynomial::getPolynomialValue(Real /*t*/, Real p1, Real p2, int n)
{
  Real phi = atan2(p2, p1);
  if (n == 0)
    return 1.0 / ( sqrt(2.0 * M_PI) );
  else
    return cos(n * phi) / ( sqrt(M_PI) );
}
