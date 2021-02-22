#include "FourierLegendreReconstruction.h"

template<>
InputParameters validParams<FourierLegendreReconstruction>()
{
  InputParameters params = validParams<Function>();
  params.addParam<bool>("dbg", false, "Print debug output");
  params.addRequiredParam<int>("l_order", "The order of the Legendre expansion");
  params.addRequiredParam<int>("f_order", "The order of the Fourier expansion");
  params.addRequiredParam<int>("l_direction", "Direction of integration for Legendre polynomial");
  params.addRequiredParam<std::string>("legendre_function_name", "Name of function to compute Legendre polynomial value at a point");
  params.addRequiredParam<std::string>("fourier_function_name", "Name of function to compute Fourier polynomial value at a point");
  params.addRequiredCoupledVar("poly_scalars", "Name of aux variable containing the Zernike coefficients");
  return params;
}

FourierLegendreReconstruction::FourierLegendreReconstruction(const InputParameters & parameters) :
    Function(parameters),
    _dbg(parameters.get<bool>("dbg")),
    _l_direction(parameters.get<int>("l_direction")),
    _l_order(parameters.get<int>("l_order")),
    _f_order(parameters.get<int>("f_order")),
    _legendre_poly_function(dynamic_cast<LegendrePolynomial&>(_mci_feproblem.getFunction(parameters.get<std::string>("legendre_function_name")))),
    _fourier_poly_function(dynamic_cast<FourierPolynomial&>(_mci_feproblem.getFunction(parameters.get<std::string>("fourier_function_name"))))
{

  // For now hard code that l_direction better be 2
  // TODO come back and put logic in to determine f diections given l directions
  if ( _l_direction == 2)
  {
    _f_direction_1 = 0;
    _f_direction_2 = 1;
  }
  else
  {
    mooseError("Need to implement logic for l direction not equal to 2");
  }

  // Get the coupled scalar variables storing the expansion coefficients
  for(int i=0; i < _f_order; i++)
    _poly_scalars.push_back(&coupledScalarValue("poly_scalars",i));

}

FourierLegendreReconstruction::~FourierLegendreReconstruction()
{
}

Real
FourierLegendreReconstruction::value(Real t, const Point & p) const
{
  Real val = 0.0;

  for (int f=0; f<_f_order; f++)
  {
    for (int l=0; l<_l_order; l++)
    {
      val += (*_poly_scalars[f])[l]
          * _legendre_poly_function.getPolynomialValue(t,p(_l_direction),l)
          * _fourier_poly_function.getPolynomialValue(t,p(_f_direction_1),p(_f_direction_2),f);
    }
  }

  return val;
}
