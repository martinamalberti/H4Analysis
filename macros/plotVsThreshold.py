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
#ROOT.gStyle.SetLabelSize(0.04,'X')
#ROOT.gStyle.SetLabelSize(0.04,'Y')
#ROOT.gStyle.SetTitleSize(0.04,'X')
#ROOT.gStyle.SetTitleSize(0.04,'Y')
ROOT.gErrorIgnoreLevel = ROOT.kWarning;





useTdiff = True
inputDir = '../v11/biasScan/'
#inputDir = '../v11/yzAngleScan/'

#ampCorrType = 'ampCorr'
ampCorrType = 'pulseIntCorr'

barRegion = ''
#barRegion = 'barCenter'
#barRegion = 'barEdge'

angle  = sys.argv[1] # angle or material thickness
sipm   = sys.argv[2] # sipm type HPK or FBK
outdir = sys.argv[3] # outdir


Vbias = [72]
Vbias = [72, 71, 70, 69, 68]
# for the Vbreak assume nominal values (measured tipycally by Yuri at 18-20 ded) + extrapolation with temperature: 59mV/deg for HPK, 41 mV/deg FBK.
# HPK : 66 V --> 66.35 V
# FBK : 36.7 --> 37 V
Vbreak = 66.35 # HPK
if (sipm == 'FBK'):
    Vbreak = 37
    Vbias  = [43, 42, 41, 40, 39]

thresholds = [20, 50, 70, 100, 150, 200, 250, 300, 350, 400, 450, 500, 550, 600, 650, 700, 750, 800, 850, 900, 1000]# ADC

filePDE = ROOT.TFile.Open('pde/PDE_vs_OV.root')
f_pde_vs_ov = filePDE.Get('fPDE_vs_OV_%s'%sipm)

# error on pde
dV = 0.2
mypde = {}
err_pde = {}
for vb in Vbias:
    mypde[vb] = {}
    for vbr in [Vbreak-dV, Vbreak, Vbreak+dV]:
        mypde[vb][vbr] = f_pde_vs_ov.Eval(vb-vbr)
    errup = abs(mypde[vb][Vbreak]-mypde[vb][Vbreak-dV])
    errdown = abs(mypde[vb][Vbreak]-mypde[vb][Vbreak+dV])
    err_pde[vb] = 0.5 *(errup+errdown)
        
print err_pde

#PDE = {
#    72 : 38, 71 : 35, 70 : 30, 69 : 23, 68 : 15, # from slide 5 of https://indico.cern.ch/event/758017/contributions/3172748/attachments/1731615/2798893/18_10_10_CERNTB_H4_Results_on_crystal_bars.pdf
#    43 : 35.3, 42 : 33.8, 41 : 30.4, 40 : 25.3, 39 : 18.3  # from Yuri?? 
#}


kAdcToV = 1000./4096


channels = ['BAR%s'%i for i in range(0,3)]

channelType = {'BAR0':'LYSO:Ce 3x3x57 mm^{3} - HPK S12572-015 3x3 mm^{2}',
               'BAR1':'LYSO:Ce 3x3x57 mm^{3} - HPK S12572-015 3x3 mm^{2}',
               'BAR2':'LYSO:Ce 3x3x57 mm^{3} - HPK S12572-015 3x3 mm^{2}'}

if (angle == '4' or angle == '3' or angle == '2'):
    channels = ['BAR%s'%i for i in range(0,1)]
    channelType = {'BAR0':'LYSO:Ce 3x%sx57 mm^{3} - FBK thin-epi 5x5 mm^{2}'%angle}
    
print channels

#prepare output dir
if (outdir != './'):
    os.system('mkdir %s'%outdir)
print 'Saving plots in %s'%outdir
#shutil.copy('index.php', '%s'%outdir)


s_eff  = {} # s_eff[Vbias][thr][channel]
s_gaus = {}
s_gaus_err = {}

sigmaPTK  = 0.0123
resolGaus = 0
resolEff  = 0
     
for vb in Vbias:
    s_eff[vb] = {}
    s_gaus[vb] = {}
    s_gaus_err[vb] = {}
    for th in thresholds:
        s_eff[vb][th] = {}
        s_gaus[vb][th] = {}
        s_gaus_err[vb][th] = {}
        # xyangle scan
        #if (barRegion == ''):
        #    fname = '%s/output_3bars_Vbias%s_thr%sADC_xyangle%s.root'%(inputDir,vb, th, angle)
        #if (barRegion == 'barCenter'): fname = '%s/barCenter/output_3bars_Vbias%s_thr%sADC_xyangle%s_barCenter.root'%(inputDir, vb, th, angle)
        #if (barRegion == 'barEdge'): fname = '%s/barEdge/output_3bars_Vbias%s_thr%sADC_xyangle%s_barEdge.root'%(inputDir, vb, th, angle)
        #if (angle == '90'):
        #    fname = fname.replace('xyangle90','xyangle90_runs7639-7679')
        fname = '%s/output_3bars_Vbias%s_thr%sADC_xyangle%s.root'%(inputDir,vb, th, angle)
        if (angle == '4' or angle == '3' or angle == '2'):
            fname = '%s/output_1bar_Vbias%s_thr%sADC_%smm.root'%(inputDir,vb, th, angle)
        if ('yz' in inputDir):
            fname = '%s/output_3bars_Vbias%s_thr%sADC_yzangle%s.root'%(inputDir,vb, th, angle)
        #print fname
        if (os.path.isfile(fname) ==  False): continue
        f = ROOT.TFile.Open(fname)
               
        for channel in channels:
            #get sigmaEff, sigmaGauss
            sigmaEff   = f.Get('h_effectiveSigmaAve_%s_posCorr_%s'%(ampCorrType,channel)).GetMean()
            h = f.Get('h_tAve_%s_posCorr_%s'%(ampCorrType,channel))
            fitfun = h.GetFunction('fitFunAve_%s_posCorr_%s'%(ampCorrType,channel))
            if (useTdiff):
                sigmaEff   = f.Get('h_effectiveSigmaDiff_%s_posCorr_%s'%(ampCorrType,channel)).GetMean()
                h = f.Get('h_tDiff_%s_posCorr_%s'%(ampCorrType,channel))
                fitfun = h.GetFunction('fitFunDiff_%s_posCorr_%s'%(ampCorrType,channel))

            if (fitfun != None):            
                sigmaGaus  = fitfun.GetParameter(2)
                esigmaGaus = fitfun.GetParError(2)
            else:
                sigmaGaus  =  99999.
                esigmaGaus  =  99999.
                
                
            if (useTdiff):
                resolGaus = sigmaGaus*1000./2
                resolEff  = sigmaEff*1000./2
            else:
                resolGaus  = ROOT.TMath.Sqrt(sigmaGaus*sigmaGaus - sigmaPTK*sigmaPTK)*1000. # ps
                resolEff   = ROOT.TMath.Sqrt(sigmaEff*sigmaEff - sigmaPTK*sigmaPTK)*1000. # ps

            #if (math.isnan(resolGaus) or resolGaus > 500. or resolGaus < 15.): resolGaus = resolEff

            s_eff[vb][th][channel]  = resolEff
            s_gaus[vb][th][channel] = resolGaus
            s_gaus_err[vb][th][channel] = esigmaGaus*1000.
            #print channel, th, '    sigma_gaus = %.1f      sigma_eff = %.1f'%(s_gaus[vb][th][channel],  s_eff[vb][th][channel])
                       
        


# find optimal threshold for each Vbias
bestThreshold = {} # [vb][channel]
for vb in Vbias:
    bestThreshold[vb] = {}
    for ch in channels:
        l = [ s_gaus[vb][th][ch] for th in thresholds ]
        bestIndex = l.index(min(l))
        bestThreshold[vb][ch] = thresholds[bestIndex]
        #print ch, vb, bestThreshold[vb][ch]
       
    
# plot time resol vs thr, vb, pde        
g_thrScan = {} # g[Vbias][channel]
g_VbiasScan = {} # g[thr][channel]
g_pde = {} # g[thr][channel]
g_VbiasScan_opt = {} # g[channel]
g_pde_opt = {} # g[channel]


for vb in Vbias:
    g_thrScan[vb] = {}
    for channel in channels:
        g_thrScan[vb][channel] = ROOT.TGraphErrors()
        n = 0
        for th in thresholds:
            if ( channel in s_gaus[vb][th] ):
                #g_thrScan[vb][channel].SetPoint(n, th, s_eff[vb][th][channel])
                g_thrScan[vb][channel].SetPoint(n, th*kAdcToV, s_gaus[vb][th][channel])
                g_thrScan[vb][channel].SetPointError(n, 0, s_gaus_err[vb][th][channel])
                n = n+1

for th in thresholds:
    g_VbiasScan[th] = {}
    g_pde[th] = {}
    for channel in channels:
        g_VbiasScan[th][channel] = ROOT.TGraphErrors()
        g_pde[th][channel] = ROOT.TGraphErrors()
        n = 0
        for vb in Vbias:
            if ( channel in s_gaus[vb][th] ):
                g_VbiasScan[th][channel].SetPoint(n, vb - Vbreak, s_gaus[vb][th][channel])
                g_VbiasScan[th][channel].SetPointError(n, 0, s_gaus_err[vb][th][channel])
                #pde = PDE[vb]
                pde = f_pde_vs_ov(vb - Vbreak)
                g_pde[th][channel].SetPoint(n, pde, s_gaus[vb][th][channel])
                g_pde[th][channel].SetPointError(n, err_pde[vb], s_gaus_err[vb][th][channel])
                n = n+1


for channel in channels:
    g_VbiasScan_opt[channel] = ROOT.TGraphErrors()
    g_pde_opt[channel] = ROOT.TGraphErrors()
    n = 0
    for vb in Vbias:
        th  = bestThreshold[vb][channel]
        if ( channel in s_gaus[vb][th] ):
            g_VbiasScan_opt[channel].SetPoint(n, vb - Vbreak, s_gaus[vb][th][channel])
            g_VbiasScan_opt[channel].SetPointError(n, 0, s_gaus_err[vb][th][channel])
            #pde = PDE[vb]
            pde = f_pde_vs_ov(vb - Vbreak)
            g_pde_opt[channel].SetPoint(n, pde, s_gaus[vb][th][channel])
            g_pde_opt[channel].SetPointError(n, err_pde[vb], s_gaus_err[vb][th][channel])
            n = n+1
            
    
canvasVb = {}
canvasPde = {}
canvasTh = {}
hdummyVb = ROOT.TH2F('','',10, 0., 7., 100,20,70)
hdummyPde = ROOT.TH2F('','',10,5,45,100,20,70)
hdummyTh = ROOT.TH2F('','',100,0, (thresholds[-1]+100)*kAdcToV,100,20,80)
    
legVb = ROOT.TLegend(0.65, 0.65, 0.89, 0.89)
legVb.SetBorderSize(0)
legPde = ROOT.TLegend(0.65, 0.65, 0.89, 0.89)
legPde.SetBorderSize(0)
legTh = ROOT.TLegend(0.65, 0.65, 0.89, 0.89)
legTh.SetBorderSize(0)

tChType = {}

        
# draw

# vs bias
for channel in channels:
    tChType[channel] = ROOT.TLatex( 0.13, 0.92, channelType[channel] )
    tChType[channel].SetNDC()
    tChType[channel].SetTextSize(0.035)

    if (angle == '4' or angle == '3' or angle == '2'):
        canvasVb[channel] = ROOT.TCanvas('scan_Vbias_%s_%smm'%(channel, angle),'scan_Vbias_%s_%smm'%(channel, angle))
    else:
        canvasVb[channel] = ROOT.TCanvas('scan_Vbias_%s_xyangle%s'%(channel, angle),'scan_Vbias_%s_xyangle%s'%(channel, angle))
    #canvasVb[channel].SetGridx()
    #canvasVb[channel].SetGridy()
    hdummyVb.GetXaxis().SetTitle('OV (V)')
    hdummyVb.GetYaxis().SetTitle('time resolution (ps)')
    hdummyVb.Draw()
    for i,th in enumerate(thresholds):
        if ( g_VbiasScan[th][channel] ):
            g_VbiasScan[th][channel].SetLineColor(i+1)
            g_VbiasScan[th][channel].SetLineWidth(2)
            g_VbiasScan[th][channel].SetMarkerColor(i+1)
            g_VbiasScan[th][channel].SetMarkerStyle(20)
            g_VbiasScan[th][channel].Draw('plsame')
            if (channel == 'BAR0' and g_VbiasScan[th][channel].GetN()>0):
                legVb.AddEntry(g_VbiasScan[th][channel], 'threshold = %.0f mV'%(th*kAdcToV), 'PL')
    legVb.Draw('same')
    tChType[channel].Draw()
    canvasVb[channel].SaveAs(outdir+'/'+canvasVb[channel].GetName()+'.png')
    canvasVb[channel].SaveAs(outdir+'/'+canvasVb[channel].GetName()+'.pdf')
    canvasVb[channel].SaveAs(outdir+'/'+canvasVb[channel].GetName()+'.C')
    #raw_input('ok?')


# vs pde
fitfun = {}
for channel in channels:
    if (angle == '4' or angle == '3' or angle == '2'):
        canvasPde[channel] = ROOT.TCanvas('scan_PDE_%s_%smm'%(channel, angle),'scan_PDE_%s_%smm'%(channel, angle))
    else:
        canvasPde[channel] = ROOT.TCanvas('scan_PDE_%s_xyangle%s'%(channel, angle),'scan_PDE_%s_xyangle%s'%(channel, angle))
    #canvasPde[channel].SetGridx()
    #canvasPde[channel].SetGridy()
    hdummyPde.GetXaxis().SetTitle('PDE (%)')
    hdummyPde.GetYaxis().SetTitle('time resolution (ps)')
    hdummyPde.Draw()
    fitfun[channel] = {}
    for i,th in enumerate(thresholds):
        if ( g_pde[th][channel] ):
            g_pde[th][channel].SetLineColor(i+1)
            g_pde[th][channel].SetLineWidth(2)
            g_pde[th][channel].SetMarkerColor(i+1)
            g_pde[th][channel].SetMarkerStyle(20)
            g_pde[th][channel].Draw('plsame')
    legVb.Draw('same')
    tChType[channel].Draw()
    canvasPde[channel].SaveAs(outdir+'/'+canvasPde[channel].GetName()+'.png')
    canvasPde[channel].SaveAs(outdir+'/'+canvasPde[channel].GetName()+'.pdf')
    canvasPde[channel].SaveAs(outdir+'/'+canvasPde[channel].GetName()+'.C')
    #raw_input('ok?')


# vs thr
for channel in channels:
    if (angle == '4' or angle == '3' or angle == '2'):
        canvasTh[channel] = ROOT.TCanvas('scan_LEthresholds_%s_%smm'%(channel, angle),'scan_LEthresholds_%s_%smm'%(channel,angle))
    else:
        canvasTh[channel] = ROOT.TCanvas('scan_LEthresholds_%s_xyangle%s'%(channel, angle),'scan_LEthresholds_%s_xyangle%s'%(channel,angle))
    #canvasTh[channel].SetGridx()
    #canvasTh[channel].SetGridy()
    #canvasTh[channel].SetLogx()
    hdummyTh.Draw()
    hdummyTh.GetXaxis().SetTitle('threshold (mV)')
    hdummyTh.GetYaxis().SetTitle('time resolution (ps)')
    for i,vb in enumerate(Vbias):
        l = [g_thrScan[vb][channel].GetY()[j] for j in range(0, g_thrScan[vb][channel].GetN())]
        indexBest = l.index(min(l))
        print channel, vb, 'Best threshold: %s ADC  --> sigma_t = %.1f ps'%(thresholds[indexBest], g_thrScan[vb][channel].GetY()[indexBest])
        if (g_thrScan[vb][channel]):
            g_thrScan[vb][channel].SetLineColor(i+1)
            g_thrScan[vb][channel].SetLineWidth(2)
            g_thrScan[vb][channel].SetMarkerColor(i+1)
            g_thrScan[vb][channel].SetMarkerStyle(20)
            g_thrScan[vb][channel].Draw('plsame')
            if (channel == 'BAR0' and g_thrScan[vb][channel].GetN()>0):
                legTh.AddEntry(g_thrScan[vb][channel], 'Vbias = %s V'%vb, 'PL')
    legTh.Draw('same')
    tChType[channel].Draw()
    if (useTdiff):
        canvasTh[channel].SaveAs(outdir+'/'+canvasTh[channel].GetName()+'_tDiff.png')
        canvasTh[channel].SaveAs(outdir+'/'+canvasTh[channel].GetName()+'_tDiff.pdf')
        canvasTh[channel].SaveAs(outdir+'/'+canvasTh[channel].GetName()+'_tDiff.C')
    else:
        canvasTh[channel].SaveAs(outdir+'/'+canvasTh[channel].GetName()+'.png')
        canvasTh[channel].SaveAs(outdir+'/'+canvasTh[channel].GetName()+'.pdf')
        canvasTh[channel].SaveAs(outdir+'/'+canvasTh[channel].GetName()+'.C')
    #raw_input('ok?')


#vs thr - all channels, only Vbias max
legThrAll = ROOT.TLegend(0.65, 0.65, 0.89, 0.89)
legThrAll.SetBorderSize(0)

if (angle == '4' or angle == '3' or angle == '2'):
    canvasThAll = ROOT.TCanvas('scan_LEthresholds_Vbias%d_%smm_FBK'%(Vbias[0], angle),'scan_LEthresholds_Vbias%d_%smm_FBK'%(Vbias[0],angle))
else:
    canvasThAll = ROOT.TCanvas('scan_LEthresholds_Vbias%d_xyangle%s_HPK'%(Vbias[0], angle),'scan_LEthresholds_Vbias%d_xyangle%s_HPK'%(Vbias[0],angle))
#canvasThAll.SetGridx()
#canvasThAll.SetGridy()
hdummyTh.Draw()
hdummyTh.GetXaxis().SetTitle('threshold (mV)')
hdummyTh.GetYaxis().SetTitle('time resolution (ps)')
for ich,channel in enumerate(channels):
    vb = Vbias[0]
    if (g_thrScan[vb][channel]):
        g_thrScan[vb][channel].SetMarkerStyle(20+ich)
        g_thrScan[vb][channel].SetMarkerSize(1.0)
        g_thrScan[vb][channel].SetMarkerColor(ROOT.kBlue+ich)
        g_thrScan[vb][channel].SetLineColor(ROOT.kBlue+ich)
        g_thrScan[vb][channel].SetLineStyle(1)
        g_thrScan[vb][channel].SetLineWidth(1)
        #g_thrScan[vb][channel].SetLineColor(i+1)
        #g_thrScan[vb][channel].SetLineWidth(2)
        #g_thrScan[vb][channel].SetMarkerColor(i+1)
        #g_thrScan[vb][channel].SetMarkerStyle(20)
        g_thrScan[vb][channel].Draw('plsame')
        legThrAll.AddEntry(g_thrScan[vb][channel], '%s'%channel, 'PL')
        tChType[channel].Draw()
legThrAll.Draw('same')
tChType[channel].Draw()
if (useTdiff):
    canvasThAll.SaveAs(outdir+'/'+canvasThAll.GetName()+'_tDiff.png')
    canvasThAll.SaveAs(outdir+'/'+canvasThAll.GetName()+'_tDiff.pdf')
    canvasThAll.SaveAs(outdir+'/'+canvasThAll.GetName()+'_tDiff.C')
else:
    canvasThAll.SaveAs(outdir+'/'+canvasThAll.GetName()+'.png')
    canvasThAll.SaveAs(outdir+'/'+canvasThAll.GetName()+'.pdf')
    canvasThAll.SaveAs(outdir+'/'+canvasThAll.GetName()+'.C')
#raw_input('ok?')




    

# vs bias - all channels , for optimal threshold
legVbAll = ROOT.TLegend(0.65, 0.65, 0.89, 0.89)
legVbAll.SetBorderSize(0)

if (angle == '4' or angle == '3' or angle == '2'):
    canvasVbAll = ROOT.TCanvas('scan_Vbias_LEthrOpt_%smm_FBK'%angle,'scan_Vbias_LEthrOpt_%smm_FBK'%angle)
else:
    canvasVbAll = ROOT.TCanvas('scan_Vbias_LEthrOpt_xyangle%s_HPK'%angle,'scan_Vbias_LEthrOpt_angle%s_HPK'%angle)
#canvasVbAll.SetGridx()
#canvasVbAll.SetGridy()
hdummyVb.Draw()
for ich, channel in enumerate(channels):
    if ( g_VbiasScan_opt[channel] ):
        g_VbiasScan_opt[channel].SetMarkerStyle(20+ich)
        g_VbiasScan_opt[channel].SetMarkerSize(1.0)
        g_VbiasScan_opt[channel].SetMarkerColor(ROOT.kBlue+ich)
        g_VbiasScan_opt[channel].SetLineColor(ROOT.kBlue+ich)
        g_VbiasScan_opt[channel].SetLineStyle(1)
        g_VbiasScan_opt[channel].SetLineWidth(1)
        g_VbiasScan_opt[channel].Draw('plsame')
        legVbAll.AddEntry(g_VbiasScan_opt[channel], '%s'%channel, 'PL')
        tChType[channel].Draw()
legVbAll.Draw('same')
canvasVbAll.SaveAs(outdir+'/'+canvasVbAll.GetName()+'.png')
canvasVbAll.SaveAs(outdir+'/'+canvasVbAll.GetName()+'.pdf')
canvasVbAll.SaveAs(outdir+'/'+canvasVbAll.GetName()+'.C')

# vs pde - all channels , for optimal threshold
legPdeAll = ROOT.TLegend(0.65, 0.65, 0.89, 0.89)
legPdeAll.SetBorderSize(0)
tPde = {}
if (angle == '4' or angle == '3' or angle == '2'):
    canvasPdeAll = ROOT.TCanvas('scan_PDE_LEthrOpt_%smm_FBK'%angle,'scan_PDE_LEthrOpt_%smm_FBK'%angle)
else:
    canvasPdeAll = ROOT.TCanvas('scan_PDE_LEthrOpt_xyangle%s_HPK'%angle,'scan_PDE_LEthrOpt_xyangle%s_HPK'%angle)
#canvasPdeAll.SetGridx()
#canvasPdeAll.SetGridy()
hdummyPde.Draw()
for ich, channel in enumerate(channels):
    if ( g_pde_opt[channel] ):
        g_pde_opt[channel].SetMarkerStyle(20+ich)
        g_pde_opt[channel].SetMarkerSize(1.0)
        g_pde_opt[channel].SetMarkerColor(ROOT.kBlue+ich)
        g_pde_opt[channel].SetLineColor(ROOT.kBlue+ich)
        g_pde_opt[channel].SetLineStyle(1)
        g_pde_opt[channel].SetLineWidth(1)

        fitfun[channel] = ROOT.TF1('fitfun_%s'%(channel), '[0]*1./pow(x,[1])', 0, 100)
        fitfun[channel].SetParameter(0,40)
        fitfun[channel].SetParameter(1,0.5)
        fitfun[channel].SetLineColor(ROOT.kBlue+ich)
        fitfun[channel].SetLineStyle(2)
        g_pde_opt[channel].Fit(fitfun[channel],'QR')
        print 'chi2/ndf = ', fitfun[channel].GetChisquare()/fitfun[channel].GetNDF()
        print '%s : 1/pow(x, alpha), alpha = %.02f +/- %.02f'%(channel, fitfun[channel].GetParameter(1), fitfun[channel].GetParError(1)) 
        tPde[channel] = ROOT.TLatex( 0.15, 0.15 + ich * 0.05, '#sigma_{t} #propto PDE^{-(%.2f +/- %.2f)}'%(fitfun[channel].GetParameter(1), fitfun[channel].GetParError(1)))
        
        #fitfun[channel] = ROOT.TF1('fitfun_%s'%(channel), 'sqrt([0]*[0]/pow(x,[1])/pow(x,[1]) + [2]*[2])', 0, 100)
        #fitfun[channel].SetParameter(0,40.)
        #fitfun[channel].SetParameter(1,0.5)
        #fitfun[channel].SetParameter(2,10.)
        #fitfun[channel].SetLineColor(ROOT.kBlue+ich)
        #fitfun[channel].SetLineStyle(2)
        #g_pde_opt[channel].Fit(fitfun[channel],'QR')
        #print 'chi2/ndf = ', fitfun[channel].GetChisquare()/fitfun[channel].GetNDF()
        #print '%s : [0]/pow(x, [1]) (+) [2] ' % channel
        #print '   [0] = %.02f +/- %.02f'%(fitfun[channel].GetParameter(0), fitfun[channel].GetParError(0)) 
        #print '   [1] = %.02f +/- %.02f'%(fitfun[channel].GetParameter(1), fitfun[channel].GetParError(1)) 
        #print '   [2] = %.02f +/- %.02f'%(fitfun[channel].GetParameter(2), fitfun[channel].GetParError(2)) 
        #tPde[channel] = ROOT.TLatex( 0.15, 0.15 + ich * 0.05, '#sigma_{t} #propto PDE^{-(%.2f +/- %.2f)}'%(fitfun[channel].GetParameter(1), fitfun[channel].GetParError(1)))
        
        #fitfun[channel] = ROOT.TF1('fitfun_%s'%(channel), 'sqrt([[0]*[0]/x + [1]*[1]/x/x + [2]*[2])', 0, 100)
        #fitfun[channel].SetParameter(0,30)
        #fitfun[channel].SetParameter(1,100)
        #fitfun[channel].SetParameter(2,1)
        #fitfun[channel].SetLineColor(ROOT.kBlue+ich)
        #fitfun[channel].SetLineStyle(2)
        #g_pde_opt[channel].Fit(fitfun[channel],'QR')
        #print 'chi2/ndf = ', fitfun[channel].GetChisquare()/fitfun[channel].GetNDF()
        #print '%s : [0]/pow(x, 0.5) (+) [1]/x (+) [2] ' % channel
        #print '   [0] = %.02f +/- %.02f'%(fitfun[channel].GetParameter(0), fitfun[channel].GetParError(0)) 
        #print '   [1] = %.02f +/- %.02f'%(fitfun[channel].GetParameter(1), fitfun[channel].GetParError(1)) 
        #print '   [2] = %.02f +/- %.02f'%(fitfun[channel].GetParameter(2), fitfun[channel].GetParError(2)) 
        #tPde[channel] = ROOT.TLatex( 0.15, 0.15 + ich * 0.05, '#sigma_{t} #propto PDE^{-(%.2f +/- %.2f)}'%(fitfun[channel].GetParameter(1), fitfun[channel].GetParError(1)))

        
        g_pde_opt[channel].Draw('psame')
        legPdeAll.AddEntry(g_pde_opt[channel], '%s'%channel, 'PL')
        legPdeAll.Draw('same')
        
        tPde[channel].SetNDC()
        tPde[channel].SetTextSize(0.035)
        tPde[channel].SetTextColor(ROOT.kBlue+ich)
        tPde[channel].Draw()
        tChType[channel].Draw()
canvasPdeAll.SaveAs(outdir+'/'+canvasPdeAll.GetName()+'.png')
canvasPdeAll.SaveAs(outdir+'/'+canvasPdeAll.GetName()+'.pdf')
canvasPdeAll.SaveAs(outdir+'/'+canvasPdeAll.GetName()+'.C')
#raw_input('ok?')
