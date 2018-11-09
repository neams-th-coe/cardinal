#ifndef CARDINAL_FOURIERLEGENDRERECONSTRUCTION_H
#define CARDINAL_FOURIERLEGENDRERECONSTRUCTION_H

#include "Function.h"
#include "math.h"
#include "FourierPolynomial.h"
#include "LegendrePolynomial.h"

class FourierLegendreReconstruction : public Function
{
public:
  FourierLegendreReconstruction(const InputParameters & parameters);
  virtual ~FourierLegendreReconstruction();

  virtual Real value(Real t, const Point & p);

protected:

  /// Debug flag to print debug output
  bool _dbg;
  /// The coordinate directions of the integration for Legendre polynomial
  int _l_direction;
  /// The coordinate directions of the itnegration for Fourier polynomials
  int _f_direction_1;
  int _f_direction_2;
  /// The order of the Legendre expansion
  int _l_order;
  /// The order of the Fourier expansion
  int _f_order;
  /// The vector of scalar variables holding the coefficients
  std::vector<VariableValue *> _poly_scalars;
  /// Legendre polynomial object.  TODO address dynamic casting of this object
  LegendrePolynomial & _legendre_poly_function;
  /// Fourier polynomial object.  TODO address dynamic casting of this object
  FourierPolynomial & _fourier_poly_function;

private:

};

template<>
InputParameters validParams<FourierLegendreReconstruction>();

#endif //CARDINAL_FOURIERLEGENDRERECONSTRUCTION_H
