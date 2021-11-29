#include "CardinalUnitApp.h"
#include "Moose.h"

InputParameters CardinalUnitApp::validParams()
{
  InputParameters params = CardinalApp::validParams();
  return params;
}

CardinalUnitApp::CardinalUnitApp(InputParameters parameters) :
    CardinalApp(parameters)
{
}
