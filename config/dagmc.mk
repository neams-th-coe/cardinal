$(DAGMC_BUILDDIR)/Makefile: build_moab | $(DAGMC_DIR)/CMakeLists.txt
	mkdir -p $(DAGMC_BUILDDIR)
	cd $(DAGMC_BUILDDIR) && \
	cmake -L \
	-DBUILD_TALLY=OFF \
	-DBUILD_TESTS=OFF \
	-DBUILD_CI_TESTS=OFF \
	-DBUILD_STATIC_LIBS=OFF \
	-DBUILD_STATIC_EXE=OFF \
	-DMOAB_DIR=$(MOAB_INSTALL_DIR) \
	-DCMAKE_BUILD_TYPE=$(BUILD_TYPE) \
	-DCMAKE_C_COMPILER="$(LIBMESH_CC_LIST)" \
	-DCMAKE_CXX_COMPILER="$(LIBMESH_CXX_LIST)" \
	-DCMAKE_Fortran_COMPILER="$(LIBMESH_F90_LIST)" \
	-DCMAKE_PREFIX_PATH=$(LIBMESH_DIR) \
	-DCMAKE_INSTALL_PREFIX=$(DAGMC_INSTALL_DIR) \
	-DCMAKE_INSTALL_LIBDIR=lib \
	$(DAGMC_DIR)

build_dagmc: | $(DAGMC_BUILDDIR)/Makefile
	make VERBOSE=1 -C $(DAGMC_BUILDDIR) install

cleanall_dagmc: | $(DAGMC_BUILDDIR)/Makefile
	make -C $(DAGMC_BUILDDIR) clean

clobber_dagmc:
	rm -rf $(DAGMC_LIB) $(DAGMC_BUILDDIR) $(DAGMC_INSTALL_DIR)

cleanall: cleanall_dagmc

clobberall: clobber_dagmc

.PHONY: build_dagmc cleanall_dagmc clobber_dagmc
