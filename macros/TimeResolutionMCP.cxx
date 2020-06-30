void TimeResolutionsMCP(){

  TChain *h4 = new TChain("h4","h4");
  /*h4->Add("6815.root");
  h4->Add("6814.root");
  h4->Add("6813.root");
  h4->Add("6812.root");
  h4->Add("6811.root");
  h4->Add("6810.root");
  */
  h4->Add("../ntuples/v11/6809.root");
  
  
  /*
  h4->Add("6808.root");
  h4->Add("6807.root");
  h4->Add("6806.root");
  h4->Add("6805.root");
  */
  
  TH1F *h_amp_PTK0   = new TH1F("h_amp_PTK0", "h_amp_PTK0", 400, 0, 4000);
  TH1F *h_time_PTK0  = new TH1F("h_time_PTK0", "h_time_PTK0", 1000, 0, 200);
  TH1F *h_brms_PTK0  = new TH1F("h_brms_PTK0", "h_brms_PTK0", 100, 0, 100);
  TH1F *h_gaussigma_PTK0  = new TH1F("h_gaussigma_PTK0", "h_gaussigma_PTK0", 1000, 0, 1);

  TH1F *h_amp_PTK1   = new TH1F("h_amp_PTK1", "h_amp_PTK1", 400, 0, 4000);
  TH1F *h_time_PTK1  = new TH1F("h_time_PTK1", "h_time_PTK1", 1000, 0, 200);
  TH1F *h_brms_PTK1  = new TH1F("h_brms_PTK1", "h_brms_PTK1", 100, 0, 100);
  TH1F *h_gaussigma_PTK1 = new TH1F("h_gaussigma_PTK1", "h_gaussigma_PTK1", 1000, 0, 1);
 
  TH1F *h_CTR = new TH1F("h_CTR","h_CTR", 500, -7, -5);

  h_amp_PTK1->SetLineColor(2);
  h_time_PTK1->SetLineColor(2);
  h_brms_PTK1->SetLineColor(2);
  h_gaussigma_PTK1->SetLineColor(2);
  
  h4->Draw("amp_max[PTK0]>>h_amp_PTK0","","GOFF");
  h4->Draw("amp_max[PTK1]>>h_amp_PTK1","","GOFF");

  h4->Draw("time_max[PTK0]>>h_time_PTK0","amp_max[PTK0]>20","GOFF");
  h4->Draw("time_max[PTK1]>>h_time_PTK1","amp_max[PTK1]>20","GOFF");

  h4->Draw("b_rms[PTK0]>>h_brms_PTK0","amp_max[PTK0]>20","GOFF");
  h4->Draw("b_rms[PTK1]>>h_brms_PTK1","amp_max[PTK1]>20","GOFF");

  // 6809-6815
  //h4->Draw("time[PTK0+CFD]-time[PTK1+CFD] >> h_CTR","amp_max[PTK0]>500 && amp_max[PTK1]>400 && amp_max[PTK0]<1500 && amp_max[PTK1]<1000 && time_max[PTK0]>38 && time_max[PTK0]<57 && time_max[PTK1]>43 && time_max[PTK1]<63 && b_rms[PTK0]<16 && b_rms[PTK1]<16");
  h4->Draw("time_max[PTK0]-time_max[PTK1] >> h_CTR","amp_max[PTK0]>500 && amp_max[PTK1]>400 && amp_max[PTK0]<1500 && amp_max[PTK1]<1000 && time_max[PTK0]>38 && time_max[PTK0]<57 && time_max[PTK1]>43 && time_max[PTK1]<63 && b_rms[PTK0]<16 && b_rms[PTK1]<16");
  h4->Draw("gaus_sigma[PTK0] >> h_gaussigma_PTK0","amp_max[PTK0]>500 && amp_max[PTK1]>400 && amp_max[PTK0]<1500 && amp_max[PTK1]<1000 && time_max[PTK0]>38 && time_max[PTK0]<57 && time_max[PTK1]>43 && time_max[PTK1]<63 && b_rms[PTK0]<16 && b_rms[PTK1]<16");
  h4->Draw("gaus_sigma[PTK1] >> h_gaussigma_PTK1","amp_max[PTK0]>500 && amp_max[PTK1]>400 && amp_max[PTK0]<1500 && amp_max[PTK1]<1000 && time_max[PTK0]>38 && time_max[PTK0]<57 && time_max[PTK1]>43 && time_max[PTK1]<63 && b_rms[PTK0]<16 && b_rms[PTK1]<16");
    
// 6805-6808
  /*
h4->Draw("time_max[PTK0]-time_max[PTK1] >> h_CTR","amp_max[PTK0]>220 && amp_max[PTK1]>220 && amp_max[PTK0]<700 && amp_max[PTK1]<500 && time_max[PTK0]>38 && time_max[PTK0]<57 && time_max[PTK1]>42 && time_max[PTK1]<62 && b_rms[PTK0]<16 && b_rms[PTK1]<16");
 h4->Draw("gaus_sigma[PTK0] >> h_gaussigma_PTK0","amp_max[PTK0]>220 && amp_max[PTK1]>220 && amp_max[PTK0]<700 && amp_max[PTK1]<500 && time_max[PTK0]>38 && time_max[PTK0]<57 && time_max[PTK1]>42 && time_max[PTK1]<62 && b_rms[PTK0]<16 && b_rms[PTK1]<16");
 h4->Draw("gaus_sigma[PTK1] >> h_gaussigma_PTK1","amp_max[PTK0]>220 && amp_max[PTK1]>220 && amp_max[PTK0]<700 && amp_max[PTK1]<500 && time_max[PTK0]>38 && time_max[PTK0]<57 && time_max[PTK1]>42 && time_max[PTK1]<62 && b_rms[PTK0]<16 && b_rms[PTK1]<16");
  */    

  
  TCanvas * cAmp = new TCanvas("cAmp","cAmp");
  h_amp_PTK0->Draw();
  h_amp_PTK1->Draw("same");

  TCanvas * cTime = new TCanvas("cTime","cTime");
  h_time_PTK0->Draw();
  h_time_PTK1->Draw("same");

  TCanvas * cRms = new TCanvas("cRms","cRms");
  h_brms_PTK0->Draw();
  h_brms_PTK1->Draw("same");

  TCanvas * cGaussigma = new TCanvas("cGaussigma","cGaussigma");
  h_gaussigma_PTK0->Draw();
  h_gaussigma_PTK1->Draw("same");
  
  TCanvas * cCTR = new TCanvas("cCTR", "cCTR");
  h_CTR->Fit("gaus");
  h_CTR->Draw("e");

}
