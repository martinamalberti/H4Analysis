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

filename = '../v4/output_3bars_Vbias%s_thr%sADC_yzangle%s.root'%(Vbias, thr, angle)
f = ROOT.TFile.Open(filename)


filenameRef = '../v4/output_3bars_Vbias%s_thr%sADC_xyangle90_runs7639-7679.root'%(Vbias, thr)
fRef = ROOT.TFile.Open(filenameRef)



tChType = ROOT.TLatex( 0.13, 0.92, 'LYSO:Ce 3x3x57 mm^{3} - HPK S12572-015')
tChType.SetNDC()
tChType.SetTextSize(0.035)

g = {}
g['BAR0'] = f.Get('g_tResolGausAve_ampCorr_vs_posY_BAR0')
g['BAR1'] = f.Get('g_tResolGausAve_ampCorr_vs_posY_BAR1')
g['BAR2'] = f.Get('g_tResolGausAve_ampCorr_vs_posY_BAR2')
g['COMB'] = f.Get('g_tResolGausSum_ampCorr_vs_posY')
g['COMBW1'] = f.Get('g_tResolGausAmpWSum_ampCorr_vs_posY')
g['COMBW2'] = f.Get('g_tResolGausResWSum_ampCorr_vs_posY')

gnorm = {}
gnorm['BAR0'] = ROOT.TGraphErrors()
gnorm['BAR1'] = ROOT.TGraphErrors()
gnorm['BAR2'] = ROOT.TGraphErrors()
gnorm['COMB'] = ROOT.TGraphErrors()
gnorm['COMBW'] = ROOT.TGraphErrors()
gnorm['COMBW1'] = ROOT.TGraphErrors()
gnorm['COMBW2'] = ROOT.TGraphErrors()

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


sigma0 = ROOT.Double(0)
sigma1 = ROOT.Double(0)
x0 = ROOT.Double(0)
x1 = ROOT.Double(0)
x = ROOT.Double(0)
s = ROOT.Double(0)
xw = ROOT.Double(0)
sw=  ROOT.Double(0)

ex0 = ROOT.Double(0)
ex1 = ROOT.Double(0)
ex = ROOT.Double(0)
es = ROOT.Double(0)
exw = ROOT.Double(0)
esw=  ROOT.Double(0)

g_exp = {}
g_exp['COMB'] = ROOT.TGraph()
g_exp['COMBW1'] = ROOT.TGraph()
g_exp['COMBW2'] = ROOT.TGraph()
g2_exp = {}
g2_exp['COMB'] = ROOT.TGraph()
g2_exp['COMBW1'] = ROOT.TGraph()
g2_exp['COMBW2'] = ROOT.TGraph()
n = 0

# BAR0-BAR1
for i in range(0,g['BAR0'].GetN()):
    sigma0 = ROOT.Double(0)
    sigma1 = ROOT.Double(0)
    x0 = ROOT.Double(0)
    x1 = ROOT.Double(0)
    g['BAR0'].GetPoint(i, x0, sigma0)
    last1 = g['BAR1'].GetN() 
    g['BAR1'].GetPoint(last1-1, x1, sigma1)
    if (x0 > x1): continue; 
    sigma1 =  g['BAR1'].Eval(x0)
    if (sigma0 > 0  and sigma1 > 0  ):
        sigma  =  0.5*math.sqrt(sigma0*sigma0+sigma1*sigma1)
        sigmaw = 1./math.sqrt(1./sigma0/sigma0+1./sigma1/sigma1)
        g_exp['COMB'].SetPoint(n, x0, sigma)
        g_exp['COMBW2'].SetPoint(n, x0, sigmaw)
        n=n+1

# BAR1-BAR2
n=0
for i in range(0,g['BAR1'].GetN()):
    sigma0 = ROOT.Double(0)
    sigma1 = ROOT.Double(0)
    x0 = ROOT.Double(0)
    x1 = ROOT.Double(0)
    g['BAR1'].GetPoint(i, x0, sigma0)
    last1 = g['BAR2'].GetN() 
    g['BAR2'].GetPoint(last1-1, x1, sigma1)
    if (x0 > x1): continue; 
    sigma1 =  g['BAR2'].Eval(x0)
    if (sigma0 > 0  and sigma1 > 0  ):
        sigma  =  0.5*math.sqrt(sigma0*sigma0+sigma1*sigma1)
        sigmaw = 1./math.sqrt(1./sigma0/sigma0+1./sigma1/sigma1)
        g2_exp['COMB'].SetPoint(n, x0, sigma)
        g2_exp['COMBW2'].SetPoint(n, x0, sigmaw)
        n=n+1

            

        
for i in ['BAR0', 'BAR1', 'BAR2','COMB', 'COMBW1', 'COMBW2']:
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
leg.AddEntry(g['COMB'],'unweighted average','PL')
leg.AddEntry(g['COMBW1'],'amplitude weighted average','PL')
leg.AddEntry(g['COMBW2'],'#sigma_{t} weighted average','PL')


# absolute time resolution
canvas = ROOT.TCanvas('bars_phiSharing_time_resolution_Vbias%s_thr%s_yzangle%s'%(Vbias, thr, angle),'bars_phiSharing_time_resolution_Vbias%s_thr%s_yzangle%s'%(Vbias, thr, angle),800,600)
canvas.SetGridy()
hdummy = ROOT.TH2F('','', 100, 20, 29.2, 100, 0, 140)
hdummy.GetYaxis().SetNdivisions(520)
hdummy.GetYaxis().SetTitle('time resolution (ps)')
hdummy.GetXaxis().SetTitle('y (mm)')
hdummy.Draw()
g['BAR0'].Draw('plsame')
g['BAR1'].Draw('plsame')
g['BAR2'].Draw('plsame')
g['COMB'].Draw('plsame')
g['COMBW1'].Draw('plsame')
g['COMBW2'].Draw('plsame')
for cmb in ['COMB','COMBW2']:
    g_exp[cmb].SetLineColor(color[cmb])    
    g_exp[cmb].SetLineStyle(2)    
    g_exp[cmb].SetLineWidth(4)    
    #g_exp[cmb].Draw('L SAME')
    g2_exp[cmb].SetLineColor(color[cmb])    
    g2_exp[cmb].SetLineStyle(2)    
    g2_exp[cmb].SetLineWidth(4)    
    #g2_exp[cmb].Draw('L SAME')

#leg.AddEntry(g_exp['COMB'],'average - expected','L')
#leg.AddEntry(g_exp['COMBW1'],'weighted average - expected','L')
leg.Draw('same')
tChType.Draw()

canvas.SaveAs(subfolder+'/'+canvas.GetName()+'.png')
canvas.SaveAs(subfolder+'/'+canvas.GetName()+'.pdf')



# normalize time resolution to normal incidence, calculate MCP resolution from tDiff vs tAve
tRes90  = {}
tRes45 = {}
tResRef90 = {}
tResRef45 = {}
for ch in ['BAR0','BAR1','BAR2']:
    # run at normal incidence
    hAve90 = fRef.Get('h_tAve_ampCorr_%s'%ch)
    funAve90 = hAve90.GetFunction('fitFunAve_ampCorr_%s'%ch)
    hDiff90 = fRef.Get('h_tDiff_ampCorr_posCorr_%s'%ch)
    funDiff90 = hDiff90.GetFunction('fitFunDiff_ampCorr_posCorr_%s'%ch)
    tResRef90[ch] = math.sqrt( funAve90.GetParameter(2)*funAve90.GetParameter(2) - funDiff90.GetParameter(2)*funDiff90.GetParameter(2)/4. )
    tRes90[ch]    = math.sqrt( funAve90.GetParameter(2)*funAve90.GetParameter(2) - tResRef90[ch]*tResRef90[ch])
    # run at 45 deg
    hAve45   = f.Get('h_tAve_ampCorr_%s'%ch)
    funAve45 = hAve45.GetFunction('fitFunAve_ampCorr_%s'%ch)
    hDiff45  = f.Get('h_tDiff_ampCorr_posCorr_%s'%ch)
    funDiff45 = hDiff45.GetFunction('fitFunDiff_ampCorr_posCorr_%s'%ch)
    tResRef45[ch] = math.sqrt( funAve45.GetParameter(2)*funAve45.GetParameter(2) - funDiff45.GetParameter(2)*funDiff45.GetParameter(2)/4. )
    tRes45[ch]    = math.sqrt( funAve45.GetParameter(2)*funAve45.GetParameter(2) - tResRef45[ch]*tResRef45[ch])
    print 'normal incidence:', funAve90.GetParameter(2), tRes90[ch],   tResRef90[ch] 
    print ' 45 deg         :', funAve45.GetParameter(2), tRes45[ch],   tResRef45[ch] 

tnorm = 1000.*(tRes90['BAR0'] + tRes90['BAR1'] + tRes90['BAR2']) / 3 # average time resolution of the three bars at normal incidence 
tnorm2 = 1000.*(tRes45['BAR0'] + tRes45['BAR1'] + tRes45['BAR2']) / 3 # average time resolution of the three bars at normal incidence 
sigmaRef90 = 1000.* ( tResRef90['BAR0'] + tResRef90['BAR1'] +  tResRef90['BAR2']) /3 
sigmaRef45 = 1000.* ( tResRef45['BAR0'] + tResRef45['BAR1'] +  tResRef45['BAR2']) /3 
print 'Estimated MCP time resolution (normal incidence) = %.01f ps'%sigmaRef90
print 'Estimated MCP time resolution (45 deg incidence) = %.01f ps'%sigmaRef45
print 'Time resolution at normal incidence = %.01f ps'%tnorm
print 'Time resolution at 45 deg incidence = %.01f ps'%tnorm2


for b in ['BAR0','BAR1','BAR2', 'COMB', 'COMBW1', 'COMBW2']:
    gnorm[b].SetMarkerStyle(markerstyle[b])
    gnorm[b].SetMarkerSize(1.0)
    gnorm[b].SetMarkerColor(color[b])
    gnorm[b].SetLineColor(color[b])
    for i in range(0,g[b].GetN()):
        x = ROOT.Double(0)
        sigma = ROOT.Double(0)
        g[b].GetPoint(i, x, sigma)
        if (sigma <= 0): continue
        #sigma = math.sqrt(sigma*sigma - sigmaRef45*sigmaRef45 + 30.*30.)
        #sigma = math.sqrt(sigma*sigma)
        xerr = g[b].GetErrorX(i)
        print tnorm, sigma
        sigmaerr =  sigma/tnorm * math.sqrt(pow(g[b].GetErrorY(i)/sigma,2) + pow(2./tnorm,2))
        gnorm[b].SetPoint(i, x, sigma/tnorm)
        gnorm[b].SetPointError(i, xerr, sigmaerr)
 
canvas2 = ROOT.TCanvas('bars_phiSharing_time_resolution_Vbias%s_thr%s_yzangle%s_normalized'%(Vbias, thr, angle),'bars_phiSharing_time_resolution_Vbias%s_thr%s_yzangle%s_normalized'%(Vbias, thr, angle),800,600)
canvas2.SetGridy()
hdummy2 = ROOT.TH2F('','', 100, 20, 29.2, 100, 0.0, 3.5)
hdummy2.GetYaxis().SetNdivisions(520)
hdummy2.GetYaxis().SetTitle('time resolution normalized to normal incidence')
hdummy2.GetXaxis().SetTitle('y (mm)')
hdummy2.Draw()
gnorm['BAR0'].Draw('plsame')
gnorm['BAR1'].Draw('plsame')
gnorm['BAR2'].Draw('plsame')
gnorm['COMB'].Draw('plsame')
gnorm['COMBW1'].Draw('plsame')
gnorm['COMBW2'].Draw('plsame')
leg.Draw('same')
tChType.Draw()

canvas2.SaveAs(subfolder+'/'+canvas2.GetName()+'.png')
canvas2.SaveAs(subfolder+'/'+canvas2.GetName()+'.pdf')

    
# fraction of events with signal above threshold in both bars
canvas3 = ROOT.TCanvas('bars_fraction_above_thr_phiSharing_Vbias%s_thr%s_yzangle%s'%(Vbias, thr, angle),'bars_fraction_above_thr_phiSharing_Vbias%s_thr%s_yzangle%s'%(Vbias, thr, angle),800,600)
canvas3.SetGridx()
canvas3.SetGridy()
g = f.Get('g_nEventsRatio_vs_posY')
g.GetXaxis().SetTitle('y (mm)')
g.GetYaxis().SetTitle('fraction of events above threshold')
g.GetYaxis().SetRangeUser(0, 1.2)
g.Draw('ap')

raw_input('ok?')



