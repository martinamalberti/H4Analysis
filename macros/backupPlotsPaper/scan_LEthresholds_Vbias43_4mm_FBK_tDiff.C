void scan_LEthresholds_Vbias43_4mm_FBK_tDiff()
{
//=========Macro generated from canvas: scan_LEthresholds_Vbias43_4mm_FBK/scan_LEthresholds_Vbias43_4mm_FBK
//=========  (Sat Feb 20 10:53:51 2021) by ROOT version 6.14/04
   TCanvas *scan_LEthresholds_Vbias43_4mm_FBK = new TCanvas("scan_LEthresholds_Vbias43_4mm_FBK", "scan_LEthresholds_Vbias43_4mm_FBK",0,0,600,600);
   gStyle->SetOptStat(0);
   gStyle->SetOptTitle(0);
   scan_LEthresholds_Vbias43_4mm_FBK->SetHighLightColor(2);
   scan_LEthresholds_Vbias43_4mm_FBK->Range(-44.19254,10.76923,295.7501,87.69231);
   scan_LEthresholds_Vbias43_4mm_FBK->SetFillColor(0);
   scan_LEthresholds_Vbias43_4mm_FBK->SetBorderMode(0);
   scan_LEthresholds_Vbias43_4mm_FBK->SetBorderSize(2);
   scan_LEthresholds_Vbias43_4mm_FBK->SetTickx(1);
   scan_LEthresholds_Vbias43_4mm_FBK->SetTicky(1);
   scan_LEthresholds_Vbias43_4mm_FBK->SetLeftMargin(0.13);
   scan_LEthresholds_Vbias43_4mm_FBK->SetRightMargin(0.08);
   scan_LEthresholds_Vbias43_4mm_FBK->SetBottomMargin(0.12);
   scan_LEthresholds_Vbias43_4mm_FBK->SetFrameFillStyle(0);
   scan_LEthresholds_Vbias43_4mm_FBK->SetFrameBorderMode(0);
   scan_LEthresholds_Vbias43_4mm_FBK->SetFrameFillStyle(0);
   scan_LEthresholds_Vbias43_4mm_FBK->SetFrameBorderMode(0);
   
   TH2F *__4 = new TH2F("__4","",100,0,268.5547,100,20,80);
   __4->SetStats(0);
   __4->SetLineStyle(0);
   __4->SetMarkerStyle(20);
   __4->GetXaxis()->SetTitle("threshold (mV)");
   __4->GetXaxis()->SetLabelFont(42);
   __4->GetXaxis()->SetLabelOffset(0.007);
   __4->GetXaxis()->SetLabelSize(0.05);
   __4->GetXaxis()->SetTitleSize(0.06);
   __4->GetXaxis()->SetTitleOffset(0.9);
   __4->GetXaxis()->SetTitleFont(42);
   __4->GetYaxis()->SetTitle("time resolution (ps)");
   __4->GetYaxis()->SetLabelFont(42);
   __4->GetYaxis()->SetLabelOffset(0.007);
   __4->GetYaxis()->SetLabelSize(0.05);
   __4->GetYaxis()->SetTitleSize(0.06);
   __4->GetYaxis()->SetTitleOffset(1.05);
   __4->GetYaxis()->SetTitleFont(42);
   __4->GetZaxis()->SetLabelFont(42);
   __4->GetZaxis()->SetLabelOffset(0.007);
   __4->GetZaxis()->SetLabelSize(0.05);
   __4->GetZaxis()->SetTitleSize(0.06);
   __4->GetZaxis()->SetTitleFont(42);
   __4->Draw("");
   
   Double_t _fx1048[21] = {
   4.882812,
   12.20703,
   17.08984,
   24.41406,
   36.62109,
   48.82812,
   61.03516,
   73.24219,
   85.44922,
   97.65625,
   109.8633,
   122.0703,
   134.2773,
   146.4844,
   158.6914,
   170.8984,
   183.1055,
   195.3125,
   207.5195,
   219.7266,
   244.1406};
   Double_t _fy1048[21] = {
   679.3033,
   63.53657,
   40.85679,
   34.32309,
   30.15184,
   27.78366,
   27.06883,
   26.72098,
   26.54468,
   26.35675,
   26.78795,
   26.8628,
   27.09259,
   26.94981,
   27.51235,
   27.78477,
   27.95495,
   28.46787,
   28.74952,
   28.98057,
   29.51887};
   Double_t _fex1048[21] = {
   0,
   0,
   0,
   0,
   0,
   0,
   0,
   0,
   0,
   0,
   0,
   0,
   0,
   0,
   0,
   0,
   0,
   0,
   0,
   0,
   0};
   Double_t _fey1048[21] = {
   28.52063,
   3.342867,
   1.089696,
   0.8635725,
   0.7610048,
   0.6624602,
   0.6588737,
   0.6518373,
   0.6361242,
   0.6396887,
   0.6342923,
   0.6285843,
   0.6605286,
   0.6528008,
   0.6627901,
   0.6744557,
   0.6600459,
   0.6937809,
   0.70023,
   0.7132279,
   0.6990834};
   TGraphErrors *gre = new TGraphErrors(21,_fx1048,_fy1048,_fex1048,_fey1048);
   gre->SetName("");
   gre->SetTitle("");
   gre->SetFillStyle(1000);

   Int_t ci;      // for color index setting
   TColor *color; // for color definition with alpha
   ci = TColor::GetColor("#0000cc");
   gre->SetLineColor(ci);

   ci = TColor::GetColor("#0000cc");
   gre->SetMarkerColor(ci);
   gre->SetMarkerStyle(20);
   
   TH1F *Graph_Graph_Graph10431048 = new TH1F("Graph_Graph_Graph10431048","",100,0,268.0664);
   Graph_Graph_Graph10431048->SetMinimum(23.14536);
   Graph_Graph_Graph10431048->SetMaximum(776.0346);
   Graph_Graph_Graph10431048->SetDirectory(0);
   Graph_Graph_Graph10431048->SetStats(0);
   Graph_Graph_Graph10431048->SetLineStyle(0);
   Graph_Graph_Graph10431048->SetMarkerStyle(20);
   Graph_Graph_Graph10431048->GetXaxis()->SetLabelFont(42);
   Graph_Graph_Graph10431048->GetXaxis()->SetLabelOffset(0.007);
   Graph_Graph_Graph10431048->GetXaxis()->SetLabelSize(0.05);
   Graph_Graph_Graph10431048->GetXaxis()->SetTitleSize(0.06);
   Graph_Graph_Graph10431048->GetXaxis()->SetTitleOffset(0.9);
   Graph_Graph_Graph10431048->GetXaxis()->SetTitleFont(42);
   Graph_Graph_Graph10431048->GetYaxis()->SetLabelFont(42);
   Graph_Graph_Graph10431048->GetYaxis()->SetLabelOffset(0.007);
   Graph_Graph_Graph10431048->GetYaxis()->SetLabelSize(0.05);
   Graph_Graph_Graph10431048->GetYaxis()->SetTitleSize(0.06);
   Graph_Graph_Graph10431048->GetYaxis()->SetTitleOffset(1.05);
   Graph_Graph_Graph10431048->GetYaxis()->SetTitleFont(42);
   Graph_Graph_Graph10431048->GetZaxis()->SetLabelFont(42);
   Graph_Graph_Graph10431048->GetZaxis()->SetLabelOffset(0.007);
   Graph_Graph_Graph10431048->GetZaxis()->SetLabelSize(0.05);
   Graph_Graph_Graph10431048->GetZaxis()->SetTitleSize(0.06);
   Graph_Graph_Graph10431048->GetZaxis()->SetTitleFont(42);
   gre->SetHistogram(Graph_Graph_Graph10431048);
   
   gre->Draw("pl");
   TLatex *   tex = new TLatex(0.13,0.92,"LYSO:Ce 3x4x57 mm^{3} - FBK NUV-HD-TE 5x5 mm^{2}");
tex->SetNDC();
   tex->SetTextSize(0.035);
   tex->SetLineWidth(2);
   tex->Draw();
      tex = new TLatex(0.13,0.92,"LYSO:Ce 3x4x57 mm^{3} - FBK NUV-HD-TE 5x5 mm^{2}");
tex->SetNDC();
   tex->SetTextSize(0.035);
   tex->SetLineWidth(2);
   tex->Draw();
   scan_LEthresholds_Vbias43_4mm_FBK->Modified();
   scan_LEthresholds_Vbias43_4mm_FBK->cd();
   scan_LEthresholds_Vbias43_4mm_FBK->SetSelected(scan_LEthresholds_Vbias43_4mm_FBK);
}
