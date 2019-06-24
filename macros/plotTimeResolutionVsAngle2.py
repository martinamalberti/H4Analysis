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


angles0 = [90, 60, 53, 45, 37, 30, 20, 14]
angles  = [90-a for a in angles0]
print angles
channels = ['BAR%s'%i for i in range(0,3)]
thresholds = [50, 100, 200, 300, 400, 500, 700, 1000, 2000]
vb = 72

f = {}
gAve = {}
gDiff = {}
gL = {}
gR = {}
h_ampR = {}
h_ampL = {}

sigmatAve = {} #[angle][th][channel]
sigmatAve_err = {} #[angle][th][channel]
sigmaPTK = 0.015

sigmatDiff = {} #[angle][th][channel]
sigmatDiff_err = {} #[angle][th][channel]

# get time resolution for optimal threshold
for angle in angles:
    sigmatAve[angle] = {}
    sigmatAve_err[angle] = {}
    sigmatDiff[angle] = {}
    sigmatDiff_err[angle] = {}
    for th in thresholds:
        sigmatAve[angle][th] = {}
        sigmatAve_err[angle][th] = {}
        sigmatDiff[angle][th] = {}
        sigmatDiff_err[angle][th] = {}
        fname = '../v1/output_3bars_Vbias%s_thr%sADC_xyangle%s.root'%(vb, th, 90-angle)
        if (os.path.isfile(fname) ==  False): continue
        ff = ROOT.TFile.Open(fname)
        for channel in channels:
            # tAve
            h = ff.Get('h_tAve_ampCorr_posCorr_'+ channel)
            fitfun = h.GetFunction('fitFunAve_ampCorr_posCorr_%s'%channel)
            if (fitfun == None):
                continue
            sigmaGaus  = fitfun.GetParameter(2)
            esigmaGaus = fitfun.GetParError(2)
            resolGaus  = ROOT.TMath.Sqrt(sigmaGaus*sigmaGaus - sigmaPTK*sigmaPTK)*1000. # ps
            sigmatAve[angle][th][channel] = resolGaus
            sigmatAve_err[angle][th][channel] = max(esigmaGaus*1000., 2.)

            # tDiff
            h = ff.Get('h_tDiff_ampCorr_posCorr_'+ channel)
            fitfun = h.GetFunction('fitFunDiff_ampCorr_posCorr_%s'%channel)
            if (fitfun == None):
                continue
            sigmaGaus  = fitfun.GetParameter(2)
            esigmaGaus = fitfun.GetParError(2)
            resolGaus  = sigmaGaus*1000. # ps
            sigmatDiff[angle][th][channel] = resolGaus/2
            sigmatDiff_err[angle][th][channel] = max(esigmaGaus*1000., 2.)

bestThresholdAve = {} # [angle][channel]
bestThresholdDiff = {} # [angle][channel]

for angle in angles:
    bestThresholdAve[angle] = {}
    bestThresholdDiff[angle] = {}
    for channel in channels:
        l = [ sigmatAve[angle][th][channel] for th in thresholds ]
        bestIndexAve = l.index(min(l))
        bestThresholdAve[angle][channel] = thresholds[bestIndexAve]
        ll = [ sigmatDiff[angle][th][channel] for th in thresholds ]
        bestIndexDiff = ll.index(min(ll))
        bestThresholdDiff[angle][channel] = thresholds[bestIndexDiff]
        
        
for angle in angles:
    f[angle] = ROOT.TFile.Open('../v1/output_3bars_Vbias72_thr%dADC_xyangle%s.root'%(bestThresholdDiff[angle][channel], 90-angle))
    gL[angle] = {}
    gR[angle] = {}
    for ch in channels:
        gR[angle][ch] = f[angle].Get('g_tResolGausR_ampCorr_%s'%ch)
        gL[angle][ch] = f[angle].Get('g_tResolGausL_ampCorr_%s'%ch)
    
tChType = ROOT.TLatex( 0.13, 0.92, 'LYSO:Ce 3x3x50 mm^{3} - HPK S12572-015')
tChType.SetNDC()
tChType.SetTextSize(0.035)

gAve_vs_angle = {}
gDiff_vs_angle = {}
gR_vs_angle = {}
gL_vs_angle = {}

gAve_vs_slantTickness = {}
gDiff_vs_slantTickness = {}
gR_vs_slantTickness = {}
gL_vs_slantTickness = {}

tResL = {}
tResR = {}
tResAve = {}
tResDiff = {}
slantTickness = {}

err = 2.

for ch in channels:
    gAve_vs_angle[ch] = ROOT.TGraphErrors()
    gDiff_vs_angle[ch] = ROOT.TGraphErrors()
    gR_vs_angle[ch] = ROOT.TGraphErrors()
    gL_vs_angle[ch] = ROOT.TGraphErrors()
    
    gDiff_vs_slantTickness[ch] = ROOT.TGraphErrors()
    gAve_vs_slantTickness[ch] = ROOT.TGraphErrors()
    gR_vs_slantTickness[ch] = ROOT.TGraphErrors()
    gL_vs_slantTickness[ch] = ROOT.TGraphErrors()


    tResL[ch] = {}
    tResR[ch] = {}
    tResAve[ch] = {}
    tResDiff[ch] = {}
    slantTickness[ch] = {}

    for i, angle in enumerate(angles):
        tResL[ch][angle] = gL[angle][ch].GetFunction(gL[angle][ch].GetName().replace('g_tResolGaus','fpol0_t') ).GetParameter(0)
        tResR[ch][angle] = gR[angle][ch].GetFunction(gR[angle][ch].GetName().replace('g_tResolGaus','fpol0_t') ).GetParameter(0)
        #tResAve[ch][angle] = gAve[angle][ch].GetFunction(gAve[angle][ch].GetName().replace('g_tResolGaus','fpol0_t') ).GetParameter(0)
        #tResDiff[ch][angle] = gDiff[angle][ch].GetFunction(gDiff[angle][ch].GetName().replace('g_tResolGaus','fpol0_t') ).GetParameter(0)/2

        thr = bestThresholdAve[angle][ch]
        tResAve[ch][angle] = sigmatAve[angle][thr][ch]
        thr = bestThresholdDiff[angle][ch]
        tResDiff[ch][angle] = sigmatDiff[angle][thr][ch]
        
        # graphs vs angle
        gAve_vs_angle[ch].SetPoint(i, angle, tResAve[ch][angle])
        gDiff_vs_angle[ch].SetPoint(i, angle, tResDiff[ch][angle])
        gL_vs_angle[ch].SetPoint(i, angle, tResL[ch][angle])
        gR_vs_angle[ch].SetPoint(i, angle, tResR[ch][angle])
        
        gAve_vs_angle[ch].SetPointError(i, 0, err )
        gDiff_vs_angle[ch].SetPointError(i, 0, err )
        gL_vs_angle[ch].SetPointError(i, 0, err)
        gR_vs_angle[ch].SetPointError(i, 0, err)
                
        # graphs vs slantTickness
        slantTickness[angle] = 3. / math.cos((angle) * math.pi / 180.)
        
        gAve_vs_slantTickness[ch].SetPoint(i, slantTickness[angle], tResAve[ch][angle])
        gDiff_vs_slantTickness[ch].SetPoint(i, slantTickness[angle], tResDiff[ch][angle])
        gL_vs_slantTickness[ch].SetPoint(i, slantTickness[angle], tResL[ch][angle])
        gR_vs_slantTickness[ch].SetPoint(i, slantTickness[angle], tResR[ch][angle])
        
        gAve_vs_slantTickness[ch].SetPointError(i, 0, err )
        gDiff_vs_slantTickness[ch].SetPointError(i, 0, err )
        gL_vs_slantTickness[ch].SetPointError(i, 0, err)
        gR_vs_slantTickness[ch].SetPointError(i, 0, err)
        
for ch in channels:
    for g in gDiff_vs_angle[ch], gDiff_vs_slantTickness[ch]:
        g.SetMarkerStyle(20)
        g.SetMarkerColor(ROOT.kGray)
        g.SetLineColor(ROOT.kGray)
        
    for g in gAve_vs_angle[ch], gAve_vs_slantTickness[ch]:
        g.SetMarkerStyle(20)
        g.SetMarkerColor(1)
        g.SetLineColor(1)
        
    for g in gL_vs_angle[ch], gL_vs_slantTickness[ch]:
        g.SetMarkerStyle(20)
        g.SetMarkerColor(ROOT.kBlue)
        g.SetLineColor(ROOT.kBlue)

    for g in gR_vs_angle[ch], gR_vs_slantTickness[ch]:
        g.SetMarkerStyle(20)
        g.SetMarkerColor(ROOT.kRed)
        g.SetLineColor(ROOT.kRed)


leg = ROOT.TLegend(0.7,0.7,0.89,0.89)
leg.AddEntry(gL_vs_angle['BAR0'],'t_{upstream}','PL')
leg.AddEntry(gR_vs_angle['BAR0'],'t_{downstream}','PL')
leg.AddEntry(gAve_vs_angle['BAR0'],'t_{average}','PL')
leg.AddEntry(gDiff_vs_angle['BAR0'],'t_{diff}','PL')

canvas = {}
canvas2 = {}

hdummy = {}
hdummy2 = {}

fitL = {}
fitR = {}
fitAve = {}
fitDiff = {}

for ch in channels:

    # vs theta
    canvas[ch] = ROOT.TCanvas('timeResolution_vs_angle_%s'%ch,'timeResolution_vs_angle_%s'%ch)
    canvas[ch].SetGridy()
    hdummy[ch] = ROOT.TH2F('hdummy_%s'%ch,'',91,-1,90,80,0,80)
    hdummy[ch].Draw()
    hdummy[ch].GetXaxis().SetTitle('#theta (#circ)')
    hdummy[ch].GetYaxis().SetTitle('time resolution (ps)')
    gDiff_vs_angle[ch].Draw('psame')
    gAve_vs_angle[ch].Draw('psame')
    gL_vs_angle[ch].Draw('psame')
    gR_vs_angle[ch].Draw('psame')
    leg.Draw('same')
    tChType.Draw()

    # vs slant tickness
    canvas2[ch] = ROOT.TCanvas('timeResolution_vs_slantTickness_%s'%ch,'timeResolution_vs_slantTickness_%s'%ch)
    canvas2[ch].SetGridy()
    hdummy2[ch] = ROOT.TH2F('hdummy2_%s'%ch,'',26,0,16,80,0,80)
    hdummy2[ch].Draw()
    hdummy2[ch].GetXaxis().SetTitle('slant tickness (mm)')
    hdummy2[ch].GetYaxis().SetTitle('time resolution (ps)')
    gDiff_vs_slantTickness[ch].Draw('psame')
    gAve_vs_slantTickness[ch].Draw('psame')
    #gL_vs_slantTickness[ch].Draw('psame')
    #gR_vs_slantTickness[ch].Draw('psame')
    leg.Draw('same')
    tChType.Draw()
    
    fitL[ch] = ROOT.TF1('fitL','[1]*1./pow(x,[0])',0,100)
    fitL[ch].SetLineColor(ROOT.kBlue)
    fitL[ch].SetParameter(0, 0.5)
    gL_vs_slantTickness[ch].Fit(fitL[ch],'Q')
    print fitL[ch].GetParameter(0), fitL[ch].GetParError(0)
    
    fitR[ch] = ROOT.TF1('fitR','[1]*1./pow(x,[0])',0,100)
    fitR[ch].SetLineColor(ROOT.kRed)
    fitR[ch].SetParameter(0, 0.5)
    gR_vs_slantTickness[ch].Fit(fitR[ch],'Q')
    print fitR[ch].GetParameter(0), fitR[ch].GetParError(0)
    
    fitAve[ch] = ROOT.TF1('fitAve','[1]*1./pow(x,[0])',0,100)
    fitAve[ch].SetLineColor(1)
    fitAve[ch].SetParameter(0, 0.5)
    gAve_vs_slantTickness[ch].Fit(fitAve[ch],'Q')
    print fitAve[ch].GetParameter(0), fitAve[ch].GetParError(0)

    fitDiff[ch] = ROOT.TF1('fitDiff','[1]*1./pow(x,[0])',0,100)
    fitDiff[ch].SetLineColor(1)
    fitDiff[ch].SetParameter(0, 0.5)
    gDiff_vs_slantTickness[ch].Fit(fitDiff[ch],'Q')
    print fitDiff[ch].GetParameter(0), fitDiff[ch].GetParError(0)

    print 'Channel %s'%ch

    print 'Time resolution vs slant tickness parametrized as [1]*1./pow(x,[0])'
    print 'right  : [0] = %.2f +/- %.2f'%(fitR[ch].GetParameter(0), fitR[ch].GetParError(0))
    print '         [1] = %.2f +/- %.2f'%(fitR[ch].GetParameter(1), fitR[ch].GetParError(1))
    
    print 'left   : [0] = %.2f +/- %.2f'%(fitL[ch].GetParameter(0), fitL[ch].GetParError(0))
    print '         [1] = %.2f +/- %.2f'%(fitL[ch].GetParameter(1), fitL[ch].GetParError(1))
    
    print 'average: [0] = %.2f +/- %.2f'%(fitAve[ch].GetParameter(0), fitAve[ch].GetParError(0))
    print '         [1] = %.2f +/- %.2f'%(fitAve[ch].GetParameter(1), fitAve[ch].GetParError(1))
    
    print 'diff   : [0] = %.2f +/- %.2f'%(fitDiff[ch].GetParameter(0), fitDiff[ch].GetParError(0))
    print '         [1] = %.2f +/- %.2f'%(fitDiff[ch].GetParameter(1), fitDiff[ch].GetParError(1))
    
    
    for c in canvas[ch], canvas2[ch]:
        c.SaveAs(c.GetName()+'.png')
        c.SaveAs(c.GetName()+'.pdf')

    #raw_input('ok?')
