/********************************************************************/
/*                  SOFTWARE COPYRIGHT NOTIFICATION                 */
/*                             Cardinal                             */
/*                                                                  */
/*                  (c) 2021 UChicago Argonne, LLC                  */
/*                        ALL RIGHTS RESERVED                       */
/*                                                                  */
/*                 Prepared by UChicago Argonne, LLC                */
/*               Under Contract No. DE-AC02-06CH11357               */
/*                With the U. S. Department of Energy               */
/*                                                                  */
/*             Prepared by Battelle Energy Alliance, LLC            */
/*               Under Contract No. DE-AC07-05ID14517               */
/*                With the U. S. Department of Energy               */
/*                                                                  */
/*                 See LICENSE for full restrictions                */
/********************************************************************/

#pragma once

#include "MooseObjectAction.h"
#include "inipp.hpp"

/**
 * Initialize Nek application by calling nekrs::setup. This needs to be
 * in an action so that it occurs before the [Mesh] is constructed (which
 * is normally the first MOOSE object constructed) because we build the
 * [Mesh] relying on internal stuff in NekRS, which needs to be initialized
 * at that point.
 */
class NekInitAction : public MooseObjectAction
{
public:
  static InputParameters validParams();

  NekInitAction(const InputParameters & parameters);

  virtual void act() override;

protected:
  /**
   * Copied from NekRS because they do not want to move this to a file
   * other than main.cpp :|
   */
  std::map<std::string, std::map<std::string, std::string>> readPar(const std::string & _setupFile,
                                                                    MPI_Comm comm);

  /// whether the user specified how many scratch slots to allocate
  const bool _specified_scratch;

  /**
   * Number of slices/slots to allocate in nrs->usrwrk to hold fields
   * for coupling (i.e. data going into NekRS, written by Cardinal), or
   * used for custom user actions, but not for coupling. By default, we just
   * allocate 7 slots (no inherent reason, just a fairly big amount). For
   * memory-limited cases, you can reduce this number to just the bare
   * minimum necessary for your use case.
   */
  const unsigned int & _n_usrwrk_slots;

  /// number of NekRS cases created
  static int _n_cases;
};
