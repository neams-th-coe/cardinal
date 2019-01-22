#ifndef CARDINAL_LEGENDREPOLYNOMIAL_H
#define CARDINAL_LEGENDREPOLYNOMIAL_H

#include "Function.h"
#include "math.h"

class LegendrePolynomial : public Function
{
public:
  LegendrePolynomial(const InputParameters & parameters);
  virtual ~LegendrePolynomial();

  virtual Real value(Real t, const Point & p);
  Real getPolynomialValue(Real t, Real p, int n);

protected:

private:

  std::vector<Real> _geom_norm;  // Pin Radius that is used for normalization
  Real _dz;                      // Total length of the domain
  bool _dbg;                     // Debug flag to print debug output
};

template<>
InputParameters validParams<LegendrePolynomial>();

#endif //CARDINAL_LEGENDREPOLYNOMIAL_H
