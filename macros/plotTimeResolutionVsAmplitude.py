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
ROOT.gStyle.SetOptFit(1)
ROOT.gStyle.SetOptStat(0)
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

#filename = '../v10/yzAngleScan/mcpCorr/output_3bars_Vbias%s_thr%sADC_yzangle%s.root'%(Vbias, thr, angle)
filename = '../v11/yzAngleScan/output_3bars_Vbias%s_thr%sADC_yzangle%s.root'%(Vbias, thr, angle)
f = ROOT.TFile.Open(filename)
print filename

tChType = ROOT.TLatex( 0.13, 0.92, 'LYSO:Ce 3x3x57 mm^{3} - HPK S12572-015')
tChType.SetNDC()
tChType.SetTextSize(0.035)

channels = ['BAR%s'%i for i in range(1,2)]

color = { 'BAR0' : ROOT.kBlue,
          'BAR1' : ROOT.kGreen+1,
          'BAR2' : ROOT.kRed}

markerstyle = { 'BAR0' : 20,
                'BAR1' : 20,
                'BAR2' : 20}


g = {}
g['BAR0'] = f.Get('g_tResolGausDiff_pulseIntCorr_posCorr_vs_amp_BAR0')
g['BAR1'] = f.Get('g_tResolGausDiff_pulseIntCorr_posCorr_vs_amp_BAR1')
g['BAR2'] = f.Get('g_tResolGausDiff_pulseIntCorr_posCorr_vs_amp_BAR2')

g1 = {}
for i in channels:
    g1[i] = g[i].Clone()
    
fun = {}
fun1 = {}
ROOT.gStyle.SetFitFormat('4.3g')
for i in channels:
    fun[i] = ROOT.TF1('fun_BAR%s'%i,'sqrt([0]*[0]/(pow(x,[1])*pow(x,[1])) + [2]*[2])', 0, 10)
    fun[i].SetParNames('a','#alpha', 'c')
    fun[i].SetParameters(10, 1, 10)
    fun[i].SetRange(0.12, 2.0)
    fun[i].SetLineColor(color[i])
    g[i].Fit(fun[i],'QRS+')
    fun1[i] = ROOT.TF1('fun1_BAR%s'%i,'sqrt([0]*[0]/(x*x) + [1]*[1]/x + [2]*[2])', 0, 10)
    fun1[i].SetParNames('n','s', 'c')
    fun1[i].SetParameters(10, 10, 10)
    fun1[i].SetParLimits(2, 0, 1000)
    fun1[i].SetRange(0.12, 2.0)
    fun1[i].SetLineColor(color[i]+1)
    fun1[i].SetLineStyle(2)
    g1[i].Fit(fun1[i],'QRS+')

for i in channels:
    for gr in g[i], g1[i]:
        gr.SetMarkerStyle(markerstyle[i])
        gr.SetMarkerSize(0.8)
        gr.SetMarkerColor(color[i])
        gr.SetLineColor(color[i])

leg = ROOT.TLegend(0.15,0.80,0.45,0.89)
leg.SetBorderSize(0)
leg.AddEntry(g['BAR0'],'top bar','PL')
leg.AddEntry(g['BAR1'],'middle bar','PL')
leg.AddEntry(g['BAR2'],'bottom bar','PL')

# absolute time resolution
canvas = ROOT.TCanvas('bars_time_resolution_vs_amplitude_thr%sADC'%thr,'bars_time_resolution_vs_amplitude_thr%sADC'%thr,700,600)
canvas.SetGridy()
canvas.SetLogy()
tChType.Draw()
hdummy = ROOT.TH2F('','', 100, 0.1, 2.1, 100, 10, 500)
hdummy.GetYaxis().SetTitle('time resolution (ps)')
hdummy.GetXaxis().SetTitle('amplitude/MPV')
hdummy.Draw()
for i in channels:
    g[i].Draw('psames')
    g1[i].Draw('psames')

    #leg.Draw('same')
    canvas.Update()
    stat = g[i].FindObject("stats");
    stat1 = g1[i].FindObject("stats");
    stat.SetLineColor(color[i]);
    stat1.SetLineColor(color[i]+1);
    height = stat1.GetY2NDC() - stat1.GetY1NDC();
    stat1.SetY1NDC(stat.GetY1NDC() - height - 0.01);
    stat1.SetY2NDC(stat.GetY1NDC() - 0.01 );
    stat1.Draw();
tChType.Draw()

canvas.SaveAs(subfolder+'/'+canvas.GetName()+'.png')
canvas.SaveAs(subfolder+'/'+canvas.GetName()+'.pdf')
raw_input('ok?')



