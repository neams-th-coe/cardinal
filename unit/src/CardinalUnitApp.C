#include "CardinalUnitApp.h"
#include "Moose.h"

template<>
InputParameters validParams<CardinalUnitApp>()
{
  InputParameters params = validParams<CardinalApp>();
  return params;
}

CardinalUnitApp::CardinalUnitApp(InputParameters parameters) :
    CardinalApp(parameters)
{
}
