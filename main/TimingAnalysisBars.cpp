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
#include "TRandom.h"

#include "interface/TrackTree.h"

#include <math.h> 

std::string timeChannelRef;
std::string ampChannelRef;

std::vector<std::string> timeChannelsL;
std::vector<std::string> ampChannelsL;

std::vector<std::string> timeChannelsR;
std::vector<std::string> ampChannelsR;


/*** tree variables ***/
struct TreeVars
{
  string *t_digiConf;
  string *t_sensorConf;
  float t_NINOthr;
  float* t_amp;
  float* t_time;
  float* t_time_maximum;
  float* t_b_rms;
  int  t_CFD;
  int  t_LED;
  int  t_TED;

  std::vector<TrackPar> *t_trackFitResult;
  int t_ntracks;

  std::map<std::string,int> *t_channelId;

};
/**********************/

void InitTreeVars(TTree* chain1, TreeVars& treeVars);
void GetTimeResolution( TH1F *ht, float &sEff, float &sGaus, TF1* fitFun);

// ====== *** MAIN *** ======
int main(int argc, char** argv)
{
  gSystem -> Load("CfgManager/lib/libCFGMan.so");
  gSystem -> Load("lib/libH4Analysis.so");

  
  if( argc < 2 )
    {
      std::cerr << ">>>>> TimingAnalysis.cpp::usage:   " << argv[0] << " configFileName" << std::endl;
      return 1;
    }
  
  //----------------------
  // parse the config file
  CfgManager opts;
  opts.ParseConfigFile(argv[1]);
  
  //std::vector<float> runs = opts.GetOpt<std::vector<float> >("Input.runList");
  float runMin = opts.GetOpt<float>("Input.runMin");
  float runMax = opts.GetOpt<float>("Input.runMax");

  timeChannelRef = opts.GetOpt<std::string>("Input.timeChannelRef");
  ampChannelRef  = opts.GetOpt<std::string>("Input.ampChannelRef");

  timeChannelsL  = opts.GetOpt<std::vector<std::string> >("Input.timeChannelsL");
  ampChannelsL   = opts.GetOpt<std::vector<std::string> >("Input.ampChannelsL");

  timeChannelsR  = opts.GetOpt<std::vector<std::string> >("Input.timeChannelsR");
  ampChannelsR   = opts.GetOpt<std::vector<std::string> >("Input.ampChannelsR");

  std::string OutputFile = opts.GetOpt<std::string>("Input.OutputFile");

  float ninoThr  = opts.GetOpt<float>("Cuts.ninoThreshold");

  float cut_ampMinRef = opts.GetOpt<float>("Cuts.ampMinRef");
  float cut_ampMaxRef = opts.GetOpt<float>("Cuts.ampMaxRef");
  
  std::vector<float> cut_ampMinL = opts.GetOpt<std::vector<float> >("Cuts.ampMinL");
  std::vector<float> cut_ampMaxL = opts.GetOpt<std::vector<float> >("Cuts.ampMaxL");

  std::vector<float> cut_ampMinR = opts.GetOpt<std::vector<float> >("Cuts.ampMinR");
  std::vector<float> cut_ampMaxR = opts.GetOpt<std::vector<float> >("Cuts.ampMaxR");

  std::vector<float> cut_Xmin = opts.GetOpt<std::vector<float> >("Cuts.Xmin");
  std::vector<float> cut_Xmax = opts.GetOpt<std::vector<float> >("Cuts.Xmax");
  std::vector<float> cut_Ymin = opts.GetOpt<std::vector<float> >("Cuts.Ymin");
  std::vector<float> cut_Ymax = opts.GetOpt<std::vector<float> >("Cuts.Ymax");


  std::cout << " Setting TDRStyle..."<<std::endl;
  //--------------------------
  // labels and canvas style
  setTDRStyle();

  
  //---------------------------
  // open input files and trees
  std::cout << "Loading trees ..." << std::endl; 
  TChain* chain1 = new TChain("h4","h4");
  for (unsigned int iRun = runMin; iRun < runMax+1; iRun++){
    std::string fileName = "/eos/cms/store/group/dpg_mtd/comm_mtd/TB/MTDTB_FNAL_Nov2018/ntuples/v3/"+ std::to_string(int(iRun))+".root";
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
  const int NBARS = 2;
  int NBINSX[NBARS] = {0, 0};

  float xbinWidth = 1 ; // mm
  if (ninoThr!=400) xbinWidth = 2 ;
  for (int iBar = 0; iBar < NBARS; iBar++){
    NBINSX[iBar] = ceil ((cut_Xmax[iBar]-cut_Xmin[iBar])/xbinWidth) ;
  }
  
  float theta[NBARS];
  theta[0] = 0;
  theta[1] = 0; 
  if (runMin >= 1361 && runMax < 1382 ){
    theta[0] = 0.;
    theta[1] = 80.*TMath::Pi()/180.;
  }
  else if (runMin >= 1389 && runMax < 1412 ){
    theta[0] = 60.*TMath::Pi()/180.;
    theta[1] = 45.*TMath::Pi()/180.;
  }
   
  float dtminL[NBARS] = {-5., -5.};
  float dtmaxL[NBARS] = {-1., -1.};
  float dtminR[NBARS] = {-3., -5.};
  float dtmaxR[NBARS] = { 1., -1.};
  float dtminAve[NBARS] = {-5., -5.};
  float dtmaxAve[NBARS] = {-1., -1.};

  if (timeChannelRef=="PTK2_0"){
    dtminL[0] = -11.;
    dtminL[1] = -11.;
    dtmaxL[0] = -7.;
    dtmaxL[1] = -7.;
    dtminR[0] = -9.;
    dtminR[1] = -11.;
    dtmaxR[0] = -5.;
    dtmaxR[1] = -7.;
    dtminAve[0] = -11.;
    dtminAve[1] = -11.;
    dtmaxAve[0] = -7.;
    dtmaxAve[1] = -7.;
  }


  float xmin = -50.;
  float xmax = 50.;

  float ymin = 0.;
  float ymax= 50.;


  // -- ref histos
  TH1F *h_ampRef_nocuts = new TH1F("h_ampRef_nocuts", "h_ampRef_nocuts", 1000, 0., 1.);
  TH1F *h_ampRef        = new TH1F("h_ampRef", "h_ampRef", 1000, 0., 1.);
  TH1F *h_timeRef       = new TH1F("h_timeRef", "h_timeRef", 200, 0., 200.);;
  TProfile2D*  p2_eff_vs_posXY_Ref = new TProfile2D("p2_eff_vs_posXY_Ref","p2_eff_vs_posXY_Ref", 200, xmin, xmax, 200, ymin, ymax, 0, 1);
  TProfile*    p_eff_vs_posX_Ref   = new TProfile("p_eff_vs_posX_Ref","p_eff_vs_posX_Ref", 200, xmin, xmax, 0, 1);
  TProfile*    p_eff_vs_posY_Ref   = new TProfile("p_eff_vs_posY_Ref","p_eff_vs_posY_Ref", 200, ymin, ymax, 0, 1);
  TProfile*    p_timeRef_vs_ampRef = new TProfile("p_timeRef_vs_ampRef","p_timeRef_vs_ampRef", 100, 0, 1, 0, 200);
  TProfile2D*  p2_ampRef_vs_posXY  = new TProfile2D("p2_ampRef_vs_posXY","p2_ampRef_vs_posXY", 200, xmin, xmax, 200, ymin, ymax, 0, 1);
  TProfile2D*  p2_timeRef_vs_posXY = new TProfile2D("p2_timeRef_vs_posXY","p2_timeRef_vs_posXY", 200, xmin, xmax, 200, ymin, ymax, 0, 200);
  TProfile*    p_ampRef_vs_posX    = new TProfile("p_ampRef_vs_posX","p_ampRef_vs_posX", 100, -50, 50, 0, 1);
  TProfile*    p_ampRef_vs_posY    = new TProfile("p_ampRef_vs_posY","p_ampRef_vs_posY", 100, -50, 50, 0, 1);



  // -- control plots
  TProfile2D*  p2_eff_vs_posXY[NBARS];
  TProfile*    p_eff_vs_posX[NBARS];
  TProfile*    p_eff_vs_posY[NBARS];

  TH1F*  h_ampL_nocuts[NBARS];
  TH1F*  h_ampL[NBARS];
  TH1F*  h_timeL[NBARS];
  TH1F*  h_totL[NBARS];
  TH1F*  h_dtimeMaximumL[NBARS];

  TProfile2D*  p2_ampL_vs_posXY[NBARS];
  TProfile*    p_ampL_vs_posX[NBARS];
  TProfile*    p_ampL_vs_posXc[NBARS];
  TProfile*    p_ampL_vs_posY[NBARS];
  TProfile*    p_ampL_vs_tDiff[NBARS];

  TH1F*  h_ampR_nocuts[NBARS];
  TH1F*  h_ampR[NBARS];
  TH1F*  h_timeR[NBARS];
  TH1F*  h_totR[NBARS];
  TH1F*  h_dtimeMaximumR[NBARS];

  TProfile2D*  p2_ampR_vs_posXY[NBARS];
  TProfile*    p_ampR_vs_posX[NBARS];
  TProfile*    p_ampR_vs_posXc[NBARS];
  TProfile*    p_ampR_vs_posY[NBARS];
  TProfile*    p_ampR_vs_tDiff[NBARS];

  TH2F* h2_tDiff_vs_posX[NBARS];


  // -- left
  TProfile2D*    p2_tL_vs_posXY[NBARS];

  TH1F*          h_tL[NBARS];
  TProfile*      p_tL_vs_amp[NBARS];
  TProfile*      p_tL_vs_posX[NBARS];
  TProfile*      p_tL_vs_posY[NBARS];
  TProfile*      p_tL_vs_tDiff[NBARS];

  TH1F*          h_tL_ampCorr[NBARS];
  TProfile*      p_tL_ampCorr_vs_amp[NBARS];
  TProfile*      p_tL_ampCorr_vs_posX[NBARS];
  TProfile*      p_tL_ampCorr_vs_posY[NBARS];
  TProfile*      p_tL_ampCorr_vs_tDiff[NBARS];
  TH1F*          h_tL_ampCorr_binX[NBARS][50];
  TProfile*      p_tL_ampCorr_vs_posXc[NBARS];

  // -- right   
  TProfile2D*    p2_tR_vs_posXY[NBARS];

  TH1F*          h_tR[NBARS];
  TProfile*      p_tR_vs_amp[NBARS];
  TProfile*      p_tR_vs_posX[NBARS];
  TProfile*      p_tR_vs_posY[NBARS];
  TProfile*      p_tR_vs_tDiff[NBARS];

  TH1F*          h_tR_ampCorr[NBARS];
  TProfile*      p_tR_ampCorr_vs_amp[NBARS];
  TProfile*      p_tR_ampCorr_vs_posX[NBARS];
  TProfile*      p_tR_ampCorr_vs_posY[NBARS];
  TProfile*      p_tR_ampCorr_vs_tDiff[NBARS];
  TH1F*          h_tR_ampCorr_binX[NBARS][50];
  TProfile*      p_tR_ampCorr_vs_posXc[NBARS];

  // -- simple average
  TProfile2D*    p2_tAve_vs_posXY[NBARS];

  TH1F*          h_tAve[NBARS];
  TProfile*      p_tAve_vs_posX[NBARS];
  TProfile*      p_tAve_vs_posY[NBARS];
  TProfile*      p_tAve_vs_tDiff[NBARS];

  TH1F*          h_tAve_ampCorr[NBARS];
  TProfile*      p_tAve_ampCorr_vs_posX[NBARS];
  TProfile*      p_tAve_ampCorr_vs_posY[NBARS];
  TProfile*      p_tAve_ampCorr_vs_tDiff[NBARS];
  TH1F*          h_tAve_ampCorr_binX[NBARS][50];
  TProfile*      p_tAve_ampCorr_vs_posXc[NBARS];

  TH1F*          h_tAve_ampCorr_tDiffCorr[NBARS];
  TProfile*      p_tAve_ampCorr_tDiffCorr_vs_posX[NBARS];
  TProfile*      p_tAve_ampCorr_tDiffCorr_vs_posY[NBARS];
  TProfile*      p_tAve_ampCorr_tDiffCorr_vs_tDiff[NBARS];
  TH1F*          h_tAve_ampCorr_tDiffCorr_binX[NBARS][50];
  TProfile*      p_tAve_ampCorr_tDiffCorr_vs_posXc[NBARS];

  TH1F*          h_tAve_ampCorr_posCorr[NBARS];
  TProfile*      p_tAve_ampCorr_posCorr_vs_posX[NBARS];
  TProfile*      p_tAve_ampCorr_posCorr_vs_posY[NBARS];
  TProfile*      p_tAve_ampCorr_posCorr_vs_tDiff[NBARS];
  TProfile*      p_tAve_ampCorr_posCorr_vs_posXc[NBARS];

  // -- amp weighted average
  TH1F*          h_tAveAmpW_ampCorr[NBARS];
  TProfile*      p_tAveAmpW_ampCorr_vs_posX[NBARS];
  TProfile*      p_tAveAmpW_ampCorr_vs_posY[NBARS];
  TProfile*      p_tAveAmpW_ampCorr_vs_tDiff[NBARS];
  TH1F*          h_tAveAmpW_ampCorr_binX[NBARS][50];  
  TProfile*      p_tAveAmpW_ampCorr_vs_posXc[NBARS];

  TH1F*          h_tAveAmpW_ampCorr_tDiffCorr[NBARS];
  TProfile*      p_tAveAmpW_ampCorr_tDiffCorr_vs_posX[NBARS];
  TProfile*      p_tAveAmpW_ampCorr_tDiffCorr_vs_posY[NBARS];
  TProfile*      p_tAveAmpW_ampCorr_tDiffCorr_vs_tDiff[NBARS];
  TH1F*          h_tAveAmpW_ampCorr_tDiffCorr_binX[NBARS][50];  
  TProfile*      p_tAveAmpW_ampCorr_tDiffCorr_vs_posXc[NBARS];

  // -- tRes weighted average
  TH1F*          h_tAveResW_ampCorr[NBARS];
  TProfile*      p_tAveResW_ampCorr_vs_posX[NBARS];
  TProfile*      p_tAveResW_ampCorr_vs_posY[NBARS];
  TProfile*      p_tAveResW_ampCorr_vs_tDiff[NBARS];
  TH1F*          h_tAveResW_ampCorr_binX[NBARS][50];  
  TProfile*      p_tAveResW_ampCorr_vs_posXc[NBARS];

  TH1F*          h_tAveResW_ampCorr_tDiffCorr[NBARS];
  TProfile*      p_tAveResW_ampCorr_tDiffCorr_vs_posX[NBARS];
  TProfile*      p_tAveResW_ampCorr_tDiffCorr_vs_posY[NBARS];
  TProfile*      p_tAveResW_ampCorr_tDiffCorr_vs_tDiff[NBARS];
  TH1F*          h_tAveResW_ampCorr_tDiffCorr_binX[NBARS][50];  
  TProfile*      p_tAveResW_ampCorr_tDiffCorr_vs_posXc[NBARS];


  int nbinsHistoX = 500;
  //if (ninoThr != 400){
  //  nbinsHistoX = 200;
  // }
  
  if (chain1->GetEntries() < 50000){
    nbinsHistoX = 200;
  }

  for (int iBar = 0; iBar < NBARS; iBar++){

    p2_eff_vs_posXY[iBar] = new TProfile2D(Form("p2_eff_vs_posXY_BAR%d",iBar),Form("p2_eff_vs_posXY_BAR%d",iBar), 400, xmin, xmax, 400, ymin, ymax, 0, 1);
    p_eff_vs_posX[iBar] = new TProfile(Form("p_eff_vs_posX_BAR%d",iBar),Form("p_eff_vs_posX_BAR%d",iBar), 400, xmin, xmax, 0, 1);
    p_eff_vs_posY[iBar] = new TProfile(Form("p_eff_vs_posY_BAR%d",iBar),Form("p_eff_vs_posY_BAR%d",iBar), 200, ymin, ymax, 0, 1);
    
    //
    h_ampL_nocuts[iBar] = new TH1F(Form("h_ampL_nocuts_BAR%d", iBar), Form("h_ampL_nocuts_BAR%d",iBar), 1000, 0., 1.0);
    h_ampL[iBar]        = new TH1F(Form("h_ampL_BAR%d", iBar), Form("h_ampL_BAR%d",iBar), 1000, 0., 1.0);
    h_timeL[iBar]       = new TH1F(Form("h_timeL_BAR%d",iBar),Form("h_timeL_BAR%d",iBar),400,0.,200.);
    h_totL[iBar]        = new TH1F(Form("h_totL_BAR%d",iBar),Form("h_totL_BAR%d",iBar),400,0.,200.);
    h_dtimeMaximumL[iBar] = new TH1F(Form("h_dtimeMaximumL_BAR%d",iBar),Form("h_dtimeMaximumL_BAR%d",iBar),2000,0.,100.);

    h_ampR_nocuts[iBar] = new TH1F(Form("h_ampR_nocuts_BAR%d", iBar), Form("h_ampR_nocuts_BAR%d",iBar), 1000, 0., 1.0);
    h_ampR[iBar]        = new TH1F(Form("h_ampR_BAR%d", iBar), Form("h_ampR_BAR%d",iBar), 1000, 0., 1.0);
    h_timeR[iBar]       = new TH1F(Form("h_timeR_BAR%d",iBar),Form("h_timeR_BAR%d",iBar),400,0.,200.);
    h_totR[iBar]        = new TH1F(Form("h_totR_BAR%d",iBar),Form("h_totR_BAR%d",iBar),400,0.,200.);
    h_dtimeMaximumR[iBar] = new TH1F(Form("h_dtimeMaximumR_BAR%d",iBar),Form("h_dtimeMaximumR_BAR%d",iBar),2000,0.,100.);

    p2_ampL_vs_posXY[iBar] = new TProfile2D(Form("p2_ampL_vs_posXY_BAR%d",iBar),Form("p2_ampL_vs_posXY_BAR%d",iBar), 100, xmin, xmax, 100, ymin, ymax, 0, 1);
    p_ampL_vs_posX[iBar]   = new TProfile(Form("p_ampL_vs_posX_BAR%d",iBar),Form("p_ampL_vs_posX_BAR%d",iBar), 100, xmin, xmax, 0, 1);
    p_ampL_vs_posXc[iBar]  = new TProfile(Form("p_ampL_vs_posXc_BAR%d",iBar),Form("p_ampL_vs_posXc_BAR%d",iBar), 100, xmin/cos(theta[iBar]), xmax/cos(theta[iBar]), 0, 1);
    p_ampL_vs_posY[iBar]   = new TProfile(Form("p_ampL_vs_posY_BAR%d",iBar),Form("p_ampL_vs_posY_BAR%d",iBar), 100, ymin, ymax, 0, 1);
    p_ampL_vs_tDiff[iBar]  = new TProfile(Form("p_ampL_vs_tDiff_BAR%d",iBar),Form("p_ampL_vs_tDiff_BAR%d",iBar), 400, -4, 4, 0, 1);

    p2_ampR_vs_posXY[iBar] = new TProfile2D(Form("p2_ampR_vs_posXY_BAR%d",iBar),Form("p2_ampR_vs_posXY_BAR%d",iBar), 100, xmin, xmax, 100, ymin, ymax, 0, 1);
    p_ampR_vs_posX[iBar]   = new TProfile(Form("p_ampR_vs_posX_BAR%d",iBar),Form("p_ampR_vs_posX_BAR%d",iBar), 100, xmin, xmax, 0, 1);
    p_ampR_vs_posXc[iBar]  = new TProfile(Form("p_ampR_vs_posXc_BAR%d",iBar),Form("p_ampR_vs_posXc_BAR%d",iBar), 100, xmin/cos(theta[iBar]), xmax/cos(theta[iBar]), 0, 1);
    p_ampR_vs_posY[iBar]   = new TProfile(Form("p_ampR_vs_posY_BAR%d",iBar),Form("p_ampR_vs_posY_BAR%d",iBar), 100, ymin, ymax, 0, 1);
    p_ampR_vs_tDiff[iBar]  = new TProfile(Form("p_ampR_vs_tDiff_BAR%d",iBar),Form("p_ampR_vs_tDiff_BAR%d",iBar), 400, -4, 4, 0, 1);

    h2_tDiff_vs_posX[iBar] = new TH2F(Form("h2_tDiff_vs_posX_BAR%d",iBar), Form("h2_tDiff_vs_posX_BAR%d",iBar), 400, xmin, xmax, 400, -4, 4);

    // -- tLeft
    p2_tL_vs_posXY[iBar]        = new TProfile2D(Form("p2_tL_vs_posXY_BAR%d",iBar),Form("p2_tL_vs_posXY_BAR%d",iBar), 400, xmin, xmax, 200, ymin, ymax, dtminL[iBar], dtmaxL[iBar]);

    h_tL[iBar]                  = new TH1F(Form("h_tL_BAR%d",iBar),Form("h_tL_BAR%d",iBar), 1000, dtminL[iBar], dtmaxL[iBar]);
    p_tL_vs_amp[iBar]           = new TProfile(Form("p_tL_vs_amp_BAR%d",iBar),Form("p_tL_vs_amp_BAR%d",iBar), 200, 0., 1.,dtminL[iBar], dtmaxL[iBar]);
    p_tL_vs_posX[iBar]          = new TProfile(Form("p_tL_vs_posX_BAR%d",iBar),Form("p_tL_vs_posX_BAR%d",iBar), 400, xmin, xmax,dtminL[iBar], dtmaxL[iBar]);
    p_tL_vs_posY[iBar]          = new TProfile(Form("p_tL_vs_posY_BAR%d",iBar),Form("p_tL_vs_posY_BAR%d",iBar), 200, ymin, ymax,dtminL[iBar], dtmaxL[iBar]);
    p_tL_vs_tDiff[iBar]         = new TProfile(Form("p_tL_vs_tDiff_BAR%d",iBar),Form("p_tL_vs_tDiff_BAR%d",iBar), 400, -4, 4, dtminL[iBar], dtmaxL[iBar]);

    h_tL_ampCorr[iBar]          = new TH1F(Form("h_tL_ampCorr_BAR%d",iBar),Form("h_tL_ampCorr_BAR%d",iBar), 1000, dtminL[iBar], dtmaxL[iBar]);
    p_tL_ampCorr_vs_amp[iBar]   = new TProfile(Form("p_tL_ampCorr_vs_amp_BAR%d",iBar),Form("p_tL_ampCorr_vs_amp_BAR%d",iBar), 200, 0., 1.,dtminL[iBar], dtmaxL[iBar]);
    p_tL_ampCorr_vs_posX[iBar]  = new TProfile(Form("p_tL_ampCorr_vs_posX_BAR%d",iBar),Form("p_tL_ampCorr_vs_posX_BAR%d",iBar), 400, xmin, xmax,dtminL[iBar], dtmaxL[iBar]);
    p_tL_ampCorr_vs_posY[iBar]  = new TProfile(Form("p_tL_ampCorr_vs_posY_BAR%d",iBar),Form("p_tL_ampCorr_vs_posY_BAR%d",iBar), 200, ymin, ymax,dtminL[iBar], dtmaxL[iBar]);
    p_tL_ampCorr_vs_tDiff[iBar] = new TProfile(Form("p_tL_ampCorr_vs_tDiff_BAR%d",iBar),Form("p_tL_ampCorr_vs_tDiff_BAR%d",iBar), 400, -4, 4,dtminL[iBar], dtmaxL[iBar]);
    p_tL_ampCorr_vs_posXc[iBar] = new TProfile(Form("p_tL_ampCorr_vs_posXc_BAR%d",iBar),Form("p_tL_ampCorr_vs_posXc_BAR%d",iBar), 400, xmin/cos(theta[iBar]), xmax/cos(theta[iBar]),dtminL[iBar], dtmaxL[iBar]);
   
    // -- tRight
    p2_tR_vs_posXY[iBar]        = new TProfile2D(Form("p2_tR_vs_posXY_BAR%d",iBar),Form("p2_tR_vs_posXY_BAR%d",iBar), 400, xmin, xmax, 200, ymin, ymax,dtminR[iBar], dtmaxR[iBar]);

    h_tR[iBar]                  = new TH1F(Form("h_tR_BAR%d",iBar),Form("h_tR_BAR%d",iBar), 1000, dtminR[iBar], dtmaxR[iBar]);
    p_tR_vs_amp[iBar]           = new TProfile(Form("p_tR_vs_amp_BAR%d",iBar),Form("p_tR_vs_amp_BAR%d",iBar), 200, 0., 1.,dtminR[iBar], dtmaxR[iBar]);
    p_tR_vs_posX[iBar]          = new TProfile(Form("p_tR_vs_posX_BAR%d",iBar),Form("p_tR_vs_posX_BAR%d",iBar), 400, xmin, xmax,dtminR[iBar], dtmaxR[iBar]);
    p_tR_vs_posY[iBar]          = new TProfile(Form("p_tR_vs_posY_BAR%d",iBar),Form("p_tR_vs_posY_BAR%d",iBar), 200, ymin, ymax,dtminR[iBar], dtmaxR[iBar]);
    p_tR_vs_tDiff[iBar]         = new TProfile(Form("p_tR_vs_tDiff_BAR%d",iBar),Form("p_tR_vs_tDiff_BAR%d",iBar), 400, -4, 4,dtminR[iBar], dtmaxR[iBar]);

    h_tR_ampCorr[iBar]          = new TH1F(Form("h_tR_ampCorr_BAR%d",iBar),Form("h_tR_ampCorr_BAR%d",iBar), 1000, dtminR[iBar], dtmaxR[iBar]);
    p_tR_ampCorr_vs_amp[iBar]   = new TProfile(Form("p_tR_ampCorr_vs_amp_BAR%d",iBar),Form("p_tR_ampCorr_vs_amp_BAR%d",iBar), 200, 0., 1., dtminR[iBar], dtmaxR[iBar]);
    p_tR_ampCorr_vs_posX[iBar]  = new TProfile(Form("p_tR_ampCorr_vs_posX_BAR%d",iBar),Form("p_tR_ampCorr_vs_posX_BAR%d",iBar), 400, xmin, xmax, dtminR[iBar], dtmaxR[iBar]);
    p_tR_ampCorr_vs_posY[iBar]  = new TProfile(Form("p_tR_ampCorr_vs_posY_BAR%d",iBar),Form("p_tR_ampCorr_vs_posY_BAR%d",iBar), 200, ymin, ymax, dtminR[iBar], dtmaxR[iBar]);
    p_tR_ampCorr_vs_tDiff[iBar] = new TProfile(Form("p_tR_ampCorr_vs_tDiff_BAR%d",iBar),Form("p_tR_ampCorr_vs_tDiff_BAR%d",iBar), 400, -4, 4, dtminR[iBar], dtmaxR[iBar]);
    p_tR_ampCorr_vs_posXc[iBar] = new TProfile(Form("p_tR_ampCorr_vs_posXc_BAR%d",iBar),Form("p_tR_ampCorr_vs_posXc_BAR%d",iBar), 400, xmin/cos(theta[iBar]), xmax/cos(theta[iBar]), dtminR[iBar], dtmaxR[iBar]);

    // -- tAverage
    p2_tAve_vs_posXY[iBar]        = new TProfile2D(Form("p2_tAve_vs_posXY_BAR%d",iBar),Form("p2_tAve_vs_posXY_BAR%d",iBar), 400, xmin, xmax, 200, ymin, ymax, dtminAve[iBar], dtmaxAve[iBar]);

    h_tAve[iBar]                  = new TH1F(Form("h_tAve_BAR%d",iBar),Form("h_tAve_BAR%d",iBar),1000, dtminAve[iBar], dtmaxAve[iBar]);
    p_tAve_vs_posX[iBar]          = new TProfile(Form("p_tAve_vs_posX_BAR%d",iBar),Form("p_tAve_vs_posX_BAR%d",iBar), 400, xmin, xmax, dtminAve[iBar], dtmaxAve[iBar]);
    p_tAve_vs_posY[iBar]          = new TProfile(Form("p_tAve_vs_posY_BAR%d",iBar),Form("p_tAve_vs_posY_BAR%d",iBar), 200, ymin, ymax, dtminAve[iBar], dtmaxAve[iBar]);
    p_tAve_vs_tDiff[iBar]         = new TProfile(Form("p_tAve_vs_tDiff_BAR%d",iBar),Form("p_tAve_vs_tDiff_BAR%d",iBar), 400, -4, 4, dtminAve[iBar], dtmaxAve[iBar]);


    // -- plain tAve
    h_tAve_ampCorr[iBar]          = new TH1F(Form("h_tAve_ampCorr_BAR%d",iBar),Form("h_tAve_ampCorr_BAR%d",iBar), 1000, dtminAve[iBar], dtmaxAve[iBar]);
    p_tAve_ampCorr_vs_posX[iBar]  = new TProfile(Form("p_tAve_ampCorr_vs_posX_BAR%d",iBar),Form("p_tAve_ampCorr_vs_posX_BAR%d",iBar), 400, xmin, xmax, dtminAve[iBar], dtmaxAve[iBar]);
    p_tAve_ampCorr_vs_posY[iBar]  = new TProfile(Form("p_tAve_ampCorr_vs_posY_BAR%d",iBar),Form("p_tAve_ampCorr_vs_posY_BAR%d",iBar), 200, ymin, ymax, dtminAve[iBar], dtmaxAve[iBar]);
    p_tAve_ampCorr_vs_tDiff[iBar] = new TProfile(Form("p_tAve_ampCorr_vs_tDiff_BAR%d",iBar),Form("p_tAve_ampCorr_vs_tDiff_BAR%d",iBar), 400, -4, 4, dtminAve[iBar], dtmaxAve[iBar]);
    p_tAve_ampCorr_vs_posXc[iBar]  = new TProfile(Form("p_tAve_ampCorr_vs_posXc_BAR%d",iBar),Form("p_tAve_ampCorr_vs_posXc_BAR%d",iBar), 400, xmin/cos(theta[iBar]), xmax/cos(theta[iBar]), dtminAve[iBar], dtmaxAve[iBar]);
    for (int ibin = 0; ibin < NBINSX[iBar]; ibin++){
      h_tL_ampCorr_binX[iBar][ibin] = new TH1F(Form("h_tL_ampCorr_binX_%d_BAR%d",ibin, iBar),Form("h_tL_ampCorr_binX_%d_BAR%d",ibin,iBar), nbinsHistoX, dtminL[iBar], dtmaxL[iBar]);
      h_tR_ampCorr_binX[iBar][ibin] = new TH1F(Form("h_tR_ampCorr_binX_%d_BAR%d",ibin, iBar),Form("h_tR_ampCorr_binX_%d_BAR%d",ibin,iBar), nbinsHistoX, dtminR[iBar], dtmaxR[iBar]);
      h_tAve_ampCorr_binX[iBar][ibin] = new TH1F(Form("h_tAve_ampCorr_binX_%d_BAR%d",ibin, iBar),Form("h_tAve_ampCorr_binX_%d_BAR%d",ibin,iBar), nbinsHistoX, dtminAve[iBar], dtmaxAve[iBar]);
    }

    // -- plain tAve after correction vs tDiff residual dependence
    h_tAve_ampCorr_tDiffCorr[iBar]          = new TH1F(Form("h_tAve_ampCorr_tDiffCorr_BAR%d",iBar),Form("h_tAve_ampCorr_tDiffCorr_BAR%d",iBar), 1000, dtminAve[iBar], dtmaxAve[iBar]);
    p_tAve_ampCorr_tDiffCorr_vs_posX[iBar]  = new TProfile(Form("p_tAve_ampCorr_tDiffCorr_vs_posX_BAR%d",iBar),Form("p_tAve_ampCorr_tDiffCorr_vs_posX_BAR%d",iBar), 400, xmin, xmax, dtminAve[iBar], dtmaxAve[iBar]);
    p_tAve_ampCorr_tDiffCorr_vs_posY[iBar]  = new TProfile(Form("p_tAve_ampCorr_tDiffCorr_vs_posY_BAR%d",iBar),Form("p_tAve_ampCorr_tDiffCorr_vs_posY_BAR%d",iBar), 200, ymin, ymax, dtminAve[iBar], dtmaxAve[iBar]);
    p_tAve_ampCorr_tDiffCorr_vs_tDiff[iBar] = new TProfile(Form("p_tAve_ampCorr_tDiffCorr_vs_tDiff_BAR%d",iBar),Form("p_tAve_ampCorr_tDiffCorr_vs_tDiff_BAR%d",iBar), 400, -4, 4, dtminAve[iBar], dtmaxAve[iBar]);
    p_tAve_ampCorr_tDiffCorr_vs_posXc[iBar]  = new TProfile(Form("p_tAve_ampCorr_tDiffCorr_vs_posXc_BAR%d",iBar),Form("p_tAve_ampCorr_tDiffCorr_vs_posXc_BAR%d",iBar), 400, xmin/cos(theta[iBar]), xmax/cos(theta[iBar]), dtminAve[iBar], dtmaxAve[iBar]);
    for (int ibin = 0; ibin < NBINSX[iBar]; ibin++){
      h_tAve_ampCorr_tDiffCorr_binX[iBar][ibin] = new TH1F(Form("h_tAve_ampCorr_tDiffCorr_binX_%d_BAR%d",ibin, iBar),Form("h_tAve_ampCorr_tDiffCorr_binX_%d_BAR%d",ibin,iBar), nbinsHistoX, dtminAve[iBar], dtmaxAve[iBar]);
    }


    // -- plain tAve after correction vs position dependence
    h_tAve_ampCorr_posCorr[iBar]          = new TH1F(Form("h_tAve_ampCorr_posCorr_BAR%d",iBar),Form("h_tAve_ampCorr_posCorr_BAR%d",iBar), 1000, dtminAve[iBar], dtmaxAve[iBar]);
    p_tAve_ampCorr_posCorr_vs_posX[iBar]  = new TProfile(Form("p_tAve_ampCorr_posCorr_vs_posX_BAR%d",iBar),Form("p_tAve_ampCorr_posCorr_vs_posX_BAR%d",iBar), 400, xmin, xmax, dtminAve[iBar], dtmaxAve[iBar]);
    p_tAve_ampCorr_posCorr_vs_posY[iBar]  = new TProfile(Form("p_tAve_ampCorr_posCorr_vs_posY_BAR%d",iBar),Form("p_tAve_ampCorr_posCorr_vs_posY_BAR%d",iBar), 200, ymin, ymax, dtminAve[iBar], dtmaxAve[iBar]);
    p_tAve_ampCorr_posCorr_vs_tDiff[iBar] = new TProfile(Form("p_tAve_ampCorr_posCorr_vs_tDiff_BAR%d",iBar),Form("p_tAve_ampCorr_posCorr_vs_tDiff_BAR%d",iBar), 400, -4, 4, dtminAve[iBar], dtmaxAve[iBar]);
    p_tAve_ampCorr_posCorr_vs_posXc[iBar]  = new TProfile(Form("p_tAve_ampCorr_posCorr_vs_posXc_BAR%d",iBar),Form("p_tAve_ampCorr_posCorr_vs_posXc_BAR%d",iBar), 400, xmin/cos(theta[iBar]), xmax/cos(theta[iBar]), dtminAve[iBar], dtmaxAve[iBar]);


    // -- amp weighted tAve
    h_tAveAmpW_ampCorr[iBar]          = new TH1F(Form("h_tAveAmpW_ampCorr_BAR%d",iBar),Form("h_tAveAmpW_ampCorr_BAR%d",iBar), 1000, dtminAve[iBar], dtmaxAve[iBar]);
    p_tAveAmpW_ampCorr_vs_posX[iBar]  = new TProfile(Form("p_tAveAmpW_ampCorr_vs_posX_BAR%d",iBar),Form("p_tAveAmpW_ampCorr_vs_posX_BAR%d",iBar), 400, xmin, xmax, dtminAve[iBar], dtmaxAve[iBar]);
    p_tAveAmpW_ampCorr_vs_posY[iBar]  = new TProfile(Form("p_tAveAmpW_ampCorr_vs_posY_BAR%d",iBar),Form("p_tAveAmpW_ampCorr_vs_posY_BAR%d",iBar), 200, ymin, ymax, dtminAve[iBar], dtmaxAve[iBar]);
    p_tAveAmpW_ampCorr_vs_tDiff[iBar] = new TProfile(Form("p_tAveAmpW_ampCorr_vs_tDiff_BAR%d",iBar),Form("p_tAveAmpW_ampCorr_vs_tDiff_BAR%d",iBar), 400, -4, 4, dtminAve[iBar], dtmaxAve[iBar]);
    p_tAveAmpW_ampCorr_vs_posXc[iBar]  = new TProfile(Form("p_tAveAmpW_ampCorr_vs_posXc_BAR%d",iBar),Form("p_tAveAmpW_ampCorr_vs_posXc_BAR%d",iBar), 400, xmin/cos(theta[iBar]), xmax/cos(theta[iBar]), dtminAve[iBar], dtmaxAve[iBar]);
    for (int ibin = 0; ibin < NBINSX[iBar]; ibin++){
      h_tAveAmpW_ampCorr_binX[iBar][ibin] = new TH1F(Form("h_tAveAmpW_ampCorr_binX_%d_BAR%d",ibin, iBar),Form("h_tAveAmpW_ampCorr_binX_%d_BAR%d",ibin,iBar), nbinsHistoX, dtminAve[iBar], dtmaxAve[iBar]);
    }

    // -- amp weighted tAve after correction vs tDiff residual dependence
    h_tAveAmpW_ampCorr_tDiffCorr[iBar]          = new TH1F(Form("h_tAveAmpW_ampCorr_tDiffCorr_BAR%d",iBar),Form("h_tAveAmpW_ampCorr_tDiffCorr_BAR%d",iBar), 1000, dtminAve[iBar], dtmaxAve[iBar]);
    p_tAveAmpW_ampCorr_tDiffCorr_vs_posX[iBar]  = new TProfile(Form("p_tAveAmpW_ampCorr_tDiffCorr_vs_posX_BAR%d",iBar),Form("p_tAveAmpW_ampCorr_tDiffCorr_vs_posX_BAR%d",iBar), 400, xmin, xmax, dtminAve[iBar], dtmaxAve[iBar]);
    p_tAveAmpW_ampCorr_tDiffCorr_vs_posY[iBar]  = new TProfile(Form("p_tAveAmpW_ampCorr_tDiffCorr_vs_posY_BAR%d",iBar),Form("p_tAveAmpW_ampCorr_tDiffCorr_vs_posY_BAR%d",iBar), 200, ymin, ymax, dtminAve[iBar], dtmaxAve[iBar]);
    p_tAveAmpW_ampCorr_tDiffCorr_vs_tDiff[iBar] = new TProfile(Form("p_tAveAmpW_ampCorr_tDiffCorr_vs_tDiff_BAR%d",iBar),Form("p_tAveAmpW_ampCorr_tDiffCorr_vs_tDiff_BAR%d",iBar), 400, -4, 4, dtminAve[iBar], dtmaxAve[iBar]);
    p_tAveAmpW_ampCorr_tDiffCorr_vs_posXc[iBar]  = new TProfile(Form("p_tAveAmpW_ampCorr_tDiffCorr_vs_posXc_BAR%d",iBar),Form("p_tAveAmpW_ampCorr_tDiffCorr_vs_posXc_BAR%d",iBar), 400, xmin/cos(theta[iBar]), xmax/cos(theta[iBar]), dtminAve[iBar], dtmaxAve[iBar]);
    for (int ibin = 0; ibin < NBINSX[iBar]; ibin++){
      h_tAveAmpW_ampCorr_tDiffCorr_binX[iBar][ibin] = new TH1F(Form("h_tAveAmpW_ampCorr_tDiffCorr_binX_%d_BAR%d",ibin, iBar),Form("h_tAveAmpW_ampCorr_tDiffCorr_binX_%d_BAR%d",ibin,iBar), nbinsHistoX, dtminAve[iBar], dtmaxAve[iBar]);
    }



    // -- tRes weighted tAve
    h_tAveResW_ampCorr[iBar]          = new TH1F(Form("h_tAveResW_ampCorr_BAR%d",iBar),Form("h_tAveResW_ampCorr_BAR%d",iBar), 1000, dtminAve[iBar], dtmaxAve[iBar]);
    p_tAveResW_ampCorr_vs_posX[iBar]  = new TProfile(Form("p_tAveResW_ampCorr_vs_posX_BAR%d",iBar),Form("p_tAveResW_ampCorr_vs_posX_BAR%d",iBar), 400, xmin, xmax, dtminAve[iBar], dtmaxAve[iBar]);
    p_tAveResW_ampCorr_vs_posY[iBar]  = new TProfile(Form("p_tAveResW_ampCorr_vs_posY_BAR%d",iBar),Form("p_tAveResW_ampCorr_vs_posY_BAR%d",iBar), 200, ymin, ymax, dtminAve[iBar], dtmaxAve[iBar]);
    p_tAveResW_ampCorr_vs_tDiff[iBar] = new TProfile(Form("p_tAveResW_ampCorr_vs_tDiff_BAR%d",iBar),Form("p_tAveResW_ampCorr_vs_tDiff_BAR%d",iBar), 400, -4, 4, dtminAve[iBar], dtmaxAve[iBar]);
    p_tAveResW_ampCorr_vs_posXc[iBar]  = new TProfile(Form("p_tAveResW_ampCorr_vs_posXc_BAR%d",iBar),Form("p_tAveResW_ampCorr_vs_posXc_BAR%d",iBar), 400, xmin/cos(theta[iBar]), xmax/cos(theta[iBar]), dtminAve[iBar], dtmaxAve[iBar]);
    for (int ibin = 0; ibin < NBINSX[iBar]; ibin++){
      h_tAveResW_ampCorr_binX[iBar][ibin] = new TH1F(Form("h_tAveResW_ampCorr_binX_%d_BAR%d",ibin, iBar),Form("h_tAveResW_ampCorr_binX_%d_BAR%d",ibin,iBar), nbinsHistoX, dtminAve[iBar], dtmaxAve[iBar]);
    }

    // -- amp weighted tAve after correction vs tDiff residual dependence                                                                                                                                                                     
    h_tAveResW_ampCorr_tDiffCorr[iBar]          = new TH1F(Form("h_tAveResW_ampCorr_tDiffCorr_BAR%d",iBar),Form("h_tAveResW_ampCorr_tDiffCorr_BAR%d",iBar), 1000, dtminAve[iBar], dtmaxAve[iBar]);
    p_tAveResW_ampCorr_tDiffCorr_vs_posX[iBar]  = new TProfile(Form("p_tAveResW_ampCorr_tDiffCorr_vs_posX_BAR%d",iBar),Form("p_tAveResW_ampCorr_tDiffCorr_vs_posX_BAR%d",iBar), 400, xmin, xmax, dtminAve[iBar], dtmaxAve[iBar]);
    p_tAveResW_ampCorr_tDiffCorr_vs_posY[iBar]  = new TProfile(Form("p_tAveResW_ampCorr_tDiffCorr_vs_posY_BAR%d",iBar),Form("p_tAveResW_ampCorr_tDiffCorr_vs_posY_BAR%d",iBar), 200, ymin, ymax, dtminAve[iBar], dtmaxAve[iBar]);
    p_tAveResW_ampCorr_tDiffCorr_vs_tDiff[iBar] = new TProfile(Form("p_tAveResW_ampCorr_tDiffCorr_vs_tDiff_BAR%d",iBar),Form("p_tAveResW_ampCorr_tDiffCorr_vs_tDiff_BAR%d",iBar), 400, -4, 4, dtminAve[iBar], dtmaxAve[iBar]);
    p_tAveResW_ampCorr_tDiffCorr_vs_posXc[iBar]  = new TProfile(Form("p_tAveResW_ampCorr_tDiffCorr_vs_posXc_BAR%d",iBar),Form("p_tAveResW_ampCorr_tDiffCorr_vs_posXc_BAR%d",iBar), 400, xmin/cos(theta[iBar]), xmax/cos(theta[iBar]), dtminAve[iBar], dtmaxAve[iBar]);
    for (int ibin = 0; ibin < NBINSX[iBar]; ibin++){
      h_tAveResW_ampCorr_tDiffCorr_binX[iBar][ibin] = new TH1F(Form("h_tAveResW_ampCorr_tDiffCorr_binX_%d_BAR%d",ibin, iBar),Form("h_tAveResW_ampCorr_tDiffCorr_binX_%d_BAR%d",ibin,iBar), nbinsHistoX, dtminAve[iBar], dtmaxAve[iBar]);
    }



  }



  float ampRef = 0;
  float tRef = 0;

  float ampL  = 0.;
  float ampR  = 0.;
  float tL    = 0.;
  float tR    = 0.;
  float tAve  = 0.;
  float totL  = 0.;
  float totR  = 0.;

  float tMaximumL = 0;
  float tMaximumR = 0;

  float posX = 0;
  float posY = 0;
  
  float kAdcToV = 1./4096.; // factor to convert amplitudes in V
  float tMinCh  = 25.;
  float tMaxCh  = 50.;
  float tMinRef = 25.;
  float tMaxRef = 50.;
  
  //float tMinCh  = 0.;
  //float tMaxCh  = 200.;
  //float tMinRef = 0.;
  //float tMaxRef = 200.;


  if (timeChannelRef=="PTK2_0"){
    tMinRef = 35.;
    tMaxRef = 60.;
  }

  //float dtMaxCh = 1.0;
  float dtMaxCh = 999999.;

  float totMin = 40;
  float totMax = 150;

  float extra_smearing_amp = 0.00;
  TRandom *gRandom = new TRandom();
  std::map<int, float> rndL[NBARS];
  std::map<int, float> rndR[NBARS];

  //-----------------------
  // first loop over events
  std::cout<<"First loop over events ..." <<std::endl;
  int nEntries = chain1 -> GetEntries();
  for(int entry = 0; entry < nEntries; ++entry)
    {
      if( entry%1000 == 0 ) std::cout << ">>> Reading entry " << entry << " / " << nEntries << "\r" << std::flush;
      chain1 -> GetEntry(entry);

      // -- position
      if (treeVars.t_ntracks != 1) continue;
      posX = (treeVars.t_trackFitResult->at(0)).x();
      posY = (treeVars.t_trackFitResult->at(0)).y();
      if (posX == -999. || posY == -999.) continue;

      // -- Photek as ref
      ampRef = treeVars.t_amp[(*treeVars.t_channelId)[ampChannelRef]] * kAdcToV ;
      tRef   = treeVars.t_time[(*treeVars.t_channelId)[timeChannelRef]+treeVars.t_CFD];

      h_ampRef_nocuts->Fill(ampRef);

      if ( ampRef > cut_ampMinRef  &&  ampRef < cut_ampMaxRef ){
	h_ampRef->Fill(ampRef);
	h_timeRef->Fill(tRef);
      }

      if ( ampRef > cut_ampMinRef){
	p2_eff_vs_posXY_Ref -> Fill(posX, posY, 1);
	p_eff_vs_posX_Ref -> Fill(posX, 1);
	p_eff_vs_posY_Ref -> Fill(posY, 1);
      }
      else{
      	p2_eff_vs_posXY_Ref -> Fill(posX, posY, 0);
	p_eff_vs_posX_Ref -> Fill(posX, 0);
	p_eff_vs_posY_Ref -> Fill(posY, 0);
      }
      
      if ( ampRef > cut_ampMinRef  &&  ampRef < cut_ampMaxRef && tRef > tMinRef && tRef < tMaxRef ) {
	p_timeRef_vs_ampRef->Fill(ampRef, tRef);
	p_ampRef_vs_posX->Fill(posX,ampRef);
	p_ampRef_vs_posY->Fill(posY,ampRef); 
	p2_ampRef_vs_posXY ->Fill(posX, posY, ampRef);
	p2_timeRef_vs_posXY->Fill(posX, posY, tRef);
      }



      // -- loop over bars
      for (int iBar = 0; iBar < NBARS; iBar++){

	ampL = treeVars.t_amp[(*treeVars.t_channelId)[ ampChannelsL[iBar] ]] * kAdcToV ;
	ampR = treeVars.t_amp[(*treeVars.t_channelId)[ ampChannelsR[iBar] ]] * kAdcToV ;
	
	tL   = treeVars.t_time[(*treeVars.t_channelId)[ timeChannelsL[iBar] ]+treeVars.t_LED];  
	tR   = treeVars.t_time[(*treeVars.t_channelId)[ timeChannelsR[iBar] ]+treeVars.t_LED];  

	tMaximumL = treeVars.t_time_maximum[(*treeVars.t_channelId)[ timeChannelsL[iBar] ]];
	tMaximumR = treeVars.t_time_maximum[(*treeVars.t_channelId)[ timeChannelsR[iBar] ]];
	
	// time-over-threshold TED-LED
	totL = treeVars.t_time[(*treeVars.t_channelId)[ timeChannelsL[iBar] ]+treeVars.t_TED] - tL;
	totR = treeVars.t_time[(*treeVars.t_channelId)[ timeChannelsR[iBar] ]+treeVars.t_TED] - tR;

	if ( ampL > cut_ampMinL[iBar]  && ampR > cut_ampMinR[iBar] ) {
	  p2_eff_vs_posXY[iBar] ->Fill(posX, posY, 1.);
	  if ( posY > cut_Ymin[iBar] && posY < cut_Ymax[iBar] ) p_eff_vs_posX[iBar] ->Fill(posX, 1.);
	  if ( posX > cut_Xmin[iBar] && posX < cut_Xmax[iBar] )	p_eff_vs_posY[iBar] ->Fill(posY, 1.);
	}
	else{
	  p2_eff_vs_posXY[iBar] ->Fill(posX, posY, 0.);
	  if ( posY > cut_Ymin[iBar] && posY < cut_Ymax[iBar] )	  p_eff_vs_posX[iBar] ->Fill(posX, 0.);
	  if ( posX > cut_Xmin[iBar] && posX < cut_Xmax[iBar] )	  p_eff_vs_posY[iBar] ->Fill(posY, 0.);
	}
	

	// -- select events
	if ( posX < cut_Xmin[iBar] || posX > cut_Xmax[iBar] ) continue;
	if ( posY < cut_Ymin[iBar] || posY > cut_Ymax[iBar] ) continue;

 	// -- fill amp histograms before any selection
	h_ampL_nocuts[iBar] -> Fill(ampL);
	h_ampR_nocuts[iBar] -> Fill(ampR);

	if ( ampRef < cut_ampMinRef     ||  ampRef > cut_ampMaxRef ) continue;
	if ( ampL   < cut_ampMinL[iBar] ||  ampL   > cut_ampMaxL[iBar] ) continue;
	if ( ampR   < cut_ampMinR[iBar] ||  ampR   > cut_ampMaxR[iBar] ) continue;

	if ( tRef < 0  ||  tRef > 200) continue;
	if ( tL   < 0  ||  tL   > 200) continue;
	if ( tR   < 0  ||  tR   > 200) continue;
	

	// -- remove events with bad reco time
	h_timeL[iBar] -> Fill(tL);
	h_timeR[iBar] -> Fill(tR);

	if ( tRef < tMinRef || tRef > tMaxRef ) continue;
	if ( tL < tMinCh || tL > tMaxCh ) continue;
	if ( tR < tMinCh || tR > tMaxCh ) continue;

	h_totL[iBar] -> Fill(totL);
        h_totR[iBar] -> Fill(totR);

	h_dtimeMaximumL[iBar] ->Fill(tMaximumL - tL);
	h_dtimeMaximumR[iBar] ->Fill(tMaximumR - tR);

	if ( fabs(tL - tMaximumL) > dtMaxCh) continue;
	if ( fabs(tR - tMaximumR) > dtMaxCh) continue;

	
	if ( totL < totMin || totL > totMax ) continue;
	if ( totR < totMin || totR > totMax ) continue;

	//-- time of Left, Right and average
	tL = tL - tRef;
	tR = tR - tRef;
	tAve  = (tL + tR)/2;
	
	
	
	// --- SiPM Left
	h_ampL[iBar]  -> Fill(ampL);
	p2_ampL_vs_posXY[iBar] ->Fill(posX,posY,ampL);
	p_ampL_vs_posX[iBar] ->Fill(posX,ampL);
	p_ampL_vs_posXc[iBar] ->Fill(posX/cos(theta[iBar]),ampL);
	p_ampL_vs_posY[iBar] ->Fill(posY,ampL);

	h_tL[iBar]->Fill(tL);
	p_tL_vs_amp[iBar]->Fill(ampL,tL);
	p2_tL_vs_posXY[iBar] ->Fill(posX,posY,tL);
	p_tL_vs_posX[iBar]->Fill(posX,tL);
	p_tL_vs_posY[iBar]->Fill(posY,tL);
	

	// --- SiPM Right
        h_ampR[iBar]  -> Fill(ampR);
        p2_ampR_vs_posXY[iBar] ->Fill(posX,posY,ampR);
        p_ampR_vs_posX[iBar] ->Fill(posX,ampR);
        p_ampR_vs_posXc[iBar] ->Fill(posX/cos(theta[iBar]),ampR);
        p_ampR_vs_posY[iBar] ->Fill(posY,ampR);
        
        h_tR[iBar]->Fill(tR);
        p_tR_vs_amp[iBar]->Fill(ampR,tR);
        p2_tR_vs_posXY[iBar] ->Fill(posX,posY,tR);
        p_tR_vs_posX[iBar]->Fill(posX,tR);
        p_tR_vs_posY[iBar]->Fill(posY,tR);
        

	// -- average
        h_tAve[iBar]->Fill(tAve);
	p2_tAve_vs_posXY[iBar] ->Fill(posX,posY,tAve);
        p_tAve_vs_posX[iBar]->Fill(posX,tAve);
        p_tAve_vs_posY[iBar]->Fill(posY,tAve);
                                                    
      } // -- end loop over bars
      
    }// -- end first loop over events
  
  
  // --- Amplitude walk corrections
  TF1*  fitFuncL_ampCorr[NBARS];  
  TF1*  fitFuncR_ampCorr[NBARS];  

  for (int iBar = 0 ; iBar < NBARS; iBar++){
    fitFuncL_ampCorr[iBar] = new TF1(Form("fitFuncL_ampCorr_%d", iBar),"pol6");
    fitFuncL_ampCorr[iBar] -> SetLineColor(kRed);
    p_tL_vs_amp[iBar] -> Fit(Form("fitFuncL_ampCorr_%d",iBar),"QSR",0,1);
    if ( fitFuncL_ampCorr[iBar] -> GetChisquare() == 0 ){
      p_tL_vs_amp[iBar] -> Fit(Form("fitFuncL_ampCorr_%d",iBar),"QSRI",0,1);
    }
    if ( fitFuncL_ampCorr[iBar] -> GetChisquare()/fitFuncL_ampCorr[iBar] -> GetNDF() > 10 ){
      fitFuncL_ampCorr[iBar] = new TF1(Form("fitFuncL_ampCorr_%d", iBar),"pol5");
      p_tL_vs_amp[iBar] -> Fit(Form("fitFuncL_ampCorr_%d",iBar),"QSR",0,1);
    }


    fitFuncR_ampCorr[iBar] = new TF1(Form("fitFuncR_ampCorr_%d", iBar),"pol6");
    fitFuncR_ampCorr[iBar] -> SetLineColor(kRed);
    p_tR_vs_amp[iBar] -> Fit(Form("fitFuncR_ampCorr_%d",iBar),"QSR",0,1);
    if ( fitFuncR_ampCorr[iBar] -> GetChisquare() == 0 ){
      p_tR_vs_amp[iBar] -> Fit(Form("fitFuncR_ampCorr_%d",iBar),"QSRI",0,1);
    }
    if ( fitFuncR_ampCorr[iBar] -> GetChisquare()/fitFuncR_ampCorr[iBar] -> GetNDF() > 10 ){
      fitFuncR_ampCorr[iBar] = new TF1(Form("fitFuncR_ampCorr_%d", iBar),"pol5");
      p_tR_vs_amp[iBar] -> Fit(Form("fitFuncR_ampCorr_%d",iBar),"QSR",0,1);
    }



  }


  // --- second loop over events -- fill histograms after applying amplitude walk corrections
  std::cout<<"Second loop over events ..." <<std::endl;
  for(int entry = 0; entry < nEntries; ++entry)
    {
      if( entry%1000 == 0 ) std::cout << ">>> Reading entry " << entry << " / " << nEntries << "\r" << std::flush;
      chain1 -> GetEntry(entry);

      // -- position
      if (treeVars.t_ntracks != 1) continue;
      posX = (treeVars.t_trackFitResult->at(0)).x();
      posY = (treeVars.t_trackFitResult->at(0)).y();
      if (posX == -999. || posY == -999.) continue;

      // -- Photek as ref
      ampRef = treeVars.t_amp[(*treeVars.t_channelId)[ampChannelRef]] * kAdcToV ;
      tRef   = treeVars.t_time[(*treeVars.t_channelId)[timeChannelRef]+treeVars.t_CFD];


      // -- loop over bars
      for (int iBar = 0; iBar < NBARS; iBar++){

	ampL = treeVars.t_amp[(*treeVars.t_channelId)[ ampChannelsL[iBar] ]] * kAdcToV ;
	ampR = treeVars.t_amp[(*treeVars.t_channelId)[ ampChannelsR[iBar] ]] * kAdcToV ;

	tL   = treeVars.t_time[(*treeVars.t_channelId)[ timeChannelsL[iBar] ]+treeVars.t_LED];  
	tR   = treeVars.t_time[(*treeVars.t_channelId)[ timeChannelsR[iBar] ]+treeVars.t_LED];  

	tMaximumL = treeVars.t_time_maximum[(*treeVars.t_channelId)[ timeChannelsL[iBar] ]];
        tMaximumR = treeVars.t_time_maximum[(*treeVars.t_channelId)[ timeChannelsR[iBar] ]];

        totL = treeVars.t_time[(*treeVars.t_channelId)[ timeChannelsL[iBar] ]+treeVars.t_TED] - tL;
        totR = treeVars.t_time[(*treeVars.t_channelId)[ timeChannelsR[iBar] ]+treeVars.t_TED] - tR;

	// -- select events
	if ( posX < cut_Xmin[iBar] || posX > cut_Xmax[iBar] ) continue;
	if ( posY < cut_Ymin[iBar] || posY > cut_Ymax[iBar] ) continue;

	if ( ampRef < cut_ampMinRef     ||  ampRef > cut_ampMaxRef ) continue;
	if ( ampL   < cut_ampMinL[iBar] ||  ampL   > cut_ampMaxL[iBar] ) continue;
	if ( ampR   < cut_ampMinR[iBar] ||  ampR   > cut_ampMaxR[iBar] ) continue;

	if ( tRef < 0  ||  tRef > 200) continue;
	if ( tL   < 0  ||  tL   > 200) continue;
	if ( tR   < 0  ||  tR   > 200) continue;

	if ( tRef < tMinRef || tRef > tMaxRef ) continue;
	if ( tL   < tMinCh  || tL   > tMaxCh  ) continue;
	if ( tR   < tMinCh  || tR   > tMaxCh  ) continue;

	if ( fabs(tL - tMaximumL) > dtMaxCh) continue;
	if ( fabs(tR - tMaximumR) > dtMaxCh) continue;

        if ( totL < totMin || totL > totMax ) continue;
        if ( totR < totMin || totR > totMax ) continue;


	// -- amplitude walk correction 
	tL = tL - tRef;
        tR = tR - tRef;
	
	rndL[iBar][entry] = gRandom->Gaus(0., extra_smearing_amp*ampL);
	rndR[iBar][entry] = gRandom->Gaus(0., extra_smearing_amp*ampR);
	float ampLsm  = ampL + rndL[iBar][entry];
	float ampRsm  = ampR + rndR[iBar][entry];
	float tL_corr = tL - fitFuncL_ampCorr[iBar]->Eval(ampLsm) + fitFuncL_ampCorr[iBar]->Eval(h_ampL[iBar]->GetMean()) ; 
	float tR_corr = tR - fitFuncR_ampCorr[iBar]->Eval(ampRsm) + fitFuncR_ampCorr[iBar]->Eval(h_ampR[iBar]->GetMean()) ; 
	//float tL_corr = tL - fitFuncL_ampCorr[iBar]->Eval(ampL) + fitFuncL_ampCorr[iBar]->Eval(h_ampL[iBar]->GetMean()) ; 
	//float tR_corr = tR - fitFuncR_ampCorr[iBar]->Eval(ampR) + fitFuncR_ampCorr[iBar]->Eval(h_ampR[iBar]->GetMean()) ; 
	float tAve_ampCorr = ( tL_corr + tR_corr)/2 ;
	float tDiff = tR_corr - tL_corr;

	float wL = ampL/(ampL+ampR);
	float wR = ampR/(ampL+ampR);
	float tAveAmpW_ampCorr = wL*tL_corr + wR*tR_corr;

	// -- tDiff vs position X
	h2_tDiff_vs_posX[iBar] ->Fill(posX, tDiff);

	// -- SiPM left
	p_ampL_vs_tDiff[iBar] ->Fill(tDiff,ampL);
	p_tL_vs_tDiff[iBar] ->Fill(tDiff,tL);
	h_tL_ampCorr[iBar]->Fill(tL_corr);
        p_tL_ampCorr_vs_amp[iBar]->Fill(ampL,tL_corr);
        p_tL_ampCorr_vs_posX[iBar]->Fill(posX,tL_corr);
        p_tL_ampCorr_vs_posXc[iBar]->Fill(posX/cos(theta[iBar]),tL_corr);
        p_tL_ampCorr_vs_posY[iBar]->Fill(posY,tL_corr);
        p_tL_ampCorr_vs_tDiff[iBar]->Fill(tDiff, tL_corr);


	// -- SiPM right
	p_ampR_vs_tDiff[iBar] ->Fill(tDiff,ampR);
	p_tR_vs_tDiff[iBar] ->Fill(tDiff,tR);
	h_tR_ampCorr[iBar]->Fill(tR_corr);
        p_tR_ampCorr_vs_amp[iBar]->Fill(ampR,tR_corr);
        p_tR_ampCorr_vs_posX[iBar]->Fill(posX,tR_corr);
	p_tR_ampCorr_vs_posXc[iBar]->Fill(posX/cos(theta[iBar]),tR_corr);
        p_tR_ampCorr_vs_posY[iBar]->Fill(posY,tR_corr);
        p_tR_ampCorr_vs_tDiff[iBar]->Fill(tDiff, tR_corr);


	// -- simple average
	p_tAve_vs_tDiff[iBar] ->Fill(tDiff, tAve);
	h_tAve_ampCorr[iBar]->Fill(tAve_ampCorr);
        p_tAve_ampCorr_vs_posX[iBar]->Fill(posX,tAve_ampCorr);
        p_tAve_ampCorr_vs_posXc[iBar]->Fill(posX/cos(theta[iBar]),tAve_ampCorr);
        p_tAve_ampCorr_vs_posY[iBar]->Fill(posY,tAve_ampCorr);
        p_tAve_ampCorr_vs_tDiff[iBar]->Fill(tDiff, tAve_ampCorr);

	int xbin = int ((posX-cut_Xmin[iBar])/xbinWidth );
	h_tL_ampCorr_binX[iBar][xbin] ->Fill(tL_corr);
	h_tR_ampCorr_binX[iBar][xbin] ->Fill(tR_corr);
	h_tAve_ampCorr_binX[iBar][xbin] ->Fill(tAve_ampCorr);

	// - amp weighted average
	h_tAveAmpW_ampCorr[iBar]->Fill(tAveAmpW_ampCorr);
        p_tAveAmpW_ampCorr_vs_posX[iBar]->Fill(posX,tAveAmpW_ampCorr);
        p_tAveAmpW_ampCorr_vs_posXc[iBar]->Fill(posX/cos(theta[iBar]),tAveAmpW_ampCorr);
        p_tAveAmpW_ampCorr_vs_posY[iBar]->Fill(posY,tAveAmpW_ampCorr);
        p_tAveAmpW_ampCorr_vs_tDiff[iBar]->Fill(tDiff, tAveAmpW_ampCorr);
        h_tAveAmpW_ampCorr_binX[iBar][xbin] ->Fill(tAveAmpW_ampCorr);
	
      } // -- end loop over bars

    }// -- end second loop over events

  



  // -- third loop over events -- compute tRes weighted average
  float tResL[NBARS];
  float tResR[NBARS];
  TF1 *fGaus = new TF1("fGaus","gaus",-10,10);
  for (int iBar = 0; iBar< NBARS; iBar++){
    float sigmaEff, sigmaGaus;
    GetTimeResolution(h_tL_ampCorr[iBar], sigmaEff, sigmaGaus, fGaus);
    //GetTimeResolution(h_tL_ampCorr_binX[iBar][int(NBINSX[iBar]/2)], sigmaEff, sigmaGaus, fGaus);
    tResL[iBar] = sigmaGaus;

    GetTimeResolution(h_tR_ampCorr[iBar], sigmaEff, sigmaGaus, fGaus);
    //GetTimeResolution(h_tR_ampCorr_binX[iBar][int(NBINSX[iBar]/2)], sigmaEff, sigmaGaus, fGaus);
    tResR[iBar] = sigmaGaus;
 
    cout << iBar << "   " << tResL[iBar] << "  " << tResR[iBar] <<endl;
  }

  std::cout<<" Third loop over events ..." <<std::endl;
  for(int entry = 0; entry < nEntries; ++entry)
    {
      if( entry%1000 == 0 ) std::cout << ">>> Reading entry " << entry << " / " << nEntries << "\r" << std::flush;
      chain1 -> GetEntry(entry);

      // -- position                                                                                                                                                                                                                          
      if (treeVars.t_ntracks != 1) continue;
      posX = (treeVars.t_trackFitResult->at(0)).x();
      posY = (treeVars.t_trackFitResult->at(0)).y();
      if (posX == -999. || posY == -999.) continue;

      // -- Photek as ref                                                                                                                                                                                                                     
      ampRef = treeVars.t_amp[(*treeVars.t_channelId)[ampChannelRef]] * kAdcToV ;
      tRef   = treeVars.t_time[(*treeVars.t_channelId)[timeChannelRef]+treeVars.t_CFD];


      // -- loop over bars                                                                                                                                                                                                                    
      for (int iBar = 0; iBar < NBARS; iBar++){

        ampL = treeVars.t_amp[(*treeVars.t_channelId)[ ampChannelsL[iBar] ]] * kAdcToV ;
        ampR = treeVars.t_amp[(*treeVars.t_channelId)[ ampChannelsR[iBar] ]] * kAdcToV ;

        tL   = treeVars.t_time[(*treeVars.t_channelId)[ timeChannelsL[iBar] ]+treeVars.t_LED];
        tR   = treeVars.t_time[(*treeVars.t_channelId)[ timeChannelsR[iBar] ]+treeVars.t_LED];

	tMaximumL = treeVars.t_time_maximum[(*treeVars.t_channelId)[ timeChannelsL[iBar] ]];
	tMaximumR = treeVars.t_time_maximum[(*treeVars.t_channelId)[ timeChannelsR[iBar] ]];

        totL = treeVars.t_time[(*treeVars.t_channelId)[ timeChannelsL[iBar] ]+treeVars.t_TED] - tL;
        totR = treeVars.t_time[(*treeVars.t_channelId)[ timeChannelsR[iBar] ]+treeVars.t_TED] - tR;

        // -- select events
        if ( posX < cut_Xmin[iBar] || posX > cut_Xmax[iBar] ) continue;
        if ( posY < cut_Ymin[iBar] || posY > cut_Ymax[iBar] ) continue;

        if ( ampRef < cut_ampMinRef     ||  ampRef > cut_ampMaxRef ) continue;
        if ( ampL   < cut_ampMinL[iBar] ||  ampL   > cut_ampMaxL[iBar] ) continue;
        if ( ampR   < cut_ampMinR[iBar] ||  ampR   > cut_ampMaxR[iBar] ) continue;

        if ( tRef < 0  ||  tRef > 200) continue;
        if ( tL   < 0  ||  tL   > 200) continue;
        if ( tR   < 0  ||  tR   > 200) continue;

        if ( tRef < tMinRef || tRef > tMaxRef ) continue;
        if ( tL   < tMinCh  || tL   > tMaxCh  ) continue;
        if ( tR   < tMinCh  || tR   > tMaxCh  ) continue;

        if ( fabs(tL - tMaximumL) > dtMaxCh) continue;
        if ( fabs(tR - tMaximumR) > dtMaxCh) continue;

        if ( totL < totMin || totL > totMax ) continue;
        if ( totR < totMin || totR > totMax ) continue;

	// -- amplitude walk correction
        tL = tL - tRef;
        tR = tR - tRef;
	float tL_corr = tL - fitFuncL_ampCorr[iBar]->Eval(ampL) + fitFuncL_ampCorr[iBar]->Eval(h_ampL[iBar]->GetMean()) ;
        float tR_corr = tR - fitFuncR_ampCorr[iBar]->Eval(ampR) + fitFuncR_ampCorr[iBar]->Eval(h_ampR[iBar]->GetMean()) ;
        float tDiff = tR_corr - tL_corr;
        float wL = 1./(tResL[iBar]*tResL[iBar]);
        float wR = 1./(tResR[iBar]*tResR[iBar]);
        float wSum = wL + wR;
	wL/=wSum;
        wR/=wSum;
        float tAveResW_ampCorr = wL*tL_corr + wR*tR_corr;


	int xbin = int ((posX-cut_Xmin[iBar])/xbinWidth );

        // -- tRes weighted average
        h_tAveResW_ampCorr[iBar]->Fill(tAveResW_ampCorr);
        p_tAveResW_ampCorr_vs_posX[iBar]->Fill(posX,tAveResW_ampCorr);
        p_tAveResW_ampCorr_vs_posXc[iBar]->Fill(posX/cos(theta[iBar]),tAveResW_ampCorr);
        p_tAveResW_ampCorr_vs_posY[iBar]->Fill(posY,tAveResW_ampCorr);
        p_tAveResW_ampCorr_vs_tDiff[iBar]->Fill(tDiff, tAveResW_ampCorr);
        h_tAveResW_ampCorr_binX[iBar][xbin] ->Fill(tAveResW_ampCorr);

      }

    } // -- end third loop over events


  // -- fourth loop over events - correction for residual tAve dependence on tDiff
  TF1*  fitFuncAve_tDiffCorr[NBARS];
  TF1*  fitFuncAveAmpW_tDiffCorr[NBARS];
  TF1*  fitFuncAveResW_tDiffCorr[NBARS];
  float mean, rms;
  float chi2ndf = 0;

  for (int iBar = 0 ; iBar < NBARS; iBar++){

    fitFuncAve_tDiffCorr[iBar] = new TF1(Form("fitFuncAve_tDiffCorr_%d", iBar),"pol4");
    mean = p_tAve_ampCorr_vs_tDiff[iBar] -> GetMean();
    rms  = p_tAve_ampCorr_vs_tDiff[iBar] -> GetRMS();
    fitFuncAve_tDiffCorr[iBar]->SetRange( mean - 2.5*rms , mean + 2.5*rms);
    fitFuncAve_tDiffCorr[iBar] -> SetLineColor(kRed);
    p_tAve_ampCorr_vs_tDiff[iBar] -> Fit(Form("fitFuncAve_tDiffCorr_%d",iBar),"QSR");
    chi2ndf = fitFuncAve_tDiffCorr[iBar] -> GetChisquare()/fitFuncAve_tDiffCorr[iBar] -> GetNDF();
    if (chi2ndf > 4.){
      fitFuncAve_tDiffCorr[iBar]->SetRange( mean - 1.0*rms , mean + 1.0*rms);
      p_tAve_ampCorr_vs_tDiff[iBar] -> Fit(Form("fitFuncAve_tDiffCorr_%d",iBar),"QSR");
    }
    

    fitFuncAveAmpW_tDiffCorr[iBar] = new TF1(Form("fitFuncAveAmpW_tDiffCorr_%d", iBar),"pol4");
    mean = p_tAveAmpW_ampCorr_vs_tDiff[iBar] -> GetMean();
    rms  = p_tAveAmpW_ampCorr_vs_tDiff[iBar] -> GetRMS();
    fitFuncAveAmpW_tDiffCorr[iBar]->SetRange( mean - 2.5*rms , mean + 2.5*rms);
    fitFuncAveAmpW_tDiffCorr[iBar] -> SetLineColor(kRed);
    p_tAveAmpW_ampCorr_vs_tDiff[iBar] -> Fit(Form("fitFuncAveAmpW_tDiffCorr_%d",iBar),"QSR");
    chi2ndf = fitFuncAveAmpW_tDiffCorr[iBar] -> GetChisquare()/fitFuncAveAmpW_tDiffCorr[iBar] -> GetNDF();
    if (chi2ndf > 4.){
      fitFuncAveAmpW_tDiffCorr[iBar]->SetRange( mean - 1.0*rms , mean + 1.0*rms);
      p_tAveAmpW_ampCorr_vs_tDiff[iBar] -> Fit(Form("fitFuncAveAmpW_tDiffCorr_%d",iBar),"QSR");
    }



    fitFuncAveResW_tDiffCorr[iBar] = new TF1(Form("fitFuncAveResW_tDiffCorr_%d", iBar),"pol4");
    mean = p_tAveResW_ampCorr_vs_tDiff[iBar] -> GetMean();
    rms  = p_tAveResW_ampCorr_vs_tDiff[iBar] -> GetRMS();
    fitFuncAveResW_tDiffCorr[iBar]->SetRange( mean - 2.5*rms , mean + 2.5*rms);
    fitFuncAveResW_tDiffCorr[iBar] -> SetLineColor(kRed);
    p_tAveResW_ampCorr_vs_tDiff[iBar] -> Fit(Form("fitFuncAveResW_tDiffCorr_%d",iBar),"QSR");
    chi2ndf = fitFuncAveResW_tDiffCorr[iBar] -> GetChisquare()/fitFuncAveResW_tDiffCorr[iBar] -> GetNDF();
    if (chi2ndf > 4.){
      fitFuncAveResW_tDiffCorr[iBar]->SetRange( mean - 1.0*rms , mean + 1.0*rms);
      p_tAveResW_ampCorr_vs_tDiff[iBar] -> Fit(Form("fitFuncAveResW_tDiffCorr_%d",iBar),"QSR");
    }

  }


  TF1*  fitFuncAve_posCorr[NBARS];
  for (int iBar = 0 ; iBar < NBARS; iBar++){
    fitFuncAve_posCorr[iBar] = new TF1(Form("fitFuncAve_posCorr_%d", iBar),"pol4");
    mean = p_tAve_ampCorr_vs_posX[iBar] -> GetMean();
    rms  = p_tAve_ampCorr_vs_posX[iBar] -> GetRMS();
    fitFuncAve_posCorr[iBar]->SetRange( mean - 2.5*rms , mean + 2.5*rms);
    fitFuncAve_posCorr[iBar] -> SetLineColor(kRed);
    p_tAve_ampCorr_vs_posX[iBar] -> Fit(Form("fitFuncAve_posCorr_%d",iBar),"QSR");
    chi2ndf = fitFuncAve_posCorr[iBar] -> GetChisquare()/fitFuncAve_posCorr[iBar] -> GetNDF();
    if (chi2ndf > 4.){
      fitFuncAve_posCorr[iBar]->SetRange( mean - 1.0*rms , mean + 1.0*rms);
      p_tAve_ampCorr_vs_posX[iBar] -> Fit(Form("fitFuncAve_posCorr_%d",iBar),"QSR");
    }
  }

  std::cout<<"Fourth loop over events ..." <<std::endl;
  for(int entry = 0; entry < nEntries; ++entry)
    {
      if( entry%1000 == 0 ) std::cout << ">>> Reading entry " << entry << " / " << nEntries << "\r" << std::flush;
      chain1 -> GetEntry(entry);

      // -- position
      if (treeVars.t_ntracks != 1) continue;
      posX = (treeVars.t_trackFitResult->at(0)).x();
      posY = (treeVars.t_trackFitResult->at(0)).y();
      if (posX == -999. || posY == -999.) continue;

      // -- Photek as ref
      ampRef = treeVars.t_amp[(*treeVars.t_channelId)[ampChannelRef]] * kAdcToV ;
      tRef   = treeVars.t_time[(*treeVars.t_channelId)[timeChannelRef]+treeVars.t_CFD];


      // -- loop over bars
      for (int iBar = 0; iBar < NBARS; iBar++){
	
        ampL = treeVars.t_amp[(*treeVars.t_channelId)[ ampChannelsL[iBar] ]] * kAdcToV ;
        ampR = treeVars.t_amp[(*treeVars.t_channelId)[ ampChannelsR[iBar] ]] * kAdcToV ;
	
        tL   = treeVars.t_time[(*treeVars.t_channelId)[ timeChannelsL[iBar] ]+treeVars.t_LED];
        tR   = treeVars.t_time[(*treeVars.t_channelId)[ timeChannelsR[iBar] ]+treeVars.t_LED];

        tMaximumL = treeVars.t_time_maximum[(*treeVars.t_channelId)[ timeChannelsL[iBar] ]];
        tMaximumR = treeVars.t_time_maximum[(*treeVars.t_channelId)[ timeChannelsR[iBar] ]];

        totL = treeVars.t_time[(*treeVars.t_channelId)[ timeChannelsL[iBar] ]+treeVars.t_TED] - tL;
        totR = treeVars.t_time[(*treeVars.t_channelId)[ timeChannelsR[iBar] ]+treeVars.t_TED] - tR;

        // -- select events
        if ( posX < cut_Xmin[iBar] || posX > cut_Xmax[iBar] ) continue;
        if ( posY < cut_Ymin[iBar] || posY > cut_Ymax[iBar] ) continue;
	
        if ( ampRef < cut_ampMinRef     ||  ampRef > cut_ampMaxRef ) continue;
        if ( ampL   < cut_ampMinL[iBar] ||  ampL   > cut_ampMaxL[iBar] ) continue;
        if ( ampR   < cut_ampMinR[iBar] ||  ampR   > cut_ampMaxR[iBar] ) continue;

        if ( tRef < 0  ||  tRef > 200) continue;
        if ( tL   < 0  ||  tL   > 200) continue;
        if ( tR   < 0  ||  tR   > 200) continue;

        if ( tRef < tMinRef || tRef > tMaxRef ) continue;
        if ( tL   < tMinCh  || tL   > tMaxCh  ) continue;
        if ( tR   < tMinCh  || tR   > tMaxCh  ) continue;

        if ( fabs(tL - tMaximumL) > dtMaxCh) continue;
        if ( fabs(tR - tMaximumR) > dtMaxCh) continue;

        if ( totL < totMin || totL > totMax ) continue;
        if ( totR < totMin || totR > totMax ) continue;

        // -- amplitude walk correction
        tL = tL - tRef;
        tR = tR - tRef;
	float ampLsm  = ampL + rndL[iBar][entry];
        float ampRsm  = ampR + rndR[iBar][entry];
	float tL_corr = tL - fitFuncL_ampCorr[iBar]->Eval(ampLsm) + fitFuncL_ampCorr[iBar]->Eval(h_ampL[iBar]->GetMean()) ; 
	float tR_corr = tR - fitFuncR_ampCorr[iBar]->Eval(ampRsm) + fitFuncR_ampCorr[iBar]->Eval(h_ampR[iBar]->GetMean()) ; 
        //float tL_corr = tL - fitFuncL_ampCorr[iBar]->Eval(ampL) + fitFuncL_ampCorr[iBar]->Eval(h_ampL[iBar]->GetMean()) ;
        //float tR_corr = tR - fitFuncR_ampCorr[iBar]->Eval(ampR) + fitFuncR_ampCorr[iBar]->Eval(h_ampR[iBar]->GetMean()) ;
        float tAve_ampCorr = ( tL_corr + tR_corr)/2 ;
        float tDiff = tR_corr - tL_corr;
	float wL = ampL/(ampL+ampR);
        float wR = ampR/(ampL+ampR);
        float tAveAmpW_ampCorr = wL*tL_corr + wR*tR_corr;

	float wL2 = 1./(tResL[iBar]*tResL[iBar]);
	float wR2 = 1./(tResR[iBar]*tResR[iBar]);
	float wSum = wL2 + wR2;
	wL2/=wSum;
	wR2/=wSum;
        float tAveResW_ampCorr = wL2*tL_corr + wR2*tR_corr;

	// -- correction for residual dependence on tDiff
	float tAve_ampCorr_tDiffCorr = tAve_ampCorr - fitFuncAve_tDiffCorr[iBar]->Eval(tDiff) + fitFuncAve_tDiffCorr[iBar]->Eval( p_tAve_ampCorr_vs_tDiff[iBar]->GetMean() ) ;
	float tAveAmpW_ampCorr_tDiffCorr = tAveAmpW_ampCorr - fitFuncAveAmpW_tDiffCorr[iBar]->Eval(tDiff) + fitFuncAveAmpW_tDiffCorr[iBar]->Eval( p_tAveAmpW_ampCorr_vs_tDiff[iBar]->GetMean() ) ;
	float tAveResW_ampCorr_tDiffCorr = tAveResW_ampCorr - fitFuncAveResW_tDiffCorr[iBar]->Eval(tDiff) + fitFuncAveResW_tDiffCorr[iBar]->Eval( p_tAveResW_ampCorr_vs_tDiff[iBar]->GetMean() ) ;

	// -- correction for residual dependence on posX
	float tAve_ampCorr_posCorr = tAve_ampCorr - fitFuncAve_posCorr[iBar]->Eval(posX) + fitFuncAve_posCorr[iBar]->Eval( p_tAve_ampCorr_vs_posX[iBar]->GetMean() ) ;

	int xbin = int ((posX-cut_Xmin[iBar])/xbinWidth );

	// -- simple average
        h_tAve_ampCorr_tDiffCorr[iBar]->Fill(tAve_ampCorr_tDiffCorr);
        p_tAve_ampCorr_tDiffCorr_vs_posX[iBar]->Fill(posX,tAve_ampCorr_tDiffCorr);
        p_tAve_ampCorr_tDiffCorr_vs_posXc[iBar]->Fill(posX/cos(theta[iBar]),tAve_ampCorr_tDiffCorr);
        p_tAve_ampCorr_tDiffCorr_vs_posY[iBar]->Fill(posY,tAve_ampCorr_tDiffCorr);
        p_tAve_ampCorr_tDiffCorr_vs_tDiff[iBar]->Fill(tDiff, tAve_ampCorr_tDiffCorr);
	h_tAve_ampCorr_tDiffCorr_binX[iBar][xbin] ->Fill(tAve_ampCorr_tDiffCorr);

	// -- amp weighted average
        h_tAveAmpW_ampCorr_tDiffCorr[iBar]->Fill(tAveAmpW_ampCorr_tDiffCorr);
        p_tAveAmpW_ampCorr_tDiffCorr_vs_posX[iBar]->Fill(posX,tAveAmpW_ampCorr_tDiffCorr);
        p_tAveAmpW_ampCorr_tDiffCorr_vs_posXc[iBar]->Fill(posX/cos(theta[iBar]),tAveAmpW_ampCorr_tDiffCorr);
        p_tAveAmpW_ampCorr_tDiffCorr_vs_posY[iBar]->Fill(posY,tAveAmpW_ampCorr_tDiffCorr);
        p_tAveAmpW_ampCorr_tDiffCorr_vs_tDiff[iBar]->Fill(tDiff, tAveAmpW_ampCorr_tDiffCorr);
        h_tAveAmpW_ampCorr_tDiffCorr_binX[iBar][xbin] ->Fill(tAveAmpW_ampCorr_tDiffCorr);

	// -- tRes weighted average
        h_tAveResW_ampCorr_tDiffCorr[iBar]->Fill(tAveResW_ampCorr_tDiffCorr);
        p_tAveResW_ampCorr_tDiffCorr_vs_posX[iBar]->Fill(posX,tAveResW_ampCorr_tDiffCorr);
        p_tAveResW_ampCorr_tDiffCorr_vs_posXc[iBar]->Fill(posX/cos(theta[iBar]),tAveResW_ampCorr_tDiffCorr);
        p_tAveResW_ampCorr_tDiffCorr_vs_posY[iBar]->Fill(posY,tAveResW_ampCorr_tDiffCorr);
        p_tAveResW_ampCorr_tDiffCorr_vs_tDiff[iBar]->Fill(tDiff, tAveResW_ampCorr_tDiffCorr);
        h_tAveResW_ampCorr_tDiffCorr_binX[iBar][xbin] ->Fill(tAveResW_ampCorr_tDiffCorr);


	// -- simple average vs posX
        h_tAve_ampCorr_posCorr[iBar]->Fill(tAve_ampCorr_posCorr);
        p_tAve_ampCorr_posCorr_vs_posX[iBar]->Fill(posX,tAve_ampCorr_posCorr);
        p_tAve_ampCorr_posCorr_vs_posXc[iBar]->Fill(posX/cos(theta[iBar]),tAve_ampCorr_posCorr);
        p_tAve_ampCorr_posCorr_vs_posY[iBar]->Fill(posY,tAve_ampCorr_posCorr);
        p_tAve_ampCorr_posCorr_vs_tDiff[iBar]->Fill(tDiff, tAve_ampCorr_posCorr);
      
      }
    
    } //--- end fourth loop over events



  cout << "Calculating time rsolution..." <<endl;
  // -- Compute resolutions (gaussian and effective sigma)
  TF1*  fitFunL_ampCorr[NBARS];
  TF1*  fitFunR_ampCorr[NBARS];

  TF1*  fitFunAve_ampCorr[NBARS];
  TF1*  fitFunAve_ampCorr_tDiffCorr[NBARS];
  TF1*  fitFunAve_ampCorr_posCorr[NBARS];

  TF1*  fitFunAveAmpW_ampCorr[NBARS];
  TF1*  fitFunAveAmpW_ampCorr_tDiffCorr[NBARS];

  TF1*  fitFunAveResW_ampCorr[NBARS];
  TF1*  fitFunAveResW_ampCorr_tDiffCorr[NBARS];

  TF1*  fitFunL_ampCorr_binX[NBARS][50];
  TF1*  fitFunR_ampCorr_binX[NBARS][50];

  TF1*  fitFunAve_ampCorr_binX[NBARS][50];
  TF1*  fitFunAve_ampCorr_tDiffCorr_binX[NBARS][50];
  TF1*  fitFunAveAmpW_ampCorr_binX[NBARS][50];
  TF1*  fitFunAveAmpW_ampCorr_tDiffCorr_binX[NBARS][50];
  TF1*  fitFunAveResW_ampCorr_binX[NBARS][50];
  TF1*  fitFunAveResW_ampCorr_tDiffCorr_binX[NBARS][50];

  TH1F*  h_effectiveSigmaL[NBARS];
  TH1F*  h_effectiveSigmaL_ampCorr[NBARS];
  TH1F*  h_effectiveSigmaR[NBARS];
  TH1F*  h_effectiveSigmaR_ampCorr[NBARS];
  TH1F*  h_effectiveSigmaAve[NBARS];
  TH1F*  h_effectiveSigmaAve_ampCorr[NBARS];
  TH1F*  h_effectiveSigmaAve_ampCorr_tDiffCorr[NBARS];
  TH1F*  h_effectiveSigmaAve_ampCorr_posCorr[NBARS];
  TH1F*  h_effectiveSigmaAveAmpW_ampCorr[NBARS];
  TH1F*  h_effectiveSigmaAveAmpW_ampCorr_tDiffCorr[NBARS];
  TH1F*  h_effectiveSigmaAveResW_ampCorr[NBARS];
  TH1F*  h_effectiveSigmaAveResW_ampCorr_tDiffCorr[NBARS];

  TGraphErrors *g_tResolGausL_ampCorr[NBARS];
  TGraphErrors *g_tResolGausR_ampCorr[NBARS];
  TGraphErrors *g_tResolGausAve_ampCorr[NBARS];
  TGraphErrors *g_tResolGausAve_ampCorr_tDiffCorr[NBARS];
  TGraphErrors *g_tResolGausAveAmpW_ampCorr[NBARS];
  TGraphErrors *g_tResolGausAveAmpW_ampCorr_tDiffCorr[NBARS];
  TGraphErrors *g_tResolGausAveResW_ampCorr[NBARS];
  TGraphErrors *g_tResolGausAveResW_ampCorr_tDiffCorr[NBARS];

  float* vals = new float[4];
  float sigmaEff;
  float sigmaGaus;   
  float sigmaGausErr;
  float resolEff;
  float resolGaus;
  float resolGausErr;
  float resolMCP = 0.017; 
  
  TF1 *fpol0_tL_ampCorr[NBARS]; 
  TF1 *fpol0_tR_ampCorr[NBARS]; 

  TF1 *fpol0_tAve_ampCorr[NBARS]; 
  TF1 *fpol0_tAve_ampCorr_tDiffCorr[NBARS]; 

  TF1 *fpol0_tAveAmpW_ampCorr[NBARS]; 
  TF1 *fpol0_tAveAmpW_ampCorr_tDiffCorr[NBARS]; 

  TF1 *fpol0_tAveResW_ampCorr[NBARS]; 
  TF1 *fpol0_tAveResW_ampCorr_tDiffCorr[NBARS]; 

  for (int iBar = 0; iBar < NBARS; iBar++){
    h_effectiveSigmaL[iBar]  = new TH1F(Form("h_effectiveSigmaL_BAR%d",iBar),Form("h_effectiveSigmaL_BAR%d",iBar),2000,0.,200.);
    h_effectiveSigmaL_ampCorr[iBar]  = new TH1F(Form("h_effectiveSigmaL_ampCorr_BAR%d",iBar),Form("h_effectiveSigmaL_ampCorr_BAR%d",iBar), 2000,0.,200.);

    h_effectiveSigmaR[iBar]  = new TH1F(Form("h_effectiveSigmaR_BAR%d",iBar),Form("h_effectiveSigmaR_BAR%d",iBar),2000,0.,200.);
    h_effectiveSigmaR_ampCorr[iBar]  = new TH1F(Form("h_effectiveSigmaR_ampCorr_BAR%d",iBar),Form("h_effectiveSigmaR_ampCorr_BAR%d",iBar), 2000,0.,200.);

    h_effectiveSigmaAve[iBar]  = new TH1F(Form("h_effectiveSigmaAve_BAR%d",iBar),Form("h_effectiveSigmaAve_BAR%d",iBar),2000,0.,200.);

    h_effectiveSigmaAve_ampCorr[iBar]  = new TH1F(Form("h_effectiveSigmaAve_ampCorr_BAR%d",iBar),Form("h_effectiveSigmaAve_ampCorr_BAR%d",iBar), 2000,0.,200.);
    h_effectiveSigmaAve_ampCorr_tDiffCorr[iBar]  = new TH1F(Form("h_effectiveSigmaAve_ampCorr_tDiffCorr_BAR%d",iBar),Form("h_effectiveSigmaAve_ampCorr_tDiffCorr_BAR%d",iBar), 2000,0.,200.);
    h_effectiveSigmaAve_ampCorr_posCorr[iBar]  = new TH1F(Form("h_effectiveSigmaAve_ampCorr_posCorr_BAR%d",iBar),Form("h_effectiveSigmaAve_ampCorr_posCorr_BAR%d",iBar), 2000,0.,200.);

    h_effectiveSigmaAveAmpW_ampCorr[iBar]  = new TH1F(Form("h_effectiveSigmaAveAmpW_ampCorr_BAR%d",iBar),Form("h_effectiveSigmaAveAmpW_ampCorr_BAR%d",iBar), 2000,0.,200.);
    h_effectiveSigmaAveAmpW_ampCorr_tDiffCorr[iBar]  = new TH1F(Form("h_effectiveSigmaAveAmpW_ampCorr_tDiffCorr_BAR%d",iBar),Form("h_effectiveSigmaAveAmpW_tDiffCorr_ampCorr_BAR%d",iBar), 2000,0.,200.);

    h_effectiveSigmaAveResW_ampCorr[iBar]  = new TH1F(Form("h_effectiveSigmaAveResW_ampCorr_BAR%d",iBar),Form("h_effectiveSigmaAveResW_ampCorr_BAR%d",iBar), 2000,0.,200.);
    h_effectiveSigmaAveResW_ampCorr_tDiffCorr[iBar]  = new TH1F(Form("h_effectiveSigmaAveResW_ampCorr_tDiffCorr_BAR%d",iBar),Form("h_effectiveSigmaAveResW_tDiffCorr_ampCorr_BAR%d",iBar), 2000,0.,200.);


    // -- Effective sigma no corrections
    FindSmallestInterval(vals,h_tL[iBar],0.68,true);
    sigmaEff = 0.5*(vals[3]-vals[2]);
    h_effectiveSigmaL[iBar]->Fill(sigmaEff);

    FindSmallestInterval(vals,h_tR[iBar],0.68,true);
    sigmaEff = 0.5*(vals[3]-vals[2]);
    h_effectiveSigmaR[iBar]->Fill(sigmaEff);

    FindSmallestInterval(vals,h_tAve[iBar],0.68,true);
    sigmaEff = 0.5*(vals[3]-vals[2]);
    h_effectiveSigmaAve[iBar]->Fill(sigmaEff);

    
    // ----- amp walk corr
    
    // -- Left
    fitFunL_ampCorr[iBar]= new TF1(Form("fitFunL_ampCorr_BAR%d",iBar),"gaus", -10, 10);
    GetTimeResolution(h_tL_ampCorr[iBar], sigmaEff, sigmaGaus, fitFunL_ampCorr[iBar]);
    h_effectiveSigmaL_ampCorr[iBar]->Fill(sigmaEff);
    
    // -- right
    fitFunR_ampCorr[iBar]= new TF1(Form("fitFunR_ampCorr_BAR%d",iBar),"gaus", -10, 10);
    GetTimeResolution(h_tR_ampCorr[iBar], sigmaEff, sigmaGaus, fitFunR_ampCorr[iBar]);
    h_effectiveSigmaR_ampCorr[iBar]->Fill(sigmaEff);

    // -- simple average
    fitFunAve_ampCorr[iBar]= new TF1(Form("fitFunAve_ampCorr_BAR%d",iBar),"gaus", -10 , 10);
    GetTimeResolution(h_tAve_ampCorr[iBar], sigmaEff, sigmaGaus, fitFunAve_ampCorr[iBar]);
    h_effectiveSigmaAve_ampCorr[iBar]->Fill(sigmaEff);

    // -- amp weighted average
    fitFunAveAmpW_ampCorr[iBar]= new TF1(Form("fitFunAveAmpW_ampCorr_BAR%d",iBar),"gaus", -10 , 10);
    GetTimeResolution(h_tAveAmpW_ampCorr[iBar], sigmaEff, sigmaGaus, fitFunAveAmpW_ampCorr[iBar]);
    h_effectiveSigmaAveAmpW_ampCorr[iBar]->Fill(sigmaEff);

    // -- res weighted average
    fitFunAveResW_ampCorr[iBar]= new TF1(Form("fitFunAveResW_ampCorr_BAR%d",iBar),"gaus", -10 , 10);
    GetTimeResolution(h_tAveResW_ampCorr[iBar], sigmaEff, sigmaGaus, fitFunAveResW_ampCorr[iBar]);
    h_effectiveSigmaAveResW_ampCorr[iBar]->Fill(sigmaEff);


    // ----- tDiff corr
    // -- simple average
    fitFunAve_ampCorr_tDiffCorr[iBar]= new TF1(Form("fitFunAve_ampCorr_tDiffCorr_BAR%d",iBar),"gaus", -10 , 10);
    GetTimeResolution(h_tAve_ampCorr_tDiffCorr[iBar], sigmaEff, sigmaGaus, fitFunAve_ampCorr_tDiffCorr[iBar]);
    h_effectiveSigmaAve_ampCorr_tDiffCorr[iBar]->Fill(sigmaEff);

    // -- amp weighted average
    fitFunAveAmpW_ampCorr_tDiffCorr[iBar]= new TF1(Form("fitFunAveAmpW_ampCorr_tDiffCorr_BAR%d",iBar),"gaus", -10 , 10);
    GetTimeResolution(h_tAveAmpW_ampCorr_tDiffCorr[iBar], sigmaEff, sigmaGaus, fitFunAveAmpW_ampCorr_tDiffCorr[iBar]);
    h_effectiveSigmaAveAmpW_ampCorr_tDiffCorr[iBar]->Fill(sigmaEff);

    // -- tRes weighted average
    fitFunAveResW_ampCorr_tDiffCorr[iBar]= new TF1(Form("fitFunAveResW_ampCorr_tDiffCorr_BAR%d",iBar),"gaus", -10 , 10);
    GetTimeResolution(h_tAveResW_ampCorr_tDiffCorr[iBar], sigmaEff, sigmaGaus, fitFunAveResW_ampCorr_tDiffCorr[iBar]);
    h_effectiveSigmaAveResW_ampCorr_tDiffCorr[iBar]->Fill(sigmaEff);

    // ----- position corr
    // -- simple average
    fitFunAve_ampCorr_posCorr[iBar]= new TF1(Form("fitFunAve_ampCorr_posCorr_BAR%d",iBar),"gaus", -10 , 10);
    GetTimeResolution(h_tAve_ampCorr_posCorr[iBar], sigmaEff, sigmaGaus, fitFunAve_ampCorr_posCorr[iBar]);
    h_effectiveSigmaAve_ampCorr_posCorr[iBar]->Fill(sigmaEff);


    // tResol graphs in bins in x
    g_tResolGausL_ampCorr[iBar] = new TGraphErrors();
    g_tResolGausL_ampCorr[iBar]->SetName(Form("g_tResolGausL_ampCorr_BAR%d",iBar));

    g_tResolGausR_ampCorr[iBar] = new TGraphErrors();
    g_tResolGausR_ampCorr[iBar]->SetName(Form("g_tResolGausR_ampCorr_BAR%d",iBar));

    g_tResolGausAve_ampCorr[iBar] = new TGraphErrors();
    g_tResolGausAve_ampCorr[iBar]->SetName(Form("g_tResolGausAve_ampCorr_BAR%d",iBar));

    g_tResolGausAve_ampCorr_tDiffCorr[iBar] = new TGraphErrors();
    g_tResolGausAve_ampCorr_tDiffCorr[iBar]->SetName(Form("g_tResolGausAve_ampCorr_tDiffCorr_BAR%d",iBar));

    g_tResolGausAveAmpW_ampCorr[iBar] = new TGraphErrors();
    g_tResolGausAveAmpW_ampCorr[iBar]->SetName(Form("g_tResolGausAveAmpW_ampCorr_BAR%d",iBar));

    g_tResolGausAveAmpW_ampCorr_tDiffCorr[iBar] = new TGraphErrors();
    g_tResolGausAveAmpW_ampCorr_tDiffCorr[iBar]->SetName(Form("g_tResolGausAveAmpW_ampCorr_tDiffCorr_BAR%d",iBar));

    g_tResolGausAveResW_ampCorr[iBar] = new TGraphErrors();
    g_tResolGausAveResW_ampCorr[iBar]->SetName(Form("g_tResolGausAveResW_ampCorr_BAR%d",iBar));

    g_tResolGausAveResW_ampCorr_tDiffCorr[iBar] = new TGraphErrors();
    g_tResolGausAveResW_ampCorr_tDiffCorr[iBar]->SetName(Form("g_tResolGausAveResW_ampCorr_tDiffCorr_BAR%d",iBar));

    
    for (int ibin = 0; ibin < NBINSX[iBar]; ibin++){

      float xx = cut_Xmin[iBar] + xbinWidth*ibin + xbinWidth/2;

      // ----- amp walk corr

      // -- Left
      fitFunL_ampCorr_binX[iBar][ibin]= new TF1(Form("fitFunL_ampCorr_binX%d_BAR%d",ibin,iBar),"gaus", -10, 10); 
      GetTimeResolution(h_tL_ampCorr_binX[iBar][ibin], sigmaEff, sigmaGaus, fitFunL_ampCorr_binX[iBar][ibin]);

      sigmaGaus    = fitFunL_ampCorr_binX[iBar][ibin]->GetParameter(2);
      sigmaGausErr = fitFunL_ampCorr_binX[iBar][ibin]->GetParError(2);
      resolEff  = sqrt(sigmaEff  * sigmaEff  - resolMCP * resolMCP);
      resolGaus = sqrt(sigmaGaus * sigmaGaus - resolMCP * resolMCP); 
      resolGausErr = sigmaGausErr/resolGaus * sigmaGaus;

      g_tResolGausL_ampCorr[iBar]->SetPoint(ibin, xx, resolGaus*1000.);
      g_tResolGausL_ampCorr[iBar]->SetPointError(ibin, xbinWidth/2., resolGausErr*1000.);

      // -- Right
      fitFunR_ampCorr_binX[iBar][ibin]= new TF1(Form("fitFunR_ampCorr_binX%d_BAR%d",ibin,iBar),"gaus", -10, 10); 
      GetTimeResolution(h_tR_ampCorr_binX[iBar][ibin], sigmaEff, sigmaGaus, fitFunR_ampCorr_binX[iBar][ibin]);

      sigmaGaus    = fitFunR_ampCorr_binX[iBar][ibin]->GetParameter(2);
      sigmaGausErr = fitFunR_ampCorr_binX[iBar][ibin]->GetParError(2);
      resolEff  = sqrt(sigmaEff*sigmaEff - resolMCP*resolMCP);
      resolGaus = sqrt(sigmaGaus*sigmaGaus - resolMCP*resolMCP);
      resolGausErr = sigmaGausErr/resolGaus * sigmaGaus;

      g_tResolGausR_ampCorr[iBar]->SetPoint(ibin, xx, resolGaus*1000.);
      g_tResolGausR_ampCorr[iBar]->SetPointError(ibin, xbinWidth/2., resolGausErr*1000.);

      // -- simple average
      fitFunAve_ampCorr_binX[iBar][ibin]= new TF1(Form("fitFunAve_ampCorr_binX%d_BAR%d",ibin,iBar),"gaus", -10, 10); 
      GetTimeResolution(h_tAve_ampCorr_binX[iBar][ibin], sigmaEff, sigmaGaus, fitFunAve_ampCorr_binX[iBar][ibin]);

      sigmaGaus    = fitFunAve_ampCorr_binX[iBar][ibin]->GetParameter(2);
      sigmaGausErr = fitFunAve_ampCorr_binX[iBar][ibin]->GetParError(2);
      resolEff  = sqrt(sigmaEff*sigmaEff - resolMCP*resolMCP);
      resolGaus = sqrt(sigmaGaus*sigmaGaus - resolMCP*resolMCP);
      resolGausErr = sigmaGausErr/resolGaus * sigmaGaus;

      g_tResolGausAve_ampCorr[iBar]->SetPoint(ibin, xx, resolGaus*1000.);
      g_tResolGausAve_ampCorr[iBar]->SetPointError(ibin, xbinWidth/2., resolGausErr*1000.);


      // -- amp weighted Ave
      fitFunAveAmpW_ampCorr_binX[iBar][ibin]= new TF1(Form("fitFunAveAmpW_ampCorr_binX%d_BAR%d",ibin,iBar),"gaus", -10, 10); 
      GetTimeResolution(h_tAveAmpW_ampCorr_binX[iBar][ibin], sigmaEff, sigmaGaus, fitFunAveAmpW_ampCorr_binX[iBar][ibin]);

      sigmaGaus    = fitFunAveAmpW_ampCorr_binX[iBar][ibin]->GetParameter(2);
      sigmaGausErr = fitFunAveAmpW_ampCorr_binX[iBar][ibin]->GetParError(2);
      resolEff  = sqrt(sigmaEff*sigmaEff - resolMCP*resolMCP);
      resolGaus = sqrt(sigmaGaus*sigmaGaus - resolMCP*resolMCP);
      resolGausErr = sigmaGausErr/resolGaus * sigmaGaus;

      g_tResolGausAveAmpW_ampCorr[iBar]->SetPoint(ibin, xx, resolGaus*1000.);
      g_tResolGausAveAmpW_ampCorr[iBar]->SetPointError(ibin, xbinWidth/2., resolGausErr*1000.);

      // -- tRes weighted Ave
      fitFunAveResW_ampCorr_binX[iBar][ibin]= new TF1(Form("fitFunAveResW_ampCorr_binX%d_BAR%d",ibin,iBar),"gaus", -10, 10); 
      GetTimeResolution(h_tAveResW_ampCorr_binX[iBar][ibin], sigmaEff, sigmaGaus, fitFunAveResW_ampCorr_binX[iBar][ibin]);

      sigmaGaus    = fitFunAveResW_ampCorr_binX[iBar][ibin]->GetParameter(2);
      sigmaGausErr = fitFunAveResW_ampCorr_binX[iBar][ibin]->GetParError(2);
      resolEff  = sqrt(sigmaEff*sigmaEff - resolMCP*resolMCP);
      resolGaus = sqrt(sigmaGaus*sigmaGaus - resolMCP*resolMCP);
      resolGausErr = sigmaGausErr/resolGaus * sigmaGaus;

      g_tResolGausAveResW_ampCorr[iBar]->SetPoint(ibin, xx, resolGaus*1000.);
      g_tResolGausAveResW_ampCorr[iBar]->SetPointError(ibin, xbinWidth/2., resolGausErr*1000.);

      

      // ----- tDiff cor

      // -- simple average
      fitFunAve_ampCorr_tDiffCorr_binX[iBar][ibin]= new TF1(Form("fitFunAve_ampCorr_tDiffCorr_binX%d_BAR%d",ibin,iBar),"gaus", -10, 10);
      GetTimeResolution(h_tAve_ampCorr_tDiffCorr_binX[iBar][ibin], sigmaEff, sigmaGaus, fitFunAve_ampCorr_tDiffCorr_binX[iBar][ibin]);

      sigmaGaus    = fitFunAve_ampCorr_tDiffCorr_binX[iBar][ibin]->GetParameter(2);
      sigmaGausErr = fitFunAve_ampCorr_tDiffCorr_binX[iBar][ibin]->GetParError(2);
      resolEff  = sqrt(sigmaEff*sigmaEff - resolMCP*resolMCP);
      resolGaus = sqrt(sigmaGaus*sigmaGaus - resolMCP*resolMCP);
      resolGausErr = sigmaGausErr/resolGaus * sigmaGaus;

      g_tResolGausAve_ampCorr_tDiffCorr[iBar]->SetPoint(ibin, xx, resolGaus*1000.);
      g_tResolGausAve_ampCorr_tDiffCorr[iBar]->SetPointError(ibin, xbinWidth/2., resolGausErr*1000.);


      // -- amp weighted Ave                                                                                                                                                                                                              
      fitFunAveAmpW_ampCorr_tDiffCorr_binX[iBar][ibin]= new TF1(Form("fitFunAveAmpW_ampCorr_tDiffCorr_binX%d_BAR%d",ibin,iBar),"gaus", -10, 10);
      GetTimeResolution(h_tAveAmpW_ampCorr_tDiffCorr_binX[iBar][ibin], sigmaEff, sigmaGaus, fitFunAveAmpW_ampCorr_tDiffCorr_binX[iBar][ibin]);

      sigmaGaus    = fitFunAveAmpW_ampCorr_tDiffCorr_binX[iBar][ibin]->GetParameter(2);
      sigmaGausErr = fitFunAveAmpW_ampCorr_tDiffCorr_binX[iBar][ibin]->GetParError(2);
      resolEff  = sqrt(sigmaEff*sigmaEff - resolMCP*resolMCP);
      resolGaus = sqrt(sigmaGaus*sigmaGaus - resolMCP*resolMCP);
      resolGausErr = sigmaGausErr/resolGaus * sigmaGaus;

      g_tResolGausAveAmpW_ampCorr_tDiffCorr[iBar]->SetPoint(ibin, xx, resolGaus*1000.);
      g_tResolGausAveAmpW_ampCorr_tDiffCorr[iBar]->SetPointError(ibin, xbinWidth/2., resolGausErr*1000.);


      // -- res weighted Ave                                                                                                                                                                                                              
      fitFunAveResW_ampCorr_tDiffCorr_binX[iBar][ibin]= new TF1(Form("fitFunAveResW_ampCorr_tDiffCorr_binX%d_BAR%d",ibin,iBar),"gaus", -10, 10);
      GetTimeResolution(h_tAveResW_ampCorr_tDiffCorr_binX[iBar][ibin], sigmaEff, sigmaGaus, fitFunAveResW_ampCorr_tDiffCorr_binX[iBar][ibin]);

      sigmaGaus    = fitFunAveResW_ampCorr_tDiffCorr_binX[iBar][ibin]->GetParameter(2);
      sigmaGausErr = fitFunAveResW_ampCorr_tDiffCorr_binX[iBar][ibin]->GetParError(2);
      resolEff  = sqrt(sigmaEff*sigmaEff - resolMCP*resolMCP);
      resolGaus = sqrt(sigmaGaus*sigmaGaus - resolMCP*resolMCP);
      resolGausErr = sigmaGausErr/resolGaus * sigmaGaus;

      g_tResolGausAveResW_ampCorr_tDiffCorr[iBar]->SetPoint(ibin, xx, resolGaus*1000.);
      g_tResolGausAveResW_ampCorr_tDiffCorr[iBar]->SetPointError(ibin, xbinWidth/2., resolGausErr*1000.);

    }



    /// -- fit with pol0 resol vs X 
    fpol0_tL_ampCorr[iBar] = new TF1(Form("fpol0_tL_ampCorr_BAR%d",iBar),"pol0",-50,50);
    fpol0_tL_ampCorr[iBar]->SetLineStyle(2);
    fpol0_tL_ampCorr[iBar]->SetLineWidth(2);    
    fpol0_tL_ampCorr[iBar]->SetLineColor(4);
    g_tResolGausL_ampCorr[iBar]->Fit(fpol0_tL_ampCorr[iBar],"QRS");

    fpol0_tR_ampCorr[iBar] = new TF1(Form("fpol0_tR_ampCorr_BAR%d",iBar),"pol0",-50,50);
    fpol0_tR_ampCorr[iBar]->SetLineStyle(2);
    fpol0_tR_ampCorr[iBar]->SetLineWidth(2);    
    fpol0_tR_ampCorr[iBar]->SetLineColor(2);
    g_tResolGausR_ampCorr[iBar]->Fit(fpol0_tR_ampCorr[iBar],"QRS");


    fpol0_tAve_ampCorr[iBar] = new TF1(Form("fpol0_tAve_ampCorr_BAR%d",iBar),"pol0",-50,50);
    fpol0_tAve_ampCorr[iBar]->SetLineStyle(2);
    fpol0_tAve_ampCorr[iBar]->SetLineWidth(2);    
    fpol0_tAve_ampCorr[iBar]->SetLineColor(1);
    g_tResolGausAve_ampCorr[iBar]->Fit(fpol0_tAve_ampCorr[iBar],"QRS");

    fpol0_tAve_ampCorr_tDiffCorr[iBar] = new TF1(Form("fpol0_tAve_ampCorr_tDiffCorr_BAR%d",iBar),"pol0",-50,50);
    fpol0_tAve_ampCorr_tDiffCorr[iBar]->SetLineStyle(2);
    fpol0_tAve_ampCorr_tDiffCorr[iBar]->SetLineWidth(2);    
    fpol0_tAve_ampCorr_tDiffCorr[iBar]->SetLineColor(1);
    g_tResolGausAve_ampCorr_tDiffCorr[iBar]->Fit(fpol0_tAve_ampCorr_tDiffCorr[iBar],"QRS");

    fpol0_tAveAmpW_ampCorr[iBar] = new TF1(Form("fpol0_tAveAmpW_ampCorr_BAR%d",iBar),"pol0",-50,50);
    fpol0_tAveAmpW_ampCorr[iBar]->SetLineStyle(2);
    fpol0_tAveAmpW_ampCorr[iBar]->SetLineWidth(2);    
    fpol0_tAveAmpW_ampCorr[iBar]->SetLineColor(1);
    g_tResolGausAveAmpW_ampCorr[iBar]->Fit(fpol0_tAveAmpW_ampCorr[iBar],"QRS");

    fpol0_tAveAmpW_ampCorr_tDiffCorr[iBar] = new TF1(Form("fpol0_tAveAmpW_ampCorr_tDiffCorr_BAR%d",iBar),"pol0",-50,50);
    fpol0_tAveAmpW_ampCorr_tDiffCorr[iBar]->SetLineStyle(2);
    fpol0_tAveAmpW_ampCorr_tDiffCorr[iBar]->SetLineWidth(2);    
    fpol0_tAveAmpW_ampCorr_tDiffCorr[iBar]->SetLineColor(1);
    g_tResolGausAveAmpW_ampCorr_tDiffCorr[iBar]->Fit(fpol0_tAveAmpW_ampCorr_tDiffCorr[iBar],"QRS");


    fpol0_tAveResW_ampCorr[iBar] = new TF1(Form("fpol0_tAveResW_ampCorr_BAR%d",iBar),"pol0",-50,50);
    fpol0_tAveResW_ampCorr[iBar]->SetLineStyle(2);
    fpol0_tAveResW_ampCorr[iBar]->SetLineWidth(2);    
    fpol0_tAveResW_ampCorr[iBar]->SetLineColor(1);
    g_tResolGausAveResW_ampCorr[iBar]->Fit(fpol0_tAveResW_ampCorr[iBar],"QRS");

    fpol0_tAveResW_ampCorr_tDiffCorr[iBar] = new TF1(Form("fpol0_tAveResW_ampCorr_tDiffCorr_BAR%d",iBar),"pol0",-50,50);
    fpol0_tAveResW_ampCorr_tDiffCorr[iBar]->SetLineStyle(2);
    fpol0_tAveResW_ampCorr_tDiffCorr[iBar]->SetLineWidth(2);    
    fpol0_tAveResW_ampCorr_tDiffCorr[iBar]->SetLineColor(1);
    g_tResolGausAveResW_ampCorr_tDiffCorr[iBar]->Fit(fpol0_tAveResW_ampCorr_tDiffCorr[iBar],"QRS");



  }//



  


  // -- Save histograms on file
  std::cout << "Saving histograms in file " << OutputFile.c_str() <<std::endl;
  
  TFile *outfile = new TFile(OutputFile.c_str(),"recreate");
  
  h_ampRef_nocuts->Write();
  h_ampRef->Write();
  h_timeRef->Write();
  p2_eff_vs_posXY_Ref -> Write();
  p_eff_vs_posX_Ref -> Write();
  p_eff_vs_posY_Ref -> Write();
  p_timeRef_vs_ampRef->Write();
  p_ampRef_vs_posX->Write();
  p_ampRef_vs_posY->Write();
  p2_ampRef_vs_posXY->Write();
  p2_timeRef_vs_posXY->Write();

  for (int iBar = 0; iBar < NBARS; iBar++){
    
    p2_eff_vs_posXY[iBar]->Write();
    p_eff_vs_posX[iBar]->Write();
    p_eff_vs_posY[iBar]->Write();
       
    h_ampL_nocuts[iBar]->Write();
    h_ampL[iBar]->Write();
    h_timeL[iBar]->Write();
    h_totL[iBar]->Write();
    h_dtimeMaximumL[iBar] ->Write();
    p2_ampL_vs_posXY[iBar]->Write();
    p_ampL_vs_posX[iBar]->Write();
    p_ampL_vs_posXc[iBar]->Write();
    p_ampL_vs_posY[iBar]->Write();
    p_ampL_vs_tDiff[iBar]->Write();

    h_ampR_nocuts[iBar]->Write();
    h_ampR[iBar]->Write();
    h_timeR[iBar]->Write();
    h_totR[iBar]->Write();
    h_dtimeMaximumR[iBar] ->Write();
    p2_ampR_vs_posXY[iBar]->Write();
    p_ampR_vs_posX[iBar]->Write();
    p_ampR_vs_posXc[iBar]->Write();
    p_ampR_vs_posY[iBar]->Write();
    p_ampR_vs_tDiff[iBar]->Write();


    h2_tDiff_vs_posX[iBar]->Write();

    // -- left
    p2_tL_vs_posXY[iBar]->Write();

    h_tL[iBar]->Write();
    p_tL_vs_amp[iBar]->Write();
    p_tL_vs_posX[iBar]->Write();
    p_tL_vs_posY[iBar]->Write();
    p_tL_vs_tDiff[iBar]->Write();
    
    h_tL_ampCorr[iBar]->Write();
    p_tL_ampCorr_vs_amp[iBar]->Write();
    p_tL_ampCorr_vs_posX[iBar]->Write();
    p_tL_ampCorr_vs_posXc[iBar]->Write();
    p_tL_ampCorr_vs_posY[iBar]->Write();
    p_tL_ampCorr_vs_tDiff[iBar]->Write();

    // -- right
    p2_tR_vs_posXY[iBar]->Write();

    h_tR[iBar]->Write();
    p_tR_vs_amp[iBar]->Write();
    p_tR_vs_posX[iBar]->Write();
    p_tR_vs_posY[iBar]->Write();
    p_tR_vs_tDiff[iBar]->Write();
    
    h_tR_ampCorr[iBar]->Write();
    p_tR_ampCorr_vs_amp[iBar]->Write();
    p_tR_ampCorr_vs_posX[iBar]->Write();
    p_tR_ampCorr_vs_posXc[iBar]->Write();
    p_tR_ampCorr_vs_posY[iBar]->Write();
    p_tR_ampCorr_vs_tDiff[iBar]->Write();


    // average
    p2_tAve_vs_posXY[iBar]->Write();

    h_tAve[iBar]->Write();
    p_tAve_vs_posX[iBar]->Write();
    p_tAve_vs_posY[iBar]->Write();
    p_tAve_vs_tDiff[iBar]->Write();
    
    h_tAve_ampCorr[iBar]->Write();
    p_tAve_ampCorr_vs_posX[iBar]->Write();
    p_tAve_ampCorr_vs_posXc[iBar]->Write();
    p_tAve_ampCorr_vs_posY[iBar]->Write();
    p_tAve_ampCorr_vs_tDiff[iBar]->Write();

    h_tAve_ampCorr_tDiffCorr[iBar]->Write();
    p_tAve_ampCorr_tDiffCorr_vs_posX[iBar]->Write();
    p_tAve_ampCorr_tDiffCorr_vs_posXc[iBar]->Write();
    p_tAve_ampCorr_tDiffCorr_vs_posY[iBar]->Write();
    p_tAve_ampCorr_tDiffCorr_vs_tDiff[iBar]->Write();

    h_tAve_ampCorr_posCorr[iBar]->Write();
    p_tAve_ampCorr_posCorr_vs_posX[iBar]->Write();
    p_tAve_ampCorr_posCorr_vs_posXc[iBar]->Write();
    p_tAve_ampCorr_posCorr_vs_posY[iBar]->Write();
    p_tAve_ampCorr_posCorr_vs_tDiff[iBar]->Write();

    h_tAveAmpW_ampCorr[iBar]->Write();
    p_tAveAmpW_ampCorr_vs_posX[iBar]->Write();
    p_tAveAmpW_ampCorr_vs_posXc[iBar]->Write();
    p_tAveAmpW_ampCorr_vs_posY[iBar]->Write();
    p_tAveAmpW_ampCorr_vs_tDiff[iBar]->Write();

    h_tAveAmpW_ampCorr_tDiffCorr[iBar]->Write();
    p_tAveAmpW_ampCorr_tDiffCorr_vs_posX[iBar]->Write();
    p_tAveAmpW_ampCorr_tDiffCorr_vs_posXc[iBar]->Write();
    p_tAveAmpW_ampCorr_tDiffCorr_vs_posY[iBar]->Write();
    p_tAveAmpW_ampCorr_tDiffCorr_vs_tDiff[iBar]->Write();

    h_tAveResW_ampCorr[iBar]->Write();
    p_tAveResW_ampCorr_vs_posX[iBar]->Write();
    p_tAveResW_ampCorr_vs_posXc[iBar]->Write();
    p_tAveResW_ampCorr_vs_posY[iBar]->Write();
    p_tAveResW_ampCorr_vs_tDiff[iBar]->Write();

    h_tAveResW_ampCorr_tDiffCorr[iBar]->Write();
    p_tAveResW_ampCorr_tDiffCorr_vs_posX[iBar]->Write();
    p_tAveResW_ampCorr_tDiffCorr_vs_posXc[iBar]->Write();
    p_tAveResW_ampCorr_tDiffCorr_vs_posY[iBar]->Write();
    p_tAveResW_ampCorr_tDiffCorr_vs_tDiff[iBar]->Write();

    // eff sigmas    
    h_effectiveSigmaL[iBar]->Write();
    h_effectiveSigmaL_ampCorr[iBar]->Write();

    h_effectiveSigmaR[iBar]->Write();
    h_effectiveSigmaR_ampCorr[iBar]->Write();

    h_effectiveSigmaAve[iBar]->Write();

    h_effectiveSigmaAve_ampCorr[iBar]->Write();
    h_effectiveSigmaAve_ampCorr_tDiffCorr[iBar]->Write();
    h_effectiveSigmaAve_ampCorr_posCorr[iBar]->Write();


    h_effectiveSigmaAveAmpW_ampCorr[iBar]->Write();
    h_effectiveSigmaAveAmpW_ampCorr_tDiffCorr[iBar]->Write();

    h_effectiveSigmaAveResW_ampCorr[iBar]->Write();
    h_effectiveSigmaAveResW_ampCorr_tDiffCorr[iBar]->Write();

    // graphs 
    g_tResolGausL_ampCorr[iBar]->Write();
    g_tResolGausR_ampCorr[iBar]->Write();
    g_tResolGausAve_ampCorr[iBar]->Write();
    g_tResolGausAve_ampCorr_tDiffCorr[iBar]->Write();
    g_tResolGausAveAmpW_ampCorr[iBar]->Write();
    g_tResolGausAveAmpW_ampCorr_tDiffCorr[iBar]->Write();
    g_tResolGausAveResW_ampCorr[iBar]->Write();
    g_tResolGausAveResW_ampCorr_tDiffCorr[iBar]->Write();
  
  }


  cout << "Saving plots in subdir..."<<endl;

  TDirectory *dir = outfile->mkdir("xBins/");  
  dir->cd();
  for (int iBar = 0; iBar < NBARS; iBar++){
    for (int ibin = 0 ; ibin < NBINSX[iBar]; ibin++){
      h_tL_ampCorr_binX[iBar][ibin] -> Write();
      h_tR_ampCorr_binX[iBar][ibin] -> Write();
      h_tAve_ampCorr_binX[iBar][ibin] -> Write();
      h_tAve_ampCorr_tDiffCorr_binX[iBar][ibin] -> Write();
      h_tAveAmpW_ampCorr_binX[iBar][ibin] -> Write();
      h_tAveAmpW_ampCorr_tDiffCorr_binX[iBar][ibin] -> Write();
      h_tAveResW_ampCorr_binX[iBar][ibin] -> Write();
      h_tAveResW_ampCorr_tDiffCorr_binX[iBar][ibin] -> Write();

    }
  }
  
  cout << "Closing file..."<<endl;

  outfile->Close();

  delete outfile;
  return 0;

}



// ---- Initialize tree -------------------------------------------------------------------------------------
void InitTreeVars(TTree* chain1,TreeVars& treeVars){
  
  treeVars.t_digiConf = 0;
  treeVars.t_sensorConf = 0;
  treeVars.t_NINOthr = 0;
  treeVars.t_amp = new float[36];
  treeVars.t_time_maximum = new float[36];
  treeVars.t_time = new float[360];
  treeVars.t_b_rms = new float[360];
  treeVars.t_CFD = 0;
  treeVars.t_LED = 0;
  treeVars.t_TED = 0;

  treeVars.t_channelId = new std::map<std::string,int>;

  treeVars.t_trackFitResult = new std::vector<TrackPar>;
  treeVars.t_ntracks = 0;

  //chain1 -> SetBranchStatus("*",0);

  chain1 -> SetBranchStatus("digiConf",      1); chain1 -> SetBranchAddress("digiConf",     &treeVars.t_digiConf);
  chain1 -> SetBranchStatus("sensorConf",      1); chain1 -> SetBranchAddress("sensorConf",     &treeVars.t_sensorConf);
  chain1 -> SetBranchStatus("NINOthr",       1); chain1 -> SetBranchAddress("NINOthr",     &treeVars.t_NINOthr);

  chain1 -> SetBranchStatus("n_tracks",      1); chain1->SetBranchAddress("n_tracks",  &treeVars.t_ntracks);
  chain1 -> SetBranchStatus("fitResult",     1); chain1->SetBranchAddress("fitResult", &treeVars.t_trackFitResult);

  chain1 -> SetBranchStatus("amp_max",       1); chain1 -> SetBranchAddress("amp_max",      treeVars.t_amp);
  chain1 -> SetBranchStatus("time_maximum",  1); chain1 -> SetBranchAddress("time_maximum", treeVars.t_time_maximum);
  chain1 -> SetBranchStatus("time",          1); chain1 -> SetBranchAddress("time",         treeVars.t_time);
  chain1 -> SetBranchStatus("b_rms",         1); chain1 -> SetBranchAddress("b_rms",        treeVars.t_b_rms);
  chain1 -> SetBranchStatus("CFD",           1); chain1 -> SetBranchAddress("CFD",          &treeVars.t_CFD);
  chain1 -> SetBranchStatus("LED",           1); chain1 -> SetBranchAddress("LED",          &treeVars.t_LED);
  chain1 -> SetBranchStatus("TED",           1); chain1 -> SetBranchAddress("TED",          &treeVars.t_TED);



  chain1 -> SetBranchStatus(timeChannelRef.c_str(),1); chain1 -> SetBranchAddress(timeChannelRef.c_str(),&((*treeVars.t_channelId)[timeChannelRef]));
  chain1 -> SetBranchStatus(ampChannelRef.c_str(),1);  chain1 -> SetBranchAddress(ampChannelRef.c_str(),&((*treeVars.t_channelId)[ampChannelRef]));

  for(unsigned int it = 0; it < timeChannelsL.size(); ++it)
    {
      std::string channelName = timeChannelsL.at(it);
      chain1 -> SetBranchStatus(channelName.c_str(),1); chain1 -> SetBranchAddress(channelName.c_str(),&((*treeVars.t_channelId)[channelName]));
    }

  for(unsigned int it = 0; it < timeChannelsR.size(); ++it)
    {
      std::string channelName = timeChannelsR.at(it);
      chain1 -> SetBranchStatus(channelName.c_str(),1); chain1 -> SetBranchAddress(channelName.c_str(),&((*treeVars.t_channelId)[channelName]));
    }


  for(unsigned int it = 0; it < ampChannelsL.size(); ++it)
    {
      std::string channelName = ampChannelsL.at(it);
      chain1 -> SetBranchStatus(channelName.c_str(),1); chain1 -> SetBranchAddress(channelName.c_str(),&((*treeVars.t_channelId)[channelName]));
    }

  for(unsigned int it = 0; it < ampChannelsR.size(); ++it)
    {
      std::string channelName = ampChannelsR.at(it);
      chain1 -> SetBranchStatus(channelName.c_str(),1); chain1 -> SetBranchAddress(channelName.c_str(),&((*treeVars.t_channelId)[channelName]));
    }

}


// -- time resol 
void GetTimeResolution( TH1F *ht, float &sEff, float &sGaus, TF1* fitFun)
{
  
  float tmin = 0;
  float tmax = 0;
  float* vals = new float[4];

  FindSmallestInterval(vals,ht,0.68,true);
  sEff = 0.5*(vals[3]-vals[2]);

  fitFun->SetLineColor(2);
  fitFun->SetLineWidth(1);
  fitFun->SetParameter(1,ht->GetMean());
  fitFun->SetParameter(2,ht->GetRMS());
  tmin = ht->GetMean()-2.0*sEff;
  tmax = ht->GetMean()+2.0*sEff;
  ht->Fit( fitFun, "QRS", "", tmin, tmax);
  sGaus = fitFun->GetParameter(2);
  
}
