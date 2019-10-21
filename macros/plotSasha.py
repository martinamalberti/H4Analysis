#! /usr/bin/env python
import os
import shutil
import glob
import math
import array
import sys
import time


import ROOT
import CMS_lumi, tdrstyle

#set the tdr style
tdrstyle.setTDRStyle()
ROOT.gStyle.SetOptStat(0)
ROOT.gStyle.SetOptFit(0)
ROOT.gStyle.SetOptTitle(0)
ROOT.gStyle.SetLabelSize(0.04,'X')
ROOT.gStyle.SetLabelSize(0.04,'Y')
ROOT.gStyle.SetTitleSize(0.04,'X')
ROOT.gStyle.SetTitleSize(0.04,'Y')
ROOT.gStyle.SetTitleOffset(1.1,'X')
ROOT.gStyle.SetTitleOffset(1.2,'Y')



angles = [90, 60, 53, 45, 37, 30, 20, 14]
resol = [27, 26, 25.2, 23.9, 21.8, 21.7, 17.5, 15.3]

g = ROOT.TGraphErrors()

for i, angle in enumerate(angles):
    slantTickness = 3. / math.sin((angle) * math.pi / 180.)
    g.SetPoint(i, slantTickness, resol[i])
    g.SetPointError(i, 0, 1.)
    

c = ROOT.TCanvas('c','c')

fitAve = ROOT.TF1('fitAve','[1]*1./pow(x,[0])',0,100)
g.Fit(fitAve,'')
g.Draw('ap')

raw_input('ok?')
    

