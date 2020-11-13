export NEKRS_INSTALL_DIR := $(CONTRIB_INSTALL_DIR)
export NEKRS_LIBDIR := $(NEKRS_INSTALL_DIR)/lib
export NEKRS_LIB := $(NEKRS_LIBDIR)/libnekrs.so 

export NEKRS_CC := $(libmesh_CC)
export NEKRS_CXX := $(libmesh_CXX)
export NEKRS_FC := $(libmesh_FC)

export HYPRE_DIR := $(HYPRE_DIR)

export OCCA_CUDA_ENABLED
export OCCA_HIP_ENABLED
export OCCA_OPENCL_ENABLED

libnekrs: $(NEKRS_LIB)

$(NEKRS_LIB): .FORCE
	cd $(NEKRS_DIR) && ./makenrs

clean_nekrs:
	cd $(NEKRS_DIR) && ./makenrs clean

.FORCE: 

