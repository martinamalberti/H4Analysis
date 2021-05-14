void scan_PDE_LEthrOpt_4mm_FBK()
{
//=========Macro generated from canvas: scan_PDE_LEthrOpt_4mm_FBK/scan_PDE_LEthrOpt_4mm_FBK
//=========  (Sat Feb 20 10:53:51 2021) by ROOT version 6.14/04
   TCanvas *scan_PDE_LEthrOpt_4mm_FBK = new TCanvas("scan_PDE_LEthrOpt_4mm_FBK", "scan_PDE_LEthrOpt_4mm_FBK",0,0,600,600);
   gStyle->SetOptStat(0);
   gStyle->SetOptTitle(0);
   scan_PDE_LEthrOpt_4mm_FBK->SetHighLightColor(2);
   scan_PDE_LEthrOpt_4mm_FBK->Range(-1.582278,12.30769,49.05063,76.41026);
   scan_PDE_LEthrOpt_4mm_FBK->SetFillColor(0);
   scan_PDE_LEthrOpt_4mm_FBK->SetBorderMode(0);
   scan_PDE_LEthrOpt_4mm_FBK->SetBorderSize(2);
   scan_PDE_LEthrOpt_4mm_FBK->SetTickx(1);
   scan_PDE_LEthrOpt_4mm_FBK->SetTicky(1);
   scan_PDE_LEthrOpt_4mm_FBK->SetLeftMargin(0.13);
   scan_PDE_LEthrOpt_4mm_FBK->SetRightMargin(0.08);
   scan_PDE_LEthrOpt_4mm_FBK->SetBottomMargin(0.12);
   scan_PDE_LEthrOpt_4mm_FBK->SetFrameFillStyle(0);
   scan_PDE_LEthrOpt_4mm_FBK->SetFrameBorderMode(0);
   scan_PDE_LEthrOpt_4mm_FBK->SetFrameFillStyle(0);
   scan_PDE_LEthrOpt_4mm_FBK->SetFrameBorderMode(0);
   
   TH2F *__6 = new TH2F("__6","",10,5,45,100,20,70);
   __6->SetStats(0);
   __6->SetLineStyle(0);
   __6->SetMarkerStyle(20);
   __6->GetXaxis()->SetTitle("PDE (%)");
   __6->GetXaxis()->SetLabelFont(42);
   __6->GetXaxis()->SetLabelOffset(0.007);
   __6->GetXaxis()->SetLabelSize(0.05);
   __6->GetXaxis()->SetTitleSize(0.06);
   __6->GetXaxis()->SetTitleOffset(0.9);
   __6->GetXaxis()->SetTitleFont(42);
   __6->GetYaxis()->SetTitle("time resolution (ps)");
   __6->GetYaxis()->SetLabelFont(42);
   __6->GetYaxis()->SetLabelOffset(0.007);
   __6->GetYaxis()->SetLabelSize(0.05);
   __6->GetYaxis()->SetTitleSize(0.06);
   __6->GetYaxis()->SetTitleOffset(1.05);
   __6->GetYaxis()->SetTitleFont(42);
   __6->GetZaxis()->SetLabelFont(42);
   __6->GetZaxis()->SetLabelOffset(0.007);
   __6->GetZaxis()->SetLabelSize(0.05);
   __6->GetZaxis()->SetTitleSize(0.06);
   __6->GetZaxis()->SetTitleFont(42);
   __6->Draw("");
   
   Double_t _fx1050[5] = {
   35.25482,
   32.99367,
   29.3419,
   24.29952,
   17.86652};
   Double_t _fy1050[5] = {
   26.35675,
   27.70481,
   28.8834,
   31.937,
   38.27177};
   Double_t _fex1050[5] = {
   0.313169,
   0.5912922,
   0.8694153,
   1.147538,
   1.425662};
   Double_t _fey1050[5] = {
   0.6396887,
   0.7084773,
   0.7141467,
   0.6963194,
   1.147337};
   TGraphErrors *gre = new TGraphErrors(5,_fx1050,_fy1050,_fex1050,_fey1050);
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
   
   TH1F *Graph_Graph1050 = new TH1F("Graph_Graph1050","",100,14.52814,37.48071);
   Graph_Graph1050->SetMinimum(24.34686);
   Graph_Graph1050->SetMaximum(40.78931);
   Graph_Graph1050->SetDirectory(0);
   Graph_Graph1050->SetStats(0);
   Graph_Graph1050->SetLineStyle(0);
   Graph_Graph1050->SetMarkerStyle(20);
   Graph_Graph1050->GetXaxis()->SetLabelFont(42);
   Graph_Graph1050->GetXaxis()->SetLabelOffset(0.007);
   Graph_Graph1050->GetXaxis()->SetLabelSize(0.05);
   Graph_Graph1050->GetXaxis()->SetTitleSize(0.06);
   Graph_Graph1050->GetXaxis()->SetTitleOffset(0.9);
   Graph_Graph1050->GetXaxis()->SetTitleFont(42);
   Graph_Graph1050->GetYaxis()->SetLabelFont(42);
   Graph_Graph1050->GetYaxis()->SetLabelOffset(0.007);
   Graph_Graph1050->GetYaxis()->SetLabelSize(0.05);
   Graph_Graph1050->GetYaxis()->SetTitleSize(0.06);
   Graph_Graph1050->GetYaxis()->SetTitleOffset(1.05);
   Graph_Graph1050->GetYaxis()->SetTitleFont(42);
   Graph_Graph1050->GetZaxis()->SetLabelFont(42);
   Graph_Graph1050->GetZaxis()->SetLabelOffset(0.007);
   Graph_Graph1050->GetZaxis()->SetLabelSize(0.05);
   Graph_Graph1050->GetZaxis()->SetTitleSize(0.06);
   Graph_Graph1050->GetZaxis()->SetTitleFont(42);
   gre->SetHistogram(Graph_Graph1050);
   
   
   TF1 *fitfun_BAR01051 = new TF1("fitfun_BAR0","[0]*1./pow(x,[1])",0,100, TF1::EAddToList::kNo);
   fitfun_BAR01051->SetFillColor(19);
   fitfun_BAR01051->SetFillStyle(0);
   fitfun_BAR01051->SetMarkerStyle(20);

   ci = TColor::GetColor("#0000cc");
   fitfun_BAR01051->SetLineColor(ci);
   fitfun_BAR01051->SetLineWidth(1);
   fitfun_BAR01051->SetLineStyle(2);
   fitfun_BAR01051->SetChisquare(0.3819874);
   fitfun_BAR01051->SetNDF(3);
   fitfun_BAR01051->GetXaxis()->SetLabelFont(42);
   fitfun_BAR01051->GetXaxis()->SetLabelOffset(0.007);
   fitfun_BAR01051->GetXaxis()->SetLabelSize(0.05);
   fitfun_BAR01051->GetXaxis()->SetTitleSize(0.06);
   fitfun_BAR01051->GetXaxis()->SetTitleOffset(0.9);
   fitfun_BAR01051->GetXaxis()->SetTitleFont(42);
   fitfun_BAR01051->GetYaxis()->SetLabelFont(42);
   fitfun_BAR01051->GetYaxis()->SetLabelOffset(0.007);
   fitfun_BAR01051->GetYaxis()->SetLabelSize(0.05);
   fitfun_BAR01051->GetYaxis()->SetTitleSize(0.06);
   fitfun_BAR01051->GetYaxis()->SetTitleOffset(1.05);
   fitfun_BAR01051->GetYaxis()->SetTitleFont(42);
   fitfun_BAR01051->SetParameter(0,173.4386);
   fitfun_BAR01051->SetParError(0,41.58275);
   fitfun_BAR01051->SetParLimits(0,0,0);
   fitfun_BAR01051->SetParameter(1,0.528101);
   fitfun_BAR01051->SetParError(1,0.07043799);
   fitfun_BAR01051->SetParLimits(1,0,0);
   fitfun_BAR01051->SetParent(gre);
   gre->GetListOfFunctions()->Add(fitfun_BAR01051);
   gre->Draw("p");
   TLatex *   tex = new TLatex(0.15,0.15,"#sigma_{t} #propto PDE^{-(0.53 +/- 0.07)}");
tex->SetNDC();

   ci = TColor::GetColor("#0000cc");
   tex->SetTextColor(ci);
   tex->SetTextSize(0.035);
   tex->SetLineWidth(2);
   tex->Draw();
      tex = new TLatex(0.13,0.92,"LYSO:Ce 3x4x57 mm^{3} - FBK NUV-HD-TE 5x5 mm^{2}");
tex->SetNDC();
   tex->SetTextSize(0.035);
   tex->SetLineWidth(2);
   tex->Draw();
   scan_PDE_LEthrOpt_4mm_FBK->Modified();
   scan_PDE_LEthrOpt_4mm_FBK->cd();
   scan_PDE_LEthrOpt_4mm_FBK->SetSelected(scan_PDE_LEthrOpt_4mm_FBK);
}
