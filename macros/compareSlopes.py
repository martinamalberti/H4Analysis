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
ROOT.gStyle.SetLabelSize(0.04,'X')
ROOT.gStyle.SetLabelSize(0.04,'Y')
ROOT.gStyle.SetTitleSize(0.04,'X')
ROOT.gStyle.SetTitleSize(0.04,'Y')
ROOT.gStyle.SetTitleOffset(1.1,'X')
ROOT.gStyle.SetTitleOffset(1.2,'Y')

channels = ['BAR%s'%i for i in range(0,3)]

runranges =  ['6369-6401',
              '6872-6913',
              '7498-7516',
              '7639-7679',
              '7850-7877',
              '7404-7497',
              '7682-7800',
              '7881-7953',
              '8173-8261',
              '8335-8443',
              ]


f = {}
pL   = {}
pR   = {}
pRef = {}

for i,r in enumerate(runranges):
    if (i < 5):
        f[r] = ROOT.TFile.Open('../v4/output_3bars_Vbias72_thr200ADC_xyangle90_runs%s.root'%(r))
    elif (i==5):
        f[r] = ROOT.TFile.Open('../v4/output_3bars_Vbias72_thr200ADC_xyangle60.root')
    elif (i==6):
        f[r] = ROOT.TFile.Open('../v4/output_3bars_Vbias72_thr200ADC_xyangle45.root')
    elif (i==7):
        f[r] = ROOT.TFile.Open('../v4/output_3bars_Vbias72_thr200ADC_xyangle30.root')
    elif (i==8):
        f[r] = ROOT.TFile.Open('../v4/output_3bars_Vbias72_thr200ADC_xyangle20.root')
    elif (i==9):
        f[r] = ROOT.TFile.Open('../v4/output_3bars_Vbias72_thr200ADC_xyangle14.root')

tChType = ROOT.TLatex( 0.13, 0.92, 'LYSO:Ce 3x3x50 mm^{3} - HPK S12572-015')
tChType.SetNDC()
tChType.SetTextSize(0.035)

g_slopeRef_vs_run = {}
g_slopeR_vs_run = {}
g_slopeL_vs_run = {}
g_sumSlope_vs_run = {}
g_deltaSlope_vs_run = {}
canvas = {}
hdummy = {}
canvas2 = {}
hdummy2 = {}

for ch in channels:
    pRef[ch] = {}
    pR[ch] = {}
    pL[ch] = {}
    
    g_slopeRef_vs_run[ch] = ROOT.TGraphErrors()
    g_slopeR_vs_run[ch] = ROOT.TGraphErrors()
    g_slopeL_vs_run[ch] = ROOT.TGraphErrors()
    g_sumSlope_vs_run[ch] = ROOT.TGraphErrors()
    g_deltaSlope_vs_run[ch] = ROOT.TGraphErrors()

    
    for i,r in enumerate(runranges):
        pRef[ch][r] = f[r].Get('p_timeRef_vs_posX')
        fpol1 = ROOT.TF1('fpol1','pol1', -100, 100)
        pRef[ch][r].Fit(fpol1,'Q')
        sRef = fpol1.GetParameter(1)*1000
        sRefErr = fpol1.GetParError(1)*1000
        g_slopeRef_vs_run[ch].SetPoint(i, i, abs(sRef))
        g_slopeRef_vs_run[ch].SetPointError(i, 0.5, sRefErr)
        #print r
        #raw_input('ok?')
        pR[ch][r] = f[r].Get('p_tR_ampCorr_vs_posXc_%s'%ch)
        #pR[ch][r] = f[r].Get('p_tR_vs_posX_%s'%ch)
        pR[ch][r].Fit(fpol1,'Q')
        sR = fpol1.GetParameter(1)*1000
        sRerr = fpol1.GetParError(1)*1000
        g_slopeR_vs_run[ch].SetPoint(i, i, sR)
        g_slopeR_vs_run[ch].SetPointError(i, 0.5, sRerr)

        pL[ch][r] = f[r].Get('p_tL_ampCorr_vs_posXc_%s'%ch)
        #pL[ch][r] = f[r].Get('p_tL_vs_posX_%s'%ch)
        pL[ch][r].Fit(fpol1,'Q')
        sL = fpol1.GetParameter(1)*1000
        sLerr = fpol1.GetParError(1)*1000
        g_slopeL_vs_run[ch].SetPoint(i, i, sL)
        g_slopeL_vs_run[ch].SetPointError(i, 0.5, sLerr)


        print r, sRef, ( (sL + sR)/2 )
        print 'slopeR, slopeL, true slope =', sR, sL, (sL-sR)/2

        err = math.sqrt( sLerr*sLerr + sRerr*sRerr)/2
        g_deltaSlope_vs_run[ch].SetPoint(i, i, (sL - sR) / 2 )
        g_deltaSlope_vs_run[ch].SetPointError(i, 0.5, err)

        g_sumSlope_vs_run[ch].SetPoint(i, i, (sL + sR)/2 )
        g_sumSlope_vs_run[ch].SetPointError(i, 0.5, err )
        
               
    g_slopeR_vs_run[ch].SetMarkerColor(ROOT.kRed)
    g_slopeR_vs_run[ch].SetLineColor(ROOT.kRed)

    g_slopeL_vs_run[ch].SetMarkerColor(ROOT.kBlue)
    g_slopeL_vs_run[ch].SetLineColor(ROOT.kBlue)

    g_deltaSlope_vs_run[ch].SetMarkerColor(ROOT.kGreen+1)
    g_deltaSlope_vs_run[ch].SetLineColor(ROOT.kGreen+1)

    g_sumSlope_vs_run[ch].SetMarkerStyle(24)
    
    canvas[ch] = ROOT.TCanvas('c_slope_vs_runrange_%s'%ch,'c_slope_vs_runrange_%s'%ch )
    canvas[ch].SetGridx()
    canvas[ch].SetGridy()
    #hdummy[ch] = ROOT.TH2F('hdummy%s'%ch,'hdummy%s'%ch, 5, -0.5, 4.5, 100, -30, 30)
    hdummy[ch] = ROOT.TH2F('hdummy%s'%ch,'hdummy%s'%ch, 10, -0.5, 9.5, 100, -30, 30)
    for ibin,r in enumerate(runranges):
        hdummy[ch].GetXaxis().SetBinLabel(ibin+1, r)
    hdummy[ch].GetYaxis().SetTitle('time delay (ps/mm)')
    hdummy[ch].Draw()
    g_slopeRef_vs_run[ch].Draw('psame')
    g_slopeR_vs_run[ch].Draw('psame')
    g_slopeL_vs_run[ch].Draw('psame')
    g_deltaSlope_vs_run[ch].Draw('psame')
    g_sumSlope_vs_run[ch].Draw('psame')

    canvas[ch].SaveAs(canvas[ch].GetName()+'.pdf')
    
    raw_input('ok?')
