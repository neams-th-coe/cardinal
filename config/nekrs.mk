# These have the same names and meanings as in makenrs
NEKRS_NEK5000_PPLIST := PARRSB DPROCMAP

$(NEKRS_BUILDDIR)/Makefile: $(NEKRS_DIR)/CMakeLists.txt
	mkdir -p $(NEKRS_BUILDDIR)
	cd $(NEKRS_BUILDDIR) && \
	cmake -L -Wno-dev -Wfatal-errors \
	-DCMAKE_BUILD_TYPE="$(BUILD_TYPE)" \
	-DCMAKE_C_COMPILER="$(LIBMESH_CC_LIST)" \
	-DCMAKE_CXX_COMPILER="$(LIBMESH_CXX_LIST)" \
	-DCMAKE_Fortran_COMPILER="$(LIBMESH_F90_LIST)" \
	-DCMAKE_INSTALL_PREFIX="$(NEKRS_INSTALL_DIR)" \
	-DNEK5000_PPLIST="$(NEKRS_NEK5000_PPLIST)" \
	-DOCCA_CXX="$(libmesh_CC)" \
	-DENABLE_CUDA="$(OCCA_CUDA_ENABLED)" \
	-DENABLE_OPENCL="$(OCCA_OPENCL_ENABLED)" \
	-DENABLE_HIP="$(OCCA_HIP_ENABLED)" \
	$(NEKRS_DIR)

build_nekrs: | $(NEKRS_BUILDDIR)/Makefile
	make -C $(NEKRS_BUILDDIR) install

cleanall_nekrs: |  $(NEKRS_BUILDDIR)/Makefile
	make -C $(NEKRS_BUILDDIR) uninstall clean

clobber_nekrs:
	rm -rf $(NEKRS_LIB) $(NEKRS_BUILDDIR) $(NEKRS_INSTALL_DIR)

# cleanall and clobberall are from moose.mk
cleanall: cleanall_nekrs

clobberall: clobber_nekrs

.PHONY: build_nekrs cleanall_nekrs clobber_nekrs
