#ifndef CARDINAL_NEKAPP_H
#define CARDINAL_NEKAPP_H

#include "MooseApp.h"

class NekApp;

template <>
InputParameters validParams<NekApp>();

class NekApp : public MooseApp
{
public:
  explicit NekApp(InputParameters parameters);
  virtual ~NekApp();

  static void registerApps();
  static void registerAll(Factory & f, ActionFactory & af, Syntax & s);

private:
  std::string _setup_file;
  int _size_target;
  int _build_only;
  int _ci_mode;
  std::string _cache_dir;
};

#endif //CARDINAL_NEKAPP<_H
