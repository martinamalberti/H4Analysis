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
ROOT.gErrorIgnoreLevel = ROOT.kWarning;

Vbias = sys.argv[1]
thr = sys.argv[2]
angle = sys.argv[3]
ampCorrType = 'pulseInt'
useTdiff = True

filename = '../../v11/output_3bars_Vbias%s_thr%sADC_xyangle%s_runs6872-6913.root'%(Vbias, thr, angle)
channelType = 'LYSO:Ce 3x3x57 mm^{3} - HPK S12572-015 3x3 mm^{2} '
channel = 'BAR1'
xbin = 4

if (angle == '4'):
    filename = '../../v11/materialLeveling/output_1bar_Vbias%s_thr%sADC_%smm.root'%(Vbias, thr, angle)
    channelType = 'LYSO:Ce 3x4x57 mm^{3} - FBK NUV-HD-TE 5x5 mm^{2}'
    channel = 'BAR0'
    xbin = 11
    
f = ROOT.TFile.Open(filename)

labels = ['local','global']
colors = {'global' : ROOT.kBlack,
          'local'  : ROOT.kViolet}
h0 = {}
h0['global'] = f.Get('h_tAve_%sCorr_%s'%(ampCorrType,channel))            
h0['local']  = f.Get('xBins/h_tAve_%sCorr_binX_%d_%s'%(ampCorrType,xbin,channel))
if (useTdiff):
    h0['global'] = f.Get('h_tDiff_%sCorr_posCorr_%s'%(ampCorrType,channel))            
    h0['local']  = f.Get('xBins/h_tDiff_%sCorr_posCorr_binX_%d_%s'%(ampCorrType,xbin,channel))
nRe = int(h0['global'].GetNbinsX()/h0['local'].GetNbinsX())
h0['global'].Rebin(nRe)

# shift to put mean at 0
h = {}
nbins = h0['global'].GetNbinsX()
xmin = h0['global'].GetXaxis().GetXmin() - h0['global'].GetMean()
xmax = h0['global'].GetXaxis().GetXmax()- h0['global'].GetMean()
if (useTdiff):
    xmin = xmin/2
    xmax = xmax/2
h['global'] = ROOT.TH1F('h_tAve_%sCorr_global_%s'%(ampCorrType,channel),'', nbins, xmin, xmax )
h['local'] = ROOT.TH1F('h_tAve_%sCorr_local_%s'%(ampCorrType,channel), '', nbins, xmin, xmax  )
for ibin in range(1, nbins):
    h['global'].SetBinContent(ibin, h0['global'].GetBinContent(ibin))
    h['global'].SetBinError(ibin, h0['global'].GetBinError(ibin))
    h['local'].SetBinContent(ibin, h0['local'].GetBinContent(ibin))
    h['local'].SetBinError(ibin, h0['local'].GetBinError(ibin))
    
fGaus =  {}
for i, label in enumerate(labels):
    h[label].Scale(1./h[label].Integral())
    fGaus[label] = ROOT.TF1('fGaus_%s'%label,'gaus', -20, 20)
    fGaus[label].SetLineColor(colors[label])
    fGaus[label].SetLineStyle(i+1)
    h[label].Fit(fGaus[label])
    
    
# plot style
for label in labels:
    h[label].SetLineColor(colors[label])
    h[label].SetMarkerColor(colors[label])
    h[label].SetMarkerSize(0.9)
    h[label].SetMarkerStyle(20)
    if (label == 'local'):  h[label].SetMarkerStyle(21)
 
legend = ROOT.TLegend(0.5,0.76,0.89,0.89)
legend.SetBorderSize(0)
legend.AddEntry(h['global'],'w/o beam spot selection','PL')
legend.AddEntry(h['local'],'#Deltax < 2 mm','PL')
    
canvas = ROOT.TCanvas('tRes_global_local__%s'%channel,'tRes_global_local__%s'%channel)
xmin = -0.25
xmax = 0.25
ymax = 0.50
if (angle == '4'): ymax = 0.30
if (useTdiff):
    ymax = 0.3
    if (angle == '4'): ymax = 0.20
    
hdummy = ROOT.TH2F('hdummy','', 100, xmin, xmax, 100, 0, ymax)
hdummy.GetYaxis().SetTitle('a.u.')
hdummy.GetXaxis().SetTitle('t_{average} - t_{MPC} (ns)')
if (useTdiff):
    hdummy.GetXaxis().SetTitle('t_{diff}/2 (ns)')
hdummy.GetXaxis().SetNdivisions(505)
hdummy.GetYaxis().SetTitleOffset(1.15)
hdummy.Draw()
h['global'].DrawNormalized('psame')
h['local'].DrawNormalized('psame')
legend.Draw()


tRes = {}
text = ''
resolMCP = 12.3
if (useTdiff): resolMCP = 0
for i,label in enumerate(labels):
    resolGaus = h[label].GetFunction('fGaus_%s'%label).GetParameter(2)*1000.
    resolGausErr = h[label].GetFunction('fGaus_%s'%label).GetParError(2)*1000.

    resolGaus = math.sqrt(resolGaus*resolGaus-resolMCP*resolMCP)

    text = '#sigma = %.1f #pm %.1f ps'%(resolGaus, resolGausErr)

    tRes[label] = ROOT.TLatex( 0.15, 0.785+i*0.06, text )
    tRes[label].SetNDC()
    tRes[label].SetTextSize(0.035)
    tRes[label].SetTextColor(colors[label])
    tRes[label].Draw()


tChType = ROOT.TLatex( 0.13, 0.92,channelType)
tChType.SetNDC()
tChType.SetTextSize(0.030)
tChType.Draw()

if (useTdiff==False):
    canvas.SaveAs(canvas.GetName()+'.png')
    canvas.SaveAs(canvas.GetName()+'.pdf')
    canvas.SaveAs(canvas.GetName()+'.C')
else:
    canvas.SaveAs(canvas.GetName()+'_tDiff.png')
    canvas.SaveAs(canvas.GetName()+'_tDiff.pdf')
    canvas.SaveAs(canvas.GetName()+'_tDiff.C')
    
#raw_input('ok?')
