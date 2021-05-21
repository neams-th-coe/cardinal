#pragma once

#include "MooseApp.h"

class NekApp;

template <>
InputParameters validParams<NekApp>();

/**
 * Creates a nekRS application wrapped as a MOOSE application.
 */
class NekApp : public MooseApp
{
public:
  explicit NekApp(InputParameters parameters);
  virtual ~NekApp();

  static void registerApps();
  static void registerAll(Factory & f, ActionFactory & af, Syntax & s);

private:
  /**
   * Case name to specify the nekRS input files; this is the "case" in the
   * case.par, case.udf, case.oudf, and case.re2 files.
   */
  std::string _setup_file;

  int _size_target;

  int _build_only;

  int _ci_mode;

  std::string _cache_dir;
};
