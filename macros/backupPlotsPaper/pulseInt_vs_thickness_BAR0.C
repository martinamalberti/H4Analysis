void pulseInt_vs_thickness_BAR0()
{
//=========Macro generated from canvas: pulseInt_vs_thickness_BAR0/pulseInt_vs_thickness_BAR0
//=========  (Wed Feb 17 16:39:32 2021) by ROOT version 6.14/04
   TCanvas *pulseInt_vs_thickness_BAR0 = new TCanvas("pulseInt_vs_thickness_BAR0", "pulseInt_vs_thickness_BAR0",0,0,600,600);
   gStyle->SetOptStat(0);
   gStyle->SetOptTitle(0);
   pulseInt_vs_thickness_BAR0->SetHighLightColor(2);
   pulseInt_vs_thickness_BAR0->Range(0.3417722,-0.3076923,5.405063,2.25641);
   pulseInt_vs_thickness_BAR0->SetFillColor(0);
   pulseInt_vs_thickness_BAR0->SetBorderMode(0);
   pulseInt_vs_thickness_BAR0->SetBorderSize(2);
   pulseInt_vs_thickness_BAR0->SetTickx(1);
   pulseInt_vs_thickness_BAR0->SetTicky(1);
   pulseInt_vs_thickness_BAR0->SetLeftMargin(0.13);
   pulseInt_vs_thickness_BAR0->SetRightMargin(0.08);
   pulseInt_vs_thickness_BAR0->SetBottomMargin(0.12);
   pulseInt_vs_thickness_BAR0->SetFrameFillStyle(0);
   pulseInt_vs_thickness_BAR0->SetFrameBorderMode(0);
   pulseInt_vs_thickness_BAR0->SetFrameFillStyle(0);
   pulseInt_vs_thickness_BAR0->SetFrameBorderMode(0);
   
   TH2F *hdummy33_BAR0__5 = new TH2F("hdummy33_BAR0__5","",10,1,5,10,0,2);
   hdummy33_BAR0__5->SetStats(0);
   hdummy33_BAR0__5->SetLineStyle(0);
   hdummy33_BAR0__5->SetMarkerStyle(20);
   hdummy33_BAR0__5->GetXaxis()->SetTitle("thickness (mm)");
   hdummy33_BAR0__5->GetXaxis()->SetLabelFont(42);
   hdummy33_BAR0__5->GetXaxis()->SetLabelOffset(0.007);
   hdummy33_BAR0__5->GetXaxis()->SetTitleOffset(1.1);
   hdummy33_BAR0__5->GetXaxis()->SetTitleFont(42);
   hdummy33_BAR0__5->GetYaxis()->SetTitle("normalized MPV");
   hdummy33_BAR0__5->GetYaxis()->SetLabelFont(42);
   hdummy33_BAR0__5->GetYaxis()->SetLabelOffset(0.007);
   hdummy33_BAR0__5->GetYaxis()->SetTitleOffset(1.2);
   hdummy33_BAR0__5->GetYaxis()->SetTitleFont(42);
   hdummy33_BAR0__5->GetZaxis()->SetLabelFont(42);
   hdummy33_BAR0__5->GetZaxis()->SetLabelOffset(0.007);
   hdummy33_BAR0__5->GetZaxis()->SetLabelSize(0.05);
   hdummy33_BAR0__5->GetZaxis()->SetTitleSize(0.06);
   hdummy33_BAR0__5->GetZaxis()->SetTitleFont(42);
   hdummy33_BAR0__5->Draw("");
   
   Double_t _fx1017[3] = {
   2,
   3,
   4};
   Double_t _fy1017[3] = {
   0.6522767,
   1.024905,
   1.446869};
   Double_t _fex1017[3] = {
   0,
   0,
   0};
   Double_t _fey1017[3] = {
   0.002402486,
   0.001927858,
   0.002780763};
   TGraphErrors *gre = new TGraphErrors(3,_fx1017,_fy1017,_fex1017,_fey1017);
   gre->SetName("");
   gre->SetTitle("");
   gre->SetFillStyle(1000);

   Int_t ci;      // for color index setting
   TColor *color; // for color definition with alpha
   ci = TColor::GetColor("#ff0000");
   gre->SetLineColor(ci);

   ci = TColor::GetColor("#ff0000");
   gre->SetMarkerColor(ci);
   gre->SetMarkerStyle(20);
   gre->SetMarkerSize(0.9);
   
   TH1F *Graph_Graph1017 = new TH1F("Graph_Graph1017","",100,1.8,4.2);
   Graph_Graph1017->SetMinimum(0.5698967);
   Graph_Graph1017->SetMaximum(1.529628);
   Graph_Graph1017->SetDirectory(0);
   Graph_Graph1017->SetStats(0);
   Graph_Graph1017->SetLineStyle(0);
   Graph_Graph1017->SetMarkerStyle(20);
   Graph_Graph1017->GetXaxis()->SetLabelFont(42);
   Graph_Graph1017->GetXaxis()->SetLabelOffset(0.007);
   Graph_Graph1017->GetXaxis()->SetTitleOffset(1.1);
   Graph_Graph1017->GetXaxis()->SetTitleFont(42);
   Graph_Graph1017->GetYaxis()->SetLabelFont(42);
   Graph_Graph1017->GetYaxis()->SetLabelOffset(0.007);
   Graph_Graph1017->GetYaxis()->SetTitleOffset(1.2);
   Graph_Graph1017->GetYaxis()->SetTitleFont(42);
   Graph_Graph1017->GetZaxis()->SetLabelFont(42);
   Graph_Graph1017->GetZaxis()->SetLabelOffset(0.007);
   Graph_Graph1017->GetZaxis()->SetLabelSize(0.05);
   Graph_Graph1017->GetZaxis()->SetTitleSize(0.06);
   Graph_Graph1017->GetZaxis()->SetTitleFont(42);
   gre->SetHistogram(Graph_Graph1017);
   
   
   TF1 *fffR1018 = new TF1("fffR","[0]*(x/3)+[1]",1.5,4.5, TF1::EAddToList::kNo);
   fffR1018->SetFillColor(19);
   fffR1018->SetFillStyle(0);
   fffR1018->SetMarkerStyle(20);

   ci = TColor::GetColor("#ff0000");
   fffR1018->SetLineColor(ci);
   fffR1018->SetLineWidth(1);
   fffR1018->SetLineStyle(3);
   fffR1018->SetChisquare(85.79199);
   fffR1018->SetNDF(1);
   fffR1018->GetXaxis()->SetLabelFont(42);
   fffR1018->GetXaxis()->SetLabelOffset(0.007);
   fffR1018->GetXaxis()->SetTitleOffset(1.1);
   fffR1018->GetXaxis()->SetTitleFont(42);
   fffR1018->GetYaxis()->SetLabelFont(42);
   fffR1018->GetYaxis()->SetLabelOffset(0.007);
   fffR1018->GetYaxis()->SetTitleOffset(1.2);
   fffR1018->GetYaxis()->SetTitleFont(42);
   fffR1018->SetParameter(0,1.186774);
   fffR1018->SetParError(0,0.005484566);
   fffR1018->SetParLimits(0,0,0);
   fffR1018->SetParameter(1,-0.1489433);
   fffR1018->SetParError(1,0.005505311);
   fffR1018->SetParLimits(1,0,0);
   fffR1018->SetParent(gre);
   gre->GetListOfFunctions()->Add(fffR1018);
   gre->Draw("p");
   
   Double_t _fx1019[3] = {
   2,
   3,
   4};
   Double_t _fy1019[3] = {
   0.7073328,
   0.9750949,
   1.423674};
   Double_t _fex1019[3] = {
   0,
   0,
   0};
   Double_t _fey1019[3] = {
   0.002877534,
   0.002429372,
   0.002822117};
   gre = new TGraphErrors(3,_fx1019,_fy1019,_fex1019,_fey1019);
   gre->SetName("");
   gre->SetTitle("");
   gre->SetFillStyle(1000);

   ci = TColor::GetColor("#0000ff");
   gre->SetLineColor(ci);

   ci = TColor::GetColor("#0000ff");
   gre->SetMarkerColor(ci);
   gre->SetMarkerStyle(20);
   gre->SetMarkerSize(0.9);
   
   TH1F *Graph_Graph1019 = new TH1F("Graph_Graph1019","",100,1.8,4.2);
   Graph_Graph1019->SetMinimum(0.6322512);
   Graph_Graph1019->SetMaximum(1.4987);
   Graph_Graph1019->SetDirectory(0);
   Graph_Graph1019->SetStats(0);
   Graph_Graph1019->SetLineStyle(0);
   Graph_Graph1019->SetMarkerStyle(20);
   Graph_Graph1019->GetXaxis()->SetLabelFont(42);
   Graph_Graph1019->GetXaxis()->SetLabelOffset(0.007);
   Graph_Graph1019->GetXaxis()->SetTitleOffset(1.1);
   Graph_Graph1019->GetXaxis()->SetTitleFont(42);
   Graph_Graph1019->GetYaxis()->SetLabelFont(42);
   Graph_Graph1019->GetYaxis()->SetLabelOffset(0.007);
   Graph_Graph1019->GetYaxis()->SetTitleOffset(1.2);
   Graph_Graph1019->GetYaxis()->SetTitleFont(42);
   Graph_Graph1019->GetZaxis()->SetLabelFont(42);
   Graph_Graph1019->GetZaxis()->SetLabelOffset(0.007);
   Graph_Graph1019->GetZaxis()->SetLabelSize(0.05);
   Graph_Graph1019->GetZaxis()->SetTitleSize(0.06);
   Graph_Graph1019->GetZaxis()->SetTitleFont(42);
   gre->SetHistogram(Graph_Graph1019);
   
   
   TF1 *fffL1020 = new TF1("fffL","[0]*(x/3)+[1]",1.5,4.5, TF1::EAddToList::kNo);
   fffL1020->SetFillColor(19);
   fffL1020->SetFillStyle(0);
   fffL1020->SetMarkerStyle(20);

   ci = TColor::GetColor("#0000ff");
   fffL1020->SetLineColor(ci);
   fffL1020->SetLineWidth(1);
   fffL1020->SetLineStyle(3);
   fffL1020->SetChisquare(820.4029);
   fffL1020->SetNDF(1);
   fffL1020->GetXaxis()->SetLabelFont(42);
   fffL1020->GetXaxis()->SetLabelOffset(0.007);
   fffL1020->GetXaxis()->SetTitleOffset(1.1);
   fffL1020->GetXaxis()->SetTitleFont(42);
   fffL1020->GetYaxis()->SetLabelFont(42);
   fffL1020->GetYaxis()->SetLabelOffset(0.007);
   fffL1020->GetYaxis()->SetTitleOffset(1.2);
   fffL1020->GetYaxis()->SetTitleFont(42);
   fffL1020->SetParameter(0,1.076661);
   fffL1020->SetParError(0,0.006045213);
   fffL1020->SetParLimits(0,0,0);
   fffL1020->SetParameter(1,-0.04801);
   fffL1020->SetParError(1,0.006263464);
   fffL1020->SetParLimits(1,0,0);
   fffL1020->SetParent(gre);
   gre->GetListOfFunctions()->Add(fffL1020);
   gre->Draw("p");
   
   TLegend *leg = new TLegend(0.7,0.8,0.89,0.89,NULL,"brNDC");
   leg->SetTextFont(62);
   leg->SetLineColor(1);
   leg->SetLineStyle(1);
   leg->SetLineWidth(1);
   leg->SetFillColor(0);
   leg->SetFillStyle(1001);
   TLegendEntry *entry=leg->AddEntry("","left","PL");

   ci = TColor::GetColor("#0000ff");
   entry->SetLineColor(ci);
   entry->SetLineStyle(1);
   entry->SetLineWidth(1);

   ci = TColor::GetColor("#0000ff");
   entry->SetMarkerColor(ci);
   entry->SetMarkerStyle(20);
   entry->SetMarkerSize(0.9);
   entry->SetTextFont(62);
   entry=leg->AddEntry("","right","PL");

   ci = TColor::GetColor("#ff0000");
   entry->SetLineColor(ci);
   entry->SetLineStyle(1);
   entry->SetLineWidth(1);

   ci = TColor::GetColor("#ff0000");
   entry->SetMarkerColor(ci);
   entry->SetMarkerStyle(20);
   entry->SetMarkerSize(0.9);
   entry->SetTextFont(62);
   leg->Draw();
   TLatex *   tex = new TLatex(0.13,0.92,"LYSO:Ce 3xtx57 mm^{3} - FBK NUV-HD-TE 5x5 mm^{2}");
tex->SetNDC();
   tex->SetTextSize(0.035);
   tex->SetLineWidth(2);
   tex->Draw();
   pulseInt_vs_thickness_BAR0->Modified();
   pulseInt_vs_thickness_BAR0->cd();
   pulseInt_vs_thickness_BAR0->SetSelected(pulseInt_vs_thickness_BAR0);
}
