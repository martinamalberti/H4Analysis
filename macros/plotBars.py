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
ROOT.gErrorIgnoreLevel = ROOT.kWarning;

Vbias = sys.argv[1]
thr   = sys.argv[2]
angle = sys.argv[3]
subfolder = sys.argv[4]

#filename = '../v11/biasScan/output_3bars_Vbias%s_thr%sADC_xyangle%s.root'%(Vbias, thr, angle)
#filename = '../v11/biasScan/output_1bar_Vbias%s_thr%sADC_%smm.root'%(Vbias, thr, angle)
filename = '../v11/output_3bars_Vbias%s_thr%sADC_xyangle%s_runs6872-6913.root'%(Vbias, thr, angle)
#filename = '../v11/xyAngleScan/output_3bars_Vbias%s_thr%sADC_xyangle%s.root'%(Vbias, thr, angle)
#filename = '../v11/materialLeveling/output_1bar_Vbias%s_thr%sADC_%smm.root'%(Vbias, thr, angle)
#filename = '../v11/yzAngleScan/output_3bars_Vbias%s_thr%sADC_yzangle%s.root'%(Vbias, thr, angle)

sigmaPTK = 0.0123 # after corrections
            

if ('3bars' in filename):
    channels = ['BAR%s'%i for i in range(0,3)]
else:
    channels = ['BAR%s'%i for i in range(0,1)]
print channels

channelType = {}
if ('xyangle' in filename):
    channelType = {'BAR0':'LYSO:Ce 3x3x57 mm^{3} - HPK S12572-015',
                   'BAR1':'LYSO:Ce 3x3x57 mm^{3} - HPK S12572-015',
                   'BAR2':'LYSO:Ce 3x3x57 mm^{3} - HPK S12572-015'}
if ('yzangle45' in filename):
    channelType = {'BAR0':'LYSO:Ce 3x3x57 mm^{3} - HPK S12572-015',
                   'BAR1':'LYSO:Ce 3x3x57 mm^{3} - HPK S12572-015',
                   'BAR2':'LYSO:Ce 3x3x57 mm^{3} - HPK S12572-015'}
if ('yzangle60' in filename or 'yzangle30' in filename or 'yzangle90' in filename):
    channelType = {'BAR0':'LYSO:Ce 2x3x57 mm^{3} - HPK S12572-015',
                   'BAR1':'LYSO:Ce 2x3x57 mm^{3} - HPK S12572-015',
                   'BAR2':'LYSO:Ce 2x3x57 mm^{3} - HPK S12572-015'}
if ('1bar' in filename):
    thickness = sys.argv[3]
    channelType = {'BAR0':'LYSO:Ce %sx3x57 mm^{3} - FBK 5x5 mm^{2}'%thickness}
    if (thickness == 3):
        channelType = {'BAR0':'LYSO:Ce %sx3x50 mm^{3} - FBK 5x5 mm^{2}'%thickness}

#open file and look for directory with plots
f = ROOT.TFile.Open(filename);

#prepare output dir
if (subfolder != './'):
    os.system('mkdir %s'%subfolder)
outdir = '%s/Vbias%s_thr%sADC_xyangle%s/'%(subfolder,Vbias, thr, angle)
if ('yzangle' in filename):
    outdir = '%s/Vbias%s_thr%sADC_yzangle%s/'%(subfolder,Vbias, thr, angle)
if ('1bar' in filename):
    outdir = '%s/Vbias%s_thr%sADC_%smm/'%(subfolder,Vbias, thr, angle)
print 'Saving plots in %s'%outdir
os.system('mkdir %s'%outdir)
shutil.copy('index.php', '%s'%outdir)


#histograms to plot
histograms = {'h_beamXY'        : ['x(mm)', 'y(mm)'],
              'p2_eff_vs_posXY' : ['x(mm)', 'y(mm)'],
              'p_eff_vs_posX'   : ['x(mm)', 'efficiency'],
              'p_eff_vs_posY'   : ['y(mm)', 'efficiency'],

              'p2_eff_vs_posXY_Ref' : ['x(mm)', 'y(mm)'],
              'p2_ampRef_vs_posXY'  : ['x(mm)', 'y(mm)'],
              'p2_timeRef_vs_posXY' : ['x(mm)', 'y(mm)'],
              'p_timeRef_vs_ampRef' : ['amplitude (V)', 'time (ns)'],
              'p_timeRef_vs_posX'   : ['x(mm)', 'time (ns)'],

              'h2_tDiff_vs_posX': ['x(mm)', 't_{right} - t_{left} (ns)'],
              
              'h_ampRef_nocuts' : ['amplitude (V)','events'],
              'h_ampRef'        : ['amplitude (V)','events'],
              'h_timeRef'       : ['time (ns)','events'],
              'h_brmsRef'       : ['baseline rms (ADC)','events'],
              
              'h_ampL_nocuts'   : ['amplitude (V)','events'],
              'h_ampL'          : ['amplitude (V)','events'],
              'h_timeL'         : ['time (ns)','events'],
              'h_brms_timeL'    : ['baseline RMS (ADC)','events'],
              
              'h_ampR_nocuts'   : ['amplitude (V)','events'],
              'h_ampR'          : ['amplitude (V)','events'],
              'h_timeR'         : ['time (ns)','events'],
              'h_brms_timeR'    : ['baseline RMS (ADC)','events'],
              
              'p_tL_vs_amp'     : ['amplitude (V)','t_{left} - t_{MCP} (ns)'],
              'p_tR_vs_amp'     : ['amplitude (V)','t_{right} - t_{MCP} (ns)'],

              'p_tL_vs_pulseInt'     : ['pulse integral (V)','t_{left} - t_{MCP} (ns)'],
              'p_tR_vs_pulseInt'     : ['pulse integral (V)','t_{right} - t_{MCP} (ns)'],

              'p_tL_ampCorr_vs_amp'      : ['amplitude (V)','t_{left} - t_{MCP} (ns)'],
              'p_tR_ampCorr_vs_amp'      : ['amplitude (V)','t_{right} - t_{MCP} (ns)'],

}

histograms1 = {'h_tL'            : ['t_{left} - t_{MCP} (ns)', 'events'],
               'h_tR'            : ['t_{right} - t_{MCP} (ns)', 'events'],
               'h_tAve'          : ['t_{ave} - t_{MCP} (ns)', 'events'],
              
               'h_tL_ampCorr'    : ['t_{left} - t_{MCP} (ns)', 'events'],
               'h_tR_ampCorr'    : ['t_{right} - t_{MCP} (ns)', 'events'],
               'h_tAve_ampCorr'  : ['t_{ave} - t_{MCP} (ns)', 'events'],
               'h_tDiff_ampCorr' : ['t_{right} - t_{left} (ns)', 'events'],
               'h_tAve_pulseIntCorr'  : ['t_{ave} - t_{MCP} (ns)', 'events'],
               'h_tDiff_pulseIntCorr' : ['t_{right} - t_{left} (ns)', 'events'],
               
               'p_tL_ampCorr_vs_posX'     : ['x (mm)','t_{left} - t_{MCP} (ns)'],
               'p_tR_ampCorr_vs_posX'     : ['x (mm)','t_{right} - t_{MCP} (ns)'],
               'p_tAve_ampCorr_vs_posX'   : ['x (mm)','t_{ave} - t_{MCP} (ns)'],

               'p_tL_pulseIntCorr_vs_posX'     : ['x (mm)','t_{left} - t_{MCP} (ns)'],
               'p_tR_pulseIntCorr_vs_posX'     : ['x (mm)','t_{right} - t_{MCP} (ns)'],
               'p_tAve_pulseIntCorr_vs_posX'   : ['x (mm)','t_{ave} - t_{MCP} (ns)'],

              }


c = {}
h = {}
hnew = {}
t0 = {}
t1 = {}
t2 = {}
tChType = {}
tlandau = {}

tx = {}
ty = {}

lineTimeMin = ROOT.TLine(38, 0, 38 , 9999)
lineTimeMax = ROOT.TLine(50, 0, 50 , 9999)
if ('1bar' in filename):
    lineTimeMax = ROOT.TLine(60, 0, 60 , 9999)

for l in lineTimeMin,lineTimeMax:
    l.SetLineStyle(2)
    l.SetLineColor(ROOT.kRed)

#plot MCP ref
for ih, hname in enumerate(['h_beamXY','h_ampRef_nocuts','h_timeRef', 'h_brmsRef', 'p2_eff_vs_posXY_Ref', 'p2_timeRef_vs_posXY', 'p2_ampRef_vs_posXY', 'p_timeRef_vs_ampRef', 'p_timeRef_vs_posX']):

    # prepare canvas
    cname = hname.replace('h_','')
    c[hname] = ROOT.TCanvas(cname,cname)
    c[hname].SetTickx()
    c[hname].SetTicky()

    # get histograms
    h[hname] = f.Get(hname)

    #set histograms attributes
    h[hname].GetXaxis().SetTitle( histograms[hname][0] )    
    h[hname].GetYaxis().SetTitle( histograms[hname][1] )
  
    #draw histos
    c[hname].cd()
    if (hname.startswith('p2_') or 'beam' in hname):
        h[hname].GetXaxis().SetRangeUser(-15,35)
        h[hname].GetYaxis().SetRangeUser(5,45)
        h[hname].Draw('colz')
    elif (hname.startswith('p_')):
        h[hname].GetYaxis().SetRangeUser(h[hname].GetMean(2)-1,h[hname].GetMean(2)+1)
        if ('vs_posX' in hname):
            h[hname].GetXaxis().SetRangeUser(-15,35)
        h[hname].Draw('')
    elif (hname.startswith('h_amp')):
        c[hname].SetLogy()
        h[hname].Rebin(1)
        h['h_ampRef_nocuts'].Draw()
        h['h_ampRef'] = f.Get('h_ampRef')
        h['h_ampRef'].SetFillColor(ROOT.kGray)
        h['h_ampRef'].SetFillStyle(1001)
        #h['h_ampRef'].SetFillStyle(1001)
        h['h_ampRef'].Draw('same')
        h['h_ampRef_nocuts'].Draw('same')
        fitpeak = h['h_ampRef_nocuts'].GetFunction('ffitRef')
        peak = fitpeak.GetParameter(1)
        tpeak = ROOT.TLatex( 0.60, 0.85, 'Peak = %.0f mV'%(peak*1000.) )
        tpeak.SetNDC()
        tpeak.SetTextSize(0.030)
        tpeak.Draw()
    elif (hname.startswith('h_brms')):
         h[hname].SetFillColor(ROOT.kGray)
         h[hname].SetFillStyle(1001)
         h[hname].Draw('')
    else:
        h[hname].Draw('')
    
    c[hname].SaveAs(outdir+'/'+c[hname].GetName()+'.png')
    c[hname].SaveAs(outdir+'/'+c[hname].GetName()+'.pdf')
    c[hname].SaveAs(outdir+'/'+c[hname].GetName()+'.C')
    #raw_input('ok?')



print 'Preparing plots for each bar....'


for hname, attr in histograms.items():
    for ich, channel in enumerate(channels):

        hnameCh= hname + '_' + channel

        # get histograms
        if (f.GetListOfKeys().Contains(hnameCh) == False): continue
        h[hnameCh] = f.Get(hnameCh)



        
for hname, attr in  histograms.items():

    if ('Ref' in hname or 'beam' in hname):
        continue
  
    for ich, channel in enumerate(channels):
        hnameCh= hname + '_' + channel
      
        # prepare canvas
        cname = hnameCh.replace('h_','').replace('h2_','').replace('p_t','t').replace('p2_','')

        c[hnameCh] = ROOT.TCanvas(cname,cname)
        c[hnameCh].SetTickx()
        c[hnameCh].SetTicky()

        #if (hnameCh.startswith('p_')):
            #c[hnameCh].SetGridx()
            #c[hnameCh].SetGridy()

        # get spatial resolution
        sigmaPos = 0
        esigmaPos = 0
        if ('p_eff_vs_pos' in hnameCh):
            fitErf = ROOT.TF1('fitErf','[2]*TMath::Erf((x-[0])/[1])+[3]', 15, 30)
            bin0 = h[hnameCh].FindFirstBinAbove(0.50)
            x0   = h[hnameCh].GetBinCenter(bin0)
            fitErf.SetParameters(x0, 0.05, 0.2, 0.1)
            fitErf.SetRange(x0 - 2. , x0 + 2.)
            h[hnameCh].Fit('fitErf','QR')
            h[hnameCh].GetYaxis().SetRangeUser(0,1)
            sigmaPos = fitErf.GetParameter(1)
            esigmaPos = fitErf.GetParError(1)
            
        # set histograms attributes    
        h[hnameCh].GetXaxis().SetTitle( histograms[hname][0] )    
        h[hnameCh].GetYaxis().SetTitle( histograms[hname][1] )    
        if (hnameCh.startswith('p2_t')):
            h[hnameCh].GetZaxis().SetTitle('#Delta t (ns)')
        if (hnameCh.startswith('p2_amp')):
            h[hnameCh].GetZaxis().SetTitle('amplitude (V)')
        if (hnameCh.startswith('p2_eff')):
            h[hnameCh].GetZaxis().SetTitle('efficiency')
                
        if (hnameCh.startswith('h_amp') ):
            c[hnameCh].SetLogy()
            h[hnameCh].GetXaxis().SetRangeUser( 0., 1.)
            if ('cuts' not in hnameCh):
                h[hnameCh].SetFillColor(ROOT.kGray)
                h[hnameCh].SetFillStyle(1001)
                
        if ('h_time' in hnameCh ):
            c[hnameCh].SetLogy()
            
        if ('p_tL' in hnameCh or 'p_tR' in hnameCh or 'p_tAve' in hnameCh):
            h[hnameCh].SetMarkerStyle(20)
            h[hnameCh].SetMarkerSize(0.7)
            if ('vs_amp' in hnameCh):
                h[hnameCh].GetXaxis().SetRangeUser( h[hnameCh].GetMean()-3*h[hnameCh].GetRMS(), h[hnameCh].GetMean()+6*h[hnameCh].GetRMS() )
                h[hnameCh].GetYaxis().SetRangeUser( h[hnameCh].GetMean(2)-5.0*h[hnameCh].GetRMS(2), h[hnameCh].GetMean(2)+3.*h[hnameCh].GetRMS(2))
            if ('vs_pulseInt' in hnameCh):
                h[hnameCh].GetXaxis().SetRangeUser( h[hnameCh].GetMean()-5*h[hnameCh].GetRMS(),  h[hnameCh].GetMean()+6*h[hnameCh].GetRMS())
                h[hnameCh].GetYaxis().SetRangeUser( h[hnameCh].GetMean(2)-5.0*h[hnameCh].GetRMS(2), h[hnameCh].GetMean(2)+3.*h[hnameCh].GetRMS(2))
            if ('vs_posX' in hnameCh):
                bmin = h[hnameCh].FindFirstBinAbove(0)-2
                xmin = h[hnameCh].GetBinCenter(bmin)
                bmax = h[hnameCh].FindLastBinAbove(0)+2
                xmax = h[hnameCh].GetBinCenter(bmax)
                h[hnameCh].GetXaxis().SetRangeUser(xmin,xmax)
                h[hnameCh].GetXaxis().SetRangeUser( h[hnameCh].GetMean(1)-3*h[hnameCh].GetRMS(), h[hnameCh].GetMean(1)+3*h[hnameCh].GetRMS())
                h[hnameCh].GetYaxis().SetRangeUser( h[hnameCh].GetMean(2)-0.25, h[hnameCh].GetMean(2)+0.25)
            if ('vs_posY' in hnameCh):
                bmin = h[hnameCh].FindFirstBinAbove(0)
                xmin = h[hnameCh].GetBinCenter(bmin)
                bmax = h[hnameCh].FindLastBinAbove(0)
                xmax = h[hnameCh].GetBinCenter(bmax)
                h[hnameCh].GetXaxis().SetRangeUser( xmin, xmax)
                h[hnameCh].GetYaxis().SetRangeUser( h[hnameCh].GetMean(2)-0.25, h[hnameCh].GetMean(2)+0.25)

        if ('p2' in hnameCh):
            #xmin = h['p2_eff_vs_posXY_%s'%channel].GetMean(1)-30.
            #xmax = h['p2_eff_vs_posXY_%s'%channel].GetMean(1)+30.
            ymin = h['p2_eff_vs_posXY_%s'%channel].GetMean(2)-10.
            ymax = h['p2_eff_vs_posXY_%s'%channel].GetMean(2)+10.
            xmin = -10
            xmax = 30
            h[hnameCh].GetXaxis().SetRangeUser( xmin, xmax )
            h[hnameCh].GetYaxis().SetRangeUser( ymin, ymax )
            h[hnameCh].GetZaxis().SetRangeUser( 0, 1)

        if ('h2_tDiff' in hnameCh):
            ymin = h[hnameCh].GetMean(2)-1.
            ymax = h[hnameCh].GetMean(2)+1.
            xmin = -10
            xmax = 40
            h[hnameCh].GetXaxis().SetRangeUser( xmin, xmax )
            h[hnameCh].GetYaxis().SetRangeUser( ymin, ymax )

        if ('risetime' in hnameCh):
            xmin =  h[hnameCh].GetMean()-30.
            xmax =  h[hnameCh].GetMean()+30.
            h[hnameCh].GetXaxis().SetRangeUser( xmin, xmax )
            h[hnameCh].GetYaxis().SetRangeUser( 0.2, 1 )

            
        #draw histos        
        c[hnameCh].cd()

        if ('p2' in hnameCh or 'h2' in hnameCh):
            h[hnameCh].Draw('colz')
        elif ('p_eff_vs_pos' in hnameCh):
            h[hnameCh].GetYaxis().SetRangeUser(0,1)
            if ('X' in hnameCh):
                tx[hnameCh] = ROOT.TLatex( 0.15, 0.84,'#sigma_{x} = %.2f +/- %.2f'%(sigmaPos, esigmaPos) )
            else:
                tx[hnameCh] = ROOT.TLatex( 0.15, 0.84,'#sigma_{y} = %.2f +/- %.2f'%(sigmaPos, esigmaPos) )
            tx[hnameCh].SetNDC()
            tx[hnameCh].SetTextSize(0.030)
            tx[hnameCh].Draw()
        elif (hname.startswith('h_amp') and 'cuts' not in hname):
            h[hname+'_nocuts_%s'%channel].Rebin(1)
            h[hname+'_%s'%channel].Rebin(1)
            h[hname+'_nocuts_%s'%channel].Draw()
            h[hname+'_%s'%channel].Draw('same')
            if ('ampR' in hnameCh):
                flandau = h[hname+'_nocuts_%s'%channel].GetFunction('fLandauR_%s'%channel)
            if ('ampL' in hnameCh):
                flandau = h[hname+'_nocuts_%s'%channel].GetFunction('fLandauL_%s'%channel)
            mipPeak = flandau.GetParameter(1)
            tlandau[hnameCh] = ROOT.TLatex( 0.60, 0.85, 'Landau MPV = %.0f mV'%(mipPeak*1000.) )
            tlandau[hnameCh].SetNDC()
            tlandau[hnameCh].SetTextSize(0.030)
            tlandau[hnameCh].Draw()
        else:
            h[hnameCh].Draw('')
            
        tChType[hnameCh] = ROOT.TLatex( 0.13, 0.92, channelType[channel] )
        tChType[hnameCh].SetNDC()
        tChType[hnameCh].SetTextSize(0.030)
        tChType[hnameCh].Draw()

        if ('h_time' in hnameCh):
            fG=h[hnameCh].GetFunction("fG")
            tmin = fG.GetParameter(1)-3*fG.GetParameter(2)
            tmax = fG.GetParameter(1)+3*fG.GetParameter(2)
            lineTimeMin = ROOT.TLine(tmin, 0, tmin  , 9999)
            lineTimeMax = ROOT.TLine(tmax, 0, tmax , 9999)
            lineTimeMin.SetY2(h[hnameCh].GetMaximum())
            lineTimeMax.SetY2(h[hnameCh].GetMaximum())
            lineTimeMin.SetLineStyle(2)
            lineTimeMin.SetLineColor(ROOT.kRed)
            lineTimeMax.SetLineStyle(2)
            lineTimeMax.SetLineColor(ROOT.kRed)
            lineTimeMin.Draw()
            lineTimeMax.Draw()
        
        c[hnameCh].cd()
        c[hnameCh].Update()


        if ('nocuts' not in hnameCh):
            c[hnameCh].SaveAs(outdir+'/'+c[hnameCh].GetName()+'.png')
            c[hnameCh].SaveAs(outdir+'/'+c[hnameCh].GetName()+'.pdf')
            c[hnameCh].SaveAs(outdir+'/'+c[hnameCh].GetName()+'.C')
        #raw_input('ok?')



# amplitude vs x for left and right SiPM
for ich,channel in enumerate(channels):
    hnameCh ='p_amp_vs_posX_%s'%channel
    hnameChL='p_ampL_vs_posX_%s'%channel
    hnameChR='p_ampR_vs_posX_%s'%channel

    cname = hnameCh

    c[hnameCh] = ROOT.TCanvas(cname,cname)
    c[hnameCh].SetTickx()
    #c[hnameCh].SetTicky()
    #c[hnameCh].SetGridy()
 
    h[hnameChL] = f.Get(hnameChL)
    h[hnameChR] = f.Get(hnameChR)
    
    h[hnameChL].SetLineColor(ROOT.kBlue)
    h[hnameChL].SetMarkerColor(ROOT.kBlue)
    
    h[hnameChR].SetLineColor(ROOT.kRed)
    h[hnameChR].SetMarkerColor(ROOT.kRed)

    ymax = 0.5*(h[hnameChL].GetMean(2)+h[hnameChR].GetMean(2)) + 0.1
    ymin = 0.5*(h[hnameChL].GetMean(2)+h[hnameChR].GetMean(2)) - 0.1
    
    h[hnameChL].GetXaxis().SetRangeUser( h[hnameChL].GetMean()-20, h[hnameChL].GetMean()+20)
    h[hnameChL].GetYaxis().SetRangeUser( ymin, ymax )
    h[hnameChL].GetXaxis().SetTitle('x (mm)')
    h[hnameChL].GetYaxis().SetTitle('average amplitude (V)')
    h[hnameChL].Draw()
    h[hnameChR].Draw('same')

    tChType[hnameCh] = ROOT.TLatex( 0.13, 0.92, channelType[channel] )
    tChType[hnameCh].SetNDC()
    tChType[hnameCh].SetTextSize(0.030)
    tChType[hnameCh].Draw()

    c[hnameCh].SaveAs(outdir+'/'+c[hnameCh].GetName()+'.png')
    c[hnameCh].SaveAs(outdir+'/'+c[hnameCh].GetName()+'.pdf')
    c[hnameCh].SaveAs(outdir+'/'+c[hnameCh].GetName()+'.C')
    #raw_input('ok?')
     

# draw plots for tL, tR, tAve
flinL = {}
flinR = {}
flinAve = {}

legend = ROOT.TLegend(0.7,0.7,0.89,0.89)
legend.SetBorderSize(0)

fitfunAve = None
fitfunL = None
fitfunR = None


for ich,channel in enumerate(channels):

    for ih, hname in enumerate(['h_tAve_ampCorr', 'p_tAve_ampCorr_vs_posX', 'p_tAve_ampCorr_vs_posXc', 'g_tResolGausAve_ampCorr',
                                #'h_tAve_ampCorr_tDiffCorr', 'p_tAve_ampCorr_tDiffCorr_vs_posX', 'p_tAve_ampCorr_tDiffCorr_vs_tDiff', 'p_tAve_ampCorr_tDiffCorr_vs_posXc', 'g_tResolGausAve_ampCorr_tDiffCorr',
                                'h_tAve_ampCorr_posCorr', 'p_tAve_ampCorr_posCorr_vs_posX', 'p_tAve_ampCorr_posCorr_vs_posXc', 'g_tResolGausAve_ampCorr_posCorr',
                                'h_tAve_pulseIntCorr', 'p_tAve_pulseIntCorr_vs_posX', 'p_tAve_pulseIntCorr_vs_posXc', 'g_tResolGausAve_pulseIntCorr',
                                #'h_tAve_pulseIntCorr_posCorr', 'p_tAve_pulseIntCorr_posCorr_vs_posX', 'p_tAve_pulseIntCorr_posCorr_vs_posXc', 'g_tResolGausAve_pulseIntCorr_posCorr'
    ]):
     

        hnameCh= hname + '_' + channel
        hnameChL= hnameCh.replace('AveResW','Ave').replace('AveAmpW','Ave').replace('Ave','L').replace('_tDiffCorr','').replace('_posCorr','')
        hnameChR= hnameCh.replace('AveResW','Ave').replace('AveAmpW','Ave').replace('Ave','R').replace('_tDiffCorr','').replace('_posCorr','')
        
        cname = hnameCh.replace('h_','').replace('h2_','').replace('p_t','t').replace('p2_','')
        
        c[hnameCh] = ROOT.TCanvas(cname,cname)
        c[hnameCh].SetTickx()
        c[hnameCh].SetTicky()
        #if (hnameCh.startswith('p_')):
            #c[hnameCh].SetGridy()

        # get histograms
        h[hnameCh]  = f.Get(hnameCh)
        h[hnameChL] = f.Get(hnameChL)
        h[hnameChR] = f.Get(hnameChR)

        h[hnameCh].SetLineColor(ROOT.kBlack)
        h[hnameChL].SetLineColor(ROOT.kBlue)
        h[hnameChR].SetLineColor(ROOT.kRed)

        h[hnameCh].SetMarkerColor(ROOT.kBlack)
        h[hnameChL].SetMarkerColor(ROOT.kBlue)
        h[hnameChR].SetMarkerColor(ROOT.kRed)

        h[hnameCh].SetMarkerStyle(20)
        h[hnameChL].SetMarkerStyle(20)
        h[hnameChR].SetMarkerStyle(20)

        h[hnameCh].SetMarkerSize(0.7)
        h[hnameChL].SetMarkerSize(0.7)
        h[hnameChR].SetMarkerSize(0.7)


        if ( hnameCh.startswith('h_tAve') ):
            h[hnameCh].GetYaxis().SetRangeUser(0,h[hnameCh].GetMaximum()*1.5  )
            h[hnameCh].GetXaxis().SetRangeUser(  h[hnameChL].GetMean() - 0.5, h[hnameChR].GetMean()+ 0.5)
            h[hnameCh].GetXaxis().SetTitle('t_{ave} - t_{MCP} (ns)')
            
            #compute time resolution as sigmaGauss (-) sigmaMCP
            fitfunAve  = h[hnameCh].GetFunction(hnameCh.replace('h_tAve','fitFunAve'))
            fitfunAve.SetLineColor(ROOT.kBlack)
            sigmaGaus  = fitfunAve.GetParameter(2)
            sigmaEff   = f.Get(hnameCh.replace('h_t','h_effectiveSigma')).GetMean()
            resolGaus  = ROOT.TMath.Sqrt(sigmaGaus*sigmaGaus - sigmaPTK*sigmaPTK)*1000. # ps
            resolGausErr = fitfunAve.GetParError(2)*1000.*sigmaGaus*1000./resolGaus
            resolEff   = ROOT.TMath.Sqrt(sigmaEff*sigmaEff - sigmaPTK*sigmaPTK)*1000.
            print '%s, resol_gaus = %.1f ps, eff_sigma = %.1f ps'%(channel, resolGaus, resolEff)

            fitfunL     = h[hnameChL].GetFunction(hnameChL.replace('h_tL','fitFunL'))            
            fitfunL.SetLineColor(ROOT.kBlue)
            sigmaGausL  = fitfunL.GetParameter(2)
            sigmaEffL   = f.Get('h_effectiveSigmaL_ampCorr_%s'%channel).GetMean()
            if ('pulseIntCorr' in hnameChL):  sigmaEffL   = f.Get('h_effectiveSigmaL_pulseIntCorr_%s'%channel).GetMean()
            resolGausL  = ROOT.TMath.Sqrt(sigmaGausL*sigmaGausL - sigmaPTK*sigmaPTK)*1000.
            resolGausErrL = fitfunL.GetParError(2)*1000.*sigmaGausL*1000./resolGausL
            resolEffL   = ROOT.TMath.Sqrt(sigmaEffL*sigmaEffL - sigmaPTK*sigmaPTK)*1000.
            print '%s, resol_gaus = %.1f ps, eff_sigma = %.1f ps'%(channel, resolGausL, resolEffL)

            fitfunR     = h[hnameChR].GetFunction(hnameChR.replace('h_tR','fitFunR'))            
            fitfunR.SetLineColor(ROOT.kRed)
            sigmaGausR  = fitfunR.GetParameter(2)
            sigmaEffR   = f.Get('h_effectiveSigmaR_ampCorr_%s'%channel).GetMean()
            if ('pulseIntCorr' in hnameChR): sigmaEffR   = f.Get('h_effectiveSigmaR_pulseIntCorr_%s'%channel).GetMean()
            resolGausR  = ROOT.TMath.Sqrt(sigmaGausR*sigmaGausR - sigmaPTK*sigmaPTK)*1000.
            resolGausErrR = fitfunR.GetParError(2)*1000.*sigmaGausR*1000./resolGausR
            resolEffR   = ROOT.TMath.Sqrt(sigmaEffR*sigmaEffR - sigmaPTK*sigmaPTK)*1000.
            print '%s, resol_gaus = %.1f ps, eff_sigma = %.1f ps'%(channel, resolGausR, resolEffR)

            tResAve = ROOT.TLatex( 0.15, 0.84,'t_{ave}  :  #sigma_{eff} = %.1f ps,  #sigma_{gaus} = %.1f #pm %.1f ps'%(resolEff, resolGaus, resolGausErr) )
            tResAve.SetNDC()
            tResAve.SetTextSize(0.030)

            tResL = ROOT.TLatex( 0.15, 0.79,  't_{left}  :  #sigma_{eff} = %.1f ps,  #sigma_{gaus} = %.1f #pm %.1f ps'%(resolEffL, resolGausL, resolGausErrL) )
            tResL.SetNDC()
            tResL.SetTextColor(ROOT.kBlue)
            tResL.SetTextSize(0.030)

            tResR = ROOT.TLatex( 0.15, 0.74,  't_{right}:  #sigma_{eff} = %.1f ps,  #sigma_{gaus} = %.1f #pm %.1f ps'%(resolEffR, resolGausR, resolGausErrR) )
            tResR.SetNDC()
            tResR.SetTextColor(ROOT.kRed)
            tResR.SetTextSize(0.030)

            
        if ( hnameCh.startswith('p_tAve') ):
            offset = h[hnameCh].GetMean(2)           
            n = h[hnameChL].GetNbinsX()
            hnew[hnameChL] = ROOT.TH1F(h[hnameChL].GetName()+'_shifted', h[hnameChL].GetName()+'_shifted', n, h[hnameChL].GetBinCenter(1), h[hnameChL].GetBinCenter(n))
            hnew[hnameChR] = ROOT.TH1F(h[hnameChR].GetName()+'_shifted', h[hnameChR].GetName()+'_shifted', n, h[hnameChR].GetBinCenter(1), h[hnameChR].GetBinCenter(n))
            hnew[hnameCh]  = ROOT.TH1F(h[hnameCh ].GetName()+'_shifted', h[hnameCh ].GetName()+'_shifted', n, h[hnameCh ].GetBinCenter(1), h[hnameCh ].GetBinCenter(n))

            hnew[hnameCh].SetLineColor(ROOT.kBlack)
            hnew[hnameChL].SetLineColor(ROOT.kBlue)
            hnew[hnameChR].SetLineColor(ROOT.kRed)
            
            hnew[hnameCh].SetMarkerColor(ROOT.kBlack)
            hnew[hnameChL].SetMarkerColor(ROOT.kBlue)
            hnew[hnameChR].SetMarkerColor(ROOT.kRed)
            
            hnew[hnameCh].SetMarkerStyle(20)
            hnew[hnameChL].SetMarkerStyle(20)
            hnew[hnameChR].SetMarkerStyle(20)

            hnew[hnameCh].SetMarkerSize(0.7)
            hnew[hnameChL].SetMarkerSize(0.7)
            hnew[hnameChR].SetMarkerSize(0.7)

            for thishname in [hnameChL, hnameChR, hnameCh]:
                hist = h[thishname]
                for ibin in range(1,hist.GetNbinsX()+1):
                    err = hist.GetBinError(ibin)
                    cont =  hist.GetBinContent(ibin)
                    if (cont!=0):
                        hnew[thishname].SetBinContent(ibin, cont-offset)
                        hnew[thishname].SetBinError(ibin, err)
                        print err, hist.GetBinError(ibin)
    
            if ('vs_posX' in hnameCh):
                h[hnameCh].GetXaxis().SetRangeUser(  h[hnameCh].GetMean()-2.5*h[hnameCh].GetRMS(), h[hnameCh].GetMean()+2.5*h[hnameCh].GetRMS())
                #h[hnameCh].GetYaxis().SetRangeUser(  h[hnameChL].GetMean(2)-0.35, h[hnameChR].GetMean(2)+0.45)
                h[hnameCh].GetYaxis().SetRangeUser(  -0.500, 0.500)
                h[hnameCh].GetXaxis().SetTitle('x (mm)')
                h[hnameCh].GetYaxis().SetTitle('t_{i} - t_{MCP} (ns)')
                hnew[hnameCh].GetXaxis().SetRangeUser(  h[hnameCh].GetMean()-2.5*h[hnameCh].GetRMS(), h[hnameCh].GetMean()+2.5*h[hnameCh].GetRMS())
                #h[hnameCh].GetYaxis().SetRangeUser(  h[hnameChL].GetMean(2)-0.35, h[hnameChR].GetMean(2)+0.45)
                hnew[hnameCh].GetYaxis().SetRangeUser(  -0.500, 0.500)
                hnew[hnameCh].GetXaxis().SetTitle('x (mm)')
                hnew[hnameCh].GetYaxis().SetTitle('t_{i} - t_{MCP} (ns)')
            if ('vs_tDiff' in hnameCh):
                h[hnameCh].GetXaxis().SetRangeUser(  h[hnameCh].GetMean() - 0.7, h[hnameCh].GetMean()+ 0.7)
                h[hnameCh].GetYaxis().SetRangeUser(  h[hnameChL].GetMean(2)-0.7, h[hnameChR].GetMean(2)+ 0.7)
                h[hnameCh].GetXaxis().SetTitle('t_{right} - t_{left} (ns)')
                h[hnameCh].GetYaxis().SetTitle('t_{ave} - t_{MCP} (ns)')

   
                

        # draw
        if ( hnameCh.startswith('h') or hnameCh.startswith('p')) :   
            h[hnameCh].Draw('e')
            h[hnameChL].Draw('esame')
            h[hnameChR].Draw('esame')
            if ( hnameCh.startswith('p_tAve') ):
                 hnew[hnameCh].Draw('e')
                 hnew[hnameChL].Draw('esame')
                 hnew[hnameChR].Draw('esame')
        else:
            #c[hnameCh].SetGridy()
            xmin = h[hnameCh].GetMean()- 2.5* h[hnameCh].GetRMS()
            xmax = h[hnameCh].GetMean()+ 2.5* h[hnameCh].GetRMS()
            hdummy = ROOT.TH2F('','',100,xmin,xmax, 100, 0, 90)            
            hdummy.GetXaxis().SetTitle('x (mm)')
            hdummy.GetYaxis().SetTitle('time resolution (ps)')
            hdummy.Draw()
            h[hnameCh].Draw('psame')
            h[hnameChL].Draw('psame')
            h[hnameChR].Draw('psame')

            resolGaus = h[hnameCh].GetFunction(hnameCh.replace('g_tResolGaus','fpol0_t')).GetParameter(0)
            resolGausErr = h[hnameCh].GetFunction(hnameCh.replace('g_tResolGaus','fpol0_t')).GetParError(0)
            tResAveX = ROOT.TLatex( 0.15, 0.74,'t_{ave}  :  #sigma_{gaus} = %.1f #pm %.1f ps'%(resolGaus, resolGausErr) )
            tResAveX.SetNDC()
            tResAveX.SetTextSize(0.030)
            tResAveX.Draw()

            resolGaus = h[hnameChL].GetFunction(hnameChL.replace('g_tResolGaus','fpol0_t')).GetParameter(0)
            resolGausErr = h[hnameChL].GetFunction(hnameChL.replace('g_tResolGaus','fpol0_t')).GetParError(0)
            tResLX = ROOT.TLatex( 0.15, 0.84,'t_{left}  :  #sigma_{gaus} = %.1f #pm %.1f ps'%(resolGaus, resolGausErr) )
            tResLX.SetTextColor(ROOT.kBlue)
            tResLX.SetNDC()
            tResLX.SetTextSize(0.030)
            tResLX.Draw()

            resolGaus = h[hnameChR].GetFunction(hnameChR.replace('g_tResolGaus','fpol0_t')).GetParameter(0)
            resolGausErr = h[hnameChR].GetFunction(hnameChR.replace('g_tResolGaus','fpol0_t')).GetParError(0)
            tResRX = ROOT.TLatex( 0.15, 0.79,'t_{right}  :  #sigma_{gaus} = %.1f #pm %.1f ps'%(resolGaus, resolGausErr) )
            tResRX.SetTextColor(ROOT.kRed)
            tResRX.SetNDC()
            tResRX.SetTextSize(0.030)
            tResRX.Draw()
            

        tChType[hnameCh] = ROOT.TLatex( 0.13, 0.92, channelType[channel] )
        tChType[hnameCh].SetNDC()
        tChType[hnameCh].SetTextSize(0.030)
        tChType[hnameCh].Draw()
        
        if (hnameCh.startswith('h_tAve')):
            tResAve.Draw()
            tResL.Draw()
            tResR.Draw()
            tChType[hnameCh].Draw()
            

        if (ich==0 and ih==0):
            legend.AddEntry(h[hnameCh],'t_{average}','PL')
            legend.AddEntry(h[hnameChL],'t_{left}','PL')
            legend.AddEntry(h[hnameChR],'t_{right}','PL')


        legend.Draw('same')

        c[hnameCh].SaveAs(outdir+'/'+c[hnameCh].GetName()+'.png')
        c[hnameCh].SaveAs(outdir+'/'+c[hnameCh].GetName()+'.pdf')
        c[hnameCh].SaveAs(outdir+'/'+c[hnameCh].GetName()+'.C')




#draw tDiff
fitfunDiff = None

for ich,channel in enumerate(channels):

    for ih, hname in enumerate([#'h_tDiff_ampCorr', 'p_tDiff_ampCorr_vs_posX', 'p_tDiff_ampCorr_vs_posXc', 'g_tResolGausDiff_ampCorr',
                                #'g_tResolGausDiff', 'g_tResolGausDiff_posCorr',
                                'h_tDiff_ampCorr_posCorr', 'p_tDiff_ampCorr_posCorr_vs_posX', 'p_tDiff_ampCorr_posCorr_vs_posXc', 'g_tResolGausDiff_ampCorr_posCorr',
                                'h_tDiff_pulseIntCorr_posCorr', 'p_tDiff_pulseIntCorr_posCorr_vs_posX', 'p_tDiff_pulseIntCorr_posCorr_vs_posXc', 'g_tResolGausDiff_pulseIntCorr_posCorr'
    ]):


        hnameCh= hname + '_' + channel
        
        cname = hnameCh.replace('h_','').replace('p_t','t')
        
        c[hnameCh] = ROOT.TCanvas(cname,cname)
        c[hnameCh].SetTickx()
        c[hnameCh].SetTicky()
        #if (hnameCh.startswith('p_')):
            #c[hnameCh].SetGridy()

        # get histograms
        h[hnameCh]  = f.Get(hnameCh)

        h[hnameCh].SetLineColor(ROOT.kBlack)
        h[hnameCh].SetMarkerColor(ROOT.kBlack)
        h[hnameCh].SetMarkerStyle(20)
        h[hnameCh].SetMarkerSize(0.7)
        
        if ( hnameCh.startswith('h_') ):
            h[hnameCh].GetYaxis().SetRangeUser(0,h[hnameCh].GetMaximum()*1.5  )
            h[hnameCh].GetXaxis().SetRangeUser(  h[hnameCh].GetMean() - 1, h[hnameCh].GetMean()+ 1)
            h[hnameCh].GetXaxis().SetTitle('t_{right} - t_{left} (ns)')

            fitfunDiff  = h[hnameCh].GetFunction(hnameCh.replace('h_tDiff','fitFunDiff'))
            fitfunDiff.SetLineColor(ROOT.kBlack)
            sigmaGaus  = fitfunDiff.GetParameter(2)
            sigmaEff   = f.Get(hnameCh.replace('h_t','h_effectiveSigma')).GetMean()
            resolGaus  = sigmaGaus*1000. # ps
            resolGausErr = fitfunDiff.GetParError(2)*1000.*sigmaGaus*1000./resolGaus
            resolEff   = sigmaEff*1000.
            print '%s, resol_gaus = %.1f ps, eff_sigma = %.1f ps'%(channel, resolGaus, resolEff)
            
            tResDiff = ROOT.TLatex( 0.15, 0.84,'t_{diff}/2  :  #sigma_{eff} = %.1f ps,  #sigma_{gaus} = %.1f #pm %.1f ps'%(resolEff/2, resolGaus/2, resolGausErr/2) )
            tResDiff.SetNDC()
            tResDiff.SetTextSize(0.030)

            
        if ( hnameCh.startswith('p_tDiff') ):
            if ('vs_posX' in hnameCh):
                h[hnameCh].GetXaxis().SetRangeUser(  h[hnameCh].GetMean() - 25, h[hnameCh].GetMean()+ 25)
                h[hnameCh].GetYaxis().SetRangeUser(  h[hnameCh].GetMean(2)-0.5, h[hnameCh].GetMean(2)+0.5)
                h[hnameCh].GetXaxis().SetTitle('x (mm)')
                h[hnameCh].GetYaxis().SetTitle('t_{right} - t_{left} (ns)')

        tChType[hnameCh] = ROOT.TLatex( 0.13, 0.92, channelType[channel] )
        tChType[hnameCh].SetNDC()
        tChType[hnameCh].SetTextSize(0.030)
        tChType[hnameCh].Draw()

        #draw
        if ( hnameCh.startswith('h') ) :   
            h[hnameCh].Draw('e')
            tResDiff.Draw()
            tChType[hnameCh].Draw()
        elif (hnameCh.startswith('p') ):
            h[hnameCh].Draw('e')
            tChType[hnameCh].Draw()
        else:
            f0 = ROOT.TF1('f0','pol0',-50,50)
            h[hnameCh].Fit(f0,'QR')
            h[hnameCh].GetYaxis().SetRangeUser(0,150)
            h[hnameCh].GetXaxis().SetTitle('x (mm)')
            h[hnameCh].GetYaxis().SetTitle('#sigma(t_{diff}) (ns)')
            tResDiffX = ROOT.TLatex( 0.15, 0.84,'t_{diff}  : #sigma_{gaus} = %.1f #pm %.1f ps'%(f0.GetParameter(0),f0.GetParError(0) ) )
            tResDiffX.SetNDC()
            tResDiffX.SetTextSize(0.030)
            h[hnameCh].Draw('ap')
            tResDiffX.Draw()
            
        c[hnameCh].SaveAs(outdir+'/'+c[hnameCh].GetName()+'.png')
        c[hnameCh].SaveAs(outdir+'/'+c[hnameCh].GetName()+'.pdf')
        c[hnameCh].SaveAs(outdir+'/'+c[hnameCh].GetName()+'.C')
