void g_tResolGausAve_pulseIntCorr_BAR0()
{
//=========Macro generated from canvas: g_tResolGausAve_pulseIntCorr_BAR0/g_tResolGausAve_pulseIntCorr_BAR0
//=========  (Fri Feb 19 12:22:07 2021) by ROOT version 6.14/04
   TCanvas *g_tResolGausAve_pulseIntCorr_BAR0 = new TCanvas("g_tResolGausAve_pulseIntCorr_BAR0", "g_tResolGausAve_pulseIntCorr_BAR0",0,0,600,600);
   gStyle->SetOptStat(0);
   gStyle->SetOptTitle(0);
   g_tResolGausAve_pulseIntCorr_BAR0->SetHighLightColor(2);
   g_tResolGausAve_pulseIntCorr_BAR0->Range(-19.55651,7.307692,42.45589,71.41026);
   g_tResolGausAve_pulseIntCorr_BAR0->SetFillColor(0);
   g_tResolGausAve_pulseIntCorr_BAR0->SetBorderMode(0);
   g_tResolGausAve_pulseIntCorr_BAR0->SetBorderSize(2);
   g_tResolGausAve_pulseIntCorr_BAR0->SetTickx(1);
   g_tResolGausAve_pulseIntCorr_BAR0->SetTicky(1);
   g_tResolGausAve_pulseIntCorr_BAR0->SetLeftMargin(0.13);
   g_tResolGausAve_pulseIntCorr_BAR0->SetRightMargin(0.08);
   g_tResolGausAve_pulseIntCorr_BAR0->SetBottomMargin(0.12);
   g_tResolGausAve_pulseIntCorr_BAR0->SetFrameFillStyle(0);
   g_tResolGausAve_pulseIntCorr_BAR0->SetFrameBorderMode(0);
   g_tResolGausAve_pulseIntCorr_BAR0->SetFrameFillStyle(0);
   g_tResolGausAve_pulseIntCorr_BAR0->SetFrameBorderMode(0);
   
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
   
   Double_t g_tResolGausAve_pulseIntCorr_BAR0_fx1001[17] = {
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
   Double_t g_tResolGausAve_pulseIntCorr_BAR0_fy1001[17] = {
   23.74748,
   23.44192,
   23.97633,
   25.35185,
   25.23503,
   24.9513,
   25.2199,
   24.59999,
   25.88261,
   24.83711,
   25.56539,
   24.15329,
   25.10824,
   26.03951,
   23.13801,
   23.40703,
   23.6956};
   Double_t g_tResolGausAve_pulseIntCorr_BAR0_fex1001[17] = {
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
   Double_t g_tResolGausAve_pulseIntCorr_BAR0_fey1001[17] = {
   0.7391492,
   0.6846678,
   0.7533059,
   0.7488051,
   0.7487815,
   0.6618653,
   0.7810284,
   0.7251172,
   0.7728677,
   0.7999244,
   0.8362026,
   0.7918665,
   0.7771986,
   0.9920793,
   0.8114852,
   0.9478403,
   0.9694781};
   TGraphErrors *gre = new TGraphErrors(17,g_tResolGausAve_pulseIntCorr_BAR0_fx1001,g_tResolGausAve_pulseIntCorr_BAR0_fy1001,g_tResolGausAve_pulseIntCorr_BAR0_fex1001,g_tResolGausAve_pulseIntCorr_BAR0_fey1001);
   gre->SetName("g_tResolGausAve_pulseIntCorr_BAR0");
   gre->SetTitle("");
   gre->SetFillStyle(1000);
   gre->SetMarkerStyle(20);
   
   TH1F *Graph_g_tResolGausAve_pulseIntCorr_BAR01001 = new TH1F("Graph_g_tResolGausAve_pulseIntCorr_BAR01001","",100,-7.4,33.4);
   Graph_g_tResolGausAve_pulseIntCorr_BAR01001->SetMinimum(21.85602);
   Graph_g_tResolGausAve_pulseIntCorr_BAR01001->SetMaximum(27.50209);
   Graph_g_tResolGausAve_pulseIntCorr_BAR01001->SetDirectory(0);
   Graph_g_tResolGausAve_pulseIntCorr_BAR01001->SetStats(0);
   Graph_g_tResolGausAve_pulseIntCorr_BAR01001->SetLineStyle(0);
   Graph_g_tResolGausAve_pulseIntCorr_BAR01001->SetMarkerStyle(20);
   Graph_g_tResolGausAve_pulseIntCorr_BAR01001->GetXaxis()->SetLabelFont(42);
   Graph_g_tResolGausAve_pulseIntCorr_BAR01001->GetXaxis()->SetLabelSize(0.03);
   Graph_g_tResolGausAve_pulseIntCorr_BAR01001->GetXaxis()->SetTitleSize(0.05);
   Graph_g_tResolGausAve_pulseIntCorr_BAR01001->GetXaxis()->SetTitleOffset(0.9);
   Graph_g_tResolGausAve_pulseIntCorr_BAR01001->GetXaxis()->SetTitleFont(42);
   Graph_g_tResolGausAve_pulseIntCorr_BAR01001->GetYaxis()->SetLabelFont(42);
   Graph_g_tResolGausAve_pulseIntCorr_BAR01001->GetYaxis()->SetLabelSize(0.03);
   Graph_g_tResolGausAve_pulseIntCorr_BAR01001->GetYaxis()->SetTitleSize(0.05);
   Graph_g_tResolGausAve_pulseIntCorr_BAR01001->GetYaxis()->SetTitleOffset(1.05);
   Graph_g_tResolGausAve_pulseIntCorr_BAR01001->GetYaxis()->SetTitleFont(42);
   Graph_g_tResolGausAve_pulseIntCorr_BAR01001->GetZaxis()->SetLabelFont(42);
   Graph_g_tResolGausAve_pulseIntCorr_BAR01001->GetZaxis()->SetLabelSize(0.03);
   Graph_g_tResolGausAve_pulseIntCorr_BAR01001->GetZaxis()->SetTitleSize(0.05);
   Graph_g_tResolGausAve_pulseIntCorr_BAR01001->GetZaxis()->SetTitleFont(42);
   gre->SetHistogram(Graph_g_tResolGausAve_pulseIntCorr_BAR01001);
   
   
   TF1 *fpol0_tAve_pulseIntCorr_BAR01002 = new TF1("fpol0_tAve_pulseIntCorr_BAR0","pol0",-50,50, TF1::EAddToList::kNo);
   fpol0_tAve_pulseIntCorr_BAR01002->SetFillColor(19);
   fpol0_tAve_pulseIntCorr_BAR01002->SetFillStyle(0);
   fpol0_tAve_pulseIntCorr_BAR01002->SetMarkerStyle(20);
   fpol0_tAve_pulseIntCorr_BAR01002->SetLineWidth(2);
   fpol0_tAve_pulseIntCorr_BAR01002->SetLineStyle(2);
   fpol0_tAve_pulseIntCorr_BAR01002->SetChisquare(20.24135);
   fpol0_tAve_pulseIntCorr_BAR01002->SetNDF(16);
   fpol0_tAve_pulseIntCorr_BAR01002->GetXaxis()->SetLabelFont(42);
   fpol0_tAve_pulseIntCorr_BAR01002->GetXaxis()->SetLabelOffset(0.007);
   fpol0_tAve_pulseIntCorr_BAR01002->GetXaxis()->SetLabelSize(0.05);
   fpol0_tAve_pulseIntCorr_BAR01002->GetXaxis()->SetTitleSize(0.06);
   fpol0_tAve_pulseIntCorr_BAR01002->GetXaxis()->SetTitleOffset(0.9);
   fpol0_tAve_pulseIntCorr_BAR01002->GetXaxis()->SetTitleFont(42);
   fpol0_tAve_pulseIntCorr_BAR01002->GetYaxis()->SetLabelFont(42);
   fpol0_tAve_pulseIntCorr_BAR01002->GetYaxis()->SetLabelOffset(0.007);
   fpol0_tAve_pulseIntCorr_BAR01002->GetYaxis()->SetLabelSize(0.05);
   fpol0_tAve_pulseIntCorr_BAR01002->GetYaxis()->SetTitleSize(0.06);
   fpol0_tAve_pulseIntCorr_BAR01002->GetYaxis()->SetTitleOffset(1.05);
   fpol0_tAve_pulseIntCorr_BAR01002->GetYaxis()->SetTitleFont(42);
   fpol0_tAve_pulseIntCorr_BAR01002->SetParameter(0,24.6051);
   fpol0_tAve_pulseIntCorr_BAR01002->SetParError(0,0.1898459);
   fpol0_tAve_pulseIntCorr_BAR01002->SetParLimits(0,0,0);
   fpol0_tAve_pulseIntCorr_BAR01002->SetParent(gre);
   gre->GetListOfFunctions()->Add(fpol0_tAve_pulseIntCorr_BAR01002);
   gre->Draw("p");
   
   Double_t g_tResolGausL_pulseIntCorr_BAR0_fx1003[17] = {
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
   Double_t g_tResolGausL_pulseIntCorr_BAR0_fy1003[17] = {
   31.97726,
   32.067,
   34.3053,
   35.79863,
   33.46696,
   33.91033,
   36.56866,
   35.97163,
   35.85906,
   36.77325,
   36.32303,
   36.62397,
   37.59198,
   38.53663,
   38.2295,
   35.60456,
   36.45267};
   Double_t g_tResolGausL_pulseIntCorr_BAR0_fex1003[17] = {
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
   Double_t g_tResolGausL_pulseIntCorr_BAR0_fey1003[17] = {
   0.7741048,
   0.7856924,
   0.9219552,
   0.9192256,
   0.8528238,
   0.9359081,
   0.982462,
   1.01924,
   1.09973,
   1.006364,
   1.114698,
   1.139921,
   1.292909,
   1.280095,
   1.132256,
   1.165091,
   1.23205};
   gre = new TGraphErrors(17,g_tResolGausL_pulseIntCorr_BAR0_fx1003,g_tResolGausL_pulseIntCorr_BAR0_fy1003,g_tResolGausL_pulseIntCorr_BAR0_fex1003,g_tResolGausL_pulseIntCorr_BAR0_fey1003);
   gre->SetName("g_tResolGausL_pulseIntCorr_BAR0");
   gre->SetTitle("");
   gre->SetFillStyle(1000);

   Int_t ci;      // for color index setting
   TColor *color; // for color definition with alpha
   ci = TColor::GetColor("#0000ff");
   gre->SetLineColor(ci);

   ci = TColor::GetColor("#0000ff");
   gre->SetMarkerColor(ci);
   gre->SetMarkerStyle(20);
   
   TH1F *Graph_g_tResolGausL_pulseIntCorr_BAR01003 = new TH1F("Graph_g_tResolGausL_pulseIntCorr_BAR01003","",100,-7.4,33.4);
   Graph_g_tResolGausL_pulseIntCorr_BAR01003->SetMinimum(30.3418);
   Graph_g_tResolGausL_pulseIntCorr_BAR01003->SetMaximum(40.67808);
   Graph_g_tResolGausL_pulseIntCorr_BAR01003->SetDirectory(0);
   Graph_g_tResolGausL_pulseIntCorr_BAR01003->SetStats(0);
   Graph_g_tResolGausL_pulseIntCorr_BAR01003->SetLineStyle(0);
   Graph_g_tResolGausL_pulseIntCorr_BAR01003->SetMarkerStyle(20);
   Graph_g_tResolGausL_pulseIntCorr_BAR01003->GetXaxis()->SetLabelFont(42);
   Graph_g_tResolGausL_pulseIntCorr_BAR01003->GetXaxis()->SetLabelSize(0.03);
   Graph_g_tResolGausL_pulseIntCorr_BAR01003->GetXaxis()->SetTitleSize(0.05);
   Graph_g_tResolGausL_pulseIntCorr_BAR01003->GetXaxis()->SetTitleOffset(0.9);
   Graph_g_tResolGausL_pulseIntCorr_BAR01003->GetXaxis()->SetTitleFont(42);
   Graph_g_tResolGausL_pulseIntCorr_BAR01003->GetYaxis()->SetLabelFont(42);
   Graph_g_tResolGausL_pulseIntCorr_BAR01003->GetYaxis()->SetLabelSize(0.03);
   Graph_g_tResolGausL_pulseIntCorr_BAR01003->GetYaxis()->SetTitleSize(0.05);
   Graph_g_tResolGausL_pulseIntCorr_BAR01003->GetYaxis()->SetTitleOffset(1.05);
   Graph_g_tResolGausL_pulseIntCorr_BAR01003->GetYaxis()->SetTitleFont(42);
   Graph_g_tResolGausL_pulseIntCorr_BAR01003->GetZaxis()->SetLabelFont(42);
   Graph_g_tResolGausL_pulseIntCorr_BAR01003->GetZaxis()->SetLabelSize(0.03);
   Graph_g_tResolGausL_pulseIntCorr_BAR01003->GetZaxis()->SetTitleSize(0.05);
   Graph_g_tResolGausL_pulseIntCorr_BAR01003->GetZaxis()->SetTitleFont(42);
   gre->SetHistogram(Graph_g_tResolGausL_pulseIntCorr_BAR01003);
   
   
   TF1 *fpol0_tL_pulseIntCorr_BAR01004 = new TF1("fpol0_tL_pulseIntCorr_BAR0","pol0",-50,50, TF1::EAddToList::kNo);
   fpol0_tL_pulseIntCorr_BAR01004->SetFillColor(19);
   fpol0_tL_pulseIntCorr_BAR01004->SetFillStyle(0);
   fpol0_tL_pulseIntCorr_BAR01004->SetMarkerStyle(20);
   fpol0_tL_pulseIntCorr_BAR01004->SetLineColor(4);
   fpol0_tL_pulseIntCorr_BAR01004->SetLineWidth(2);
   fpol0_tL_pulseIntCorr_BAR01004->SetLineStyle(2);
   fpol0_tL_pulseIntCorr_BAR01004->SetChisquare(66.92704);
   fpol0_tL_pulseIntCorr_BAR01004->SetNDF(16);
   fpol0_tL_pulseIntCorr_BAR01004->GetXaxis()->SetLabelFont(42);
   fpol0_tL_pulseIntCorr_BAR01004->GetXaxis()->SetLabelOffset(0.007);
   fpol0_tL_pulseIntCorr_BAR01004->GetXaxis()->SetLabelSize(0.05);
   fpol0_tL_pulseIntCorr_BAR01004->GetXaxis()->SetTitleSize(0.06);
   fpol0_tL_pulseIntCorr_BAR01004->GetXaxis()->SetTitleOffset(0.9);
   fpol0_tL_pulseIntCorr_BAR01004->GetXaxis()->SetTitleFont(42);
   fpol0_tL_pulseIntCorr_BAR01004->GetYaxis()->SetLabelFont(42);
   fpol0_tL_pulseIntCorr_BAR01004->GetYaxis()->SetLabelOffset(0.007);
   fpol0_tL_pulseIntCorr_BAR01004->GetYaxis()->SetLabelSize(0.05);
   fpol0_tL_pulseIntCorr_BAR01004->GetYaxis()->SetTitleSize(0.06);
   fpol0_tL_pulseIntCorr_BAR01004->GetYaxis()->SetTitleOffset(1.05);
   fpol0_tL_pulseIntCorr_BAR01004->GetYaxis()->SetTitleFont(42);
   fpol0_tL_pulseIntCorr_BAR01004->SetParameter(0,35.11496);
   fpol0_tL_pulseIntCorr_BAR01004->SetParError(0,0.2429902);
   fpol0_tL_pulseIntCorr_BAR01004->SetParLimits(0,0,0);
   fpol0_tL_pulseIntCorr_BAR01004->SetParent(gre);
   gre->GetListOfFunctions()->Add(fpol0_tL_pulseIntCorr_BAR01004);
   gre->Draw("p");
   
   Double_t g_tResolGausR_pulseIntCorr_BAR0_fx1005[17] = {
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
   Double_t g_tResolGausR_pulseIntCorr_BAR0_fy1005[17] = {
   36.1779,
   35.76844,
   35.28557,
   37.26658,
   35.65329,
   36.50397,
   34.32081,
   36.34556,
   37.4247,
   37.15335,
   35.90385,
   32.96564,
   34.76587,
   32.82392,
   33.27703,
   33.77539,
   31.02544};
   Double_t g_tResolGausR_pulseIntCorr_BAR0_fex1005[17] = {
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
   Double_t g_tResolGausR_pulseIntCorr_BAR0_fey1005[17] = {
   0.9526249,
   0.9655674,
   0.9278514,
   1.064835,
   0.8971973,
   0.9984961,
   0.9847274,
   0.9174742,
   1.056266,
   1.104453,
   0.962846,
   0.9340882,
   1.161846,
   1.131496,
   1.100158,
   1.102941,
   1.229083};
   gre = new TGraphErrors(17,g_tResolGausR_pulseIntCorr_BAR0_fx1005,g_tResolGausR_pulseIntCorr_BAR0_fy1005,g_tResolGausR_pulseIntCorr_BAR0_fex1005,g_tResolGausR_pulseIntCorr_BAR0_fey1005);
   gre->SetName("g_tResolGausR_pulseIntCorr_BAR0");
   gre->SetTitle("");
   gre->SetFillStyle(1000);

   ci = TColor::GetColor("#ff0000");
   gre->SetLineColor(ci);

   ci = TColor::GetColor("#ff0000");
   gre->SetMarkerColor(ci);
   gre->SetMarkerStyle(20);
   
   TH1F *Graph_g_tResolGausR_pulseIntCorr_BAR01005 = new TH1F("Graph_g_tResolGausR_pulseIntCorr_BAR01005","",100,-7.4,33.4);
   Graph_g_tResolGausR_pulseIntCorr_BAR01005->SetMinimum(28.9279);
   Graph_g_tResolGausR_pulseIntCorr_BAR01005->SetMaximum(39.34943);
   Graph_g_tResolGausR_pulseIntCorr_BAR01005->SetDirectory(0);
   Graph_g_tResolGausR_pulseIntCorr_BAR01005->SetStats(0);
   Graph_g_tResolGausR_pulseIntCorr_BAR01005->SetLineStyle(0);
   Graph_g_tResolGausR_pulseIntCorr_BAR01005->SetMarkerStyle(20);
   Graph_g_tResolGausR_pulseIntCorr_BAR01005->GetXaxis()->SetLabelFont(42);
   Graph_g_tResolGausR_pulseIntCorr_BAR01005->GetXaxis()->SetLabelSize(0.03);
   Graph_g_tResolGausR_pulseIntCorr_BAR01005->GetXaxis()->SetTitleSize(0.05);
   Graph_g_tResolGausR_pulseIntCorr_BAR01005->GetXaxis()->SetTitleOffset(0.9);
   Graph_g_tResolGausR_pulseIntCorr_BAR01005->GetXaxis()->SetTitleFont(42);
   Graph_g_tResolGausR_pulseIntCorr_BAR01005->GetYaxis()->SetLabelFont(42);
   Graph_g_tResolGausR_pulseIntCorr_BAR01005->GetYaxis()->SetLabelSize(0.03);
   Graph_g_tResolGausR_pulseIntCorr_BAR01005->GetYaxis()->SetTitleSize(0.05);
   Graph_g_tResolGausR_pulseIntCorr_BAR01005->GetYaxis()->SetTitleOffset(1.05);
   Graph_g_tResolGausR_pulseIntCorr_BAR01005->GetYaxis()->SetTitleFont(42);
   Graph_g_tResolGausR_pulseIntCorr_BAR01005->GetZaxis()->SetLabelFont(42);
   Graph_g_tResolGausR_pulseIntCorr_BAR01005->GetZaxis()->SetLabelSize(0.03);
   Graph_g_tResolGausR_pulseIntCorr_BAR01005->GetZaxis()->SetTitleSize(0.05);
   Graph_g_tResolGausR_pulseIntCorr_BAR01005->GetZaxis()->SetTitleFont(42);
   gre->SetHistogram(Graph_g_tResolGausR_pulseIntCorr_BAR01005);
   
   
   TF1 *fpol0_tR_pulseIntCorr_BAR01006 = new TF1("fpol0_tR_pulseIntCorr_BAR0","pol0",-50,50, TF1::EAddToList::kNo);
   fpol0_tR_pulseIntCorr_BAR01006->SetFillColor(19);
   fpol0_tR_pulseIntCorr_BAR01006->SetFillStyle(0);
   fpol0_tR_pulseIntCorr_BAR01006->SetMarkerStyle(20);
   fpol0_tR_pulseIntCorr_BAR01006->SetLineColor(2);
   fpol0_tR_pulseIntCorr_BAR01006->SetLineWidth(2);
   fpol0_tR_pulseIntCorr_BAR01006->SetLineStyle(2);
   fpol0_tR_pulseIntCorr_BAR01006->SetChisquare(44.12143);
   fpol0_tR_pulseIntCorr_BAR01006->SetNDF(16);
   fpol0_tR_pulseIntCorr_BAR01006->GetXaxis()->SetLabelFont(42);
   fpol0_tR_pulseIntCorr_BAR01006->GetXaxis()->SetLabelOffset(0.007);
   fpol0_tR_pulseIntCorr_BAR01006->GetXaxis()->SetLabelSize(0.05);
   fpol0_tR_pulseIntCorr_BAR01006->GetXaxis()->SetTitleSize(0.06);
   fpol0_tR_pulseIntCorr_BAR01006->GetXaxis()->SetTitleOffset(0.9);
   fpol0_tR_pulseIntCorr_BAR01006->GetXaxis()->SetTitleFont(42);
   fpol0_tR_pulseIntCorr_BAR01006->GetYaxis()->SetLabelFont(42);
   fpol0_tR_pulseIntCorr_BAR01006->GetYaxis()->SetLabelOffset(0.007);
   fpol0_tR_pulseIntCorr_BAR01006->GetYaxis()->SetLabelSize(0.05);
   fpol0_tR_pulseIntCorr_BAR01006->GetYaxis()->SetTitleSize(0.06);
   fpol0_tR_pulseIntCorr_BAR01006->GetYaxis()->SetTitleOffset(1.05);
   fpol0_tR_pulseIntCorr_BAR01006->GetYaxis()->SetTitleFont(42);
   fpol0_tR_pulseIntCorr_BAR01006->SetParameter(0,35.20167);
   fpol0_tR_pulseIntCorr_BAR01006->SetParError(0,0.2465153);
   fpol0_tR_pulseIntCorr_BAR01006->SetParLimits(0,0,0);
   fpol0_tR_pulseIntCorr_BAR01006->SetParent(gre);
   gre->GetListOfFunctions()->Add(fpol0_tR_pulseIntCorr_BAR01006);
   gre->Draw("p");
   
   Double_t g_tResolGausDiff_pulseIntCorr_posCorr_BAR0_fx1007[17] = {
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
   Double_t g_tResolGausDiff_pulseIntCorr_posCorr_BAR0_fy1007[17] = {
   23.36558,
   24.69932,
   24.60435,
   26.02177,
   25.19411,
   24.92887,
   25.46526,
   24.95176,
   27.01127,
   25.92546,
   25.3411,
   24.63176,
   25.51637,
   25.16898,
   26.11817,
   24.95064,
   24.1299};
   Double_t g_tResolGausDiff_pulseIntCorr_posCorr_BAR0_fex1007[17] = {
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
   Double_t g_tResolGausDiff_pulseIntCorr_posCorr_BAR0_fey1007[17] = {
   0.5836458,
   0.5309153,
   0.5558648,
   0.6392169,
   0.6200396,
   0.6073386,
   0.6270072,
   0.6036388,
   0.6789847,
   0.6620947,
   0.632936,
   0.6218374,
   0.7505187,
   0.716421,
   0.7649987,
   0.7785102,
   0.8233405};
   gre = new TGraphErrors(17,g_tResolGausDiff_pulseIntCorr_posCorr_BAR0_fx1007,g_tResolGausDiff_pulseIntCorr_posCorr_BAR0_fy1007,g_tResolGausDiff_pulseIntCorr_posCorr_BAR0_fex1007,g_tResolGausDiff_pulseIntCorr_posCorr_BAR0_fey1007);
   gre->SetName("g_tResolGausDiff_pulseIntCorr_posCorr_BAR0");
   gre->SetTitle("");
   gre->SetFillStyle(1000);

   ci = TColor::GetColor("#666666");
   gre->SetLineColor(ci);

   ci = TColor::GetColor("#666666");
   gre->SetMarkerColor(ci);
   gre->SetMarkerStyle(24);
   
   TH1F *Graph_g_tResolGausDiff_pulseIntCorr_posCorr_BAR01007 = new TH1F("Graph_g_tResolGausDiff_pulseIntCorr_posCorr_BAR01007","",100,-7.4,33.4);
   Graph_g_tResolGausDiff_pulseIntCorr_posCorr_BAR01007->SetMinimum(44.5822);
   Graph_g_tResolGausDiff_pulseIntCorr_posCorr_BAR01007->SetMaximum(56.36218);
   Graph_g_tResolGausDiff_pulseIntCorr_posCorr_BAR01007->SetDirectory(0);
   Graph_g_tResolGausDiff_pulseIntCorr_posCorr_BAR01007->SetStats(0);
   Graph_g_tResolGausDiff_pulseIntCorr_posCorr_BAR01007->SetLineStyle(0);
   Graph_g_tResolGausDiff_pulseIntCorr_posCorr_BAR01007->SetMarkerStyle(20);
   Graph_g_tResolGausDiff_pulseIntCorr_posCorr_BAR01007->GetXaxis()->SetLabelFont(42);
   Graph_g_tResolGausDiff_pulseIntCorr_posCorr_BAR01007->GetXaxis()->SetLabelSize(0.03);
   Graph_g_tResolGausDiff_pulseIntCorr_posCorr_BAR01007->GetXaxis()->SetTitleSize(0.05);
   Graph_g_tResolGausDiff_pulseIntCorr_posCorr_BAR01007->GetXaxis()->SetTitleOffset(0.9);
   Graph_g_tResolGausDiff_pulseIntCorr_posCorr_BAR01007->GetXaxis()->SetTitleFont(42);
   Graph_g_tResolGausDiff_pulseIntCorr_posCorr_BAR01007->GetYaxis()->SetLabelFont(42);
   Graph_g_tResolGausDiff_pulseIntCorr_posCorr_BAR01007->GetYaxis()->SetLabelSize(0.03);
   Graph_g_tResolGausDiff_pulseIntCorr_posCorr_BAR01007->GetYaxis()->SetTitleSize(0.05);
   Graph_g_tResolGausDiff_pulseIntCorr_posCorr_BAR01007->GetYaxis()->SetTitleOffset(1.05);
   Graph_g_tResolGausDiff_pulseIntCorr_posCorr_BAR01007->GetYaxis()->SetTitleFont(42);
   Graph_g_tResolGausDiff_pulseIntCorr_posCorr_BAR01007->GetZaxis()->SetLabelFont(42);
   Graph_g_tResolGausDiff_pulseIntCorr_posCorr_BAR01007->GetZaxis()->SetLabelSize(0.03);
   Graph_g_tResolGausDiff_pulseIntCorr_posCorr_BAR01007->GetZaxis()->SetTitleSize(0.05);
   Graph_g_tResolGausDiff_pulseIntCorr_posCorr_BAR01007->GetZaxis()->SetTitleFont(42);
   gre->SetHistogram(Graph_g_tResolGausDiff_pulseIntCorr_posCorr_BAR01007);
   
   
   TF1 *fpol0_tDiff_pulseIntCorr_BAR01008 = new TF1("fpol0_tDiff_pulseIntCorr_BAR0","[0]",-100,100, TF1::EAddToList::kNo);
   fpol0_tDiff_pulseIntCorr_BAR01008->SetFillColor(19);
   fpol0_tDiff_pulseIntCorr_BAR01008->SetFillStyle(0);
   fpol0_tDiff_pulseIntCorr_BAR01008->SetMarkerStyle(20);

   ci = TColor::GetColor("#666666");
   fpol0_tDiff_pulseIntCorr_BAR01008->SetLineColor(ci);
   fpol0_tDiff_pulseIntCorr_BAR01008->SetLineWidth(1);
   fpol0_tDiff_pulseIntCorr_BAR01008->SetLineStyle(2);
   fpol0_tDiff_pulseIntCorr_BAR01008->SetChisquare(26.46529);
   fpol0_tDiff_pulseIntCorr_BAR01008->SetNDF(16);
   fpol0_tDiff_pulseIntCorr_BAR01008->GetXaxis()->SetLabelFont(42);
   fpol0_tDiff_pulseIntCorr_BAR01008->GetXaxis()->SetLabelOffset(0.007);
   fpol0_tDiff_pulseIntCorr_BAR01008->GetXaxis()->SetLabelSize(0.05);
   fpol0_tDiff_pulseIntCorr_BAR01008->GetXaxis()->SetTitleSize(0.06);
   fpol0_tDiff_pulseIntCorr_BAR01008->GetXaxis()->SetTitleOffset(0.9);
   fpol0_tDiff_pulseIntCorr_BAR01008->GetXaxis()->SetTitleFont(42);
   fpol0_tDiff_pulseIntCorr_BAR01008->GetYaxis()->SetLabelFont(42);
   fpol0_tDiff_pulseIntCorr_BAR01008->GetYaxis()->SetLabelOffset(0.007);
   fpol0_tDiff_pulseIntCorr_BAR01008->GetYaxis()->SetLabelSize(0.05);
   fpol0_tDiff_pulseIntCorr_BAR01008->GetYaxis()->SetTitleSize(0.06);
   fpol0_tDiff_pulseIntCorr_BAR01008->GetYaxis()->SetTitleOffset(1.05);
   fpol0_tDiff_pulseIntCorr_BAR01008->GetYaxis()->SetTitleFont(42);
   fpol0_tDiff_pulseIntCorr_BAR01008->SetParameter(0,25.11952);
   fpol0_tDiff_pulseIntCorr_BAR01008->SetParError(0,0.1564459);
   fpol0_tDiff_pulseIntCorr_BAR01008->SetParLimits(0,0,0);
   fpol0_tDiff_pulseIntCorr_BAR01008->SetParent(gre);
   gre->GetListOfFunctions()->Add(fpol0_tDiff_pulseIntCorr_BAR01008);
   gre->Draw("p");
   
   TLegend *leg = new TLegend(0.7,0.7,0.89,0.89,NULL,"brNDC");
   leg->SetBorderSize(0);
   leg->SetTextFont(62);
   leg->SetLineColor(1);
   leg->SetLineStyle(1);
   leg->SetLineWidth(1);
   leg->SetFillColor(0);
   leg->SetFillStyle(1001);
   TLegendEntry *entry=leg->AddEntry("g_tResolGausAve_pulseIntCorr_BAR0","t_{average}","PL");
   entry->SetLineColor(1);
   entry->SetLineStyle(1);
   entry->SetLineWidth(1);
   entry->SetMarkerColor(1);
   entry->SetMarkerStyle(20);
   entry->SetMarkerSize(1);
   entry->SetTextFont(62);
   entry=leg->AddEntry("g_tResolGausDiff_pulseIntCorr_posCorr_BAR0","t_{diff}/2","PL");

   ci = TColor::GetColor("#666666");
   entry->SetLineColor(ci);
   entry->SetLineStyle(1);
   entry->SetLineWidth(1);

   ci = TColor::GetColor("#666666");
   entry->SetMarkerColor(ci);
   entry->SetMarkerStyle(24);
   entry->SetMarkerSize(1);
   entry->SetTextFont(62);
   entry=leg->AddEntry("g_tResolGausL_pulseIntCorr_BAR0","t_{left}","PL");

   ci = TColor::GetColor("#0000ff");
   entry->SetLineColor(ci);
   entry->SetLineStyle(1);
   entry->SetLineWidth(1);

   ci = TColor::GetColor("#0000ff");
   entry->SetMarkerColor(ci);
   entry->SetMarkerStyle(20);
   entry->SetMarkerSize(1);
   entry->SetTextFont(62);
   entry=leg->AddEntry("g_tResolGausR_pulseIntCorr_BAR0","t_{right}","PL");

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
      tex = new TLatex(0.15,0.8,"t_{diff}/2 : #sigma = 25.1 #pm 0.2 ps");
tex->SetNDC();

   ci = TColor::GetColor("#666666");
   tex->SetTextColor(ci);
   tex->SetTextSize(0.035);
   tex->SetLineWidth(2);
   tex->Draw();
      tex = new TLatex(0.15,0.75,"t_{left}   : #sigma = 35.1 #pm 0.2 ps");
tex->SetNDC();

   ci = TColor::GetColor("#0000ff");
   tex->SetTextColor(ci);
   tex->SetTextSize(0.035);
   tex->SetLineWidth(2);
   tex->Draw();
      tex = new TLatex(0.15,0.7,"t_{right}  : #sigma = 35.2 #pm 0.2 ps");
tex->SetNDC();

   ci = TColor::GetColor("#ff0000");
   tex->SetTextColor(ci);
   tex->SetTextSize(0.035);
   tex->SetLineWidth(2);
   tex->Draw();
      tex = new TLatex(0.13,0.92,"LYSO:Ce 3x4x57 mm^{3} - FBK NUV-HD-TE 5x5 mm^{2}");
tex->SetNDC();
   tex->SetTextSize(0.03);
   tex->SetLineWidth(2);
   tex->Draw();
   g_tResolGausAve_pulseIntCorr_BAR0->Modified();
   g_tResolGausAve_pulseIntCorr_BAR0->cd();
   g_tResolGausAve_pulseIntCorr_BAR0->SetSelected(g_tResolGausAve_pulseIntCorr_BAR0);
}
