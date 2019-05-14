#!/usr/bin/env python
import os, re
import commands
import math, time
import sys
import argparse
import subprocess

parser = argparse.ArgumentParser(description='This script splits H4Reco tasks in multiple jobs and sends them on HTCondor')

parser.add_argument("-r",  "--run",            required=True,     type=str,   help="run number")
parser.add_argument("-b",  "--baseFolder",     required=True,     type=str,   help="base folder")
parser.add_argument("-e",  "--exeName",        required=True,     type=str,   help="absolute path of executable")
parser.add_argument("-c",  "--configFile",     required=True,     type=str,   help="config file")
parser.add_argument("-s",  "--submit",                                        help="submit jobs", action='store_true')
parser.add_argument("-v",  "--verbose",                                       help="increase output verbosity", action='store_true')

args = parser.parse_args()



#print 
#print 'START'
#print 

currDir = os.getcwd()

#print

##### creates directory and file list for job #######
jobDir = currDir+'/jobs_condor/run'+args.run+'/'
print 'Job dir :', jobDir
os.system('mkdir '+jobDir)
os.system('mkdir '+jobDir+'output/')
os.system('mkdir '+jobDir+'error/')
os.system('mkdir '+jobDir+'log/')
os.chdir(jobDir)
   
##### creates config file #######
with open(args.baseFolder+'/'+args.configFile) as fi:
   contents = fi.read()
   with open(jobDir+"/config.cfg", "w") as fo:
      fo.write(contents)

##### creates jobs scripts #######
with open('job_%s.sh'%(str(args.run)), 'w') as fout:
   fout.write("#!/bin/sh\n")
   fout.write("echo\n")
   fout.write("echo 'START---------------'\n")
   fout.write("echo 'current dir: ' ${PWD}\n")
   fout.write("cd "+str(args.baseFolder)+"\n")
   fout.write("echo 'current dir: ' ${PWD}\n")
   fout.write("source scripts/setup.sh\n")
   fout.write(args.exeName+" "+jobDir+"/config.cfg "+args.run+"\n")
   fout.write("echo 'STOP---------------'\n")
   fout.write("echo\n")
   fout.write("echo\n")
   os.system("chmod 755 job_%s.sh"%(str(args.run)))


