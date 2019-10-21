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
thr = sys.argv[2]
angle = sys.argv[3]
subfolder = sys.argv[4]

filename = '../v7/yzAngleScan/output_3bars_Vbias%s_thr%sADC_yzangle%s.root'%(Vbias, thr, angle)
f = ROOT.TFile.Open(filename)


filenameRef = '../v7/output_3bars_Vbias%s_thr%sADC_xyangle90_runs7639-7679.root'%(Vbias, thr)
fRef = ROOT.TFile.Open(filenameRef)



tChType = ROOT.TLatex( 0.13, 0.92, 'LYSO:Ce 3x3x57 mm^{3} - HPK S12572-015')
tChType.SetNDC()
tChType.SetTextSize(0.035)

g = {}
g['BAR0'] = f.Get('g_tResolGausDiff_ampCorr_posCorr_vs_posY_BAR0')
g['BAR1'] = f.Get('g_tResolGausDiff_ampCorr_posCorr_vs_posY_BAR1')
g['BAR2'] = f.Get('g_tResolGausDiff_ampCorr_posCorr_vs_posY_BAR2')
g['COMB'] = f.Get('g_tResolGausAveDiff_ampCorr_posCorr_vs_posY')

gnorm = {}
gnorm['BAR0'] = ROOT.TGraphErrors()
gnorm['BAR1'] = ROOT.TGraphErrors()
gnorm['BAR2'] = ROOT.TGraphErrors()
gnorm['COMB'] = ROOT.TGraphErrors()

color = { 'BAR0' : ROOT.kAzure+10,
          'BAR1' : ROOT.kAzure+0,
          'BAR2' : ROOT.kAzure-4,
          'COMB' : ROOT.kGreen+2,
          'COMBW1' : ROOT.kOrange+1,
          'COMBW2' : ROOT.kRed}

markerstyle = { 'BAR0' : 21,
                'BAR1' : 21,
                'BAR2' : 21,
                'COMB' : 20,
                'COMBW1' : 20,
                'COMBW2' : 20}


for i in ['BAR0', 'BAR1', 'BAR2','COMB']:
    g[i].SetMarkerStyle(markerstyle[i])
    g[i].SetMarkerSize(1.0)
    g[i].SetMarkerColor(color[i])
    g[i].SetLineColor(color[i])
    if ('COMB' in i):  g[i].SetMarkerSize(0.9)
    #tDiff of the single bar /2
    if ('BAR' in i):
        for j in range(0, g[i].GetN()):
            g[i].GetY()[j] *= 0.5
            g[i].GetEY()[j] *= 0.5
    #tDiff between  two  bars
    if ('COMB' in i):
        for j in range(0, g[i].GetN()):
            g[i].GetY()[j] /=1
            g[i].GetEY()[j] /=1
    

leg = ROOT.TLegend(0.15,0.7,0.45,0.89)
leg.SetBorderSize(0)
leg.AddEntry(g['BAR0'],'top bar','PL')
leg.AddEntry(g['BAR1'],'middle bar','PL')
leg.AddEntry(g['BAR2'],'bottom bar','PL')
leg.AddEntry(g['COMB'],'unweighted diff','PL')

# absolute time resolution
canvas = ROOT.TCanvas('bars_phiSharing_time_resolution_Vbias%s_thr%s_yzangle%s'%(Vbias, thr, angle),'bars_phiSharing_time_resolution_Vbias%s_thr%s_yzangle%s'%(Vbias, thr, angle),800,600)
canvas.SetGridy()
hdummy = ROOT.TH2F('','', 100, 20, 29.2, 100, 0, 100)
hdummy.GetYaxis().SetNdivisions(520)
hdummy.GetYaxis().SetTitle('#sigma(tDiff) (ps)')
hdummy.GetXaxis().SetTitle('y (mm)')
hdummy.Draw()
g['BAR0'].Draw('plsame')
g['BAR1'].Draw('plsame')
g['BAR2'].Draw('plsame')
g['COMB'].Draw('plsame')

canvas.SaveAs(subfolder+'/'+canvas.GetName()+'.png')
canvas.SaveAs(subfolder+'/'+canvas.GetName()+'.pdf')



raw_input('ok?')



