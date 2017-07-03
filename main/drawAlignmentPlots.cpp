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
  float* t_beamX;
  float* t_beamY;
  float* t_Vbias;
  float t_tableX;
  float t_tableY;
  float t_NINOthr;
  float* t_ped;
  float* t_amp;
  float* t_dur;
  float* t_time;
  int* t_isOk;
  std::map<std::string,int>* t_channelId;
};

void InitTreeVars(TTree* chain1, TTree* chain2, TTree* chain3,
                  TreeVars& treeVars);



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
  
  
  
  //------------------------
  // labels and canvas style
  setTDRStyle();
  TApplication* theApp;
  if( popupPlots )
    theApp = new TApplication("App", &argc, argv);
  
  
  
  //---------------------------
  // open input files and trees
  TChain* chain1 = new TChain("info","info");
  TChain* chain2 = new TChain("hodo","hodo");
  TChain* chain3 = new TChain("digi","digi");
  for(unsigned int fileIt = 0; fileIt < inputFiles.size(); ++fileIt)
  {
    chain1 -> Add(inputFiles.at(fileIt).c_str());
    chain2 -> Add(inputFiles.at(fileIt).c_str());
    chain3 -> Add(inputFiles.at(fileIt).c_str());
  }
  chain2 -> BuildIndex("index");
  chain1 -> AddFriend("hodo");
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
  // Define histograms
  std::map<std::string,TH1F*> h1_amp;
  std::map<std::string,TEfficiency*> p1_eff_vs_X;
  std::map<std::string,TEfficiency*> p1_eff_vs_Y;
  std::map<std::string,TEfficiency*> p1_eff_Y_vs_X;
  TH1F* h1_beam_X = new TH1F("h1_beam_X","",200,-50.,50.);
  TH1F* h1_beam_Y = new TH1F("h1_beam_Y","",200,-50.,50.);
  TH2F* h2_beam_Y_vs_X = new TH2F("h2_beam_Y_vs_X","",200,-50.,50.,200,-50.,50.);
  
  
  //-----------------------
  // first loop over events
  int nEntries = chain1 -> GetEntries();
  for(int entry = 0; entry < nEntries; ++entry)
  {
    if( entry%1000 == 0 ) std::cout << ">>> loop 1/1: reading entry " << entry << " / " << nEntries << "\r" << std::flush;
    chain1 -> GetEntry(entry);
    
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
        p1_eff_vs_X[channelName] = new TEfficiency(Form("p1_eff_%s_vs_X",channelName.c_str()),"",160,200.,280.);
        p1_eff_vs_Y[channelName] = new TEfficiency(Form("p1_eff_%s_vs_Y",channelName.c_str()),"",160,120.,200.);
        p1_eff_Y_vs_X[channelName] = new TEfficiency(Form("p1_eff_%s_Y_vs_X",channelName.c_str()),"",160,200.,280.,160,120.,200.);
      }
      
      h1_amp[channelName] -> Fill( amp );
      if( amp > 200. )
      {
        p1_eff_vs_X[channelName] -> Fill( true,treeVars.t_tableX );
        p1_eff_vs_Y[channelName] -> Fill( true,treeVars.t_tableY );
        p1_eff_Y_vs_X[channelName] -> Fill( true,treeVars.t_tableX,treeVars.t_tableY );
      }
      else
      {
        p1_eff_vs_X[channelName] -> Fill( false,treeVars.t_tableX );
        p1_eff_vs_Y[channelName] -> Fill( false,treeVars.t_tableY );
        p1_eff_Y_vs_X[channelName] -> Fill( false,treeVars.t_tableX,treeVars.t_tableY );
      }
    }
  }
  std::cout << std::endl;
  
  
  
  //-----------
  // draw plots
  TCanvas* c1_beam_Y_vs_X = new TCanvas("c1_beam_Y_vs_X","beam profile",1200,600);
  c1_beam_Y_vs_X -> Divide(3);
  c1_beam_Y_vs_X -> cd(1);
  h1_beam_X -> SetTitle(";hodoscope X [mm];entries");
  h1_beam_X -> Draw();
  c1_beam_Y_vs_X -> cd(2);
  h1_beam_Y -> SetTitle(";hodoscope Y [mm];entries");
  h1_beam_Y -> Draw();
  c1_beam_Y_vs_X -> cd(3);
  h2_beam_Y_vs_X -> SetTitle(";hodoscope X [mm];hodoscope Y [mm]");
  h2_beam_Y_vs_X -> Draw("colz");
  gPad -> Update();
  
  TCanvas* c1_amp = new TCanvas("c1_amp","channel amplitudes",1200,600);
  c1_amp -> Divide(3,2);
  for(unsigned int it = 0; it < channels.size(); ++it)
  {
    std::string channelName = channels.at(it);
    
    c1_amp -> cd(it+1);
    gPad -> SetLogy();
    h1_amp[channelName] -> SetTitle(";max amplitude [mV];entries");
    h1_amp[channelName] -> Draw();
    gPad -> Update();
  }
  
  TCanvas* c1_eff_vs_X = new TCanvas("c1_eff_vs_X","channel efficiencies",1200,600);
  c1_eff_vs_X -> Divide(3,2);
  for(unsigned int it = 0; it < channels.size(); ++it)
  {
    std::string channelName = channels.at(it);
    
    c1_eff_vs_X -> cd(it+1);
    p1_eff_vs_X[channelName] -> SetTitle(";table X [mm];efficiency (amp > 200.)");
    p1_eff_vs_X[channelName] -> Draw();
    gPad -> Update();
  }
  
  TCanvas* c1_eff_vs_Y = new TCanvas("c1_eff_vs_Y","channel efficiencies",1200,600);
  c1_eff_vs_Y -> Divide(3,2);
  for(unsigned int it = 0; it < channels.size(); ++it)
  {
    std::string channelName = channels.at(it);
    
    c1_eff_vs_Y -> cd(it+1);
    p1_eff_vs_Y[channelName] -> SetTitle(";table Y [mm];efficiency (amp > 200.)");
    p1_eff_vs_Y[channelName] -> Draw();
    gPad -> Update();
  }
  
  TCanvas* c1_eff_Y_vs_X = new TCanvas("c1_eff_Y_vs_X","channel efficiencies",1200,600);
  c1_eff_Y_vs_X -> Divide(3,2);
  for(unsigned int it = 0; it < channels.size(); ++it)
  {
    std::string channelName = channels.at(it);
    
    c1_eff_Y_vs_X -> cd(it+1);
    p1_eff_Y_vs_X[channelName] -> SetTitle(";table X [mm];table Y [mm];efficiency (amp > 200.)");
    p1_eff_Y_vs_X[channelName] -> Draw("colz");
    gPad -> Update();
  }
  
  if( popupPlots ) theApp -> Run();
  return 0;
}



void InitTreeVars(TTree* chain1, TTree* chain2, TTree* chain3,
                  TreeVars& treeVars)
{
  treeVars.t_Vbias = new float[3];
  treeVars.t_ped = new float[16];
  treeVars.t_amp = new float[16];
  treeVars.t_dur = new float[16];
  treeVars.t_time = new float[16];
  treeVars.t_isOk = new int[16];
  treeVars.t_beamX = new float[2];
  treeVars.t_beamY = new float[2];
  treeVars.t_channelId = new std::map<std::string,int>;
  
  //chain1 -> SetBranchStatus("*",0);
  chain1 -> SetBranchStatus("NINOthr",1); chain1 -> SetBranchAddress("NINOthr",&treeVars.t_NINOthr);
  chain1 -> SetBranchStatus("Vbias1" ,1); chain1 -> SetBranchAddress("Vbias1", &treeVars.t_Vbias[0]);
  chain1 -> SetBranchStatus("Vbias2" ,1); chain1 -> SetBranchAddress("Vbias2", &treeVars.t_Vbias[1]);
  chain1 -> SetBranchStatus("Vbias3" ,1); chain1 -> SetBranchAddress("Vbias3", &treeVars.t_Vbias[2]);
  chain1 -> SetBranchStatus("tableX" ,1); chain1 -> SetBranchAddress("tableX", &treeVars.t_tableX);
  chain1 -> SetBranchStatus("tableY" ,1); chain1 -> SetBranchAddress("tableY", &treeVars.t_tableY);
  
  //chain2 -> SetBranchStatus("*",0);
  chain2 -> SetBranchStatus("X",1); chain2 -> SetBranchAddress("X",treeVars.t_beamX);
  chain2 -> SetBranchStatus("Y",1); chain2 -> SetBranchAddress("Y",treeVars.t_beamY);
  
  //chain3 -> SetBranchStatus("*",0);
  chain3 -> SetBranchStatus("amp_max",   1); chain3 -> SetBranchAddress("amp_max",   treeVars.t_amp);
  chain3 -> SetBranchStatus("charge_sig",1); chain3 -> SetBranchAddress("charge_sig",treeVars.t_dur);
  chain3 -> SetBranchStatus("time",      1); chain3 -> SetBranchAddress("time",      treeVars.t_time);
  for(unsigned int it = 0; it < channels.size(); ++it)
  {
    std::string channelName = channels.at(it);
    chain3 -> SetBranchStatus(channelName.c_str(),1); chain3 -> SetBranchAddress(channelName.c_str(),&((*treeVars.t_channelId)[channelName]));
  }
}
