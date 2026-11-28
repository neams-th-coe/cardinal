$(DOUBLEDOWN_BUILDDIR)/Makefile: build_moab build_embree | $(DOUBLEDOWN_DIR)/CMakeLists.txt
	mkdir -p $(DOUBLEDOWN_BUILDDIR)
	sed -i \
	  -e 's/-march=native //' \
	  -e '/set(CMAKE_CXX_FLAGS "$${CMAKE_CXX_FLAGS} -mavx2")/d' \
	  -e '/set(CMAKE_C_FLAGS "$${CMAKE_C_FLAGS} -mavx2")/d' \
	  $(DOUBLEDOWN_DIR)/CMakeLists.txt
	cd $(DOUBLEDOWN_BUILDDIR) && \
	cmake -L \
	-DEMBREE_DIR=$(EMBREE_INSTALL_DIR) \
	-DMOAB_DIR=$(MOAB_INSTALL_DIR) \
	-DCMAKE_BUILD_TYPE=$(BUILD_TYPE) \
	-DCMAKE_C_COMPILER="$(LIBMESH_CC_LIST)" \
	-DCMAKE_CXX_COMPILER="$(LIBMESH_CXX_LIST)" \
	-DCMAKE_Fortran_COMPILER="$(LIBMESH_F90_LIST)" \
	-DCMAKE_PREFIX_PATH=$(LIBMESH_DIR) \
	-DCMAKE_INSTALL_PREFIX=$(DOUBLEDOWN_INSTALL_DIR) \
	-DCMAKE_INSTALL_LIBDIR=lib \
	-DCMAKE_INSTALL_MESSAGE=LAZY \
	$(DOUBLEDOWN_DIR)

build_doubledown: | $(DOUBLEDOWN_BUILDDIR)/Makefile
	$(MAKE) VERBOSE=1 -C $(DOUBLEDOWN_BUILDDIR) install

cleanall_doubledown: | $(DOUBLEDOWN_BUILDDIR)/Makefile
	$(MAKE) -C $(DOUBLEDOWN_BUILDDIR) clean

clobber_doubledown:
	rm -rf $(DOUBLEDOWN_LIB) $(DOUBLEDOWN_BUILDDIR) $(DOUBLEDOWN_INSTALL_DIR)

cleanall: cleanall_doubledown

clobberall: clobber_doubledown

.PHONY: build_doubledown cleanall_doubledown clobber_doubledown
