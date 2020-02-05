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

filename1 = '../v10/yzAngleScan/mcpCorr/output_3bars_Vbias72_thr100ADC_yzangle45.root'
filename2 = '../v10/yzAngleScan/no_mcpCorr/output_3bars_Vbias72_thr100ADC_yzangle45.root'


f1 = ROOT.TFile.Open(filename1)
f2 = ROOT.TFile.Open(filename2)


channels = ['BAR0', 'BAR1', 'BAR2']

tChType = ROOT.TLatex( 0.13, 0.92, 'LYSO:Ce 3x3x57 mm^{3} - HPK S12572-015')
tChType.SetNDC()
tChType.SetTextSize(0.035)

color = { 'BAR0' : ROOT.kBlue,
          'BAR1' : ROOT.kGreen,
          'BAR2' : ROOT.kRed}

markerstyle = { 'BAR0' : 21,
                'BAR1' : 21,
                'BAR2' : 21}


g1 = {}
g1['BAR0'] = f1.Get('g_tResolGausAve_pulseIntCorr_vs_posY_BAR0')
g1['BAR1'] = f1.Get('g_tResolGausAve_pulseIntCorr_vs_posY_BAR1')
g1['BAR2'] = f1.Get('g_tResolGausAve_pulseIntCorr_vs_posY_BAR2')

g2 = {}
g2['BAR0'] = f2.Get('g_tResolGausAve_pulseIntCorr_vs_posY_BAR0')
g2['BAR1'] = f2.Get('g_tResolGausAve_pulseIntCorr_vs_posY_BAR1')
g2['BAR2'] = f2.Get('g_tResolGausAve_pulseIntCorr_vs_posY_BAR2')

gMCP = {}
gMCP['BAR0'] = ROOT.TGraphErrors()
gMCP['BAR1'] = ROOT.TGraphErrors()
gMCP['BAR2'] = ROOT.TGraphErrors()

for b in ['BAR0','BAR1','BAR2']:
    gMCP[b].SetMarkerSize(1.0)
    gMCP[b].SetMarkerColor(color[b])
    gMCP[b].SetLineColor(color[b])
    n = 0
    for i in range(0,g2[b].GetN()):
        y = ROOT.Double(0)
        ey = ROOT.Double(0)
        sigma1  = ROOT.Double(0)
        sigma2 = ROOT.Double(0)
        esigma1  = ROOT.Double(0)
        esigma2 = ROOT.Double(0)
        g1[b].GetPoint(i, y, sigma1)
        g2[b].GetPoint(i, y, sigma2)
        esigma1 = g1[b].GetErrorY(i)
        esigma2 = g2[b].GetErrorY(i)
        if (sigma2 > sigma1):
            sigmaMCP = math.sqrt(sigma2*sigma2 - sigma1*sigma1)
            esigmaMCP = 1./sigmaMCP * math.sqrt(pow(sigma2*esigma2,2) + pow(sigma1*esigma1,2))
        else:
            continue
        gMCP[b].SetPoint(n, y, sigmaMCP)
        gMCP[b].SetPointError(n, 0, esigmaMCP)
        n=n+1
        
for i, ch in enumerate(channels):
    for j,g in enumerate([g1[ch], g2[ch]]):
        g.SetMarkerStyle(24+j)
        g.SetMarkerSize(0.7)
        g.SetMarkerColor(color[ch]+j)
        g.SetLineColor(color[ch]+j)


leg = ROOT.TLegend(0.15,0.7,0.45,0.89)
leg.SetBorderSize(0)
leg.AddEntry(g1['BAR0'],'with MCP corrections','PL')
leg.AddEntry(g2['BAR0'],'w/o MCP corrections','PL')

# absolute time resolution
canvas = ROOT.TCanvas('bars_mcpCorr_comparison','bars_mcpCorr_comparison',700,600)
canvas.SetGridy()
hdummy = ROOT.TH2F('','', 100, 20, 29.2, 100, 20, 100)
hdummy.GetYaxis().SetTitle('time resolution (ps)')
hdummy.GetXaxis().SetTitle('y (mm)')
hdummy.Draw()
for i in channels:
    g1[i].Draw('plsame')
    g2[i].Draw('plsame')
leg.Draw('same')
tChType.Draw()
raw_input('ok?')
canvas.SaveAs(canvas.GetName()+'.png')
canvas.SaveAs(canvas.GetName()+'.pdf')

canvasMCP = ROOT.TCanvas('c_MCPcorr','c_MCPcorr')
canvasMCP.SetGridx()
canvasMCP.SetGridy()
hdummyMCP = ROOT.TH2F('','', 100, 20, 30, 100, 0, 100)
hdummyMCP.GetYaxis().SetTitle('#sigma of MCP corrections (ps)')
hdummyMCP.GetXaxis().SetTitle('y (mm)')
hdummyMCP.Draw()
gMCP['BAR0'].Draw('psame')
gMCP['BAR1'].Draw('psame')
gMCP['BAR2'].Draw('psame')
canvasMCP.SaveAs(canvasMCP.GetName()+'.png')
canvasMCP.SaveAs(canvasMCP.GetName()+'.pdf')
raw_input('continue?')
