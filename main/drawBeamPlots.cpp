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
  float* t_beamX_hodo;
  float* t_beamY_hodo;
  
  float* t_beamX_wire;
  float* t_beamY_wire;
};

void InitTreeVars(TTree* chain1, TTree* chain2, TreeVars& treeVars);

bool AcceptEventBeamX(TreeVars& treeVars);
bool AcceptEventBeamY(TreeVars& treeVars);



int main(int argc, char** argv)
{
  gSystem -> Load("CfgManager/lib/libCfgManager.so");
  
  if( argc < 2 )
  {
    std::cerr << ">>>>> drawBeamPlots.cpp::usage:   " << argv[0] << " configFileName" << std::endl;
    return 1;
  }
  
  
  
  //----------------------
  // parse the config file
  CfgManager opts;
  opts.ParseConfigFile(argv[1]);
  
  std::vector<std::string> inputFiles = opts.GetOpt<std::vector<std::string> >("Input.inputFiles");
  
  
  
  //------------------------
  // labels and canvas style
  setTDRStyle();
  TApplication* theApp;
  if( popupPlots )
    theApp = new TApplication("App", &argc, argv);
  
  
  
  //---------------------------
  // open input files and trees
  TChain* chain1 = new TChain("hodo","hodo");
  TChain* chain2 = new TChain("wire","wire");
  for(unsigned int fileIt = 0; fileIt < inputFiles.size(); ++fileIt)
  {
    chain1 -> Add(inputFiles.at(fileIt).c_str());
    chain2 -> Add(inputFiles.at(fileIt).c_str());
  }
  chain2 -> BuildIndex("index");
  chain1 -> AddFriend("wire");
  chain1 -> BuildIndex("index");
  std::cout << " Read " << chain1->GetEntries() << " total events in tree " << chain1->GetName() << std::endl;
  std::cout << " Read " << chain2->GetEntries() << " total events in tree " << chain2->GetName() << std::endl;
  
  // set branches
  TreeVars treeVars;
  InitTreeVars(chain1,chain2,treeVars);
  
  
  //------------------
  // Define histograms
  TH1F* h1_beam_X_hodo1 = new TH1F("h1_beam_X_hodo1","",80,-20.,20.);
  TH1F* h1_beam_X_hodo2 = new TH1F("h1_beam_X_hodo2","",80,-20.,20.);
  TH1F* h1_beam_X_wire  = new TH1F("h1_beam_X_wire", "",80,-20.,20.);
  
  TH1F* h1_beam_Y_hodo1 = new TH1F("h1_beam_Y_hodo1","",80,-20.,20.);
  TH1F* h1_beam_Y_hodo2 = new TH1F("h1_beam_Y_hodo2","",80,-20.,20.);
  TH1F* h1_beam_Y_wire  = new TH1F("h1_beam_Y_wire", "",100,-50.,50.);
  
  TH2F* h2_beam_Y_vs_X_hodo1 = new TH2F("h2_beam_Y_vs_X_hodo1","",80,-20.,20.,80,-20.,20.);
  TH2F* h2_beam_Y_vs_X_hodo2 = new TH2F("h2_beam_Y_vs_X_hodo2","",80,-20.,20.,80,-20.,20.);
  TH2F* h2_beam_Y_vs_X_wire  = new TH2F("h2_beam_Y_vs_X_wire", "",100,-50.,50.,100,-50.,50.);
  
  TH1F* h1_beam_X_hodo2_hodo1 = new TH1F("h1_beam_X_hodo2_hodo1","",160,-20.125,19.875);
  TH1F* h1_beam_Y_hodo2_hodo1 = new TH1F("h1_beam_Y_hodo2_hodo1","",160,-20.125,19.875);
  TH2F* h2_beam_X_hodo2_vs_hodo1 = new TH2F("h2_beam_X_hodo2_vs_hodo1","",80,-20.,20.,80,-20.,20.);
  TH2F* h2_beam_Y_hodo2_vs_hodo1 = new TH2F("h2_beam_Y_hodo2_vs_hodo1","",80,-20.,20.,80,-20.,20.);
  
  
  //-----------------------
  // first loop over events
  int nEntries = chain1 -> GetEntries();
  for(int entry = 0; entry < nEntries; ++entry)
  {
    if( entry%1000 == 0 ) std::cout << ">>> loop 1/1: reading entry " << entry << " / " << nEntries << "\r" << std::flush;
    chain1 -> GetEntry(entry);
    
    
    h1_beam_X_hodo1 -> Fill(treeVars.t_beamX_hodo[0]);
    h1_beam_Y_hodo1 -> Fill(treeVars.t_beamY_hodo[0]);
    h2_beam_Y_vs_X_hodo1 -> Fill(treeVars.t_beamX_hodo[0],treeVars.t_beamY_hodo[0]);
    
    h1_beam_X_hodo2 -> Fill(treeVars.t_beamX_hodo[1]);
    h1_beam_Y_hodo2 -> Fill(treeVars.t_beamY_hodo[1]);
    h2_beam_Y_vs_X_hodo2 -> Fill(treeVars.t_beamX_hodo[1],treeVars.t_beamY_hodo[1]);
    
    h1_beam_X_wire -> Fill(treeVars.t_beamX_wire[0]);
    h1_beam_Y_wire -> Fill(treeVars.t_beamY_wire[0]);
    h2_beam_Y_vs_X_wire -> Fill(treeVars.t_beamX_wire[0],treeVars.t_beamY_wire[0]);
    
    if( AcceptEventBeamX(treeVars) )
    {
      h1_beam_X_hodo2_hodo1 -> Fill( treeVars.t_beamX_hodo[1]-treeVars.t_beamX_hodo[0]  );
      h2_beam_X_hodo2_vs_hodo1 -> Fill( treeVars.t_beamX_hodo[0],treeVars.t_beamX_hodo[1] );
    }
    if( AcceptEventBeamY(treeVars) )
    {
      h1_beam_Y_hodo2_hodo1 -> Fill( treeVars.t_beamY_hodo[1]-treeVars.t_beamY_hodo[0]  );
      h2_beam_Y_hodo2_vs_hodo1 -> Fill( treeVars.t_beamY_hodo[0],treeVars.t_beamY_hodo[1] );
    }
  }
  std::cout << std::endl;
  
  
  
  //-----------
  // draw plots
  TCanvas* c1_beam_Y_vs_X_hodo1 = new TCanvas("c1_beam_Y_vs_X_hodo1","beam profile -- hodo1",1200,400);
  c1_beam_Y_vs_X_hodo1 -> Divide(3);
  c1_beam_Y_vs_X_hodo1 -> cd(1);
  h1_beam_X_hodo1 -> SetTitle(";hodoscope_{1} X [mm];entries");
  h1_beam_X_hodo1 -> Draw();
  c1_beam_Y_vs_X_hodo1 -> cd(2);
  h1_beam_Y_hodo1 -> SetTitle(";hodoscope_{1} Y [mm];entries");
  h1_beam_Y_hodo1 -> Draw();
  c1_beam_Y_vs_X_hodo1 -> cd(3);
  h2_beam_Y_vs_X_hodo1 -> SetTitle(";hodoscope_{1} X [mm];hodoscope_{1} Y [mm];entries");
  h2_beam_Y_vs_X_hodo1 -> Draw("colz");
  gPad -> Update();
  
  TCanvas* c1_beam_Y_vs_X_hodo2 = new TCanvas("c1_beam_Y_vs_X_hodo2","beam profile -- hodo2",1200,400);
  c1_beam_Y_vs_X_hodo2 -> Divide(3);
  c1_beam_Y_vs_X_hodo2 -> cd(1);
  h1_beam_X_hodo2 -> SetTitle(";hodoscope_{2} X [mm];entries");
  h1_beam_X_hodo2 -> Draw();
  c1_beam_Y_vs_X_hodo2 -> cd(2);
  h1_beam_Y_hodo2 -> SetTitle(";hodoscope_{2} Y [mm];entries");
  h1_beam_Y_hodo2 -> Draw();
  c1_beam_Y_vs_X_hodo2 -> cd(3);
  h2_beam_Y_vs_X_hodo2 -> SetTitle(";hodoscope_{2} X [mm];hodoscope_{2} Y [mm];entries");
  h2_beam_Y_vs_X_hodo2 -> Draw("colz");
  gPad -> Update();
  
  TCanvas* c1_beam_Y_vs_X_wire = new TCanvas("c1_beam_Y_vs_X_wire","beam profile -- wire",1200,400);
  c1_beam_Y_vs_X_wire -> Divide(3);
  c1_beam_Y_vs_X_wire -> cd(1);
  h1_beam_X_wire -> SetTitle(";wire chamber X [mm];entries");
  h1_beam_X_wire -> Draw();
  c1_beam_Y_vs_X_wire -> cd(2);
  h1_beam_Y_wire -> SetTitle(";wire chamber Y [mm];entries");
  h1_beam_Y_wire -> Draw();
  c1_beam_Y_vs_X_wire -> cd(3);
  h2_beam_Y_vs_X_wire -> SetTitle(";wire chamber X [mm];wire chamber Y [mm];entries");
  h2_beam_Y_vs_X_wire -> Draw("colz");
  gPad -> Update();
  
  TCanvas* c1_beam_hodo2_vs_hodo1 = new TCanvas("c1_beam_hodo2_vs_hodo1","hodo2 vs hodo1",800,400);
  c1_beam_hodo2_vs_hodo1 -> Divide(2);
  c1_beam_hodo2_vs_hodo1 -> cd(1);
  h2_beam_X_hodo2_vs_hodo1 -> SetTitle(";hodoscope_{1} X [mm];hodoscope_{2} X [mm]");
  h2_beam_X_hodo2_vs_hodo1 -> Draw("colz");
  c1_beam_hodo2_vs_hodo1 -> cd(2);
  h2_beam_Y_hodo2_vs_hodo1 -> SetTitle(";hodoscope_{1} Y [mm];hodoscope_{2} Y [mm]");
  h2_beam_Y_hodo2_vs_hodo1 -> Draw("colz");
  
  TCanvas* c1_beam_hodo2_hodo1 = new TCanvas("c1_beam_hodo2_hodo1","hodo2 vs hodo1",800,400);
  c1_beam_hodo2_hodo1 -> Divide(2);
  c1_beam_hodo2_hodo1 -> cd(1);
  h1_beam_X_hodo2_hodo1 -> SetTitle("; X_{hodo2} - X_{hodo1} [mm];events");
  h1_beam_X_hodo2_hodo1 -> Draw("colz");
  c1_beam_hodo2_hodo1 -> cd(2);
  h1_beam_Y_hodo2_hodo1 -> SetTitle("; Y_{hodo2} - Y_{hodo1} [mm];events");
  h1_beam_Y_hodo2_hodo1 -> Draw("colz");
  
  
  if( popupPlots ) theApp -> Run();
  return 0;
}



void InitTreeVars(TTree* chain1, TTree* chain2, TreeVars& treeVars)
{
  treeVars.t_beamX_hodo = new float[2];
  treeVars.t_beamY_hodo = new float[2];
  treeVars.t_beamX_wire = new float[2];
  treeVars.t_beamY_wire = new float[2];
  
  //chain1 -> SetBranchStatus("*",0);
  chain1 -> SetBranchStatus("X",1); chain1 -> SetBranchAddress("X",treeVars.t_beamX_hodo);
  chain1 -> SetBranchStatus("Y",1); chain1 -> SetBranchAddress("Y",treeVars.t_beamY_hodo);
  
  //chain2 -> SetBranchStatus("*",0);
  chain2 -> SetBranchStatus("X",1); chain2 -> SetBranchAddress("X",treeVars.t_beamX_wire);
  chain2 -> SetBranchStatus("Y",1); chain2 -> SetBranchAddress("Y",treeVars.t_beamY_wire);
}

bool AcceptEventBeamX(TreeVars& treeVars)
{
  if( treeVars.t_beamX_hodo[1] >= -15. && treeVars.t_beamX_hodo[1] < -14.5 ) return false;
  if( treeVars.t_beamX_hodo[1] >= 8.5  && treeVars.t_beamX_hodo[1] < 9.5 )   return false;
  return true;
}

bool AcceptEventBeamY(TreeVars& treeVars)
{
  if( treeVars.t_beamY_hodo[0] >= -16. && treeVars.t_beamY_hodo[0] < -15.5 ) return false;
  if( treeVars.t_beamY_hodo[1] >= -16. && treeVars.t_beamY_hodo[1] < -15.5 ) return false;
  return true;
}
