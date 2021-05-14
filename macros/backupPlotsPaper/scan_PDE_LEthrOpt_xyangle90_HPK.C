void scan_PDE_LEthrOpt_xyangle90_HPK()
{
//=========Macro generated from canvas: scan_PDE_LEthrOpt_xyangle90_HPK/scan_PDE_LEthrOpt_xyangle90_HPK
//=========  (Sat Feb 20 10:54:01 2021) by ROOT version 6.14/04
   TCanvas *scan_PDE_LEthrOpt_xyangle90_HPK = new TCanvas("scan_PDE_LEthrOpt_xyangle90_HPK", "scan_PDE_LEthrOpt_xyangle90_HPK",0,0,600,600);
   gStyle->SetOptStat(0);
   gStyle->SetOptTitle(0);
   scan_PDE_LEthrOpt_xyangle90_HPK->SetHighLightColor(2);
   scan_PDE_LEthrOpt_xyangle90_HPK->Range(-1.582278,12.30769,49.05063,76.41026);
   scan_PDE_LEthrOpt_xyangle90_HPK->SetFillColor(0);
   scan_PDE_LEthrOpt_xyangle90_HPK->SetBorderMode(0);
   scan_PDE_LEthrOpt_xyangle90_HPK->SetBorderSize(2);
   scan_PDE_LEthrOpt_xyangle90_HPK->SetTickx(1);
   scan_PDE_LEthrOpt_xyangle90_HPK->SetTicky(1);
   scan_PDE_LEthrOpt_xyangle90_HPK->SetLeftMargin(0.13);
   scan_PDE_LEthrOpt_xyangle90_HPK->SetRightMargin(0.08);
   scan_PDE_LEthrOpt_xyangle90_HPK->SetBottomMargin(0.12);
   scan_PDE_LEthrOpt_xyangle90_HPK->SetFrameFillStyle(0);
   scan_PDE_LEthrOpt_xyangle90_HPK->SetFrameBorderMode(0);
   scan_PDE_LEthrOpt_xyangle90_HPK->SetFrameFillStyle(0);
   scan_PDE_LEthrOpt_xyangle90_HPK->SetFrameBorderMode(0);
   
   TH2F *__12 = new TH2F("__12","",10,5,45,100,20,70);
   __12->SetStats(0);
   __12->SetLineStyle(0);
   __12->SetMarkerStyle(20);
   __12->GetXaxis()->SetTitle("PDE (%)");
   __12->GetXaxis()->SetLabelFont(42);
   __12->GetXaxis()->SetLabelOffset(0.007);
   __12->GetXaxis()->SetLabelSize(0.05);
   __12->GetXaxis()->SetTitleSize(0.06);
   __12->GetXaxis()->SetTitleOffset(0.9);
   __12->GetXaxis()->SetTitleFont(42);
   __12->GetYaxis()->SetTitle("time resolution (ps)");
   __12->GetYaxis()->SetLabelFont(42);
   __12->GetYaxis()->SetLabelOffset(0.007);
   __12->GetYaxis()->SetLabelSize(0.05);
   __12->GetYaxis()->SetTitleSize(0.06);
   __12->GetYaxis()->SetTitleOffset(1.05);
   __12->GetYaxis()->SetTitleFont(42);
   __12->GetZaxis()->SetLabelFont(42);
   __12->GetZaxis()->SetLabelOffset(0.007);
   __12->GetZaxis()->SetLabelSize(0.05);
   __12->GetZaxis()->SetTitleSize(0.06);
   __12->GetZaxis()->SetTitleFont(42);
   __12->Draw("");
   
   Double_t _fx1148[5] = {
   34.80492,
   31.2419,
   27.38275,
   22.35237,
   15.27564};
   Double_t _fy1148[5] = {
   28.72976,
   28.77826,
   34.38408,
   41.13879,
   53.12531};
   Double_t _fex1148[5] = {
   0.7424989,
   0.7142128,
   0.860949,
   1.182708,
   1.679489};
   Double_t _fey1148[5] = {
   1.447332,
   1.41553,
   1.852385,
   2.094898,
   3.736828};
   TGraphErrors *gre = new TGraphErrors(5,_fx1148,_fy1148,_fex1148,_fey1148);
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
   
   TH1F *Graph_Graph1148 = new TH1F("Graph_Graph1148","",100,11.40102,37.74254);
   Graph_Graph1148->SetMinimum(24.32445);
   Graph_Graph1148->SetMaximum(59.82011);
   Graph_Graph1148->SetDirectory(0);
   Graph_Graph1148->SetStats(0);
   Graph_Graph1148->SetLineStyle(0);
   Graph_Graph1148->SetMarkerStyle(20);
   Graph_Graph1148->GetXaxis()->SetLabelFont(42);
   Graph_Graph1148->GetXaxis()->SetLabelOffset(0.007);
   Graph_Graph1148->GetXaxis()->SetLabelSize(0.05);
   Graph_Graph1148->GetXaxis()->SetTitleSize(0.06);
   Graph_Graph1148->GetXaxis()->SetTitleOffset(0.9);
   Graph_Graph1148->GetXaxis()->SetTitleFont(42);
   Graph_Graph1148->GetYaxis()->SetLabelFont(42);
   Graph_Graph1148->GetYaxis()->SetLabelOffset(0.007);
   Graph_Graph1148->GetYaxis()->SetLabelSize(0.05);
   Graph_Graph1148->GetYaxis()->SetTitleSize(0.06);
   Graph_Graph1148->GetYaxis()->SetTitleOffset(1.05);
   Graph_Graph1148->GetYaxis()->SetTitleFont(42);
   Graph_Graph1148->GetZaxis()->SetLabelFont(42);
   Graph_Graph1148->GetZaxis()->SetLabelOffset(0.007);
   Graph_Graph1148->GetZaxis()->SetLabelSize(0.05);
   Graph_Graph1148->GetZaxis()->SetTitleSize(0.06);
   Graph_Graph1148->GetZaxis()->SetTitleFont(42);
   gre->SetHistogram(Graph_Graph1148);
   
   
   TF1 *fitfun_BAR01149 = new TF1("fitfun_BAR0","[0]*1./pow(x,[1])",0,100, TF1::EAddToList::kNo);
   fitfun_BAR01149->SetFillColor(19);
   fitfun_BAR01149->SetFillStyle(0);
   fitfun_BAR01149->SetMarkerStyle(20);

   ci = TColor::GetColor("#ff9933");
   fitfun_BAR01149->SetLineColor(ci);
   fitfun_BAR01149->SetLineWidth(1);
   fitfun_BAR01149->SetLineStyle(2);
   fitfun_BAR01149->SetChisquare(1.772767);
   fitfun_BAR01149->SetNDF(3);
   fitfun_BAR01149->GetXaxis()->SetLabelFont(42);
   fitfun_BAR01149->GetXaxis()->SetLabelOffset(0.007);
   fitfun_BAR01149->GetXaxis()->SetLabelSize(0.05);
   fitfun_BAR01149->GetXaxis()->SetTitleSize(0.06);
   fitfun_BAR01149->GetXaxis()->SetTitleOffset(0.9);
   fitfun_BAR01149->GetXaxis()->SetTitleFont(42);
   fitfun_BAR01149->GetYaxis()->SetLabelFont(42);
   fitfun_BAR01149->GetYaxis()->SetLabelOffset(0.007);
   fitfun_BAR01149->GetYaxis()->SetLabelSize(0.05);
   fitfun_BAR01149->GetYaxis()->SetTitleSize(0.06);
   fitfun_BAR01149->GetYaxis()->SetTitleOffset(1.05);
   fitfun_BAR01149->GetYaxis()->SetTitleFont(42);
   fitfun_BAR01149->SetParameter(0,524.9245);
   fitfun_BAR01149->SetParError(0,232.1467);
   fitfun_BAR01149->SetParLimits(0,0,0);
   fitfun_BAR01149->SetParameter(1,0.8284071);
   fitfun_BAR01149->SetParError(1,0.132011);
   fitfun_BAR01149->SetParLimits(1,0,0);
   fitfun_BAR01149->SetParent(gre);
   gre->GetListOfFunctions()->Add(fitfun_BAR01149);
   gre->Draw("p");
   
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
   TLatex *   tex = new TLatex(0.15,0.15,"#sigma_{t} #propto PDE^{-(0.83 +/- 0.13)}");
tex->SetNDC();

   ci = TColor::GetColor("#ff9933");
   tex->SetTextColor(ci);
   tex->SetTextSize(0.035);
   tex->SetLineWidth(2);
   tex->Draw();
      tex = new TLatex(0.13,0.92,"LYSO:Ce 3x3x57 mm^{3} - HPK S12572-015 3x3 mm^{2}");
tex->SetNDC();
   tex->SetTextSize(0.035);
   tex->SetLineWidth(2);
   tex->Draw();
   
   Double_t _fx1150[5] = {
   34.80492,
   31.2419,
   27.38275,
   22.35237,
   15.27564};
   Double_t _fy1150[5] = {
   28.42518,
   30.36794,
   32.95441,
   38.74025,
   49.6887};
   Double_t _fex1150[5] = {
   0.7424989,
   0.7142128,
   0.860949,
   1.182708,
   1.679489};
   Double_t _fey1150[5] = {
   0.8114015,
   0.8371492,
   0.9101066,
   0.9512217,
   1.560866};
   gre = new TGraphErrors(5,_fx1150,_fy1150,_fex1150,_fey1150);
   gre->SetName("");
   gre->SetTitle("");
   gre->SetFillStyle(1000);

   ci = TColor::GetColor("#00cc00");
   gre->SetLineColor(ci);

   ci = TColor::GetColor("#00cc00");
   gre->SetMarkerColor(ci);
   gre->SetMarkerStyle(21);
   
   TH1F *Graph_Graph1150 = new TH1F("Graph_Graph1150","",100,11.40102,37.74254);
   Graph_Graph1150->SetMinimum(25.2502);
   Graph_Graph1150->SetMaximum(53.61314);
   Graph_Graph1150->SetDirectory(0);
   Graph_Graph1150->SetStats(0);
   Graph_Graph1150->SetLineStyle(0);
   Graph_Graph1150->SetMarkerStyle(20);
   Graph_Graph1150->GetXaxis()->SetLabelFont(42);
   Graph_Graph1150->GetXaxis()->SetLabelOffset(0.007);
   Graph_Graph1150->GetXaxis()->SetLabelSize(0.05);
   Graph_Graph1150->GetXaxis()->SetTitleSize(0.06);
   Graph_Graph1150->GetXaxis()->SetTitleOffset(0.9);
   Graph_Graph1150->GetXaxis()->SetTitleFont(42);
   Graph_Graph1150->GetYaxis()->SetLabelFont(42);
   Graph_Graph1150->GetYaxis()->SetLabelOffset(0.007);
   Graph_Graph1150->GetYaxis()->SetLabelSize(0.05);
   Graph_Graph1150->GetYaxis()->SetTitleSize(0.06);
   Graph_Graph1150->GetYaxis()->SetTitleOffset(1.05);
   Graph_Graph1150->GetYaxis()->SetTitleFont(42);
   Graph_Graph1150->GetZaxis()->SetLabelFont(42);
   Graph_Graph1150->GetZaxis()->SetLabelOffset(0.007);
   Graph_Graph1150->GetZaxis()->SetLabelSize(0.05);
   Graph_Graph1150->GetZaxis()->SetTitleSize(0.06);
   Graph_Graph1150->GetZaxis()->SetTitleFont(42);
   gre->SetHistogram(Graph_Graph1150);
   
   
   TF1 *fitfun_BAR11151 = new TF1("fitfun_BAR1","[0]*1./pow(x,[1])",0,100, TF1::EAddToList::kNo);
   fitfun_BAR11151->SetFillColor(19);
   fitfun_BAR11151->SetFillStyle(0);
   fitfun_BAR11151->SetMarkerStyle(20);

   ci = TColor::GetColor("#00cc00");
   fitfun_BAR11151->SetLineColor(ci);
   fitfun_BAR11151->SetLineWidth(1);
   fitfun_BAR11151->SetLineStyle(2);
   fitfun_BAR11151->SetChisquare(0.204085);
   fitfun_BAR11151->SetNDF(3);
   fitfun_BAR11151->GetXaxis()->SetLabelFont(42);
   fitfun_BAR11151->GetXaxis()->SetLabelOffset(0.007);
   fitfun_BAR11151->GetXaxis()->SetLabelSize(0.05);
   fitfun_BAR11151->GetXaxis()->SetTitleSize(0.06);
   fitfun_BAR11151->GetXaxis()->SetTitleOffset(0.9);
   fitfun_BAR11151->GetXaxis()->SetTitleFont(42);
   fitfun_BAR11151->GetYaxis()->SetLabelFont(42);
   fitfun_BAR11151->GetYaxis()->SetLabelOffset(0.007);
   fitfun_BAR11151->GetYaxis()->SetLabelSize(0.05);
   fitfun_BAR11151->GetYaxis()->SetTitleSize(0.06);
   fitfun_BAR11151->GetYaxis()->SetTitleOffset(1.05);
   fitfun_BAR11151->GetYaxis()->SetTitleFont(42);
   fitfun_BAR11151->SetParameter(0,324.9001);
   fitfun_BAR11151->SetParError(0,92.4025);
   fitfun_BAR11151->SetParLimits(0,0,0);
   fitfun_BAR11151->SetParameter(1,0.6880108);
   fitfun_BAR11151->SetParError(1,0.08453982);
   fitfun_BAR11151->SetParLimits(1,0,0);
   fitfun_BAR11151->SetParent(gre);
   gre->GetListOfFunctions()->Add(fitfun_BAR11151);
   gre->Draw("p");
   
   leg = new TLegend(0.65,0.65,0.89,0.84,NULL,"brNDC");
   leg->SetBorderSize(0);
   leg->SetTextFont(62);
   leg->SetLineColor(1);
   leg->SetLineStyle(1);
   leg->SetLineWidth(1);
   leg->SetFillColor(0);
   leg->SetFillStyle(1001);
   entry=leg->AddEntry("","BAR0","PL");

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
      tex = new TLatex(0.15,0.2,"#sigma_{t} #propto PDE^{-(0.69 +/- 0.08)}");
tex->SetNDC();

   ci = TColor::GetColor("#00cc00");
   tex->SetTextColor(ci);
   tex->SetTextSize(0.035);
   tex->SetLineWidth(2);
   tex->Draw();
      tex = new TLatex(0.13,0.92,"LYSO:Ce 3x3x57 mm^{3} - HPK S12572-015 3x3 mm^{2}");
tex->SetNDC();
   tex->SetTextSize(0.035);
   tex->SetLineWidth(2);
   tex->Draw();
   
   Double_t _fx1152[5] = {
   34.80492,
   31.2419,
   27.38275,
   22.35237,
   15.27564};
   Double_t _fy1152[5] = {
   29.99082,
   31.2958,
   36.51118,
   42.20048,
   57.44607};
   Double_t _fex1152[5] = {
   0.7424989,
   0.7142128,
   0.860949,
   1.182708,
   1.679489};
   Double_t _fey1152[5] = {
   1.034933,
   1.201989,
   1.467274,
   1.328938,
   2.567984};
   gre = new TGraphErrors(5,_fx1152,_fy1152,_fex1152,_fey1152);
   gre->SetName("");
   gre->SetTitle("");
   gre->SetFillStyle(1000);

   ci = TColor::GetColor("#0000cc");
   gre->SetLineColor(ci);

   ci = TColor::GetColor("#0000cc");
   gre->SetMarkerColor(ci);
   gre->SetMarkerStyle(22);
   
   TH1F *Graph_Graph1152 = new TH1F("Graph_Graph1152","",100,11.40102,37.74254);
   Graph_Graph1152->SetMinimum(25.85007);
   Graph_Graph1152->SetMaximum(63.11987);
   Graph_Graph1152->SetDirectory(0);
   Graph_Graph1152->SetStats(0);
   Graph_Graph1152->SetLineStyle(0);
   Graph_Graph1152->SetMarkerStyle(20);
   Graph_Graph1152->GetXaxis()->SetLabelFont(42);
   Graph_Graph1152->GetXaxis()->SetLabelOffset(0.007);
   Graph_Graph1152->GetXaxis()->SetLabelSize(0.05);
   Graph_Graph1152->GetXaxis()->SetTitleSize(0.06);
   Graph_Graph1152->GetXaxis()->SetTitleOffset(0.9);
   Graph_Graph1152->GetXaxis()->SetTitleFont(42);
   Graph_Graph1152->GetYaxis()->SetLabelFont(42);
   Graph_Graph1152->GetYaxis()->SetLabelOffset(0.007);
   Graph_Graph1152->GetYaxis()->SetLabelSize(0.05);
   Graph_Graph1152->GetYaxis()->SetTitleSize(0.06);
   Graph_Graph1152->GetYaxis()->SetTitleOffset(1.05);
   Graph_Graph1152->GetYaxis()->SetTitleFont(42);
   Graph_Graph1152->GetZaxis()->SetLabelFont(42);
   Graph_Graph1152->GetZaxis()->SetLabelOffset(0.007);
   Graph_Graph1152->GetZaxis()->SetLabelSize(0.05);
   Graph_Graph1152->GetZaxis()->SetTitleSize(0.06);
   Graph_Graph1152->GetZaxis()->SetTitleFont(42);
   gre->SetHistogram(Graph_Graph1152);
   
   
   TF1 *fitfun_BAR21153 = new TF1("fitfun_BAR2","[0]*1./pow(x,[1])",0,100, TF1::EAddToList::kNo);
   fitfun_BAR21153->SetFillColor(19);
   fitfun_BAR21153->SetFillStyle(0);
   fitfun_BAR21153->SetMarkerStyle(20);

   ci = TColor::GetColor("#0000cc");
   fitfun_BAR21153->SetLineColor(ci);
   fitfun_BAR21153->SetLineWidth(1);
   fitfun_BAR21153->SetLineStyle(2);
   fitfun_BAR21153->SetChisquare(0.785075);
   fitfun_BAR21153->SetNDF(3);
   fitfun_BAR21153->GetXaxis()->SetLabelFont(42);
   fitfun_BAR21153->GetXaxis()->SetLabelOffset(0.007);
   fitfun_BAR21153->GetXaxis()->SetLabelSize(0.05);
   fitfun_BAR21153->GetXaxis()->SetTitleSize(0.06);
   fitfun_BAR21153->GetXaxis()->SetTitleOffset(0.9);
   fitfun_BAR21153->GetXaxis()->SetTitleFont(42);
   fitfun_BAR21153->GetYaxis()->SetLabelFont(42);
   fitfun_BAR21153->GetYaxis()->SetLabelOffset(0.007);
   fitfun_BAR21153->GetYaxis()->SetLabelSize(0.05);
   fitfun_BAR21153->GetYaxis()->SetTitleSize(0.06);
   fitfun_BAR21153->GetYaxis()->SetTitleOffset(1.05);
   fitfun_BAR21153->GetYaxis()->SetTitleFont(42);
   fitfun_BAR21153->SetParameter(0,519.1081);
   fitfun_BAR21153->SetParError(0,182.1026);
   fitfun_BAR21153->SetParLimits(0,0,0);
   fitfun_BAR21153->SetParameter(1,0.8073382);
   fitfun_BAR21153->SetParError(1,0.1042698);
   fitfun_BAR21153->SetParLimits(1,0,0);
   fitfun_BAR21153->SetParent(gre);
   gre->GetListOfFunctions()->Add(fitfun_BAR21153);
   gre->Draw("p");
   
   leg = new TLegend(0.65,0.65,0.89,0.84,NULL,"brNDC");
   leg->SetBorderSize(0);
   leg->SetTextFont(62);
   leg->SetLineColor(1);
   leg->SetLineStyle(1);
   leg->SetLineWidth(1);
   leg->SetFillColor(0);
   leg->SetFillStyle(1001);
   entry=leg->AddEntry("","BAR0","PL");

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
      tex = new TLatex(0.15,0.25,"#sigma_{t} #propto PDE^{-(0.81 +/- 0.10)}");
tex->SetNDC();

   ci = TColor::GetColor("#0000cc");
   tex->SetTextColor(ci);
   tex->SetTextSize(0.035);
   tex->SetLineWidth(2);
   tex->Draw();
      tex = new TLatex(0.13,0.92,"LYSO:Ce 3x3x57 mm^{3} - HPK S12572-015 3x3 mm^{2}");
tex->SetNDC();
   tex->SetTextSize(0.035);
   tex->SetLineWidth(2);
   tex->Draw();
   scan_PDE_LEthrOpt_xyangle90_HPK->Modified();
   scan_PDE_LEthrOpt_xyangle90_HPK->cd();
   scan_PDE_LEthrOpt_xyangle90_HPK->SetSelected(scan_PDE_LEthrOpt_xyangle90_HPK);
}
