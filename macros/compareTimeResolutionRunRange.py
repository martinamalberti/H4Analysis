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
#set the tdr style
tdrstyle.setTDRStyle()
ROOT.gStyle.SetOptStat(0)
ROOT.gStyle.SetOptFit(0)



channels = ['BAR%s'%i for i in range(0,3)]

runranges =  ['6369-6401',
              '6872-6913',
              '7498-7516',
              '7639-7679',
              '7850-7877']


f = {}
ampRef = {}

for r in runranges:
    f[r] = ROOT.TFile.Open('../v4/output_3bars_Vbias72_thr250ADC_xyangle90_runs%s.root'%(r))
    
tChType = ROOT.TLatex( 0.13, 0.92, 'LYSO:Ce 3x3x50 mm^{3} - HPK S12572-015')
tChType.SetNDC()
tChType.SetTextSize(0.035)


gDiff = {}
gAve = {}

tResAve = {}
tResDiff = {}

gAve_vs_runrange = {}
gDiff_vs_runrange = {}

sigmaPTK = 0.014 # ps

for ch in channels:
    gAve[ch] = {}
    gDiff[ch] = {}
    tResAve[ch] = {}
    tResDiff[ch] = {}
    gAve_vs_runrange[ch] = ROOT.TGraphErrors()
    gDiff_vs_runrange[ch] = ROOT.TGraphErrors()
    for i,r in enumerate(runranges):
        # tAve
        h = f[r].Get('h_tAve_ampCorr_posCorr_'+ ch)
        fitfun = h.GetFunction('fitFunAve_ampCorr_posCorr_%s'%ch)
        if (fitfun == None):
            continue
        sigmaGaus  = fitfun.GetParameter(2)
        esigmaGaus = fitfun.GetParError(2)*1000
        tResAve[ch][r] = ROOT.TMath.Sqrt(sigmaGaus*sigmaGaus - sigmaPTK*sigmaPTK)*1000. # ps

        # tDiff
        h = f[r].Get('h_tDiff_ampCorr_posCorr_'+ ch)
        fitfun = h.GetFunction('fitFunDiff_ampCorr_posCorr_%s'%ch)
        if (fitfun == None):
            continue
        sigmaGaus  = fitfun.GetParameter(2)
        esigmaGaus = math.sqrt(fitfun.GetParError(2)*fitfun.GetParError(2)+0.002*0.002)*1000
        tResDiff[ch][r] = sigmaGaus/2*1000. # ps

        print ch, r, tResAve[ch][r], tResDiff[ch][r], esigmaGaus
        
        # graphs vs run range
        gAve_vs_runrange[ch].SetPoint(i, i, tResAve[ch][r])
        gDiff_vs_runrange[ch].SetPoint(i, i, tResDiff[ch][r])

        gAve_vs_runrange[ch].SetPointError(i, 0, esigmaGaus)
        gDiff_vs_runrange[ch].SetPointError(i, 0, esigmaGaus  )

        gAve_vs_runrange[ch].GetHistogram().GetXaxis().SetBinLabel(i+1, r)
        gDiff_vs_runrange[ch].GetHistogram().GetXaxis().SetBinLabel(i+1, r)


for ch in channels:
    gAve_vs_runrange[ch].SetMarkerColor(1)
    gAve_vs_runrange[ch].SetLineColor(1)
    gDiff_vs_runrange[ch].SetMarkerColor(1)
    gDiff_vs_runrange[ch].SetLineColor(1)
    gDiff_vs_runrange[ch].SetMarkerStyle(24)
        
leg = ROOT.TLegend(0.15,0.75,0.45,0.89)
leg.SetBorderSize(0)
leg.AddEntry(gAve_vs_runrange['BAR0'],'#sigma(t_{average} - t_{MCP})','PL')
leg.AddEntry(gDiff_vs_runrange['BAR0'],'#sigma(t_{right} - t_{left})/2','PL')

canvas = {}
hdummy = {}

for ch in channels:
    canvas[ch] = ROOT.TCanvas('timeResolution_vs_runrange_%s'%ch,'timeResolution_vs_runrange_%s'%ch)
    canvas[ch].SetGridy()
    hdummy[ch] = ROOT.TH2F('hdummy%s'%ch,'',5,-0.5,4.5,100,0,70)
    for ibin,r in enumerate(runranges):
        hdummy[ch].GetXaxis().SetBinLabel(ibin+1, r)
    hdummy[ch].Draw()
    hdummy[ch].GetXaxis().SetTitle('run range')
    hdummy[ch].GetYaxis().SetTitle('time resolution (ps)')
    hdummy[ch].GetYaxis().SetTitleOffset(1.0)
    gAve_vs_runrange[ch].Draw('psame')
    gDiff_vs_runrange[ch].Draw('psame')
    leg.Draw()

    for t in '.pdf','.png':
        canvas[ch].SaveAs(canvas[ch].GetName()+t)

    raw_input('ok?')

    
