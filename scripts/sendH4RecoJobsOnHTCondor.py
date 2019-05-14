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
parser.add_argument("-r1",  "--runmin",        required=True,     type=int,   help="run min")
parser.add_argument("-r2",  "--runmax",        required=True,     type=int,   help="run max")
parser.add_argument("-l",  "--l",           required=True,     type=str,   help="list of good runs")
parser.add_argument("-s",  "--submit",                                        help="submit jobs", action='store_true')
parser.add_argument("-v",  "--verbose",                                       help="increase output verbosity", action='store_true')

args = parser.parse_args()


with open(args.l) as f:
  goodrunslist = [ int(i) for i in f ]
  
currDir = os.getcwd()
  
jobDir = currDir+'/jobs_condor/'
os.chdir(jobDir)


with open('runlist_job%s.txt'%str(args.job),'w') as flist:
  for run in range(args.job,args.job+args.n):
    if (run >= args.runmin and run <= args.runmax and run in goodrunslist):
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
  print "HTCondor job for job" + str(args.job) + " submitted"
os.chdir("../..")
  
#print
#print "your jobs:"
#os.system("bjobs")
#print
#print 'END'
#print
