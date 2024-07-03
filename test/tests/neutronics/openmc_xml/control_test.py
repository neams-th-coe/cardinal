# coding: utf-8
from MooseControl import MooseControl
cmd='/home/pshriwise/soft/cardinal-dev/cardinal-opt -i openmc.i server.i --n-threads=10'
control = MooseControl(cmd.split(), moose_control_name='webserver')
control.initialize()
