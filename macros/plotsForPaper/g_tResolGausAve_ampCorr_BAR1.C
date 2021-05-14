void g_tResolGausAve_ampCorr_BAR1()
{
//=========Macro generated from canvas: g_tResolGausAve_ampCorr_BAR1/g_tResolGausAve_ampCorr_BAR1
//=========  (Mon Jun 29 12:32:35 2020) by ROOT version 6.14/04
   TCanvas *g_tResolGausAve_ampCorr_BAR1 = new TCanvas("g_tResolGausAve_ampCorr_BAR1", "g_tResolGausAve_ampCorr_BAR1",0,45,600,600);
   gStyle->SetOptStat(0);
   gStyle->SetOptTitle(0);
   g_tResolGausAve_ampCorr_BAR1->SetHighLightColor(2);
   g_tResolGausAve_ampCorr_BAR1->Range(-8.226913,7.307692,20.77673,71.41026);
   g_tResolGausAve_ampCorr_BAR1->SetFillColor(0);
   g_tResolGausAve_ampCorr_BAR1->SetBorderMode(0);
   g_tResolGausAve_ampCorr_BAR1->SetBorderSize(2);
   g_tResolGausAve_ampCorr_BAR1->SetTickx(1);
   g_tResolGausAve_ampCorr_BAR1->SetTicky(1);
   g_tResolGausAve_ampCorr_BAR1->SetLeftMargin(0.13);
   g_tResolGausAve_ampCorr_BAR1->SetRightMargin(0.08);
   g_tResolGausAve_ampCorr_BAR1->SetBottomMargin(0.12);
   g_tResolGausAve_ampCorr_BAR1->SetFrameFillStyle(0);
   g_tResolGausAve_ampCorr_BAR1->SetFrameBorderMode(0);
   g_tResolGausAve_ampCorr_BAR1->SetFrameFillStyle(0);
   g_tResolGausAve_ampCorr_BAR1->SetFrameBorderMode(0);
   
   TH2F *hdummy__1 = new TH2F("hdummy__1","",100,-4.456439,18.45644,100,15,65);
   hdummy__1->SetStats(0);
   hdummy__1->SetLineStyle(0);
   hdummy__1->SetMarkerStyle(20);
   hdummy__1->GetXaxis()->SetTitle("x (mm)");
   hdummy__1->GetXaxis()->SetLabelFont(42);
   hdummy__1->GetXaxis()->SetLabelOffset(0.007);
   hdummy__1->GetXaxis()->SetLabelSize(0.05);
   hdummy__1->GetXaxis()->SetTitleSize(0.06);
   hdummy__1->GetXaxis()->SetTitleOffset(0.9);
   hdummy__1->GetXaxis()->SetTitleFont(42);
   hdummy__1->GetYaxis()->SetTitle("time resolution (ps)");
   hdummy__1->GetYaxis()->SetLabelFont(42);
   hdummy__1->GetYaxis()->SetLabelOffset(0.007);
   hdummy__1->GetYaxis()->SetLabelSize(0.05);
   hdummy__1->GetYaxis()->SetTitleSize(0.06);
   hdummy__1->GetYaxis()->SetTitleOffset(1.05);
   hdummy__1->GetYaxis()->SetTitleFont(42);
   hdummy__1->GetZaxis()->SetLabelFont(42);
   hdummy__1->GetZaxis()->SetLabelOffset(0.007);
   hdummy__1->GetZaxis()->SetLabelSize(0.05);
   hdummy__1->GetZaxis()->SetTitleSize(0.06);
   hdummy__1->GetZaxis()->SetTitleFont(42);
   hdummy__1->Draw("");
   
   Double_t g_tResolGausAve_ampCorr_BAR1_fx1001[8] = {
   0,
   2,
   4,
   6,
   8,
   10,
   12,
   14};
   Double_t g_tResolGausAve_ampCorr_BAR1_fy1001[8] = {
   31.40179,
   29.62538,
   31.01885,
   29.41231,
   29.53199,
   30.78341,
   31.31791,
   30.02205};
   Double_t g_tResolGausAve_ampCorr_BAR1_fex1001[8] = {
   1,
   1,
   1,
   1,
   1,
   1,
   1,
   1};
   Double_t g_tResolGausAve_ampCorr_BAR1_fey1001[8] = {
   1.362599,
   1.561921,
   1.410516,
   1.334252,
   1.517953,
   1.478852,
   1.456598,
   1.373813};
   TGraphErrors *gre = new TGraphErrors(8,g_tResolGausAve_ampCorr_BAR1_fx1001,g_tResolGausAve_ampCorr_BAR1_fy1001,g_tResolGausAve_ampCorr_BAR1_fex1001,g_tResolGausAve_ampCorr_BAR1_fey1001);
   gre->SetName("g_tResolGausAve_ampCorr_BAR1");
   gre->SetTitle("");
   gre->SetFillStyle(1000);
   gre->SetMarkerStyle(20);
   
   TH1F *Graph_g_tResolGausAve_ampCorr_BAR11001 = new TH1F("Graph_g_tResolGausAve_ampCorr_BAR11001","",100,-2.6,16.6);
   Graph_g_tResolGausAve_ampCorr_BAR11001->SetMinimum(27.53799);
   Graph_g_tResolGausAve_ampCorr_BAR11001->SetMaximum(33.25055);
   Graph_g_tResolGausAve_ampCorr_BAR11001->SetDirectory(0);
   Graph_g_tResolGausAve_ampCorr_BAR11001->SetStats(0);
   Graph_g_tResolGausAve_ampCorr_BAR11001->SetLineStyle(0);
   Graph_g_tResolGausAve_ampCorr_BAR11001->SetMarkerStyle(20);
   Graph_g_tResolGausAve_ampCorr_BAR11001->GetXaxis()->SetLabelFont(42);
   Graph_g_tResolGausAve_ampCorr_BAR11001->GetXaxis()->SetLabelSize(0.03);
   Graph_g_tResolGausAve_ampCorr_BAR11001->GetXaxis()->SetTitleSize(0.05);
   Graph_g_tResolGausAve_ampCorr_BAR11001->GetXaxis()->SetTitleOffset(0.9);
   Graph_g_tResolGausAve_ampCorr_BAR11001->GetXaxis()->SetTitleFont(42);
   Graph_g_tResolGausAve_ampCorr_BAR11001->GetYaxis()->SetLabelFont(42);
   Graph_g_tResolGausAve_ampCorr_BAR11001->GetYaxis()->SetLabelSize(0.03);
   Graph_g_tResolGausAve_ampCorr_BAR11001->GetYaxis()->SetTitleSize(0.05);
   Graph_g_tResolGausAve_ampCorr_BAR11001->GetYaxis()->SetTitleOffset(1.05);
   Graph_g_tResolGausAve_ampCorr_BAR11001->GetYaxis()->SetTitleFont(42);
   Graph_g_tResolGausAve_ampCorr_BAR11001->GetZaxis()->SetLabelFont(42);
   Graph_g_tResolGausAve_ampCorr_BAR11001->GetZaxis()->SetLabelSize(0.03);
   Graph_g_tResolGausAve_ampCorr_BAR11001->GetZaxis()->SetTitleSize(0.05);
   Graph_g_tResolGausAve_ampCorr_BAR11001->GetZaxis()->SetTitleFont(42);
   gre->SetHistogram(Graph_g_tResolGausAve_ampCorr_BAR11001);
   
   
   TF1 *fpol0_tAve_ampCorr_BAR11002 = new TF1("fpol0_tAve_ampCorr_BAR1","pol0",-50,50, TF1::EAddToList::kNo);
   fpol0_tAve_ampCorr_BAR11002->SetFillColor(19);
   fpol0_tAve_ampCorr_BAR11002->SetFillStyle(0);
   fpol0_tAve_ampCorr_BAR11002->SetMarkerStyle(20);
   fpol0_tAve_ampCorr_BAR11002->SetLineWidth(2);
   fpol0_tAve_ampCorr_BAR11002->SetLineStyle(2);
   fpol0_tAve_ampCorr_BAR11002->SetChisquare(2.393948);
   fpol0_tAve_ampCorr_BAR11002->SetNDF(7);
   fpol0_tAve_ampCorr_BAR11002->GetXaxis()->SetLabelFont(42);
   fpol0_tAve_ampCorr_BAR11002->GetXaxis()->SetLabelOffset(0.007);
   fpol0_tAve_ampCorr_BAR11002->GetXaxis()->SetLabelSize(0.05);
   fpol0_tAve_ampCorr_BAR11002->GetXaxis()->SetTitleSize(0.06);
   fpol0_tAve_ampCorr_BAR11002->GetXaxis()->SetTitleOffset(0.9);
   fpol0_tAve_ampCorr_BAR11002->GetXaxis()->SetTitleFont(42);
   fpol0_tAve_ampCorr_BAR11002->GetYaxis()->SetLabelFont(42);
   fpol0_tAve_ampCorr_BAR11002->GetYaxis()->SetLabelOffset(0.007);
   fpol0_tAve_ampCorr_BAR11002->GetYaxis()->SetLabelSize(0.05);
   fpol0_tAve_ampCorr_BAR11002->GetYaxis()->SetTitleSize(0.06);
   fpol0_tAve_ampCorr_BAR11002->GetYaxis()->SetTitleOffset(1.05);
   fpol0_tAve_ampCorr_BAR11002->GetYaxis()->SetTitleFont(42);
   fpol0_tAve_ampCorr_BAR11002->SetParameter(0,30.40237);
   fpol0_tAve_ampCorr_BAR11002->SetParError(0,0.5060288);
   fpol0_tAve_ampCorr_BAR11002->SetParLimits(0,0,0);
   fpol0_tAve_ampCorr_BAR11002->SetParent(gre);
   gre->GetListOfFunctions()->Add(fpol0_tAve_ampCorr_BAR11002);
   gre->Draw("p");
   
   Double_t g_tResolGausL_ampCorr_BAR1_fx1003[8] = {
   0,
   2,
   4,
   6,
   8,
   10,
   12,
   14};
   Double_t g_tResolGausL_ampCorr_BAR1_fy1003[8] = {
   41.95195,
   42.65475,
   43.57061,
   44.38858,
   43.56159,
   45.97959,
   44.58639,
   41.88665};
   Double_t g_tResolGausL_ampCorr_BAR1_fex1003[8] = {
   1,
   1,
   1,
   1,
   1,
   1,
   1,
   1};
   Double_t g_tResolGausL_ampCorr_BAR1_fey1003[8] = {
   1.711171,
   1.879964,
   2.186368,
   2.120799,
   1.895059,
   2.149315,
   2.726689,
   1.950617};
   gre = new TGraphErrors(8,g_tResolGausL_ampCorr_BAR1_fx1003,g_tResolGausL_ampCorr_BAR1_fy1003,g_tResolGausL_ampCorr_BAR1_fex1003,g_tResolGausL_ampCorr_BAR1_fey1003);
   gre->SetName("g_tResolGausL_ampCorr_BAR1");
   gre->SetTitle("");
   gre->SetFillStyle(1000);

   Int_t ci;      // for color index setting
   TColor *color; // for color definition with alpha
   ci = TColor::GetColor("#0000ff");
   gre->SetLineColor(ci);

   ci = TColor::GetColor("#0000ff");
   gre->SetMarkerColor(ci);
   gre->SetMarkerStyle(20);
   
   TH1F *Graph_g_tResolGausL_ampCorr_BAR11003 = new TH1F("Graph_g_tResolGausL_ampCorr_BAR11003","",100,-2.6,16.6);
   Graph_g_tResolGausL_ampCorr_BAR11003->SetMinimum(39.11675);
   Graph_g_tResolGausL_ampCorr_BAR11003->SetMaximum(48.94819);
   Graph_g_tResolGausL_ampCorr_BAR11003->SetDirectory(0);
   Graph_g_tResolGausL_ampCorr_BAR11003->SetStats(0);
   Graph_g_tResolGausL_ampCorr_BAR11003->SetLineStyle(0);
   Graph_g_tResolGausL_ampCorr_BAR11003->SetMarkerStyle(20);
   Graph_g_tResolGausL_ampCorr_BAR11003->GetXaxis()->SetLabelFont(42);
   Graph_g_tResolGausL_ampCorr_BAR11003->GetXaxis()->SetLabelSize(0.03);
   Graph_g_tResolGausL_ampCorr_BAR11003->GetXaxis()->SetTitleSize(0.05);
   Graph_g_tResolGausL_ampCorr_BAR11003->GetXaxis()->SetTitleOffset(0.9);
   Graph_g_tResolGausL_ampCorr_BAR11003->GetXaxis()->SetTitleFont(42);
   Graph_g_tResolGausL_ampCorr_BAR11003->GetYaxis()->SetLabelFont(42);
   Graph_g_tResolGausL_ampCorr_BAR11003->GetYaxis()->SetLabelSize(0.03);
   Graph_g_tResolGausL_ampCorr_BAR11003->GetYaxis()->SetTitleSize(0.05);
   Graph_g_tResolGausL_ampCorr_BAR11003->GetYaxis()->SetTitleOffset(1.05);
   Graph_g_tResolGausL_ampCorr_BAR11003->GetYaxis()->SetTitleFont(42);
   Graph_g_tResolGausL_ampCorr_BAR11003->GetZaxis()->SetLabelFont(42);
   Graph_g_tResolGausL_ampCorr_BAR11003->GetZaxis()->SetLabelSize(0.03);
   Graph_g_tResolGausL_ampCorr_BAR11003->GetZaxis()->SetTitleSize(0.05);
   Graph_g_tResolGausL_ampCorr_BAR11003->GetZaxis()->SetTitleFont(42);
   gre->SetHistogram(Graph_g_tResolGausL_ampCorr_BAR11003);
   
   
   TF1 *fpol0_tL_ampCorr_BAR11004 = new TF1("fpol0_tL_ampCorr_BAR1","pol0",-50,50, TF1::EAddToList::kNo);
   fpol0_tL_ampCorr_BAR11004->SetFillColor(19);
   fpol0_tL_ampCorr_BAR11004->SetFillStyle(0);
   fpol0_tL_ampCorr_BAR11004->SetMarkerStyle(20);
   fpol0_tL_ampCorr_BAR11004->SetLineColor(4);
   fpol0_tL_ampCorr_BAR11004->SetLineWidth(2);
   fpol0_tL_ampCorr_BAR11004->SetLineStyle(2);
   fpol0_tL_ampCorr_BAR11004->SetChisquare(3.331905);
   fpol0_tL_ampCorr_BAR11004->SetNDF(7);
   fpol0_tL_ampCorr_BAR11004->GetXaxis()->SetLabelFont(42);
   fpol0_tL_ampCorr_BAR11004->GetXaxis()->SetLabelOffset(0.007);
   fpol0_tL_ampCorr_BAR11004->GetXaxis()->SetLabelSize(0.05);
   fpol0_tL_ampCorr_BAR11004->GetXaxis()->SetTitleSize(0.06);
   fpol0_tL_ampCorr_BAR11004->GetXaxis()->SetTitleOffset(0.9);
   fpol0_tL_ampCorr_BAR11004->GetXaxis()->SetTitleFont(42);
   fpol0_tL_ampCorr_BAR11004->GetYaxis()->SetLabelFont(42);
   fpol0_tL_ampCorr_BAR11004->GetYaxis()->SetLabelOffset(0.007);
   fpol0_tL_ampCorr_BAR11004->GetYaxis()->SetLabelSize(0.05);
   fpol0_tL_ampCorr_BAR11004->GetYaxis()->SetTitleSize(0.06);
   fpol0_tL_ampCorr_BAR11004->GetYaxis()->SetTitleOffset(1.05);
   fpol0_tL_ampCorr_BAR11004->GetYaxis()->SetTitleFont(42);
   fpol0_tL_ampCorr_BAR11004->SetParameter(0,43.35654);
   fpol0_tL_ampCorr_BAR11004->SetParError(0,0.7164391);
   fpol0_tL_ampCorr_BAR11004->SetParLimits(0,0,0);
   fpol0_tL_ampCorr_BAR11004->SetParent(gre);
   gre->GetListOfFunctions()->Add(fpol0_tL_ampCorr_BAR11004);
   gre->Draw("p");
   
   Double_t g_tResolGausR_ampCorr_BAR1_fx1005[8] = {
   0,
   2,
   4,
   6,
   8,
   10,
   12,
   14};
   Double_t g_tResolGausR_ampCorr_BAR1_fy1005[8] = {
   42.85238,
   38.66692,
   41.19371,
   41.67011,
   42.67771,
   48.78451,
   41.80668,
   43.2063};
   Double_t g_tResolGausR_ampCorr_BAR1_fex1005[8] = {
   1,
   1,
   1,
   1,
   1,
   1,
   1,
   1};
   Double_t g_tResolGausR_ampCorr_BAR1_fey1005[8] = {
   1.748872,
   1.827941,
   1.890332,
   1.833873,
   2.004577,
   2.282149,
   2.22519,
   2.065701};
   gre = new TGraphErrors(8,g_tResolGausR_ampCorr_BAR1_fx1005,g_tResolGausR_ampCorr_BAR1_fy1005,g_tResolGausR_ampCorr_BAR1_fex1005,g_tResolGausR_ampCorr_BAR1_fey1005);
   gre->SetName("g_tResolGausR_ampCorr_BAR1");
   gre->SetTitle("");
   gre->SetFillStyle(1000);

   ci = TColor::GetColor("#ff0000");
   gre->SetLineColor(ci);

   ci = TColor::GetColor("#ff0000");
   gre->SetMarkerColor(ci);
   gre->SetMarkerStyle(20);
   
   TH1F *Graph_g_tResolGausR_ampCorr_BAR11005 = new TH1F("Graph_g_tResolGausR_ampCorr_BAR11005","",100,-2.6,16.6);
   Graph_g_tResolGausR_ampCorr_BAR11005->SetMinimum(35.41621);
   Graph_g_tResolGausR_ampCorr_BAR11005->SetMaximum(52.48942);
   Graph_g_tResolGausR_ampCorr_BAR11005->SetDirectory(0);
   Graph_g_tResolGausR_ampCorr_BAR11005->SetStats(0);
   Graph_g_tResolGausR_ampCorr_BAR11005->SetLineStyle(0);
   Graph_g_tResolGausR_ampCorr_BAR11005->SetMarkerStyle(20);
   Graph_g_tResolGausR_ampCorr_BAR11005->GetXaxis()->SetLabelFont(42);
   Graph_g_tResolGausR_ampCorr_BAR11005->GetXaxis()->SetLabelSize(0.03);
   Graph_g_tResolGausR_ampCorr_BAR11005->GetXaxis()->SetTitleSize(0.05);
   Graph_g_tResolGausR_ampCorr_BAR11005->GetXaxis()->SetTitleOffset(0.9);
   Graph_g_tResolGausR_ampCorr_BAR11005->GetXaxis()->SetTitleFont(42);
   Graph_g_tResolGausR_ampCorr_BAR11005->GetYaxis()->SetLabelFont(42);
   Graph_g_tResolGausR_ampCorr_BAR11005->GetYaxis()->SetLabelSize(0.03);
   Graph_g_tResolGausR_ampCorr_BAR11005->GetYaxis()->SetTitleSize(0.05);
   Graph_g_tResolGausR_ampCorr_BAR11005->GetYaxis()->SetTitleOffset(1.05);
   Graph_g_tResolGausR_ampCorr_BAR11005->GetYaxis()->SetTitleFont(42);
   Graph_g_tResolGausR_ampCorr_BAR11005->GetZaxis()->SetLabelFont(42);
   Graph_g_tResolGausR_ampCorr_BAR11005->GetZaxis()->SetLabelSize(0.03);
   Graph_g_tResolGausR_ampCorr_BAR11005->GetZaxis()->SetTitleSize(0.05);
   Graph_g_tResolGausR_ampCorr_BAR11005->GetZaxis()->SetTitleFont(42);
   gre->SetHistogram(Graph_g_tResolGausR_ampCorr_BAR11005);
   
   
   TF1 *fpol0_tR_ampCorr_BAR11006 = new TF1("fpol0_tR_ampCorr_BAR1","pol0",-50,50, TF1::EAddToList::kNo);
   fpol0_tR_ampCorr_BAR11006->SetFillColor(19);
   fpol0_tR_ampCorr_BAR11006->SetFillStyle(0);
   fpol0_tR_ampCorr_BAR11006->SetMarkerStyle(20);
   fpol0_tR_ampCorr_BAR11006->SetLineColor(2);
   fpol0_tR_ampCorr_BAR11006->SetLineWidth(2);
   fpol0_tR_ampCorr_BAR11006->SetLineStyle(2);
   fpol0_tR_ampCorr_BAR11006->SetChisquare(12.86011);
   fpol0_tR_ampCorr_BAR11006->SetNDF(7);
   fpol0_tR_ampCorr_BAR11006->GetXaxis()->SetLabelFont(42);
   fpol0_tR_ampCorr_BAR11006->GetXaxis()->SetLabelOffset(0.007);
   fpol0_tR_ampCorr_BAR11006->GetXaxis()->SetLabelSize(0.05);
   fpol0_tR_ampCorr_BAR11006->GetXaxis()->SetTitleSize(0.06);
   fpol0_tR_ampCorr_BAR11006->GetXaxis()->SetTitleOffset(0.9);
   fpol0_tR_ampCorr_BAR11006->GetXaxis()->SetTitleFont(42);
   fpol0_tR_ampCorr_BAR11006->GetYaxis()->SetLabelFont(42);
   fpol0_tR_ampCorr_BAR11006->GetYaxis()->SetLabelOffset(0.007);
   fpol0_tR_ampCorr_BAR11006->GetYaxis()->SetLabelSize(0.05);
   fpol0_tR_ampCorr_BAR11006->GetYaxis()->SetTitleSize(0.06);
   fpol0_tR_ampCorr_BAR11006->GetYaxis()->SetTitleOffset(1.05);
   fpol0_tR_ampCorr_BAR11006->GetYaxis()->SetTitleFont(42);
   fpol0_tR_ampCorr_BAR11006->SetParameter(0,42.32318);
   fpol0_tR_ampCorr_BAR11006->SetParError(0,0.6932753);
   fpol0_tR_ampCorr_BAR11006->SetParLimits(0,0,0);
   fpol0_tR_ampCorr_BAR11006->SetParent(gre);
   gre->GetListOfFunctions()->Add(fpol0_tR_ampCorr_BAR11006);
   gre->Draw("p");
   
   Double_t g_tResolGausDiff_ampCorr_posCorr_BAR1_fx1007[8] = {
   0,
   2,
   4,
   6,
   8,
   10,
   12,
   14};
   Double_t g_tResolGausDiff_ampCorr_posCorr_BAR1_fy1007[8] = {
   27.85987,
   27.04823,
   26.79649,
   29.20369,
   30.28784,
   32.14221,
   29.98264,
   31.53096};
   Double_t g_tResolGausDiff_ampCorr_posCorr_BAR1_fex1007[8] = {
   1,
   1,
   1,
   1,
   1,
   1,
   1,
   1};
   Double_t g_tResolGausDiff_ampCorr_posCorr_BAR1_fey1007[8] = {
   1.174628,
   1.075913,
   1.079893,
   1.263323,
   1.233929,
   1.492658,
   1.32224,
   1.433247};
   gre = new TGraphErrors(8,g_tResolGausDiff_ampCorr_posCorr_BAR1_fx1007,g_tResolGausDiff_ampCorr_posCorr_BAR1_fy1007,g_tResolGausDiff_ampCorr_posCorr_BAR1_fex1007,g_tResolGausDiff_ampCorr_posCorr_BAR1_fey1007);
   gre->SetName("g_tResolGausDiff_ampCorr_posCorr_BAR1");
   gre->SetTitle("");
   gre->SetFillStyle(1000);

   ci = TColor::GetColor("#666666");
   gre->SetLineColor(ci);

   ci = TColor::GetColor("#666666");
   gre->SetMarkerColor(ci);
   gre->SetMarkerStyle(24);
   
   TH1F *Graph_g_tResolGausDiff_ampCorr_posCorr_BAR11007 = new TH1F("Graph_g_tResolGausDiff_ampCorr_posCorr_BAR11007","",100,-2.6,16.6);
   Graph_g_tResolGausDiff_ampCorr_posCorr_BAR11007->SetMinimum(49.84954);
   Graph_g_tResolGausDiff_ampCorr_posCorr_BAR11007->SetMaximum(68.85339);
   Graph_g_tResolGausDiff_ampCorr_posCorr_BAR11007->SetDirectory(0);
   Graph_g_tResolGausDiff_ampCorr_posCorr_BAR11007->SetStats(0);
   Graph_g_tResolGausDiff_ampCorr_posCorr_BAR11007->SetLineStyle(0);
   Graph_g_tResolGausDiff_ampCorr_posCorr_BAR11007->SetMarkerStyle(20);
   Graph_g_tResolGausDiff_ampCorr_posCorr_BAR11007->GetXaxis()->SetLabelFont(42);
   Graph_g_tResolGausDiff_ampCorr_posCorr_BAR11007->GetXaxis()->SetLabelSize(0.03);
   Graph_g_tResolGausDiff_ampCorr_posCorr_BAR11007->GetXaxis()->SetTitleSize(0.05);
   Graph_g_tResolGausDiff_ampCorr_posCorr_BAR11007->GetXaxis()->SetTitleOffset(0.9);
   Graph_g_tResolGausDiff_ampCorr_posCorr_BAR11007->GetXaxis()->SetTitleFont(42);
   Graph_g_tResolGausDiff_ampCorr_posCorr_BAR11007->GetYaxis()->SetLabelFont(42);
   Graph_g_tResolGausDiff_ampCorr_posCorr_BAR11007->GetYaxis()->SetLabelSize(0.03);
   Graph_g_tResolGausDiff_ampCorr_posCorr_BAR11007->GetYaxis()->SetTitleSize(0.05);
   Graph_g_tResolGausDiff_ampCorr_posCorr_BAR11007->GetYaxis()->SetTitleOffset(1.05);
   Graph_g_tResolGausDiff_ampCorr_posCorr_BAR11007->GetYaxis()->SetTitleFont(42);
   Graph_g_tResolGausDiff_ampCorr_posCorr_BAR11007->GetZaxis()->SetLabelFont(42);
   Graph_g_tResolGausDiff_ampCorr_posCorr_BAR11007->GetZaxis()->SetLabelSize(0.03);
   Graph_g_tResolGausDiff_ampCorr_posCorr_BAR11007->GetZaxis()->SetTitleSize(0.05);
   Graph_g_tResolGausDiff_ampCorr_posCorr_BAR11007->GetZaxis()->SetTitleFont(42);
   gre->SetHistogram(Graph_g_tResolGausDiff_ampCorr_posCorr_BAR11007);
   
   
   TF1 *fpol0_tDiff_ampCorr_BAR11008 = new TF1("fpol0_tDiff_ampCorr_BAR1","[0]",-100,100, TF1::EAddToList::kNo);
   fpol0_tDiff_ampCorr_BAR11008->SetFillColor(19);
   fpol0_tDiff_ampCorr_BAR11008->SetFillStyle(0);
   fpol0_tDiff_ampCorr_BAR11008->SetMarkerStyle(20);

   ci = TColor::GetColor("#666666");
   fpol0_tDiff_ampCorr_BAR11008->SetLineColor(ci);
   fpol0_tDiff_ampCorr_BAR11008->SetLineWidth(1);
   fpol0_tDiff_ampCorr_BAR11008->SetLineStyle(2);
   fpol0_tDiff_ampCorr_BAR11008->SetChisquare(17.60439);
   fpol0_tDiff_ampCorr_BAR11008->SetNDF(7);
   fpol0_tDiff_ampCorr_BAR11008->GetXaxis()->SetLabelFont(42);
   fpol0_tDiff_ampCorr_BAR11008->GetXaxis()->SetLabelOffset(0.007);
   fpol0_tDiff_ampCorr_BAR11008->GetXaxis()->SetLabelSize(0.05);
   fpol0_tDiff_ampCorr_BAR11008->GetXaxis()->SetTitleSize(0.06);
   fpol0_tDiff_ampCorr_BAR11008->GetXaxis()->SetTitleOffset(0.9);
   fpol0_tDiff_ampCorr_BAR11008->GetXaxis()->SetTitleFont(42);
   fpol0_tDiff_ampCorr_BAR11008->GetYaxis()->SetLabelFont(42);
   fpol0_tDiff_ampCorr_BAR11008->GetYaxis()->SetLabelOffset(0.007);
   fpol0_tDiff_ampCorr_BAR11008->GetYaxis()->SetLabelSize(0.05);
   fpol0_tDiff_ampCorr_BAR11008->GetYaxis()->SetTitleSize(0.06);
   fpol0_tDiff_ampCorr_BAR11008->GetYaxis()->SetTitleOffset(1.05);
   fpol0_tDiff_ampCorr_BAR11008->GetYaxis()->SetTitleFont(42);
   fpol0_tDiff_ampCorr_BAR11008->SetParameter(0,28.95722);
   fpol0_tDiff_ampCorr_BAR11008->SetParError(0,0.4369726);
   fpol0_tDiff_ampCorr_BAR11008->SetParLimits(0,0,0);
   fpol0_tDiff_ampCorr_BAR11008->SetParent(gre);
   gre->GetListOfFunctions()->Add(fpol0_tDiff_ampCorr_BAR11008);
   gre->Draw("p");
   
   TLegend *leg = new TLegend(0.7,0.7,0.89,0.89,NULL,"brNDC");
   leg->SetBorderSize(0);
   leg->SetTextFont(62);
   leg->SetLineColor(1);
   leg->SetLineStyle(1);
   leg->SetLineWidth(1);
   leg->SetFillColor(0);
   leg->SetFillStyle(1001);
   TLegendEntry *entry=leg->AddEntry("g_tResolGausAve_ampCorr_BAR1","t_{average}","PL");
   entry->SetLineColor(1);
   entry->SetLineStyle(1);
   entry->SetLineWidth(1);
   entry->SetMarkerColor(1);
   entry->SetMarkerStyle(20);
   entry->SetMarkerSize(1);
   entry->SetTextFont(62);
   entry=leg->AddEntry("g_tResolGausDiff_ampCorr_posCorr_BAR1","t_{diff/2}","PL");

   ci = TColor::GetColor("#666666");
   entry->SetLineColor(ci);
   entry->SetLineStyle(1);
   entry->SetLineWidth(1);

   ci = TColor::GetColor("#666666");
   entry->SetMarkerColor(ci);
   entry->SetMarkerStyle(24);
   entry->SetMarkerSize(1);
   entry->SetTextFont(62);
   entry=leg->AddEntry("g_tResolGausL_ampCorr_BAR1","t_{left}","PL");

   ci = TColor::GetColor("#0000ff");
   entry->SetLineColor(ci);
   entry->SetLineStyle(1);
   entry->SetLineWidth(1);

   ci = TColor::GetColor("#0000ff");
   entry->SetMarkerColor(ci);
   entry->SetMarkerStyle(20);
   entry->SetMarkerSize(1);
   entry->SetTextFont(62);
   entry=leg->AddEntry("g_tResolGausR_ampCorr_BAR1","t_{right}","PL");

   ci = TColor::GetColor("#ff0000");
   entry->SetLineColor(ci);
   entry->SetLineStyle(1);
   entry->SetLineWidth(1);

   ci = TColor::GetColor("#ff0000");
   entry->SetMarkerColor(ci);
   entry->SetMarkerStyle(20);
   entry->SetMarkerSize(1);
   entry->SetTextFont(62);
   leg->Draw();
   TLatex *   tex = new TLatex(0.15,0.85,"t_{average}: #sigma = 30.4 #pm 0.5 ps");
tex->SetNDC();
   tex->SetTextSize(0.035);
   tex->SetLineWidth(2);
   tex->Draw();
      tex = new TLatex(0.15,0.8,"t_{diff/2} : #sigma = 29.0 #pm 0.4 ps");
tex->SetNDC();

   ci = TColor::GetColor("#666666");
   tex->SetTextColor(ci);
   tex->SetTextSize(0.035);
   tex->SetLineWidth(2);
   tex->Draw();
      tex = new TLatex(0.15,0.75,"t_{left}   : #sigma = 43.4 #pm 0.7 ps");
tex->SetNDC();

   ci = TColor::GetColor("#0000ff");
   tex->SetTextColor(ci);
   tex->SetTextSize(0.035);
   tex->SetLineWidth(2);
   tex->Draw();
      tex = new TLatex(0.15,0.7,"t_{right}  : #sigma = 42.3 #pm 0.7 ps");
tex->SetNDC();

   ci = TColor::GetColor("#ff0000");
   tex->SetTextColor(ci);
   tex->SetTextSize(0.035);
   tex->SetLineWidth(2);
   tex->Draw();
      tex = new TLatex(0.13,0.92,"LYSO:Ce 3x3x57 mm^{3} - HPK S12572-015 3x3 mm^{2}");
tex->SetNDC();
   tex->SetTextSize(0.03);
   tex->SetLineWidth(2);
   tex->Draw();
   g_tResolGausAve_ampCorr_BAR1->Modified();
   g_tResolGausAve_ampCorr_BAR1->cd();
   g_tResolGausAve_ampCorr_BAR1->SetSelected(g_tResolGausAve_ampCorr_BAR1);
}
