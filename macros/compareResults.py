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

filename1 = '../v10/yzAngleScan/mcpCorr/output_3bars_Vbias72_thr100ADC_yzangle45.root'
filename2 = '../v10/yzAngleScan/mcpCorr_pulseIntAve/output_3bars_Vbias72_thr100ADC_yzangle45.root'


f1 = ROOT.TFile.Open(filename1)
f2 = ROOT.TFile.Open(filename2)


channels = ['BAR0', 'BAR1', 'BAR2']

tChType = ROOT.TLatex( 0.13, 0.92, 'LYSO:Ce 3x3x57 mm^{3} - HPK S12572-015')
tChType.SetNDC()
tChType.SetTextSize(0.035)

color = { 'BAR0' : ROOT.kBlue,
          'BAR1' : ROOT.kGreen,
          'BAR2' : ROOT.kRed}

markerstyle = { 'BAR0' : 21,
                'BAR1' : 21,
                'BAR2' : 21}


g1 = {}
g1['BAR0'] = f1.Get('g_tResolGausDiff_ampCorr_posCorr_vs_posY_BAR0')
g1['BAR1'] = f1.Get('g_tResolGausDiff_ampCorr_posCorr_vs_posY_BAR1')
g1['BAR2'] = f1.Get('g_tResolGausDiff_ampCorr_posCorr_vs_posY_BAR2')

g2 = {}
g2['BAR0'] = f1.Get('g_tResolGausDiff_pulseIntCorr_posCorr_vs_posY_BAR0')
g2['BAR1'] = f1.Get('g_tResolGausDiff_pulseIntCorr_posCorr_vs_posY_BAR1')
g2['BAR2'] = f1.Get('g_tResolGausDiff_pulseIntCorr_posCorr_vs_posY_BAR2')


g3 = {}
g3['BAR0'] = f2.Get('g_tResolGausDiff_pulseIntCorr_posCorr_vs_posY_BAR0')
g3['BAR1'] = f2.Get('g_tResolGausDiff_pulseIntCorr_posCorr_vs_posY_BAR1')
g3['BAR2'] = f2.Get('g_tResolGausDiff_pulseIntCorr_posCorr_vs_posY_BAR2')


for i, ch in enumerate(channels):
    for j,g in enumerate([g1[ch], g2[ch], g3[ch]]):
        g.SetMarkerStyle(24+j)
        g.SetMarkerSize(0.7)
        g.SetMarkerColor(color[ch]+j)
        g.SetLineColor(color[ch]+j)


leg = ROOT.TLegend(0.15,0.7,0.45,0.89)
leg.SetBorderSize(0)
leg.AddEntry(g1['BAR0'],'amplitude','PL')
leg.AddEntry(g2['BAR0'],'pulse integral','PL')
leg.AddEntry(g3['BAR0'],'average pulse integral','PL')

# absolute time resolution
canvas = ROOT.TCanvas('bars_ampWalkCorr_comparison','bars_ampWalkCorr_comparison',700,600)
canvas.SetGridy()
hdummy = ROOT.TH2F('','', 100, 20, 29.2, 100, 20, 50)
hdummy.GetYaxis().SetTitle('time resolution (ps)')
hdummy.GetXaxis().SetTitle('y (mm)')
hdummy.Draw()
for i in channels:
    g1[i].Draw('plsame')
    g2[i].Draw('plsame')
    g3[i].Draw('plsame')
leg.Draw('same')
tChType.Draw()

raw_input('ok?')
canvas.SaveAs(canvas.GetName()+'.png')
canvas.SaveAs(canvas.GetName()+'.pdf')
