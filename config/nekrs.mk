export NEKRS_INSTALL_DIR := $(CURDIR)
export NEKRS_OBJDIR := $(CURDIR)/obj_nekrs
export NEKRS_LIBDIR := $(NEKRS_INSTALL_DIR)/lib
export NEKRS_LIB := $(NEKRS_LIBDIR)/libnekrs.so

export NEKRS_CC := $(libmesh_CC)
export NEKRS_CXX := $(libmesh_CXX)
export NEKRS_FC := $(libmesh_FC)
#export NEKRS_CXXFLAGS := $(CXXFLAGS) -DUSE_OCCA_MEM_BYTE_ALIGN=32
export NEKRS_CXXFLAGS := -O2 -g -DUSE_OCCA_MEM_BYTE_ALIGN=32

export OCCA_CUDA_ENABLED
export OCCA_HIP_ENABLED
export OCCA_OPENCL_ENABLED

$(NEKRS_LIB): .FORCE
	mkdir -p $(NEKRS_OBJDIR)
	cd $(NEKRS_DIR) && ./makenrs

clean_nekrs:
	cd $(NEKRS_DIR) && ./makenrs clean

.FORCE: 

