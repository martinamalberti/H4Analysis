void Checks(){

  TChain *h4 = new TChain("h4","h4");
  h4->Add("../ntuples/v11/89*.root");
  //h4->Add("../ntuples/v10/86*.root");
  //h4->Add("../ntuples/v10/87*.root");
  //h4->Add("../ntuples/v10/88*.root");
  //h4->Add("../ntuples/v10/89*.root");
  
  TH1F *h_amp_PTK0   = new TH1F("h_amp_PTK0", "h_amp_PTK0", 400, 0, 4000);
  TH1F *h_time_PTK0  = new TH1F("h_time_PTK0", "h_time_PTK0", 1000, 0, 200);
  TH1F *h_brms_PTK0  = new TH1F("h_brms_PTK0", "h_brms_PTK0", 100, 0, 100);
  TH1F *h_gaussigma_PTK0  = new TH1F("h_gaussigma_PTK0", "h_gaussigma_PTK0", 1000, 0, 1);

  TH1F *h_amp_1   = new TH1F("h_amp_1", "h_amp_1", 400, 0, 4000);
  TH1F *h_time_1  = new TH1F("h_time_1", "h_time_1", 1000, 0, 200);
  TH1F *h_brms_1  = new TH1F("h_brms_1", "h_brms_1", 100, 0, 100);

  TH1F *h_amp_2   = new TH1F("h_amp_2", "h_amp_2", 400, 0, 4000);
  TH1F *h_time_2  = new TH1F("h_time_2", "h_time_2", 1000, 0, 200);
  TH1F *h_brms_2  = new TH1F("h_brms_2", "h_brms_2", 100, 0, 100);

  TH2F *h_pedestal = new TH2F("h","h",100, -10, 10, 100, -10, 10);

  h_amp_2 ->SetLineColor(2);
  h_time_2 ->SetLineColor(2);
  h_brms_2 ->SetLineColor(2);

  h4->Draw("amp_max[PTK0]>>h_amp_PTK0","","GOFF");

  h4->Draw("time_max[PTK0]>>h_time_PTK0","amp_max[PTK0]>20","GOFF");

  h4->Draw("b_rms[PTK0]>>h_brms_PTK0","amp_max[PTK0]>20","GOFF");

  h4->Draw("gaus_sigma[PTK0] >> h_gaussigma_PTK0","amp_max[PTK0]>250 && amp_max[PTK0]<700 && time_max[PTK0]>40 && time_max[PTK0]<62 && b_rms[PTK0]<16");


  h4->Draw("amp_max[MIDAMP1]>>h_amp_1","","GOFF");
  h4->Draw("amp_max[MIDAMP2]>>h_amp_2","","GOFF");

  h4->Draw("time[MIDTIM1+LED100]>>h_time_1","amp_max[MIDAMP1]>100","GOFF");
  h4->Draw("time[MIDTIM2+LED100]>>h_time_2","amp_max[MIDAMP2]>100","GOFF");

  h4->Draw("b_rms[MIDTIM1]>>h_brms_1","amp_max[MIDAMP1]>100","GOFF");
  h4->Draw("b_rms[MIDTIM2]>>h_brms_2","amp_max[MIDAMP2]>100","GOFF");

  h4->Draw("pedestal[MIDTIM1]:pedestal[MIDTIM2] >> h_pedestal");
	   //"amp_max[PTK0]>250 && amp_max[PTK0]<700 && time_max[PTK0]>40 && time_max[PTK0]<62 && b_rms[PTK0]<16 && amp_max[MIDAMP1]>100 && amp_max[MIDAMP2]>100 && b_rms[MIDTIM1]<15 && b_rms[MIDTIM2]<15 && time[MIDTIM1+LED100]<50 && time[MIDTIM2+LED100]<50","GOFF");

  
  TCanvas * cAmpRef = new TCanvas("cAmpRef","cAmpRef");
  h_amp_PTK0->Draw();

  TCanvas * cTimeRef = new TCanvas("cTimeRef","cTimeRef");
  h_time_PTK0->Draw();

  TCanvas * cRmsRef = new TCanvas("cRmsRef","cRmsRef");
  h_brms_PTK0->Draw();

  TCanvas * cGaussigma = new TCanvas("cGaussigma","cGaussigma");
  h_gaussigma_PTK0->Draw();

  TCanvas * cAmp = new TCanvas("cAmp","cAmp");
  h_amp_1->Draw();
  h_amp_2->Draw("same");

  TCanvas * cTime = new TCanvas("cTime","cTime");
  h_time_1->Draw();
  h_time_2->Draw("same");

  TCanvas * cRms = new TCanvas("cRms","cRms");
  h_brms_1->Draw();
  h_brms_2->Draw("same");  

  TCanvas * c = new TCanvas("c","c");
  h_pedestal->Draw("colz");
}
