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
ROOT.gStyle.SetOptStat(0)
ROOT.gStyle.SetOptFit(0)
ROOT.gStyle.SetOptTitle(0)
ROOT.gStyle.SetLabelSize(0.04,'X')
ROOT.gStyle.SetLabelSize(0.04,'Y')
ROOT.gStyle.SetTitleSize(0.04,'X')
ROOT.gStyle.SetTitleSize(0.04,'Y')
ROOT.gStyle.SetTitleOffset(1.1,'X')
ROOT.gStyle.SetTitleOffset(1.2,'Y')

Vbias = sys.argv[1]
angle = sys.argv[2]
outdir = sys.argv[3]


# LE thresholds
thresholds = [50, 100, 200, 500, 700]

color = { 30  : ROOT.kOrange-1,
          50  : ROOT.kOrange,
          100 : ROOT.kRed,
          150 : ROOT.kOrange+1,
          200 : ROOT.kGreen+1,
          250 : ROOT.kGreen,
          300 : ROOT.kCyan,
          350 : ROOT.kCyan+1,
          400 : ROOT.kCyan+2,
          500 : ROOT.kBlue,
          700 : ROOT.kMagenta,
          1000 : ROOT.kMagenta+1,
          2000 : ROOT.kMagenta+2}

tChType = ROOT.TLatex( 0.13, 0.92, 'LYSO:Ce 3x3x57 mm^{3} - HPK S12572-015')
tChType.SetNDC()
tChType.SetTextSize(0.035)

kAdcToV = 1000./4096

g = {}
f = {}
leg = ROOT.TLegend(0.15,0.7,0.45,0.89)
leg.SetBorderSize(0)

canvas = ROOT.TCanvas('bars_phiSharing_time_resolution_vs_LEthreshold','', 1000, 800)
canvas.SetGridx()
canvas.SetGridy()
hdummy = ROOT.TH2F('hdummy','hdummy',100, 19, 30, 100, 10, 140)
hdummy.GetXaxis().SetTitle('y (mm)')
hdummy.GetYaxis().SetTitle('time resolution (ps)')
hdummy.Draw()

for i, thr in enumerate(thresholds):
    filename = '../v4_old/output_3bars_Vbias%s_thr%sADC_yzangle%s.root'%(Vbias, thr, angle)
    f[thr] = ROOT.TFile.Open(filename)
    g[thr] = f[thr].Get('g_tResolGausResWSum_ampCorr_tDiffCorr_vs_posY')
    g[thr].SetMarkerColor(color[thr])
    g[thr].SetLineColor(color[thr])
    leg.AddEntry(g[thr],'thr = %.0f mV'%(thr*kAdcToV),'PL')
    g[thr].Draw('plsame')


leg.Draw('same')
tChType.Draw()



# MIP thresholds
MIPthresholds= [0.15, 0.25, 0.30, 0.40]
#MIPthresholds= [0.15, 0.25]

leg2 = ROOT.TLegend(0.15,0.7,0.45,0.89)
leg2.SetBorderSize(0)

canvas2 = ROOT.TCanvas('bars_phiSharing_time_resolution_vs_MIPthreshold','', 1000, 800)
canvas2.SetGridx()
canvas2.SetGridy()
hdummy2 = ROOT.TH2F('hdummy2','hdummy2',100, 19, 30, 100, 10, 140)
hdummy2.GetXaxis().SetTitle('y (mm)')
hdummy2.GetYaxis().SetTitle('time resolution (ps)')
hdummy2.Draw()
for i, thr in enumerate(MIPthresholds):
    strthr = '%.02f'%thr
    filename = '../v4_old/output_3bars_Vbias%s_thr100ADC_yzangle%s_%sMIP.root'%(Vbias, angle, strthr.replace('.',''))
    ff = ROOT.TFile.Open(filename)
    g[thr] = ff.Get('g_tResolGausResWSum_ampCorr_tDiffCorr_vs_posY')
    #g[thr] = ff.Get('g_tResolGausSum_ampCorr_tDiffCorr_vs_posY')
    g[thr].SetMarkerColor(1+i)
    g[thr].SetLineColor(1+i)
    g[thr].Draw('plsame')
    leg2.AddEntry(g[thr], 'MIP threshold = %.02f'%thr,'PL')
leg2.Draw()
tChType.Draw()
  
canvas3 = ROOT.TCanvas('bars_phiSharing_fraction_events_above_threshold_vs_MIPthreshold','', 1000, 800)
canvas3.SetGridx()
canvas3.SetGridy()
hdummy3 = ROOT.TH2F('hdummy3','hdummy3',100, 19, 30, 100, 0, 1.5)
hdummy3.GetXaxis().SetTitle('y (mm)')
hdummy3.GetYaxis().SetTitle('fraction of events above threshold')
hdummy3.GetXaxis().SetRangeUser(21.5, 28)
hdummy3.Draw()
for i, thr in enumerate(MIPthresholds):
    strthr = '%.02f'%thr
    filename = '../v4_old/output_3bars_Vbias%s_thr100ADC_yzangle%s_%sMIP.root'%(Vbias, angle, strthr.replace('.',''))
    fff = ROOT.TFile.Open(filename)
    g[thr] = fff.Get('g_nEventsRatio_vs_posY')
    g[thr].SetMarkerColor(1+i)
    g[thr].SetLineColor(1+i)
    g[thr].Draw('pcsame')
leg2.Draw()
tChType.Draw()

for can in canvas, canvas2, canvas3:
    can.SaveAs(outdir+'/'+can.GetName()+'.png')
    can.SaveAs(outdir+'/'+can.GetName()+'.pdf')
    
raw_input('ok?')


