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
parser.add_argument("-s",  "--submit",                                        help="submit jobs", action='store_true')
parser.add_argument("-v",  "--verbose",                                       help="increase output verbosity", action='store_true')
parser.add_argument("-p1", "--digiconf",       required=True,     type=str,   help="")
parser.add_argument("-p2", "--sensorconf",     required=True,     type=str,   help="")
parser.add_argument("-p3", "--vbias1",         required=True,     type=float, help="")
parser.add_argument("-p4", "--vbias2",         required=True,     type=float, help="")
parser.add_argument("-p5", "--thr",            required=True,     type=float, help="")

args = parser.parse_args()



#print 
#print 'START'
#print 

currDir = os.getcwd()

#print

##### creates directory and file list for job #######
#jobDir = currDir+'/jobs/run'+args.run+'/'
jobDir = currDir+'/jobs_condor/run'+args.run+'/'
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
   command = 'sed -i \"s%^originValues .*$%originValues '+str(args.run)+'%\" '+jobDir+'/config.cfg'
   os.system(command)
   command = 'sed -i \"s%^sensorConf .*$%sensorConf '+str(args.sensorconf)+'%\" '+jobDir+'/config.cfg'
   os.system(command)
   command = 'sed -i \"s%^digiConf .*$%digiConf '+str(args.digiconf)+'%\" '+jobDir+'/config.cfg'
   os.system(command)
   command = 'sed -i \"s%^NINOthr .*$%NINOthr '+str(args.thr)+'%\" '+jobDir+'/config.cfg'
   os.system(command)
   command = 'sed -i \"s%^biasNINOfront .*$%biasNINOfront '+str(args.vbias1)+'%\" '+jobDir+'/config.cfg'
   os.system(command)
   command = 'sed -i \"s%^biasNINOrear .*$%biasNINOrear '+str(args.vbias2)+'%\" '+jobDir+'/config.cfg'
   os.system(command)


   
##### creates jobs scripts #######
with open('job_%s.sh'%(str(args.run)), 'w') as fout:
   fout.write("#!/bin/tcsh\n")
   fout.write("echo\n")
   fout.write("echo 'START---------------'\n")
   fout.write("echo 'current dir: ' ${PWD}\n")
   fout.write("cd "+str(args.baseFolder)+"\n")
   fout.write("echo 'current dir: ' ${PWD}\n")
   fout.write("cd /afs/cern.ch/work/m/malberti/MTD/CMSSW_10_2_5/src/\n")
   fout.write("eval `scramv1 runtime -csh`\n")
   fout.write("cd -\n")
   fout.write("source scripts/setup.csh\n")
   fout.write(args.exeName+" "+jobDir+"/config.cfg "+args.run+"\n")
   fout.write("echo 'STOP---------------'\n")
   fout.write("echo\n")
   fout.write("echo\n")
   os.system("chmod 755 job_%s.sh"%(str(args.run)))


##### creates HTCondor scripts #######
with open('htcondor_job_%s.sub'%(str(args.run)), 'w') as fout:
   fout.write("executable            = %s/job_%s.sh\n"%(jobDir,str(args.run)))
   fout.write("arguments             = $(ClusterId) $(ProcId)\n")
   fout.write("output                = %s/output/job_%s.$(ClusterId).$(ProcId).out\n"%(jobDir,str(args.run)) )
   fout.write("error                 = %s/error/job_%s.$(ClusterId).$(ProcId).err\n"%(jobDir,str(args.run)) )
   fout.write("log                   = %s/log/job_%s.$(ClusterId).log\n"%(jobDir,str(args.run)) )
   fout.write("queue \n")
                                                

###### sends HTCondor jobs ######
if args.submit:
   command = "condor_submit "+jobDir+"htcondor_job_%s.sub"%(str(args.run))
   print command
   os.system(command)
   print "HTCondor job for run" + str(args.run) + " submitted"
os.chdir("../..")
   
#print
#print "your jobs:"
#os.system("bjobs")
#print
#print 'END'
#print