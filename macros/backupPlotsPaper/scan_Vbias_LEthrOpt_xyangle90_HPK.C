void scan_Vbias_LEthrOpt_xyangle90_HPK()
{
//=========Macro generated from canvas: scan_Vbias_LEthrOpt_xyangle90_HPK/scan_Vbias_LEthrOpt_angle90_HPK
//=========  (Sat Feb 20 10:54:01 2021) by ROOT version 6.14/04
   TCanvas *scan_Vbias_LEthrOpt_xyangle90_HPK = new TCanvas("scan_Vbias_LEthrOpt_xyangle90_HPK", "scan_Vbias_LEthrOpt_angle90_HPK",0,0,600,600);
   gStyle->SetOptStat(0);
   gStyle->SetOptTitle(0);
   scan_Vbias_LEthrOpt_xyangle90_HPK->SetHighLightColor(2);
   scan_Vbias_LEthrOpt_xyangle90_HPK->Range(-1.151899,12.30769,7.708861,76.41026);
   scan_Vbias_LEthrOpt_xyangle90_HPK->SetFillColor(0);
   scan_Vbias_LEthrOpt_xyangle90_HPK->SetBorderMode(0);
   scan_Vbias_LEthrOpt_xyangle90_HPK->SetBorderSize(2);
   scan_Vbias_LEthrOpt_xyangle90_HPK->SetTickx(1);
   scan_Vbias_LEthrOpt_xyangle90_HPK->SetTicky(1);
   scan_Vbias_LEthrOpt_xyangle90_HPK->SetLeftMargin(0.13);
   scan_Vbias_LEthrOpt_xyangle90_HPK->SetRightMargin(0.08);
   scan_Vbias_LEthrOpt_xyangle90_HPK->SetBottomMargin(0.12);
   scan_Vbias_LEthrOpt_xyangle90_HPK->SetFrameFillStyle(0);
   scan_Vbias_LEthrOpt_xyangle90_HPK->SetFrameBorderMode(0);
   scan_Vbias_LEthrOpt_xyangle90_HPK->SetFrameFillStyle(0);
   scan_Vbias_LEthrOpt_xyangle90_HPK->SetFrameBorderMode(0);
   
   TH2F *__11 = new TH2F("__11","",10,0,7,100,20,70);
   __11->SetStats(0);
   __11->SetLineStyle(0);
   __11->SetMarkerStyle(20);
   __11->GetXaxis()->SetTitle("OV (V)");
   __11->GetXaxis()->SetLabelFont(42);
   __11->GetXaxis()->SetLabelOffset(0.007);
   __11->GetXaxis()->SetLabelSize(0.05);
   __11->GetXaxis()->SetTitleSize(0.06);
   __11->GetXaxis()->SetTitleOffset(0.9);
   __11->GetXaxis()->SetTitleFont(42);
   __11->GetYaxis()->SetTitle("time resolution (ps)");
   __11->GetYaxis()->SetLabelFont(42);
   __11->GetYaxis()->SetLabelOffset(0.007);
   __11->GetYaxis()->SetLabelSize(0.05);
   __11->GetYaxis()->SetTitleSize(0.06);
   __11->GetYaxis()->SetTitleOffset(1.05);
   __11->GetYaxis()->SetTitleFont(42);
   __11->GetZaxis()->SetLabelFont(42);
   __11->GetZaxis()->SetLabelOffset(0.007);
   __11->GetZaxis()->SetLabelSize(0.05);
   __11->GetZaxis()->SetTitleSize(0.06);
   __11->GetZaxis()->SetTitleFont(42);
   __11->Draw("");
   
   Double_t _fx1145[5] = {
   5.65,
   4.65,
   3.65,
   2.65,
   1.65};
   Double_t _fy1145[5] = {
   28.72976,
   28.77826,
   34.38408,
   41.13879,
   53.12531};
   Double_t _fex1145[5] = {
   0,
   0,
   0,
   0,
   0};
   Double_t _fey1145[5] = {
   1.447332,
   1.41553,
   1.852385,
   2.094898,
   3.736828};
   TGraphErrors *gre = new TGraphErrors(5,_fx1145,_fy1145,_fex1145,_fey1145);
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
   
   TH1F *Graph_Graph1145 = new TH1F("Graph_Graph1145","",100,1.25,6.05);
   Graph_Graph1145->SetMinimum(24.32445);
   Graph_Graph1145->SetMaximum(59.82011);
   Graph_Graph1145->SetDirectory(0);
   Graph_Graph1145->SetStats(0);
   Graph_Graph1145->SetLineStyle(0);
   Graph_Graph1145->SetMarkerStyle(20);
   Graph_Graph1145->GetXaxis()->SetLabelFont(42);
   Graph_Graph1145->GetXaxis()->SetLabelOffset(0.007);
   Graph_Graph1145->GetXaxis()->SetLabelSize(0.05);
   Graph_Graph1145->GetXaxis()->SetTitleSize(0.06);
   Graph_Graph1145->GetXaxis()->SetTitleOffset(0.9);
   Graph_Graph1145->GetXaxis()->SetTitleFont(42);
   Graph_Graph1145->GetYaxis()->SetLabelFont(42);
   Graph_Graph1145->GetYaxis()->SetLabelOffset(0.007);
   Graph_Graph1145->GetYaxis()->SetLabelSize(0.05);
   Graph_Graph1145->GetYaxis()->SetTitleSize(0.06);
   Graph_Graph1145->GetYaxis()->SetTitleOffset(1.05);
   Graph_Graph1145->GetYaxis()->SetTitleFont(42);
   Graph_Graph1145->GetZaxis()->SetLabelFont(42);
   Graph_Graph1145->GetZaxis()->SetLabelOffset(0.007);
   Graph_Graph1145->GetZaxis()->SetLabelSize(0.05);
   Graph_Graph1145->GetZaxis()->SetTitleSize(0.06);
   Graph_Graph1145->GetZaxis()->SetTitleFont(42);
   gre->SetHistogram(Graph_Graph1145);
   
   gre->Draw("pl");
   TLatex *   tex = new TLatex(0.13,0.92,"LYSO:Ce 3x3x57 mm^{3} - HPK S12572-015 3x3 mm^{2}");
tex->SetNDC();
   tex->SetTextSize(0.035);
   tex->SetLineWidth(2);
   tex->Draw();
   
   Double_t _fx1146[5] = {
   5.65,
   4.65,
   3.65,
   2.65,
   1.65};
   Double_t _fy1146[5] = {
   28.42518,
   30.36794,
   32.95441,
   38.74025,
   49.6887};
   Double_t _fex1146[5] = {
   0,
   0,
   0,
   0,
   0};
   Double_t _fey1146[5] = {
   0.8114015,
   0.8371492,
   0.9101066,
   0.9512217,
   1.560866};
   gre = new TGraphErrors(5,_fx1146,_fy1146,_fex1146,_fey1146);
   gre->SetName("");
   gre->SetTitle("");
   gre->SetFillStyle(1000);

   ci = TColor::GetColor("#00cc00");
   gre->SetLineColor(ci);

   ci = TColor::GetColor("#00cc00");
   gre->SetMarkerColor(ci);
   gre->SetMarkerStyle(21);
   
   TH1F *Graph_Graph1146 = new TH1F("Graph_Graph1146","",100,1.25,6.05);
   Graph_Graph1146->SetMinimum(25.2502);
   Graph_Graph1146->SetMaximum(53.61314);
   Graph_Graph1146->SetDirectory(0);
   Graph_Graph1146->SetStats(0);
   Graph_Graph1146->SetLineStyle(0);
   Graph_Graph1146->SetMarkerStyle(20);
   Graph_Graph1146->GetXaxis()->SetLabelFont(42);
   Graph_Graph1146->GetXaxis()->SetLabelOffset(0.007);
   Graph_Graph1146->GetXaxis()->SetLabelSize(0.05);
   Graph_Graph1146->GetXaxis()->SetTitleSize(0.06);
   Graph_Graph1146->GetXaxis()->SetTitleOffset(0.9);
   Graph_Graph1146->GetXaxis()->SetTitleFont(42);
   Graph_Graph1146->GetYaxis()->SetLabelFont(42);
   Graph_Graph1146->GetYaxis()->SetLabelOffset(0.007);
   Graph_Graph1146->GetYaxis()->SetLabelSize(0.05);
   Graph_Graph1146->GetYaxis()->SetTitleSize(0.06);
   Graph_Graph1146->GetYaxis()->SetTitleOffset(1.05);
   Graph_Graph1146->GetYaxis()->SetTitleFont(42);
   Graph_Graph1146->GetZaxis()->SetLabelFont(42);
   Graph_Graph1146->GetZaxis()->SetLabelOffset(0.007);
   Graph_Graph1146->GetZaxis()->SetLabelSize(0.05);
   Graph_Graph1146->GetZaxis()->SetTitleSize(0.06);
   Graph_Graph1146->GetZaxis()->SetTitleFont(42);
   gre->SetHistogram(Graph_Graph1146);
   
   gre->Draw("pl");
      tex = new TLatex(0.13,0.92,"LYSO:Ce 3x3x57 mm^{3} - HPK S12572-015 3x3 mm^{2}");
tex->SetNDC();
   tex->SetTextSize(0.035);
   tex->SetLineWidth(2);
   tex->Draw();
   
   Double_t _fx1147[5] = {
   5.65,
   4.65,
   3.65,
   2.65,
   1.65};
   Double_t _fy1147[5] = {
   29.99082,
   31.2958,
   36.51118,
   42.20048,
   57.44607};
   Double_t _fex1147[5] = {
   0,
   0,
   0,
   0,
   0};
   Double_t _fey1147[5] = {
   1.034933,
   1.201989,
   1.467274,
   1.328938,
   2.567984};
   gre = new TGraphErrors(5,_fx1147,_fy1147,_fex1147,_fey1147);
   gre->SetName("");
   gre->SetTitle("");
   gre->SetFillStyle(1000);

   ci = TColor::GetColor("#0000cc");
   gre->SetLineColor(ci);

   ci = TColor::GetColor("#0000cc");
   gre->SetMarkerColor(ci);
   gre->SetMarkerStyle(22);
   
   TH1F *Graph_Graph1147 = new TH1F("Graph_Graph1147","",100,1.25,6.05);
   Graph_Graph1147->SetMinimum(25.85007);
   Graph_Graph1147->SetMaximum(63.11987);
   Graph_Graph1147->SetDirectory(0);
   Graph_Graph1147->SetStats(0);
   Graph_Graph1147->SetLineStyle(0);
   Graph_Graph1147->SetMarkerStyle(20);
   Graph_Graph1147->GetXaxis()->SetLabelFont(42);
   Graph_Graph1147->GetXaxis()->SetLabelOffset(0.007);
   Graph_Graph1147->GetXaxis()->SetLabelSize(0.05);
   Graph_Graph1147->GetXaxis()->SetTitleSize(0.06);
   Graph_Graph1147->GetXaxis()->SetTitleOffset(0.9);
   Graph_Graph1147->GetXaxis()->SetTitleFont(42);
   Graph_Graph1147->GetYaxis()->SetLabelFont(42);
   Graph_Graph1147->GetYaxis()->SetLabelOffset(0.007);
   Graph_Graph1147->GetYaxis()->SetLabelSize(0.05);
   Graph_Graph1147->GetYaxis()->SetTitleSize(0.06);
   Graph_Graph1147->GetYaxis()->SetTitleOffset(1.05);
   Graph_Graph1147->GetYaxis()->SetTitleFont(42);
   Graph_Graph1147->GetZaxis()->SetLabelFont(42);
   Graph_Graph1147->GetZaxis()->SetLabelOffset(0.007);
   Graph_Graph1147->GetZaxis()->SetLabelSize(0.05);
   Graph_Graph1147->GetZaxis()->SetTitleSize(0.06);
   Graph_Graph1147->GetZaxis()->SetTitleFont(42);
   gre->SetHistogram(Graph_Graph1147);
   
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
   scan_Vbias_LEthrOpt_xyangle90_HPK->Modified();
   scan_Vbias_LEthrOpt_xyangle90_HPK->cd();
   scan_Vbias_LEthrOpt_xyangle90_HPK->SetSelected(scan_Vbias_LEthrOpt_xyangle90_HPK);
}
