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



filename1 = '../../v11/output_3bars_Vbias72_thr100ADC_xyangle90_runs6872-6913.root'
#filename2 = '../../v11/materialLeveling/output_1bar_Vbias43_thr400ADC_4mm.root'
filename2 = '../../v11/materialLeveling/output_1bar_Vbias43_thr400ADC_4mm_averageAmpWalkCorr.root'

#tChType = ROOT.TLatex( 0.13, 0.92,channelType)
#tChType.SetNDC()
#tChType.SetTextSize(0.030)


sipms = ['HPK','FBK']
f = {}
f['HPK'] = ROOT.TFile.Open(filename1)
f['FBK'] = ROOT.TFile.Open(filename2)

channel = {'HPK':'BAR1', 'FBK':'BAR0'}

channelType = {}
channelType['HPK'] = 'LYSO:Ce 3x3x57 mm^{3} - HPK S12572-015 3x3 mm^{2}'
channelType['FBK'] = 'LYSO:Ce 3x4x57 mm^{3} - FBK NUV-HD-TE 5x5 mm^{2}'

h = {}
h2 = {}
h2_pfx = {}

for i in sipms:
    h[i]  = f[i].Get('h_posX_residuals_pulseIntCorr_%s'%(channel[i]))
    h2[i] = f[i].Get('h2_posX_residuals_pulseIntCorr_vs_posX_%s'%(channel[i]))
    #h2[i] = f[i].Get('h2_posX_residuals_ampCorr_vs_posX_%s'%(channel[i]))
    h2_pfx[i] = h2[i].ProfileX('h2_pfx_%s'%i)
    h[i].Scale(1./h[i].Integral())

fGaus = {}
for ii, i in enumerate(sipms):
    fGaus[i] = ROOT.TF1('fGaus_%s'%i,'gaus',-100, 100)
    fGaus[i].SetLineColor(1)
    if (i == 'HPK'):
        fGaus[i].SetLineStyle(2)
        h[i].SetMarkerStyle(24)
    h[i].Fit(fGaus[i],'Q','', h[i].GetMean()-3*h[i].GetRMS(),h[i].GetMean()+3*h[i].GetRMS())


#plot
canvas = ROOT.TCanvas('position_residuals','position_residuals')
h['FBK'].GetXaxis().SetRangeUser(-20,20)
h['FBK'].GetYaxis().SetRangeUser(0,0.20)
h['FBK'].GetXaxis().SetTitle('x_{reco} - x_{track} (mm)')
h['FBK'].GetYaxis().SetTitle('normalized entries')
h['FBK'].Draw('e')
h['HPK'].Draw('esame')

leg = ROOT.TLegend(0.15, 0.75, 0.70, 0.89)
leg.SetBorderSize(0)
for ii, i in enumerate(sipms):
    text = '#sigma_{x} = %.2f #pm %.2f mm'%(h[i].GetFunction('fGaus_%s'%i).GetParameter(2), h[i].GetFunction('fGaus_%s'%i).GetParError(2))
    if (i=='HPK'): leg.AddEntry(h[i],'3x3x57 mm^{3}+HPK: %s'%text, 'PL')
    else: leg.AddEntry(h[i],'3x4x57 mm^{3}+FBK: %s'%text, 'PL')
leg.Draw()



tChType = {}
canvas2 = ROOT.TCanvas('position_residuals_vs_x','position_residuals_vs_x')
canvas2.Divide(1,2)
for ii,i in enumerate(sipms):
    canvas2.cd(ii+1)
    binmin = h2_pfx[i].FindFirstBinAbove(0)
    binmax = h2_pfx[i].FindLastBinAbove(0)
    xmin = h2_pfx[i].GetBinCenter(binmin)
    xmax = h2_pfx[i].GetBinCenter(binmax)
    print binmin, binmax, xmin, xmax
    h2[i].GetXaxis().SetTitleSize(0.062)
    h2[i].GetXaxis().SetLabelSize(0.062)
    h2[i].GetYaxis().SetTitleSize(0.062)
    h2[i].GetYaxis().SetLabelSize(0.062)
    h2[i].GetZaxis().SetTitleSize(0.062)
    h2[i].GetZaxis().SetLabelSize(0.062)
    
    h2[i].GetXaxis().SetRangeUser(xmin,xmax)
    h2[i].GetYaxis().SetRangeUser(-20,20)
    h2[i].GetXaxis().SetTitle('x_{track} (mm)')
    h2[i].GetYaxis().SetTitle('x_{reco} - x_{track} (mm)')
    h2[i].Draw('colz')
    h2_pfx[i].Draw('same')
    tChType[i] = ROOT.TLatex( 0.13, 0.92,channelType[i])
    tChType[i].SetNDC()
    tChType[i].SetTextSize(0.050)
    tChType[i].Draw()


for c in canvas, canvas2:
    c.SaveAs(c.GetName()+'.png')
    c.SaveAs(c.GetName()+'.pdf')
    c.SaveAs(c.GetName()+'.C')

#raw_input('ok?')
