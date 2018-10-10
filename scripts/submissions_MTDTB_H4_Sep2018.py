#!/usr/bin/python

import os
import csv
import sys

confNumber = sys.argv[1]

with open("/afs/cern.ch/user/m/malberti/public/runList_H4_Sep2018_digiConf"+confNumber+".txt", 'rb') as f:
    reader = csv.reader(f,delimiter='\t')
    
    for row in reader:
        h4run, tableX, tableY, sensorconf, digiconf, thr, vbias1, vbias2, vbiascal, vbiasmat = row
        command = "./sendH4RecoJobsOnLxplus.py -r " + str(h4run) + \
            " -b /afs/cern.ch/work/m/malberti/MTD/TBatH4Sept2018/H4Analysis/" + \
            " -e ./bin/H4Reco.exe" + \
            " -c cfg/MTDTB_H4_Sep2018/MTD_H4_Sep2018_digiConf"+confNumber+".cfg" + \
            " -q cmscaf1nw " + \
            " -p1 " + str(tableX) + \
            " -p2 " + str(tableY) + \
            " -p3 " + str(sensorconf) + \
            " -p4 " + str(digiconf) + \
            " -p5 " + str(thr) + \
            " -p6 " + str(vbias1) + \
            " -p7 " + str(vbias2) + \
            " -p8 " + str(vbiascal) + \
            " -p9 " + str(vbiasmat) + \
            " -s "
        print command
        #os.system(command)
