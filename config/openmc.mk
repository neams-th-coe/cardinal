OPENMC_OBJDIR := $(CURDIR)/obj_openmc
OPENMC_LIBDIR := $(OPENMC_OBJDIR)/lib
OPENMC_LIB := $(OPENMC_LIBDIR)/libopenmc.so

$(OPENMC_LIB):
	mkdir -p $(OPENMC_OBJDIR)
	mkdir -p $(LIBMESH_DIR)/include/contrib
	cd $(OPENMC_OBJDIR) && \
	cmake -L -Dlibmesh=ON -Doptimize=ON -DLIBMESH_DIR=$(LIBMESH_DIR) -DCMAKE_INSTALL_PREFIX=$(CURDIR) $(OPENMC_DIR) && \
	cmake $(OPENMC_DIR) && make VERBOSE=1 -C $(OPENMC_OBJDIR) install

clean_openmc:
	make -C $(OPENMC_OBJDIR) clean

libopenmc: $(OPENMC_LIB)

.PHONY: libopenmc clean_openmc
