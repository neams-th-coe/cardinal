OPENMC_OBJDIR := $(CURDIR)/obj_openmc
OPENMC_LIBDIR := $(CURDIR)/lib
OPENMC_LIB := $(OPENMC_LIBDIR)/libopenmc.so

$(OPENMC_LIB): 
	mkdir -p $(OPENMC_OBJDIR)
	cd $(OPENMC_OBJDIR) && cmake -DCMAKE_INSTALL_PREFIX=$(CURDIR) $(OPENMC_DIR)
	make -C $(OPENMC_OBJDIR) install

clean_openmc:
	make -C $(OPENMC_OBJDIR) clean

libopenmc: $(OPENMC_LIB)

.PHONY: libopenmc clean_openmc
