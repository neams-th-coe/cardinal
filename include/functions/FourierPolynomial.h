#ifndef CARDINAL_FOURIERPOLYNOMIAL_H
#define CARDINAL_FOURIERPOLYNOMIAL_H

#include "Function.h"
#include "math.h"

class FourierPolynomial : public Function
{
public:
  FourierPolynomial(const InputParameters & parameters);
  virtual ~FourierPolynomial();

  virtual Real value(Real t, const Point & p);
  Real getPolynomialValue(Real t, Real p1, Real p2,  int n);

protected:

private:

  bool _dbg;                     // Debug flag to print debug output
};

template<>
InputParameters validParams<FourierPolynomial>();

#endif //CARDINAL_FOURIERPOLYNOMIAL_H
