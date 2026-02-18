import struct
import sys
import re
import os

# The time-averaged files written by NekRS are all written at time zero to facilitate averaging
# of those files together. However, this prevents those files from then being viewed in Paraview,
# since Paraview likes to have files ordered sequentially in time. This script changes the time
# index in the avg files so that you can view the files in Paraview. This file originally
# comes from https://github.com/yslan/nek-utils/tree/main/usr_extra

os.chdir(os.getcwd())

if len(sys.argv) !=2 :
    print('\nChange the time inside *0.f0* file based on timestep')
    print('Usage: python3 ./%s <case_name> \n\n'%(sys.argv[0]))
    quit(0)

fnek5000 = 'avg' + sys.argv[1] + '.nek5000'

try:
    print('Reading '+fnek5000,end='')
    f = open(fnek5000)

except FileNotFoundError:
    print('\n%s file not found!'%fnek5000)
    quit(1)
else:
    with f:
        inlines = f.readlines()
        cname = inlines[0].split()[1].split('%')[0]
        regx = inlines[0].split()[1].split('%')[1]
        firsttimestep = int(inlines[1].split()[1])
        numtimesteps = int(inlines[2].split()[1])
        num_zero = int(regx[1])

for i in range(numtimesteps):
    ifile = i + firsttimestep
    fname = cname+'0'*num_zero+'.f%05d'%ifile
    print('  %s'%fname,end ="")

    with open(fname,'r+b') as f:
        inline = f.read(132).decode("utf-8")

        header = inline.split()
        time = float(header[7])
        if (i == 0):
          newTime = time
        else:
          newTime += time

        timestep = int(header[8])

        newHeader = inline[:38] + '%20.13E'%newTime + inline[58:]
        print('   step=%d time=%g newTime=%g'%(timestep,time,newTime))
        f.seek(0)
        f.write(newHeader.encode("utf-8"))
