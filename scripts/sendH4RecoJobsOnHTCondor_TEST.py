#!/usr/bin/env python
import os, re
import commands
import math, time
import sys
import argparse
import subprocess

parser = argparse.ArgumentParser(description='This script splits H4Reco tasks in multiple jobs and sends them on HTCondor')

parser.add_argument("-j",  "--job",            required=True,     type=int,   help="job number")
parser.add_argument("-n",  "--n",              required=True,     type=int,   help="runs per job")
parser.add_argument("-s",  "--submit",                                        help="submit jobs", action='store_true')
parser.add_argument("-v",  "--verbose",                                       help="increase output verbosity", action='store_true')

args = parser.parse_args()

currDir = os.getcwd()

jobDir = currDir+'/jobs_condor_test/'
os.chdir(jobDir)

with open('runlist_job%s.txt'%str(args.job),'w') as flist:
   for run in range(args.job,args.job+args.n):
      flist.write('%s\n'%str(run))

##### creates HTCondor scripts #######
with open('htcondor_job_%s.sub'%(str(args.job)), 'w') as fout:
   fout.write("executable            = %s/run$(rr)/job_$(rr).sh\n"%jobDir)
   fout.write("arguments             = $(ClusterId) $(ProcId)\n")
   fout.write("output                = %s/run$(rr)/output/job_$(rr).$(ClusterId).$(ProcId).out\n"%jobDir)
   fout.write("error                 = %s/run$(rr)/error/job_$(rr).$(ClusterId).$(ProcId).err\n"%jobDir)
   fout.write("log                   = %s/run$(rr)/log/job_$(rr).$(ClusterId).log\n"%jobDir)
   fout.write("queue rr from runlist_job%s.txt\n"%str(args.job))

###### sends HTCondor jobs ######
if args.submit:
   command = "condor_submit "+jobDir+"htcondor_job_%s.sub"%(str(args.job))
   print command
   os.system(command)
   print "HTCondor job for run" + str(args.job) + " submitted"
os.chdir("../..")
   
#print
#print "your jobs:"
#os.system("bjobs")
#print
#print 'END'
#print
