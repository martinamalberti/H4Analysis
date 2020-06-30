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
ROOT.gStyle.SetPadRightMargin(1.2)
ROOT.gErrorIgnoreLevel = ROOT.kWarning;

Vbias = 72
thr = 100
angle = 45
channelType = 'LYSO:Ce 3x3x57 mm^{3} - HPK S12572-015 3x3 mm^{2}'
ampCorrType = 'amp'

filename = '../../v11/yzAngleScan/output_3bars_Vbias%d_thr%dADC_yzangle%d.root'%(Vbias, thr, angle)
f = ROOT.TFile.Open(filename)

# get histograms
p_tL_vs_amp = f.Get('p_tL_vs_%s_BAR1'%ampCorrType)
h2_tL_vs_amp = f.Get('h2_tL_vs_%s_BAR1'%ampCorrType)
h_ampSum = f.Get('h_%sSum_nocuts'%ampCorrType)

# redefine x axis in units of MPV, shift y axis to be centered in deltaT = 0
h_ampSum.GetXaxis().SetRangeUser(0.05, 1)
if (ampCorrType == 'pulseInt'): h_ampSum.GetXaxis().SetRangeUser(5, 100)
binmax = h_ampSum.GetMaximumBin()
fLandau = ROOT.TF1('fLandau','landau',0,100)
fLandau.SetRange(0.85*h_ampSum.GetBinCenter(binmax), 2.0*h_ampSum.GetBinCenter(binmax))
fLandau.SetParameter(1,h_ampSum.GetBinCenter(binmax))
h_ampSum.Fit('fLandau','QR')
mpv = fLandau.GetParameter(1)
nbinsx = h2_tL_vs_amp.GetNbinsX()
nbinsy = h2_tL_vs_amp.GetNbinsY()
xmin = p_tL_vs_amp.GetXaxis().GetXmin()/mpv
xmax = p_tL_vs_amp.GetXaxis().GetXmax()/mpv
ymean = h2_tL_vs_amp.GetMean(2)
ymin = -20 - ymean
ymax = 20 - ymean

hnew = ROOT.TH1F('hnew_tL_vs_amp_BAR1','',nbinsx,xmin, xmax)
h2new = ROOT.TH2F('h2new_tL_vs_amp_BAR1','',nbinsx,xmin, xmax, nbinsy, ymin, ymax)

for ibin in range(1, nbinsx):
    cont = p_tL_vs_amp.GetBinContent(ibin) - ymean
    err  = p_tL_vs_amp.GetBinError(ibin)
    hnew.SetBinContent(ibin, cont)
    hnew.SetBinError(ibin, err)
    for jbin in range(1, nbinsy):
        cont2 = h2_tL_vs_amp.GetBinContent(ibin,jbin)
        err2  = h2_tL_vs_amp.GetBinError(ibin,jbin)
        h2new.SetBinContent(ibin, jbin,cont2)
        h2new.SetBinError(ibin, jbin, err2)

# refit shifted distribution
fitFunc = p_tL_vs_amp.GetFunction('fitFuncL_%sCorr_1'%ampCorrType)
hnew.Fit(fitFunc)


#draw
canvas = ROOT.TCanvas('tL_vs_amp_BAR1','tL_vs_amp_BAR1')
h2new.GetXaxis().SetTitle('amplitude/MPV')
h2new.GetYaxis().SetTitle('t_{left} - t_{MCP} (ns)')
h2new.GetXaxis().SetRangeUser(0,3.5)
h2new.GetYaxis().SetRangeUser(-0.5,0.7)
h2new.GetXaxis().SetLabelSize(0.045)
h2new.GetYaxis().SetLabelSize(0.045)
h2new.GetZaxis().SetLabelSize(0.045)
h2new.Draw('colz')
hnew.Draw('same')
tChType = ROOT.TLatex( 0.13, 0.92, channelType)
tChType.SetNDC()
tChType.SetTextSize(0.030)
tChType.Draw()

ROOT.gPad.Update()

canvas.SaveAs(canvas.GetName()+'.png')
canvas.SaveAs(canvas.GetName()+'.pdf')
canvas.SaveAs(canvas.GetName()+'.C')

#raw_input('ok?')
