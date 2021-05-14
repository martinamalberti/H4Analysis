void scan_Vbias_LEthrOpt_4mm_FBK()
{
//=========Macro generated from canvas: scan_Vbias_LEthrOpt_4mm_FBK/scan_Vbias_LEthrOpt_4mm_FBK
//=========  (Sat Feb 20 10:53:51 2021) by ROOT version 6.14/04
   TCanvas *scan_Vbias_LEthrOpt_4mm_FBK = new TCanvas("scan_Vbias_LEthrOpt_4mm_FBK", "scan_Vbias_LEthrOpt_4mm_FBK",0,0,600,600);
   gStyle->SetOptStat(0);
   gStyle->SetOptTitle(0);
   scan_Vbias_LEthrOpt_4mm_FBK->SetHighLightColor(2);
   scan_Vbias_LEthrOpt_4mm_FBK->Range(-1.151899,12.30769,7.708861,76.41026);
   scan_Vbias_LEthrOpt_4mm_FBK->SetFillColor(0);
   scan_Vbias_LEthrOpt_4mm_FBK->SetBorderMode(0);
   scan_Vbias_LEthrOpt_4mm_FBK->SetBorderSize(2);
   scan_Vbias_LEthrOpt_4mm_FBK->SetTickx(1);
   scan_Vbias_LEthrOpt_4mm_FBK->SetTicky(1);
   scan_Vbias_LEthrOpt_4mm_FBK->SetLeftMargin(0.13);
   scan_Vbias_LEthrOpt_4mm_FBK->SetRightMargin(0.08);
   scan_Vbias_LEthrOpt_4mm_FBK->SetBottomMargin(0.12);
   scan_Vbias_LEthrOpt_4mm_FBK->SetFrameFillStyle(0);
   scan_Vbias_LEthrOpt_4mm_FBK->SetFrameBorderMode(0);
   scan_Vbias_LEthrOpt_4mm_FBK->SetFrameFillStyle(0);
   scan_Vbias_LEthrOpt_4mm_FBK->SetFrameBorderMode(0);
   
   TH2F *__5 = new TH2F("__5","",10,0,7,100,20,70);
   __5->SetStats(0);
   __5->SetLineStyle(0);
   __5->SetMarkerStyle(20);
   __5->GetXaxis()->SetTitle("OV (V)");
   __5->GetXaxis()->SetLabelFont(42);
   __5->GetXaxis()->SetLabelOffset(0.007);
   __5->GetXaxis()->SetLabelSize(0.05);
   __5->GetXaxis()->SetTitleSize(0.06);
   __5->GetXaxis()->SetTitleOffset(0.9);
   __5->GetXaxis()->SetTitleFont(42);
   __5->GetYaxis()->SetTitle("time resolution (ps)");
   __5->GetYaxis()->SetLabelFont(42);
   __5->GetYaxis()->SetLabelOffset(0.007);
   __5->GetYaxis()->SetLabelSize(0.05);
   __5->GetYaxis()->SetTitleSize(0.06);
   __5->GetYaxis()->SetTitleOffset(1.05);
   __5->GetYaxis()->SetTitleFont(42);
   __5->GetZaxis()->SetLabelFont(42);
   __5->GetZaxis()->SetLabelOffset(0.007);
   __5->GetZaxis()->SetLabelSize(0.05);
   __5->GetZaxis()->SetTitleSize(0.06);
   __5->GetZaxis()->SetTitleFont(42);
   __5->Draw("");
   
   Double_t _fx1049[5] = {
   6,
   5,
   4,
   3,
   2};
   Double_t _fy1049[5] = {
   26.35675,
   27.70481,
   28.8834,
   31.937,
   38.27177};
   Double_t _fex1049[5] = {
   0,
   0,
   0,
   0,
   0};
   Double_t _fey1049[5] = {
   0.6396887,
   0.7084773,
   0.7141467,
   0.6963194,
   1.147337};
   TGraphErrors *gre = new TGraphErrors(5,_fx1049,_fy1049,_fex1049,_fey1049);
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
   
   TH1F *Graph_Graph1049 = new TH1F("Graph_Graph1049","",100,1.6,6.4);
   Graph_Graph1049->SetMinimum(24.34686);
   Graph_Graph1049->SetMaximum(40.78931);
   Graph_Graph1049->SetDirectory(0);
   Graph_Graph1049->SetStats(0);
   Graph_Graph1049->SetLineStyle(0);
   Graph_Graph1049->SetMarkerStyle(20);
   Graph_Graph1049->GetXaxis()->SetLabelFont(42);
   Graph_Graph1049->GetXaxis()->SetLabelOffset(0.007);
   Graph_Graph1049->GetXaxis()->SetLabelSize(0.05);
   Graph_Graph1049->GetXaxis()->SetTitleSize(0.06);
   Graph_Graph1049->GetXaxis()->SetTitleOffset(0.9);
   Graph_Graph1049->GetXaxis()->SetTitleFont(42);
   Graph_Graph1049->GetYaxis()->SetLabelFont(42);
   Graph_Graph1049->GetYaxis()->SetLabelOffset(0.007);
   Graph_Graph1049->GetYaxis()->SetLabelSize(0.05);
   Graph_Graph1049->GetYaxis()->SetTitleSize(0.06);
   Graph_Graph1049->GetYaxis()->SetTitleOffset(1.05);
   Graph_Graph1049->GetYaxis()->SetTitleFont(42);
   Graph_Graph1049->GetZaxis()->SetLabelFont(42);
   Graph_Graph1049->GetZaxis()->SetLabelOffset(0.007);
   Graph_Graph1049->GetZaxis()->SetLabelSize(0.05);
   Graph_Graph1049->GetZaxis()->SetTitleSize(0.06);
   Graph_Graph1049->GetZaxis()->SetTitleFont(42);
   gre->SetHistogram(Graph_Graph1049);
   
   gre->Draw("pl");
   TLatex *   tex = new TLatex(0.13,0.92,"LYSO:Ce 3x4x57 mm^{3} - FBK NUV-HD-TE 5x5 mm^{2}");
tex->SetNDC();
   tex->SetTextSize(0.035);
   tex->SetLineWidth(2);
   tex->Draw();
   scan_Vbias_LEthrOpt_4mm_FBK->Modified();
   scan_Vbias_LEthrOpt_4mm_FBK->cd();
   scan_Vbias_LEthrOpt_4mm_FBK->SetSelected(scan_Vbias_LEthrOpt_4mm_FBK);
}
