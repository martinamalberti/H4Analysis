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

filename1 = '../v3/output_digiConf2_sensorConf3_Vbias72_NINOthr400.root'
filename2 = '../v3/output_digiConf2_sensorConf4_Vbias72_NINOthr400.root'



pR = {}
pL = {}

f1 = ROOT.TFile.Open(filename1)
f2 = ROOT.TFile.Open(filename2)


pL[0]  = f1.Get('p_tL_ampCorr_vs_posXc_BAR0')
pL[45] = f2.Get('p_tL_ampCorr_vs_posXc_BAR1')
pL[60] = f2.Get('p_tL_ampCorr_vs_posXc_BAR0' )
pL[80] = f1.Get('p_tL_ampCorr_vs_posXc_BAR1')

pR[0]  = f1.Get('p_tR_ampCorr_vs_posXc_BAR0')
pR[45] = f2.Get('p_tR_ampCorr_vs_posXc_BAR1')
pR[60] = f2.Get('p_tR_ampCorr_vs_posXc_BAR0' )
pR[80] = f1.Get('p_tR_ampCorr_vs_posXc_BAR1')


slopeL = {}
slopeR = {}

eslopeL = {}
eslopeR = {}

c = {}
gR_delay_vs_angle = ROOT.TGraphErrors()
gL_delay_vs_angle = ROOT.TGraphErrors()


legg = ROOT.TLegend(0.7,0.7,0.89,0.89)

for i,angle in enumerate([0, 45, 60, 80]):

    fitfunL = ROOT.TF1('fitfunL','pol1',-100,100)
    fitfunL.SetLineColor(ROOT.kBlue)
    #fitfunL.SetRange(pL[angle].GetMean()-1.5*pL[angle].GetRMS(),pL[angle].GetMean()+1.5*pL[angle].GetRMS())
    pL[angle].Fit(fitfunL,'QR')
    slopeL[angle] = fitfunL.GetParameter(1)*1000.
    eslopeL[angle] = fitfunL.GetParError(1)*1000.
    
    fitfunR = ROOT.TF1('fitfunR','pol1',-100,100)
    #fitfunR.SetRange(pR[angle].GetMean()-1.5*pR[angle].GetRMS(),pR[angle].GetMean()+1.5*pR[angle].GetRMS())
    pR[angle].Fit(fitfunR,'QR')
    slopeR[angle] = fitfunR.GetParameter(1)*1000.
    eslopeR[angle] = fitfunR.GetParError(1)*1000.

    c[angle] = ROOT.TCanvas('c_delay_%s'%angle,'c_%s'%angle,600,500)
    c[angle].SetGridy()
    pL[angle].SetMarkerColor(ROOT.kBlue)
    pR[angle].SetMarkerColor(ROOT.kRed)
    meany =  0.5*(pL[angle].GetMean(2) +pR[angle].GetMean(2) )
    pL[angle].GetYaxis().SetRangeUser(meany-1.5, meany+1.5)
    pL[angle].GetXaxis().SetRangeUser(pL[angle].GetMean()-2*pL[angle].GetRMS() ,pL[angle].GetMean()+2*pL[angle].GetRMS() )
   
    pL[angle].GetXaxis().SetTitle('x (mm)')
    pL[angle].GetYaxis().SetTitle('time (ps)')
    pL[angle].Draw()
    pR[angle].Draw('same')
    if (i==0):
        legg.AddEntry(pL[angle],'upstream SiPM','PL')
        legg.AddEntry(pR[angle],'downstream SiPM','PL')
    legg.Draw('same')
    c[angle].SaveAs(c[angle].GetName()+'.pdf')
    c[angle].SaveAs(c[angle].GetName()+'.png')
        
    gL_delay_vs_angle.SetPoint( i, angle, -slopeL[angle] )
    gL_delay_vs_angle.SetPointError( i, 0, eslopeL[angle] )

    gR_delay_vs_angle.SetPoint( i, angle, slopeR[angle] )
    gR_delay_vs_angle.SetPointError( i, 0, eslopeR[angle] )

    raw_input('ok?')

print slopeR
print slopeL

gL_delay_vs_angle.SetMarkerStyle(20)
gL_delay_vs_angle.SetLineColor(4)
gL_delay_vs_angle.SetMarkerColor(4)

gR_delay_vs_angle.SetMarkerStyle(20)
gR_delay_vs_angle.SetLineColor(2)
gR_delay_vs_angle.SetMarkerColor(2)

canvas = ROOT.TCanvas('timeDelay_vs_angle','',600,500)
canvas.SetGridy()

hdummy = ROOT.TH2F('hdummy','',91,-1,90,10,0,15)
hdummy.Draw()
hdummy.GetXaxis().SetTitle('#theta (#circ)')
hdummy.GetYaxis().SetTitle('time delay (ps/mm)')
gL_delay_vs_angle.Draw('plsame')
gR_delay_vs_angle.Draw('plsame')

leg = ROOT.TLegend(0.7,0.7,0.89,0.89)
leg.AddEntry(gL_delay_vs_angle,'t_{upstream}','PL')
leg.AddEntry(gR_delay_vs_angle,'t_{downstream}','PL')
leg.Draw('same')

canvas.SaveAs(canvas.GetName()+'.png')
canvas.SaveAs(canvas.GetName()+'.pdf')

raw_input('ok?')
