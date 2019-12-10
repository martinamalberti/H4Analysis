#!/usr/bin/env python
import os, re
import commands
import math, time
import sys
import argparse
import subprocess

Vbias = [72, 71, 70, 69, 68]
#Vbias = [43, 42, 41, 40, 39]
thresholds = [20, 30, 50, 60, 70, 80, 80, 100, 150, 200, 250, 300, 350, 400, 450, 500, 550, 600, 650, 700, 750, 800, 850, 900, 950,1000, 1500, 2000]

for vb in Vbias:
    for threshold in thresholds:
        command1 = 'cp analysis_3bars_Vbias%s_thr1000_xyangle90.cfg analysis_3bars_Vbias%s_thr%s_xyangle90.cfg'%(vb, vb, threshold)
        command2 = 'sed -i -- "s/1000/%s/g" analysis_3bars_Vbias%s_thr%s_xyangle90.cfg'%(threshold, vb, threshold)
        #command1 = 'cp analysis_1bar_Vbias%s_thr1000_4mm.cfg analysis_1bar_Vbias%s_thr%s_4mm.cfg'%(vb, vb, threshold)
        #command2 = 'sed -i -- "s/1000/%s/g" analysis_1bar_Vbias%s_thr%s_4mm.cfg'%(threshold, vb, threshold)
        os.system(command1)
        os.system(command2)

print 'Done!'
