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

outdir = sys.argv[1]

#ampCorrType = 'ampCorr'
ampCorrType = 'pulseIntCorr'

barRegion = ''
#barRegion = 'barCenter'
#barRegion = 'barEdge'

if (outdir != './'):
    os.system('mkdir %s'%outdir)
print 'Saving plots in %s'%outdir
shutil.copy('index.php', '%s'%outdir)

angles0 = [90, 60, 53, 45, 37, 30, 20, 14]
angles  = [90-a for a in angles0]
channels = ['BAR%s'%i for i in range(0,3)]
thresholds = [50, 70, 100, 150, 200, 250, 300, 350, 400, 450, 500, 600, 700, 800, 900, 1000, 1500, 2000]
#thresholds = [20, 50, 100, 150, 200, 250, 300, 350, 400, 450, 500, 600, 700, 800, 900, 1000]
vb = 72

bestThreshold = -1 # -1 to use optimal threshold for each point
#bestThreshold = 200 # 

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
sigmaPTK = 0.014

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
        if (barRegion == ''):
            #fname = '../v7/output_3bars_Vbias%s_thr%sADC_xyangle%s_binnedAmpWalkCorr.root'%(vb, th, 90-angle)
            fname = '../v7/output_3bars_Vbias%s_thr%sADC_xyangle%s.root'%(vb, th, 90-angle)
        if (barRegion == 'barCenter'): fname = '../v7/barCenter/output_3bars_Vbias%s_thr%sADC_xyangle%s_barCenter.root'%(vb, th, 90-angle)
        if (barRegion == 'barEdge'): fname = '../v7/barEdge/output_3bars_Vbias%s_thr%sADC_xyangle%s_barEdge.root'%(vb, th, 90-angle)
        if (angle == 0):
            fname = fname.replace('xyangle90','xyangle90_runs7639-7679')
        print fname
        if (os.path.isfile(fname) ==  False):
            print 'File %s', fname, 'not found!'
            continue
        ff = ROOT.TFile.Open(fname)
        for channel in channels:
            # tAve
            sigmaEff = ff.Get('h_effectiveSigmaAve_%s_posCorr_%s'%(ampCorrType,channel)).GetMean()
            h = ff.Get('h_tAve_%s_posCorr_%s'%(ampCorrType,channel))
            fitfun = h.GetFunction('fitFunAve_%s_posCorr_%s'%(ampCorrType,channel))
            if (fitfun == None):
                continue
            sigmaGaus  = fitfun.GetParameter(2)
            esigmaGaus = fitfun.GetParError(2)
            resolGaus  = ROOT.TMath.Sqrt(sigmaGaus*sigmaGaus - sigmaPTK*sigmaPTK)*1000. # ps
            if (math.isnan(resolGaus) or resolGaus < 15.): resolGaus = ROOT.TMath.Sqrt(sigmaEff*sigmaEff - sigmaPTK*sigmaPTK)*1000. # ps
            sigmatAve[angle][th][channel] = resolGaus
            sigmatAve_err[angle][th][channel] = esigmaGaus*1000.

            # tDiff
            h = ff.Get('h_tDiff_%s_posCorr_%s'%(ampCorrType,channel))
            fitfun = h.GetFunction('fitFunDiff_%s_posCorr_%s'%(ampCorrType,channel))
            if (fitfun == None):
                continue
            sigmaGaus  = fitfun.GetParameter(2)
            esigmaGaus = fitfun.GetParError(2)
            resolGaus  = sigmaGaus*1000. # ps
            sigmatDiff[angle][th][channel] = resolGaus/2
            sigmatDiff_err[angle][th][channel] = esigmaGaus/2*1000.

bestThresholdAve = {} # [angle][channel]
bestThresholdDiff = {} # [angle][channel]

for angle in angles:
    bestThresholdAve[angle] = {}
    bestThresholdDiff[angle] = {}
    for ch in channels:
        print angle, th, ch 
        l = [ sigmatAve[angle][th][ch] for th in thresholds ]
        bestIndexAve = l.index(min(l))
        bestThresholdAve[angle][ch] = thresholds[bestIndexAve]
        ll = [ sigmatDiff[angle][th][ch] for th in thresholds ]
        bestIndexDiff = ll.index(min(ll))
        if (bestThreshold == -1):
            bestThresholdDiff[angle][ch] = thresholds[bestIndexDiff]
        else:
            bestThresholdAve[angle][ch] = bestThreshold
            bestThresholdDiff[angle][ch] = bestThreshold


for angle in angles:
    f[angle] = {}
    gL[angle] = {}
    gR[angle] = {}
    pL[angle] = {}
    pR[angle] = {}
    h_ampL[angle] = {}
    h_ampR[angle] = {}
    h2_ampL_vs_posXc[angle] = {}
    h2_ampR_vs_posXc[angle] = {}
    for ch in channels:
        if (barRegion == ''):
            #fname = '../v7/output_3bars_Vbias%s_thr%sADC_xyangle%s_binnedAmpWalkCorr.root'%(vb, th, 90-angle)
            fname = '../v7/output_3bars_Vbias%s_thr%sADC_xyangle%s.root'%(vb, th, 90-angle)
        if (barRegion == 'barCenter'): fname = '../v7/barCenter/output_3bars_Vbias%s_thr%sADC_xyangle%s_barCenter.root'%(vb, th, 90-angle)
        if (barRegion == 'barEdge'): fname = '../v7/barEdge/output_3bars_Vbias%s_thr%sADC_xyangle%s_barEdge.root'%(vb, th, 90-angle)
        if (angle == 0):
            fname = fname.replace('xyangle90','xyangle90_runs7639-7679')
        f[angle][ch] = ROOT.TFile.Open(fname)
        gR[angle][ch] = f[angle][ch].Get('g_tResolGausR_%s_%s'%(ampCorrType,ch) )
        gL[angle][ch] = f[angle][ch].Get('g_tResolGausL_%s_%s'%(ampCorrType,ch) )
        pR[angle][ch] = f[angle][ch].Get('p_tR_%s_vs_posXc_%s'%(ampCorrType,ch) )
        pL[angle][ch] = f[angle][ch].Get('p_tL_%s_vs_posXc_%s'%(ampCorrType,ch) )
        h_ampR[angle][ch] = f[angle][ch].Get('h_ampR_%s'%ch)
        h_ampL[angle][ch] = f[angle][ch].Get('h_ampL_%s'%ch)
        h2_ampR_vs_posXc[angle][ch] = f[angle][ch].Get('h2_ampR_vs_posXc_%s'%ch)
        h2_ampL_vs_posXc[angle][ch] = f[angle][ch].Get('h2_ampL_vs_posXc_%s'%ch)
    
tChType = ROOT.TLatex( 0.13, 0.92, 'LYSO:Ce 3x3x57 mm^{3} - HPK S12572-015')
tChType.SetNDC()
tChType.SetTextSize(0.035)
g_ampL_vs_slantTickness = {}
g_ampR_vs_slantTickness = {}

gAve_vs_angle = {}
gDiff_vs_angle = {}
gR_vs_angle = {}
gL_vs_angle = {}

gAve_vs_slantTickness = {}
gDiff_vs_slantTickness = {}
gR_vs_slantTickness = {}
gL_vs_slantTickness = {}

gDelayL_vs_angle = {}
gDelayR_vs_angle = {}

gAmpSlopeL_vs_angle = {}
gAmpSlopeR_vs_angle = {}



ampR = {}
ampL = {}

tResL = {}
tResR = {}
tResAve = {}
tResDiff = {}
slantTickness = {}

slopeL = {}
slopeR = {}

eslopeL = {}
eslopeR = {}

err = 2.
errAve = 2.
errDiff = 2.

for ch in channels:
    gAve_vs_angle[ch] = ROOT.TGraphErrors()
    gDiff_vs_angle[ch] = ROOT.TGraphErrors()
    gR_vs_angle[ch] = ROOT.TGraphErrors()
    gL_vs_angle[ch] = ROOT.TGraphErrors()
    gDelayR_vs_angle[ch] = ROOT.TGraphErrors()
    gDelayL_vs_angle[ch] = ROOT.TGraphErrors()
    gAmpSlopeR_vs_angle[ch] = ROOT.TGraphErrors()
    gAmpSlopeL_vs_angle[ch] = ROOT.TGraphErrors()

    g_ampR_vs_slantTickness[ch] = ROOT.TGraphErrors()
    g_ampL_vs_slantTickness[ch] = ROOT.TGraphErrors()
    
    gDiff_vs_slantTickness[ch] = ROOT.TGraphErrors()
    gAve_vs_slantTickness[ch] = ROOT.TGraphErrors()
    gR_vs_slantTickness[ch] = ROOT.TGraphErrors()
    gL_vs_slantTickness[ch] = ROOT.TGraphErrors()
    
    tResL[ch] = {}
    tResR[ch] = {}
    tResAve[ch] = {}
    tResDiff[ch] = {}
    slopeL[ch] = {}
    slopeR[ch] = {}
    eslopeL[ch] = {}
    eslopeR[ch] = {}
    
    for i, angle in enumerate(angles):
        #################
        # time resolution
        tResL[ch][angle] = gL[angle][ch].GetFunction(gL[angle][ch].GetName().replace('g_tResolGaus','fpol0_t') ).GetParameter(0)
        tResR[ch][angle] = gR[angle][ch].GetFunction(gR[angle][ch].GetName().replace('g_tResolGaus','fpol0_t') ).GetParameter(0)
       
        thr = bestThresholdDiff[angle][ch]
        tResDiff[ch][angle] = sigmatDiff[angle][thr][ch]
        errDiff = math.sqrt(sigmatDiff_err[angle][thr][ch]*sigmatDiff_err[angle][thr][ch] + err * err)

        print ch, angle, tResDiff[ch][angle], thr
        
        thr = bestThresholdAve[angle][ch]
        tResAve[ch][angle] = sigmatAve[angle][thr][ch]
        errAve = math.sqrt(sigmatAve_err[angle][thr][ch]*sigmatAve_err[angle][thr][ch] + err * err)
       

        #if (sigmatAve[angle][thr][ch]>=sigmatDiff[angle][thr][ch]):
        #    sigmaRef = math.sqrt(sigmatAve[angle][thr][ch]*sigmatAve[angle][thr][ch] - sigmatDiff[angle][thr][ch]*sigmatDiff[angle][thr][ch])
        #else:
        #    sigmaRef = 0.
        #print ch, angle, 'sigma_MCP = %.1f ps'%(sigmaRef)
        #tResL[ch][angle] = math.sqrt(tResL[ch][angle]*tResL[ch][angle]-sigmaRef*sigmaRef)
        #tResR[ch][angle] = math.sqrt(tResR[ch][angle]*tResR[ch][angle]-sigmaRef*sigmaRef)

        # graphs time resol vs angle
        gAve_vs_angle[ch].SetPoint(i, angles0[i], tResAve[ch][angle])
        gDiff_vs_angle[ch].SetPoint(i, angles0[i], tResDiff[ch][angle])
        gL_vs_angle[ch].SetPoint(i, angles0[i], tResL[ch][angle])
        gR_vs_angle[ch].SetPoint(i, angles0[i], tResR[ch][angle])
        
        gAve_vs_angle[ch].SetPointError(i, 1, errAve )
        gDiff_vs_angle[ch].SetPointError(i, 1, errDiff )
        gL_vs_angle[ch].SetPointError(i, 1, err)
        gR_vs_angle[ch].SetPointError(i, 1, err)
                
        # graphs time resol vs slantTickness
        slantTickness = 3. / math.cos((angle) * math.pi / 180.)
        #if (angle == 14) : slantTickness = 3. / math.cos((15.8) * math.pi / 180.)
        
        gAve_vs_slantTickness[ch].SetPoint(i, slantTickness, tResAve[ch][angle])
        gDiff_vs_slantTickness[ch].SetPoint(i, slantTickness, tResDiff[ch][angle])
        gL_vs_slantTickness[ch].SetPoint(i, slantTickness, tResL[ch][angle])
        gR_vs_slantTickness[ch].SetPoint(i, slantTickness, tResR[ch][angle])

        errSl = 3./ pow(math.cos((angle) * math.pi / 180.), 2) * math.sin((angle) * math.pi / 180.) * 2. * math.pi / 180.
        #errSl = 0
        
        gAve_vs_slantTickness[ch].SetPointError(i, errSl, errAve )
        gDiff_vs_slantTickness[ch].SetPointError(i, errSl, errDiff )
        gL_vs_slantTickness[ch].SetPointError(i, errSl, err)
        gR_vs_slantTickness[ch].SetPointError(i, errSl, err)

        ####################
        #time delay vs angle
        fpol1L = ROOT.TF1('fpol1L','pol1',-100,100)
        fpol1L.SetLineColor(ROOT.kBlue)
        x1 = pL[angle][ch].GetMean(1) - 50
        x2 = pL[angle][ch].GetMean(1) + 50
        fpol1L.SetRange(x1,x2)
        pL[angle][ch].Fit(fpol1L,'QR')
        slopeL[ch][angle] = fpol1L.GetParameter(1)*1000.
        eslopeL[ch][angle] = fpol1L.GetParError(1)*1000.
        if (fpol1L.GetChisquare()/fpol1L.GetNDF()>20):
            pL[angle][ch].Fit(fpol1L,'QRW')
            slopeL[ch][angle] = fpol1L.GetParameter(1)*1000.
            eslopeL[ch][angle] = 0.2

                
        fpol1R = ROOT.TF1('fpol1R','pol1',-100,100)
        fpol1R.SetLineColor(ROOT.kRed)
        x1 = pR[angle][ch].GetMean(1) - 50
        x2 = pR[angle][ch].GetMean(1) + 50
        fpol1R.SetRange(x1,x2)
        pR[angle][ch].Fit(fpol1R,'QR')
        slopeR[ch][angle] = fpol1R.GetParameter(1)*1000.
        eslopeR[ch][angle] = fpol1R.GetParError(1)*1000.
        if (fpol1R.GetChisquare()/fpol1R.GetNDF()>20):
            pR[angle][ch].Fit(fpol1R,'QRW')
            slopeR[ch][angle] = fpol1R.GetParameter(1)*1000.
            eslopeR[ch][angle] = 0.2

 
        cc = ROOT.TCanvas('time_vs_posXc_angle%s_%s'%(angles0[i],ch))
        cc.SetGridy()
        pL[angle][ch].SetMarkerColor(ROOT.kBlue)
        pR[angle][ch].SetMarkerColor(ROOT.kRed)
        pL[angle][ch].SetLineColor(ROOT.kBlue)
        pR[angle][ch].SetLineColor(ROOT.kRed)
        meany =  0.5*(pL[angle][ch].GetMean(2)+pR[angle][ch].GetMean(2) )
        pL[angle][ch].GetYaxis().SetRangeUser(meany-1.0, meany+1.0)
        pL[angle][ch].GetXaxis().SetRangeUser(pL[angle][ch].GetMean()-2*pL[angle][ch].GetRMS(),pL[angle][ch].GetMean()+2*pL[angle][ch].GetRMS() )
        pL[angle][ch].GetXaxis().SetTitle('x (mm)')
        pL[angle][ch].GetYaxis().SetTitle('time (ps)')
        pL[angle][ch].Draw()
        pR[angle][ch].Draw('same')
        cc.SaveAs(outdir+'/'+cc.GetName()+'.png')
        cc.SaveAs(outdir+'/'+cc.GetName()+'.pdf')

        slopeR[ch][angle] =  slopeR[ch][angle] - 0
        slopeL[ch][angle] =  slopeL[ch][angle] - 0
        
        dtof = math.sin( (angle) * math.pi / 180.)/ 0.299792 # ps/mm
        #dtof = 0
        #print dtof, slopeR[ch][angle], slopeL[ch][angle]
        
        gDelayL_vs_angle[ch].SetPoint( i, angles0[i], (slopeL[ch][angle] + dtof))
        gDelayL_vs_angle[ch].SetPointError( i, 0, eslopeL[ch][angle] )

        gDelayR_vs_angle[ch].SetPoint( i, angles0[i], (slopeR[ch][angle] + dtof) )
        gDelayR_vs_angle[ch].SetPointError( i, 0, eslopeR[ch][angle] )

        #############################
        # amplitude vs slant tickness
        fLandau = ROOT.TF1('fLandau','landau', 0, 1)
        #fLandau.SetRange(0.08,0.90)
        h_ampR[angle][ch].Fit(fLandau,'QR')
        mpvR = fLandau.GetParameter(1)
        g_ampR_vs_slantTickness[ch].SetPoint( i, slantTickness, fLandau.GetParameter(1))
        g_ampR_vs_slantTickness[ch].SetPointError( i, 0, fLandau.GetParError(1) )
        h_ampL[angle][ch].Fit(fLandau,'QR')
        mpvL = fLandau.GetParameter(1)
        g_ampL_vs_slantTickness[ch].SetPoint( i, slantTickness, fLandau.GetParameter(1))
        g_ampL_vs_slantTickness[ch].SetPointError( i, 0, fLandau.GetParError(1) )

        ##########################
        # amplitude slope vs angle
        fLandau.SetParameter(1, h2_ampR_vs_posXc[angle][ch].GetMean(2))
        fLandau.SetParameter(0, 0.01)
        hsliceR = ROOT.TObjArray()
        h2_ampR_vs_posXc[angle][ch].FitSlicesY(fLandau,1,-1, 0, 'QNG2', hsliceR)
        fpol1R.SetRange(-100, 100)
        hsliceR[1].Fit(fpol1R,'QR')
        ampSlopeR = fpol1R.GetParameter(1)/mpvR * 100
        ampSlopeR_err = fpol1R.GetParError(1)/mpvR *100
                
        fLandau.SetParameter(1, h2_ampL_vs_posXc[angle][ch].GetMean(2))
        fLandau.SetParameter(0, 0.01)
        hsliceL = ROOT.TObjArray()
        h2_ampL_vs_posXc[angle][ch].FitSlicesY(fLandau,1,-1, 0, 'QRG2', hsliceL)
        fpol1L.SetRange(-100, 100)
        hsliceL[1].Fit(fpol1L,'QR')
        ampSlopeL = fpol1L.GetParameter(1)/mpvL * 100
        ampSlopeL_err = fpol1L.GetParError(1)/mpvL *100

        gAmpSlopeL_vs_angle[ch].SetPoint( i, angles0[i], ampSlopeL)
        gAmpSlopeL_vs_angle[ch].SetPointError( i, 0, ampSlopeL_err )
        
        gAmpSlopeR_vs_angle[ch].SetPoint( i, angles0[i], ampSlopeR )
        gAmpSlopeR_vs_angle[ch].SetPointError( i, 0, ampSlopeR_err) 

        
        ccc = ROOT.TCanvas('amplitude_vs_posXc_angle%s_%s'%(angles0[i],ch))
        ccc.SetGridy()
        hsliceL[1].SetMarkerColor(ROOT.kBlue)
        hsliceR[1].SetMarkerColor(ROOT.kRed)
        hsliceL[1].SetLineColor(ROOT.kBlue)
        hsliceR[1].SetLineColor(ROOT.kRed)
        meany =  0.5*(mpvR+mpvL)
        hsliceL[1].GetYaxis().SetRangeUser(meany-0.1, meany+0.1)
        hsliceL[1].GetXaxis().SetRangeUser(hsliceL[1].GetMean()-2*hsliceL[1].GetRMS(),hsliceL[1].GetMean()+2*hsliceL[1].GetRMS() )
        hsliceL[1].GetXaxis().SetTitle('x (mm)')
        hsliceL[1].GetYaxis().SetTitle('amplitude (V)')
        hsliceL[1].Draw()
        hsliceR[1].Draw('same')
        tChType.Draw()
        ccc.SaveAs(outdir+'/'+ccc.GetName()+'.png')
        ccc.SaveAs(outdir+'/'+ccc.GetName()+'.pdf')
        
        
for ch in channels:
    for g in gDiff_vs_angle[ch], gDiff_vs_slantTickness[ch]:
        g.SetMarkerStyle(20)
        g.SetMarkerColor(ROOT.kBlack)
        g.SetLineColor(ROOT.kBlack)
        
    for g in gAve_vs_angle[ch], gAve_vs_slantTickness[ch]:
        g.SetMarkerStyle(20)
        g.SetMarkerColor(ROOT.kGray+1)
        g.SetLineColor(ROOT.kGray+1)
        
    for g in gL_vs_angle[ch], gL_vs_slantTickness[ch], gDelayL_vs_angle[ch], g_ampL_vs_slantTickness[ch],  gAmpSlopeL_vs_angle[ch]:
        g.SetMarkerStyle(20)
        g.SetMarkerColor(ROOT.kBlue)
        g.SetLineColor(ROOT.kBlue)

    for g in gR_vs_angle[ch], gR_vs_slantTickness[ch], gDelayR_vs_angle[ch], g_ampR_vs_slantTickness[ch], gAmpSlopeR_vs_angle[ch]:
        g.SetMarkerStyle(20)
        g.SetMarkerColor(ROOT.kRed)
        g.SetLineColor(ROOT.kRed)


leg1 = ROOT.TLegend(0.7,0.75,0.89,0.89)
leg1.AddEntry(gL_vs_angle['BAR0'],'downstream','PL')
leg1.AddEntry(gR_vs_angle['BAR0'],'upstream','PL')
leg1.AddEntry(gAve_vs_angle['BAR0'],'average','PL')
leg1.AddEntry(gDiff_vs_angle['BAR0'],'diff','PL')

leg2 = ROOT.TLegend(0.7,0.75,0.89,0.89)
leg2.AddEntry(gL_vs_angle['BAR0'],'downstream','PL')
leg2.AddEntry(gR_vs_angle['BAR0'],'upstream','PL')

leg3 = ROOT.TLegend(0.7,0.75,0.89,0.89)
leg3.AddEntry(gAve_vs_angle['BAR0'],'t_{average}','PL')
leg3.AddEntry(gDiff_vs_angle['BAR0'],'t_{diff}','PL')

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

    #amp vs slant tickness
    canvas3[ch] = ROOT.TCanvas('amplitude_vs_slantTickness_%s'%ch,'amplitude_vs_slantTickness_%s'%ch)
    canvas3[ch].SetGridy()
    hdummy3[ch] = ROOT.TH2F('hdummy3_%s'%ch,'',91,0,15,10,0,0.7)
    hdummy3[ch].Draw()
    hdummy3[ch].GetXaxis().SetTitle('slant tickness (mm)')
    hdummy3[ch].GetYaxis().SetTitle('amplitude MPV (V)')
    ffR = ROOT.TF1('ffR','pol1')
    ffR.SetLineColor(ROOT.kRed)
    ffR.SetLineStyle(3)
    ffL = ROOT.TF1('ffL','pol1')
    ffL.SetLineColor(ROOT.kBlue)
    ffL.SetLineStyle(3)
    g_ampR_vs_slantTickness[ch].Fit(ffR)
    g_ampL_vs_slantTickness[ch].Fit(ffL)
    g_ampR_vs_slantTickness[ch].Draw('psame')
    g_ampL_vs_slantTickness[ch].Draw('psame')
    leg2.Draw('same')
    tChType.Draw()


    # amp slope vs angle
    canvas4[ch] = ROOT.TCanvas('amplitudeSlope_vs_angle_%s'%ch,'amplitudeSlope_vs_angle_%s'%ch)
    canvas4[ch].SetGridy()
    hdummy4[ch] = ROOT.TH2F('hdummy4_%s'%ch,'hdummy4_%s'%ch,92,-1,91,10,-1,1)
    hdummy4[ch].Draw()
    hdummy4[ch].GetXaxis().SetTitle('angle of incidence (#circ)')
    hdummy4[ch].GetYaxis().SetTitle('amplitude MPV variation(%/mm)')
    gAmpSlopeL_vs_angle[ch].Draw('psame')
    gAmpSlopeR_vs_angle[ch].Draw('psame')
    leg2.Draw('same')
    tChType.Draw()

    #delay vs angle
    canvas0[ch] = ROOT.TCanvas('timeDelay_vs_angle_%s'%ch,'timeDelay_vs_angle_%s'%ch)
    canvas0[ch].SetGridy()
    hdummy0[ch] = ROOT.TH2F('hdummy0_%s'%ch,'',92,-1,91,10,-15,15)
    hdummy0[ch].Draw()
    hdummy0[ch].GetXaxis().SetTitle('angle of incidence (#circ)')
    hdummy0[ch].GetYaxis().SetTitle('time delay (ps/mm)')
    gDelayL_vs_angle[ch].Draw('psame')
    gDelayR_vs_angle[ch].Draw('psame')
    leg2.Draw('same')
    tChType.Draw()

    # vs theta
    canvas1[ch] = ROOT.TCanvas('timeResolution_vs_angle_%s'%ch,'timeResolution_vs_angle_%s'%ch)
    canvas1[ch].SetGridy()
    hdummy1[ch] = ROOT.TH2F('hdummy1_%s'%ch,'',92,-1,91,80,0,80)
    hdummy1[ch].Draw()
    hdummy1[ch].GetXaxis().SetTitle('angle of incidence (#circ)')
    hdummy1[ch].GetYaxis().SetTitle('time resolution (ps)')
    gDiff_vs_angle[ch].Draw('psame')
    gAve_vs_angle[ch].Draw('psame')
    gL_vs_angle[ch].Draw('psame')
    gR_vs_angle[ch].Draw('psame')
    leg1.Draw('same')
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
    leg3.Draw('same')
    tChType.Draw()
    
    fitL[ch] = ROOT.TF1('fitL','[1]*1./pow(x,[0])',0,100)
    fitL[ch].SetLineColor(ROOT.kBlue)
    fitL[ch].SetParameter(0, 0.5)
    gL_vs_slantTickness[ch].RemovePoint(2)
    gL_vs_slantTickness[ch].RemovePoint(2)
    gL_vs_slantTickness[ch].RemovePoint(2)
    gL_vs_slantTickness[ch].RemovePoint(2)
    gL_vs_slantTickness[ch].Fit(fitL[ch],'Q')
    print fitL[ch].GetParameter(0), fitL[ch].GetParError(0)
    
    fitR[ch] = ROOT.TF1('fitR','[1]*1./pow(x,[0])',0,100)
    fitR[ch].SetLineColor(ROOT.kRed)
    fitR[ch].SetParameter(0, 0.5)
    gR_vs_slantTickness[ch].RemovePoint(2)
    gR_vs_slantTickness[ch].RemovePoint(2)
    gR_vs_slantTickness[ch].RemovePoint(2)
    gR_vs_slantTickness[ch].RemovePoint(2)
    gR_vs_slantTickness[ch].Fit(fitR[ch],'Q')
    print fitR[ch].GetParameter(0), fitR[ch].GetParError(0)
    
    fitAve[ch] = ROOT.TF1('fitAve','[1]*1./pow(x,[0])',0,100)
    fitAve[ch].SetLineColor(ROOT.kGray+1)
    fitAve[ch].SetLineStyle(2)
    fitAve[ch].SetParameter(1, 70)
    fitAve[ch].SetParameter(0, 0.3)
    gAve_vs_slantTickness[ch].Fit(fitAve[ch],'Q')

       
    fitDiff[ch] = ROOT.TF1('fitDiff','[1]*1./pow(x,[0])',0,100)
    fitDiff[ch].SetLineColor(ROOT.kBlack)
    fitDiff[ch].SetLineStyle(1)
    fitDiff[ch].SetParameter(0, 0.5)
    gDiff_vs_slantTickness[ch].Fit(fitDiff[ch],'Q')

    
    tPowAve[ch] = ROOT.TLatex( 0.15, 0.23, 'tAve ~  x^{-(%.2f +/- %.2f)}'%(fitAve[ch].GetParameter(0), fitAve[ch].GetParError(0)))
    tPowAve[ch].SetNDC()
    tPowAve[ch].SetTextSize(0.035)
    tPowAve[ch].Draw()

    tPowDiff[ch] = ROOT.TLatex( 0.15, 0.15, 'tDiff ~ x^{-(%.2f +/- %.2f)}'%(fitDiff[ch].GetParameter(0), fitDiff[ch].GetParError(0)))
    tPowDiff[ch].SetNDC()
    tPowDiff[ch].SetTextSize(0.035)
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
      

    
    for c in canvas0[ch], canvas1[ch], canvas2[ch], canvas3[ch], canvas4[ch]:
        c.SaveAs(outdir+'/'+c.GetName()+'.png')
        c.SaveAs(outdir+'/'+c.GetName()+'.pdf')

    #raw_input('ok?')



# plot with all three bars
legAll = ROOT.TLegend(0.15,0.15,0.40,0.32)
leg1.AddEntry(gDiff_vs_angle['BAR0'],'diff','PL')
# amp slope vs angle
canvasAmpSlope = ROOT.TCanvas('amplitudeSlope_vs_angle','amplitudeSlope_vs_angle')
canvasAmpSlope.SetGridy()
hdummy4['BAR0'].Draw()
for i,ch in enumerate(channels):
    gAmpSlopeL_vs_angle[ch].SetMarkerStyle(20+i)
    gAmpSlopeL_vs_angle[ch].SetMarkerColor(ROOT.kBlue+i)
    gAmpSlopeL_vs_angle[ch].SetLineColor(ROOT.kBlue+i)
    gAmpSlopeR_vs_angle[ch].SetMarkerStyle(24+i)
    gAmpSlopeR_vs_angle[ch].SetMarkerColor(ROOT.kRed+i)
    gAmpSlopeR_vs_angle[ch].SetLineColor(ROOT.kRed+i)
    gAmpSlopeL_vs_angle[ch].Draw('psame')
    gAmpSlopeR_vs_angle[ch].Draw('psame')
    legAll.AddEntry(gAmpSlopeR_vs_angle[ch],'%s - upstream'%ch,'PL')
    legAll.AddEntry(gAmpSlopeL_vs_angle[ch],'%s - downstream'%ch,'PL')
legAll.Draw('same')
tChType.Draw()
#raw_input('ok?')
canvasAmpSlope.SaveAs(outdir+'/'+canvasAmpSlope.GetName()+'.png')
canvasAmpSlope.SaveAs(outdir+'/'+canvasAmpSlope.GetName()+'.pdf')

#delay vs angle
canvasTimeSlope = ROOT.TCanvas('timeDelay_vs_angle','timeDelay_vs_angle')
canvasTimeSlope.SetGridy()
hdummy0['BAR0'].Draw()
for i,ch in enumerate(channels):
    gDelayL_vs_angle[ch].SetMarkerStyle(20+i)
    gDelayL_vs_angle[ch].SetMarkerColor(ROOT.kBlue+i)
    gDelayL_vs_angle[ch].SetLineColor(ROOT.kBlue+i)
    gDelayR_vs_angle[ch].SetMarkerStyle(24+i)
    gDelayR_vs_angle[ch].SetMarkerColor(ROOT.kRed+i)
    gDelayR_vs_angle[ch].SetLineColor(ROOT.kRed+i)
    gDelayL_vs_angle[ch].Draw('psame')
    gDelayR_vs_angle[ch].Draw('psame')
legAll.Draw('same')
tChType.Draw()
#raw_input('ok?')
canvasTimeSlope.SaveAs(outdir+'/'+canvasTimeSlope.GetName()+'.png')
canvasTimeSlope.SaveAs(outdir+'/'+canvasTimeSlope.GetName()+'.pdf')
    
outfile = ROOT.TFile(outdir+'/graphs_timeResolution_vs_slantThickness.root','recreate');
for ch in channels:
    gAve_vs_slantTickness[ch].SetName('gAve_vs_slantTickness_%s'%ch)
    gDiff_vs_slantTickness[ch].SetName('gDiff_vs_slantTickness_%s'%ch)
    gAve_vs_slantTickness[ch].Write()
    gDiff_vs_slantTickness[ch].Write()
outfile.Close()
    
        
    
