#!/usr/bin/env python
import os, re
import commands
import math, time
import sys
import argparse
import subprocess

thickness = [2,3,4]
Vbias = [43]
thresholds = [20, 30, 40, 50, 60, 70, 80, 90, 100, 150, 200, 250, 300, 350, 400, 450, 500, 550, 600, 650, 700, 750, 800, 850, 900, 950, 1000, 1500, 2000, 2500, 3000]


for vb in Vbias:
    for t in thickness:
        for threshold in thresholds:
            command1 = 'cp analysis_1bar_Vbias%s_thr1000_%dmm.cfg analysis_1bar_Vbias%s_thr%s_%dmm.cfg'%(vb, t, vb, threshold, t)
            command2 = 'sed -i -- "s/1000/%s/g" analysis_1bar_Vbias%s_thr%s_%dmm.cfg'%(threshold, vb, threshold, t)
            #print command1
            #print command2
            os.system(command1)
            os.system(command2)

print 'Done!'
