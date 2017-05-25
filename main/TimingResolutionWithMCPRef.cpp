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
int mcp1;
int mcp2;
int VbiasIndex1;

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
  mcp1 = opts.GetOpt<int>("Input.mcp1");
  mcp2 = opts.GetOpt<int>("Input.mcp2");
  VbiasIndex1 = opts.GetOpt<int>("Input.VbiasIndex1");
  int configuration = opts.GetOpt<int>("Input.configuration");
  int recoTimeTypeMCP = opts.GetOpt<int>("Input.recoTimeTypeMCP");
  std::string OutputFile = opts.GetOpt<std::string>("Input.OutputFile");

  float cut_NINOthrMin = opts.GetOpt<float>("Cuts.minThreshold");
  float cut_NINOthrMax = opts.GetOpt<float>("Cuts.maxThreshold");
  float cut_VbiasMin = opts.GetOpt<float>("Cuts.minVbias");
  float cut_VbiasMax = opts.GetOpt<float>("Cuts.maxVbias");  
  
  std::vector<float> cut_VbiasValues = opts.GetOpt<std::vector<float> >("Cuts.VbiasValues");
  std::vector<float> cut_minAmplitudes1 = opts.GetOpt<std::vector<float> >("Cuts.minAmplitudes1");
  std::vector<float> cut_maxAmplitudes1 = opts.GetOpt<std::vector<float> >("Cuts.maxAmplitudes1");
  std::map<float,float> cut_ampMin1;
  std::map<float,float> cut_ampMax1;
  for(unsigned int it = 0; it < cut_VbiasValues.size(); ++it)
  {
    cut_ampMin1[cut_VbiasValues.at(it)] = cut_minAmplitudes1.at(it);
    cut_ampMax1[cut_VbiasValues.at(it)] = cut_maxAmplitudes1.at(it);
  }
  
  std::vector<float> cut_minDurations1 = opts.GetOpt<std::vector<float> >("Cuts.minDurations1");
  std::vector<float> cut_maxDurations1 = opts.GetOpt<std::vector<float> >("Cuts.maxDurations1");
  std::map<float,float> cut_durMin1;
  std::map<float,float> cut_durMax1;
  for(unsigned int it = 0; it < cut_VbiasValues.size(); ++it)
  {
    cut_durMin1[cut_VbiasValues.at(it)] = cut_minDurations1.at(it);
    cut_durMax1[cut_VbiasValues.at(it)] = cut_maxDurations1.at(it);
  }
  

  float cut_ampMinMCP1 = opts.GetOpt<float>("Cuts.minAmplitudesMCP1");
  float cut_ampMaxMCP1 = opts.GetOpt<float>("Cuts.maxAmplitudesMCP1");

  float cut_ampMinMCP2 = opts.GetOpt<float>("Cuts.minAmplitudesMCP2");
  float cut_ampMaxMCP2 = opts.GetOpt<float>("Cuts.maxAmplitudesMCP2");

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
  TH1F* h_amp_mcp1 = new TH1F("h_amp_mcp1","",4000,0.,4000.);
  TH1F* h_amp_mcp2 = new TH1F("h_amp_mcp2","",4000,0.,4000.);
  TH1F* h_amp_ratio1 = new TH1F("h_amp_ratio1","",100,0.,5.);
  TH1F* h_amp_ratio2 = new TH1F("h_amp_ratio2","",100,0.,5.);
  TH1F* h_amp_mcpRatio = new TH1F("h_amp_mcpRatio","",100,0.,5.);

  TH1F* h_dur_ch1_cut = new TH1F("h_dur_ch1_cut","",1000,0.,500.);
  TH1F* h_amp_ch1_cut = new TH1F("h_amp_ch1_cut","",1000,0.,1000.);
  TH1F* h_amp_mcp1_cut = new TH1F("h_amp_mcp1_cut","",4000,0.,4000.);
  TH1F* h_amp_mcp2_cut = new TH1F("h_amp_mcp2_cut","",4000,0.,4000.);
  TH1F* h_amp_ratio1_cut = new TH1F("h_amp_ratio1_cut","",100,0.,5.);
  TH1F* h_amp_ratio2_cut = new TH1F("h_amp_ratio2_cut","",100,0.,5.);
  TH1F* h_amp_mcpRatio_cut = new TH1F("h_amp_mcpRatio_cut","",100,0.,5.);

  TH2F* h2_time_ch1_vs_amp_ch1 = new TH2F("h2_time_ch1_vs_amp_ch1","",100,0.,1000., 1000, 0., 200.);
  TH2F* h2_time_mcp1_vs_amp_mcp1 = new TH2F("h2_time_mcp1_vs_amp_mcp1","",400,0.,4000., 1000, 0., 200.);
  TH2F* h2_time_mcp2_vs_amp_mcp2 = new TH2F("h2_time_mcp2_vs_amp_mcp2","",400,0.,4000., 1000, 0., 200.);

  TH1F* h_dt_mcp = new TH1F("h_dt_mcp","",1000, -2., 2.);
  TH1F* h_dt_corr_mcp = new TH1F("h_dt_corr_mcp","",1000, -2., 2.);
  TH2F* h2_dt_vs_amp_mcp1 = new TH2F("h2_dt_vs_amp_mcp1","",400,0.,4000., 1000, -5., 5.);
  TH2F* h2_dt_vs_amp_mcp2 = new TH2F("h2_dt_vs_amp_mcp2","",400,0.,4000., 1000, -5., 5.);
  TH2F* h2_dt_vs_amp_mcpRatio = new TH2F("h2_dt_vs_amp_mcpRatio","",250,0.,5., 1000, -5., 5.);
  TProfile* p_dt_vs_amp_mcp1 = new TProfile("p_dt_vs_amp_mcp1","",100,0.,4000.);
  TProfile* p_dt_vs_amp_mcp2 = new TProfile("p_dt_vs_amp_mcp2","",100,0.,4000.);
  TProfile* p_dt_vs_amp_mcpRatio = new TProfile("p_dt_vs_amp_mcpRatio","",250,0.,5.);


  TH1F* h_dt1 = new TH1F("h_dt1","",1000, -2., 2.);
  TH1F* h_dt1_corr = new TH1F("h_dt1_corr","",1000, -2., 2.);
  TH1F* h_dt2 = new TH1F("h_dt2","",1000, -2., 2.);
  TH1F* h_dt2_corr = new TH1F("h_dt2_corr","",1000, -2., 2.);

  TH2F* h2_dt1_vs_amp_ch1 = new TH2F("h2_dt1_vs_amp_ch1","",100,0.,1000., 1000, -5., 5.);
  TH2F* h2_dt2_vs_amp_ch1 = new TH2F("h2_dt2_vs_amp_ch1","",100,0.,1000., 1000, -5., 5.);
  TH2F* h2_dt1_vs_amp_mcp1 = new TH2F("h2_dt1_vs_amp_mcp1","",400,0.,4000., 1000, -5., 5.);
  TH2F* h2_dt2_vs_amp_mcp2 = new TH2F("h2_dt2_vs_amp_mcp2","",400,0.,4000., 1000, -5., 5.);
  TH2F* h2_dt1_vs_amp_ratio1 = new TH2F("h2_dt1_vs_amp_ratio1","",500,0.,5., 1000, -5., 5.);
  TH2F* h2_dt2_vs_amp_ratio2 = new TH2F("h2_dt2_vs_amp_ratio2","",500,0.,5., 1000, -5., 5.);

  TProfile* p_dt1_vs_amp_ch1 = new TProfile("p_dt1_vs_amp_ch1","",100,0.,1000.);
  TProfile* p_dt2_vs_amp_ch1 = new TProfile("p_dt2_vs_amp_ch1","",100,0.,1000.);
  TProfile* p_dt1_vs_amp_mcp1 = new TProfile("p_dt1_vs_amp_mcp1","",100,0.,4000.);
  TProfile* p_dt2_vs_amp_mcp2 = new TProfile("p_dt2_vs_amp_mcp2","",100,0.,4000.);
  TProfile* p_dt1_vs_amp_ratio1 = new TProfile("p_dt1_vs_amp_ratio1","",100,0.,5.);
  TProfile* p_dt2_vs_amp_ratio2 = new TProfile("p_dt2_vs_amp_ratio2","",100,0.,5.);


  TH2F* h2_dt1corr_vs_amp_ch1 = new TH2F("h2_dt1corr_vs_amp_ch1","",100,0.,1000., 1000, -5., 5.);
  TH2F* h2_dt2corr_vs_amp_ch1 = new TH2F("h2_dt2corr_vs_amp_ch1","",100,0.,1000., 1000, -5., 5.);
  TH2F* h2_dt1corr_vs_amp_mcp1 = new TH2F("h2_dt1corr_vs_amp_mcp1","",400,0.,4000., 1000, -5., 5.);
  TH2F* h2_dt2corr_vs_amp_mcp2 = new TH2F("h2_dt2corr_vs_amp_mcp2","",400,0.,4000., 1000, -5., 5.);
  TH2F* h2_dt1corr_vs_amp_ratio1 = new TH2F("h2_dt1corr_vs_amp_ratio1","",500,0.,5., 1000, -5., 5.);
  TH2F* h2_dt2corr_vs_amp_ratio2 = new TH2F("h2_dt2corr_vs_amp_ratio2","",500,0.,5., 1000, -5., 5.);

  TProfile* p_dt1corr_vs_amp_ch1 = new TProfile("p_dt1corr_vs_amp_ch1","",100,0.,1000.);
  TProfile* p_dt2corr_vs_amp_ch1 = new TProfile("p_dt2corr_vs_amp_ch1","",100,0.,1000.);
  TProfile* p_dt1corr_vs_amp_mcp1 = new TProfile("p_dt1corr_vs_amp_mcp1","",100,0.,4000.);
  TProfile* p_dt2corr_vs_amp_mcp2 = new TProfile("p_dt2corr_vs_amp_mcp2","",100,0.,4000.);
  TProfile* p_dt1corr_vs_amp_ratio1 = new TProfile("p_dt1corr_vs_amp_ratio1","",100,0.,5.);
  TProfile* p_dt2corr_vs_amp_ratio2 = new TProfile("p_dt2corr_vs_amp_ratio2","",100,0.,5.);
  
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

    float amp_mcp1  = treeVars.t_amp[mcp1];
    float time_mcp1 = treeVars.t_time[mcp1 + recoTimeTypeMCP * treeVars.t_n_channels]; // for MCPs: recoTimeTypeMCP = 1 (CFD)

    float amp_mcp2  = treeVars.t_amp[mcp2];
    float time_mcp2 = treeVars.t_time[mcp2 + recoTimeTypeMCP * treeVars.t_n_channels];// for MCPs: recoTimeTypeMCP = 1 (CFD)

    float dt1 = time_ch1 - time_mcp1;
    float dt2 = time_ch1 - time_mcp2;
    float dt  = time_mcp1 - time_mcp2;
    
    // --- Fill histograms before selections
    h2_beam_Y_vs_X -> Fill(treeVars.t_beamX,treeVars.t_beamY);

    h_dur_ch1 -> Fill( dur_ch1 );
    h_amp_ch1 -> Fill( amp_ch1 );
    h_amp_mcp1 -> Fill( amp_mcp1 );
    h_amp_mcp2 -> Fill( amp_mcp2 );
    h_amp_ratio1 -> Fill( amp_ch1/amp_mcp1 );
    h_amp_ratio2 -> Fill( amp_ch1/amp_mcp2 );
    h_amp_mcpRatio -> Fill( amp_mcp1/amp_mcp2 );


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

    // -- amplitude in mcp1 and mcp2
    if( !AcceptEventAmpMCP(treeVars,cut_ampMinMCP1,cut_ampMaxMCP1, mcp1 )) continue;
    if( !AcceptEventAmpMCP(treeVars,cut_ampMinMCP2,cut_ampMaxMCP2, mcp2 )) continue;

    //if (fabs(dt1) > 5 || fabs(dt2) > 5 ){
    //  std::cout << "ch1  = " << ch1 << "  time = " << time_ch1 << "   amp = " << amp_ch1 << std::endl;
    //  std::cout << "mcp1 = " << mcp1 <<  "  time = " << time_mcp1 << "   amp = " << amp_mcp1 << std::endl;
    //  std::cout << "mcp2 = " << mcp2 <<  "  time = " << time_mcp2 << "   amp = " << amp_mcp2 << std::endl;
    //}

    // Fill histograms after selections
    h2_beam_Y_vs_X_cut -> Fill(treeVars.t_beamX,treeVars.t_beamY);
  
    h_dur_ch1_cut -> Fill( dur_ch1 );
    h_amp_ch1_cut -> Fill( amp_ch1 );
    h_amp_mcp1_cut -> Fill( amp_mcp1 );
    h_amp_mcp2_cut -> Fill( amp_mcp2 );
    h_amp_ratio1_cut -> Fill( amp_ch1/amp_mcp1 );
    h_amp_ratio2_cut -> Fill( amp_ch1/amp_mcp2 );
    h_amp_mcpRatio_cut -> Fill( amp_mcp1/amp_mcp2 );

    h2_time_ch1_vs_amp_ch1 ->Fill(amp_ch1, time_ch1);
    h2_time_mcp1_vs_amp_mcp1 ->Fill(amp_mcp1, time_mcp1);
    h2_time_mcp2_vs_amp_mcp2 ->Fill(amp_mcp2, time_mcp2);

    h_dt1 -> Fill(dt1);
    h_dt2 -> Fill(dt2);
    h_dt_mcp -> Fill(dt);
    h2_dt_vs_amp_mcp1-> Fill(amp_mcp1, dt);
    h2_dt_vs_amp_mcp2-> Fill(amp_mcp2, dt);
    h2_dt_vs_amp_mcpRatio-> Fill(amp_mcp1/amp_mcp2, dt);
    if ( fabs(dt) < maxDT  ) p_dt_vs_amp_mcp1 -> Fill(amp_mcp1, dt);
    if ( fabs(dt) < maxDT  ) p_dt_vs_amp_mcp2 -> Fill(amp_mcp2, dt);
    if ( fabs(dt) < maxDT  ) p_dt_vs_amp_mcpRatio -> Fill(amp_mcp1/amp_mcp2, dt);

    if ( fabs(dt1) < maxDT ) h2_dt1_vs_amp_ch1 -> Fill(amp_ch1, dt1);
    if ( fabs(dt2) < maxDT ) h2_dt2_vs_amp_ch1 -> Fill(amp_ch1, dt2);
    if ( fabs(dt1) < maxDT ) h2_dt1_vs_amp_mcp1 -> Fill(amp_mcp1, dt1);
    if ( fabs(dt2) < maxDT ) h2_dt2_vs_amp_mcp2 -> Fill(amp_mcp2, dt2);
    if ( fabs(dt1) < maxDT ) h2_dt1_vs_amp_ratio1 -> Fill(amp_ch1/amp_mcp1, dt1);
    if ( fabs(dt2) < maxDT ) h2_dt2_vs_amp_ratio2 -> Fill(amp_ch1/amp_mcp2, dt2);

    if ( fabs(dt1) < maxDT ) p_dt1_vs_amp_ch1 -> Fill(amp_ch1, dt1);
    if ( fabs(dt2) < maxDT ) p_dt2_vs_amp_ch1 -> Fill(amp_ch1, dt2);
    if ( fabs(dt1) < maxDT ) p_dt1_vs_amp_mcp1 -> Fill(amp_mcp1, dt1);
    if ( fabs(dt2) < maxDT ) p_dt2_vs_amp_mcp2 -> Fill(amp_mcp2, dt2);
    if ( fabs(dt1) < maxDT ) p_dt1_vs_amp_ratio1 -> Fill(amp_ch1/amp_mcp1, dt1);
    if ( fabs(dt2) < maxDT ) p_dt2_vs_amp_ratio2 -> Fill(amp_ch1/amp_mcp2, dt2);

  }
  std::cout << std::endl;


  // -- Time walk corrections
  TF1* fitFunc_corr1 = new TF1("fitFunc_corr1","[0]*log([1]*x)+[2]",0.,1000.);
  fitFunc_corr1 -> SetParameters(0.05,0.0000001, p_dt1_vs_amp_ch1 ->GetMinimum());
  fitFunc_corr1 -> SetLineColor(kRed);
  p_dt1_vs_amp_ch1 -> Fit("fitFunc_corr1","QS+","",
  			  h_amp_ch1_cut->GetMean()-2.*h_amp_ch1_cut->GetRMS(),
  			  h_amp_ch1_cut->GetMean()+4.*h_amp_ch1_cut->GetRMS());  

  TF1* fitFunc_corr2 = new TF1("fitFunc_corr2","[0]*log([1]*x)+[2]",0.,1000.);
  fitFunc_corr2 -> SetParameters(0.05,0.0000001,p_dt2_vs_amp_ch1 ->GetMinimum());
  fitFunc_corr2 -> SetLineColor(kBlue);
  p_dt2_vs_amp_ch1 -> Fit("fitFunc_corr2","QS+","",
  			  h_amp_ch1_cut->GetMean()-2.*h_amp_ch1_cut->GetRMS(),
  			  h_amp_ch1_cut->GetMean()+4.*h_amp_ch1_cut->GetRMS());


  TF1* fitFunc_corrRatio1 = new TF1("fitFunc_corrRatio1","[0]*log([1]*x)+[2]",0.,1000.);
  fitFunc_corrRatio1 -> SetParameters(0.05,0.0000001,p_dt1_vs_amp_ratio1 ->GetMinimum());
  fitFunc_corrRatio1 -> SetLineColor(kRed);
  p_dt1_vs_amp_ratio1 -> Fit("fitFunc_corrRatio1","QS+","",
			  h_amp_ratio1_cut->GetMean()-2.*h_amp_ratio1_cut->GetRMS(),
			  h_amp_ratio1_cut->GetMean()+4.*h_amp_ratio1_cut->GetRMS());

  TF1* fitFunc_corrRatio2 = new TF1("fitFunc_corrRatio2","[0]*log([1]*x)+[2]",0.,1000.);
  fitFunc_corrRatio2 -> SetParameters(0.05,0.0000001,p_dt2_vs_amp_ratio2 ->GetMinimum());
  fitFunc_corrRatio2 -> SetLineColor(kBlue);
  p_dt2_vs_amp_ratio2 -> Fit("fitFunc_corrRatio2","QS+","",
			  h_amp_ratio2_cut->GetMean()-2.*h_amp_ratio2_cut->GetRMS(),
			  h_amp_ratio2_cut->GetMean()+4.*h_amp_ratio2_cut->GetRMS());

  TF1* fitFunc_corrRatioMCP = new TF1("fitFunc_corrRatioMCP","[0]*log([1]*x)+[2]",0.,1000.);
  fitFunc_corrRatioMCP -> SetParameters(0.05,0.0000001,0.);
  fitFunc_corrRatioMCP-> SetLineColor(kGreen);
  p_dt_vs_amp_mcpRatio-> Fit("fitFunc_corrRatioMCP","QS+","",
			 h_amp_mcpRatio_cut->GetMean()-2.*h_amp_mcpRatio_cut->GetRMS(),
			 h_amp_mcpRatio_cut->GetMean()+4.*h_amp_mcpRatio_cut->GetRMS());
  


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

      float amp_mcp1  = treeVars.t_amp[mcp1];
      float time_mcp1 = treeVars.t_time[mcp1 + recoTimeTypeMCP * treeVars.t_n_channels];

      float amp_mcp2  = treeVars.t_amp[mcp2];
      float time_mcp2 = treeVars.t_time[mcp2 + recoTimeTypeMCP * treeVars.t_n_channels];

      //float dt1 = (time_ch1 - time_mcp1) - fitFunc_corrRatio1->Eval(amp_ch1/amp_mcp1) + fitFunc_corrRatio1->Eval(h_amp_ratio1_cut->GetMean()) ;
      //float dt2 = (time_ch1 - time_mcp2) - fitFunc_corrRatio2->Eval(amp_ch1/amp_mcp2) + fitFunc_corrRatio2->Eval(h_amp_ratio2_cut->GetMean()) ;

      // -- corr MCP time resolution from fitted function
      //float dt1 = (time_ch1 - time_mcp1) - fitFunc_corr1->Eval(amp_ch1) + fitFunc_corr1->Eval(h_amp_ch1_cut->GetMean()) ;
      //float dt2 = (time_ch1 - time_mcp2) - fitFunc_corr2->Eval(amp_ch1) + fitFunc_corr2->Eval(h_amp_ch1_cut->GetMean()) ;
      float dt = (time_mcp1 - time_mcp2) - fitFunc_corrRatioMCP->Eval(amp_mcp1/amp_mcp2) + fitFunc_corrRatioMCP->Eval(h_amp_mcpRatio_cut->GetMean()) ;  

      // -- corr from TProfile
      int bin1 = p_dt1_vs_amp_ch1->FindBin(amp_ch1);
      int bin2 = p_dt2_vs_amp_ch1->FindBin(amp_ch1);
      int binMean1 = p_dt1_vs_amp_ch1->FindBin( h_amp_ch1_cut->GetMean() );
      int binMean2 = p_dt2_vs_amp_ch1->FindBin( h_amp_ch1_cut->GetMean() );
      float dt1 = (time_ch1 - time_mcp1) - p_dt1_vs_amp_ch1->GetBinContent(bin1) + p_dt1_vs_amp_ch1->GetBinContent(binMean1);
      float dt2 = (time_ch1 - time_mcp2) - p_dt2_vs_amp_ch1->GetBinContent(bin2) + p_dt2_vs_amp_ch1->GetBinContent(binMean2);

      // --- Select events
      // -- amplitude and duration in ch1
      if( !AcceptEventAmp(treeVars,cut_ampMin1[Vbias1],cut_ampMax1[Vbias1], ch1 )) continue;
      if( !AcceptEventDur(treeVars,cut_durMin1[Vbias1],cut_durMax1[Vbias1], ch1 )) continue;
      if( !AcceptEventVbias(treeVars,cut_VbiasMin,cut_VbiasMax) ) continue;
      if( !AcceptEventTh(treeVars,cut_NINOthrMin,cut_NINOthrMax) ) continue;

      // -- amplitude in mcp1 and mcp2
      if( !AcceptEventAmpMCP(treeVars,cut_ampMinMCP1,cut_ampMaxMCP1, mcp1 )) continue;
      if( !AcceptEventAmpMCP(treeVars,cut_ampMinMCP2,cut_ampMaxMCP2, mcp2 )) continue;

      h_dt_corr_mcp -> Fill(dt);
      h_dt1_corr -> Fill(dt1);
      h_dt2_corr -> Fill(dt2);

      if ( fabs(dt1) < maxDT  )    h2_dt1corr_vs_amp_ch1 -> Fill(amp_ch1, dt1);
      if ( fabs(dt2) < maxDT  )    h2_dt2corr_vs_amp_ch1 -> Fill(amp_ch1, dt2);
      if ( fabs(dt1) < maxDT  )    h2_dt1corr_vs_amp_mcp1 -> Fill(amp_mcp1, dt1);
      if ( fabs(dt2) < maxDT  )    h2_dt2corr_vs_amp_mcp2 -> Fill(amp_mcp2, dt2);
      if ( fabs(dt1) < maxDT  )    h2_dt1corr_vs_amp_ratio1 -> Fill(amp_ch1/amp_mcp1, dt1);
      if ( fabs(dt2) < maxDT  )    h2_dt2corr_vs_amp_ratio2 -> Fill(amp_ch1/amp_mcp2, dt2);

      if ( fabs(dt1) < maxDT  ) p_dt1corr_vs_amp_ch1 -> Fill(amp_ch1, dt1);
      if ( fabs(dt2) < maxDT  ) p_dt2corr_vs_amp_ch1 -> Fill(amp_ch1, dt2);
      if ( fabs(dt1) < maxDT  ) p_dt1corr_vs_amp_mcp1 -> Fill(amp_mcp1, dt1);
      if ( fabs(dt2) < maxDT  ) p_dt2corr_vs_amp_mcp2 -> Fill(amp_mcp2, dt2);
      if ( fabs(dt1) < maxDT  ) p_dt1corr_vs_amp_ratio1 -> Fill(amp_ch1/amp_mcp1, dt1);
      if ( fabs(dt2) < maxDT  ) p_dt2corr_vs_amp_ratio2 -> Fill(amp_ch1/amp_mcp2, dt2);

    }

  // -- Save histograms on file
  TFile *outfile = new TFile(OutputFile.c_str(),"recreate");
  
  TDirectory *outdir = outfile->mkdir(label1.c_str());
  outdir->cd();
  
  h_dur_ch1 -> Write();
  h_amp_ch1 -> Write();
  h_amp_mcp1 -> Write();
  h_amp_mcp2 -> Write();
  h_amp_ratio1 -> Write();
  h_amp_ratio2 -> Write();
  h_amp_mcpRatio -> Write();

  h_dur_ch1_cut -> Write();
  h_amp_ch1_cut -> Write();
  h_amp_mcp1_cut -> Write();
  h_amp_mcp2_cut -> Write();
  h_amp_ratio1_cut -> Write();
  h_amp_ratio2_cut -> Write();
  h_amp_mcpRatio_cut -> Write();

  h2_time_ch1_vs_amp_ch1 -> Write();
  h2_time_mcp1_vs_amp_mcp1 -> Write();
  h2_time_mcp2_vs_amp_mcp2 -> Write();

  h_dt1 -> Write();
  h_dt2 -> Write();
  h_dt1_corr -> Write();
  h_dt2_corr -> Write();
  h_dt_mcp -> Write();
  h_dt_corr_mcp->Write();

  h2_dt_vs_amp_mcp1->Write();
  h2_dt_vs_amp_mcp2->Write();
  h2_dt_vs_amp_mcpRatio->Write();
  p_dt_vs_amp_mcp1 -> Write();
  p_dt_vs_amp_mcp2 -> Write();
  p_dt_vs_amp_mcpRatio -> Write();

  h2_dt1_vs_amp_ch1 -> Write();
  h2_dt2_vs_amp_ch1 -> Write();
  h2_dt1_vs_amp_mcp1 -> Write();
  h2_dt2_vs_amp_mcp2 -> Write();
  h2_dt1_vs_amp_ratio1 -> Write();
  h2_dt2_vs_amp_ratio2 -> Write();

  p_dt1_vs_amp_ch1 -> Write();
  p_dt2_vs_amp_ch1 -> Write();
  p_dt1_vs_amp_mcp1 -> Write();
  p_dt2_vs_amp_mcp2 -> Write();
  p_dt1_vs_amp_ratio1 -> Write();
  p_dt2_vs_amp_ratio2 -> Write();

  h2_dt1corr_vs_amp_ch1 -> Write();
  h2_dt2corr_vs_amp_ch1 -> Write();
  h2_dt1corr_vs_amp_mcp1 -> Write();
  h2_dt2corr_vs_amp_mcp2 -> Write();
  h2_dt1corr_vs_amp_ratio1 -> Write();
  h2_dt2corr_vs_amp_ratio2 -> Write();

  p_dt1corr_vs_amp_ch1 -> Write();
  p_dt2corr_vs_amp_ch1 -> Write();
  p_dt1corr_vs_amp_mcp1 -> Write();
  p_dt2corr_vs_amp_mcp2 -> Write();
  p_dt1corr_vs_amp_ratio1 -> Write();
  p_dt2corr_vs_amp_ratio2 -> Write();

  h2_beam_Y_vs_X -> Write();
  h2_beam_Y_vs_X_cut -> Write();

  outfile->Close();
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
