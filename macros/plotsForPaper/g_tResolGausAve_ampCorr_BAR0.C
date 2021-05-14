void g_tResolGausAve_ampCorr_BAR0()
{
//=========Macro generated from canvas: g_tResolGausAve_ampCorr_BAR0/g_tResolGausAve_ampCorr_BAR0
//=========  (Mon Jun 29 12:34:03 2020) by ROOT version 6.14/04
   TCanvas *g_tResolGausAve_ampCorr_BAR0 = new TCanvas("g_tResolGausAve_ampCorr_BAR0", "g_tResolGausAve_ampCorr_BAR0",0,45,600,600);
   gStyle->SetOptStat(0);
   gStyle->SetOptTitle(0);
   g_tResolGausAve_ampCorr_BAR0->SetHighLightColor(2);
   g_tResolGausAve_ampCorr_BAR0->Range(-19.55651,7.307692,42.45589,71.41026);
   g_tResolGausAve_ampCorr_BAR0->SetFillColor(0);
   g_tResolGausAve_ampCorr_BAR0->SetBorderMode(0);
   g_tResolGausAve_ampCorr_BAR0->SetBorderSize(2);
   g_tResolGausAve_ampCorr_BAR0->SetTickx(1);
   g_tResolGausAve_ampCorr_BAR0->SetTicky(1);
   g_tResolGausAve_ampCorr_BAR0->SetLeftMargin(0.13);
   g_tResolGausAve_ampCorr_BAR0->SetRightMargin(0.08);
   g_tResolGausAve_ampCorr_BAR0->SetBottomMargin(0.12);
   g_tResolGausAve_ampCorr_BAR0->SetFrameFillStyle(0);
   g_tResolGausAve_ampCorr_BAR0->SetFrameBorderMode(0);
   g_tResolGausAve_ampCorr_BAR0->SetFrameFillStyle(0);
   g_tResolGausAve_ampCorr_BAR0->SetFrameBorderMode(0);
   
   TH2F *hdummy__1 = new TH2F("hdummy__1","",100,-11.4949,37.4949,100,15,65);
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
   
   Double_t g_tResolGausAve_ampCorr_BAR0_fx1001[17] = {
   -3,
   -1,
   1,
   3,
   5,
   7,
   9,
   11,
   13,
   15,
   17,
   19,
   21,
   23,
   25,
   27,
   29};
   Double_t g_tResolGausAve_ampCorr_BAR0_fy1001[17] = {
   23.70709,
   23.72891,
   24.12497,
   25.47994,
   25.10545,
   24.95787,
   24.87518,
   25.19615,
   25.6262,
   24.62321,
   25.31374,
   23.20226,
   25.00298,
   26.12245,
   23.63272,
   23.99349,
   23.92128};
   Double_t g_tResolGausAve_ampCorr_BAR0_fex1001[17] = {
   1,
   1,
   1,
   1,
   1,
   1,
   1,
   1,
   1,
   1,
   1,
   1,
   1,
   1,
   1,
   1,
   1};
   Double_t g_tResolGausAve_ampCorr_BAR0_fey1001[17] = {
   0.6982376,
   0.7162545,
   0.7745125,
   0.7319549,
   0.6808169,
   0.6478576,
   0.7518767,
   0.7618844,
   0.7376595,
   0.7626486,
   0.7745239,
   0.6710771,
   0.7733857,
   0.9415693,
   0.8938724,
   0.9806341,
   1.053102};
   TGraphErrors *gre = new TGraphErrors(17,g_tResolGausAve_ampCorr_BAR0_fx1001,g_tResolGausAve_ampCorr_BAR0_fy1001,g_tResolGausAve_ampCorr_BAR0_fex1001,g_tResolGausAve_ampCorr_BAR0_fey1001);
   gre->SetName("g_tResolGausAve_ampCorr_BAR0");
   gre->SetTitle("");
   gre->SetFillStyle(1000);
   gre->SetMarkerStyle(20);
   
   TH1F *Graph_g_tResolGausAve_ampCorr_BAR01001 = new TH1F("Graph_g_tResolGausAve_ampCorr_BAR01001","",100,-7.4,33.4);
   Graph_g_tResolGausAve_ampCorr_BAR01001->SetMinimum(22.07789);
   Graph_g_tResolGausAve_ampCorr_BAR01001->SetMaximum(27.5173);
   Graph_g_tResolGausAve_ampCorr_BAR01001->SetDirectory(0);
   Graph_g_tResolGausAve_ampCorr_BAR01001->SetStats(0);
   Graph_g_tResolGausAve_ampCorr_BAR01001->SetLineStyle(0);
   Graph_g_tResolGausAve_ampCorr_BAR01001->SetMarkerStyle(20);
   Graph_g_tResolGausAve_ampCorr_BAR01001->GetXaxis()->SetLabelFont(42);
   Graph_g_tResolGausAve_ampCorr_BAR01001->GetXaxis()->SetLabelSize(0.03);
   Graph_g_tResolGausAve_ampCorr_BAR01001->GetXaxis()->SetTitleSize(0.05);
   Graph_g_tResolGausAve_ampCorr_BAR01001->GetXaxis()->SetTitleOffset(0.9);
   Graph_g_tResolGausAve_ampCorr_BAR01001->GetXaxis()->SetTitleFont(42);
   Graph_g_tResolGausAve_ampCorr_BAR01001->GetYaxis()->SetLabelFont(42);
   Graph_g_tResolGausAve_ampCorr_BAR01001->GetYaxis()->SetLabelSize(0.03);
   Graph_g_tResolGausAve_ampCorr_BAR01001->GetYaxis()->SetTitleSize(0.05);
   Graph_g_tResolGausAve_ampCorr_BAR01001->GetYaxis()->SetTitleOffset(1.05);
   Graph_g_tResolGausAve_ampCorr_BAR01001->GetYaxis()->SetTitleFont(42);
   Graph_g_tResolGausAve_ampCorr_BAR01001->GetZaxis()->SetLabelFont(42);
   Graph_g_tResolGausAve_ampCorr_BAR01001->GetZaxis()->SetLabelSize(0.03);
   Graph_g_tResolGausAve_ampCorr_BAR01001->GetZaxis()->SetTitleSize(0.05);
   Graph_g_tResolGausAve_ampCorr_BAR01001->GetZaxis()->SetTitleFont(42);
   gre->SetHistogram(Graph_g_tResolGausAve_ampCorr_BAR01001);
   
   
   TF1 *fpol0_tAve_ampCorr_BAR01002 = new TF1("fpol0_tAve_ampCorr_BAR0","pol0",-50,50, TF1::EAddToList::kNo);
   fpol0_tAve_ampCorr_BAR01002->SetFillColor(19);
   fpol0_tAve_ampCorr_BAR01002->SetFillStyle(0);
   fpol0_tAve_ampCorr_BAR01002->SetMarkerStyle(20);
   fpol0_tAve_ampCorr_BAR01002->SetLineWidth(2);
   fpol0_tAve_ampCorr_BAR01002->SetLineStyle(2);
   fpol0_tAve_ampCorr_BAR01002->SetChisquare(18.49862);
   fpol0_tAve_ampCorr_BAR01002->SetNDF(16);
   fpol0_tAve_ampCorr_BAR01002->GetXaxis()->SetLabelFont(42);
   fpol0_tAve_ampCorr_BAR01002->GetXaxis()->SetLabelOffset(0.007);
   fpol0_tAve_ampCorr_BAR01002->GetXaxis()->SetLabelSize(0.05);
   fpol0_tAve_ampCorr_BAR01002->GetXaxis()->SetTitleSize(0.06);
   fpol0_tAve_ampCorr_BAR01002->GetXaxis()->SetTitleOffset(0.9);
   fpol0_tAve_ampCorr_BAR01002->GetXaxis()->SetTitleFont(42);
   fpol0_tAve_ampCorr_BAR01002->GetYaxis()->SetLabelFont(42);
   fpol0_tAve_ampCorr_BAR01002->GetYaxis()->SetLabelOffset(0.007);
   fpol0_tAve_ampCorr_BAR01002->GetYaxis()->SetLabelSize(0.05);
   fpol0_tAve_ampCorr_BAR01002->GetYaxis()->SetTitleSize(0.06);
   fpol0_tAve_ampCorr_BAR01002->GetYaxis()->SetTitleOffset(1.05);
   fpol0_tAve_ampCorr_BAR01002->GetYaxis()->SetTitleFont(42);
   fpol0_tAve_ampCorr_BAR01002->SetParameter(0,24.62509);
   fpol0_tAve_ampCorr_BAR01002->SetParError(0,0.1856854);
   fpol0_tAve_ampCorr_BAR01002->SetParLimits(0,0,0);
   fpol0_tAve_ampCorr_BAR01002->SetParent(gre);
   gre->GetListOfFunctions()->Add(fpol0_tAve_ampCorr_BAR01002);
   gre->Draw("p");
   
   Double_t g_tResolGausL_ampCorr_BAR0_fx1003[17] = {
   -3,
   -1,
   1,
   3,
   5,
   7,
   9,
   11,
   13,
   15,
   17,
   19,
   21,
   23,
   25,
   27,
   29};
   Double_t g_tResolGausL_ampCorr_BAR0_fy1003[17] = {
   32.10573,
   32.1608,
   34.22096,
   35.99118,
   35.50733,
   33.22501,
   36.97699,
   35.42084,
   36.9873,
   35.7959,
   36.52155,
   36.05627,
   37.63939,
   40.17194,
   38.28559,
   36.02253,
   36.30051};
   Double_t g_tResolGausL_ampCorr_BAR0_fex1003[17] = {
   1,
   1,
   1,
   1,
   1,
   1,
   1,
   1,
   1,
   1,
   1,
   1,
   1,
   1,
   1,
   1,
   1};
   Double_t g_tResolGausL_ampCorr_BAR0_fey1003[17] = {
   0.8473956,
   0.7669017,
   0.8488068,
   0.9153474,
   0.9774098,
   0.8756974,
   1.018252,
   0.9188463,
   1.086205,
   0.9368573,
   1.05034,
   1.174654,
   1.24149,
   1.189688,
   1.203881,
   1.24258,
   1.287197};
   gre = new TGraphErrors(17,g_tResolGausL_ampCorr_BAR0_fx1003,g_tResolGausL_ampCorr_BAR0_fy1003,g_tResolGausL_ampCorr_BAR0_fex1003,g_tResolGausL_ampCorr_BAR0_fey1003);
   gre->SetName("g_tResolGausL_ampCorr_BAR0");
   gre->SetTitle("");
   gre->SetFillStyle(1000);

   Int_t ci;      // for color index setting
   TColor *color; // for color definition with alpha
   ci = TColor::GetColor("#0000ff");
   gre->SetLineColor(ci);

   ci = TColor::GetColor("#0000ff");
   gre->SetMarkerColor(ci);
   gre->SetMarkerStyle(20);
   
   TH1F *Graph_g_tResolGausL_ampCorr_BAR01003 = new TH1F("Graph_g_tResolGausL_ampCorr_BAR01003","",100,-7.4,33.4);
   Graph_g_tResolGausL_ampCorr_BAR01003->SetMinimum(30.248);
   Graph_g_tResolGausL_ampCorr_BAR01003->SetMaximum(42.37196);
   Graph_g_tResolGausL_ampCorr_BAR01003->SetDirectory(0);
   Graph_g_tResolGausL_ampCorr_BAR01003->SetStats(0);
   Graph_g_tResolGausL_ampCorr_BAR01003->SetLineStyle(0);
   Graph_g_tResolGausL_ampCorr_BAR01003->SetMarkerStyle(20);
   Graph_g_tResolGausL_ampCorr_BAR01003->GetXaxis()->SetLabelFont(42);
   Graph_g_tResolGausL_ampCorr_BAR01003->GetXaxis()->SetLabelSize(0.03);
   Graph_g_tResolGausL_ampCorr_BAR01003->GetXaxis()->SetTitleSize(0.05);
   Graph_g_tResolGausL_ampCorr_BAR01003->GetXaxis()->SetTitleOffset(0.9);
   Graph_g_tResolGausL_ampCorr_BAR01003->GetXaxis()->SetTitleFont(42);
   Graph_g_tResolGausL_ampCorr_BAR01003->GetYaxis()->SetLabelFont(42);
   Graph_g_tResolGausL_ampCorr_BAR01003->GetYaxis()->SetLabelSize(0.03);
   Graph_g_tResolGausL_ampCorr_BAR01003->GetYaxis()->SetTitleSize(0.05);
   Graph_g_tResolGausL_ampCorr_BAR01003->GetYaxis()->SetTitleOffset(1.05);
   Graph_g_tResolGausL_ampCorr_BAR01003->GetYaxis()->SetTitleFont(42);
   Graph_g_tResolGausL_ampCorr_BAR01003->GetZaxis()->SetLabelFont(42);
   Graph_g_tResolGausL_ampCorr_BAR01003->GetZaxis()->SetLabelSize(0.03);
   Graph_g_tResolGausL_ampCorr_BAR01003->GetZaxis()->SetTitleSize(0.05);
   Graph_g_tResolGausL_ampCorr_BAR01003->GetZaxis()->SetTitleFont(42);
   gre->SetHistogram(Graph_g_tResolGausL_ampCorr_BAR01003);
   
   
   TF1 *fpol0_tL_ampCorr_BAR01004 = new TF1("fpol0_tL_ampCorr_BAR0","pol0",-50,50, TF1::EAddToList::kNo);
   fpol0_tL_ampCorr_BAR01004->SetFillColor(19);
   fpol0_tL_ampCorr_BAR01004->SetFillStyle(0);
   fpol0_tL_ampCorr_BAR01004->SetMarkerStyle(20);
   fpol0_tL_ampCorr_BAR01004->SetLineColor(4);
   fpol0_tL_ampCorr_BAR01004->SetLineWidth(2);
   fpol0_tL_ampCorr_BAR01004->SetLineStyle(2);
   fpol0_tL_ampCorr_BAR01004->SetChisquare(73.3961);
   fpol0_tL_ampCorr_BAR01004->SetNDF(16);
   fpol0_tL_ampCorr_BAR01004->GetXaxis()->SetLabelFont(42);
   fpol0_tL_ampCorr_BAR01004->GetXaxis()->SetLabelOffset(0.007);
   fpol0_tL_ampCorr_BAR01004->GetXaxis()->SetLabelSize(0.05);
   fpol0_tL_ampCorr_BAR01004->GetXaxis()->SetTitleSize(0.06);
   fpol0_tL_ampCorr_BAR01004->GetXaxis()->SetTitleOffset(0.9);
   fpol0_tL_ampCorr_BAR01004->GetXaxis()->SetTitleFont(42);
   fpol0_tL_ampCorr_BAR01004->GetYaxis()->SetLabelFont(42);
   fpol0_tL_ampCorr_BAR01004->GetYaxis()->SetLabelOffset(0.007);
   fpol0_tL_ampCorr_BAR01004->GetYaxis()->SetLabelSize(0.05);
   fpol0_tL_ampCorr_BAR01004->GetYaxis()->SetTitleSize(0.06);
   fpol0_tL_ampCorr_BAR01004->GetYaxis()->SetTitleOffset(1.05);
   fpol0_tL_ampCorr_BAR01004->GetYaxis()->SetTitleFont(42);
   fpol0_tL_ampCorr_BAR01004->SetParameter(0,35.3325);
   fpol0_tL_ampCorr_BAR01004->SetParError(0,0.2419053);
   fpol0_tL_ampCorr_BAR01004->SetParLimits(0,0,0);
   fpol0_tL_ampCorr_BAR01004->SetParent(gre);
   gre->GetListOfFunctions()->Add(fpol0_tL_ampCorr_BAR01004);
   gre->Draw("p");
   
   Double_t g_tResolGausR_ampCorr_BAR0_fx1005[17] = {
   -3,
   -1,
   1,
   3,
   5,
   7,
   9,
   11,
   13,
   15,
   17,
   19,
   21,
   23,
   25,
   27,
   29};
   Double_t g_tResolGausR_ampCorr_BAR0_fy1005[17] = {
   35.87007,
   35.02629,
   34.80284,
   36.35148,
   36.97531,
   36.18806,
   34.76026,
   36.25482,
   36.94798,
   37.56259,
   36.9265,
   34.21764,
   35.49681,
   33.50974,
   32.10473,
   34.28815,
   29.31443};
   Double_t g_tResolGausR_ampCorr_BAR0_fex1005[17] = {
   1,
   1,
   1,
   1,
   1,
   1,
   1,
   1,
   1,
   1,
   1,
   1,
   1,
   1,
   1,
   1,
   1};
   Double_t g_tResolGausR_ampCorr_BAR0_fey1005[17] = {
   0.962001,
   0.9066253,
   0.8991602,
   1.073502,
   1.038561,
   0.9723041,
   0.9141612,
   0.8675064,
   1.045691,
   1.129221,
   1.074233,
   1.142233,
   1.190653,
   1.213212,
   0.9688602,
   1.105505,
   1.109024};
   gre = new TGraphErrors(17,g_tResolGausR_ampCorr_BAR0_fx1005,g_tResolGausR_ampCorr_BAR0_fy1005,g_tResolGausR_ampCorr_BAR0_fex1005,g_tResolGausR_ampCorr_BAR0_fey1005);
   gre->SetName("g_tResolGausR_ampCorr_BAR0");
   gre->SetTitle("");
   gre->SetFillStyle(1000);

   ci = TColor::GetColor("#ff0000");
   gre->SetLineColor(ci);

   ci = TColor::GetColor("#ff0000");
   gre->SetMarkerColor(ci);
   gre->SetMarkerStyle(20);
   
   TH1F *Graph_g_tResolGausR_ampCorr_BAR01005 = new TH1F("Graph_g_tResolGausR_ampCorr_BAR01005","",100,-7.4,33.4);
   Graph_g_tResolGausR_ampCorr_BAR01005->SetMinimum(27.15677);
   Graph_g_tResolGausR_ampCorr_BAR01005->SetMaximum(39.74045);
   Graph_g_tResolGausR_ampCorr_BAR01005->SetDirectory(0);
   Graph_g_tResolGausR_ampCorr_BAR01005->SetStats(0);
   Graph_g_tResolGausR_ampCorr_BAR01005->SetLineStyle(0);
   Graph_g_tResolGausR_ampCorr_BAR01005->SetMarkerStyle(20);
   Graph_g_tResolGausR_ampCorr_BAR01005->GetXaxis()->SetLabelFont(42);
   Graph_g_tResolGausR_ampCorr_BAR01005->GetXaxis()->SetLabelSize(0.03);
   Graph_g_tResolGausR_ampCorr_BAR01005->GetXaxis()->SetTitleSize(0.05);
   Graph_g_tResolGausR_ampCorr_BAR01005->GetXaxis()->SetTitleOffset(0.9);
   Graph_g_tResolGausR_ampCorr_BAR01005->GetXaxis()->SetTitleFont(42);
   Graph_g_tResolGausR_ampCorr_BAR01005->GetYaxis()->SetLabelFont(42);
   Graph_g_tResolGausR_ampCorr_BAR01005->GetYaxis()->SetLabelSize(0.03);
   Graph_g_tResolGausR_ampCorr_BAR01005->GetYaxis()->SetTitleSize(0.05);
   Graph_g_tResolGausR_ampCorr_BAR01005->GetYaxis()->SetTitleOffset(1.05);
   Graph_g_tResolGausR_ampCorr_BAR01005->GetYaxis()->SetTitleFont(42);
   Graph_g_tResolGausR_ampCorr_BAR01005->GetZaxis()->SetLabelFont(42);
   Graph_g_tResolGausR_ampCorr_BAR01005->GetZaxis()->SetLabelSize(0.03);
   Graph_g_tResolGausR_ampCorr_BAR01005->GetZaxis()->SetTitleSize(0.05);
   Graph_g_tResolGausR_ampCorr_BAR01005->GetZaxis()->SetTitleFont(42);
   gre->SetHistogram(Graph_g_tResolGausR_ampCorr_BAR01005);
   
   
   TF1 *fpol0_tR_ampCorr_BAR01006 = new TF1("fpol0_tR_ampCorr_BAR0","pol0",-50,50, TF1::EAddToList::kNo);
   fpol0_tR_ampCorr_BAR01006->SetFillColor(19);
   fpol0_tR_ampCorr_BAR01006->SetFillStyle(0);
   fpol0_tR_ampCorr_BAR01006->SetMarkerStyle(20);
   fpol0_tR_ampCorr_BAR01006->SetLineColor(2);
   fpol0_tR_ampCorr_BAR01006->SetLineWidth(2);
   fpol0_tR_ampCorr_BAR01006->SetLineStyle(2);
   fpol0_tR_ampCorr_BAR01006->SetChisquare(59.02101);
   fpol0_tR_ampCorr_BAR01006->SetNDF(16);
   fpol0_tR_ampCorr_BAR01006->GetXaxis()->SetLabelFont(42);
   fpol0_tR_ampCorr_BAR01006->GetXaxis()->SetLabelOffset(0.007);
   fpol0_tR_ampCorr_BAR01006->GetXaxis()->SetLabelSize(0.05);
   fpol0_tR_ampCorr_BAR01006->GetXaxis()->SetTitleSize(0.06);
   fpol0_tR_ampCorr_BAR01006->GetXaxis()->SetTitleOffset(0.9);
   fpol0_tR_ampCorr_BAR01006->GetXaxis()->SetTitleFont(42);
   fpol0_tR_ampCorr_BAR01006->GetYaxis()->SetLabelFont(42);
   fpol0_tR_ampCorr_BAR01006->GetYaxis()->SetLabelOffset(0.007);
   fpol0_tR_ampCorr_BAR01006->GetYaxis()->SetLabelSize(0.05);
   fpol0_tR_ampCorr_BAR01006->GetYaxis()->SetTitleSize(0.06);
   fpol0_tR_ampCorr_BAR01006->GetYaxis()->SetTitleOffset(1.05);
   fpol0_tR_ampCorr_BAR01006->GetYaxis()->SetTitleFont(42);
   fpol0_tR_ampCorr_BAR01006->SetParameter(0,35.1396);
   fpol0_tR_ampCorr_BAR01006->SetParError(0,0.2474971);
   fpol0_tR_ampCorr_BAR01006->SetParLimits(0,0,0);
   fpol0_tR_ampCorr_BAR01006->SetParent(gre);
   gre->GetListOfFunctions()->Add(fpol0_tR_ampCorr_BAR01006);
   gre->Draw("p");
   
   Double_t g_tResolGausDiff_ampCorr_posCorr_BAR0_fx1007[17] = {
   -3,
   -1,
   1,
   3,
   5,
   7,
   9,
   11,
   13,
   15,
   17,
   19,
   21,
   23,
   25,
   27,
   29};
   Double_t g_tResolGausDiff_ampCorr_posCorr_BAR0_fy1007[17] = {
   23.34503,
   24.47246,
   24.41914,
   25.97042,
   25.51202,
   25.00095,
   25.30307,
   24.37735,
   26.92315,
   27.17517,
   26.31786,
   25.14573,
   25.43434,
   26.1308,
   25.29595,
   25.6257,
   23.05525};
   Double_t g_tResolGausDiff_ampCorr_posCorr_BAR0_fex1007[17] = {
   1,
   1,
   1,
   1,
   1,
   1,
   1,
   1,
   1,
   1,
   1,
   1,
   1,
   1,
   1,
   1,
   1};
   Double_t g_tResolGausDiff_ampCorr_posCorr_BAR0_fey1007[17] = {
   0.5686497,
   0.5267981,
   0.536048,
   0.6443015,
   0.653388,
   0.6341583,
   0.6078141,
   0.5627943,
   0.7031027,
   0.7195888,
   0.7222184,
   0.6623201,
   0.6812729,
   0.8235489,
   0.7191962,
   0.7428212,
   0.8570137};
   gre = new TGraphErrors(17,g_tResolGausDiff_ampCorr_posCorr_BAR0_fx1007,g_tResolGausDiff_ampCorr_posCorr_BAR0_fy1007,g_tResolGausDiff_ampCorr_posCorr_BAR0_fex1007,g_tResolGausDiff_ampCorr_posCorr_BAR0_fey1007);
   gre->SetName("g_tResolGausDiff_ampCorr_posCorr_BAR0");
   gre->SetTitle("");
   gre->SetFillStyle(1000);

   ci = TColor::GetColor("#666666");
   gre->SetLineColor(ci);

   ci = TColor::GetColor("#666666");
   gre->SetMarkerColor(ci);
   gre->SetMarkerStyle(24);
   
   TH1F *Graph_g_tResolGausDiff_ampCorr_posCorr_BAR01007 = new TH1F("Graph_g_tResolGausDiff_ampCorr_posCorr_BAR01007","",100,-7.4,33.4);
   Graph_g_tResolGausDiff_ampCorr_posCorr_BAR01007->SetMinimum(43.25716);
   Graph_g_tResolGausDiff_ampCorr_posCorr_BAR01007->SetMaximum(56.92882);
   Graph_g_tResolGausDiff_ampCorr_posCorr_BAR01007->SetDirectory(0);
   Graph_g_tResolGausDiff_ampCorr_posCorr_BAR01007->SetStats(0);
   Graph_g_tResolGausDiff_ampCorr_posCorr_BAR01007->SetLineStyle(0);
   Graph_g_tResolGausDiff_ampCorr_posCorr_BAR01007->SetMarkerStyle(20);
   Graph_g_tResolGausDiff_ampCorr_posCorr_BAR01007->GetXaxis()->SetLabelFont(42);
   Graph_g_tResolGausDiff_ampCorr_posCorr_BAR01007->GetXaxis()->SetLabelSize(0.03);
   Graph_g_tResolGausDiff_ampCorr_posCorr_BAR01007->GetXaxis()->SetTitleSize(0.05);
   Graph_g_tResolGausDiff_ampCorr_posCorr_BAR01007->GetXaxis()->SetTitleOffset(0.9);
   Graph_g_tResolGausDiff_ampCorr_posCorr_BAR01007->GetXaxis()->SetTitleFont(42);
   Graph_g_tResolGausDiff_ampCorr_posCorr_BAR01007->GetYaxis()->SetLabelFont(42);
   Graph_g_tResolGausDiff_ampCorr_posCorr_BAR01007->GetYaxis()->SetLabelSize(0.03);
   Graph_g_tResolGausDiff_ampCorr_posCorr_BAR01007->GetYaxis()->SetTitleSize(0.05);
   Graph_g_tResolGausDiff_ampCorr_posCorr_BAR01007->GetYaxis()->SetTitleOffset(1.05);
   Graph_g_tResolGausDiff_ampCorr_posCorr_BAR01007->GetYaxis()->SetTitleFont(42);
   Graph_g_tResolGausDiff_ampCorr_posCorr_BAR01007->GetZaxis()->SetLabelFont(42);
   Graph_g_tResolGausDiff_ampCorr_posCorr_BAR01007->GetZaxis()->SetLabelSize(0.03);
   Graph_g_tResolGausDiff_ampCorr_posCorr_BAR01007->GetZaxis()->SetTitleSize(0.05);
   Graph_g_tResolGausDiff_ampCorr_posCorr_BAR01007->GetZaxis()->SetTitleFont(42);
   gre->SetHistogram(Graph_g_tResolGausDiff_ampCorr_posCorr_BAR01007);
   
   
   TF1 *fpol0_tDiff_ampCorr_BAR01008 = new TF1("fpol0_tDiff_ampCorr_BAR0","[0]",-100,100, TF1::EAddToList::kNo);
   fpol0_tDiff_ampCorr_BAR01008->SetFillColor(19);
   fpol0_tDiff_ampCorr_BAR01008->SetFillStyle(0);
   fpol0_tDiff_ampCorr_BAR01008->SetMarkerStyle(20);

   ci = TColor::GetColor("#666666");
   fpol0_tDiff_ampCorr_BAR01008->SetLineColor(ci);
   fpol0_tDiff_ampCorr_BAR01008->SetLineWidth(1);
   fpol0_tDiff_ampCorr_BAR01008->SetLineStyle(2);
   fpol0_tDiff_ampCorr_BAR01008->SetChisquare(42.43352);
   fpol0_tDiff_ampCorr_BAR01008->SetNDF(16);
   fpol0_tDiff_ampCorr_BAR01008->GetXaxis()->SetLabelFont(42);
   fpol0_tDiff_ampCorr_BAR01008->GetXaxis()->SetLabelOffset(0.007);
   fpol0_tDiff_ampCorr_BAR01008->GetXaxis()->SetLabelSize(0.05);
   fpol0_tDiff_ampCorr_BAR01008->GetXaxis()->SetTitleSize(0.06);
   fpol0_tDiff_ampCorr_BAR01008->GetXaxis()->SetTitleOffset(0.9);
   fpol0_tDiff_ampCorr_BAR01008->GetXaxis()->SetTitleFont(42);
   fpol0_tDiff_ampCorr_BAR01008->GetYaxis()->SetLabelFont(42);
   fpol0_tDiff_ampCorr_BAR01008->GetYaxis()->SetLabelOffset(0.007);
   fpol0_tDiff_ampCorr_BAR01008->GetYaxis()->SetLabelSize(0.05);
   fpol0_tDiff_ampCorr_BAR01008->GetYaxis()->SetTitleSize(0.06);
   fpol0_tDiff_ampCorr_BAR01008->GetYaxis()->SetTitleOffset(1.05);
   fpol0_tDiff_ampCorr_BAR01008->GetYaxis()->SetTitleFont(42);
   fpol0_tDiff_ampCorr_BAR01008->SetParameter(0,25.1485);
   fpol0_tDiff_ampCorr_BAR01008->SetParError(0,0.157789);
   fpol0_tDiff_ampCorr_BAR01008->SetParLimits(0,0,0);
   fpol0_tDiff_ampCorr_BAR01008->SetParent(gre);
   gre->GetListOfFunctions()->Add(fpol0_tDiff_ampCorr_BAR01008);
   gre->Draw("p");
   
   TLegend *leg = new TLegend(0.7,0.7,0.89,0.89,NULL,"brNDC");
   leg->SetBorderSize(0);
   leg->SetTextFont(62);
   leg->SetLineColor(1);
   leg->SetLineStyle(1);
   leg->SetLineWidth(1);
   leg->SetFillColor(0);
   leg->SetFillStyle(1001);
   TLegendEntry *entry=leg->AddEntry("g_tResolGausAve_ampCorr_BAR0","t_{average}","PL");
   entry->SetLineColor(1);
   entry->SetLineStyle(1);
   entry->SetLineWidth(1);
   entry->SetMarkerColor(1);
   entry->SetMarkerStyle(20);
   entry->SetMarkerSize(1);
   entry->SetTextFont(62);
   entry=leg->AddEntry("g_tResolGausDiff_ampCorr_posCorr_BAR0","t_{diff/2}","PL");

   ci = TColor::GetColor("#666666");
   entry->SetLineColor(ci);
   entry->SetLineStyle(1);
   entry->SetLineWidth(1);

   ci = TColor::GetColor("#666666");
   entry->SetMarkerColor(ci);
   entry->SetMarkerStyle(24);
   entry->SetMarkerSize(1);
   entry->SetTextFont(62);
   entry=leg->AddEntry("g_tResolGausL_ampCorr_BAR0","t_{left}","PL");

   ci = TColor::GetColor("#0000ff");
   entry->SetLineColor(ci);
   entry->SetLineStyle(1);
   entry->SetLineWidth(1);

   ci = TColor::GetColor("#0000ff");
   entry->SetMarkerColor(ci);
   entry->SetMarkerStyle(20);
   entry->SetMarkerSize(1);
   entry->SetTextFont(62);
   entry=leg->AddEntry("g_tResolGausR_ampCorr_BAR0","t_{right}","PL");

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
   TLatex *   tex = new TLatex(0.15,0.85,"t_{average}: #sigma = 24.6 #pm 0.2 ps");
tex->SetNDC();
   tex->SetTextSize(0.035);
   tex->SetLineWidth(2);
   tex->Draw();
      tex = new TLatex(0.15,0.8,"t_{diff/2} : #sigma = 25.1 #pm 0.2 ps");
tex->SetNDC();

   ci = TColor::GetColor("#666666");
   tex->SetTextColor(ci);
   tex->SetTextSize(0.035);
   tex->SetLineWidth(2);
   tex->Draw();
      tex = new TLatex(0.15,0.75,"t_{left}   : #sigma = 35.3 #pm 0.2 ps");
tex->SetNDC();

   ci = TColor::GetColor("#0000ff");
   tex->SetTextColor(ci);
   tex->SetTextSize(0.035);
   tex->SetLineWidth(2);
   tex->Draw();
      tex = new TLatex(0.15,0.7,"t_{right}  : #sigma = 35.1 #pm 0.2 ps");
tex->SetNDC();

   ci = TColor::GetColor("#ff0000");
   tex->SetTextColor(ci);
   tex->SetTextSize(0.035);
   tex->SetLineWidth(2);
   tex->Draw();
      tex = new TLatex(0.13,0.92,"LYSO:Ce 3x3x57 mm^{3} - FBK NUV-HD-TE 5x5 mm^{2}");
tex->SetNDC();
   tex->SetTextSize(0.03);
   tex->SetLineWidth(2);
   tex->Draw();
   g_tResolGausAve_ampCorr_BAR0->Modified();
   g_tResolGausAve_ampCorr_BAR0->cd();
   g_tResolGausAve_ampCorr_BAR0->SetSelected(g_tResolGausAve_ampCorr_BAR0);
}
