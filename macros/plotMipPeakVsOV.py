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
ROOT.gStyle.SetOptFit(1)
ROOT.gStyle.SetOptTitle(0)
ROOT.gStyle.SetLabelSize(0.04,'X')
ROOT.gStyle.SetLabelSize(0.04,'Y')
ROOT.gStyle.SetTitleSize(0.04,'X')
ROOT.gStyle.SetTitleSize(0.04,'Y')
ROOT.gStyle.SetTitleOffset(1.1,'X')
ROOT.gStyle.SetTitleOffset(1.2,'Y')


  


###
inputDir = '../v11/biasScan/'


sipm   = sys.argv[1] # sipm type HPK or FBK
outdir = sys.argv[2] # outdir

file = {}

vbr = 66
Vbias = [72, 71, 70, 69, 68]
channels = ['BAR%s'%i for i in range(0,3)]
channelType = {'BAR0':'LYSO:Ce 3x3x57 mm^{3} - HPK S12572-015',
               'BAR1':'LYSO:Ce 3x3x57 mm^{3} - HPK S12572-015',
               'BAR2':'LYSO:Ce 3x3x57 mm^{3} - HPK S12572-015'}
for vb in Vbias:
  file[vb] = ROOT.TFile.Open('%s/output_3bars_Vbias%s_thr100ADC_xyangle90.root'%(inputDir,vb),"READ")
  
if (sipm == 'FBK'):
  vbr = 37.
  Vbias = [43, 42, 41, 40, 39]
  channels = ['BAR%s'%i for i in range(0,1)]
  channelType = {'BAR0':'LYSO:Ce 3x4x57 mm^{3} - FBK thin-epi 5x5 mm^{2}'}
  for vb in Vbias:
    file[vb] = ROOT.TFile.Open('%s/output_1bar_Vbias%s_thr100ADC_4mm.root'%(inputDir,vb),"READ")



  
g = {} 
for ch in channels:
  g[ch] = ROOT.TGraphErrors()
  for vb in Vbias:
    h_ampL = file[vb].Get('h_ampL_%s'%ch)
    h_ampR = file[vb].Get('h_ampR_%s'%ch)
    fLandauL = ROOT.TF1('fLandauL','landau',0,1)
    fLandauR = ROOT.TF1('fLandauR','landau',0,1)
    h_ampL.Fit('fLandauL','QR')
    h_ampR.Fit('fLandauR','QR')
    mpvL = fLandauL.GetParameter(1)
    mpvLerr = fLandauL.GetParError(1)
    mpvR = fLandauR.GetParameter(1)
    mpvRerr = fLandauR.GetParError(1)
    mpv = 0.5 * (mpvR+mpvL)
    mpverr = 0.5 * math.sqrt(mpvRerr*mpvRerr+mpvLerr*mpvLerr)
    g[ch].SetPoint(g[ch].GetN(), vb - vbr, mpv*1000.) # mV
    g[ch].SetPointError(g[ch].GetN()-1, 0, mpverr*1000.) # mV


#### fitting function
# amp vs OV from pde*gain*enc
filePDE = ROOT.TFile.Open('pde/PDE_vs_OV.root')
f_pde_vs_ov = filePDE.Get('fPDE_vs_OV_%s'%sipm)
f_gain_vs_ov = filePDE.Get('fGain_vs_OV_%s'%sipm)
f_enc_vs_ov = filePDE.Get('fENC_vs_OV_%s'%sipm)

g_exp = ROOT.TGraphErrors()
for i in range(0,100000):
  ov  = i * (10-0)/100000
  amp = f_pde_vs_ov.Eval(ov) * f_gain_vs_ov.Eval(ov) * f_enc_vs_ov.Eval(ov)
  g_exp.SetPoint(i, ov, amp)

def fitFunction(x, par):
  y = g_exp.Eval(x[0]-par[0])*par[1]
  return y

fitFunc = {}

for ich,ch in enumerate(channels):
  fitFunc[ch] = ROOT.TF1('fitFunc_%s'%ch,fitFunction, -10, 10, 2)
  fitFunc[ch].SetLineColor(ROOT.kBlue+ich)
  fitFunc[ch].SetParameter(0, 0.1)
  scale  =  g[ch].Eval(5)/g_exp.Eval(5)
  fitFunc[ch].SetParameter(1, scale)
  fitFunc[ch].SetRange(-1,10)
  g[ch].Fit(fitFunc[ch],'QR')
  print ch, '#Delta(Vbr) = ', fitFunc[ch].GetParameter(0),' +/- ', fitFunc[ch].GetParError(0)

    
# plot
tl = ROOT.TLatex( 0.15, 0.92, '%s'%channelType['BAR0'])
tl.SetNDC()
tl.SetTextSize(0.030)
  
canvas1 = ROOT.TCanvas('c_amplitude_vs_vov_%s'%sipm,'c_amplitude_vs_vov_%s'%sipm)
canvas1.SetGridy()
canvas1.SetGridx()
ymax = 150
if (sipm == 'FBK'): ymax = 80
hdummy1 = ROOT.TH2F('hdummy1','', 10, -1, 7, 100, 0, ymax)
hdummy1.GetXaxis().SetTitle('OV (V)')
hdummy1.GetYaxis().SetTitle('amplitude (mV)')
hdummy1.Draw()
g_exp.SetLineStyle(2)
#g_exp.Draw('lsame')
for ich, ch in enumerate(channels):
  g[ch].SetMarkerColor(ROOT.kBlue+ich)
  g[ch].Draw('psame')
tl.Draw()


# save plots
print 'Saving plots in %s'%outdir
os.system('mkdir %s'%outdir)
shutil.copy('index.php', '%s'%outdir)

for c in [canvas1]:
  c.SaveAs(outdir+'/'+c.GetName()+'.png')
  c.SaveAs(outdir+'/'+c.GetName()+'.pdf')
  c.SaveAs(outdir+'/'+c.GetName()+'.C')
  
raw_input('ok?')
