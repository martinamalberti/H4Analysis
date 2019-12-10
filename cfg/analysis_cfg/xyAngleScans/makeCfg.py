#!/usr/bin/env python
import os, re
import commands
import math, time
import sys
import argparse
import subprocess

Vbias = [72]
thresholds = [20, 30, 50, 60, 70, 80, 90, 100, 150, 200, 250, 300, 350, 400, 450, 500, 550, 600, 650, 700, 750, 800, 850, 900, 950, 1000, 1500, 2000, 2500, 3000]
#angles = [90, 60, 53, 45, 37, 30, 20, 14]
angles = [14]

for vb in Vbias:
    for angle in angles:
        for threshold in thresholds:
            command1 = 'cp analysis_3bars_Vbias%s_thr100_xyangle%s.cfg analysis_3bars_Vbias%s_thr%s_xyangle%s.cfg'%(vb, angle, vb, threshold, angle) 
            command2 = 'sed -i -- "s/100/%s/g" analysis_3bars_Vbias%s_thr%s_xyangle%s.cfg'%(threshold, vb, threshold,angle)
            #print command1
            #print command2
            os.system(command1)
            os.system(command2)

print 'Done!'
