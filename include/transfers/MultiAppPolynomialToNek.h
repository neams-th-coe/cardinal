#ifndef CARDINAL_MULTIAPPPOLYNOMIALTONEK_H
#define CARDINAL_MULTIAPPPOLYNOMIALTONEK_H

#include "MultiAppTransfer.h"

// Forward declarations
class MultiAppPolynomialToNek;

template<>
InputParameters validParams<MultiAppPolynomialToNek>();

class MultiAppPolynomialToNek :
    public MultiAppTransfer
{
public:
  MultiAppPolynomialToNek(const InputParameters & parameters);
  virtual void execute() override;

protected:
  std::vector<VariableName> _source_variable_names;
  std::vector<VariableName> _to_aux_names;
};

#endif //CARDINAL_MULTIAPPPOLYNOMIALTONEK_H
