#!/usr/bin/python

import os
import csv
import sys

#confNumber = sys.argv[1]
confNumber = '2'


with open("/afs/cern.ch/work/m/malberti/MTD/TBatFNALNov2018/H4Analysis/runList_FNAL_Nov2018_digiConf"+confNumber+".txt", 'rb') as f:
    reader = csv.reader(f,delimiter='\t')
    
    for row in reader:
        h4run, digiconf, sensorconf, vbias1, vbias2, thr = row
        command = "./sendH4RecoJobsOnHTCondor.py -r " + str(h4run) + \
            " -b /afs/cern.ch/work/m/malberti/MTD/TBatFNALNov2018/H4Analysis/" + \
            " -e ./bin/H4Reco" + \
            " -c cfg/MTDTB_FNAL_Nov2018/MTDTB_FNAL_Nov2018_digiConf"+confNumber+".cfg" + \
            " -p1 " + str(digiconf) + \
            " -p2 " + str(sensorconf) + \
            " -p3 " + str(vbias1) + \
            " -p4 " + str(vbias2) + \
            " -p5 " + str(thr) + \
            " -s "
        print command
        os.system(command)
