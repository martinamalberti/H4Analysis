void scan_LEthresholds_Vbias72_xyangle90_HPK_tDiff()
{
//=========Macro generated from canvas: scan_LEthresholds_Vbias72_xyangle90_HPK/scan_LEthresholds_Vbias72_xyangle90_HPK
//=========  (Sat Feb 20 10:54:01 2021) by ROOT version 6.14/04
   TCanvas *scan_LEthresholds_Vbias72_xyangle90_HPK = new TCanvas("scan_LEthresholds_Vbias72_xyangle90_HPK", "scan_LEthresholds_Vbias72_xyangle90_HPK",0,0,600,600);
   gStyle->SetOptStat(0);
   gStyle->SetOptTitle(0);
   scan_LEthresholds_Vbias72_xyangle90_HPK->SetHighLightColor(2);
   scan_LEthresholds_Vbias72_xyangle90_HPK->Range(-44.19254,10.76923,295.7501,87.69231);
   scan_LEthresholds_Vbias72_xyangle90_HPK->SetFillColor(0);
   scan_LEthresholds_Vbias72_xyangle90_HPK->SetBorderMode(0);
   scan_LEthresholds_Vbias72_xyangle90_HPK->SetBorderSize(2);
   scan_LEthresholds_Vbias72_xyangle90_HPK->SetTickx(1);
   scan_LEthresholds_Vbias72_xyangle90_HPK->SetTicky(1);
   scan_LEthresholds_Vbias72_xyangle90_HPK->SetLeftMargin(0.13);
   scan_LEthresholds_Vbias72_xyangle90_HPK->SetRightMargin(0.08);
   scan_LEthresholds_Vbias72_xyangle90_HPK->SetBottomMargin(0.12);
   scan_LEthresholds_Vbias72_xyangle90_HPK->SetFrameFillStyle(0);
   scan_LEthresholds_Vbias72_xyangle90_HPK->SetFrameBorderMode(0);
   scan_LEthresholds_Vbias72_xyangle90_HPK->SetFrameFillStyle(0);
   scan_LEthresholds_Vbias72_xyangle90_HPK->SetFrameBorderMode(0);
   
   TH2F *__10 = new TH2F("__10","",100,0,268.5547,100,20,80);
   __10->SetStats(0);
   __10->SetLineStyle(0);
   __10->SetMarkerStyle(20);
   __10->GetXaxis()->SetTitle("threshold (mV)");
   __10->GetXaxis()->SetLabelFont(42);
   __10->GetXaxis()->SetLabelOffset(0.007);
   __10->GetXaxis()->SetLabelSize(0.05);
   __10->GetXaxis()->SetTitleSize(0.06);
   __10->GetXaxis()->SetTitleOffset(0.9);
   __10->GetXaxis()->SetTitleFont(42);
   __10->GetYaxis()->SetTitle("time resolution (ps)");
   __10->GetYaxis()->SetLabelFont(42);
   __10->GetYaxis()->SetLabelOffset(0.007);
   __10->GetYaxis()->SetLabelSize(0.05);
   __10->GetYaxis()->SetTitleSize(0.06);
   __10->GetYaxis()->SetTitleOffset(1.05);
   __10->GetYaxis()->SetTitleFont(42);
   __10->GetZaxis()->SetLabelFont(42);
   __10->GetZaxis()->SetLabelOffset(0.007);
   __10->GetZaxis()->SetLabelSize(0.05);
   __10->GetZaxis()->SetTitleSize(0.06);
   __10->GetZaxis()->SetTitleFont(42);
   __10->Draw("");
   
   Double_t _fx1142[21] = {
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
   Double_t _fy1142[21] = {
   39.40348,
   28.72976,
   28.8448,
   29.01161,
   29.9742,
   31.6699,
   32.22377,
   32.32706,
   32.66615,
   33.36673,
   34.38056,
   35.7082,
   37.51466,
   37.58181,
   39.35528,
   42.07249,
   44.01901,
   46.21113,
   49.88963,
   51.65202,
   56.25474};
   Double_t _fex1142[21] = {
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
   Double_t _fey1142[21] = {
   2.424631,
   1.447332,
   1.343748,
   1.469897,
   1.455013,
   1.496058,
   1.498589,
   1.504287,
   1.555741,
   1.572842,
   1.630633,
   1.705419,
   1.992382,
   1.963791,
   2.078268,
   2.328236,
   2.473777,
   2.734177,
   3.053693,
   3.027059,
   3.740881};
   TGraphErrors *gre = new TGraphErrors(21,_fx1142,_fy1142,_fex1142,_fey1142);
   gre->SetName("");
   gre->SetTitle("");
   gre->SetFillStyle(1000);

   Int_t ci;      // for color index setting
   TColor *color; // for color definition with alpha
   ci = TColor::GetColor("#ff9933");
   gre->SetLineColor(ci);

   ci = TColor::GetColor("#ff9933");
   gre->SetMarkerColor(ci);
   gre->SetMarkerStyle(20);
   
   TH1F *Graph_Graph_Graph11271142 = new TH1F("Graph_Graph_Graph11271142","",100,0,268.0664);
   Graph_Graph_Graph11271142->SetMinimum(24.0111);
   Graph_Graph_Graph11271142->SetMaximum(63.26694);
   Graph_Graph_Graph11271142->SetDirectory(0);
   Graph_Graph_Graph11271142->SetStats(0);
   Graph_Graph_Graph11271142->SetLineStyle(0);
   Graph_Graph_Graph11271142->SetMarkerStyle(20);
   Graph_Graph_Graph11271142->GetXaxis()->SetLabelFont(42);
   Graph_Graph_Graph11271142->GetXaxis()->SetLabelOffset(0.007);
   Graph_Graph_Graph11271142->GetXaxis()->SetLabelSize(0.05);
   Graph_Graph_Graph11271142->GetXaxis()->SetTitleSize(0.06);
   Graph_Graph_Graph11271142->GetXaxis()->SetTitleOffset(0.9);
   Graph_Graph_Graph11271142->GetXaxis()->SetTitleFont(42);
   Graph_Graph_Graph11271142->GetYaxis()->SetLabelFont(42);
   Graph_Graph_Graph11271142->GetYaxis()->SetLabelOffset(0.007);
   Graph_Graph_Graph11271142->GetYaxis()->SetLabelSize(0.05);
   Graph_Graph_Graph11271142->GetYaxis()->SetTitleSize(0.06);
   Graph_Graph_Graph11271142->GetYaxis()->SetTitleOffset(1.05);
   Graph_Graph_Graph11271142->GetYaxis()->SetTitleFont(42);
   Graph_Graph_Graph11271142->GetZaxis()->SetLabelFont(42);
   Graph_Graph_Graph11271142->GetZaxis()->SetLabelOffset(0.007);
   Graph_Graph_Graph11271142->GetZaxis()->SetLabelSize(0.05);
   Graph_Graph_Graph11271142->GetZaxis()->SetTitleSize(0.06);
   Graph_Graph_Graph11271142->GetZaxis()->SetTitleFont(42);
   gre->SetHistogram(Graph_Graph_Graph11271142);
   
   gre->Draw("pl");
   TLatex *   tex = new TLatex(0.13,0.92,"LYSO:Ce 3x3x57 mm^{3} - HPK S12572-015 3x3 mm^{2}");
tex->SetNDC();
   tex->SetTextSize(0.035);
   tex->SetLineWidth(2);
   tex->Draw();
   
   Double_t _fx1143[21] = {
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
   Double_t _fy1143[21] = {
   42.05244,
   29.80661,
   29.61738,
   28.42518,
   29.22421,
   29.77599,
   31.78673,
   31.85,
   32.5464,
   33.25205,
   34.22573,
   34.94651,
   35.97768,
   36.70759,
   37.21531,
   38.51413,
   39.13856,
   40.00377,
   41.29959,
   42.4873,
   44.13466};
   Double_t _fex1143[21] = {
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
   Double_t _fey1143[21] = {
   1.414891,
   0.8563021,
   0.8985616,
   0.8114015,
   0.8628558,
   0.8413066,
   0.8534196,
   0.8942797,
   0.8876646,
   0.9190722,
   0.9487664,
   0.9702972,
   1.023022,
   1.052294,
   1.035568,
   1.131995,
   1.139964,
   1.182509,
   1.202963,
   1.292317,
   1.342606};
   gre = new TGraphErrors(21,_fx1143,_fy1143,_fex1143,_fey1143);
   gre->SetName("");
   gre->SetTitle("");
   gre->SetFillStyle(1000);

   ci = TColor::GetColor("#00cc00");
   gre->SetLineColor(ci);

   ci = TColor::GetColor("#00cc00");
   gre->SetMarkerColor(ci);
   gre->SetMarkerStyle(21);
   
   TH1F *Graph_Graph_Graph11321143 = new TH1F("Graph_Graph_Graph11321143","",100,0,268.0664);
   Graph_Graph_Graph11321143->SetMinimum(25.82743);
   Graph_Graph_Graph11321143->SetMaximum(47.26361);
   Graph_Graph_Graph11321143->SetDirectory(0);
   Graph_Graph_Graph11321143->SetStats(0);
   Graph_Graph_Graph11321143->SetLineStyle(0);
   Graph_Graph_Graph11321143->SetMarkerStyle(20);
   Graph_Graph_Graph11321143->GetXaxis()->SetLabelFont(42);
   Graph_Graph_Graph11321143->GetXaxis()->SetLabelOffset(0.007);
   Graph_Graph_Graph11321143->GetXaxis()->SetLabelSize(0.05);
   Graph_Graph_Graph11321143->GetXaxis()->SetTitleSize(0.06);
   Graph_Graph_Graph11321143->GetXaxis()->SetTitleOffset(0.9);
   Graph_Graph_Graph11321143->GetXaxis()->SetTitleFont(42);
   Graph_Graph_Graph11321143->GetYaxis()->SetLabelFont(42);
   Graph_Graph_Graph11321143->GetYaxis()->SetLabelOffset(0.007);
   Graph_Graph_Graph11321143->GetYaxis()->SetLabelSize(0.05);
   Graph_Graph_Graph11321143->GetYaxis()->SetTitleSize(0.06);
   Graph_Graph_Graph11321143->GetYaxis()->SetTitleOffset(1.05);
   Graph_Graph_Graph11321143->GetYaxis()->SetTitleFont(42);
   Graph_Graph_Graph11321143->GetZaxis()->SetLabelFont(42);
   Graph_Graph_Graph11321143->GetZaxis()->SetLabelOffset(0.007);
   Graph_Graph_Graph11321143->GetZaxis()->SetLabelSize(0.05);
   Graph_Graph_Graph11321143->GetZaxis()->SetTitleSize(0.06);
   Graph_Graph_Graph11321143->GetZaxis()->SetTitleFont(42);
   gre->SetHistogram(Graph_Graph_Graph11321143);
   
   gre->Draw("pl");
      tex = new TLatex(0.13,0.92,"LYSO:Ce 3x3x57 mm^{3} - HPK S12572-015 3x3 mm^{2}");
tex->SetNDC();
   tex->SetTextSize(0.035);
   tex->SetLineWidth(2);
   tex->Draw();
   
   Double_t _fx1144[21] = {
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
   Double_t _fy1144[21] = {
   41.28289,
   32.00369,
   30.12542,
   31.00347,
   29.99082,
   31.74493,
   32.93309,
   34.61078,
   35.09953,
   36.07115,
   37.40945,
   38.06578,
   39.21237,
   40.60458,
   41.89853,
   43.84644,
   46.12182,
   48.10072,
   50.76396,
   52.29804,
   58.92683};
   Double_t _fex1144[21] = {
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
   Double_t _fey1144[21] = {
   1.863479,
   1.173277,
   1.131225,
   1.163969,
   1.034933,
   1.111951,
   1.109987,
   1.189075,
   1.197312,
   1.25772,
   1.3747,
   1.383447,
   1.438116,
   1.569715,
   1.576256,
   1.669961,
   1.889334,
   1.960347,
   2.029109,
   2.1762,
   2.526495};
   gre = new TGraphErrors(21,_fx1144,_fy1144,_fex1144,_fey1144);
   gre->SetName("");
   gre->SetTitle("");
   gre->SetFillStyle(1000);

   ci = TColor::GetColor("#0000cc");
   gre->SetLineColor(ci);

   ci = TColor::GetColor("#0000cc");
   gre->SetMarkerColor(ci);
   gre->SetMarkerStyle(22);
   
   TH1F *Graph_Graph_Graph11371144 = new TH1F("Graph_Graph_Graph11371144","",100,0,268.0664);
   Graph_Graph_Graph11371144->SetMinimum(25.70615);
   Graph_Graph_Graph11371144->SetMaximum(64.70307);
   Graph_Graph_Graph11371144->SetDirectory(0);
   Graph_Graph_Graph11371144->SetStats(0);
   Graph_Graph_Graph11371144->SetLineStyle(0);
   Graph_Graph_Graph11371144->SetMarkerStyle(20);
   Graph_Graph_Graph11371144->GetXaxis()->SetLabelFont(42);
   Graph_Graph_Graph11371144->GetXaxis()->SetLabelOffset(0.007);
   Graph_Graph_Graph11371144->GetXaxis()->SetLabelSize(0.05);
   Graph_Graph_Graph11371144->GetXaxis()->SetTitleSize(0.06);
   Graph_Graph_Graph11371144->GetXaxis()->SetTitleOffset(0.9);
   Graph_Graph_Graph11371144->GetXaxis()->SetTitleFont(42);
   Graph_Graph_Graph11371144->GetYaxis()->SetLabelFont(42);
   Graph_Graph_Graph11371144->GetYaxis()->SetLabelOffset(0.007);
   Graph_Graph_Graph11371144->GetYaxis()->SetLabelSize(0.05);
   Graph_Graph_Graph11371144->GetYaxis()->SetTitleSize(0.06);
   Graph_Graph_Graph11371144->GetYaxis()->SetTitleOffset(1.05);
   Graph_Graph_Graph11371144->GetYaxis()->SetTitleFont(42);
   Graph_Graph_Graph11371144->GetZaxis()->SetLabelFont(42);
   Graph_Graph_Graph11371144->GetZaxis()->SetLabelOffset(0.007);
   Graph_Graph_Graph11371144->GetZaxis()->SetLabelSize(0.05);
   Graph_Graph_Graph11371144->GetZaxis()->SetTitleSize(0.06);
   Graph_Graph_Graph11371144->GetZaxis()->SetTitleFont(42);
   gre->SetHistogram(Graph_Graph_Graph11371144);
   
   gre->Draw("pl");
      tex = new TLatex(0.13,0.92,"LYSO:Ce 3x3x57 mm^{3} - HPK S12572-015 3x3 mm^{2}");
tex->SetNDC();
   tex->SetTextSize(0.035);
   tex->SetLineWidth(2);
   tex->Draw();
   
   TLegend *leg = new TLegend(0.65,0.65,0.89,0.84,NULL,"brNDC");
   leg->SetBorderSize(0);
   leg->SetTextFont(62);
   leg->SetLineColor(1);
   leg->SetLineStyle(1);
   leg->SetLineWidth(1);
   leg->SetFillColor(0);
   leg->SetFillStyle(1001);
   TLegendEntry *entry=leg->AddEntry("","BAR0","PL");

   ci = TColor::GetColor("#ff9933");
   entry->SetLineColor(ci);
   entry->SetLineStyle(1);
   entry->SetLineWidth(1);

   ci = TColor::GetColor("#ff9933");
   entry->SetMarkerColor(ci);
   entry->SetMarkerStyle(20);
   entry->SetMarkerSize(1);
   entry->SetTextFont(62);
   entry=leg->AddEntry("","BAR1","PL");

   ci = TColor::GetColor("#00cc00");
   entry->SetLineColor(ci);
   entry->SetLineStyle(1);
   entry->SetLineWidth(1);

   ci = TColor::GetColor("#00cc00");
   entry->SetMarkerColor(ci);
   entry->SetMarkerStyle(21);
   entry->SetMarkerSize(1);
   entry->SetTextFont(62);
   entry=leg->AddEntry("","BAR2","PL");

   ci = TColor::GetColor("#0000cc");
   entry->SetLineColor(ci);
   entry->SetLineStyle(1);
   entry->SetLineWidth(1);

   ci = TColor::GetColor("#0000cc");
   entry->SetMarkerColor(ci);
   entry->SetMarkerStyle(22);
   entry->SetMarkerSize(1);
   entry->SetTextFont(62);
   leg->Draw();
      tex = new TLatex(0.13,0.92,"LYSO:Ce 3x3x57 mm^{3} - HPK S12572-015 3x3 mm^{2}");
tex->SetNDC();
   tex->SetTextSize(0.035);
   tex->SetLineWidth(2);
   tex->Draw();
   scan_LEthresholds_Vbias72_xyangle90_HPK->Modified();
   scan_LEthresholds_Vbias72_xyangle90_HPK->cd();
   scan_LEthresholds_Vbias72_xyangle90_HPK->SetSelected(scan_LEthresholds_Vbias72_xyangle90_HPK);
}
