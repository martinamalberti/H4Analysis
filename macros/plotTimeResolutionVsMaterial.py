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
ROOT.gErrorIgnoreLevel = ROOT.kWarning;


inDir = '../v11/materialLeveling/barCenter/'
outdir = sys.argv[1]

#ampCorrType = 'ampCorr'
ampCorrType = 'pulseIntCorr'


if (outdir != './'):
    os.system('mkdir %s'%outdir)
print 'Saving plots in %s'%outdir
shutil.copy('index.php', '%s'%outdir)

thickness = [2, 3, 4]
channels = ['BAR%s'%i for i in range(0,1)]
thresholds = [50, 100, 150, 200, 250, 300, 350, 400, 450, 500, 600, 700, 800, 900, 1000, 1500]
vb = 43

f = {}
gAve = {}
gDiff = {}
gL = {}
pR = {}
pL = {}
gR = {}
h_ampR = {}
h_ampL = {}
h2_ampR_vs_posXc = {}
h2_ampL_vs_posXc = {}

sigmatAve = {} #[angle][th][channel]
sigmatAve_err = {} #[angle][th][channel]
sigmaPTK = 0.0123 # after corrections

sigmatDiff = {} #[angle][th][channel]
sigmatDiff_err = {} #[angle][th][channel]

# get time resolution for optimal threshold
for t in thickness:
    sigmatAve[t] = {}
    sigmatAve_err[t] = {}
    sigmatDiff[t] = {}
    sigmatDiff_err[t] = {}
    for th in thresholds:
        sigmatAve[t][th] = {}
        sigmatAve_err[t][th] = {}
        sigmatDiff[t][th] = {}
        sigmatDiff_err[t][th] = {}
        fname = '%s/output_1bar_Vbias%s_thr%sADC_%smm.root'%(inDir,vb, th, t)
        if (os.path.isfile(fname) ==  False):
            print 'File %s', fname, 'not found!'
            continue
        ff = ROOT.TFile.Open(fname)
        for channel in channels:
            # tAve
            sigmaEff = ff.Get('h_effectiveSigmaAve_%s_%s'%(ampCorrType, channel)).GetMean()
            h = ff.Get('h_tAve_%s_%s'%(ampCorrType, channel))
            fitfun = h.GetFunction('fitFunAve_%s_%s'%(ampCorrType, channel))
            if (fitfun == None):
                continue
            sigmaGaus  = fitfun.GetParameter(2)
            esigmaGaus = fitfun.GetParError(2)
            resolGaus  = ROOT.TMath.Sqrt(sigmaGaus*sigmaGaus - sigmaPTK*sigmaPTK)*1000. # ps
            if (math.isnan(resolGaus) or resolGaus < 15.): resolGaus = ROOT.TMath.Sqrt(sigmaEff*sigmaEff - sigmaPTK*sigmaPTK)*1000. # ps
            sigmatAve[t][th][channel] = resolGaus
            sigmatAve_err[t][th][channel] = esigmaGaus*1000.

            # tDiff
            h = ff.Get('h_tDiff_%s_posCorr_%s'%(ampCorrType, channel))
            fitfun = h.GetFunction('fitFunDiff_%s_posCorr_%s'%(ampCorrType, channel))
            if (fitfun == None):
                continue
            sigmaGaus  = fitfun.GetParameter(2)
            esigmaGaus = fitfun.GetParError(2)
            resolGaus  = sigmaGaus*1000. # ps
            sigmatDiff[t][th][channel] = resolGaus/2
            sigmatDiff_err[t][th][channel] = esigmaGaus/2*1000.

bestThresholdAve = {} # [t][channel]
bestThresholdDiff = {} # [t][channel]

for t in thickness:
    bestThresholdAve[t] = {}
    bestThresholdDiff[t] = {}
    for ch in channels:
        l = [ sigmatAve[t][th][ch] for th in thresholds ]
        bestIndexAve = l.index(min(l))
        bestThresholdAve[t][ch] = thresholds[bestIndexAve]
        ll = [ sigmatDiff[t][th][ch] for th in thresholds ]
        bestIndexDiff = ll.index(min(ll))
        bestThresholdDiff[t][ch] = thresholds[bestIndexDiff]
        print ch, t,  bestThresholdAve[t][ch], bestThresholdDiff[t][ch]
        #bestThresholdAve[t][ch] = 1000
        #bestThresholdDiff[t][ch] = 1000

        
for t in thickness:
    f[t] = {}
    gL[t] = {}
    gR[t] = {}
    pL[t] = {}
    pR[t] = {}
    h_ampL[t] = {}
    h_ampR[t] = {}
    h2_ampL_vs_posXc[t] = {}
    h2_ampR_vs_posXc[t] = {}
    for ch in channels:
        fname = '%s/output_1bar_Vbias43_thr%dADC_%smm.root'%(inDir,bestThresholdDiff[t][ch], t)
        f[t][ch] = ROOT.TFile.Open(fname)
        gR[t][ch] = f[t][ch].Get('g_tResolGausR_%s_%s'%(ampCorrType, ch))
        gL[t][ch] = f[t][ch].Get('g_tResolGausL_%s_%s'%(ampCorrType, ch))
        pR[t][ch] = f[t][ch].Get('p_tR_%s_vs_posXc_%s'%(ampCorrType, ch))
        pL[t][ch] = f[t][ch].Get('p_tL_%s_vs_posXc_%s'%(ampCorrType, ch))
        h_ampR[t][ch] = f[t][ch].Get('h_ampR_%s'%ch)
        h_ampL[t][ch] = f[t][ch].Get('h_ampL_%s'%ch)
        h2_ampR_vs_posXc[t][ch] = f[t][ch].Get('h2_ampR_vs_posXc_%s'%ch)
        h2_ampL_vs_posXc[t][ch] = f[t][ch].Get('h2_ampL_vs_posXc_%s'%ch)
    
tChType = ROOT.TLatex( 0.13, 0.92, 'LYSO:Ce 3xtx57 mm^{3} - FBK NUV-HD-TE 5x5 mm^{2}')
tChType.SetNDC()
tChType.SetTextSize(0.035)


g_ampL_vs_thickness = {}
g_ampR_vs_thickness = {}

gAve_vs_thickness = {}
gDiff_vs_thickness = {}
gR_vs_thickness = {}
gL_vs_thickness = {}

gDelayL_vs_thickness = {}
gDelayR_vs_thickness = {}

gAmpSlopeL_vs_thickness = {}
gAmpSlopeR_vs_thickness = {}



ampR = {}
ampL = {}

tResL = {}
tResR = {}
tResAve = {}
tResDiff = {}

slopeL = {}
slopeR = {}

eslopeL = {}
eslopeR = {}

err = 0.
errAve = 0.
errDiff = 0.

for ch in channels:

    gAve_vs_thickness[ch] = ROOT.TGraphErrors()
    gDiff_vs_thickness[ch] = ROOT.TGraphErrors()
    gR_vs_thickness[ch] = ROOT.TGraphErrors()
    gL_vs_thickness[ch] = ROOT.TGraphErrors()
    gDelayR_vs_thickness[ch] = ROOT.TGraphErrors()
    gDelayL_vs_thickness[ch] = ROOT.TGraphErrors()
    gAmpSlopeR_vs_thickness[ch] = ROOT.TGraphErrors()
    gAmpSlopeL_vs_thickness[ch] = ROOT.TGraphErrors()
    g_ampR_vs_thickness[ch] = ROOT.TGraphErrors()
    g_ampL_vs_thickness[ch] = ROOT.TGraphErrors()
    
    tResL[ch] = {}
    tResR[ch] = {}
    tResAve[ch] = {}
    tResDiff[ch] = {}
    slopeL[ch] = {}
    slopeR[ch] = {}
    eslopeL[ch] = {}
    eslopeR[ch] = {}
 
    for i, t in enumerate(thickness):
  
        #################
        # time resolution
        tResL[ch][t] = gL[t][ch].GetFunction(gL[t][ch].GetName().replace('g_tResolGaus','fpol0_t') ).GetParameter(0)
        tResR[ch][t] = gR[t][ch].GetFunction(gR[t][ch].GetName().replace('g_tResolGaus','fpol0_t') ).GetParameter(0)

        thr = bestThresholdDiff[t][ch]
        tResDiff[ch][t] = sigmatDiff[t][thr][ch]
        errDiff = math.sqrt(sigmatDiff_err[t][thr][ch]*sigmatDiff_err[t][thr][ch] + err * err)
        print ch, t, thr,  tResDiff[ch][t]
        
        thr = bestThresholdAve[t][ch]
        tResAve[ch][t] = sigmatAve[t][thr][ch]
        errAve = math.sqrt(sigmatAve_err[t][thr][ch]*sigmatAve_err[t][thr][ch] + err * err)
        
        # graphs time resol vs thickness
        gAve_vs_thickness[ch].SetPoint(i, t, tResAve[ch][t])
        gDiff_vs_thickness[ch].SetPoint(i, t, tResDiff[ch][t])
        gL_vs_thickness[ch].SetPoint(i, t, tResL[ch][t])
        gR_vs_thickness[ch].SetPoint(i, t, tResR[ch][t])

        gAve_vs_thickness[ch].SetPointError(i, 0., errAve )
        gDiff_vs_thickness[ch].SetPointError(i, 0., errDiff )
        gL_vs_thickness[ch].SetPointError(i, 0., err)
        gR_vs_thickness[ch].SetPointError(i, 0., err)
                

        ####################
        #time delay vs thickness
        fpol1L = ROOT.TF1('fpol1L','pol1',-100,100)
        fpol1L.SetLineColor(ROOT.kBlue)
        x1 = pL[t][ch].GetMean(1) - 50
        x2 = pL[t][ch].GetMean(1) + 50
        fpol1L.SetRange(x1,x2)
        pL[t][ch].Draw()
        pL[t][ch].Fit(fpol1L,'QR')
        slopeL[ch][t] = fpol1L.GetParameter(1)*1000.
        eslopeL[ch][t] = fpol1L.GetParError(1)*1000.
        if (fpol1L.GetChisquare()/fpol1L.GetNDF()>20):
            pL[t][ch].Fit(fpol1L,'QRW')
            slopeL[ch][t] = fpol1L.GetParameter(1)*1000.
            eslopeL[ch][t] = 0.2

                
        fpol1R = ROOT.TF1('fpol1R','pol1',-100,100)
        fpol1R.SetLineColor(ROOT.kRed)
        x1 = pR[t][ch].GetMean(1) - 50
        x2 = pR[t][ch].GetMean(1) + 50
        fpol1R.SetRange(x1,x2)
        pR[t][ch].Fit(fpol1R,'QR')
        slopeR[ch][t] = fpol1R.GetParameter(1)*1000.
        eslopeR[ch][t] = fpol1R.GetParError(1)*1000.
        if (fpol1R.GetChisquare()/fpol1R.GetNDF()>20):
            pR[t][ch].Fit(fpol1R,'QRW')
            slopeR[ch][t] = fpol1R.GetParameter(1)*1000.
            eslopeR[ch][t] = 0.2

 
        cc = ROOT.TCanvas('time_vs_posXc_%smm_%s'%(t,ch))
        #cc.SetGridy()
        pL[t][ch].SetMarkerColor(ROOT.kBlue)
        pR[t][ch].SetMarkerColor(ROOT.kRed)
        pL[t][ch].SetLineColor(ROOT.kBlue)
        pR[t][ch].SetLineColor(ROOT.kRed)
        meany =  0.5*(pL[t][ch].GetMean(2)+pR[t][ch].GetMean(2) )
        pL[t][ch].GetYaxis().SetRangeUser(meany-1.0, meany+1.0)
        pL[t][ch].GetXaxis().SetRangeUser(pL[t][ch].GetMean()-2*pL[t][ch].GetRMS(),pL[t][ch].GetMean()+2*pL[t][ch].GetRMS() )
        pL[t][ch].GetXaxis().SetTitle('x (mm)')
        pL[t][ch].GetYaxis().SetTitle('time (ps)')
        pL[t][ch].Draw()
        pR[t][ch].Draw('same')
        cc.SaveAs(outdir+'/'+cc.GetName()+'.png')
        cc.SaveAs(outdir+'/'+cc.GetName()+'.pdf')
          
        gDelayL_vs_thickness[ch].SetPoint( i, t, (slopeL[ch][t]))
        gDelayL_vs_thickness[ch].SetPointError( i, 0, eslopeL[ch][t] )
        
        gDelayR_vs_thickness[ch].SetPoint( i, t, (slopeR[ch][t]) )
        gDelayR_vs_thickness[ch].SetPointError( i, 0, eslopeR[ch][t] )

        #############################
        # amplitude vs slant tickness
        fLandau = ROOT.TF1('fLandau','landau', 0, 1)
        fLandau.SetRange(0,0.1)
        h_ampR[t][ch].Fit(fLandau,'QR')
        fLandau.SetRange(fLandau.GetParameter(1)*0.9,fLandau.GetParameter(1)*2)
        h_ampR[t][ch].Fit(fLandau,'QR')
        mpvR = fLandau.GetParameter(1)
        g_ampR_vs_thickness[ch].SetPoint( i, t, fLandau.GetParameter(1))
        g_ampR_vs_thickness[ch].SetPointError( i, 0, fLandau.GetParError(1) )
        h_ampL[t][ch].Fit(fLandau,'QR')
        fLandau.SetRange(fLandau.GetParameter(1)*0.9,fLandau.GetParameter(1)*2)
        h_ampL[t][ch].Fit(fLandau,'QR')
        mpvL = fLandau.GetParameter(1)
        g_ampL_vs_thickness[ch].SetPoint( i, t, fLandau.GetParameter(1))
        g_ampL_vs_thickness[ch].SetPointError( i, 0, fLandau.GetParError(1) )

        raw_input('ok?')
        
        ##########################
        # amplitude slope vs angle
        fLandau.SetParameter(1, h2_ampR_vs_posXc[t][ch].GetMean(2))
        fLandau.SetParameter(0, 0.01)
        hsliceR = ROOT.TObjArray()
        h2_ampR_vs_posXc[t][ch].FitSlicesY(fLandau,1,-1, 0, 'QNG2', hsliceR)
        fpol1R.SetRange(-100, 100)
        hsliceR[1].Fit(fpol1R,'QR')
        ampSlopeR = fpol1R.GetParameter(1)/mpvR * 100
        ampSlopeR_err = fpol1R.GetParError(1)/mpvR *100

        fLandau.SetParameter(1, h2_ampL_vs_posXc[t][ch].GetMean(2))
        fLandau.SetParameter(0, 0.01)
        hsliceL = ROOT.TObjArray()
        h2_ampL_vs_posXc[t][ch].FitSlicesY(fLandau,1,-1, 0, 'QRG2', hsliceL)
        fpol1L.SetRange(-100, 100)
        hsliceL[1].Fit(fpol1L,'QR')
        ampSlopeL = fpol1L.GetParameter(1)/mpvL * 100
        ampSlopeL_err = fpol1L.GetParError(1)/mpvL *100

        gAmpSlopeL_vs_thickness[ch].SetPoint( i, t, ampSlopeL)
        gAmpSlopeL_vs_thickness[ch].SetPointError( i, 0, ampSlopeL_err )
        
        gAmpSlopeR_vs_thickness[ch].SetPoint( i, t, ampSlopeR )
        gAmpSlopeR_vs_thickness[ch].SetPointError( i, 0, ampSlopeR_err) 

        
        ccc = ROOT.TCanvas('amplitude_vs_posXc_%smm_%s'%(t,ch))
        #ccc.SetGridy()
        hsliceL[1].SetMarkerColor(ROOT.kBlue)
        hsliceR[1].SetMarkerColor(ROOT.kRed)
        hsliceL[1].SetLineColor(ROOT.kBlue)
        hsliceR[1].SetLineColor(ROOT.kRed)
        meany =  0.5*(mpvR+mpvL)
        hsliceL[1].GetYaxis().SetRangeUser(meany-0.1, meany+0.1)
        hsliceL[1].GetXaxis().SetRangeUser(hsliceL[1].GetMean()-2*hsliceL[1].GetRMS(),hsliceL[1].GetMean()+2*hsliceL[1].GetRMS() )
        hsliceL[1].GetXaxis().SetTitle('x (mm)')
        hsliceL[1].GetYaxis().SetTitle('amplitude (mV)')
        hsliceL[1].Draw()
        hsliceR[1].Draw('same')
        tChType.Draw()
        ccc.SaveAs(outdir+'/'+ccc.GetName()+'.png')
        ccc.SaveAs(outdir+'/'+ccc.GetName()+'.pdf')
             
        
for ch in channels:
    for g in [gDiff_vs_thickness[ch]]:
        g.SetMarkerStyle(24)
        g.SetMarkerColor(ROOT.kBlack)
        g.SetLineColor(ROOT.kBlack)
        
    for g in [gAve_vs_thickness[ch]]:
        g.SetMarkerStyle(20)
        g.SetMarkerSize(0.9)
        g.SetMarkerColor(ROOT.kBlack)
        g.SetLineColor(ROOT.kBlack)
        
    for g in gL_vs_thickness[ch], gL_vs_thickness[ch], gDelayL_vs_thickness[ch], g_ampL_vs_thickness[ch],  gAmpSlopeL_vs_thickness[ch]:
        g.SetMarkerStyle(20)
        g.SetMarkerSize(0.9)
        g.SetMarkerColor(ROOT.kBlue)
        g.SetLineColor(ROOT.kBlue)

    for g in gR_vs_thickness[ch], gR_vs_thickness[ch], gDelayR_vs_thickness[ch], g_ampR_vs_thickness[ch], gAmpSlopeR_vs_thickness[ch]:
        g.SetMarkerStyle(20)
        g.SetMarkerSize(0.9)
        g.SetMarkerColor(ROOT.kRed)
        g.SetLineColor(ROOT.kRed)


leg1 = ROOT.TLegend(0.65,0.65,0.89,0.89)
leg1.AddEntry(gL_vs_thickness['BAR0'],'t_{left} - t_{MCP}','PL')
leg1.AddEntry(gR_vs_thickness['BAR0'],'t_{right} - t_{MCP}','PL')
leg1.AddEntry(gAve_vs_thickness['BAR0'],'t_{average} - t_{MCP}','PL')
leg1.AddEntry(gDiff_vs_thickness['BAR0'],'t_{diff}/2','PL')

leg2 = ROOT.TLegend(0.7,0.8,0.89,0.89)
leg2.AddEntry(gL_vs_thickness['BAR0'],'left','PL')
leg2.AddEntry(gR_vs_thickness['BAR0'],'right','PL')

leg3 = ROOT.TLegend(0.7,0.7,0.89,0.89)
leg3.AddEntry(gAve_vs_thickness['BAR0'],'t_{average} - t_{MCP}','PL')
leg3.AddEntry(gDiff_vs_thickness['BAR0'],'t_{diff}/2','PL')

canvas4 = {}
canvas0 = {}
canvas1 = {}
canvas2 = {}
canvas3 = {}

hdummy4 = {}
hdummy0 = {}
hdummy1 = {}
hdummy2 = {}
hdummy3 = {}


fitL = {}
fitR = {}
fitAve = {}
fitDiff = {}

tPowAve = {}
tPowDiff = {}



for ch in channels:

    #amp vs thickness
    # normalize to 3 mm thickness
    normAmp = 0.5*(g_ampR_vs_thickness[ch].Eval(3)+g_ampL_vs_thickness[ch].Eval(3))
    print 'normAmp', normAmp
    for j in range(0, g_ampR_vs_thickness[ch].GetN()):
        g_ampR_vs_thickness[ch].SetPoint(j, g_ampR_vs_thickness[ch].GetX()[j], g_ampR_vs_thickness[ch].GetY()[j]/normAmp);
        g_ampR_vs_thickness[ch].SetPointError(j, 0, g_ampR_vs_thickness[ch].GetEY()[j]/normAmp);
        g_ampL_vs_thickness[ch].GetY()[j] = g_ampL_vs_thickness[ch].GetY()[j]/normAmp;
        g_ampL_vs_thickness[ch].SetPointError(j, 0, g_ampL_vs_thickness[ch].GetEY()[j]/normAmp);
        
        
    canvas3[ch] = ROOT.TCanvas('amplitude_vs_thickness_%s'%ch,'amplitude_vs_thickness_%s'%ch)
    #canvas3[ch].SetGridy()
    hdummy3[ch] = ROOT.TH2F('hdummy3_%s'%ch,'',10,1,5,10,0,2)
    hdummy3[ch].Draw()
    hdummy3[ch].GetXaxis().SetTitle('thickness (mm)')
    hdummy3[ch].GetYaxis().SetTitle('normalized amplitude MPV')
    ffR = ROOT.TF1('ffR','[0]*(x/3)+[1]',0,10)
    #ffR = ROOT.TF1('ffR','[0]*(x/3)',0,10)
    ffR.SetLineColor(ROOT.kRed)
    ffR.SetLineStyle(3)
    ffL = ROOT.TF1('ffL','[0]*(x/3)+[1]',0,10)
    #ffL = ROOT.TF1('ffL','[0]*(x/3)',0,10)
    ffL.SetLineColor(ROOT.kBlue)
    ffL.SetLineStyle(3)
    g_ampR_vs_thickness[ch].Fit(ffR,'Q','',1.5,4.5)
    g_ampL_vs_thickness[ch].Fit(ffL,'Q','',1.5,4.5)
    g_ampR_vs_thickness[ch].Draw('psame')
    g_ampL_vs_thickness[ch].Draw('psame')
    leg2.Draw('same')
    tChType.Draw()


    # amp slope vs thickness
    canvas4[ch] = ROOT.TCanvas('amplitudeSlope_vs_thickness_%s'%ch,'amplitudeSlope_vs_thickness_%s'%ch)
    #canvas4[ch].SetGridy()
    hdummy4[ch] = ROOT.TH2F('hdummy4_%s'%ch,'',100,0,6,10,-2,2)
    hdummy4[ch].Draw()
    hdummy4[ch].GetXaxis().SetTitle('thickness (mm)')
    hdummy4[ch].GetYaxis().SetTitle('amplitude MPV variation(%/mm)')
    gAmpSlopeL_vs_thickness[ch].Draw('psame')
    gAmpSlopeR_vs_thickness[ch].Draw('psame')
    leg2.Draw('same')
    tChType.Draw()

    #delay vs thickness
    canvas0[ch] = ROOT.TCanvas('timeDelay_vs_thickness_%s'%ch,'timeDelay_vs_thickness_%s'%ch)
    #canvas0[ch].SetGridy()
    hdummy0[ch] = ROOT.TH2F('hdummy0_%s'%ch,'',100,0,6,10,-15,15)
    hdummy0[ch].Draw()
    hdummy0[ch].GetXaxis().SetTitle('thickness (mm)')
    hdummy0[ch].GetYaxis().SetTitle('time delay (ps/mm)')
    gDelayL_vs_thickness[ch].Draw('psame')
    gDelayR_vs_thickness[ch].Draw('psame')
    leg2.Draw('same')
    tChType.Draw()

    # time resolution vs slant tickness
    canvas2[ch] = ROOT.TCanvas('timeResolution_vs_thickness_%s'%ch,'timeResolution_vs_thickness_%s'%ch)
    #canvas2[ch].SetGridy()
    hdummy2[ch] = ROOT.TH2F('hdummy2_%s'%ch,'',26,1,5,80,10,70)
    hdummy2[ch].Draw()
    hdummy2[ch].GetXaxis().SetTitle('thickness (mm)')
    hdummy2[ch].GetYaxis().SetTitle('time resolution (ps)')
    gDiff_vs_thickness[ch].Draw('psame')
    gAve_vs_thickness[ch].Draw('psame')
    gL_vs_thickness[ch].Draw('psame')
    gR_vs_thickness[ch].Draw('psame')
    leg1.Draw('same')
    tChType.Draw()
    
    fitL[ch] = ROOT.TF1('fitL','[1]*1./pow(x,[0])',0,100)
    fitL[ch].SetLineColor(ROOT.kBlue)
    fitL[ch].SetParameter(0, 0.5)
    fitL[ch].SetParameter(1, 70)
    gL_vs_thickness[ch].Fit(fitL[ch],'Q')
    print fitL[ch].GetParameter(0), fitL[ch].GetParError(0)
    
    fitR[ch] = ROOT.TF1('fitR','[1]*1./pow(x,[0])',0,100)
    fitR[ch].SetLineColor(ROOT.kRed)
    fitR[ch].SetParameter(0, 0.5)
    fitR[ch].SetParameter(1, 70)
    gR_vs_thickness[ch].Fit(fitR[ch],'Q')
    print fitR[ch].GetParameter(0), fitR[ch].GetParError(0)
    
    fitAve[ch] = ROOT.TF1('fitAve','[1]*1./pow(x,[0])',0,100)
    fitAve[ch].SetLineColor(ROOT.kBlack)
    fitAve[ch].SetLineStyle(1)
    fitAve[ch].SetParameter(0, 0.5)
    fitAve[ch].SetParameter(1, 50)
    gAve_vs_thickness[ch].Fit(fitAve[ch],'Q')

        
    fitDiff[ch] = ROOT.TF1('fitDiff','[1]*1./pow(x,[0])',0,100)
    fitDiff[ch].SetLineColor(ROOT.kBlack)
    fitDiff[ch].SetLineStyle(2)
    fitDiff[ch].SetParameter(0, 0.5)
    fitDiff[ch].SetParameter(1, 50)
    gDiff_vs_thickness[ch].Fit(fitDiff[ch],'Q')

   
    tPowAve[ch] = ROOT.TLatex( 0.15, 0.23, '#sigma_{tAve} ~  x^{- (%.2f +/- %.2f)}'%(fitAve[ch].GetParameter(0), fitAve[ch].GetParError(0)))
    tPowAve[ch].SetNDC()
    tPowAve[ch].SetTextSize(0.040)
    tPowAve[ch].Draw()

    tPowDiff[ch] = ROOT.TLatex( 0.15, 0.15, '#sigma_{tDiff/2} ~ x^{- (%.2f +/- %.2f)}'%(fitDiff[ch].GetParameter(0), fitDiff[ch].GetParError(0)))
    tPowDiff[ch].SetNDC()
    tPowDiff[ch].SetTextSize(0.040)
    tPowDiff[ch].Draw()
    
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
    
    
    for c in canvas0[ch],  canvas2[ch], canvas3[ch], canvas4[ch]:
        c.SaveAs(outdir+'/'+c.GetName()+'.png')
        c.SaveAs(outdir+'/'+c.GetName()+'.pdf')
        c.SaveAs(outdir+'/'+c.GetName()+'.C')

    #raw_input('ok?')
