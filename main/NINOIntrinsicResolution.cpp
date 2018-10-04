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
#include <stdlib.h>

#include "TSystem.h"
#include "TFile.h"
#include "TTree.h"
#include "TChain.h"
#include "TGraph.h"
#include "TGraphErrors.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TProfile.h"
#include "TProfile2D.h"
#include "TF1.h"
#include "TCanvas.h"
#include "TLatex.h"
#include "TLine.h"
#include "TApplication.h"


std::vector<std::string> timeChannels;
std::vector<std::string> ampChannels;


/*** tree variables ***/
struct TreeVars
{
  float t_NINOthr;
  float* t_amp;
  float* t_time;
  float* t_time_max;
  float* t_b_rms;
  int  t_CFD;
  int  t_LED;
  float* t_X;
  float* t_Y;

  std::map<std::string,int> *t_channelId;

};
/**********************/




void InitTreeVars(TTree* chain1, TreeVars& treeVars);
float GetPosition(float x1, float x2);
int GetIndex(float val, std::vector<float> vals);

// ====== *** MAIN *** ======
int main(int argc, char** argv)
{
  gSystem -> Load("CfgManager/lib/libCFGMan.so");
  
  if( argc < 2 )
    {
      std::cerr << ">>>>> TimingAnalysis.cpp::usage:   " << argv[0] << " configFileName" << std::endl;
      return 1;
    }
  
  //----------------------
  // parse the config file
  CfgManager opts;
  opts.ParseConfigFile(argv[1]);
  
  std::vector<float> runs = opts.GetOpt<std::vector<float> >("Input.runList");
  timeChannels   = opts.GetOpt<std::vector<std::string> >("Input.timeChannels");
  ampChannels   = opts.GetOpt<std::vector<std::string> >("Input.ampChannels");
  std::string OutputFile = opts.GetOpt<std::string>("Input.OutputFile");

  float ninoThr  = opts.GetOpt<float>("Cuts.ninoThreshold");
  std::vector<float> cut_ampMin = opts.GetOpt<std::vector<float> >("Cuts.ampMin");
  std::vector<float> cut_ampMax = opts.GetOpt<std::vector<float> >("Cuts.ampMax");
  std::map<std::string,float>  minAmplitude, maxAmplitude;
  for (unsigned int ich = 0; ich < cut_ampMin.size(); ich++){
    minAmplitude[ampChannels[ich]] = cut_ampMin[ich];
    maxAmplitude[ampChannels[ich]] = cut_ampMax[ich];
    std::cout << ich << "  " << ampChannels[ich].c_str() << "  ampMin = " << minAmplitude[ampChannels[ich]]  << "  ampMax = " <<maxAmplitude[ampChannels[ich]] <<std::endl;
  } 

  std::vector<float> cut_hodoXmin = opts.GetOpt<std::vector<float> >("Cuts.hodoXmin");
  std::vector<float> cut_hodoXmax = opts.GetOpt<std::vector<float> >("Cuts.hodoXmax");
  std::vector<float> cut_hodoYmin = opts.GetOpt<std::vector<float> >("Cuts.hodoYmin");
  std::vector<float> cut_hodoYmax = opts.GetOpt<std::vector<float> >("Cuts.hodoYmax");


  std::cout << " Setting TDRStyle..."<<std::endl;
  //--------------------------
  // labels and canvas style
  setTDRStyle();

  
  //---------------------------
  // open input files and trees
  std::cout << "Loading trees ..." << std::endl; 
  TChain* chain1 = new TChain("h4","h4");
  for (unsigned int runIt =0; runIt < runs.size(); ++runIt){
    std::string fileName = "/eos/cms/store/group/dpg_mtd/comm_mtd/TB/MTDTB_H4_Sep2018/ntuples_v2/"+ std::to_string(int(runs[runIt]))+".root";
    std::cout << fileName << std::endl;
    chain1 -> Add(fileName.c_str());
  }
  std::cout << " Read " << chain1->GetEntries() << " total events in tree " << chain1->GetName() << std::endl;
  

  //--------------------------
  // set branches
  TreeVars treeVars;
  InitTreeVars(chain1,treeVars);
  
  
  //--------------------------
  // Book histograms
  const int NCHANNELS = timeChannels.size();

  std::map<std::string,TH1F*>  h_amp_nocuts;
  std::map<std::string,TH1F*>  h_amp;
  std::map<std::string,TH1F*>  h_time;
  std::map<std::string,TH1F*>  h_dt;
 
  std::string channelName;
  std::string channelNameA;

  float dtmin = -2;
  float dtmax = 2;

  for (int i = 0 ; i < NCHANNELS; i++){

    channelName = timeChannels.at(i); // es: ch = MCP1, NINOMAT1, etc...

    h_amp_nocuts[channelName]  = new TH1F(Form("h_amp_nocuts_%s", channelName.c_str()), Form("h_amp_nocuts_%s",channelName.c_str()), 1000, 0., 1.0);
    h_amp[channelName]  = new TH1F(Form("h_amp_%s", channelName.c_str()), Form("h_amp_%s",channelName.c_str()), 1000, 0., 1.0);
    h_time[channelName] = new TH1F(Form("h_time_%s",channelName.c_str()),Form("h_time_%s",channelName.c_str()),200,0.,200.);
    h_dt[channelName]   = new TH1F(Form("h_dt_%s",channelName.c_str()),Form("h_dt_%s",channelName.c_str()),8000, dtmin, dtmax);
  }



  float amp  = 0.;
  float time = 0.;
  float ampRef = 0;
  float timeRef = 0;
  float dt   = 0.;
  float posX = 0;
  float posY = 0;
  
  float k = 1./4096.; // factor to convert amplitudes in V
  float tMaxCh  = 40;
  float tMaxRef = 35;
  float bRmsMax = 200;

  //-----------------------
  // first loop over events
  std::cout<<"First loop over events ..." <<std::endl;
  int nEntries = chain1 -> GetEntries();
  for(int entry = 0; entry < nEntries; ++entry)
    {
      if( entry%1000 == 0 ) std::cout << ">>> Reading entry " << entry << " / " << nEntries << "\r" << std::flush;
      chain1 -> GetEntry(entry);

      // -- position
      posX = GetPosition(treeVars.t_X[0], treeVars.t_X[1]);
      posY = GetPosition(treeVars.t_Y[0], treeVars.t_Y[1]);
      if (posX == -999. || posY == -999.) continue;
      
      //--- loop over (NINO) channels 
      for (int ich = 0; ich < NCHANNELS; ich++){
	
	channelName = timeChannels[ich];
	channelNameA = ampChannels[ich];

	// -- read amplitude and time for each channel
	amp  = treeVars.t_amp[(*treeVars.t_channelId)[channelNameA]] * k;
        time = treeVars.t_time[(*treeVars.t_channelId)[channelName]+treeVars.t_LED];
	ampRef = amp;

	if (channelName == "NINO2") 
	  timeRef = treeVars.t_time[(*treeVars.t_channelId)["NINOINT2"]+treeVars.t_CFD]; 
	if (channelName == "NINO3") 
	  timeRef = treeVars.t_time[(*treeVars.t_channelId)["NINOINT3"]+treeVars.t_CFD]; 

	if (channelName == "NINOINT2") 
	  timeRef = treeVars.t_time[(*treeVars.t_channelId)["NINO2"]+treeVars.t_CFD]; 
	if (channelName == "NINOINT3") 
	  timeRef = treeVars.t_time[(*treeVars.t_channelId)["NINO3"]+treeVars.t_CFD]; 


       	// -- fill histograms before any selection
	h_amp_nocuts[channelName] -> Fill(amp);

	// -- select events
	if ( amp < minAmplitude[channelNameA]  || amp > maxAmplitude[channelNameA]) continue;
	if ( time < 0 || time > 200) continue;
	if ( timeRef < 0 || timeRef > 200) continue;
	//if ( treeVars.t_b_rms[(*treeVars.t_channelId)["NINOINT2"]] > bRmsMax) continue; // cleaning reco failures... 
	//if ( treeVars.t_b_rms[(*treeVars.t_channelId)["NINOINT3"]] > bRmsMax) continue; // cleaning reco failures... 

	dt   = time - timeRef; 
	  
	// -- remove events with bad reco time (second peak in the time distribution)
	if ( time    > tMaxCh  ) continue;
	if ( timeRef > tMaxRef ) continue;	

	// -- keep events on the crystal
	if ( posX < cut_hodoXmin[ich] || posX > cut_hodoXmax[ich] ) continue;
	if ( posY < cut_hodoYmin[ich] || posY > cut_hodoYmax[ich] ) continue;

	// -- fill histograms
	h_time[channelName] -> Fill(time);
	h_amp[channelName]  -> Fill(amp);
	h_dt[channelName]->Fill(dt);
      } // -- end loop over channels
      
    }// -- end first loop over events
  
  
  // -- Compute resolutions (gaussian and effective sigma)
  std::map<std::string,TF1*>  fitFun;

  float tmin = 0;
  float tmax = 0;
  double sigmaEff;
  float* vals = new float[6];

  std::map<std::string,TH1F*>  h_effectiveSigma;

  for (int ich = 0; ich < NCHANNELS; ich++){
    channelName = timeChannels[ich];
    h_effectiveSigma[channelName]  = new TH1F(Form("h_effectiveSigma_%s",channelName.c_str()),Form("h_effectiveSigma_%s",channelName.c_str()),2000,0.,200.);
  }

  int nRe = 4;
  for (int ich = 0; ich < NCHANNELS; ich++){

    channelName = timeChannels[ich];  

    // -- get the effective (68%) sigma
    FindSmallestInterval(vals,h_dt[channelName],0.68,true); 
    sigmaEff = 0.5*(vals[5]-vals[4]); 
    h_effectiveSigma[channelName]->Fill(sigmaEff);
    h_dt[channelName]->Rebin(nRe);    

    // -- get the gaussian sigma
    fitFun[channelName]= new TF1(Form("fitFun_%s",channelName.c_str()),"gaus");
    fitFun[channelName]->SetLineColor(1);
    fitFun[channelName]->SetLineWidth(1);
    fitFun[channelName]->SetParameter(1,h_dt[channelName]->GetMean());
    fitFun[channelName]->SetParameter(2,h_dt[channelName]->GetRMS());
    tmin = h_dt[channelName]->GetMean()-1.5*sigmaEff;
    tmax = h_dt[channelName]->GetMean()+1.5*sigmaEff;
    h_dt[channelName]->Fit(Form("fitFun_%s",channelName.c_str()),"QRS","", tmin, tmax);
    
  }




  // -- Save histograms on file
  std::cout << "Saving histograms in file " << OutputFile.c_str() <<std::endl;
  
  TFile *outfile = new TFile(OutputFile.c_str(),"recreate");
  
  for (int ich = 0; ich < NCHANNELS; ich++){
    channelName = timeChannels[ich];
    std::cout << channelName <<std::endl;

    h_amp_nocuts[channelName]->Write();
    h_amp[channelName]->Write();
    h_time[channelName]->Write();
    h_dt[channelName]->Write();
    h_effectiveSigma[channelName]->Write();
        
  }

  outfile->Close();
  return 0;

}



// ---- Initialize tree -------------------------------------------------------------------------------------
void InitTreeVars(TTree* chain1,TreeVars& treeVars){
  std::cout << "ciao" <<std::endl;
  
  treeVars.t_NINOthr = 0;
  treeVars.t_amp = new float[36];
  treeVars.t_time_max = new float[36];
  treeVars.t_time = new float[360];
  treeVars.t_b_rms = new float[360];
  treeVars.t_CFD = 0;
  treeVars.t_LED = 0;
  treeVars.t_channelId = new std::map<std::string,int>;
  treeVars.t_X = new float[2];
  treeVars.t_Y = new float[2];

  //chain1 -> SetBranchStatus("*",0);

  chain1 -> SetBranchStatus("NINOthr",       1); chain1 -> SetBranchAddress("NINOthr",     &treeVars.t_NINOthr);

  chain1 -> SetBranchStatus("X",        1); chain1 -> SetBranchAddress("X",      treeVars.t_X);
  chain1 -> SetBranchStatus("Y",        1); chain1 -> SetBranchAddress("Y",      treeVars.t_Y);

  chain1 -> SetBranchStatus("amp_max",       1); chain1 -> SetBranchAddress("amp_max",      treeVars.t_amp);
  chain1 -> SetBranchStatus("time_max",      1); chain1 -> SetBranchAddress("time_max",     treeVars.t_time_max);
  chain1 -> SetBranchStatus("time",          1); chain1 -> SetBranchAddress("time",         treeVars.t_time);
  chain1 -> SetBranchStatus("b_rms",          1); chain1 -> SetBranchAddress("b_rms",         treeVars.t_b_rms);
  chain1 -> SetBranchStatus("CFD",           1); chain1 -> SetBranchAddress("CFD",         &treeVars.t_CFD);
  chain1 -> SetBranchStatus("LED",           1); chain1 -> SetBranchAddress("LED",         &treeVars.t_LED);

  for(unsigned int it = 0; it < timeChannels.size(); ++it)
    {
      std::string channelName = timeChannels.at(it);
      chain1 -> SetBranchStatus(channelName.c_str(),1); chain1 -> SetBranchAddress(channelName.c_str(),&((*treeVars.t_channelId)[channelName]));
    }

  for(unsigned int it = 0; it < ampChannels.size(); ++it)
    {
      std::string channelName = ampChannels.at(it);
      chain1 -> SetBranchStatus(channelName.c_str(),1); chain1 -> SetBranchAddress(channelName.c_str(),&((*treeVars.t_channelId)[channelName]));
    }

}



// -- position from hodoscopes ------------------------------------
float GetPosition(float x1, float x2) {
  
  float pos = -999.;

  if (x1 > -999. && x2 > -999.)
    pos = 0.5 * (x1 + x2);
  else if (x1 > -999.)
    pos = x1;
  else if (x2 > -999.)
    pos = x2;


  pos = x2;

  return pos;
  
}


// -- --------------------------------------------------------------
int GetIndex(float val, std::vector<float> vals){
  
  int index = -1;
  auto Itr = std::find(vals.begin(), vals.end(), val);
  if(Itr != vals.end()) {
    index =  std::distance( vals.begin(), Itr) ;
  }
  return index;
  
}

