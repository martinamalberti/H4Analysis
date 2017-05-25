#include "interface/FitUtils.h"
#include "interface/SetTDRStyle.h"

#include "CfgManager/interface/CfgManager.h"
#include "CfgManager/interface/CfgManagerT.h"

#include <iostream>
#include <vector>
#include <map>
#include <algorithm>
#include <cstdlib>
#include <string>


#include "TSystem.h"
#include "TFile.h"
#include "TTree.h"
#include "TChain.h"
#include "TGraph.h"
#include "TGraphErrors.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TProfile.h"
#include "TF1.h"
#include "TCanvas.h"
#include "TLatex.h"
#include "TLine.h"
#include "TApplication.h"


int ch1;
int ch2;
int VbiasIndex1;
int VbiasIndex2;

/*** tree variables ***/
struct TreeVars
{
  unsigned int t_n_channels;
  float t_beamX;
  float t_beamY;
  float* t_Vbias;
  float t_NINOthr;
  float* t_ped;
  float* t_amp;
  float* t_dur;
  float* t_time;
  //float* t_time_max;
  //float* t_time_maximum;
  int* t_isOk;
};

void InitTreeVars(TTree* chain1, TTree* chain2, TTree* chain3,
                  TreeVars& treeVars);

bool AcceptEvent(TreeVars treeVars);
bool AcceptEventAmp(TreeVars treeVars, const float& ampMin, const float& ampMax, const int& ch);
bool AcceptEventDur(TreeVars treeVars, const float& durMin, const float& durMax, const int& ch);
bool AcceptEventTh(TreeVars treeVars, const float& thMin, const float& thMax);
bool AcceptEventVbias(TreeVars treeVars, const float& VbiasMin, const float& VbiasMax);
bool AcceptEventAmpMCP(TreeVars treeVars, const float& ampMin, const float& ampMax, const int& mcp);


int main(int argc, char** argv)
{
  gSystem -> Load("CfgManager/lib/libCFGMan.so");
  
  if( argc < 2 )
  {
    std::cerr << ">>>>> TimingResolutionWithMCPRef.cpp::usage:   " << argv[0] << " configFileName" << std::endl;
    return 1;
  }
  
  float maxDT = 1.;
  
  //----------------------
  // parse the config file
  CfgManager opts;
  opts.ParseConfigFile(argv[1]);
  
  std::vector<std::string> inputFiles = opts.GetOpt<std::vector<std::string> >("Input.inputFiles");
  
  ch1 = opts.GetOpt<int>("Input.ch1");
  ch2 = opts.GetOpt<int>("Input.ch2");
  VbiasIndex1 = opts.GetOpt<int>("Input.VbiasIndex1");
  VbiasIndex2 = opts.GetOpt<int>("Input.VbiasIndex2");
  int configuration = opts.GetOpt<int>("Input.configuration");
  std::string OutputFile = opts.GetOpt<std::string>("Input.OutputFile");

  float cut_NINOthrMin = opts.GetOpt<float>("Cuts.minThreshold");
  float cut_NINOthrMax = opts.GetOpt<float>("Cuts.maxThreshold");
  float cut_VbiasMin = opts.GetOpt<float>("Cuts.minVbias");
  float cut_VbiasMax = opts.GetOpt<float>("Cuts.maxVbias");  
  
  std::vector<float> cut_VbiasValues1 = opts.GetOpt<std::vector<float> >("Cuts.VbiasValues1");
  std::vector<float> cut_minAmplitudes1 = opts.GetOpt<std::vector<float> >("Cuts.minAmplitudes1");
  std::vector<float> cut_maxAmplitudes1 = opts.GetOpt<std::vector<float> >("Cuts.maxAmplitudes1");
  std::map<float,float> cut_ampMin1;
  std::map<float,float> cut_ampMax1;
  for(unsigned int it = 0; it < cut_VbiasValues1.size(); ++it)
  {
    cut_ampMin1[cut_VbiasValues1.at(it)] = cut_minAmplitudes1.at(it);
    cut_ampMax1[cut_VbiasValues1.at(it)] = cut_maxAmplitudes1.at(it);
  }

  std::vector<float> cut_VbiasValues2 = opts.GetOpt<std::vector<float> >("Cuts.VbiasValues2");
  std::vector<float> cut_minAmplitudes2 = opts.GetOpt<std::vector<float> >("Cuts.minAmplitudes2");
  std::vector<float> cut_maxAmplitudes2 = opts.GetOpt<std::vector<float> >("Cuts.maxAmplitudes2");
  std::map<float,float> cut_ampMin2;
  std::map<float,float> cut_ampMax2;
  for(unsigned int it = 0; it < cut_VbiasValues2.size(); ++it)
  {
    cut_ampMin2[cut_VbiasValues2.at(it)] = cut_minAmplitudes2.at(it);
    cut_ampMax2[cut_VbiasValues2.at(it)] = cut_maxAmplitudes2.at(it);
  }
  
  std::vector<float> cut_minDurations1 = opts.GetOpt<std::vector<float> >("Cuts.minDurations1");
  std::vector<float> cut_maxDurations1 = opts.GetOpt<std::vector<float> >("Cuts.maxDurations1");
  std::map<float,float> cut_durMin1;
  std::map<float,float> cut_durMax1;
  for(unsigned int it = 0; it < cut_VbiasValues1.size(); ++it)
  {
    cut_durMin1[cut_VbiasValues1.at(it)] = cut_minDurations1.at(it);
    cut_durMax1[cut_VbiasValues1.at(it)] = cut_maxDurations1.at(it);
  }


  std::vector<float> cut_minDurations2 = opts.GetOpt<std::vector<float> >("Cuts.minDurations2");
  std::vector<float> cut_maxDurations2 = opts.GetOpt<std::vector<float> >("Cuts.maxDurations2");
  std::map<float,float> cut_durMin2;
  std::map<float,float> cut_durMax2;
  for(unsigned int it = 0; it < cut_VbiasValues2.size(); ++it)
  {
    cut_durMin2[cut_VbiasValues2.at(it)] = cut_minDurations2.at(it);
    cut_durMax2[cut_VbiasValues2.at(it)] = cut_maxDurations2.at(it);
  }
  

  std::string label1 = opts.GetOpt<std::string>("Plots.label1");
  
  //------------------------
  // labels and canvas style
  setTDRStyle();
  
  /*
  std::string baseDir(Form("/afs/cern.ch/user/m/malberti/www/PrecisionTiming/TBatT9May2017/config%02d/",configuration));
  system(Form("mkdir -p %s",baseDir.c_str()));
  system(Form("cp /afs/cern.ch/user/m/malberti/www/index.php %s",baseDir.c_str()));
  std::string plotDir(Form("/afs/cern.ch/user/m/malberti/www/PrecisionTiming/TBatT9May2017/config%02d/%s/",configuration,label1.c_str()));
  system(Form("mkdir %s",plotDir.c_str()));
  system(Form("cp /afs/cern.ch/user/m/malberti/www/index.php %s",plotDir.c_str()));
  */  
  

  //---------------------------
  // open input files and trees
  TChain* chain1 = new TChain("info","info");
  TChain* chain2 = new TChain("wire","wire");
  TChain* chain3 = new TChain("digi","digi");
  for(unsigned int fileIt = 0; fileIt < inputFiles.size(); ++fileIt)
  {
    chain1 -> Add(inputFiles.at(fileIt).c_str());
    chain2 -> Add(inputFiles.at(fileIt).c_str());
    chain3 -> Add(inputFiles.at(fileIt).c_str());
  }
  chain2 -> BuildIndex("index");
  chain1 -> AddFriend("wire");
  chain3 -> BuildIndex("index");
  chain1 -> AddFriend("digi");
  chain1 -> BuildIndex("index");
  std::cout << " Read " << chain1->GetEntries() << " total events in tree " << chain1->GetName() << std::endl;
  std::cout << " Read " << chain2->GetEntries() << " total events in tree " << chain2->GetName() << std::endl;
  std::cout << " Read " << chain3->GetEntries() << " total events in tree " << chain3->GetName() << std::endl;
  
  // set branches
  TreeVars treeVars;
  InitTreeVars(chain1,chain2,chain1,treeVars);
  
  
  //------------------
  // Book histograms
  TH1F* h_dur_ch1 = new TH1F("h_dur_ch1","",1000,0.,500.);
  TH1F* h_amp_ch1 = new TH1F("h_amp_ch1","",1000,0.,1000.);
  TH1F* h_dur_ch2 = new TH1F("h_dur_ch2","",1000,0.,500.);
  TH1F* h_amp_ch2 = new TH1F("h_amp_ch2","",1000,0.,1000.);
  TH1F* h_amp_ratio= new TH1F("h_amp_ratio","",100,0.,5.);

  TH1F* h_dur_ch1_cut = new TH1F("h_dur_ch1_cut ","",1000,0.,500.);
  TH1F* h_amp_ch1_cut  = new TH1F("h_amp_ch1_cut ","",1000,0.,1000.);
  TH1F* h_dur_ch2_cut  = new TH1F("h_dur_ch2_cut ","",1000,0.,500.);
  TH1F* h_amp_ch2_cut  = new TH1F("h_amp_ch2_cut ","",1000,0.,1000.);
  TH1F* h_amp_ratio_cut = new TH1F("h_amp_ratio_cut","",100,0.,5.);

  TH1F* h_dt = new TH1F("h_dt","",1000, -2., 2.);
  TH1F* h_dt_corr = new TH1F("h_dt_corr","",1000, -2., 2.);

  TH2F* h2_dt_vs_amp_ch1 = new TH2F("h2_dt_vs_amp_ch1","",100,0.,1000., 1000, -5., 5.);
  TH2F* h2_dt_vs_amp_ch2 = new TH2F("h2_dt_vs_amp_ch2","",100,0.,1000., 1000, -5., 5.);
  TH2F* h2_dt_vs_amp_ratio = new TH2F("h2_dt_vs_amp_ratio","",250, 0.,5., 1000, -5., 5.);
  TProfile* p_dt_vs_amp_ch1 = new TProfile("p_dt_vs_amp_ch1","",100,0.,1000.);
  TProfile* p_dt_vs_amp_ch2 = new TProfile("p_dt_vs_amp_ch2","",100,0.,1000.);
  TProfile* p_dt_vs_amp_ratio = new TProfile("p_dt_vs_amp_ratio","",250,0.,5.);

  TH2F* h2_dtcorr_vs_amp_ch1 = new TH2F("h2_dtcorr_vs_amp_ch1","",100,0.,1000., 1000, -5., 5.);
  TH2F* h2_dtcorr_vs_amp_ch2 = new TH2F("h2_dtcorr_vs_amp_ch2","",100,0.,1000., 1000, -5., 5.);
  TH2F* h2_dtcorr_vs_amp_ratio = new TH2F("h2_dtcorr_vs_amp_ratio","",250,0.,5., 1000, -5., 5.);
  TProfile* p_dtcorr_vs_amp_ch1 = new TProfile("p_dtcorr_vs_amp_ch1","",100,0.,1000.);
  TProfile* p_dtcorr_vs_amp_ch2 = new TProfile("p_dtcorr_vs_amp_ch2","",100,0.,1000.);
  TProfile* p_dtcorr_vs_amp_ratio = new TProfile("p_dtcorr_vs_amp_ratio","",250,0.,5.);
  
  TH2F* h2_beam_Y_vs_X = new TH2F("h2_beam_Y_vs_X","",200,-50.,50.,200,-50.,50.);
  TH2F* h2_beam_Y_vs_X_cut = new TH2F("h2_beam_Y_vs_X_cut","",200,-50.,50.,200,-50.,50.);
  
  
  //-----------------------
  // first loop over events
  int nEntries = chain1 -> GetEntries();
  for(int entry = 0; entry < nEntries; ++entry)
  {
    if( entry%1000 == 0 ) std::cout << ">>> Reading entry " << entry << " / " << nEntries << "\r" << std::flush;
    chain1 -> GetEntry(entry);
    
    if( !AcceptEvent(treeVars) ) continue;
    
    float Vbias1 = treeVars.t_Vbias[VbiasIndex1];
    float amp_ch1 = treeVars.t_amp[8+ch1] * 0.25;
    float dur_ch1 = treeVars.t_dur[ch1] * 0.2;
    float time_ch1 = treeVars.t_time[ch1];

    float Vbias2 = treeVars.t_Vbias[VbiasIndex2];
    float amp_ch2 = treeVars.t_amp[8+ch2] * 0.25;
    float dur_ch2 = treeVars.t_dur[ch2] * 0.2;
    float time_ch2 = treeVars.t_time[ch2];

    float dt  = time_ch1 - time_ch2;
    
    // --- Fill histograms before selections
    h2_beam_Y_vs_X -> Fill(treeVars.t_beamX,treeVars.t_beamY);

    h_dur_ch1 -> Fill( dur_ch1 );
    h_amp_ch1 -> Fill( amp_ch1 );
    h_dur_ch2 -> Fill( dur_ch2 );
    h_amp_ch2 -> Fill( amp_ch2 );
    h_amp_ratio -> Fill(amp_ch1/amp_ch2);

    // --- Select events
    // -- amplitude and duration in ch1
    //std::cout << amp_ch1 << "  " << cut_ampMin1[Vbias1] << "  " << cut_ampMax1[Vbias1] <<std::endl; 
    //std::cout << dur_ch1 << "  " << cut_durMin1[Vbias1] << "  " << cut_durMax1[Vbias1] <<std::endl; 
    //std::cout << Vbias1 << "  " << cut_VbiasMin << "  " << cut_VbiasMax <<std::endl; 
    //std::cout << treeVars.t_NINOthr << "  " << cut_NINOthrMin << "  "<< cut_NINOthrMax << std::endl;
    if( !AcceptEventAmp(treeVars,cut_ampMin1[Vbias1],cut_ampMax1[Vbias1], ch1 )) continue;
    if( !AcceptEventDur(treeVars,cut_durMin1[Vbias1],cut_durMax1[Vbias1], ch1 )) continue;
    if( !AcceptEventVbias(treeVars,cut_VbiasMin,cut_VbiasMax) ) continue;
    if( !AcceptEventTh(treeVars,cut_NINOthrMin,cut_NINOthrMax) ) continue;
    // -- amplitude and duration in ch2
    //std::cout << amp_ch2 << "  " << cut_ampMin2[Vbias2] << "  " << cut_ampMax2[Vbias2] <<std::endl; 
    //std::cout << dur_ch2 << "  " << cut_durMin2[Vbias2] << "  " << cut_durMax2[Vbias2] <<std::endl; 
    //std::cout << Vbias2 << "  " << cut_VbiasMin << "  " << cut_VbiasMax <<std::endl; 
    //std::cout << treeVars.t_NINOthr << "  " << cut_NINOthrMin << "  "<< cut_NINOthrMax << std::endl;
    if( !AcceptEventAmp(treeVars,cut_ampMin2[Vbias2],cut_ampMax2[Vbias2], ch2 )) continue;
    if( !AcceptEventDur(treeVars,cut_durMin2[Vbias2],cut_durMax2[Vbias2], ch2 )) continue;
    //if( !AcceptEventVbias(treeVars,cut_VbiasMin,cut_VbiasMax) ) continue;
    if( !AcceptEventTh(treeVars,cut_NINOthrMin,cut_NINOthrMax) ) continue;

    // Fill histograms after selections
    h2_beam_Y_vs_X_cut -> Fill(treeVars.t_beamX,treeVars.t_beamY);
  
    h_dur_ch1_cut -> Fill( dur_ch1 );
    h_amp_ch1_cut -> Fill( amp_ch1 );
    h_dur_ch2_cut -> Fill( dur_ch2 );
    h_amp_ch2_cut -> Fill( amp_ch2 );
    h_amp_ratio_cut -> Fill( amp_ch1/amp_ch2 );

    h_dt  -> Fill(dt);
    h2_dt_vs_amp_ch1-> Fill(amp_ch1, dt);
    h2_dt_vs_amp_ch2-> Fill(amp_ch2, dt);
    h2_dt_vs_amp_ratio-> Fill(amp_ch1/amp_ch2, dt);
    if ( fabs(dt) < maxDT  ) p_dt_vs_amp_ch1 -> Fill(amp_ch1, dt);
    if ( fabs(dt) < maxDT  ) p_dt_vs_amp_ch2 -> Fill(amp_ch2, dt);
    if ( fabs(dt) < maxDT  ) p_dt_vs_amp_ratio -> Fill(amp_ch1/amp_ch2, dt);

  }
  std::cout << std::endl;


  // -- Time walk corrections
  TF1* fitFunc_corr1 = new TF1("fitFunc_corr1","[0]*log([1]*x)+[2]",0.,1000.);
  fitFunc_corr1 -> SetParameters(0.05,0.0000001, p_dt_vs_amp_ch1 ->GetMinimum());
  fitFunc_corr1 -> SetLineColor(kRed);
  p_dt_vs_amp_ch1 -> Fit("fitFunc_corr1","QS+","",
  			  h_amp_ch1_cut->GetMean()-2.*h_amp_ch1_cut->GetRMS(),
  			  h_amp_ch1_cut->GetMean()+4.*h_amp_ch1_cut->GetRMS());  
			  
  TF1* fitFunc_corr2 = new TF1("fitFunc_corr2","[0]*log([1]*x)+[2]",0.,1000.);
  fitFunc_corr2 -> SetParameters(0.05,0.0000001,p_dt_vs_amp_ch2 ->GetMinimum());
  fitFunc_corr2 -> SetLineColor(kBlue);
  p_dt_vs_amp_ch2 -> Fit("fitFunc_corr2","QS+","",
  			  h_amp_ch2_cut->GetMean()-2.*h_amp_ch2_cut->GetRMS(),
  			  h_amp_ch2_cut->GetMean()+4.*h_amp_ch2_cut->GetRMS());

  TF1* fitFunc_corrRatio = new TF1("fitFunc_corrRatio","[0]*log([1]*x)+[2]",0.,1000.);
  fitFunc_corrRatio -> SetParameters(0.05,0.0000001,p_dt_vs_amp_ratio ->GetMinimum());
  fitFunc_corrRatio -> SetLineColor(kGreen);
  p_dt_vs_amp_ratio -> Fit("fitFunc_corrRatio","QS+","",
			  h_amp_ratio_cut->GetMean()-2.*h_amp_ratio_cut->GetRMS(),
			  h_amp_ratio_cut->GetMean()+4.*h_amp_ratio_cut->GetRMS());


  // second loop over events -- fill histograms after applying timewalk correction
  for(int entry = 0; entry < nEntries; ++entry)
    {
      if( entry%1000 == 0 ) std::cout << ">>> Reading entry " << entry << " / " << nEntries << "\r" << std::flush;
      chain1 -> GetEntry(entry);

      if( !AcceptEvent(treeVars) ) continue;

      float Vbias1 = treeVars.t_Vbias[VbiasIndex1];
      float amp_ch1 = treeVars.t_amp[8+ch1] * 0.25;
      float dur_ch1 = treeVars.t_dur[ch1] * 0.2;
      float time_ch1 = treeVars.t_time[ch1];

      float Vbias2 = treeVars.t_Vbias[VbiasIndex2];
      float amp_ch2 = treeVars.t_amp[8+ch2] * 0.25;
      float dur_ch2 = treeVars.t_dur[ch2] * 0.2;
      float time_ch2 = treeVars.t_time[ch2];

      // corr from fit fun
      float dt = (time_ch1 - time_ch2) - fitFunc_corrRatio->Eval(amp_ch1/amp_ch2) + fitFunc_corrRatio->Eval(h_amp_ratio_cut->GetMean()) ;  


      // --- Select events
      // -- amplitude and duration in ch1
      if( !AcceptEventAmp(treeVars,cut_ampMin1[Vbias1],cut_ampMax1[Vbias1], ch1 )) continue;
      if( !AcceptEventDur(treeVars,cut_durMin1[Vbias1],cut_durMax1[Vbias1], ch1 )) continue;
      if( !AcceptEventVbias(treeVars,cut_VbiasMin,cut_VbiasMax) ) continue;
      if( !AcceptEventTh(treeVars,cut_NINOthrMin,cut_NINOthrMax) ) continue;

      // -- amplitude and duration in ch2
      if( !AcceptEventAmp(treeVars,cut_ampMin2[Vbias2],cut_ampMax2[Vbias2], ch2 )) continue;
      if( !AcceptEventDur(treeVars,cut_durMin2[Vbias2],cut_durMax2[Vbias2], ch2 )) continue;
      //if( !AcceptEventVbias(treeVars,cut_VbiasMin,cut_VbiasMax) ) continue;
      if( !AcceptEventTh(treeVars,cut_NINOthrMin,cut_NINOthrMax) ) continue;

      h_dt_corr -> Fill(dt);
      if ( fabs(dt) < maxDT  )    h2_dtcorr_vs_amp_ch1 -> Fill(amp_ch1, dt);
      if ( fabs(dt) < maxDT  )    h2_dtcorr_vs_amp_ch2 -> Fill(amp_ch2, dt);
      if ( fabs(dt) < maxDT  )    h2_dtcorr_vs_amp_ratio -> Fill(amp_ch1/amp_ch2, dt);
      if ( fabs(dt) < maxDT  )    p_dtcorr_vs_amp_ch1 -> Fill(amp_ch1, dt);
      if ( fabs(dt) < maxDT  )    p_dtcorr_vs_amp_ch2 -> Fill(amp_ch2, dt);
      if ( fabs(dt) < maxDT  )    p_dtcorr_vs_amp_ratio -> Fill(amp_ch1/amp_ch2, dt);


    }

  // -- Save histograms on file
  TFile *fout = new TFile(OutputFile.c_str(),"recreate");
  h_dur_ch1 -> Write();
  h_amp_ch1 -> Write();
  h_dur_ch2 -> Write();
  h_amp_ch2 -> Write();
  h_amp_ratio -> Write();

  h_dur_ch1_cut -> Write();
  h_amp_ch1_cut -> Write();
  h_dur_ch2_cut -> Write();
  h_amp_ch2_cut -> Write();
  h_amp_ratio_cut -> Write();

  h_dt -> Write();
  h_dt_corr -> Write();

  h2_dt_vs_amp_ch1->Write();
  h2_dt_vs_amp_ch2->Write();
  h2_dt_vs_amp_ratio->Write();
  p_dt_vs_amp_ch1->Write();
  p_dt_vs_amp_ch2->Write();
  p_dt_vs_amp_ratio->Write();

  h2_dtcorr_vs_amp_ch1->Write();
  h2_dtcorr_vs_amp_ch2->Write();
  h2_dtcorr_vs_amp_ratio->Write();
  p_dtcorr_vs_amp_ch1->Write();
  p_dtcorr_vs_amp_ch2->Write();
  p_dtcorr_vs_amp_ratio->Write();

  h2_beam_Y_vs_X -> Write();
  h2_beam_Y_vs_X_cut -> Write();

  fout->Close();
  return 0;
}



bool AcceptEvent(TreeVars treeVars)
{
  //if( treeVars.t_batch != cut_batch ) return false;
  
  //if( !(treeVars.t_isOk[ch1] && treeVars.t_isOk[ch2]) ) return false;
  
  // if( fabs(treeVars.t_beamX+22.) > 5. ) return false;
  // if( fabs(treeVars.t_beamY-6.) > 5. ) return false;
  
  return true;
}

bool AcceptEventAmp(TreeVars treeVars, const float& ampMin, const float& ampMax, const int& ch)
{
  if( (treeVars.t_amp[8+ch]*0.25 < ampMin) || (treeVars.t_amp[8+ch]*0.25 > ampMax) ) return false;
  return true;
}

bool AcceptEventDur(TreeVars treeVars, const float& durMin, const float& durMax, const int& ch)
{
  if( (treeVars.t_dur[ch]*0.2 < durMin) || (treeVars.t_dur[ch]*0.2 > durMax) ) return false;
  return true;
}

bool AcceptEventAmpMCP(TreeVars treeVars, const float& ampMin, const float& ampMax, const int& mcp)
{
  if( (treeVars.t_amp[mcp] < ampMin) || (treeVars.t_amp[mcp] > ampMax) ) return false;
  return true;
}


bool AcceptEventTh(TreeVars treeVars, const float& thMin, const float& thMax)
{
  if( (treeVars.t_NINOthr < thMin) || (treeVars.t_NINOthr > thMax) ) return false;
  
  return true;
}

bool AcceptEventVbias(TreeVars treeVars, const float& VbiasMin, const float& VbiasMax)
{
  float Vbias1 = treeVars.t_Vbias[VbiasIndex1];
  if( (Vbias1 < VbiasMin) || (Vbias1 > VbiasMax) ) return false;
  return true;
}


void InitTreeVars(TTree* chain1, TTree* chain2, TTree* chain3,
                  TreeVars& treeVars)
{
  treeVars.t_Vbias = new float[3];
  treeVars.t_ped = new float[16];
  treeVars.t_amp = new float[16];
  treeVars.t_dur = new float[16];
  treeVars.t_time = new float[16];
  //treeVars.t_time_max = new float[16];
  //treeVars.t_time_maximum = new float[16];
  treeVars.t_isOk = new int[16];
  
  //chain1 -> SetBranchStatus("*",0);
  chain1 -> SetBranchStatus("NINOthr",1); chain1 -> SetBranchAddress("NINOthr",&treeVars.t_NINOthr);
  chain1 -> SetBranchStatus("Vbias1" ,1); chain1 -> SetBranchAddress("Vbias1", &treeVars.t_Vbias[0]);
  chain1 -> SetBranchStatus("Vbias2" ,1); chain1 -> SetBranchAddress("Vbias2", &treeVars.t_Vbias[1]);
  chain1 -> SetBranchStatus("Vbias3" ,1); chain1 -> SetBranchAddress("Vbias3", &treeVars.t_Vbias[2]);
  
  //chain2 -> SetBranchStatus("*",0);
  chain2 -> SetBranchStatus("X",1); chain2 -> SetBranchAddress("X",&treeVars.t_beamX);
  chain2 -> SetBranchStatus("Y",1); chain2 -> SetBranchAddress("Y",&treeVars.t_beamY);
  
  //chain3 -> SetBranchStatus("*",0);
  chain3 -> SetBranchStatus("n_channels",    1); chain3 -> SetBranchAddress("n_channels",   &treeVars.t_n_channels);
  chain3 -> SetBranchStatus("amp_max",       1); chain3 -> SetBranchAddress("amp_max",      treeVars.t_amp);
  chain3 -> SetBranchStatus("charge_sig",    1); chain3 -> SetBranchAddress("charge_sig",   treeVars.t_dur);
  chain3 -> SetBranchStatus("time",          1); chain3 -> SetBranchAddress("time",         treeVars.t_time);
  //chain3 -> SetBranchStatus("time_maximum",  1); chain3 -> SetBranchAddress("time_maximum", treeVars.t_time_maximum);
  //chain3 -> SetBranchStatus("time_max",      1); chain3 -> SetBranchAddress("time_max",     treeVars.t_time_max);
}
