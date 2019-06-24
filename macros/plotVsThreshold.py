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

useTdiff = False

angle = sys.argv[1]
outdir = sys.argv[2]

Vbreak = 66

Vbias = [72, 71, 70, 69, 68]
#Vbias = [43, 42, 41, 40, 39]
thresholds = [50, 100, 150, 200, 250, 300, 350, 400, 500, 700, 1000]# ADC
#thresholds = [20, 50, 100, 200, 300, 400, 500, 700, 1000, 2000] # ADC


PDE = { 72 : 38, 71 : 35, 70 : 30, 69 : 23, 68 : 15,
        43 : 35, 42 : 30, 41 : 27, 40 : 23, 39 : 17
}

kAdcToV = 1000./4096


channels = ['BAR%s'%i for i in range(0,3)]

channelType = {'BAR0':'LYSO:Ce 3x3x57 mm^{3} - HPK S12572-015',
               'BAR1':'LYSO:Ce 3x3x57 mm^{3} - HPK S12572-015',
               'BAR2':'LYSO:Ce 3x3x57 mm^{3} - HPK S12572-015'}

if (angle == '4'):
    channels = ['BAR%s'%i for i in range(0,1)]
    channelType = {'BAR0':'LYSO:Ce 3x4x57 mm^{3} - FBK thin-epi 5x5 mm^{2}'}
    Vbreak = 37
if (angle == '3'):
    channels = ['BAR%s'%i for i in range(0,1)]
    channelType = {'BAR0':'LYSO:Ce 3x3x50 mm^{3} - FBK thin-epi 5x5 mm^{2}'}
    Vbreak = 37
if (angle == '2'):
    channels = ['BAR%s'%i for i in range(0,1)]
    channelType = {'BAR0':'LYSO:Ce 3x2x57 mm^{3} - FBK thin-epi 5x5 mm^{2}'}
    Vbreak = 37
    
print channels

#prepare output dir
if (outdir != './'):
    os.system('mkdir %s'%outdir)
print 'Saving plots in %s'%outdir
#shutil.copy('index.php', '%s'%outdir)


s_eff  = {} # s_eff[Vbias][thr][channel]
s_gaus = {}
s_gaus_err = {}

sigmaPTK  = 0.014
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
        
        #fname = '../v1/output_3bars_Vbias%s_thr%sADC_xyangle90_runs6369-6401.root'%(vb, th)
        #fname = '../v1/output_3bars_Vbias%s_thr%sADC_xyangle90_runs6872-6913.root'%(vb, th)
        #fname = '../v4/output_3bars_Vbias%s_thr%sADC_xyangle90_runs6369-6401.root'%(vb, th)
        #fname = '../v4/output_3bars_Vbias%s_thr%sADC_xyangle90_runs6872-6913.root'%(vb, th)
        #fname = '../v4/output_3bars_Vbias%s_thr%sADC_xyangle90_runs7498-7516.root'%(vb, th)
        #fname = '../v4/output_3bars_Vbias%s_thr%sADC_xyangle90_runs7639-7679.root'%(vb, th)
        #fname = '../v4/output_3bars_Vbias%s_thr%sADC_xyangle90_runs7850-7877.root'%(vb, th)
        #fname = '../v4/output_3bars_Vbias%s_thr%sADC_xyangle%s.root'%(vb, th, angle)
        #fname = '../v4/biasScan/output_1bar_Vbias%s_thr%sADC_%smm.root'%(vb, th, angle)
        fname = '../v4/biasScan/output_3bars_Vbias%s_thr%sADC_xyangle%s.root'%(vb, th, angle)
        if (os.path.isfile(fname) ==  False): continue
        f = ROOT.TFile.Open(fname)

        for channel in channels:
            #get sigmaEff, sigmaGauss
            sigmaEff   = f.Get('h_effectiveSigmaAve_ampCorr_posCorr_%s'%channel).GetMean()
            h = f.Get('h_tAve_ampCorr_posCorr_'+ channel)
            fitfun = h.GetFunction('fitFunAve_ampCorr_posCorr_%s'%channel)
            if (useTdiff):
                sigmaEff   = f.Get('h_effectiveSigmaDiff_ampCorr_posCorr_%s'%channel).GetMean()
                h = f.Get('h_tDiff_ampCorr_posCorr_'+ channel)
                fitfun = h.GetFunction('fitFunDiff_ampCorr_posCorr_%s'%channel)
            if (fitfun == None):
                continue
            
            sigmaGaus  = fitfun.GetParameter(2)
            esigmaGaus = fitfun.GetParError(2)

            
            if (useTdiff):
                resolGaus = sigmaGaus*1000./2
                resolEff  = sigmaEff*1000./2
            else:
                resolGaus  = ROOT.TMath.Sqrt(sigmaGaus*sigmaGaus - sigmaPTK*sigmaPTK)*1000. # ps
                resolEff   = ROOT.TMath.Sqrt(sigmaEff*sigmaEff - sigmaPTK*sigmaPTK)*1000. # ps

            #if (math.isnan(resolGaus) or resolGaus > 500. or resolGaus < 15.): resolGaus = resolEff

            s_eff[vb][th][channel]  = resolEff
            s_gaus[vb][th][channel] = resolGaus
            s_gaus_err[vb][th][channel] = max(esigmaGaus*1000., 2.)
            #print channel, th, '    sigma_gaus = %.1f      sigma_eff = %.1f'%(s_gaus[vb][th][channel],  s_eff[vb][th][channel])
                       
        


# find optimal threshold for each Vbias
bestThreshold = {} # [vb][channel]
for vb in Vbias:
    bestThreshold[vb] = {}
    print vb
    for ch in channels:
        l = [ s_gaus[vb][th][ch] for th in thresholds ]
        bestIndex = l.index(min(l))
        bestThreshold[vb][ch] = thresholds[bestIndex]
        print ch, vb, bestThreshold[vb][ch]
       
    
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
                pde = PDE[vb]
                g_pde[th][channel].SetPoint(n, pde, s_gaus[vb][th][channel])
                g_pde[th][channel].SetPointError(n, 0, s_gaus_err[vb][th][channel])
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
            pde = PDE[vb]
            g_pde_opt[channel].SetPoint(n, pde, s_gaus[vb][th][channel])
            g_pde_opt[channel].SetPointError(n, 0, s_gaus_err[vb][th][channel])
            n = n+1
            
    
canvasVb = {}
canvasPde = {}
canvasTh = {}
hdummyVb = ROOT.TH2F('','',10, 0, 8,100,0,100)
hdummyPde = ROOT.TH2F('','',10,5,55,100,0,100)
hdummyTh = ROOT.TH2F('','',100,4, (thresholds[-1]+1000)*kAdcToV,100,0,100)
    
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
    canvasVb[channel].SetGridx()
    canvasVb[channel].SetGridy()
    #hdummyVb.GetXaxis().SetTitle('V_{bias} (V)')
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
                legVb.AddEntry(g_VbiasScan[th][channel], 'LE threshold = %.0f mV'%(th*kAdcToV), 'PL')
    legVb.Draw('same')
    tChType[channel].Draw()
    canvasVb[channel].SaveAs(outdir+'/'+canvasVb[channel].GetName()+'.png')
    canvasVb[channel].SaveAs(outdir+'/'+canvasVb[channel].GetName()+'.pdf')
    #raw_input('ok?')


# vs pde
fitfun = {}
for channel in channels:
    if (angle == '4' or angle == '3' or angle == '2'):
        canvasPde[channel] = ROOT.TCanvas('scan_PDE_%s_%smm'%(channel, angle),'scan_PDE_%s_%smm'%(channel, angle))
    else:
        canvasPde[channel] = ROOT.TCanvas('scan_PDE_%s_xyangle%s'%(channel, angle),'scan_PDE_%s_xyangle%s'%(channel, angle))
    canvasPde[channel].SetGridx()
    canvasPde[channel].SetGridy()
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
    #raw_input('ok?')


# vs thr
for channel in channels:
    if (angle == '4' or angle == '3' or angle == '2'):
        canvasTh[channel] = ROOT.TCanvas('scan_LEthresholds_%s_%smm'%(channel, angle),'scan_LEthresholds_%s_%smm'%(channel,angle))
    else:
        canvasTh[channel] = ROOT.TCanvas('scan_LEthresholds_%s_xyangle%s'%(channel, angle),'scan_LEthresholds_%s_xyangle%s'%(channel,angle))
    canvasTh[channel].SetGridx()
    canvasTh[channel].SetGridy()
    canvasTh[channel].SetLogx()
    hdummyTh.Draw()
    hdummyTh.GetXaxis().SetTitle('LE threshold (mV)')
    hdummyTh.GetYaxis().SetTitle('time resolution (ps)')
    for i,vb in enumerate(Vbias):
        print channel
        l = [g_thrScan[vb][channel].GetY()[i] for i in range(0, g_thrScan[vb][channel].GetN())]
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
    else:
        canvasTh[channel].SaveAs(outdir+'/'+canvasTh[channel].GetName()+'.png')
        canvasTh[channel].SaveAs(outdir+'/'+canvasTh[channel].GetName()+'.pdf')
    #raw_input('ok?')



# vs bias - all channels , for optimal threshold
legVbAll = ROOT.TLegend(0.65, 0.65, 0.89, 0.89)
legVbAll.SetBorderSize(0)

if (angle == '4' or angle == '3' or angle == '2'):
    canvasVbAll = ROOT.TCanvas('scan_Vbias_LEthrOpt_%smm'%angle,'scan_Vbias_LEthrOpt_%smm'%angle)
else:
    canvasVbAll = ROOT.TCanvas('scan_Vbias_LEthrOpt_xyangle%s'%angle,'scan_Vbias_LEthrOpt_angle%s'%angle)
canvasVbAll.SetGridx()
canvasVbAll.SetGridy()
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
        legVbAll.Draw('same')
        tChType[channel].Draw()
canvasVbAll.SaveAs(outdir+'/'+canvasVbAll.GetName()+'.png')
canvasVbAll.SaveAs(outdir+'/'+canvasVbAll.GetName()+'.pdf')

# vs pde - all channels , for optimal threshold
legPdeAll = ROOT.TLegend(0.65, 0.65, 0.89, 0.89)
legPdeAll.SetBorderSize(0)
tPde = {}
if (angle == '4' or angle == '3' or angle == '2'):
    canvasPdeAll = ROOT.TCanvas('scan_PDE_LEthrOpt_%smm'%angle,'scan_PDE_LEthrOpt_%smm'%angle)
else:
    canvasPdeAll = ROOT.TCanvas('scan_PDE_LEthrOpt_xyangle%s'%angle,'scan_PDE_LEthrOpt_xyangle%s'%angle)
canvasPdeAll.SetGridx()
canvasPdeAll.SetGridy()
hdummyPde.Draw()
for ich, channel in enumerate(channels):
    if ( g_pde_opt[channel] ):
        g_pde_opt[channel].SetMarkerStyle(20+ich)
        g_pde_opt[channel].SetMarkerSize(1.0)
        g_pde_opt[channel].SetMarkerColor(ROOT.kBlue+ich)
        g_pde_opt[channel].SetLineColor(ROOT.kBlue+ich)
        g_pde_opt[channel].SetLineStyle(1)
        g_pde_opt[channel].SetLineWidth(1)
        fitfun[channel] = ROOT.TF1('fitfun_%s'%(channel), '[1]*1./pow(x,[0])', 0, 100)
        fitfun[channel].SetLineColor(ROOT.kBlue+ich)
        fitfun[channel].SetLineStyle(2)
        g_pde_opt[channel].Fit(fitfun[channel],'QR')
        print '%s : 1/pow(x, alpha), alpha = %.02f +/- %.02f'%(channel, fitfun[channel].GetParameter(0), fitfun[channel].GetParError(0)) 
        g_pde_opt[channel].Draw('psame')
        legPdeAll.AddEntry(g_pde_opt[channel], '%s'%channel, 'PL')
        legPdeAll.Draw('same')
        tPde[channel] = ROOT.TLatex( 0.15, 0.15 + ich * 0.05, '#sigma_{t} ~  PDE^{-(%.2f +/- %.2f)}'%(fitfun[channel].GetParameter(0), fitfun[channel].GetParError(0)))
        tPde[channel].SetNDC()
        tPde[channel].SetTextSize(0.035)
        tPde[channel].SetTextColor(ROOT.kBlue+ich)
        tPde[channel].Draw()
        tChType[channel].Draw()
canvasPdeAll.SaveAs(outdir+'/'+canvasPdeAll.GetName()+'.png')
canvasPdeAll.SaveAs(outdir+'/'+canvasPdeAll.GetName()+'.pdf')
raw_input('ok?')
