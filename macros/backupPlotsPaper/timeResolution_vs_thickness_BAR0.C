void timeResolution_vs_thickness_BAR0()
{
//=========Macro generated from canvas: timeResolution_vs_thickness_BAR0/timeResolution_vs_thickness_BAR0
//=========  (Wed Feb 17 16:39:32 2021) by ROOT version 6.14/04
   TCanvas *timeResolution_vs_thickness_BAR0 = new TCanvas("timeResolution_vs_thickness_BAR0", "timeResolution_vs_thickness_BAR0",0,0,600,600);
   gStyle->SetOptStat(0);
   gStyle->SetOptTitle(0);
   timeResolution_vs_thickness_BAR0->SetHighLightColor(2);
   timeResolution_vs_thickness_BAR0->Range(0.3417722,0.7692306,5.405063,77.69231);
   timeResolution_vs_thickness_BAR0->SetFillColor(0);
   timeResolution_vs_thickness_BAR0->SetBorderMode(0);
   timeResolution_vs_thickness_BAR0->SetBorderSize(2);
   timeResolution_vs_thickness_BAR0->SetTickx(1);
   timeResolution_vs_thickness_BAR0->SetTicky(1);
   timeResolution_vs_thickness_BAR0->SetLeftMargin(0.13);
   timeResolution_vs_thickness_BAR0->SetRightMargin(0.08);
   timeResolution_vs_thickness_BAR0->SetBottomMargin(0.12);
   timeResolution_vs_thickness_BAR0->SetFrameFillStyle(0);
   timeResolution_vs_thickness_BAR0->SetFrameBorderMode(0);
   timeResolution_vs_thickness_BAR0->SetFrameFillStyle(0);
   timeResolution_vs_thickness_BAR0->SetFrameBorderMode(0);
   
   TH2F *hdummy2_BAR0__2 = new TH2F("hdummy2_BAR0__2","",26,1,5,80,10,70);
   hdummy2_BAR0__2->SetStats(0);
   hdummy2_BAR0__2->SetLineStyle(0);
   hdummy2_BAR0__2->SetMarkerStyle(20);
   hdummy2_BAR0__2->GetXaxis()->SetTitle("thickness (mm)");
   hdummy2_BAR0__2->GetXaxis()->SetLabelFont(42);
   hdummy2_BAR0__2->GetXaxis()->SetLabelOffset(0.007);
   hdummy2_BAR0__2->GetXaxis()->SetTitleOffset(1.1);
   hdummy2_BAR0__2->GetXaxis()->SetTitleFont(42);
   hdummy2_BAR0__2->GetYaxis()->SetTitle("time resolution (ps)");
   hdummy2_BAR0__2->GetYaxis()->SetLabelFont(42);
   hdummy2_BAR0__2->GetYaxis()->SetLabelOffset(0.007);
   hdummy2_BAR0__2->GetYaxis()->SetTitleOffset(1.2);
   hdummy2_BAR0__2->GetYaxis()->SetTitleFont(42);
   hdummy2_BAR0__2->GetZaxis()->SetLabelFont(42);
   hdummy2_BAR0__2->GetZaxis()->SetLabelOffset(0.007);
   hdummy2_BAR0__2->GetZaxis()->SetLabelSize(0.05);
   hdummy2_BAR0__2->GetZaxis()->SetTitleSize(0.06);
   hdummy2_BAR0__2->GetZaxis()->SetTitleFont(42);
   hdummy2_BAR0__2->Draw("");
   
   Double_t _fx1003[3] = {
   2,
   3,
   4};
   Double_t _fy1003[3] = {
   35.51654,
   29.03593,
   25.03079};
   Double_t _fex1003[3] = {
   0,
   0,
   0};
   Double_t _fey1003[3] = {
   0.6545405,
   0.2782804,
   0.2641352};
   TGraphErrors *gre = new TGraphErrors(3,_fx1003,_fy1003,_fex1003,_fey1003);
   gre->SetName("");
   gre->SetTitle("");
   gre->SetFillStyle(1000);
   gre->SetMarkerStyle(24);
   
   TH1F *Graph_Graph1003 = new TH1F("Graph_Graph1003","",100,1.8,4.2);
   Graph_Graph1003->SetMinimum(23.62622);
   Graph_Graph1003->SetMaximum(37.31153);
   Graph_Graph1003->SetDirectory(0);
   Graph_Graph1003->SetStats(0);
   Graph_Graph1003->SetLineStyle(0);
   Graph_Graph1003->SetMarkerStyle(20);
   Graph_Graph1003->GetXaxis()->SetLabelFont(42);
   Graph_Graph1003->GetXaxis()->SetLabelOffset(0.007);
   Graph_Graph1003->GetXaxis()->SetTitleOffset(1.1);
   Graph_Graph1003->GetXaxis()->SetTitleFont(42);
   Graph_Graph1003->GetYaxis()->SetLabelFont(42);
   Graph_Graph1003->GetYaxis()->SetLabelOffset(0.007);
   Graph_Graph1003->GetYaxis()->SetTitleOffset(1.2);
   Graph_Graph1003->GetYaxis()->SetTitleFont(42);
   Graph_Graph1003->GetZaxis()->SetLabelFont(42);
   Graph_Graph1003->GetZaxis()->SetLabelOffset(0.007);
   Graph_Graph1003->GetZaxis()->SetLabelSize(0.05);
   Graph_Graph1003->GetZaxis()->SetTitleSize(0.06);
   Graph_Graph1003->GetZaxis()->SetTitleFont(42);
   gre->SetHistogram(Graph_Graph1003);
   
   
   TF1 *fitDiff1004 = new TF1("fitDiff","[1]*1./pow(x,[0])",1.8,4.2, TF1::EAddToList::kNo);
   fitDiff1004->SetFillColor(19);
   fitDiff1004->SetFillStyle(0);
   fitDiff1004->SetMarkerStyle(20);
   fitDiff1004->SetLineWidth(1);
   fitDiff1004->SetLineStyle(2);
   fitDiff1004->SetChisquare(0.05465151);
   fitDiff1004->SetNDF(1);
   fitDiff1004->GetXaxis()->SetLabelFont(42);
   fitDiff1004->GetXaxis()->SetLabelOffset(0.007);
   fitDiff1004->GetXaxis()->SetTitleOffset(1.1);
   fitDiff1004->GetXaxis()->SetTitleFont(42);
   fitDiff1004->GetYaxis()->SetLabelFont(42);
   fitDiff1004->GetYaxis()->SetLabelOffset(0.007);
   fitDiff1004->GetYaxis()->SetTitleOffset(1.2);
   fitDiff1004->GetYaxis()->SetTitleFont(42);
   fitDiff1004->SetParameter(0,0.5066407);
   fitDiff1004->SetParError(0,0.02950522);
   fitDiff1004->SetParLimits(0,0,0);
   fitDiff1004->SetParameter(1,50.58057);
   fitDiff1004->SetParError(1,1.762579);
   fitDiff1004->SetParLimits(1,0,0);
   fitDiff1004->SetParent(gre);
   gre->GetListOfFunctions()->Add(fitDiff1004);
   gre->Draw("p");
   
   Double_t _fx1005[3] = {
   2,
   3,
   4};
   Double_t _fy1005[3] = {
   35.01944,
   28.68628,
   25.14024};
   Double_t _fex1005[3] = {
   0,
   0,
   0};
   Double_t _fey1005[3] = {
   0.5894541,
   0.3053442,
   0.3010119};
   gre = new TGraphErrors(3,_fx1005,_fy1005,_fex1005,_fey1005);
   gre->SetName("");
   gre->SetTitle("");
   gre->SetFillStyle(1000);
   gre->SetMarkerStyle(20);
   gre->SetMarkerSize(0.9);
   
   TH1F *Graph_Graph1005 = new TH1F("Graph_Graph1005","",100,1.8,4.2);
   Graph_Graph1005->SetMinimum(23.76226);
   Graph_Graph1005->SetMaximum(36.68586);
   Graph_Graph1005->SetDirectory(0);
   Graph_Graph1005->SetStats(0);
   Graph_Graph1005->SetLineStyle(0);
   Graph_Graph1005->SetMarkerStyle(20);
   Graph_Graph1005->GetXaxis()->SetLabelFont(42);
   Graph_Graph1005->GetXaxis()->SetLabelOffset(0.007);
   Graph_Graph1005->GetXaxis()->SetTitleOffset(1.1);
   Graph_Graph1005->GetXaxis()->SetTitleFont(42);
   Graph_Graph1005->GetYaxis()->SetLabelFont(42);
   Graph_Graph1005->GetYaxis()->SetLabelOffset(0.007);
   Graph_Graph1005->GetYaxis()->SetTitleOffset(1.2);
   Graph_Graph1005->GetYaxis()->SetTitleFont(42);
   Graph_Graph1005->GetZaxis()->SetLabelFont(42);
   Graph_Graph1005->GetZaxis()->SetLabelOffset(0.007);
   Graph_Graph1005->GetZaxis()->SetLabelSize(0.05);
   Graph_Graph1005->GetZaxis()->SetTitleSize(0.06);
   Graph_Graph1005->GetZaxis()->SetTitleFont(42);
   gre->SetHistogram(Graph_Graph1005);
   
   
   TF1 *fitAve1006 = new TF1("fitAve","[1]*1./pow(x,[0])",1.8,4.2, TF1::EAddToList::kNo);
   fitAve1006->SetFillColor(19);
   fitAve1006->SetFillStyle(0);
   fitAve1006->SetMarkerStyle(20);
   fitAve1006->SetLineWidth(1);
   fitAve1006->SetChisquare(0.1490007);
   fitAve1006->SetNDF(1);
   fitAve1006->GetXaxis()->SetLabelFont(42);
   fitAve1006->GetXaxis()->SetLabelOffset(0.007);
   fitAve1006->GetXaxis()->SetTitleOffset(1.1);
   fitAve1006->GetXaxis()->SetTitleFont(42);
   fitAve1006->GetYaxis()->SetLabelFont(42);
   fitAve1006->GetYaxis()->SetLabelOffset(0.007);
   fitAve1006->GetYaxis()->SetTitleOffset(1.2);
   fitAve1006->GetYaxis()->SetTitleFont(42);
   fitAve1006->SetParameter(0,0.4768549);
   fitAve1006->SetParError(0,0.0297323);
   fitAve1006->SetParLimits(0,0,0);
   fitAve1006->SetParameter(1,48.58403);
   fitAve1006->SetParError(1,1.667006);
   fitAve1006->SetParLimits(1,0,0);
   fitAve1006->SetParent(gre);
   gre->GetListOfFunctions()->Add(fitAve1006);
   gre->Draw("p");
   
   Double_t _fx1007[3] = {
   2,
   3,
   4};
   Double_t _fy1007[3] = {
   49.06484,
   41.94898,
   35.55073};
   Double_t _fex1007[3] = {
   0,
   0,
   0};
   Double_t _fey1007[3] = {
   0,
   0,
   0};
   gre = new TGraphErrors(3,_fx1007,_fy1007,_fex1007,_fey1007);
   gre->SetName("");
   gre->SetTitle("");
   gre->SetFillStyle(1000);

   Int_t ci;      // for color index setting
   TColor *color; // for color definition with alpha
   ci = TColor::GetColor("#0000ff");
   gre->SetLineColor(ci);

   ci = TColor::GetColor("#0000ff");
   gre->SetMarkerColor(ci);
   gre->SetMarkerStyle(20);
   gre->SetMarkerSize(0.9);
   
   TH1F *Graph_Graph1007 = new TH1F("Graph_Graph1007","",100,1.8,4.2);
   Graph_Graph1007->SetMinimum(34.19932);
   Graph_Graph1007->SetMaximum(50.41626);
   Graph_Graph1007->SetDirectory(0);
   Graph_Graph1007->SetStats(0);
   Graph_Graph1007->SetLineStyle(0);
   Graph_Graph1007->SetMarkerStyle(20);
   Graph_Graph1007->GetXaxis()->SetLabelFont(42);
   Graph_Graph1007->GetXaxis()->SetLabelOffset(0.007);
   Graph_Graph1007->GetXaxis()->SetTitleOffset(1.1);
   Graph_Graph1007->GetXaxis()->SetTitleFont(42);
   Graph_Graph1007->GetYaxis()->SetLabelFont(42);
   Graph_Graph1007->GetYaxis()->SetLabelOffset(0.007);
   Graph_Graph1007->GetYaxis()->SetTitleOffset(1.2);
   Graph_Graph1007->GetYaxis()->SetTitleFont(42);
   Graph_Graph1007->GetZaxis()->SetLabelFont(42);
   Graph_Graph1007->GetZaxis()->SetLabelOffset(0.007);
   Graph_Graph1007->GetZaxis()->SetLabelSize(0.05);
   Graph_Graph1007->GetZaxis()->SetTitleSize(0.06);
   Graph_Graph1007->GetZaxis()->SetTitleFont(42);
   gre->SetHistogram(Graph_Graph1007);
   
   
   TF1 *fitL1008 = new TF1("fitL","[1]*1./pow(x,[0])",1.8,4.2, TF1::EAddToList::kNo);
   fitL1008->SetFillColor(19);
   fitL1008->SetFillStyle(0);
   fitL1008->SetMarkerStyle(20);

   ci = TColor::GetColor("#0000ff");
   fitL1008->SetLineColor(ci);
   fitL1008->SetLineWidth(1);
   fitL1008->SetChisquare(1.100524);
   fitL1008->SetNDF(1);
   fitL1008->GetXaxis()->SetLabelFont(42);
   fitL1008->GetXaxis()->SetLabelOffset(0.007);
   fitL1008->GetXaxis()->SetTitleOffset(1.1);
   fitL1008->GetXaxis()->SetTitleFont(42);
   fitL1008->GetYaxis()->SetLabelFont(42);
   fitL1008->GetYaxis()->SetLabelOffset(0.007);
   fitL1008->GetYaxis()->SetTitleOffset(1.2);
   fitL1008->GetYaxis()->SetTitleFont(42);
   fitL1008->SetParameter(0,0.4508212);
   fitL1008->SetParError(0,0.04982251);
   fitL1008->SetParLimits(0,0,0);
   fitL1008->SetParameter(1,67.45938);
   fitL1008->SetParError(1,3.449622);
   fitL1008->SetParLimits(1,0,0);
   fitL1008->SetParent(gre);
   gre->GetListOfFunctions()->Add(fitL1008);
   gre->Draw("p");
   
   Double_t _fx1009[3] = {
   2,
   3,
   4};
   Double_t _fy1009[3] = {
   52.48129,
   38.95591,
   35.23974};
   Double_t _fex1009[3] = {
   0,
   0,
   0};
   Double_t _fey1009[3] = {
   0,
   0,
   0};
   gre = new TGraphErrors(3,_fx1009,_fy1009,_fex1009,_fey1009);
   gre->SetName("");
   gre->SetTitle("");
   gre->SetFillStyle(1000);

   ci = TColor::GetColor("#ff0000");
   gre->SetLineColor(ci);

   ci = TColor::GetColor("#ff0000");
   gre->SetMarkerColor(ci);
   gre->SetMarkerStyle(20);
   gre->SetMarkerSize(0.9);
   
   TH1F *Graph_Graph1009 = new TH1F("Graph_Graph1009","",100,1.8,4.2);
   Graph_Graph1009->SetMinimum(33.51558);
   Graph_Graph1009->SetMaximum(54.20544);
   Graph_Graph1009->SetDirectory(0);
   Graph_Graph1009->SetStats(0);
   Graph_Graph1009->SetLineStyle(0);
   Graph_Graph1009->SetMarkerStyle(20);
   Graph_Graph1009->GetXaxis()->SetLabelFont(42);
   Graph_Graph1009->GetXaxis()->SetLabelOffset(0.007);
   Graph_Graph1009->GetXaxis()->SetTitleOffset(1.1);
   Graph_Graph1009->GetXaxis()->SetTitleFont(42);
   Graph_Graph1009->GetYaxis()->SetLabelFont(42);
   Graph_Graph1009->GetYaxis()->SetLabelOffset(0.007);
   Graph_Graph1009->GetYaxis()->SetTitleOffset(1.2);
   Graph_Graph1009->GetYaxis()->SetTitleFont(42);
   Graph_Graph1009->GetZaxis()->SetLabelFont(42);
   Graph_Graph1009->GetZaxis()->SetLabelOffset(0.007);
   Graph_Graph1009->GetZaxis()->SetLabelSize(0.05);
   Graph_Graph1009->GetZaxis()->SetTitleSize(0.06);
   Graph_Graph1009->GetZaxis()->SetTitleFont(42);
   gre->SetHistogram(Graph_Graph1009);
   
   
   TF1 *fitR1010 = new TF1("fitR","[1]*1./pow(x,[0])",1.8,4.2, TF1::EAddToList::kNo);
   fitR1010->SetFillColor(19);
   fitR1010->SetFillStyle(0);
   fitR1010->SetMarkerStyle(20);

   ci = TColor::GetColor("#ff0000");
   fitR1010->SetLineColor(ci);
   fitR1010->SetLineWidth(1);
   fitR1010->SetChisquare(4.318129);
   fitR1010->SetNDF(1);
   fitR1010->GetXaxis()->SetLabelFont(42);
   fitR1010->GetXaxis()->SetLabelOffset(0.007);
   fitR1010->GetXaxis()->SetTitleOffset(1.1);
   fitR1010->GetXaxis()->SetTitleFont(42);
   fitR1010->GetYaxis()->SetLabelFont(42);
   fitR1010->GetYaxis()->SetLabelOffset(0.007);
   fitR1010->GetYaxis()->SetTitleOffset(1.2);
   fitR1010->GetYaxis()->SetTitleFont(42);
   fitR1010->SetParameter(0,0.6076079);
   fitR1010->SetParError(0,0.1005233);
   fitR1010->SetParLimits(0,0,0);
   fitR1010->SetParameter(1,79.15288);
   fitR1010->SetParError(1,7.958715);
   fitR1010->SetParLimits(1,0,0);
   fitR1010->SetParent(gre);
   gre->GetListOfFunctions()->Add(fitR1010);
   gre->Draw("p");
   
   TLegend *leg = new TLegend(0.65,0.65,0.89,0.89,NULL,"brNDC");
   leg->SetTextFont(62);
   leg->SetLineColor(1);
   leg->SetLineStyle(1);
   leg->SetLineWidth(1);
   leg->SetFillColor(0);
   leg->SetFillStyle(1001);
   TLegendEntry *entry=leg->AddEntry("","t_{left} - t_{MCP}","PL");

   ci = TColor::GetColor("#0000ff");
   entry->SetLineColor(ci);
   entry->SetLineStyle(1);
   entry->SetLineWidth(1);

   ci = TColor::GetColor("#0000ff");
   entry->SetMarkerColor(ci);
   entry->SetMarkerStyle(20);
   entry->SetMarkerSize(0.9);
   entry->SetTextFont(62);
   entry=leg->AddEntry("","t_{right} - t_{MCP}","PL");

   ci = TColor::GetColor("#ff0000");
   entry->SetLineColor(ci);
   entry->SetLineStyle(1);
   entry->SetLineWidth(1);

   ci = TColor::GetColor("#ff0000");
   entry->SetMarkerColor(ci);
   entry->SetMarkerStyle(20);
   entry->SetMarkerSize(0.9);
   entry->SetTextFont(62);
   entry=leg->AddEntry("","t_{average} - t_{MCP}","PL");
   entry->SetLineColor(1);
   entry->SetLineStyle(1);
   entry->SetLineWidth(1);
   entry->SetMarkerColor(1);
   entry->SetMarkerStyle(20);
   entry->SetMarkerSize(0.9);
   entry->SetTextFont(62);
   entry=leg->AddEntry("","t_{diff}/2","PL");
   entry->SetLineColor(1);
   entry->SetLineStyle(1);
   entry->SetLineWidth(1);
   entry->SetMarkerColor(1);
   entry->SetMarkerStyle(24);
   entry->SetMarkerSize(1);
   entry->SetTextFont(62);
   leg->Draw();
   TLatex *   tex = new TLatex(0.13,0.92,"LYSO:Ce 3xtx57 mm^{3} - FBK NUV-HD-TE 5x5 mm^{2}");
tex->SetNDC();
   tex->SetTextSize(0.035);
   tex->SetLineWidth(2);
   tex->Draw();
      tex = new TLatex(0.15,0.23,"#sigma_{tAve} ~  x^{- (0.48 +/- 0.03)}");
tex->SetNDC();
   tex->SetTextSize(0.04);
   tex->SetLineWidth(2);
   tex->Draw();
      tex = new TLatex(0.15,0.15,"#sigma_{tDiff/2} ~ x^{- (0.51 +/- 0.03)}");
tex->SetNDC();
   tex->SetTextSize(0.04);
   tex->SetLineWidth(2);
   tex->Draw();
   timeResolution_vs_thickness_BAR0->Modified();
   timeResolution_vs_thickness_BAR0->cd();
   timeResolution_vs_thickness_BAR0->SetSelected(timeResolution_vs_thickness_BAR0);
}
