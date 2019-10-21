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

inputdir = '../v7/yzAngleScan/'

Vbias = sys.argv[1]
angle = sys.argv[2]
outdir = sys.argv[3]


# LE thresholds
thresholds = [50, 100, 200, 700]
color = { 70  : ROOT.kOrange-1,
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

gn = {}
g = {}
g0 = {}
g1 = {}
g2 = {}
f = {}
leg = ROOT.TLegend(0.15,0.7,0.45,0.89)
leg.SetBorderSize(0)


# sharing region
canvas0 = ROOT.TCanvas('bars_time_resolution_vs_LEthreshold_sharingRegion','', 700, 600)
canvas0.SetGridx()
canvas0.SetGridy()
hdummy0 = ROOT.TH2F('hdummy0','hdummy0',100, 19, 30, 100, 10, 140)
hdummy0.GetXaxis().SetTitle('y (mm)')
hdummy0.GetYaxis().SetTitle('time resolution (ps)')
hdummy0.GetXaxis().SetRangeUser(21.5, 28)
hdummy0.Draw()
for i, thr in enumerate(thresholds):
    filename = '%s/output_3bars_Vbias%s_thr%sADC_yzangle%s.root'%(inputdir,Vbias, thr, angle)
    f[thr] = ROOT.TFile.Open(filename)
    g[thr] = f[thr].Get('g_tResolGausDiffDiff_ampCorr_posCorr_vs_posY_overlap')
    g[thr].SetMarkerColor(color[thr])
    g[thr].SetLineColor(color[thr])
    leg.AddEntry(g[thr],'thr = %.0f mV'%(thr*kAdcToV),'PL')
    g[thr].Draw('psame')
leg.Draw('same')
tChType.Draw()


# fraction of events above threshold
canvas00 = ROOT.TCanvas('bars_fraction_events_above_threshold_vs_LEthreshold_sharingRegion','', 700, 600)
canvas00.SetGridx()
canvas00.SetGridy()
hdummy00 = ROOT.TH2F('hdummy3','hdummy3',100, 19, 30, 100, 0, 1.2)
hdummy00.GetXaxis().SetTitle('y (mm)')
hdummy00.GetYaxis().SetTitle('fraction of events with 2 hits')
hdummy00.GetXaxis().SetRangeUser(21.5, 28)
hdummy00.Draw()
for i, thr in enumerate(thresholds):
    gn[thr] = f[thr].Get('g_nEventsRatio_vs_posY')
    gn[thr].SetMarkerColor(color[thr])
    gn[thr].SetLineColor(color[thr])
    gn[thr].Draw('pcsame')
leg.Draw('same')
tChType.Draw()


# single bars
canvas1 = ROOT.TCanvas('bars_time_resolution_vs_LEthreshold_singleBars','', 700, 600)
canvas1.SetGridx()
canvas1.SetGridy()
hdummy1 = ROOT.TH2F('hdummy1','hdummy1',100, 19, 30, 100, 10, 140)
hdummy1.GetXaxis().SetTitle('y (mm)')
hdummy1.GetYaxis().SetTitle('time resolution (ps)')
hdummy1.Draw()
for i, thr in enumerate(thresholds):
    g0[thr] = f[thr].Get('g_tResolGausDiff_ampCorr_posCorr_vs_posY_BAR0')
    g1[thr] = f[thr].Get('g_tResolGausDiff_ampCorr_posCorr_vs_posY_BAR1')
    g2[thr] = f[thr].Get('g_tResolGausDiff_ampCorr_posCorr_vs_posY_BAR2')
    for i, gr in  enumerate([g0[thr], g1[thr], g2[thr]]):
        gr.SetMarkerColor(color[thr])
        gr.SetLineColor(color[thr])
        gr.SetLineColor(color[thr])
        gr.SetMarkerStyle(20+i)
        gr.SetMarkerSize(0.7)
    g2[thr].Draw('plsame')
    g1[thr].Draw('plsame')
    g0[thr].Draw('plsame')
leg.Draw('same')
tChType.Draw()



# MIP thresholds
MIPthresholds= [0.15, 0.25, 0.30, 0.40, 0.50]

leg2 = ROOT.TLegend(0.15,0.7,0.45,0.89)
leg2.SetBorderSize(0)

canvas2 = ROOT.TCanvas('bars_signalSharing_time_resolution_vs_MIPthreshold','', 700, 600)
canvas2.SetGridx()
canvas2.SetGridy()
hdummy2 = ROOT.TH2F('hdummy2','hdummy2',100, 19, 30, 100, 10, 140)
hdummy2.GetXaxis().SetTitle('y (mm)')
hdummy2.GetYaxis().SetTitle('time resolution (ps)')
hdummy2.GetXaxis().SetRangeUser(21.5, 28)
hdummy2.Draw()
for i, thr in enumerate(MIPthresholds):
    strthr = '%.02f'%thr
    filename = '%s/output_3bars_Vbias%s_thr100ADC_yzangle%s_%sMIP.root'%(inputdir,Vbias, angle, strthr.replace('.',''))
    print filename
    ff = ROOT.TFile.Open(filename)
    g[thr] = ff.Get('g_tResolGausDiffDiff_ampCorr_posCorr_vs_posY_overlap')
    g[thr].SetMarkerColor(ROOT.kRed+i)
    g[thr].SetLineColor(ROOT.kRed+i)
    g[thr].Draw('psame')
    leg2.AddEntry(g[thr], 'MIP threshold = %.02f'%thr,'PL')
leg2.Draw()
tChType.Draw()
  
canvas3 = ROOT.TCanvas('bars_fraction_events_above_threshold_vs_MIPthreshold_sharingRegion','', 700, 600)
canvas3.SetGridx()
canvas3.SetGridy()
hdummy3 = ROOT.TH2F('hdummy3','hdummy3',100, 19, 30, 100, 0, 1.2)
hdummy3.GetXaxis().SetTitle('y (mm)')
hdummy3.GetYaxis().SetTitle('fraction of events with 2 hits')
hdummy3.GetXaxis().SetRangeUser(21.5, 28)
hdummy3.Draw()
for i, thr in enumerate(MIPthresholds):
    strthr = '%.02f'%thr
    filename = '%s/output_3bars_Vbias%s_thr100ADC_yzangle%s_%sMIP.root'%(inputdir,Vbias, angle, strthr.replace('.',''))
    fff = ROOT.TFile.Open(filename)
    g[thr] = fff.Get('g_nEventsRatio_vs_posY')
    g[thr].SetMarkerColor(ROOT.kRed+i)
    g[thr].SetLineColor(ROOT.kRed+i)
    g[thr].Draw('pcsame')
leg2.Draw()
tChType.Draw()

for can in canvas00, canvas0, canvas1, canvas2, canvas3:
    can.SaveAs(outdir+'/'+can.GetName()+'.png')
    can.SaveAs(outdir+'/'+can.GetName()+'.pdf')
    
#raw_input('ok?')


