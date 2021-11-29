#pragma once

#include "CardinalApp.h"

class CardinalUnitApp : public CardinalApp
{
public:
  CardinalUnitApp(InputParameters parameters);

  static InputParameters validParams();
};
