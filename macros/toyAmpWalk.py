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




fname = '../v11/materialLeveling/output_1bar_Vbias43_thr400ADC_4mm.root'
f = ROOT.TFile.Open(fname)

h_beamXY = f.Get('h_beamXY')
h_beamXY_px = h_beamXY.ProjectionX()

p = f.Get('p_tR_vs_amp_binXc_1_BAR0')
func = p.GetFunction('fitFuncR_ampCorr_1_0')

p_bin = {}
for ibin in range(0,4):
    p_bin[ibin] = ROOT.TProfile('p_bin_%d'%ibin,'p_bin_%d'%ibin, 100, 0, 1)
    p_bin[ibin].SetMarkerColor(ibin+2)

p_all = ROOT.TProfile('p_all','p_all', 100, 0, 1)
p_all.SetMarkerSize(0.5)

h_amp = ROOT.TH1F('h_amp','h_amp',200,0,0.5)

gRandom = ROOT.TRandom()
xmin = -5
xmax = 35
N = 100000
for n in range(0,N):
    x = gRandom.Uniform(xmin,xmax)
    #x = h_beamXY_px.GetRandom()
    dt   = -7.5 * x
    time = dt + gRandom.Gaus(0, 25)
    ampx  = 0.06 + 2.847*0.00001*x + 3.445*0.00001*x*x
    #ampx  = 0.06 * (1 + 0.002*x)
    amp   = gRandom.Landau(ampx, 0.005)
    xbin = 0
    if ( x >  5 and x < 15): xbin = 1
    if ( x > 15 and x < 25): xbin = 2
    if ( x > 25 and x < 35): xbin = 3    
    p_bin[xbin].Fill(amp, time)
    p_all.Fill(amp, time)
    if (xbin == 3): h_amp.Fill(amp)


c1 = ROOT.TCanvas('c1','c1')
h_amp.Draw()

c2 = ROOT.TCanvas('c2','c2')
hdummy = ROOT.TH2F('hdummy','',1000,0,0.5,1000, -500, 500)
hdummy.Draw()
for ibin in range(0,4):
    p_bin[ibin].Draw('same')
p_all.Draw('same')

raw_input('OK?')
