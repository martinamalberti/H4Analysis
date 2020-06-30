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

filename = '../../v11/output_3bars_Vbias%s_thr%sADC_xyangle%s_runs6872-6913.root'%(Vbias, thr, angle)
channelType = 'LYSO:Ce 3x3x57 mm^{3} - HPK S12572-015 3x3 mm^{2}'
channel = 'BAR1'

if (angle == '4'):
    filename = '../../v11/materialLeveling/output_1bar_Vbias%s_thr%sADC_%smm.root'%(Vbias, thr, angle)
    channelType = 'LYSO:Ce 3x4x57 mm^{3} - FBK NUV-HD-TE 5x5 mm^{2}'
    channel = 'BAR0'

f = ROOT.TFile.Open(filename)

labels = ['Ave','Diff','L','R']
colors = {'Ave' : ROOT.kBlack,
          'L'   : ROOT.kBlue,
          'R'   : ROOT.kRed,
          'Diff': ROOT.kGray+2}
g = {}
g['Ave'] = f.Get('g_tResolGausAve_%sCorr_%s'%(ampCorrType,channel))
g['L'] = f.Get('g_tResolGausL_%sCorr_%s'%(ampCorrType,channel))
g['R'] = f.Get('g_tResolGausR_%sCorr_%s'%(ampCorrType,channel))
g['Diff'] = f.Get('g_tResolGausDiff_%sCorr_posCorr_%s'%(ampCorrType,channel))

# scale tDiff by 0.5
g['Diff'].GetFunction('fpol0_tDiff_%sCorr_posCorr_%s'%(ampCorrType,channel)).Delete()
for i in range(0, g['Diff'].GetN()):
    g['Diff'].GetY()[i] = g['Diff'].GetY()[i] * 0.5
    g['Diff'].GetEY()[i] = g['Diff'].GetEY()[i] * 0.5
fDiff = ROOT.TF1('fpol0_tDiff_%sCorr_%s'%(ampCorrType,channel),'[0]', -100, 100)
fDiff.SetLineStyle(2)
fDiff.SetLineColor(colors['Diff'])
g['Diff'].Fit(fDiff, 'QR')


# plot style
for label in labels:
    g[label].SetLineColor(colors[label])
    g[label].SetMarkerColor(colors[label])
    #g[label].SetMarkerSize(0.7)
    g[label].SetMarkerStyle(20)
    if (label == 'Diff'): g[label].SetMarkerStyle(24)

legend = ROOT.TLegend(0.7,0.7,0.89,0.89)
legend.SetBorderSize(0)
legend.AddEntry(g['Ave'],'t_{average}','PL')
legend.AddEntry(g['Diff'],'t_{diff/2}','PL')
legend.AddEntry(g['L'],'t_{left}','PL')
legend.AddEntry(g['R'],'t_{right}','PL')

canvas = ROOT.TCanvas('g_tResolGausAve_%sCorr_%s'%(ampCorrType,channel),'g_tResolGausAve_%sCorr_%s'%(ampCorrType,channel))
xmin = g['Ave'].GetMean()-2.5*g['Ave'].GetRMS()
xmax = g['Ave'].GetMean()+2.5*g['Ave'].GetRMS()
hdummy = ROOT.TH2F('hdummy','', 100, xmin, xmax, 100, 15, 65)
hdummy.GetXaxis().SetTitle('x (mm)')
hdummy.GetYaxis().SetTitle('time resolution (ps)')
hdummy.Draw()
g['Ave'].Draw('psame')
g['L'].Draw('psame')
g['R'].Draw('psame')
g['Diff'].Draw('psame')
legend.Draw()

tRes = {}
text = ''
for i,label in enumerate(labels):
    resolGaus = g[label].GetFunction('fpol0_t%s_%sCorr_%s'%(label,ampCorrType,channel)).GetParameter(0)
    resolGausErr = g[label].GetFunction('fpol0_t%s_%sCorr_%s'%(label,ampCorrType,channel)).GetParError(0)
    
    if (label == 'L'):    text = 't_{left}   : #sigma = %.1f #pm %.1f ps'%(resolGaus, resolGausErr)
    if (label == 'R'):    text = 't_{right}  : #sigma = %.1f #pm %.1f ps'%(resolGaus, resolGausErr)
    if (label == 'Ave'):  text = 't_{average}: #sigma = %.1f #pm %.1f ps'%(resolGaus, resolGausErr)
    if (label == 'Diff'): text = 't_{diff/2} : #sigma = %.1f #pm %.1f ps'%(resolGaus, resolGausErr)

    tRes[label] = ROOT.TLatex( 0.15, 0.85-i*0.05, text )
    tRes[label].SetNDC()
    tRes[label].SetTextSize(0.035)
    tRes[label].SetTextColor(colors[label])
    tRes[label].Draw()

tChType = ROOT.TLatex( 0.13, 0.92,channelType)
tChType.SetNDC()
tChType.SetTextSize(0.030)
tChType.Draw()

canvas.SaveAs(canvas.GetName()+'.png')
canvas.SaveAs(canvas.GetName()+'.pdf')
canvas.SaveAs(canvas.GetName()+'.C')

#raw_input('ok?')
