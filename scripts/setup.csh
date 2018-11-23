#!/bin/csh

#source /cvmfs/sft.cern.ch/lcg/releases/LCG_94/ROOT/6.14.04/x86_64-slc6-gcc62-opt/ROOT-env.sh
    
#setenv LD_LIBRARY_PATH DynamicTTree/lib/:$LD_LIBRARY_PATH


setenv LD_LIBRARY_PATH ./lib:DynamicTTree/lib/:CfgManager/lib/:$LD_LIBRARY_PATH
setenv ROOT_INCLUDE_PATH ./interface:DynamicTTree/interface/:CfgManager/interface/:$ROOT_INCLUDE_PATH

