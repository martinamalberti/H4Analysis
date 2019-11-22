#!/usr/bin/env python
import os, re
import commands
import math, time
import sys
import argparse
import subprocess

Vbias = [72]
thresholds = [20, 30, 50, 70, 100, 150, 200, 250, 300, 350, 400, 450, 500, 600, 700, 800, 900, 1000, 1500, 2000]

for vb in Vbias:
    for threshold in thresholds:
        command1 = 'cp analysis_3bars_Vbias%s_thr100_yzangle45.cfg analysis_3bars_Vbias%s_thr%s_yzangle45.cfg'%(vb, vb, threshold) 
        command2 = 'sed -i -- "s/100/%s/g" analysis_3bars_Vbias%s_thr%s_yzangle45.cfg'%(threshold, vb, threshold)
        #print command1
        #print command2
        os.system(command1)
        os.system(command2)

print 'Done!'
