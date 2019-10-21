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

channels = ['BAR%s'%i for i in range(0,3)]
thresholds = ['Opt', '200', '500', '1000']

ff = {}
gAve = {}
gDiff = {}

# load graphs
for thr in thresholds:
    fname = 'xyAngleScans_v4/plotsVsAngle/LEthreshold%s/graphs_timeResolution_vs_slantThickness.root'%thr
    if ( thr != 'Opt'):
        fname = 'xyAngleScans_v4/plotsVsAngle/LEthreshold%sADC/graphs_timeResolution_vs_slantThickness.root'%thr
    ff[thr] = ROOT.TFile.Open(fname)
    gAve[thr] = {}
    gDiff[thr] = {}
    for ch in channels:
        gAve[thr][ch] = ff[thr].Get('gAve_vs_slantTickness_%s'%ch)
        gDiff[thr][ch] = ff[thr].Get('gDiff_vs_slantTickness_%s'%ch)


canvasAve = {}
canvasDiff = {}
tPowAve = {}
tPowDiff = {}

leg = ROOT.TLegend(0.60, 0.68, 0.89, 0.89)
leg.SetBorderSize(0)

hdummy = ROOT.TH2F('hdummy','',100,0,16, 100, 0, 100)
hdummy.GetXaxis().SetTitle('slant thickness (mm) ')
hdummy.GetYaxis().SetTitle('time resolution (ps) ')

tChType = ROOT.TLatex( 0.13, 0.92, 'LYSO:Ce 3x3x57 mm^{3} - HPK S12572-015')
tChType.SetNDC()
tChType.SetTextSize(0.035)
# plot
for ich, ch in enumerate(channels):
    canvasAve[ch] = ROOT.TCanvas('c_timeResolAve_vs_slantThickness_vs_LEthreshold_%s'%ch,'c_timeResolAve_vs_slantThickness_vs_LEthreshold_%s'%ch)
    canvasAve[ch].SetGridx()
    canvasAve[ch].SetGridy()
    hdummy.Draw()
    canvasDiff[ch] = ROOT.TCanvas('c_timeResolDiff_vs_slantThickness_vs_LEthreshold_%s'%ch,'c_timeResolDiff_vs_slantThickness_vs_LEthreshold_%s'%ch)
    canvasDiff[ch].SetGridx()
    canvasDiff[ch].SetGridy()
    hdummy.Draw()

    tPowAve[ch] = {}
    tPowDiff[ch] = {}
    
    for i, thr in enumerate(thresholds):
        canvasAve[ch].cd()
        (gAve[thr][ch].GetFunction('fitAve')).SetLineColor(1+i)
        gAve[thr][ch].SetLineColor(1+i)
        gAve[thr][ch].SetMarkerColor(1+i)
        gAve[thr][ch].SetMarkerSize(0.7)
        gAve[thr][ch].Draw('psame')
        tPowAve[ch][thr] = ROOT.TLatex( 0.15, 0.85-i*0.05, '#sigma_{t} ~  x^{%.2f +/- %.2f}'%( (gAve[thr][ch].GetFunction('fitAve')).GetParameter(0), (gAve[thr][ch].GetFunction('fitAve')).GetParError(0)))
        tPowAve[ch][thr].SetNDC()
        tPowAve[ch][thr].SetTextSize(0.045)
        tPowAve[ch][thr].SetTextColor(1+i)
        tPowAve[ch][thr].Draw()
        tChType.Draw()

        if (ich == 0):
            if (thr == 'Opt'): leg.AddEntry(gAve[thr][ch],'optimal LE thr','PL')
            else: leg.AddEntry(gAve[thr][ch],'LE thr = %.0f mV'%(float(thr) * 1000./4096),'PL')
                
        canvasDiff[ch].cd()
        (gDiff[thr][ch].GetFunction('fitDiff')).SetLineColor(1+i)
        gDiff[thr][ch].SetLineColor(1+i)
        gDiff[thr][ch].SetMarkerColor(1+i)
        gDiff[thr][ch].SetMarkerSize(0.7)
        gDiff[thr][ch].Draw('psame')
        tPowDiff[ch][thr] = ROOT.TLatex( 0.15, 0.85-i*0.05, '#sigma_{t} ~  x^{-(%.2f +/- %.2f)}'%( (gDiff[thr][ch].GetFunction('fitDiff')).GetParameter(0), (gDiff[thr][ch].GetFunction('fitDiff')).GetParError(0)))
        tPowDiff[ch][thr].SetNDC()
        tPowDiff[ch][thr].SetTextSize(0.045)
        tPowDiff[ch][thr].SetTextColor(1+i)
        tPowDiff[ch][thr].Draw()
        tChType.Draw()
    
    canvasAve[ch].cd()
    leg.Draw()
    canvasDiff[ch].cd()
    leg.Draw()


    
    canvasAve[ch].SaveAs(outdir+'/'+canvasAve[ch].GetName()+'.png')
    canvasAve[ch].SaveAs(outdir+'/'+canvasAve[ch].GetName()+'.pdf')
    canvasDiff[ch].SaveAs(outdir+'/'+canvasDiff[ch].GetName()+'.pdf')
    canvasDiff[ch].SaveAs(outdir+'/'+canvasDiff[ch].GetName()+'.png')

    #raw_input('ok?')
