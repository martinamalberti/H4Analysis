#!/usr/bin/python

import os
import csv
import sys
import math

runmin = 6780
runmax = 6908

nRunsPerJob = 10

print nRunsPerJob

jobmin = int(math.floor(runmin/nRunsPerJob) * nRunsPerJob)
jobmax = int(math.floor(runmax/nRunsPerJob) * nRunsPerJob) + nRunsPerJob

print jobmin, jobmax

#prepare jobs for each run
for run in range(runmin, (runmax+1)):
        command = "./prepareJobs.py -r " + str(run) + \
                  " -b /afs/cern.ch/work/m/malberti/MTD/TBatFNALApril2019/H4Analysis/" + \
                  " -e ./bin/H4Reco" + \
                  " -c cfg/MTDTB_FNAL_Apr2019/MTDTB_FNAL_Apr2019_TEST.cfg" 
        print command
        os.system(command)
                

# prepare condor job on multiple files
for job in range(jobmin, jobmax, nRunsPerJob):
        print job
        command = "./sendH4RecoJobsOnHTCondor_TEST.py -j " + str(job) + \
                  " -n " + str(nRunsPerJob) + \
                  " -s " 
        print command
        os.system(command)
        
