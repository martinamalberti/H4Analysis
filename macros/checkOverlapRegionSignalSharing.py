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

outdir = sys.argv[1]

#inputdir = '../v7/yzAngleScan/signalInBothBars/'
inputdir = '../v7/yzAngleScan/'
thresholds = [30, 50, 70, 100, 150, 200, 250, 300, 350, 400, 450, 500, 600, 700, 800, 1000]
#thresholds =  [50, 100, 500, 1000]

kAdcToV = 1000./4096
resolMCP = 0.

bars = ['BAR0', 'BAR1']
#bars = ['BAR2', 'BAR1']

# find bins for 50% overlap and no overlap
ff = ROOT.TFile.Open('%s/output_3bars_Vbias72_thr100ADC_yzangle45.root'%(inputdir))
gg = ff.Get('g_tResolGausAveSum_ampCorr_posCorr_vs_posY')
binY = -1
#binY = 12
if ('BAR2' in bars):
    binY = min(range(len(gg.GetX())), key=lambda i: abs(gg.GetX()[i]-23.4))
if ('BAR0' in bars):
    binY = min(range(len(gg.GetX())), key=lambda i: abs(gg.GetX()[i]-25.8))

#binYd = {'BAR2': 6, 'BAR1': 16, 'BAR0': 26}
binYd = {'BAR2': min(range(len(gg.GetX())), key=lambda i: abs(gg.GetX()[i]-22.0)),
         'BAR1': min(range(len(gg.GetX())), key=lambda i: abs(gg.GetX()[i]-24.6)),
         'BAR0': min(range(len(gg.GetX())), key=lambda i: abs(gg.GetX()[i]-27.0))}
                     
ff.Close()

print binY, binYd['BAR0'], binYd['BAR1'], binYd['BAR2']
    
tChType = ROOT.TLatex( 0.13, 0.92, 'LYSO:Ce 3x3x57 mm^{3} - HPK S12572-015')
tChType.SetNDC()
tChType.SetTextSize(0.035)

# 50%-50% overlapping region
h_tDiff = {} #[BAR][THR]
h_tAve  = {} #[BAR][THR]
h_tAveSum = {} 
h_tAveDiff = {} 
h_tDiffDiff_1hit = {} 
h_tDiffDiff_2hit = {} 

g_tDiff = {}
g_tAve = {}
g_tAveSum = ROOT.TGraphErrors()
g_tAveDiff = ROOT.TGraphErrors()
g_tDiffDiff_1hit = ROOT.TGraphErrors()
g_tDiffDiff_2hit = ROOT.TGraphErrors()


# non overlapping region (diagonal)
h_tDiff_d = {} #[BAR][THR]
h_tAve_d  = {} #[BAR][THR]
g_tDiff_d = {}
g_tAve_d= {}


for bar in bars:
    h_tDiff[bar] = {}
    h_tAve[bar] = {}
    g_tDiff[bar] = ROOT.TGraphErrors()
    g_tAve[bar] = ROOT.TGraphErrors()

    h_tDiff_d[bar] = {}
    h_tAve_d[bar] = {}
    g_tDiff_d[bar] = ROOT.TGraphErrors()
    g_tAve_d[bar] = ROOT.TGraphErrors()
     
f = {}

for i, thr in enumerate(thresholds):
    filename = '%s/output_3bars_Vbias72_thr%sADC_yzangle45.root'%(inputdir, thr)
    f[thr] = ROOT.TFile.Open(filename)

    # single bar
    for bar in bars:
        # tdiff
        h_tDiff[bar][thr] = f[thr].Get('yBins/h_tDiff_ampCorr_posCorr_binY_%d_%s'%(binY,bar))
        funDiff = h_tDiff[bar][thr].GetFunction('fitFunDiff_ampCorr_posCorr_binY%d_%s'%(binY,bar))
        sigma_tDiff = funDiff.GetParameter(2)*1000.
        g_tDiff[bar].SetPoint(i, thr*kAdcToV, 0.5*sigma_tDiff)
        g_tDiff[bar].SetPointError(i, 0, 0.5*funDiff.GetParError(2)*1000.)

        # tAve
        h_tAve[bar][thr] = f[thr].Get('yBins/h_tAve_ampCorr_posCorr_binY_%d_%s'%(binY,bar))
        funAve = h_tAve[bar][thr].GetFunction('fitFunAve_ampCorr_posCorr_binY%d_%s'%(binY,bar))
        sigma_tAve  = funAve.GetParameter(2)*1000.
        sigma_tAve = math.sqrt(sigma_tAve*sigma_tAve - resolMCP*resolMCP)
        g_tAve[bar].SetPoint(i, thr*kAdcToV, sigma_tAve )
        g_tAve[bar].SetPointError(i, 0, funAve.GetParError(2)*1000.)

        h_tDiff_d[bar][thr] = f[thr].Get('yBins/h_tDiff_ampCorr_posCorr_binY_%d_%s'%(binYd[bar],bar))
        funDiff_d = h_tDiff_d[bar][thr].GetFunction('fitFunDiff_ampCorr_posCorr_binY%d_%s'%(binYd[bar],bar))
        sigma_tDiff_d = funDiff_d.GetParameter(2)*1000.
        g_tDiff_d[bar].SetPoint(i, thr*kAdcToV, 0.5*sigma_tDiff_d)
        g_tDiff_d[bar].SetPointError(i, 0, 0.5*funDiff_d.GetParError(2)*1000.)
        
        h_tAve_d[bar][thr] = f[thr].Get('yBins/h_tAve_ampCorr_posCorr_binY_%d_%s'%(binYd[bar],bar))
        funAve_d = h_tAve_d[bar][thr].GetFunction('fitFunAve_ampCorr_posCorr_binY%d_%s'%(binYd[bar],bar))
        sigma_tAve_d  = funAve_d.GetParameter(2)*1000.
        sigma_tAve_d = math.sqrt(sigma_tAve_d*sigma_tAve_d - resolMCP*resolMCP)
        g_tAve_d[bar].SetPoint(i, thr*kAdcToV, sigma_tAve_d)
        g_tAve_d[bar].SetPointError(i, 0, funAve_d.GetParError(2)*1000.)
        
    # compare two bars
    # difference between bar1 and bar 2 average
    h_tAveDiff[thr] = f[thr].Get('yBins/h_tAveDiff_ampCorr_posCorr_binY_%d'%(binY))
    funDiff = h_tAveDiff[thr].GetFunction('fitFunAveDiff_ampCorr_posCorr_binY%d'%(binY))
    sigma_tAveDiff = funDiff.GetParameter(2)*1000.
    g_tAveDiff.SetPoint(i, thr*kAdcToV, sigma_tAveDiff/2) 
    g_tAveDiff.SetPointError(i, 0, funDiff.GetParError(2)*1000./2)

    # difference of tDiff 
    h_tDiffDiff_2hit[thr] = f[thr].Get('yBins/h_tDiffDiff_ampCorr_posCorr_2hit_binY_%d'%(binY))
    funDiff = h_tDiffDiff_2hit[thr].GetFunction('fitFunDiffDiff_ampCorr_posCorr_2hit_binY%d'%(binY))
    sigma_tDiffDiff = funDiff.GetParameter(2)*1000.
    g_tDiffDiff_2hit.SetPoint(i, thr*kAdcToV, sigma_tDiffDiff/4) 
    g_tDiffDiff_2hit.SetPointError(i, 0, funDiff.GetParError(2)*1000./4)
    
    h_tDiffDiff_1hit[thr] = f[thr].Get('yBins/h_tDiffDiff_ampCorr_posCorr_1hit_binY_%d'%(binY))
    funDiff = h_tDiffDiff_1hit[thr].GetFunction('fitFunDiffDiff_ampCorr_posCorr_1hit_binY%d'%(binY))
    sigma_tDiffDiff = funDiff.GetParameter(2)*1000.
    g_tDiffDiff_1hit.SetPoint(i, thr*kAdcToV, sigma_tDiffDiff/2) 
    g_tDiffDiff_1hit.SetPointError(i, 0, funDiff.GetParError(2)*1000./2) 

    # average between bar1 and bar2
    h_tAveSum[thr] = f[thr].Get('yBins/h_tAveSum_ampCorr_posCorr_binY_%d'%(binY))
    funAve = h_tAveSum[thr].GetFunction('fitFunAveSum_ampCorr_posCorr_binY%d'%(binY))
    sigma_tAveSum  = funAve.GetParameter(2)*1000.
    sigma_tAveSum = math.sqrt(sigma_tAveSum*sigma_tAveSum - resolMCP*resolMCP)
    g_tAveSum.SetPoint(i, thr*kAdcToV, sigma_tAveSum) 
    g_tAveSum.SetPointError(i, 0, funAve.GetParError(2)*1000.) 

leg = ROOT.TLegend(0.15,0.8,0.45,0.89)
leg.SetBorderSize(0)

canvas = {}
hdummy = {}
for ibar, bar in enumerate(bars):
    canvas[bar] = ROOT.TCanvas('time_resolution_vs_LEthreshold_overlapping_%s'%bar,'time_resolution_vs_LEthreshold_overlapping_%s'%bar, 700, 600)
    canvas[bar].SetGridx()
    canvas[bar].SetGridy()
    hdummy[bar] = ROOT.TH2F('hdummy%s'%bar,'hdummy',100, 0, thresholds[-1]*kAdcToV+10., 100, 10, 100)
    hdummy[bar].GetXaxis().SetTitle('LE threshold (mV)')
    hdummy[bar].GetYaxis().SetTitle('time resolution (ps)')
    hdummy[bar].Draw()
    g_tAve[bar].SetMarkerStyle(24)
    g_tDiff[bar].SetMarkerStyle(20)
    g_tAve[bar].SetMarkerColor(ROOT.kOrange+ibar)
    g_tAve[bar].SetLineColor(ROOT.kOrange+ibar)
    g_tDiff[bar].SetMarkerColor(ROOT.kOrange+ibar)
    g_tDiff[bar].SetLineColor(ROOT.kOrange+ibar)
    g_tDiff[bar].Draw('plsame')
    g_tAve[bar].Draw('plsame')
    if (ibar == 0):
        leg.AddEntry(g_tDiff[bar], 't_{Diff}/2', 'PL')
        leg.AddEntry(g_tAve[bar], 't_{Average}', 'PL')
    leg.Draw('same')
    tChType.Draw()
    canvas[bar].SaveAs(outdir+'/'+canvas[bar].GetName()+'.pdf')
    canvas[bar].SaveAs(outdir+'/'+canvas[bar].GetName()+'.png')


canvas2 = {}
hdummy2 = {}
for ibar, bar in enumerate(bars):
    canvas2[bar] = ROOT.TCanvas('time_resolution_vs_LEthreshold_notoverlapping_%s'%bar,'time_resolution_vs_LEthreshold_notoverlapping_%s'%bar, 700, 600)
    canvas2[bar].SetGridx()
    canvas2[bar].SetGridy()
    hdummy2[bar] = ROOT.TH2F('hdummy2%s'%bar,'hdummy',100, 0, thresholds[-1]*kAdcToV+10., 100, 10, 100)
    hdummy2[bar].GetXaxis().SetTitle('LE threshold (mV)')
    hdummy2[bar].GetYaxis().SetTitle('time resolution (ps)')
    hdummy2[bar].Draw()
    g_tAve_d[bar].SetMarkerStyle(24)
    g_tDiff_d[bar].SetMarkerStyle(20)
    g_tAve_d[bar].SetMarkerColor(ROOT.kGray+1+ibar)
    g_tAve_d[bar].SetLineColor(ROOT.kGray+1+ibar)
    g_tDiff_d[bar].SetMarkerColor(ROOT.kGray+1+ibar)
    g_tDiff_d[bar].SetLineColor(ROOT.kGray+1+ibar)
    g_tDiff_d[bar].Draw('plsame')
    g_tAve_d[bar].Draw('plsame')
    leg.Draw('same')
    tChType.Draw()
    canvas2[bar].SaveAs(outdir+'/'+canvas2[bar].GetName()+'.pdf')
    canvas2[bar].SaveAs(outdir+'/'+canvas2[bar].GetName()+'.png')

canvas3 =  ROOT.TCanvas('time_resolution_vs_LEthreshold_barsAverage','time_resolution_vs_LEthreshold_barsAverage', 700, 600)
canvas3.SetGridx()
canvas3.SetGridy()
hdummy3 = ROOT.TH2F('hdummy','hdummy',100, 0, thresholds[-1]*kAdcToV+10., 100, 10, 100)
hdummy3.GetXaxis().SetTitle('LE threshold (mV)')
hdummy3.GetYaxis().SetTitle('time resolution (ps)')
hdummy3.Draw()
g_tAveSum.SetMarkerStyle(24)
g_tAveDiff.SetMarkerStyle(20)
g_tAveSum.SetMarkerColor(ROOT.kRed)
g_tAveDiff.SetMarkerColor(ROOT.kRed)
g_tAveSum.SetLineColor(ROOT.kRed)
g_tAveDiff.SetLineColor(ROOT.kRed)
g_tAveSum.Draw('plsame')
g_tAveDiff.Draw('plsame')
leg3 = ROOT.TLegend(0.15,0.8,0.45,0.89)
leg3.SetBorderSize(0)
leg3.AddEntry(g_tAveDiff, '(t_{Ave1}-t_{Ave2})/2', 'PL')
leg3.AddEntry(g_tAveSum, '(t_{Ave1}+t_{Ave2})/2', 'PL')
leg3.Draw('same')
tChType.Draw()
canvas3.SaveAs(outdir+'/'+canvas3.GetName()+'.pdf')
canvas3.SaveAs(outdir+'/'+canvas3.GetName()+'.png')
raw_input('ok?')


legAll = ROOT.TLegend(0.15,0.7,0.45,0.89)
legAll.SetBorderSize(0)
canvasAll =  ROOT.TCanvas('time_resolution_overlap_vs_LEthreshold','time_resolution_overlap_vs_LEthreshold', 700, 600)
canvasAll.SetGridx()
canvasAll.SetGridy()
hdummy3.Draw()
g_tDiffDiff_2hit.Draw('plsame')
g_tDiffDiff_2hit.SetMarkerStyle(30)
g_tDiffDiff_1hit.Draw('plsame')
g_tDiffDiff_1hit.SetMarkerStyle(31)
legAll.AddEntry(g_tAveDiff, '#sigma((Diff_{%s}-tDiff_{%s})/2)'%(bars[0], bars[1]), 'PL')
for ibar, bar in enumerate(bars):
    g_tDiff[bar].Draw('plsame')
    legAll.AddEntry(g_tDiff[bar], '%s - overlapping region'%bars[ibar], 'PL')
for ibar, bar in enumerate(bars):
    g_tDiff_d[bar].Draw('plsame')
    legAll.AddEntry(g_tDiff_d[bar], '%s - not overlapping region'%(bars[ibar]), 'PL')
legAll.Draw('same')
tChType.Draw()
canvasAll.SaveAs(outdir+'/'+canvasAll.GetName()+'.pdf')
canvasAll.SaveAs(outdir+'/'+canvasAll.GetName()+'.png')
raw_input('ok?')
