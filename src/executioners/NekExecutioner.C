#include "NekExecutioner.h"
#include "NekInterface.h"

template<>
InputParameters validParams<NekExecutioner>()
{
  InputParameters params = validParams<Transient>();
  return params;
}

NekExecutioner::NekExecutioner(const InputParameters & parameters) :
    Transient(parameters)
{
}

void NekExecutioner::init() {
  Transient::init();
  //FORTRAN_CALL(Nek5000::nek_init)(_communicator.get());
}

void NekExecutioner::preStep() {
  Transient::preStep();
  //FORTRAN_CALL(Nek5000::nek_init_step)();
}

void NekExecutioner::postStep() {
  Transient::postStep();
  //FORTRAN_CALL(Nek5000::nek_finalize_step)();
}
