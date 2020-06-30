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


thresholds = [100, 50, 300]
f = {}
for thr in thresholds:
#filename = '../v11/output_3bars_Vbias%s_thr%sADC_xyangle%s_runs6872-6913.root'%(Vbias, thr, angle)
    filename = '../v11/yzAngleScan/output_3bars_Vbias72_thr%dADC_yzangle45.root'%thr
    f[thr] = ROOT.TFile.Open(filename)


channels = ['BAR%s'%i for i in range(0,3)]

colors = {'BAR0' : ROOT.kRed,
          'BAR1' : ROOT.kGreen+1,
          'BAR2' : ROOT.kBlue
}

p_tL_vs_amp = {}
p_tR_vs_amp = {}

p_ratioR_channel    = {}
p_ratioL_channel    = {}

p_ratioR_threshold   = {}
p_ratioL_threshold   = {}

for thr in thresholds:
    p_tL_vs_amp[thr] = {}
    p_tR_vs_amp[thr] = {}
    for ich,ch in enumerate(channels):
        p_tL_vs_amp[thr][ch] = f[thr].Get('p_tL_vs_amp_%s'%ch)
        p_tR_vs_amp[thr][ch] = f[thr].Get('p_tR_vs_amp_%s'%ch)
        
        p_tL_vs_amp[thr][ch].SetMarkerSize(0.5)
        p_tR_vs_amp[thr][ch].SetMarkerSize(0.5)

        p_tL_vs_amp[thr][ch].SetMarkerStyle(20)
        p_tR_vs_amp[thr][ch].SetMarkerStyle(24)
        
        p_tL_vs_amp[thr][ch].SetMarkerColor(colors[ch])
        p_tR_vs_amp[thr][ch].SetMarkerColor(colors[ch])
        
        p_tL_vs_amp[thr][ch].SetLineColor(colors[ch])
        p_tR_vs_amp[thr][ch].SetLineColor(colors[ch])

        (p_tL_vs_amp[thr][ch].GetFunction('fitFuncL_ampCorr_%d'%ich)).SetLineColor(colors[ch])
        (p_tR_vs_amp[thr][ch].GetFunction('fitFuncR_ampCorr_%d'%ich)).SetLineColor(colors[ch])



# ratios
for ch in channels:
    thr = 100
    href = p_tR_vs_amp[thr]['BAR0'].ProjectionX()

    p_ratioR_channel[ch] = p_tR_vs_amp[thr][ch].Clone()
    p_ratioR_channel[ch] = p_tR_vs_amp[thr][ch].ProjectionX()
    #p_ratioR_channel[ch].Divide(href)
    p_ratioR_channel[ch].Add(href,-1)

    p_ratioL_channel[ch] = p_tL_vs_amp[thr][ch].Clone()
    p_ratioL_channel[ch] = p_tL_vs_amp[thr][ch].ProjectionX()
    #p_ratioL_channel[ch].Divide(href)
    p_ratioL_channel[ch].Add(href,-1)


ch = 'BAR1'
thref = 100
for thr in thresholds:
    href = p_tR_vs_amp[thref][ch].ProjectionX()
    p_ratioR_threshold[thr] = p_tR_vs_amp[thr][ch].ProjectionX()
    #p_ratioR_threshold[thr].Divide(href)
    p_ratioR_threshold[thr].Add(href,-1) 

    p_ratioL_threshold[thr] = p_tL_vs_amp[thr][ch].ProjectionX()
    #p_ratioL_threshold[thr].Divide(href) 
    p_ratioL_threshold[thr].Add(href,-1) 

    
leg1 = ROOT.TLegend(0.45,0.65,0.89, 0.89)
leg1.SetBorderSize(0)

leg2 = ROOT.TLegend(0.45,0.65,0.89, 0.89)
leg2.SetBorderSize(0)

canvas1 = ROOT.TCanvas('ampCorr_vs_channel','ampCorr_vs_channel')
hdummy1 = ROOT.TH2F('hdummy1','',100,0,0.5, 100, -12, -9)
hdummy1.Draw()
for ch in channels:
    p_tL_vs_amp[100][ch].Draw('same')
    p_tR_vs_amp[100][ch].Draw('same')
    leg1.AddEntry( p_tL_vs_amp[100][ch],'%s'%ch,'PL')
leg1.Draw()


canvas11 = ROOT.TCanvas('ratio_ampCorr_vs_channel','ratio_ampCorr_vs_channel')
hdummy11 = ROOT.TH2F('hdummy11','',100,0,0.5, 100, -0.500, 0.500)
hdummy11.Draw()
for ch in channels:
    p_ratioL_channel[ch].SetMarkerStyle(20)
    p_ratioL_channel[ch].SetMarkerSize(0.5)
    p_ratioL_channel[ch].SetMarkerColor(colors[ch])
    p_ratioL_channel[ch].SetLineColor(colors[ch])

    p_ratioR_channel[ch].SetMarkerStyle(24)
    p_ratioR_channel[ch].SetMarkerSize(0.5)
    p_ratioR_channel[ch].SetMarkerColor(colors[ch])
    p_ratioR_channel[ch].SetLineColor(colors[ch])
    
    p_ratioR_channel[ch].Draw('same')
    p_ratioL_channel[ch].Draw('same')
leg1.Draw()

    
canvas2 = ROOT.TCanvas('ampCorr_vs_threshold','ampCorr_vs_threshold')
hdummy2 = ROOT.TH2F('hdummy1','',100,0,0.5, 100, -12, -9)
hdummy2.Draw()
ch = 'BAR1'
for i,thr in enumerate(thresholds):
    p_tL_vs_amp[thr][ch].SetMarkerColor(colors[ch]+i)
    p_tR_vs_amp[thr][ch].SetMarkerColor(colors[ch]+i)
    p_tL_vs_amp[thr][ch].SetLineColor(colors[ch]+i)
    p_tR_vs_amp[thr][ch].SetLineColor(colors[ch]+i)

    (p_tL_vs_amp[thr][ch].GetFunction('fitFuncL_ampCorr_1')).SetLineColor(colors[ch]+i)
    (p_tR_vs_amp[thr][ch].GetFunction('fitFuncR_ampCorr_1')).SetLineColor(colors[ch]+i)

    p_tL_vs_amp[thr][ch].Draw('same')
    p_tR_vs_amp[thr][ch].Draw('same')

    leg2.AddEntry(p_tL_vs_amp[thr][ch],'thr = %d'%thr, 'PL')
leg2.Draw()


canvas22 = ROOT.TCanvas('ratio_ampCorr_vs_threshold','ratio_ampCorr_vs_threshold')
hdummy22 = ROOT.TH2F('hdummy22','',100,0,0.5, 100, -0.500, 0.500)
hdummy22.Draw()
for i,thr in enumerate(thresholds):
    p_ratioL_threshold[thr].SetMarkerStyle(20)
    p_ratioL_threshold[thr].SetMarkerSize(0.5)
    p_ratioL_threshold[thr].SetMarkerColor(colors[ch]+i)
    p_ratioL_threshold[thr].SetLineColor(colors[ch]+i)

    p_ratioR_threshold[thr].SetMarkerStyle(24)
    p_ratioR_threshold[thr].SetMarkerSize(0.5)
    p_ratioR_threshold[thr].SetMarkerColor(colors[ch]+i)
    p_ratioR_threshold[thr].SetLineColor(colors[ch]+i)
    
    p_ratioR_threshold[thr].Draw('same')
    p_ratioL_threshold[thr].Draw('same')
leg2.Draw()
    
raw_input('ok?')
