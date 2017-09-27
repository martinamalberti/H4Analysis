#include "interface/FitUtils.h"
#include "interface/SetTDRStyle.h"

#include "CfgManager/interface/CfgManager.h"
#include "CfgManager/interface/CfgManagerT.h"

#include <iostream>
#include <vector>
#include <map>
#include <algorithm>
#include <cstdlib>

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
#include "TEfficiency.h"
#include "TF1.h"
#include "TCanvas.h"
#include "TLatex.h"
#include "TLine.h"
#include "TApplication.h"


bool popupPlots = true;

std::vector<std::string> channels;

/*** tree variables ***/
struct TreeVars
{
  unsigned int t_run;
  unsigned int t_spill;
  unsigned int t_event;
  float* t_beamX;
  float* t_beamY;
  float* t_amp;
  float* t_dur;
  float* t_time;
  std::map<std::string,int>* t_channelId;
};

void InitTreeVars(TTree* chain, TreeVars& treeVars);



int main(int argc, char** argv)
{
  gSystem -> Load("CfgManager/lib/libCfgManager.so");
  
  if( argc < 2 )
  {
    std::cerr << ">>>>> drawAlignmentPlots.cpp::usage:   " << argv[0] << " configFileName" << std::endl;
    return 1;
  }
  
  
  
  //----------------------
  // parse the config file
  CfgManager opts;
  opts.ParseConfigFile(argv[1]);
  
  std::vector<std::string> inputFiles = opts.GetOpt<std::vector<std::string> >("Input.inputFiles");
  channels = opts.GetOpt<std::vector<std::string> >("Input.channels");
  
  
  
  //---------------------------
  // open input files and trees
  TChain* chain0 = new TChain("h4","h4");
  for(unsigned int fileIt = 0; fileIt < inputFiles.size(); ++fileIt)
  {
    chain0 -> Add(inputFiles.at(fileIt).c_str());
  }
  //chain0 -> BuildIndex("index");
  std::cout << " Read " << chain0->GetEntries() << " total events in tree " << chain0->GetName() << std::endl;
  
  // set branches
  TreeVars treeVars;
  InitTreeVars(chain0,treeVars);
  
  
  //------------------
  // Define histograms
  std::map<std::string,TH1F*> h1_amp;
  std::map<std::string,TProfile2D*> p2_amp_Y_vs_X;
  TH1F* h1_beam_X = new TH1F("h1_beam_X","",100,-50.,50.);
  TH1F* h1_beam_Y = new TH1F("h1_beam_Y","",100,-50.,50.);
  TH2F* h2_beam_Y_vs_X = new TH2F("h2_beam_Y_vs_X","",100,-50.,50.,100,-50.,50.);
  
  
  //------------------------
  // labels and canvas style
  setTDRStyle();
  gStyle -> SetOptStat(1111);
  TApplication* theApp;
  if( popupPlots )
    theApp = new TApplication("App", &argc, argv);
  
  
  //-----------------------
  // first loop over events
  int nEntries = chain0 -> GetEntries();
  for(int entry = 0; entry < nEntries; ++entry)
  {
    if( entry%1 == 0 ) std::cout << ">>> loop 1/1: reading entry " << entry << " / " << nEntries << "\r" << std::flush;
    chain0 -> GetEntry(entry);
    
    h1_beam_X -> Fill(treeVars.t_beamX[0]);
    h1_beam_Y -> Fill(treeVars.t_beamY[0]);
    h2_beam_Y_vs_X -> Fill(treeVars.t_beamX[0],treeVars.t_beamY[0]);
    
     for(unsigned int it = 0; it < channels.size(); ++it)
     {
      std::string channelName = channels.at(it);
      
      float amp = treeVars.t_amp[(*treeVars.t_channelId)[channelName]] * 0.25;
      
      if(h1_amp[channelName] == NULL)
      {
        h1_amp[channelName] = new TH1F(Form("h1_%s",channelName.c_str()),"",1000,0.,1000.);
        p2_amp_Y_vs_X[channelName] = new TProfile2D(Form("p2_amp_%s_Y_vs_X",channelName.c_str()),"",100,-50.,50.,100,-50.,50.);
      }
      
      h1_amp[channelName] -> Fill( amp );
      p2_amp_Y_vs_X[channelName] -> Fill( treeVars.t_beamX[0],treeVars.t_beamY[0],amp );
    }
  }
  std::cout << std::endl;
  
  
  
  //-----------
  // draw plots
  TCanvas* c1_beam_Y_vs_X = new TCanvas("c1_beam_Y_vs_X","beam profile",1200,600);
  c1_beam_Y_vs_X -> Divide(3);
  c1_beam_Y_vs_X -> cd(1);
  h1_beam_X -> SetTitle(";wire chamber X [mm];entries");
  h1_beam_X -> Draw();
  c1_beam_Y_vs_X -> cd(2);
  h1_beam_Y -> SetTitle(";wire chamber Y [mm];entries");
  h1_beam_Y -> Draw();
  c1_beam_Y_vs_X -> cd(3);
  h2_beam_Y_vs_X -> SetTitle(";wire chamber X [mm];wire chamber Y [mm]");
  h2_beam_Y_vs_X -> Draw("colz");
  gPad -> Update();
  
  for(unsigned int it = 0; it < channels.size(); ++it)
  {
    std::string channelName = channels.at(it);
    
    TCanvas* c1_channel = new TCanvas(Form("c1_%s",channelName.c_str()),Form("c1_%s",channelName.c_str()),1200,600);
    c1_channel -> Divide(2,1);
    
    c1_channel -> cd(1);
    gPad -> SetLogy();
    h1_amp[channelName] -> SetTitle(";max. amplitude [mV];entries");
    h1_amp[channelName] -> Draw();
    gPad -> Update();
    
    c1_channel -> cd(2);
    p2_amp_Y_vs_X[channelName] -> SetTitle(";wire chamber X [mm];wire chamber Y [mm];#LT max. amplitude #LT [mV]");
    p2_amp_Y_vs_X[channelName] -> Draw("colz");
    gPad -> SetLogz();
    gPad -> Update();
  }
  
  if( popupPlots ) theApp -> Run();
  return 0;
}



void InitTreeVars(TTree* chain, TreeVars& treeVars)
{
  treeVars.t_amp = new float[15];
  treeVars.t_dur = new float[15];
  treeVars.t_time = new float[30];
  treeVars.t_beamX = new float[2];
  treeVars.t_beamY = new float[2];
  treeVars.t_channelId = new std::map<std::string,int>;
  
  // chain -> SetBranchStatus("*",0);
  
  chain -> SetBranchStatus("run",  1); chain -> SetBranchAddress("run",  &treeVars.t_run);
  chain -> SetBranchStatus("spill",1); chain -> SetBranchAddress("spill",&treeVars.t_spill);
  chain -> SetBranchStatus("event",1); chain -> SetBranchAddress("event",&treeVars.t_event);
  
  chain -> SetBranchStatus("X",1); chain -> SetBranchAddress("X",treeVars.t_beamX);
  chain -> SetBranchStatus("Y",1); chain -> SetBranchAddress("Y",treeVars.t_beamY);
  
  chain -> SetBranchStatus("amp_max",   1); chain -> SetBranchAddress("amp_max",   treeVars.t_amp);
  chain -> SetBranchStatus("charge_sig",1); chain -> SetBranchAddress("charge_sig",treeVars.t_dur);
  chain -> SetBranchStatus("time",      1); chain -> SetBranchAddress("time",      treeVars.t_time);
  for(unsigned int it = 0; it < channels.size(); ++it)
  {
    std::string channelName = channels.at(it);
    chain -> SetBranchStatus(channelName.c_str(),1); chain -> SetBranchAddress(channelName.c_str(),&((*treeVars.t_channelId)[channelName]));
  }
}
