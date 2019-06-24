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



channels = ['BAR%s'%i for i in range(0,1)]
vb = 72

f = {}
pR = {}
pL = {}
pRuncorr = {}
pLuncorr = {}
pAve = {}


for ch in channels:
    #fname = '../v4/output_1bar_Vbias43_thr1000ADC_4mm_binnedAmpWalkCorr.root'
    fname = '../v4_old/output_3bars_Vbias72_thr700ADC_xyangle14.root'
    #fname = '../v4/output_3bars_Vbias72_thr200ADC_xyangle90_runs6872-6913.root'
    f[ch] = ROOT.TFile.Open(fname)
    pRuncorr[ch] = f[ch].Get('p_tR_ampCorr_vs_posXc_%s'%ch)
    pLuncorr[ch] = f[ch].Get('p_tL_ampCorr_vs_posXc_%s'%ch)
    pAve[ch] = f[ch].Get('p_tAve_ampCorr_vs_posXc_%s'%ch)
    
tChType = ROOT.TLatex( 0.13, 0.92, 'LYSO:Ce 4x3x57 mm^{3} - HPK S12572-015')
tChType.SetNDC()
tChType.SetTextSize(0.035)



for ch in channels:

    pL[ch] = ROOT.TH1F('pL','pL',pLuncorr[ch].GetNbinsX(), -50, 50)
    pR[ch] = ROOT.TH1F('pR','pR',pRuncorr[ch].GetNbinsX(), -50, 50)

    ####################
    #time delay vs angle
    fpol2Ave = ROOT.TF1('fpol2Ave','pol4',-100,100)
    pAve[ch].Fit(fpol2Ave,'QRS')
    raw_input('ok?')

    for ibin in range(1, pLuncorr[ch].GetNbinsX()):
        x = pLuncorr[ch].GetBinCenter(ibin)
        if (pLuncorr[ch].GetBinContent(ibin) !=0):
            #cont = pLuncorr[ch].GetBinContent(ibin) - fpol2Ave.Eval(x) + fpol2Ave.Eval(  pLuncorr[ch].GetMean() )
            cont = pLuncorr[ch].GetBinContent(ibin)
            pL[ch].Fill(x, cont )
            pL[ch].SetBinError(ibin, pLuncorr[ch].GetBinError(ibin) )
            
    for ibin in range(1, pRuncorr[ch].GetNbinsX()):
        x = pRuncorr[ch].GetBinCenter(ibin)
        if (pRuncorr[ch].GetBinContent(ibin) !=0):
            #cont = pRuncorr[ch].GetBinContent(ibin) - fpol2Ave.Eval(x) + fpol2Ave.Eval(  pRuncorr[ch].GetMean() )
            cont = pRuncorr[ch].GetBinContent(ibin) 
            pR[ch].Fill(x, cont )
            pR[ch].SetBinError(ibin, pRuncorr[ch].GetBinError(ibin) )

        
    fpol1L = ROOT.TF1('fpol1L','pol1',-100,100)
    fpol1L.SetLineColor(ROOT.kBlue)
    x1 = pL[ch].GetMean(1) - 50
    x2 = pL[ch].GetMean(1) + 50
    fpol1L.SetRange(x1,x2)
    pL[ch].Fit(fpol1L,'QR')
    print 'slopeL = ', fpol1L.GetParameter(1)*1000
                  
    fpol1R = ROOT.TF1('fpol1R','pol1',-100,100)
    fpol1R.SetLineColor(ROOT.kRed)
    x1 = pR[ch].GetMean(1) - 50
    x2 = pR[ch].GetMean(1) + 50
    fpol1R.SetRange(x1,x2)
    pR[ch].Fit(fpol1R,'QR')
    print 'slopeR = ', fpol1R.GetParameter(1)*1000
   
    cc = ROOT.TCanvas('time_vs_posXc')
    cc.SetGridy()
    pL[ch].SetMarkerColor(ROOT.kBlue)
    pR[ch].SetMarkerColor(ROOT.kRed)
    pL[ch].SetLineColor(ROOT.kBlue)
    pR[ch].SetLineColor(ROOT.kRed)
    meany =  0.5*(pLuncorr[ch].GetMean(2)+pRuncorr[ch].GetMean(2) )
    pL[ch].GetYaxis().SetRangeUser(meany-1.0, meany+1.0)
    pL[ch].GetXaxis().SetRangeUser(pLuncorr[ch].GetMean()-2*pLuncorr[ch].GetRMS(),pLuncorr[ch].GetMean()+2*pLuncorr[ch].GetRMS() )
    pL[ch].GetXaxis().SetTitle('x (mm)')
    pL[ch].GetYaxis().SetTitle('time (ps)')
    pL[ch].Draw()
    pR[ch].Draw('same')
    #pAve[ch].Draw('same')
    #cc.SaveAs(outdir+'/'+cc.GetName()+'.png')
    #cc.SaveAs(outdir+'/'+cc.GetName()+'.pdf')


    raw_input('ok?')
