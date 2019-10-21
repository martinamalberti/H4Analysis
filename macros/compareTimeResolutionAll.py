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
#set the tdr style
tdrstyle.setTDRStyle()
ROOT.gStyle.SetOptStat(0)
ROOT.gStyle.SetOptFit(0)


outdir = sys.argv[1]

channels = ['BAR%s'%i for i in range(0,3)]

regions =  ['barFull','barCenter', 'barEdge']
#ampCorrTypes = ['ampCorr','pulseIntCorr']
ampCorrTypes = ['pulseIntCorr']
#ampCorrTypes = ['ampCorr']

f = {}
f2 = {}

col = {'barFull'  : ROOT.kBlue,
       'barCenter': ROOT.kOrange,
       'barEdge'  : ROOT.kOrange+2
      }

for r in regions:
    f[r] = {}
    f2[r] = {}
    for corr in ampCorrTypes:
        f[r][corr] = ROOT.TFile.Open('./xyAngleScans_v5/%s/%s/graphs_timeResolution_vs_slantThickness.root'%(r,corr))
        if (r == 'barFull'):
            f2[r][corr] = ROOT.TFile.Open('./xyAngleScans_v5/%s/binnedAmpWalkCorr/%s/graphs_timeResolution_vs_slantThickness.root'%(r,corr))
            
tChType = ROOT.TLatex( 0.13, 0.92, 'LYSO:Ce 3x3x57 mm^{3} - HPK S12572-015')
tChType.SetNDC()
tChType.SetTextSize(0.035)

gDiff = {}
gDiff2 = {}
canvas = {}

leg = {}

hdummy = ROOT.TH2F('hdummy','',100, 0, 15, 100, 10, 45)

for ch in channels:
    gDiff[ch] = {}
    gDiff2[ch] = {}
    canvas[ch] = ROOT.TCanvas('comparison_timeResolution_'+ch, 'comparison_timeResolution_'+ch)
    canvas[ch].SetGridx()
    canvas[ch].SetGridy()
    hdummy.GetXaxis().SetTitle('slant thickness (mm)')
    hdummy.GetYaxis().SetTitle('time resolution (ps)')
    hdummy.Draw()
    leg[ch] = ROOT.TLegend(0.15,0.75,0.65,0.89)
    leg[ch].SetBorderSize(0)
    for i,r in enumerate(regions):
        gDiff[ch][r] = {}
        gDiff2[ch][r] = {}
        for j,corr in enumerate(ampCorrTypes):
            if (r == 'barFull'):
                gDiff2[ch][r][corr] = f2[r][corr].Get('gDiff_vs_slantTickness_'+ ch)
                gDiff2[ch][r][corr].SetMarkerSize(0.7)
                gDiff2[ch][r][corr].SetMarkerColor(1)
                gDiff2[ch][r][corr].SetLineColor(1)
                if (j==0):
                    gDiff2[ch][r][corr].SetMarkerStyle(20)
                else:
                    gDiff2[ch][r][corr].SetMarkerStyle(24)
                gDiff2[ch][r][corr].GetFunction('fitDiff').SetLineColor(1)
                leg[ch].AddEntry(gDiff2[ch][r][corr],'%s - %s - binned '%(r,corr), 'PL')
                gDiff2[ch][r][corr].Draw('psame')
                
            gDiff[ch][r][corr] = f[r][corr].Get('gDiff_vs_slantTickness_'+ ch)
            gDiff[ch][r][corr].SetMarkerSize(0.7)
            gDiff[ch][r][corr].SetMarkerColor(col[r])
            gDiff[ch][r][corr].SetLineColor(col[r])
            gDiff[ch][r][corr].GetFunction('fitDiff').SetLineColor(col[r])
            if (j==0):
                gDiff[ch][r][corr].SetMarkerStyle(20)
            else:
                gDiff[ch][r][corr].SetMarkerStyle(24)

            leg[ch].AddEntry(gDiff[ch][r][corr],'%s - %s'%(r,corr), 'PL')
            gDiff[ch][r][corr].Draw('psame')


    leg[ch].Draw()            


    for t in '.pdf','.png':
        canvas[ch].SaveAs(outdir+'/'+canvas[ch].GetName()+t)

raw_input('ok?')

    
