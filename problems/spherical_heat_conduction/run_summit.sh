export NEKRS_HOME="$(realpath ../..)"
export OPENMC_CROSS_SECTIONS=/gpfs/alpine/nfi114/proj-shared/rahaman/cross_sections/nndc_hdf5/cross_sections.xml
export OPENMC_ENDF_DATA=/gpfs/alpine/nfi114/proj-shared/rahaman/cross_sections/endf-b-vii.1

bsub -env "all" -nnodes 8 -W 30 -P nfi114 -J cardinal jsrun -X 1 -n48 -r6 -a1 -c2 -g1 -b packed:1 -d packed ../../cardinal-opt --app nek -i nek.i --nekrs-setup onepebble2
