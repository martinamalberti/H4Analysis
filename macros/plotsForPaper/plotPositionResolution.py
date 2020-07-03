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

h = f.Get('h_posX_residuals_pulseIntCorr_%s'%(channel))

canvas = ROOT.TCanvas('position_residuals_%s'%(channel),'position_residuals_%s'%(channel))
h.GetXaxis().SetRangeUser(-40,40)
h.GetYaxis().SetRangeUser(0,h.GetMaximum()*1.1)
h.GetXaxis().SetTitle('x_{reco} -x_{track} (mm)')
h.GetYaxis().SetTitle('entries')
h.Draw('e')
h.Fit('gaus','Q','', h.GetMean()-3*h.GetRMS(),h.GetMean()+3*h.GetRMS())
canvas.Modified()
canvas.Update()

text = '#sigma_{x} = %.2f #pm %.2f mm'%(h.GetFunction('gaus').GetParameter(2), h.GetFunction('gaus').GetParError(2))
latex = ROOT.TLatex( 0.15, 0.82, text )
latex.SetNDC()
latex.SetTextSize(0.035)
latex.Draw()

#ROOT.gStyle.SetOptFit(0)
#ROOT.gStyle.SetFitFormat('4.2g')
#stat = h.GetListOfFunctions().FindObject('stats')
#stat.SetX1NDC(0.65)
#stat.SetX2NDC(0.90)
#stat.SetY1NDC(0.72)
#stat.SetY2NDC(0.88)
#canvas.Modified()
#canvas.Update()



tChType = ROOT.TLatex( 0.13, 0.92,channelType)
tChType.SetNDC()
tChType.SetTextSize(0.030)
tChType.Draw()

canvas.SaveAs(canvas.GetName()+'.png')
canvas.SaveAs(canvas.GetName()+'.pdf')
canvas.SaveAs(canvas.GetName()+'.C')

#raw_input('ok?')
