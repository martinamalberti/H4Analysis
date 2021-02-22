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

outdir = sys.argv[1]
sipm = sys.argv[2]

channels = ['BAR%s'%i for i in range(0,1)]

tChType = ROOT.TLatex( 0.13, 0.92, 'LYSO:Ce 3x4x57 mm^{3} - FBK thin-epi 5x5 mm^{2} ')
tChType.SetNDC()
tChType.SetTextSize(0.035)

if (sipm == 'HPK'):
    channels = ['BAR%s'%i for i in range(1,2)]
    tChType = ROOT.TLatex( 0.13, 0.92, 'LYSO:Ce 3x3x57 mm^{3} - HPK 3x3 mm^{2}')

leg = ROOT.TLegend(0.65, 0.65, 0.89, 0.89)
leg.SetBorderSize(0)

f = {}
pR = {}
pL = {}

pRall = {}
pLall = {}

canvasL = {}
canvasR = {}

nbins = 4
corrType = 'amp'
#corrType = 'pulseInt'

xmin = 0.05
xmax = 0.30
ymin = -6.1
ymax = -5.3
if (sipm=='HPK'):
    ymin = -11.6
    ymax = -10.8
if (corrType == 'pulseInt'):
    xmin = 8.5
    xmax = 45
hdummy = ROOT.TH2F('hdummy','',100, xmin, xmax, 100, ymin, ymax)
hdummy.GetYaxis().SetTitle('time (ns)')
hdummy.GetXaxis().SetTitle('amplitude (V)')
if (corrType == 'pulseInt'):
    hdummy.GetXaxis().SetTitle('pulse integral (V ns)')

for ich, ch in enumerate(channels):
    fname = '../v11/materialLeveling/output_1bar_Vbias43_thr400ADC_4mm.root'
    if (sipm=='HPK'): fname = '../v11/yzAngleScan/output_3bars_Vbias72_thr100ADC_yzangle45_binneAmpWalkCorr.root'
    f[ch] = ROOT.TFile.Open(fname)
    pRall[ch] = f[ch].Get('p_tR_vs_%s_%s'%(corrType,ch))
    pLall[ch] = f[ch].Get('p_tL_vs_%s_%s'%(corrType,ch))
    pR[ch] = {}
    pL[ch] = {}
    canvasL[ch] = ROOT.TCanvas('tL_vs_%s_%s'%(corrType,ch), 'tL_vs_%s_%s'%(corrType,ch))
    canvasL[ch].SetGridx()
    canvasL[ch].SetGridy()
    hdummy.Draw()
    canvasR[ch] = ROOT.TCanvas('tR_vs_%s_%s'%(corrType,ch), 'tR_vs_%s_%s'%(corrType,ch))
    canvasR[ch].SetGridx()
    canvasR[ch].SetGridy()
    hdummy.Draw()
    for ibin in range(0, nbins):
        pR[ch][ibin] = f[ch].Get('p_tR_vs_%s_binXc_%s_%s'%(corrType,ibin,ch))
        pL[ch][ibin] = f[ch].Get('p_tL_vs_%s_binXc_%s_%s'%(corrType,ibin,ch))
        for p in pL[ch][ibin], pR[ch][ibin]:
            p.SetMarkerStyle(22)
            p.SetMarkerSize(0.9)
            col = ibin+2
            if (col == 5 ): col = 800
            p.SetMarkerColor(col)
            p.SetLineColor(col)
        canvasR[ch].cd()
        if (sipm == 'HPK'): ich = 1
        pR[ch][ibin].GetFunction('fitFuncR_%sCorr_%s_%s'%(corrType,ibin, ich)).SetLineColor(col)
        pR[ch][ibin].GetFunction('fitFuncR_%sCorr_%s_%s'%(corrType,ibin, ich)).SetLineWidth(2)
        pR[ch][ibin].Draw('psame')
        canvasL[ch].cd()
        pL[ch][ibin].GetFunction('fitFuncL_%sCorr_%s_%s'%(corrType,ibin, ich)).SetLineColor(col)
        pL[ch][ibin].GetFunction('fitFuncL_%sCorr_%s_%s'%(corrType,ibin, ich)).SetLineWidth(2)
        pL[ch][ibin].Draw('psame')
        print 'BIN %d: mean amplitude = %.02f'%(ibin, pL[ch][ibin].GetMean(1))
        leg.AddEntry(pL[ch][ibin], 'bin %s'%ibin, 'PL')

    leg.AddEntry(pLall[ch], 'all events', 'PL')
    canvasR[ch].cd()
    pRall[ch].GetFunction('fitFuncR_%sCorr_%s'%(corrType,ich)).SetLineColor(ROOT.kBlack)
    pRall[ch].SetMarkerSize(0.5)
    pRall[ch].Draw('psame')
    leg.Draw()
    tChType.Draw()
    canvasL[ch].cd()
    pLall[ch].GetFunction('fitFuncL_%sCorr_%s'%(corrType,ich)).SetLineColor(ROOT.kBlack)
    pLall[ch].SetMarkerSize(0.5)
    pLall[ch].Draw('psame')
    print 'ALL: mean amplitude = %.02f'%pLall[ch].GetMean(1)
    leg.Draw()
    tChType.Draw()
    ROOT.gPad.Update()

    canvasR[ch].SaveAs(outdir+'/'+canvasR[ch].GetName()+'_%s.pdf'%sipm)
    canvasR[ch].SaveAs(outdir+'/'+canvasR[ch].GetName()+'_%s.png'%sipm)
    canvasL[ch].SaveAs(outdir+'/'+canvasL[ch].GetName()+'_%s.pdf'%sipm)
    canvasL[ch].SaveAs(outdir+'/'+canvasL[ch].GetName()+'_%s.png'%sipm)
    
    #raw_input('ok?')
