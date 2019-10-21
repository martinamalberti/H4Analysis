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
print filename

filenameRef = '../v7/output_3bars_Vbias%s_thr%sADC_xyangle90_runs7639-7679.root'%(Vbias, thr)
fRef = ROOT.TFile.Open(filenameRef)



tChType = ROOT.TLatex( 0.13, 0.92, 'LYSO:Ce 3x3x57 mm^{3} - HPK S12572-015')
tChType.SetNDC()
tChType.SetTextSize(0.035)

g = {}
g['BAR0'] = f.Get('g_tResolGausDiff_ampCorr_posCorr_vs_posY_BAR0')
g['BAR1'] = f.Get('g_tResolGausDiff_ampCorr_posCorr_vs_posY_BAR1')
g['BAR2'] = f.Get('g_tResolGausDiff_ampCorr_posCorr_vs_posY_BAR2')
g['COMB'] = f.Get('g_tResolGausDiffDiff_ampCorr_posCorr_vs_posY_overlap')
g['COMB1hit'] = f.Get('g_tResolGausDiffDiff_ampCorr_posCorr_vs_posY_1hit')
g['COMB2hit'] = f.Get('g_tResolGausDiffDiff_ampCorr_posCorr_vs_posY_2hit')


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


fit0 = ROOT.TF1('fit0','pol6',-100,100)
fit0.SetLineColor(color['BAR0'])
g['BAR0'].Fit(fit0,'QSRN','', 25,30)

fit1 = ROOT.TF1('fit1','pol6',-100,100)
fit1.SetLineColor(color['BAR1'])
g['BAR1'].Fit(fit1,'QSRN','', 22.5,27)

fit2 = ROOT.TF1('fit2','pol6',-100,100)
fit2.SetLineColor(color['BAR2'])
g['BAR2'].Fit(fit2,'QSRN','', 20,24)
        
for i in ['BAR0', 'BAR1', 'BAR2','COMB']:
    g[i].SetMarkerStyle(markerstyle[i])
    g[i].SetMarkerSize(1.0)
    g[i].SetMarkerColor(color[i])
    g[i].SetLineColor(color[i])
    if ('COMB' in i):
        g[i].SetMarkerSize(0.9)

leg = ROOT.TLegend(0.15,0.7,0.45,0.89)
leg.SetBorderSize(0)
leg.AddEntry(g['BAR0'],'top bar','PL')
leg.AddEntry(g['BAR1'],'middle bar','PL')
leg.AddEntry(g['BAR2'],'bottom bar','PL')
leg.AddEntry(g['COMB'],'average','PL')
#leg.AddEntry(g['COMBW1'],'amplitude weighted average','PL')
#leg.AddEntry(g['COMBW2'],'#sigma_{t} weighted average','PL')


# absolute time resolution
canvas = ROOT.TCanvas('bars_signalSharing_time_resolution_Vbias%s_thr%s_yzangle%s'%(Vbias, thr, angle),'bars_signalSharing_time_resolution_Vbias%s_thr%s_yzangle%s'%(Vbias, thr, angle),700,600)
canvas.SetGridy()
hdummy = ROOT.TH2F('','', 100, 20, 29.2, 100, 0, 140)
hdummy.GetYaxis().SetNdivisions(520)
hdummy.GetYaxis().SetTitle('time resolution (ps)')
hdummy.GetXaxis().SetTitle('y (mm)')
hdummy.Draw()
g['BAR0'].Draw('plsame')
g['BAR1'].Draw('plsame')
g['BAR2'].Draw('plsame')
g['COMB'].Draw('psame')
g['COMB1hit'].Draw('psame') 
g['COMB2hit'].Draw('p*same') 
leg.Draw('same')
tChType.Draw()
canvas.SaveAs(subfolder+'/'+canvas.GetName()+'.png')
canvas.SaveAs(subfolder+'/'+canvas.GetName()+'.pdf')



# normalize time resolution at normal incidence
tResRef = {}
for ch in ['BAR0','BAR1','BAR2']:
    # run at normal incidence
    hDiff    = fRef.Get('h_tDiff_ampCorr_posCorr_%s'%ch)
    funDiff  = hDiff.GetFunction('fitFunDiff_ampCorr_posCorr_%s'%ch)
    tResRef[ch] = funDiff.GetParameter(2)/2
sigmatNorm = 1000.*(tResRef['BAR0'] + tResRef['BAR1'] + tResRef['BAR2']) / 3 # average time resolution of the three bars at normal incidence 
print 'Time resolution at 90 deg incidence = %.01f ps'%sigmatNorm

for b in ['BAR0','BAR1','BAR2', 'COMB']:
    gnorm[b].SetMarkerStyle(markerstyle[b])
    gnorm[b].SetMarkerSize(1.0)
    gnorm[b].SetMarkerColor(color[b])
    gnorm[b].SetLineColor(color[b])
    for i in range(0,g[b].GetN()):
        x = ROOT.Double(0)
        sigma = ROOT.Double(0)
        g[b].GetPoint(i, x, sigma)
        if (sigma <= 0): continue
        xerr = g[b].GetErrorX(i)
        sigmaerr =  sigma/sigmatNorm * math.sqrt(pow(g[b].GetErrorY(i)/sigma,2) + pow(2./sigmatNorm,2))
        gnorm[b].SetPoint(i, x, sigma/sigmatNorm)
        gnorm[b].SetPointError(i, xerr, sigmaerr)
 
canvas2 = ROOT.TCanvas('bars_signalSharing_time_resolution_Vbias%s_thr%s_yzangle%s_normalized'%(Vbias, thr, angle),'bars_signalSharing_time_resolution_Vbias%s_thr%s_yzangle%s_normalized'%(Vbias, thr, angle),700,600)
canvas2.SetGridy()
hdummy2 = ROOT.TH2F('','', 100, 20, 29.2, 100, 0.0, 4.5)
#hdummy2.GetYaxis().SetNdivisions(520)
hdummy2.GetYaxis().SetTitle('time resolution normalized to normal incidence')
hdummy2.GetXaxis().SetTitle('y (mm)')
hdummy2.Draw()
gnorm['BAR0'].Draw('plsame')
gnorm['BAR1'].Draw('plsame')
gnorm['BAR2'].Draw('plsame')
gnorm['COMB'].Draw('psame')
leg.Draw('same')
tChType.Draw()

canvas2.SaveAs(subfolder+'/'+canvas2.GetName()+'.png')
canvas2.SaveAs(subfolder+'/'+canvas2.GetName()+'.pdf')

    
# fraction of events with signal above threshold in both bars
canvas3 = ROOT.TCanvas('bars_fraction_above_thr_signalSharing_Vbias%s_thr%s_yzangle%s'%(Vbias, thr, angle),'bars_fraction_above_thr_signalSharing_Vbias%s_thr%s_yzangle%s'%(Vbias, thr, angle),700,600)
canvas3.SetGridx()
canvas3.SetGridy()
g = f.Get('g_nEventsRatio_vs_posY')
g.GetXaxis().SetTitle('y (mm)')
g.GetYaxis().SetTitle('fraction of events with 2 hits')
g.GetYaxis().SetRangeUser(0, 1.1)
g.Draw('apc')
tChType.Draw()

canvas3.SaveAs(subfolder+'/'+canvas3.GetName()+'.png')
canvas3.SaveAs(subfolder+'/'+canvas3.GetName()+'.pdf')

#raw_input('ok?')



