DAGMC_BUILDDIR := $(CARDINAL_DIR)/build/DAGMC
DAGMC_INSTALL_DIR := $(CONTRIB_INSTALL_DIR)

# By default, build dagmc
BUILD_DAGMC ?= yes

ifeq ($(BUILD_DAGMC),yes)

$(DAGMC_BUILDDIR)/Makefile: build_doubledown | $(DAGMC_DIR)/CMakeLists.txt
	mkdir -p $(DAGMC_BUILDDIR)
	cd $(DAGMC_BUILDDIR) && \
	cmake -L \
	-DBUILD_TALLY=OFF \
	-DBUILD_TESTS=OFF \
	-DBUILD_CI_TESTS=OFF \
	-DBUILD_STATIC_LIBS=OFF \
	-DBUILD_STATIC_EXE=OFF \
	-DBUILD_UWUW=OFF \
  -DENABLE_NETCDF=ON \
	-DDOUBLE_DOWN=$(ENABLE_DOUBLE_DOWN) \
	-DDOUBLE_DOWN_DIR=$(DOUBLEDOWN_INSTALL_DIR) \
	-DEigen3_DIR=$(EIGEN3_DIR) \
	-DMOAB_DIR=$(MOAB_INSTALL_DIR) \
	-DCMAKE_BUILD_TYPE=$(BUILD_TYPE) \
	-DCMAKE_C_COMPILER="$(LIBMESH_CC_LIST)" \
	-DCMAKE_CXX_COMPILER="$(LIBMESH_CXX_LIST)" \
	-DCMAKE_Fortran_COMPILER="$(LIBMESH_F90_LIST)" \
	-DCMAKE_PREFIX_PATH=$(LIBMESH_DIR) \
	-DCMAKE_INSTALL_PREFIX=$(DAGMC_INSTALL_DIR) \
	-DCMAKE_INSTALL_LIBDIR=lib \
	-DCMAKE_INSTALL_MESSAGE=LAZY \
	$(DAGMC_DIR)

build_dagmc: | $(DAGMC_BUILDDIR)/Makefile
	make VERBOSE=1 -C $(DAGMC_BUILDDIR) install

cleanall_dagmc: | $(DAGMC_BUILDDIR)/Makefile
	make -C $(DAGMC_BUILDDIR) clean

clobber_dagmc:
	rm -rf $(DAGMC_LIB) $(DAGMC_BUILDDIR) $(DAGMC_INSTALL_DIR)

else # BUILD_DAGMC=no

DAGMC_INSTALL_DIR = $(DAGMC_DIR)
build_dagmc:
	@echo "Using pre-built dagmc from $(DAGMC_INSTALL_DIR)"

cleanall_dagmc:
	@echo "Not cleaning pre-built dagmc"

clobber_dagmc:
	@echo "Not clobbering pre-built dagmc"

endif # BUILD_DAGMC

cleanall: cleanall_dagmc

clobberall: clobber_dagmc

.PHONY: build_dagmc cleanall_dagmc clobber_dagmc
