define n


endef

ifneq ($(NEKRS_HOME), $(CONTRIB_INSTALL_DIR))
  $(error $n$n"Environment variable NEKRS_HOME needs to be set to $(CONTRIB_INSTALL_DIR)". It is currently set to $(NEKRS_HOME).")
endif
