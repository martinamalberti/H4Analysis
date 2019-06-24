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



channels = ['BAR%s'%i for i in range(0,1)]

f = {}
gAve = {}
gDiff = {}
gMCP = {}

for ch in channels:
    fname = '../v4/output_1bar_Vbias43_thr1000ADC_4mm_binnedAmpWalkCorr.root'
    #fname = '../v4/output_1bar_Vbias43_thr500ADC_2mm.root'
    f[ch] = ROOT.TFile.Open(fname)
    gAve[ch] = f[ch].Get('g_tResolGausAve_ampCorr_posCorr_%s'%ch)
    gDiff[ch] = f[ch].Get('g_tResolGausDiff_ampCorr_posCorr_%s'%ch)
    gMCP[ch] = ROOT.TGraphErrors()
    
tChType = ROOT.TLatex( 0.13, 0.92, 'LYSO:Ce 4x3x57 mm^{3} - HPK S12572-015')
tChType.SetNDC()
tChType.SetTextSize(0.035)



for ch in channels:
    xAve = ROOT.Double(0)
    sigmaAve = ROOT.Double(0)
    xDiff = ROOT.Double(0)
    sigmaDiff = ROOT.Double(0)

    exAve = ROOT.Double(0)
    exDiff = ROOT.Double(0)
    esigmaAve = ROOT.Double(0)
    esigmaDiff = ROOT.Double(0)
    
    for i in range(0, gAve[ch].GetN()):
        gAve[ch].GetPoint(i, xAve, sigmaAve)
        exAve = gAve[ch].GetErrorX(i)
        esigmaAve = gAve[ch].GetErrorY(i)
        gDiff[ch].GetPoint(i, xDiff, sigmaDiff)
        exDiff = gDiff[ch].GetErrorX(i)
        esigmaDiff = gDiff[ch].GetErrorY(i)
        if(xAve != xDiff): continue
        sigmaMCP = 0
        esigmaMCP = 0
        #if (sigmaAve > sigmaDiff/2):
        sigmaMCP = math.sqrt(sigmaAve*sigmaAve+14*14 - sigmaDiff*sigmaDiff/4)
        esigmaMCP = math.sqrt(esigmaAve*esigmaAve + (esigmaDiff*esigmaDiff/4))
        gMCP[ch].SetPoint(i, xAve, sigmaMCP)
        gMCP[ch].SetPointError(i, exAve, esigmaMCP)
        gDiff[ch].SetPoint(i, xAve, sigmaDiff/2)

    cc = ROOT.TCanvas('time_resol')
    cc.SetGridy()
    hdummy = ROOT.TH2F('hdummy','hdummy', 100, -10, 35, 100, 0, 70)
    hdummy.Draw()
    gAve[ch].SetMarkerColor(ROOT.kBlack)
    gAve[ch].SetLineColor(ROOT.kBlack)
    gDiff[ch].SetMarkerColor(ROOT.kGray)
    gDiff[ch].SetLineColor(ROOT.kGray)
    gMCP[ch].SetMarkerColor(ROOT.kMagenta)
    gMCP[ch].SetLineColor(ROOT.kMagenta)

    
    gAve[ch].Draw('psame')
    gDiff[ch].Draw('psame')
    gMCP[ch].Draw('psame')
    


    raw_input('ok?')
