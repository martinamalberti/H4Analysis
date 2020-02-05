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
ROOT.gStyle.SetTitleOffset(1.05,'Y')
ROOT.gErrorIgnoreLevel = ROOT.kWarning;


sipms = ['HDR2', 'FBK']
OVs = [0, 0.5, 1, 1.5, 2, 2.5, 3, 3.5, 4, 4.5, 5, 5.5, 6]

# from: https://indico.cern.ch/event/766237/contributions/3180787/attachments/1758277/2851590/SiPMs_for_MTD-after-2.1E12n_SN-dVB_calculations-for-Marco-v2.pdf#search=musienko%20AND%20cerntaxonomy%3A"Indico%2FExperiments%2FCMS%20meetings%2FMTD%20-%20MIP%20Timing%20Detector%2FSensors"
f = {}
f['HDR2'] = ROOT.TF1("fHDR2"," 0.4198*pow(x,3) - 4.8388 * pow(x,2) + 21.277*x + 3.6737", 0, 10)
f['FBK']  = ROOT.TF1("fFBK","-5.684E-14*pow(x,3) -9.055E-01*pow(x,2) + 1.148E+01*x -9.960E-01",0, 10);

g = {}

for i, s in enumerate(sipms):
    print s
    g[s] = ROOT.TGraph()
    g[s].SetLineColor(i+1)
    for j, ov in enumerate(OVs):
        pde = f[s].Eval(ov)
        g[s].SetPoint(j, ov, pde)
        print '%.01f   %.01f'%(ov, pde)

canvas = ROOT.TCanvas('PDE_vs_OV','PDE_vs_OV')
canvas.SetGridx()
canvas.SetGridy()
hdummy = ROOT.TH2F('hdummy','hdummy',100, 0., 7., 100, 0, 60)
hdummy.GetXaxis().SetTitle('OV (V)')
hdummy.GetYaxis().SetTitle('PDE (%)')
hdummy.Draw()

g['HDR2'].Draw("lsame")
g['FBK'].Draw("lsame")

raw_input("ok?")
