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
ROOT.gErrorIgnoreLevel = ROOT.kWarning;

#filename = '../v7/output_3bars_Vbias72_thr100ADC_xyangle90_runs7639-7679.root
filename = '../v7/output_3bars_Vbias72_thr100ADC_xyangle60.root'
channels = ['BAR%s'%i for i in range(0,3)]

f = ROOT.TFile.Open(filename); 

p = {}
fitErf = {}


for ich, ch in enumerate(channels):
    p[ch]= f.Get('p_eff_vs_posY_%s'%ch)
    fitErf[ch] = ROOT.TF1('fitErf_%s'%ch,'[2]*TMath::Erf((x-[0])/[1])+[3]', 15, 30)
    fitErf[ch].SetLineColor(ich+1)
    bin0 = p[ch].FindFirstBinAbove(0.50)
    x0   = p[ch].GetBinCenter(bin0)
    fitErf[ch].SetParameters(x0, 0.05, 0.2, 0.1)
    fitErf[ch].SetRange(x0 - 2. , x0 + 2.)
    p[ch].Fit(fitErf[ch],'QR')
    sigmaPos = fitErf[ch].GetParameter(1)
    esigmaPos = fitErf[ch].GetParError(1)
    p[ch].SetLineColor(ich+1)
    p[ch].SetMarkerColor(ich+1)



c1 = ROOT.TCanvas('eff_vs_posY_3bars','eff_vs_posY_3bars')
c1.SetGridx()
c1.SetGridy()

hdummy = ROOT.TH2F('hdummy','',100, 19, 31, 100, 0., 1.1)
hdummy.GetXaxis().SetTitle('y (mm)')
hdummy.GetYaxis().SetTitle('efficiency')
hdummy.Draw()
for ich, ch in enumerate(channels):
    p[ch].Draw('lsame')
    
raw_input('ok?')
