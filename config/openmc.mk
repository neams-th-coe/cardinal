cmake_openmc:
	mkdir -p $(OPENMC_BUILDDIR)
	mkdir -p $(LIBMESH_DIR)/include/contrib
	cd $(OPENMC_BUILDDIR) && cmake -L -Dlibmesh=ON -Doptimize=ON -DLIBMESH_DIR=$(LIBMESH_DIR) -DCMAKE_INSTALL_PREFIX=$(CONTRIB_INSTALL_DIR) $(OPENMC_DIR)

build_openmc: | cmake_openmc
	make VERBOSE=1 -C $(OPENMC_BUILDDIR) install

cleanall_openmc: | cmake_openmc
	make -C $(OPENMC_BUILDDIR) uninstall clean

clobber_openmc:
	rm -rf $(OPENMC_LIB) $(OPENMC_BUILDDIR) $(OPENMC_INSTALL_DIR)

cleanall: cleanall_openmc

clobberall: clobber_openmc

.PHONY: build_openmc cmake_openmc cleanall_openmc clobber_openmc

