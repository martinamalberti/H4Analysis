#!/usr/bin/env python
import os, re
import commands
import math, time
import sys
import argparse
import subprocess

parser = argparse.ArgumentParser(description='This script splits CMSSW tasks in multiple jobs and sends them on LXBATCH')

parser.add_argument("-r",  "--run",            required=True,     type=str,   help="run number")
parser.add_argument("-b",  "--baseFolder",     required=True,     type=str,   help="base folder")
parser.add_argument("-e",  "--exeName",        required=True,     type=str,   help="absolute path of executable")
parser.add_argument("-c",  "--configFile",     required=True,     type=str,   help="config file")
parser.add_argument("-q",  "--queue",          default="1nh",     type=str,   help="hercules queue to use")
parser.add_argument("-s",  "--submit",                                        help="submit jobs", action='store_true')
parser.add_argument("-v",  "--verbose",                                       help="increase output verbosity", action='store_true')
parser.add_argument("-p1", "--tableX",         required=True,     type=float, help="")
parser.add_argument("-p2", "--tableY",         required=True,     type=float, help="")
parser.add_argument("-p3", "--sensorconf",     required=True,     type=str,   help="")
parser.add_argument("-p4", "--digiconf",       required=True,     type=str,   help="")
parser.add_argument("-p5", "--thr",            required=True,     type=float, help="")
parser.add_argument("-p6", "--vbias1",         required=True,     type=float, help="")
parser.add_argument("-p7", "--vbias2",         required=True,     type=float, help="")
parser.add_argument("-p8", "--vbiascal",       required=True,     type=float, help="")
parser.add_argument("-p9", "--vbiasmat",       required=True,     type=float, help="")

args = parser.parse_args()



#print 
#print 'START'
#print 

currDir = os.getcwd()

#print

##### creates directory and file list for job #######
jobDir = currDir+'/jobs/run'+args.run+'/'
os.system('mkdir '+jobDir)
os.chdir(jobDir)
   
##### creates config file #######
with open(args.baseFolder+'/'+args.configFile) as fi:
   contents = fi.read()
   with open(jobDir+"/config.cfg", "w") as fo:
      fo.write(contents)
   command = 'sed -i \"s%^originValues .*$%originValues '+str(args.run)+'%\" '+jobDir+'/config.cfg'
   os.system(command)
   command = 'sed -i \"s%^tableX .*$%tableX '+str(args.tableX)+'%\" '+jobDir+'/config.cfg'
   os.system(command)
   command = 'sed -i \"s%^tableY .*$%tableY '+str(args.tableY)+'%\" '+jobDir+'/config.cfg'
   os.system(command)
   command = 'sed -i \"s%^sensorConf .*$%sensorConf '+str(args.sensorconf)+'%\" '+jobDir+'/config.cfg'
   os.system(command)
   command = 'sed -i \"s%^digiConf .*$%digiConf '+str(args.digiconf)+'%\" '+jobDir+'/config.cfg'
   os.system(command)
   command = 'sed -i \"s%^NINOthr .*$%NINOthr '+str(args.thr)+'%\" '+jobDir+'/config.cfg'
   os.system(command)
   command = 'sed -i \"s%^Vbias1 .*$%Vbias1 '+str(args.vbias1)+'%\" '+jobDir+'/config.cfg'
   os.system(command)
   command = 'sed -i \"s%^Vbias2 .*$%Vbias2 '+str(args.vbias2)+'%\" '+jobDir+'/config.cfg'
   os.system(command)
   command = 'sed -i \"s%^VbiasCal .*$%VbiasCal '+str(args.vbiascal)+'%\" '+jobDir+'/config.cfg'
   os.system(command)
   command = 'sed -i \"s%^VbiasMat .*$%VbiasMat '+str(args.vbiasmat)+'%\" '+jobDir+'/config.cfg'
   os.system(command)


   
##### creates jobs #######
with open('job_%s.sh'%(str(args.run)), 'w') as fout:
   fout.write("#!/bin/sh\n")
   fout.write("echo\n")
   fout.write("echo 'START---------------'\n")
   fout.write("echo 'current dir: ' ${PWD}\n")
   fout.write("cd "+str(args.baseFolder)+"\n")
   fout.write("echo 'current dir: ' ${PWD}\n")
   fout.write("cd /afs/cern.ch/work/m/malberti/MTD/CMSSW_10_2_5/src/\n")
   fout.write("eval `scramv1 runtime -csh`\n")
   fout.write("cd -\n")
   fout.write("source scripts/setup.csh\n")
   fout.write("source scripts/setup.sh\n")
   fout.write(args.exeName+" "+jobDir+"/config.cfg "+args.run+"\n")
   fout.write("echo 'STOP---------------'\n")
   fout.write("echo\n")
   fout.write("echo\n")
   os.system("chmod 755 job_%s.sh"%(str(args.run)))

###### sends bjobs ######
if args.submit:
   command = "bsub -q "+args.queue+" -cwd "+jobDir+" job_%s.sh"%(str(args.run))
   print command
   os.system(command)
   print "job run" + str(args.run) + " submitted"
os.chdir("../..")
   
#print
#print "your jobs:"
#os.system("bjobs")
#print
#print 'END'
#print
