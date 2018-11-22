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
  float t_digiConf;
  float t_NINOthr;
  float* t_amp;
  float* t_time;
  float* t_time_max;
  float* t_b_rms;
  int  t_CFD;
  int  t_LED;
  int  t_LED50;
  int  t_LED100;
  int  t_LED200;
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
  //timeChannels   = opts.GetOpt<std::vector<std::string> >("Input.timeChannels");
  ampChannels   = opts.GetOpt<std::vector<std::string> >("Input.ampChannels");
  std::string OutputFile = opts.GetOpt<std::string>("Input.OutputFile");

  std::vector<float> cut_ampMin = opts.GetOpt<std::vector<float> >("Cuts.ampMin");
  std::vector<float> cut_ampMax = opts.GetOpt<std::vector<float> >("Cuts.ampMax");
  std::map<std::string,float>  minAmplitude, maxAmplitude;
  for (unsigned int ich = 0; ich < cut_ampMin.size(); ich++){
    minAmplitude[ampChannels[ich]] = cut_ampMin[ich];
    maxAmplitude[ampChannels[ich]] = cut_ampMax[ich];
    std::cout << ich << "  " << ampChannels[ich].c_str() << "  ampMin = " << minAmplitude[ampChannels[ich]]  << "  ampMax = " <<maxAmplitude[ampChannels[ich]] <<std::endl;
  } 

  //  std::vector<float> cut_hodoXmin = opts.GetOpt<std::vector<float> >("Cuts.hodoXmin");
  //std::vector<float> cut_hodoXmax = opts.GetOpt<std::vector<float> >("Cuts.hodoXmax");
  //std::vector<float> cut_hodoYmin = opts.GetOpt<std::vector<float> >("Cuts.hodoYmin");
  //std::vector<float> cut_hodoYmax = opts.GetOpt<std::vector<float> >("Cuts.hodoYmax");


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
  const int NCHANNELS = ampChannels.size();

  std::map<std::string,TH1F*>  h_amp_nocuts;
  std::map<std::string,TH1F*>  h_amp;
  std::map<std::string,TProfile2D*>  p2_matrix_lightSharing;
  TProfile2D  *p2_matrix_averageAmp = new TProfile2D("p2_matrix_averageAmp","p2_matrix_averageAmp", 4, -0.5, 3.5, 4, -0.5, 3.5);


  std::string channelName;
  std::string channelNameA;


  for (int i = 0 ; i < NCHANNELS; i++){

    channelNameA = ampChannels.at(i); // es: ch = MCP1, NINOMAT1, etc...

    h_amp_nocuts[channelNameA]  = new TH1F(Form("h_amp_nocuts_%s", channelNameA.c_str()), Form("h_amp_nocuts_%s",channelNameA.c_str()), 10000, 0., 1.0);
    h_amp[channelNameA]  = new TH1F(Form("h_amp_%s", channelNameA.c_str()), Form("h_amp_%s",channelNameA.c_str()), 10000, 0., 1.0);
    p2_matrix_lightSharing[channelNameA]  = new TProfile2D(Form("p2_matrix_lightSharing_%s", channelNameA.c_str()), Form("p2_matrix_lightSharing_%s",channelNameA.c_str()), 4, -0.5, 3.5, 4, -0.5, 3.5);

  }



  float amp  = 0.;
  float k = 1./4096.; // factor to convert amplitudes in V

  std::map<std::string,std::vector<int>>  channelMapping;  
  channelMapping["AMPMAT11"] = {1,4};
  channelMapping["AMPMAT12"] = {2,4};
  channelMapping["AMPMAT16"] = {3,4};
  channelMapping["AMPMAT15"] = {4,4};

  channelMapping["AMPMAT3"] = {1,3};
  channelMapping["AMPMAT4"] = {2,3};
  channelMapping["AMPMAT8"] = {3,3};
  channelMapping["AMPMAT7"] = {4,3};

  channelMapping["AMPMAT10"] = {1,2};
  channelMapping["AMPMAT9"]  = {2,2};
  channelMapping["AMPMAT13"] = {3,2};
  channelMapping["AMPMAT14"] = {4,2};

  channelMapping["AMPMAT2"] = {1,1};
  channelMapping["AMPMAT1"] = {2,1};
  channelMapping["AMPMAT5"] = {3,1};
  channelMapping["AMPMAT6"] = {4,1};

  
  //-----------------------
  // first loop over events
  std::cout<<"First loop over events ..." <<std::endl;
  int nEntries = chain1 -> GetEntries();
  for(int entry = 0; entry < nEntries; ++entry)
    {
      if( entry%1000 == 0 ) std::cout << ">>> Reading entry " << entry << " / " << nEntries << "\r" << std::flush;
      chain1 -> GetEntry(entry);
      
      //--- loop over channels 
      for (int ich = 0; ich < NCHANNELS; ich++){
	
	//	channelName = timeChannels[ich];
	channelNameA = ampChannels[ich];
	
	// -- read amplitude and time for each channel
	amp  = treeVars.t_amp[(*treeVars.t_channelId)[channelNameA]] * k;
	
       	// -- fill histograms before any selection
	h_amp_nocuts[channelNameA] -> Fill(amp);
	
	// -- select events
	if ( amp < minAmplitude[channelNameA]  || amp > maxAmplitude[channelNameA]) continue;

	// -- veto mip in all other channels
	bool mipInOtherChannel = false;
	for (int ch = 0; ch < NCHANNELS; ch++){
	  std::string chname = ampChannels[ch];
	  float amp2 = treeVars.t_amp[(*treeVars.t_channelId)[chname]] * k;
	  if (channelNameA!=chname && amp2 > minAmplitude[chname]){
	    mipInOtherChannel = true;
	    break;
	  }
	}
	if (mipInOtherChannel) continue;

	// -- fill histograms
	h_amp[channelNameA]  -> Fill(amp);
	
	int binx = channelMapping[channelNameA][0];
	int biny = channelMapping[channelNameA][1];
	float xx = p2_matrix_averageAmp->GetXaxis()->GetBinCenter(binx);
	float yy = p2_matrix_averageAmp->GetYaxis()->GetBinCenter(biny);

	p2_matrix_averageAmp->Fill(xx,yy, amp);

	for (int jch = 0; jch < NCHANNELS; jch++){
	  std::string channelNameA2 = ampChannels[jch];
	  int binx2 = channelMapping[channelNameA2][0];
	  int biny2 = channelMapping[channelNameA2][1];
	  float xx2 = p2_matrix_lightSharing[channelNameA2]->GetXaxis()->GetBinCenter(binx2);
	  float yy2 = p2_matrix_lightSharing[channelNameA2]->GetYaxis()->GetBinCenter(biny2);
	  float amp2 = treeVars.t_amp[(*treeVars.t_channelId)[channelNameA2]] * k;
	  if (channelNameA2 == channelNameA){
	    p2_matrix_lightSharing[channelNameA] -> Fill(xx2, yy2, amp2);
	  }
	  if (channelNameA2 != channelNameA  && amp2 < minAmplitude[channelNameA2]){
	    p2_matrix_lightSharing[channelNameA] -> Fill(xx2, yy2, amp2);
	  }
	}

      } // -- end loop over channels
      
    }// -- end first loop over events
  


  for (int ich = 0; ich < NCHANNELS; ich++){
    std::string channelNameA = ampChannels[ich];
    float ampmax = p2_matrix_lightSharing[channelNameA] -> GetMaximum();
    p2_matrix_lightSharing[channelNameA] -> Scale(1./ampmax);
  }
  



  // -- Save histograms on file
  std::cout << "Saving histograms in file " << OutputFile.c_str() <<std::endl;
  
  TFile *outfile = new TFile(OutputFile.c_str(),"recreate");

  p2_matrix_averageAmp->Write();

  for (int ich = 0; ich < NCHANNELS; ich++){
    channelNameA = ampChannels[ich];
    h_amp_nocuts[channelNameA]->Write();
    h_amp[channelNameA]->Write();
    p2_matrix_lightSharing[channelNameA] ->Write();    
  }

  outfile->Close();
  return 0;

}



// ---- Initialize tree -------------------------------------------------------------------------------------
void InitTreeVars(TTree* chain1,TreeVars& treeVars){
  std::cout << "ciao" <<std::endl;
  
  treeVars.t_digiConf = 0;
  treeVars.t_NINOthr = 0;
  treeVars.t_amp = new float[36];
  treeVars.t_time_max = new float[36];
  treeVars.t_time = new float[360];
  treeVars.t_b_rms = new float[360];
  treeVars.t_CFD = 0;
  treeVars.t_LED = 0;
  treeVars.t_LED50 = 0;
  treeVars.t_LED100 = 0;
  treeVars.t_LED200 = 0;

  treeVars.t_channelId = new std::map<std::string,int>;
  treeVars.t_X = new float[2];
  treeVars.t_Y = new float[2];

  //chain1 -> SetBranchStatus("*",0);

  chain1 -> SetBranchStatus("digiConf",      1); chain1 -> SetBranchAddress("digiConf",     &treeVars.t_digiConf);

  chain1 -> SetBranchStatus("NINOthr",       1); chain1 -> SetBranchAddress("NINOthr",     &treeVars.t_NINOthr);

  chain1 -> SetBranchStatus("X",        1); chain1 -> SetBranchAddress("X",      treeVars.t_X);
  chain1 -> SetBranchStatus("Y",        1); chain1 -> SetBranchAddress("Y",      treeVars.t_Y);

  chain1 -> SetBranchStatus("amp_max",       1); chain1 -> SetBranchAddress("amp_max",      treeVars.t_amp);
  chain1 -> SetBranchStatus("time_max",      1); chain1 -> SetBranchAddress("time_max",     treeVars.t_time_max);
  chain1 -> SetBranchStatus("time",          1); chain1 -> SetBranchAddress("time",         treeVars.t_time);
  chain1 -> SetBranchStatus("b_rms",          1); chain1 -> SetBranchAddress("b_rms",         treeVars.t_b_rms);
  chain1 -> SetBranchStatus("CFD",           1); chain1 -> SetBranchAddress("CFD",         &treeVars.t_CFD);
  chain1 -> SetBranchStatus("LED",           1); chain1 -> SetBranchAddress("LED",         &treeVars.t_LED);
  chain1 -> SetBranchStatus("LED50",           1); chain1 -> SetBranchAddress("LED50",         &treeVars.t_LED50);
  chain1 -> SetBranchStatus("LED100",           1); chain1 -> SetBranchAddress("LED100",         &treeVars.t_LED100);
  chain1 -> SetBranchStatus("LED200",           1); chain1 -> SetBranchAddress("LED200",         &treeVars.t_LED200);

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

  /*
    if (x1 > -999. && x2 > -999.)
    pos = 0.5 * (x1 + x2);
  else if (x1 > -999.)
    pos = x1;
  else if (x2 > -999.)
    pos = x2;
  */

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
