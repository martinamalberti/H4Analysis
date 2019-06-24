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

channels = ['BAR%s'%i for i in range(0,1)]
        
tChType = ROOT.TLatex( 0.13, 0.92, 'LYSO:Ce 4x3x57 mm^{3} - FBK thin-epi 5x5 mm^{2} ')
tChType.SetNDC()
tChType.SetTextSize(0.035)

leg = ROOT.TLegend(0.65, 0.65, 0.89, 0.89)
leg.SetBorderSize(0)

f = {}
pR = {}
pL = {}

pRall = {}
pLall = {}

canvasL = {}
canvasR = {}

nbins = 7

hdummy = ROOT.TH2F('hdummy','',100, 0, 0.3, 100, -5.4, -4.6)
hdummy.GetYaxis().SetTitle('time (ns)')
hdummy.GetXaxis().SetTitle('amplitude (V)')

for ich, ch in enumerate(channels):
    fname = '../v4/output_1bar_Vbias43_thr1000ADC_4mm.root'
    #fname = '../v4/output_1bar_Vbias43_thr700ADC_3mm.root'
    if ('2mm' in fname or '3mm' in fname): nbins = 6
    f[ch] = ROOT.TFile.Open(fname)
    pRall[ch] = f[ch].Get('p_tR_vs_amp_%s'%(ch))
    pLall[ch] = f[ch].Get('p_tL_vs_amp_%s'%(ch))
    pR[ch] = {}
    pL[ch] = {}
    canvasL[ch] = ROOT.TCanvas('tL_vs_amp_%s'%ch, 'tL_vs_amp_%s'%ch)
    canvasL[ch].SetGridx()
    canvasL[ch].SetGridy()
    hdummy.Draw()
    canvasR[ch] = ROOT.TCanvas('tR_vs_amp_%s'%ch, 'tR_vs_amp_%s'%ch)
    canvasR[ch].SetGridx()
    canvasR[ch].SetGridy()
    hdummy.Draw()
    for ibin in range(0, nbins):
        pR[ch][ibin] = f[ch].Get('p_tR_vs_amp_binXc_%s_%s'%(ibin,ch))
        pL[ch][ibin] = f[ch].Get('p_tL_vs_amp_binXc_%s_%s'%(ibin,ch))
        for p in pL[ch][ibin], pR[ch][ibin]:
            p.SetMarkerStyle(22)
            p.SetMarkerSize(0.9)
            col = ibin+2
            if (col == 5 ): col = 800
            p.SetMarkerColor(col)
            p.SetLineColor(col)
        canvasR[ch].cd()
        pR[ch][ibin].GetFunction('fitFuncR_ampCorr_%s_%s'%(ibin, ich)).Delete()
        pR[ch][ibin].Draw('psame')
        canvasL[ch].cd()
        pL[ch][ibin].GetFunction('fitFuncL_ampCorr_%s_%s'%(ibin, ich)).Delete()
        pL[ch][ibin].Draw('psame')
        leg.AddEntry(pL[ch][ibin], 'bin %s'%ibin, 'PL')

    leg.AddEntry(pLall[ch], 'all events', 'PL')
    canvasR[ch].cd()
    pRall[ch].GetFunction('fitFuncR_ampCorr_%s'%(ich)).Delete()
    pRall[ch].SetMarkerSize(0.5)
    pRall[ch].Draw('psame')
    leg.Draw()
    tChType.Draw()
    canvasL[ch].cd()
    pLall[ch].GetFunction('fitFuncL_ampCorr_%s'%(ich)).Delete()
    pLall[ch].SetMarkerSize(0.5)
    pLall[ch].Draw('psame')
    leg.Draw()
    tChType.Draw()
    ROOT.gPad.Update()


    canvasR[ch].SaveAs(outdir+'/'+canvasR[ch].GetName()+'.pdf')
    canvasR[ch].SaveAs(outdir+'/'+canvasR[ch].GetName()+'.png')
    canvasL[ch].SaveAs(outdir+'/'+canvasL[ch].GetName()+'.pdf')
    canvasL[ch].SaveAs(outdir+'/'+canvasL[ch].GetName()+'.png')
    
    raw_input('ok?')
