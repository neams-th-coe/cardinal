#pragma once

#include "CardinalApp.h"

class CardinalUnitApp;

template<>
InputParameters validParams<CardinalUnitApp>();

class CardinalUnitApp : public CardinalApp
{
public:
  CardinalUnitApp(InputParameters parameters);
};

