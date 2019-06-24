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

outdir = sys.argv[1]


angles = [90,60, 53, 45, 37, 30, 20 , 14]

col = {90 : ROOT.kBlack,
       60 : ROOT.kRed,
       53 : ROOT.kOrange+1,
       45 : ROOT.kOrange,
       37 : ROOT.kGreen,
       30 : ROOT.kCyan,
       20 : ROOT.kBlue,
       14 : ROOT.kViolet,
}

f = {}
h = {}


leg = ROOT. TLegend(0.8, 0.6, 0.89, 0.89)
leg.SetBorderSize(0)

canvas = ROOT.TCanvas('c_mip','c_mip')
canvas.SetLogy()
hdummy = ROOT.TH2F('hdummy','', 100, 0, 1, 100, 0.0005, 1)
hdummy.GetXaxis().SetTitle('amplitude (V)')
hdummy.GetYaxis().SetTitle('a.u.')
hdummy.Draw()
for angle in angles:
    fname = '../v4/output_3bars_Vbias72_thr200ADC_xyangle%s.root'%angle
    if (angle == 90):
        fname = '../v4/output_3bars_Vbias72_thr200ADC_xyangle%s_runs7639-7679.root'%angle
    f[angle] = ROOT.TFile.Open(fname)
    h[angle] = f[angle].Get('h_ampR_nocuts_BAR1')
    h[angle].SetLineColor(col[angle])
    h[angle].SetLineWidth(2)
    h[angle].DrawNormalized('same')
    leg.AddEntry(h[angle],'#theta = %d^{#circ}'%angle, 'L')

leg.Draw()
tChType = ROOT.TLatex( 0.13, 0.92, 'LYSO:Ce 3x3x57 mm^{3} - HPK S12572-015')
tChType.SetNDC()
tChType.SetTextSize(0.035)
tChType.Draw()

raw_input('ok?')
