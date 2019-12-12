FROM herter4171/ubuntu-moose:5294b5aa433e0a5985e20e28755db01a91b356ec

# OpenMC variables
ENV OPENMC_CROSS_SECTIONS=$HOME/nndc_hdf5/cross_sections.xml \
    OPENMC_ENDF_DATA=$HOME/endf-b-vii.1 \
    CPATH=/usr/include/hdf5/mpich/

# OpenMC and NekRS packages
RUN apt-get update -y && \
    apt-get install -y libhdf5-serial-dev libhdf5-mpich-dev && \
    apt-get install -y wget && \
    apt-get autoremove

# OpenMC cross-sections
RUN wget -q -O - https://anl.box.com/shared/static/pzutl4i2717yypv12l78l7fn5nmg6grs.xz | tar -C $HOME -xJ
RUN wget -q -O - https://anl.box.com/shared/static/4kd2gxnf4gtk4w1c8eua5fsua22kvgjb.xz | tar -C $HOME -xJ