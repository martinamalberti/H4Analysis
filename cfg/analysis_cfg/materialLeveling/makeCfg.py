#!/usr/bin/env python
import os, re
import commands
import math, time
import sys
import argparse
import subprocess

t = 2
Vbias = [43]
thresholds = [20, 50, 100, 150, 200, 250, 300, 350, 400, 450, 500, 600, 700, 800, 900, 1000, 1500, 2000]


for vb in Vbias:
    for threshold in thresholds:
        command1 = 'cp analysis_1bar_Vbias%s_thr1000_%dmm.cfg analysis_1bar_Vbias%s_thr%s_%dmm.cfg'%(vb, t, vb, threshold, t)
        command2 = 'sed -i -- "s/1000/%s/g" analysis_1bar_Vbias%s_thr%s_%dmm.cfg'%(threshold, vb, threshold, t)
        #print command1
        #print command2
        os.system(command1)
        os.system(command2)

print 'Done!'
