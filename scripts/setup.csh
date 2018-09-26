#!/bin/csh

#source /afs/cern.ch/sw/lcg/external/gcc/4.9/x86_64-slc6-gcc49-opt/setup.csh
#cd /afs/cern.ch/sw/lcg/app/releases/ROOT/6.05.02/x86_64-slc6-gcc49-opt/root/
#source ./bin/thisroot.csh
#cd -
#setenv LD_LIBRARY_PATH DynamicTTree/lib/:$LD_LIBRARY_PATH


#bash source /cvmfs/sft.cern.ch/lcg/releases/LCG_94/ROOT/6.14.04/x86_64-slc6-gcc62-opt/bin/thisroot.csh
setenv LD_LIBRARY_PATH ./lib:DynamicTTree/lib/:CfgManager/lib/:$LD_LIBRARY_PATH
setenv ROOT_INCLUDE_PATH ./interface:DynamicTTree/interface/:CfgManager/interface/:$ROOT_INCLUDE_PATH


