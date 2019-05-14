#!/usr/bin/python

import os
import csv
import sys
import math


# runs to be recosntructed
runmin = 6293
runmax = 10747
goodrunsfile = '/afs/cern.ch/user/a/abenagli/public/good_runs.txt' 
with open(goodrunsfile) as f:
  goodrunslist = [ int(i) for i in f ]

nRunsPerJob = 100

jobmin = int(math.floor(runmin/nRunsPerJob) * nRunsPerJob)
jobmax = int(math.floor(runmax/nRunsPerJob) * nRunsPerJob) + nRunsPerJob

print jobmin, jobmax

#prepare jobs for each run
for run in range(runmin, (runmax+1)):
        if run in goodrunslist:
                command = './prepareJobs.py -r ' + str(run) + \
                          ' -b /afs/cern.ch/work/m/malberti/MTD/TBatFNALApril2019/H4Analysis/' + \
                          ' -e ./bin/H4Reco' + \
                          ' -c cfg/MTDTB_FNAL_Apr2019/MTDTB_FNAL_Apr2019.cfg' 
                print command
                os.system(command)
                

# prepare condor job on multiple files
for job in range(jobmin, jobmax, nRunsPerJob):
        print job
        command = './sendH4RecoJobsOnHTCondor.py -j ' + str(job) + \
                  ' -n ' + str(nRunsPerJob) + \
                  ' -r1 ' + str(runmin) + \
                  ' -r2 ' + str(runmax) + \
                  ' -l ' + goodrunsfile + \
                  ' -s ' 
        print command
        os.system(command)
        
