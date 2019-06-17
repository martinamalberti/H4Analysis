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
  float* t_amp;
  float* t_time;
  float* t_b_rms;
  int  t_CFD;
  int  t_LED;

  std::vector<TrackPar> *t_trackFitResult;
  int t_ntracks;

  std::map<std::string,int> *t_channelId;

};
/**********************/

void InitTreeVars(TTree* chain1, TreeVars& treeVars, float threshold);
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
  
  float runMin = opts.GetOpt<float>("Input.runMin");
  float runMax = opts.GetOpt<float>("Input.runMax");

  timeChannelRef = opts.GetOpt<std::string>("Input.timeChannelRef");
  ampChannelRef  = opts.GetOpt<std::string>("Input.ampChannelRef");

  timeChannelsL  = opts.GetOpt<std::vector<std::string> >("Input.timeChannelsL");
  ampChannelsL   = opts.GetOpt<std::vector<std::string> >("Input.ampChannelsL");

  timeChannelsR  = opts.GetOpt<std::vector<std::string> >("Input.timeChannelsR");
  ampChannelsR   = opts.GetOpt<std::vector<std::string> >("Input.ampChannelsR");

  float xyangle = opts.GetOpt<float>("Input.xyangle");

  float threshold = opts.GetOpt<float>("Input.threshold");

  std::string fitFunction = opts.GetOpt<std::string>("Input.fitFunction");;

  std::string OutputFile = opts.GetOpt<std::string>("Input.OutputFile");

  
  float cut_brmsMaxRef = opts.GetOpt<float>("Cuts.brmsMaxRef");  
  std::vector<float> cut_brmsMaxTimeCh = opts.GetOpt<std::vector<float> >("Cuts.brmsMaxTimeCh");

  float cut_minTimeRef = opts.GetOpt<float>("Cuts.minTimeRef");
  float cut_maxTimeRef = opts.GetOpt<float>("Cuts.maxTimeRef");
  std::vector<float> cut_minTime = opts.GetOpt<std::vector<float> >("Cuts.minTime");
  std::vector<float> cut_maxTime = opts.GetOpt<std::vector<float> >("Cuts.maxTime");
  for (int iBar = 0; iBar<3; iBar++){
    cut_minTime[iBar] = 0;
    cut_maxTime[iBar] = 200;
 }

  float cut_ampMinFraction = opts.GetOpt<float>("Cuts.ampMinFraction");

  float cut_XminRef = opts.GetOpt<float>("Cuts.XminRef");
  float cut_XmaxRef = opts.GetOpt<float>("Cuts.XmaxRef");
  float cut_YminRef = opts.GetOpt<float>("Cuts.YminRef");
  float cut_YmaxRef = opts.GetOpt<float>("Cuts.YmaxRef");

  std::vector<float> cut_Xmin = opts.GetOpt<std::vector<float> >("Cuts.Xmin");
  std::vector<float> cut_Xmax = opts.GetOpt<std::vector<float> >("Cuts.Xmax");
  std::vector<float> cut_Ymin = opts.GetOpt<std::vector<float> >("Cuts.Ymin");
  std::vector<float> cut_Ymax = opts.GetOpt<std::vector<float> >("Cuts.Ymax");


  // -- list of runs without tracking desync
  ifstream goodRunsFile("/afs/cern.ch/user/a/abenagli/public/MTDTB_FNAL_Apr2019_good_runs.txt");
  vector<int> goodRuns;
  goodRuns.clear();
  if(goodRunsFile.is_open()){
    int run;
    while (goodRunsFile >> run){
      goodRuns.push_back(run);
    }
  }


  std::cout << " Setting TDRStyle..."<<std::endl;
  //--------------------------
  // labels and canvas style
  setTDRStyle();

  
  //---------------------------
  // open input files and trees
  std::cout << "Loading trees ..." << std::endl; 
  TChain* chain1 = new TChain("h4","h4");
  for (unsigned int iRun = runMin; iRun < runMax+1; iRun++){
    // -- check if this is a good run
    if ( !goodRuns.empty() && std::count(goodRuns.begin(), goodRuns.end(), int(iRun)) == false) continue;
    // -- check if file exists
    std::string fileName = "/eos/cms/store/group/dpg_mtd/comm_mtd/TB/MTDTB_FNAL_Apr2019/ntuples/v4/"+ std::to_string(int(iRun))+".root";
    if (gSystem->AccessPathName(fileName.c_str()) == 1) continue;
    std::cout << fileName << std::endl;
    chain1 -> Add(fileName.c_str());
  }
  std::cout << " Read " << chain1->GetEntries() << " total events in tree " << chain1->GetName() << std::endl;
  

  //--------------------------
  // set branches
  TreeVars treeVars;
  InitTreeVars(chain1,treeVars,threshold);
  
  
  //--------------------------
  // Book histograms
  const int NBARS = timeChannelsL.size();
  int NBINSX[NBARS];
  float theta[NBARS];

  float xbinWidth = 2.0 ; // mm
  for (int iBar = 0; iBar < NBARS; iBar++){
    float x1 = min(cut_Xmax[iBar], cut_XmaxRef);
    float x2 = max(cut_Xmin[iBar], cut_XminRef);
    if (x1 < x2){
      cout << "Ref  xmin = " << cut_XminRef <<  "   xmax " << cut_XmaxRef <<endl; 
      cout << "BAR" << iBar << " xmin = " << cut_Xmin[iBar] <<  "   xmax " << cut_Xmax[iBar] <<endl; 
      cout << "Ref and bar not overlapping in x ... exiting!"<<endl;
      return 0;
    } 
    NBINSX[iBar] = ceil ((x1 - x2)/xbinWidth) ;
    theta[iBar]  = (90. - xyangle) * TMath::Pi()/180.;
  }
   
  float ybinWidth = 0.2 ; // mm
  int NBINSY = ceil ((cut_Ymax[0]-cut_Ymin[NBARS-1])/ybinWidth );

  float xbinWidthForAmpWalk =  999; // mm 
  int NBINSXAMPWALK[NBARS];
  for (int iBar = 0; iBar < NBARS; iBar++){
    float x1 = min(cut_Xmax[iBar], cut_XmaxRef);
    float x2 = max(cut_Xmin[iBar], cut_XminRef);
    if (x1 < x2){
      cout << "Ref  xmin = " << cut_XminRef <<  "   xmax " << cut_XmaxRef <<endl;
      cout << "BAR" << iBar << " xmin = " << cut_Xmin[iBar] <<  "   xmax " << cut_Xmax[iBar] <<endl;
      cout << "Ref and bar not overlapping in x ... exiting!"<<endl;
      return 0;
    }
    if ( fabs(x1 -x2) < xbinWidthForAmpWalk){
      NBINSXAMPWALK[iBar] = 1;
    }
    else{
      NBINSXAMPWALK[iBar] = ceil ((x1 - x2)/cos(theta[iBar])/xbinWidthForAmpWalk) ;
    }
  }

  int   nTimeBins = 8000;
  float dtminL[NBARS]   = {-20., -20., -20.};
  float dtmaxL[NBARS]   = { 20.,  20.,  20.};
  float dtminR[NBARS]   = {-20., -20., -20.};
  float dtmaxR[NBARS]   = { 20.,  20.,  20.};
  float dtminAve[NBARS] = {-20., -20., -20.};
  float dtmaxAve[NBARS] = { 20.,  20.,  20.};
  
  float xmin = -50.;
  float xmax = 50.;

  float ymin = 0.;
  float ymax= 50.;

  
  // -- beam profile histos
  TH2F* h_beamXY = new TH2F("h_beamXY","h_beamXY",100,xmin,xmax,100,ymin,ymax);
  TH1F* h_beamX  = new TH1F("h_beamX", "h_beamX", 100,xmin,xmax);
  TH1F* h_beamY  = new TH1F("h_beamY", "h_beamY", 100,ymin,ymax);

  // -- ref histos
  TH1F *h_ampRef_nocuts = new TH1F("h_ampRef_nocuts", "h_ampRef_nocuts", 500, 0., 1.);
  TH1F *h_ampRef        = new TH1F("h_ampRef", "h_ampRef", 500, 0., 1.);
  TH1F *h_timeRef       = new TH1F("h_timeRef", "h_timeRef", 200, 0., 200.);;
  TH1F *h_brmsRef       = new TH1F("h_brmsRef", "h_brmsRef", 200, 0., 200.);;
  TProfile2D*  p2_eff_vs_posXY_Ref = new TProfile2D("p2_eff_vs_posXY_Ref","p2_eff_vs_posXY_Ref", 200, xmin, xmax, 200, ymin, ymax, 0, 1);
  TProfile*    p_eff_vs_posX_Ref   = new TProfile("p_eff_vs_posX_Ref","p_eff_vs_posX_Ref", 200, xmin, xmax, 0, 1);
  TProfile*    p_eff_vs_posY_Ref   = new TProfile("p_eff_vs_posY_Ref","p_eff_vs_posY_Ref", 200, ymin, ymax, 0, 1);
  TProfile*    p_timeRef_vs_ampRef = new TProfile("p_timeRef_vs_ampRef","p_timeRef_vs_ampRef", 100, 0, 1, 0, 200);
  TProfile2D*  p2_ampRef_vs_posXY  = new TProfile2D("p2_ampRef_vs_posXY","p2_ampRef_vs_posXY", 200, xmin, xmax, 200, ymin, ymax, 0, 1);
  TProfile2D*  p2_timeRef_vs_posXY = new TProfile2D("p2_timeRef_vs_posXY","p2_timeRef_vs_posXY", 200, xmin, xmax, 200, ymin, ymax, 0, 200);
  TProfile*    p_ampRef_vs_posX    = new TProfile("p_ampRef_vs_posX","p_ampRef_vs_posX", 100, -50, 50, 0, 1);
  TProfile*    p_ampRef_vs_posY    = new TProfile("p_ampRef_vs_posY","p_ampRef_vs_posY", 100, -50, 50, 0, 1);
  TProfile*    p_timeRef_vs_posX    = new TProfile("p_timeRef_vs_posX","p_timeRef_vs_posX", 100, -50, 50, 0, 200);
  TProfile*    p_timeRef_vs_posY    = new TProfile("p_timeRef_vs_posY","p_timeRef_vs_posY", 100, -50, 50, 0, 200);


  // -- control plots
  TProfile2D*  p2_eff_vs_posXY[NBARS];
  TProfile*    p_eff_vs_posX[NBARS];
  TProfile*    p_eff_vs_posY[NBARS];

  TH1F*  h_ampL_nocuts[NBARS];
  TH1F*  h_ampL[NBARS];
  TH1F*  h_timeL[NBARS];
  TH1F*  h_brms_timeL[NBARS];
  TH2F*  h2_timeL_vs_brms[NBARS];

  TProfile2D*  p2_ampL_vs_posXY[NBARS];
  TProfile*    p_ampL_vs_posX[NBARS];
  TProfile*    p_ampL_vs_posXc[NBARS];
  TProfile*    p_ampL_vs_posY[NBARS];
  TProfile*    p_ampL_vs_tDiff[NBARS];

  TH1F*  h_ampR_nocuts[NBARS];
  TH1F*  h_ampR[NBARS];
  TH1F*  h_timeR[NBARS];
  TH1F*  h_brms_timeR[NBARS];
  TH2F*  h2_timeR_vs_brms[NBARS];
 
  TProfile2D*  p2_ampR_vs_posXY[NBARS];
  TProfile*    p_ampR_vs_posX[NBARS];
  TProfile*    p_ampR_vs_posXc[NBARS];
  TProfile*    p_ampR_vs_posY[NBARS];
  TProfile*    p_ampR_vs_tDiff[NBARS];

  TH2F* h2_tDiff_vs_posX[NBARS];
  TH2F* h2_tDiff_ampCorr_vs_posX[NBARS];
  TH1F* h_ampDiff_binX[NBARS][1000];
  for (int iBar = 0; iBar < NBARS; iBar++){
    for (int ibin = 0; ibin < NBINSX[iBar]; ibin++){
      h_ampDiff_binX[iBar][ibin]  = new TH1F(Form("h_ampDiff_binX_%d_BAR%d",ibin, iBar), Form("h_ampDiff_binX_%d_BAR%d",ibin, iBar), 500, -0.2, 0.2);
    }
  }

  // -- left
  TProfile2D*    p2_tL_vs_posXY[NBARS];

  TH1F*          h_tL[NBARS];
  TH2F*          h2_tL_vs_amp[NBARS];
  TProfile*      p_tL_vs_amp[NBARS];
  TProfile*      p_tL_vs_amp_binned[NBARS][1000];                                                                  TProfile*      p_tL_vs_posX[NBARS];
  TProfile*      p_tL_vs_posY[NBARS];
  TProfile*      p_tL_vs_tDiff[NBARS];

  TH1F*          h_tL_ampCorr[NBARS];
  TProfile*      p_tL_ampCorr_vs_amp[NBARS];
  TProfile*      p_tL_ampCorr_vs_amp_binned[NBARS][1000];  
  TProfile*      p_tL_ampCorr_vs_posX[NBARS];
  TProfile*      p_tL_ampCorr_vs_posY[NBARS];
  TProfile*      p_tL_ampCorr_vs_tDiff[NBARS];
  TH1F*          h_tL_ampCorr_binX[NBARS][1000];
  TProfile*      p_tL_ampCorr_vs_posXc[NBARS];


  // -- right   
  TProfile2D*    p2_tR_vs_posXY[NBARS];

  TH1F*          h_tR[NBARS];
  TH2F*          h2_tR_vs_amp[NBARS];
  TProfile*      p_tR_vs_amp[NBARS];
  TProfile*      p_tR_vs_amp_binned[NBARS][1000];
  TProfile*      p_tR_vs_posX[NBARS];
  TProfile*      p_tR_vs_posY[NBARS];
  TProfile*      p_tR_vs_tDiff[NBARS];

  TH1F*          h_tR_ampCorr[NBARS];
  TProfile*      p_tR_ampCorr_vs_amp[NBARS];
  TProfile*      p_tR_ampCorr_vs_amp_binned[NBARS][1000];  
  TProfile*      p_tR_ampCorr_vs_posX[NBARS];
  TProfile*      p_tR_ampCorr_vs_posY[NBARS];
  TProfile*      p_tR_ampCorr_vs_tDiff[NBARS];
  TH1F*          h_tR_ampCorr_binX[NBARS][1000];
  TProfile*      p_tR_ampCorr_vs_posXc[NBARS];

  // -- tDiff
  TH1F*          h_tDiff[NBARS];
  TH1F*          h_tDiff_ampCorr[NBARS];
  TH1F*          h_tDiff_ampCorr_posCorr[NBARS];
  TProfile*      p_tDiff_ampCorr_vs_posX[NBARS];
  TProfile*      p_tDiff_ampCorr_vs_posXc[NBARS];
  TProfile*      p_tDiff_ampCorr_posCorr_vs_posX[NBARS];
  TProfile*      p_tDiff_ampCorr_posCorr_vs_posXc[NBARS];

  TH1F* h_tDiff_ampCorr_binX[NBARS][1000];
  TH1F* h_tDiff_ampCorr_posCorr_binX[NBARS][1000];

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
  TH1F*          h_tAve_ampCorr_binX[NBARS][1000];
  TH1F*          h_tAve_ampCorr_binY[NBARS][1000];
  TProfile*      p_tAve_ampCorr_vs_posXc[NBARS];

  TH1F*          h_tAve_ampCorr_tDiffCorr[NBARS];
  TProfile*      p_tAve_ampCorr_tDiffCorr_vs_posX[NBARS];
  TProfile*      p_tAve_ampCorr_tDiffCorr_vs_posY[NBARS];
  TProfile*      p_tAve_ampCorr_tDiffCorr_vs_tDiff[NBARS];
  TH1F*          h_tAve_ampCorr_tDiffCorr_binX[NBARS][1000];
  TH1F*          h_tAve_ampCorr_tDiffCorr_binY[NBARS][1000];
  TProfile*      p_tAve_ampCorr_tDiffCorr_vs_posXc[NBARS];

  TH1F*          h_tAve_ampCorr_posCorr[NBARS];
  TProfile*      p_tAve_ampCorr_posCorr_vs_posX[NBARS];
  TProfile*      p_tAve_ampCorr_posCorr_vs_posY[NBARS];
  TProfile*      p_tAve_ampCorr_posCorr_vs_tDiff[NBARS];
  TH1F*          h_tAve_ampCorr_posCorr_binX[NBARS][1000];
  TH1F*          h_tAve_ampCorr_posCorr_binY[NBARS][1000];
  TProfile*      p_tAve_ampCorr_posCorr_vs_posXc[NBARS];


  int nbinsHistoX = 4000;
  if (chain1->GetEntries() < 100000){
    nbinsHistoX = 2000;
  }
  int nbinsHistoY = nbinsHistoX;

  int nAmpBins = 250;
  cout << "n amp bins = " << nAmpBins <<endl;

  for (int iBar = 0; iBar < NBARS; iBar++){

    p2_eff_vs_posXY[iBar] = new TProfile2D(Form("p2_eff_vs_posXY_BAR%d",iBar),Form("p2_eff_vs_posXY_BAR%d",iBar), 200, xmin, xmax, 200, ymin, ymax, 0, 1);
    p_eff_vs_posX[iBar] = new TProfile(Form("p_eff_vs_posX_BAR%d",iBar),Form("p_eff_vs_posX_BAR%d",iBar), 200, xmin, xmax, 0, 1);
    p_eff_vs_posY[iBar] = new TProfile(Form("p_eff_vs_posY_BAR%d",iBar),Form("p_eff_vs_posY_BAR%d",iBar), 200, ymin, ymax, 0, 1);
    
    h_ampL_nocuts[iBar] = new TH1F(Form("h_ampL_nocuts_BAR%d", iBar), Form("h_ampL_nocuts_BAR%d",iBar), 500, 0., 1.0);
    h_ampL[iBar]        = new TH1F(Form("h_ampL_BAR%d", iBar), Form("h_ampL_BAR%d",iBar), 500, 0., 1.0);
    h_timeL[iBar]       = new TH1F(Form("h_timeL_BAR%d",iBar),Form("h_timeL_BAR%d",iBar),400,0.,200.);
    h_brms_timeL[iBar]   = new TH1F(Form("h_brms_timeL_BAR%d",iBar),Form("h_brms_timeL_BAR%d",iBar),400,0.,200.);
    h2_timeL_vs_brms[iBar] = new TH2F(Form("h2_timeL_vs_brms_BAR%d",iBar),Form("h2_timeL_vs_brms_BAR%d",iBar),500,0.,500., 2000, 0, 200);

    h_ampR_nocuts[iBar] = new TH1F(Form("h_ampR_nocuts_BAR%d", iBar), Form("h_ampR_nocuts_BAR%d",iBar), 500, 0., 1.0);
    h_ampR[iBar]        = new TH1F(Form("h_ampR_BAR%d", iBar), Form("h_ampR_BAR%d",iBar), 500, 0., 1.0);
    h_timeR[iBar]       = new TH1F(Form("h_timeR_BAR%d",iBar),Form("h_timeR_BAR%d",iBar),400,0.,200.);
    h_brms_timeR[iBar]   = new TH1F(Form("h_brms_timeR_BAR%d",iBar),Form("h_brms_timeR_BAR%d",iBar),400,0.,200.);
    h2_timeR_vs_brms[iBar] = new TH2F(Form("h2_timeR_vs_brms_BAR%d",iBar),Form("h2_timeR_vs_brms_BAR%d",iBar),500,0.,500., 2000, 0, 200);

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

    // -- tLeft
    p2_tL_vs_posXY[iBar]        = new TProfile2D(Form("p2_tL_vs_posXY_BAR%d",iBar),Form("p2_tL_vs_posXY_BAR%d",iBar), 400, xmin, xmax, 200, ymin, ymax, dtminL[iBar], dtmaxL[iBar]);

    h_tL[iBar]                  = new TH1F(Form("h_tL_BAR%d",iBar),Form("h_tL_BAR%d",iBar), nTimeBins, dtminL[iBar], dtmaxL[iBar]);
    h2_tL_vs_amp[iBar]          = new TH2F(Form("h2_tL_vs_amp_BAR%d",iBar),Form("h2_tL_vs_amp_BAR%d",iBar), nAmpBins, 0., 1., 200, dtminL[iBar], dtmaxL[iBar]);
    p_tL_vs_amp[iBar]           = new TProfile(Form("p_tL_vs_amp_BAR%d",iBar),Form("p_tL_vs_amp_BAR%d",iBar), nAmpBins, 0., 1.,dtminL[iBar], dtmaxL[iBar]);
    p_tL_vs_posX[iBar]          = new TProfile(Form("p_tL_vs_posX_BAR%d",iBar),Form("p_tL_vs_posX_BAR%d",iBar), 100, xmin, xmax,dtminL[iBar], dtmaxL[iBar]);
    p_tL_vs_posY[iBar]          = new TProfile(Form("p_tL_vs_posY_BAR%d",iBar),Form("p_tL_vs_posY_BAR%d",iBar), 100, ymin, ymax,dtminL[iBar], dtmaxL[iBar]);
    p_tL_vs_tDiff[iBar]         = new TProfile(Form("p_tL_vs_tDiff_BAR%d",iBar),Form("p_tL_vs_tDiff_BAR%d",iBar), 100, -4, 4, dtminL[iBar], dtmaxL[iBar]);

    h_tL_ampCorr[iBar]          = new TH1F(Form("h_tL_ampCorr_BAR%d",iBar),Form("h_tL_ampCorr_BAR%d",iBar), nTimeBins, dtminL[iBar], dtmaxL[iBar]);
    p_tL_ampCorr_vs_amp[iBar]   = new TProfile(Form("p_tL_ampCorr_vs_amp_BAR%d",iBar),Form("p_tL_ampCorr_vs_amp_BAR%d",iBar), nAmpBins, 0., 1.,dtminL[iBar], dtmaxL[iBar]);
    p_tL_ampCorr_vs_posX[iBar]  = new TProfile(Form("p_tL_ampCorr_vs_posX_BAR%d",iBar),Form("p_tL_ampCorr_vs_posX_BAR%d",iBar), 100, xmin, xmax,dtminL[iBar], dtmaxL[iBar]);
    p_tL_ampCorr_vs_posY[iBar]  = new TProfile(Form("p_tL_ampCorr_vs_posY_BAR%d",iBar),Form("p_tL_ampCorr_vs_posY_BAR%d",iBar), 100, ymin, ymax,dtminL[iBar], dtmaxL[iBar]);
    p_tL_ampCorr_vs_tDiff[iBar] = new TProfile(Form("p_tL_ampCorr_vs_tDiff_BAR%d",iBar),Form("p_tL_ampCorr_vs_tDiff_BAR%d",iBar), 100, -4, 4,dtminL[iBar], dtmaxL[iBar]);
    p_tL_ampCorr_vs_posXc[iBar] = new TProfile(Form("p_tL_ampCorr_vs_posXc_BAR%d",iBar),Form("p_tL_ampCorr_vs_posXc_BAR%d",iBar), 100, xmin/cos(theta[iBar]), xmax/cos(theta[iBar]),dtminL[iBar], dtmaxL[iBar]);
   
    // -- tRight
    p2_tR_vs_posXY[iBar]        = new TProfile2D(Form("p2_tR_vs_posXY_BAR%d",iBar),Form("p2_tR_vs_posXY_BAR%d",iBar), 400, xmin, xmax, 200, ymin, ymax,dtminR[iBar], dtmaxR[iBar]);

    h_tR[iBar]                  = new TH1F(Form("h_tR_BAR%d",iBar),Form("h_tR_BAR%d",iBar), nTimeBins, dtminR[iBar], dtmaxR[iBar]);
    h2_tR_vs_amp[iBar]          = new TH2F(Form("h2_tR_vs_amp_BAR%d",iBar),Form("h2_tR_vs_amp_BAR%d",iBar), nAmpBins, 0., 1., 200, dtminR[iBar], dtmaxR[iBar]);
    p_tR_vs_amp[iBar]           = new TProfile(Form("p_tR_vs_amp_BAR%d",iBar),Form("p_tR_vs_amp_BAR%d",iBar), nAmpBins, 0., 1.,dtminR[iBar], dtmaxR[iBar]);
    p_tR_vs_posX[iBar]          = new TProfile(Form("p_tR_vs_posX_BAR%d",iBar),Form("p_tR_vs_posX_BAR%d",iBar), 100, xmin, xmax,dtminR[iBar], dtmaxR[iBar]);
    p_tR_vs_posY[iBar]          = new TProfile(Form("p_tR_vs_posY_BAR%d",iBar),Form("p_tR_vs_posY_BAR%d",iBar), 100, ymin, ymax,dtminR[iBar], dtmaxR[iBar]);
    p_tR_vs_tDiff[iBar]         = new TProfile(Form("p_tR_vs_tDiff_BAR%d",iBar),Form("p_tR_vs_tDiff_BAR%d",iBar), 100, -4, 4,dtminR[iBar], dtmaxR[iBar]);

    h_tR_ampCorr[iBar]          = new TH1F(Form("h_tR_ampCorr_BAR%d",iBar),Form("h_tR_ampCorr_BAR%d",iBar), nTimeBins, dtminR[iBar], dtmaxR[iBar]);
    p_tR_ampCorr_vs_amp[iBar]   = new TProfile(Form("p_tR_ampCorr_vs_amp_BAR%d",iBar),Form("p_tR_ampCorr_vs_amp_BAR%d",iBar), nAmpBins, 0., 1., dtminR[iBar], dtmaxR[iBar]);
    p_tR_ampCorr_vs_posX[iBar]  = new TProfile(Form("p_tR_ampCorr_vs_posX_BAR%d",iBar),Form("p_tR_ampCorr_vs_posX_BAR%d",iBar), 100, xmin, xmax, dtminR[iBar], dtmaxR[iBar]);
    p_tR_ampCorr_vs_posY[iBar]  = new TProfile(Form("p_tR_ampCorr_vs_posY_BAR%d",iBar),Form("p_tR_ampCorr_vs_posY_BAR%d",iBar), 100, ymin, ymax, dtminR[iBar], dtmaxR[iBar]);
    p_tR_ampCorr_vs_tDiff[iBar] = new TProfile(Form("p_tR_ampCorr_vs_tDiff_BAR%d",iBar),Form("p_tR_ampCorr_vs_tDiff_BAR%d",iBar), 100, -4, 4, dtminR[iBar], dtmaxR[iBar]);
    p_tR_ampCorr_vs_posXc[iBar] = new TProfile(Form("p_tR_ampCorr_vs_posXc_BAR%d",iBar),Form("p_tR_ampCorr_vs_posXc_BAR%d",iBar), 100, xmin/cos(theta[iBar]), xmax/cos(theta[iBar]), dtminR[iBar], dtmaxR[iBar]);

    // -- tDiff
    h_tDiff[iBar]                  = new TH1F(Form("h_tDiff_BAR%d",iBar),Form("h_tDiff_BAR%d",iBar),nTimeBins/2, -4, 4);
    h_tDiff_ampCorr[iBar]          = new TH1F(Form("h_tDiff_ampCorr_BAR%d",iBar),Form("h_tDiff_ampCorr_BAR%d",iBar),nTimeBins/2, -4, 4);
    h_tDiff_ampCorr_posCorr[iBar]  = new TH1F(Form("h_tDiff_ampCorr_posCorr_BAR%d",iBar),Form("h_tDiff_ampCorr_posCorr_BAR%d",iBar),nTimeBins/2, -4, 4);
    p_tDiff_ampCorr_vs_posX[iBar]  = new TProfile(Form("p_tDiff_ampCorr_vs_posX_BAR%d",iBar),Form("p_tDiff_ampCorr_vs_posX_BAR%d",iBar), 100, xmin, xmax, -4, 4);
    p_tDiff_ampCorr_vs_posXc[iBar] = new TProfile(Form("p_tDiff_ampCorr_vs_posXc_BAR%d",iBar),Form("p_tDiff_ampCorr_vs_posXc_BAR%d",iBar), 100, xmin/cos(theta[iBar]), xmax/cos(theta[iBar]), -4, 4);
    p_tDiff_ampCorr_posCorr_vs_posX[iBar]  = new TProfile(Form("p_tDiff_ampCorr_posCorr_vs_posX_BAR%d",iBar),Form("p_tDiff_ampCorr_posCorr_vs_posX_BAR%d",iBar), 100, xmin, xmax, -4, 4);
    p_tDiff_ampCorr_posCorr_vs_posXc[iBar] = new TProfile(Form("p_tDiff_ampCorr_posCorr_vs_posXc_BAR%d",iBar),Form("p_tDiff_ampCorr_posCorr_vs_posXc_BAR%d",iBar), 200, xmin/cos(theta[iBar]), xmax/cos(theta[iBar]), -4, 4);
    h2_tDiff_vs_posX[iBar] = new TH2F(Form("h2_tDiff_vs_posX_BAR%d",iBar), Form("h2_tDiff_vs_posX_BAR%d",iBar), 100, xmin, xmax, 200, -4, 4);
    h2_tDiff_ampCorr_vs_posX[iBar] = new TH2F(Form("h2_tDiff_ampCorr_vs_posX_BAR%d",iBar), Form("h2_tDiff_ampCorr_vs_posX_BAR%d",iBar), 400, xmin, xmax, 400, -4, 4);


    // -- tAverage
    p2_tAve_vs_posXY[iBar]        = new TProfile2D(Form("p2_tAve_vs_posXY_BAR%d",iBar),Form("p2_tAve_vs_posXY_BAR%d",iBar), 400, xmin, xmax, 200, ymin, ymax, dtminAve[iBar], dtmaxAve[iBar]);

    h_tAve[iBar]                  = new TH1F(Form("h_tAve_BAR%d",iBar),Form("h_tAve_BAR%d",iBar),nTimeBins, dtminAve[iBar], dtmaxAve[iBar]);
    p_tAve_vs_posX[iBar]          = new TProfile(Form("p_tAve_vs_posX_BAR%d",iBar),Form("p_tAve_vs_posX_BAR%d",iBar), 100, xmin, xmax, dtminAve[iBar], dtmaxAve[iBar]);
    p_tAve_vs_posY[iBar]          = new TProfile(Form("p_tAve_vs_posY_BAR%d",iBar),Form("p_tAve_vs_posY_BAR%d",iBar), 100, ymin, ymax, dtminAve[iBar], dtmaxAve[iBar]);
    p_tAve_vs_tDiff[iBar]         = new TProfile(Form("p_tAve_vs_tDiff_BAR%d",iBar),Form("p_tAve_vs_tDiff_BAR%d",iBar), 100, -4, 4, dtminAve[iBar], dtmaxAve[iBar]);


    // -- plain tAve
    h_tAve_ampCorr[iBar]          = new TH1F(Form("h_tAve_ampCorr_BAR%d",iBar),Form("h_tAve_ampCorr_BAR%d",iBar), nTimeBins, dtminAve[iBar], dtmaxAve[iBar]);
    p_tAve_ampCorr_vs_posX[iBar]  = new TProfile(Form("p_tAve_ampCorr_vs_posX_BAR%d",iBar),Form("p_tAve_ampCorr_vs_posX_BAR%d",iBar), 100, xmin, xmax, dtminAve[iBar], dtmaxAve[iBar]);
    p_tAve_ampCorr_vs_posY[iBar]  = new TProfile(Form("p_tAve_ampCorr_vs_posY_BAR%d",iBar),Form("p_tAve_ampCorr_vs_posY_BAR%d",iBar), 100, ymin, ymax, dtminAve[iBar], dtmaxAve[iBar]);
    p_tAve_ampCorr_vs_tDiff[iBar] = new TProfile(Form("p_tAve_ampCorr_vs_tDiff_BAR%d",iBar),Form("p_tAve_ampCorr_vs_tDiff_BAR%d",iBar), 100, -4, 4, dtminAve[iBar], dtmaxAve[iBar]);
    p_tAve_ampCorr_vs_posXc[iBar]  = new TProfile(Form("p_tAve_ampCorr_vs_posXc_BAR%d",iBar),Form("p_tAve_ampCorr_vs_posXc_BAR%d",iBar), 100, xmin/cos(theta[iBar]), xmax/cos(theta[iBar]), dtminAve[iBar], dtmaxAve[iBar]);
    for (int ibin = 0; ibin < NBINSX[iBar]; ibin++){
      h_tL_ampCorr_binX[iBar][ibin] = new TH1F(Form("h_tL_ampCorr_binX_%d_BAR%d",ibin, iBar),Form("h_tL_ampCorr_binX_%d_BAR%d",ibin,iBar), nbinsHistoX, dtminL[iBar], dtmaxL[iBar]);
      h_tR_ampCorr_binX[iBar][ibin] = new TH1F(Form("h_tR_ampCorr_binX_%d_BAR%d",ibin, iBar),Form("h_tR_ampCorr_binX_%d_BAR%d",ibin,iBar), nbinsHistoX, dtminR[iBar], dtmaxR[iBar]);
      h_tAve_ampCorr_binX[iBar][ibin] = new TH1F(Form("h_tAve_ampCorr_binX_%d_BAR%d",ibin, iBar),Form("h_tAve_ampCorr_binX_%d_BAR%d",ibin,iBar), nbinsHistoX, dtminAve[iBar], dtmaxAve[iBar]);
    }
    for (int ibin = 0; ibin < NBINSY; ibin++){
      h_tAve_ampCorr_binY[iBar][ibin] = new TH1F(Form("h_tAve_ampCorr_binY_%d_BAR%d",ibin, iBar),Form("h_tAve_ampCorr_binY_%d_BAR%d",ibin,iBar), nbinsHistoY, dtminAve[iBar], dtmaxAve[iBar]);
    }


    // -- plain tAve after correction vs tDiff residual dependence
    h_tAve_ampCorr_tDiffCorr[iBar]          = new TH1F(Form("h_tAve_ampCorr_tDiffCorr_BAR%d",iBar),Form("h_tAve_ampCorr_tDiffCorr_BAR%d",iBar), nTimeBins, dtminAve[iBar], dtmaxAve[iBar]);
    p_tAve_ampCorr_tDiffCorr_vs_posX[iBar]  = new TProfile(Form("p_tAve_ampCorr_tDiffCorr_vs_posX_BAR%d",iBar),Form("p_tAve_ampCorr_tDiffCorr_vs_posX_BAR%d",iBar), 100, xmin, xmax, dtminAve[iBar], dtmaxAve[iBar]);
    p_tAve_ampCorr_tDiffCorr_vs_posY[iBar]  = new TProfile(Form("p_tAve_ampCorr_tDiffCorr_vs_posY_BAR%d",iBar),Form("p_tAve_ampCorr_tDiffCorr_vs_posY_BAR%d",iBar), 100, ymin, ymax, dtminAve[iBar], dtmaxAve[iBar]);
    p_tAve_ampCorr_tDiffCorr_vs_tDiff[iBar] = new TProfile(Form("p_tAve_ampCorr_tDiffCorr_vs_tDiff_BAR%d",iBar),Form("p_tAve_ampCorr_tDiffCorr_vs_tDiff_BAR%d",iBar), 100, -4, 4, dtminAve[iBar], dtmaxAve[iBar]);
    p_tAve_ampCorr_tDiffCorr_vs_posXc[iBar]  = new TProfile(Form("p_tAve_ampCorr_tDiffCorr_vs_posXc_BAR%d",iBar),Form("p_tAve_ampCorr_tDiffCorr_vs_posXc_BAR%d",iBar), 100, xmin/cos(theta[iBar]), xmax/cos(theta[iBar]), dtminAve[iBar], dtmaxAve[iBar]);
    for (int ibin = 0; ibin < NBINSX[iBar]; ibin++){
      h_tAve_ampCorr_tDiffCorr_binX[iBar][ibin] = new TH1F(Form("h_tAve_ampCorr_tDiffCorr_binX_%d_BAR%d",ibin, iBar),Form("h_tAve_ampCorr_tDiffCorr_binX_%d_BAR%d",ibin,iBar), nbinsHistoX, dtminAve[iBar], dtmaxAve[iBar]);
    }
    for (int ibin = 0; ibin < NBINSY; ibin++){
      h_tAve_ampCorr_tDiffCorr_binY[iBar][ibin] = new TH1F(Form("h_tAve_ampCorr_tDiffCorr_binY_%d_BAR%d",ibin, iBar),Form("h_tAve_ampCorr_tDiffCorr_binY_%d_BAR%d",ibin,iBar), nbinsHistoY, dtminAve[iBar], dtmaxAve[iBar]);
    }


    // -- plain tAve after correction vs position dependence
    h_tAve_ampCorr_posCorr[iBar]          = new TH1F(Form("h_tAve_ampCorr_posCorr_BAR%d",iBar),Form("h_tAve_ampCorr_posCorr_BAR%d",iBar), nTimeBins, dtminAve[iBar], dtmaxAve[iBar]);
    p_tAve_ampCorr_posCorr_vs_posX[iBar]  = new TProfile(Form("p_tAve_ampCorr_posCorr_vs_posX_BAR%d",iBar),Form("p_tAve_ampCorr_posCorr_vs_posX_BAR%d",iBar), 100, xmin, xmax, dtminAve[iBar], dtmaxAve[iBar]);
    p_tAve_ampCorr_posCorr_vs_posY[iBar]  = new TProfile(Form("p_tAve_ampCorr_posCorr_vs_posY_BAR%d",iBar),Form("p_tAve_ampCorr_posCorr_vs_posY_BAR%d",iBar), 100, ymin, ymax, dtminAve[iBar], dtmaxAve[iBar]);
    p_tAve_ampCorr_posCorr_vs_tDiff[iBar] = new TProfile(Form("p_tAve_ampCorr_posCorr_vs_tDiff_BAR%d",iBar),Form("p_tAve_ampCorr_posCorr_vs_tDiff_BAR%d",iBar), 100, -4, 4, dtminAve[iBar], dtmaxAve[iBar]);
    p_tAve_ampCorr_posCorr_vs_posXc[iBar]  = new TProfile(Form("p_tAve_ampCorr_posCorr_vs_posXc_BAR%d",iBar),Form("p_tAve_ampCorr_posCorr_vs_posXc_BAR%d",iBar), 100, xmin/cos(theta[iBar]), xmax/cos(theta[iBar]), dtminAve[iBar], dtmaxAve[iBar]);
    for (int ibin = 0; ibin < NBINSX[iBar]; ibin++){
      h_tAve_ampCorr_posCorr_binX[iBar][ibin] = new TH1F(Form("h_tAve_ampCorr_posCorr_binX_%d_BAR%d",ibin, iBar),Form("h_tAve_ampCorr_posCorr_binX_%d_BAR%d",ibin,iBar), nbinsHistoX, dtminAve[iBar], dtmaxAve[iBar]);
    }


    // -- tDiff 
    for (int ibin = 0; ibin < NBINSX[iBar]; ibin++){
      h_tDiff_ampCorr_binX[iBar][ibin] = new TH1F(Form("h_tDiff_ampCorr_binX_%d_BAR%d",ibin, iBar),Form("h_tDiff_ampCorr_binX_%d_BAR%d",ibin,iBar), int(nbinsHistoX/2), -4, 4);
      h_tDiff_ampCorr_posCorr_binX[iBar][ibin] = new TH1F(Form("h_tDiff_ampCorr_posCorr_binX_%d_BAR%d",ibin, iBar),Form("h_tDiff_ampCorr_posCorr_binX_%d_BAR%d",ibin,iBar), int(nbinsHistoX/2), -4, 4);
    }



    // binned amp walk corr                                                                                                                     
    for (int ibin = 0; ibin < NBINSXAMPWALK[iBar]; ibin++){
      p_tL_vs_amp_binned[iBar][ibin] = new TProfile(Form("p_tL_vs_amp_binXc_%d_BAR%d",ibin,iBar),Form("p_tL_vs_amp_binXc_%d_BAR%d",ibin,iBar), nAmpBins, 0., 1.,dtminL[iBar], dtmaxL[iBar]);
      p_tR_vs_amp_binned[iBar][ibin] = new TProfile(Form("p_tR_vs_amp_binXc_%d_BAR%d",ibin,iBar),Form("p_tR_vs_amp_binXc_%d_BAR%d",ibin,iBar),  nAmpBins, 0., 1.,dtminR[iBar], dtmaxR[iBar]);
      p_tL_ampCorr_vs_amp_binned[iBar][ibin] = new TProfile(Form("p_tL_ampCorr_vs_amp_binXc_%d_BAR%d",ibin,iBar),Form("p_tL_ampCorr_vs_amp_binXc_%d_BAR%d",ibin,iBar), nAmpBins, 0., 1.,dtminL[iBar], dtmaxL[iBar]);
      p_tR_ampCorr_vs_amp_binned[iBar][ibin] = new TProfile(Form("p_tR_ampCorr_vs_amp_binXc_%d_BAR%d",ibin,iBar),Form("p_tR_ampCorr_vs_amp_binXc_%d_BAR%d",ibin,iBar), nAmpBins, 0., 1.,dtminR[iBar], dtmaxR[iBar]);
    }



  }




  // -- combination of the two bars
  float dtminAveSum = -20;
  float dtmaxAveSum = 20;

  TH2F *h2_tAve1_vs_tAve2 = new TH2F("h2_tAve1_vs_tAve2","h2_tAve1_vs_tAve2", 200, -1, 1, 200, -1, 1);


  // -- simple average
  TH1F*          h_tAveSum_ampCorr = new TH1F("h_tAveSum_ampCorr","h_tAveSum_ampCorr",1000, dtminAveSum, dtmaxAveSum);
  TProfile*      p_tAveSum_ampCorr_vs_posX  = new TProfile("p_tAveSum_ampCorr_vs_posX","p_tAveSum_ampCorr_vs_posX", 400, xmin, xmax, dtminAveSum, dtmaxAveSum);
  TProfile*      p_tAveSum_ampCorr_vs_posY  = new TProfile("p_tAveSum_ampCorr_vs_posY","p_tAveSum_ampCorr_vs_posY", 200, ymin, ymax, dtminAveSum, dtmaxAveSum);
  TH1F*          h_tAveSum_ampCorr_binX[1000];
  for (int ibin = 0; ibin < NBINSX[0]; ibin++){
    h_tAveSum_ampCorr_binX[ibin] = new TH1F(Form("h_tAveSum_ampCorr_binX_%d",ibin),Form("h_tAveSum_ampCorr_binX_%d",ibin), nbinsHistoX, dtminAveSum, dtmaxAveSum);
  }
  TH1F*          h_tAveSum_ampCorr_binY[1000] ;
  for (int ibin = 0; ibin < NBINSY; ibin++){
    h_tAveSum_ampCorr_binY[ibin] = new TH1F(Form("h_tAveSum_ampCorr_binY_%d",ibin),Form("h_tAveSum_ampCorr_binY_%d",ibin), nbinsHistoY, dtminAveSum, dtmaxAveSum);
  }

  TH1F*          h_tAveSum_ampCorr_tDiffCorr = new TH1F("h_tAveSum_ampCorr_tDiffCorr","h_tAveSum_ampCorr_tDiffCorr",1000, dtminAveSum, dtmaxAveSum);
  TProfile*      p_tAveSum_ampCorr_tDiffCorr_vs_posX  = new TProfile("p_tAveSum_ampCorr_tDiffCorr_vs_posX","p_tAveSum_ampCorr_tDiffCorr_vs_posX", 400, xmin, xmax, dtminAveSum, dtmaxAveSum);
  TProfile*      p_tAveSum_ampCorr_tDiffCorr_vs_posY  = new TProfile("p_tAveSum_ampCorr_tDiffCorr_vs_posY","p_tAveSum_ampCorr_tDiffCorr_vs_posY", 200, ymin, ymax, dtminAveSum, dtmaxAveSum);
  TH1F*          h_tAveSum_ampCorr_tDiffCorr_binX[1000];
  for (int ibin = 0; ibin < NBINSX[0]; ibin++){
    h_tAveSum_ampCorr_tDiffCorr_binX[ibin] = new TH1F(Form("h_tAveSum_ampCorr_tDiffCorr_binX_%d",ibin),Form("h_tAveSum_ampCorr_tDiffCorr_binX_%d",ibin), nbinsHistoX, dtminAveSum, dtmaxAveSum);
  }
  TH1F*          h_tAveSum_ampCorr_tDiffCorr_binY[1000] ;
  for (int ibin = 0; ibin < NBINSY; ibin++){
    h_tAveSum_ampCorr_tDiffCorr_binY[ibin] = new TH1F(Form("h_tAveSum_ampCorr_tDiffCorr_binY_%d",ibin),Form("h_tAveSum_ampCorr_tDiffCorr_binY_%d",ibin), nbinsHistoY, dtminAveSum, dtmaxAveSum);
  }


  // -- amp weighted average
  TH1F*          h_tAveAmpWSum_ampCorr = new TH1F("h_tAveAmpWSum_ampCorr","h_tAveAmpWSum_ampCorr",1000, dtminAveSum, dtmaxAveSum);
  TProfile*      p_tAveAmpWSum_ampCorr_vs_posX  = new TProfile("p_tAveAmpWSum_ampCorr_vs_posX","p_tAveAmpWSum_ampCorr_vs_posX", 400, xmin, xmax, dtminAveSum, dtmaxAveSum);
  TProfile*      p_tAveAmpWSum_ampCorr_vs_posY  = new TProfile("p_tAveAmpWSum_ampCorr_vs_posY","p_tAveAmpWSum_ampCorr_vs_posY", 200, ymin, ymax, dtminAveSum, dtmaxAveSum);
  TH1F*          h_tAveAmpWSum_ampCorr_binX[1000];
  for (int ibin = 0; ibin < NBINSX[0]; ibin++){
    h_tAveAmpWSum_ampCorr_binX[ibin] = new TH1F(Form("h_tAveAmpWSum_ampCorr_binX_%d",ibin),Form("h_tAveAmpWSum_ampCorr_binX_%d",ibin), nbinsHistoX, dtminAveSum, dtmaxAveSum);
  }
  TH1F*          h_tAveAmpWSum_ampCorr_binY[1000] ;
  for (int ibin = 0; ibin < NBINSY; ibin++){
    h_tAveAmpWSum_ampCorr_binY[ibin] = new TH1F(Form("h_tAveAmpWSum_ampCorr_binY_%d",ibin),Form("h_tAveAmpWSum_ampCorr_binY_%d",ibin), nbinsHistoY, dtminAveSum, dtmaxAveSum);
  }


  TH1F*          h_tAveAmpWSum_ampCorr_tDiffCorr = new TH1F("h_tAveAmpWSum_ampCorr_tDiffCorr","h_tAveAmpWSum_ampCorr_tDiffCorr",1000, dtminAveSum, dtmaxAveSum);
  TProfile*      p_tAveAmpWSum_ampCorr_tDiffCorr_vs_posX  = new TProfile("p_tAveAmpWSum_ampCorr_tDiffCorr_vs_posX","p_tAveAmpWSum_ampCorr_tDiffCorr_vs_posX", 400, xmin, xmax, dtminAveSum, dtmaxAveSum);
  TProfile*      p_tAveAmpWSum_ampCorr_tDiffCorr_vs_posY  = new TProfile("p_tAveAmpWSum_ampCorr_tDiffCorr_vs_posY","p_tAveAmpWSum_ampCorr_tDiffCorr_vs_posY", 200, ymin, ymax, dtminAveSum, dtmaxAveSum);
  TH1F*          h_tAveAmpWSum_ampCorr_tDiffCorr_binX[1000];
  for (int ibin = 0; ibin < NBINSX[0]; ibin++){
    h_tAveAmpWSum_ampCorr_tDiffCorr_binX[ibin] = new TH1F(Form("h_tAveAmpWSum_ampCorr_tDiffCorr_binX_%d",ibin),Form("h_tAveAmpWSum_ampCorr_tDiffCorr_binX_%d",ibin), nbinsHistoX, dtminAveSum, dtmaxAveSum);
  }
  TH1F*          h_tAveAmpWSum_ampCorr_tDiffCorr_binY[1000] ;
  for (int ibin = 0; ibin < NBINSY; ibin++){
    h_tAveAmpWSum_ampCorr_tDiffCorr_binY[ibin] = new TH1F(Form("h_tAveAmpWSum_ampCorr_tDiffCorr_binY_%d",ibin),Form("h_tAveAmpWSum_ampCorr_tDiffCorr_binY_%d",ibin), nbinsHistoY, dtminAveSum, dtmaxAveSum);
  }


  // -- time resolution weighted average
  TH1F*          h_tAveResWSum_ampCorr = new TH1F("h_tAveResWSum_ampCorr","h_tAveResWSum_ampCorr",1000, dtminAveSum, dtmaxAveSum);
  TProfile*      p_tAveResWSum_ampCorr_vs_posX  = new TProfile("p_tAveResWSum_ampCorr_vs_posX","p_tAveResWSum_ampCorr_vs_posX", 400, xmin, xmax, dtminAveSum, dtmaxAveSum);
  TProfile*      p_tAveResWSum_ampCorr_vs_posY  = new TProfile("p_tAveResWSum_ampCorr_vs_posY","p_tAveResWSum_ampCorr_vs_posY", 200, ymin, ymax, dtminAveSum, dtmaxAveSum);
  TH1F*          h_tAveResWSum_ampCorr_binX[1000];
  for (int ibin = 0; ibin < NBINSX[0]; ibin++){
    h_tAveResWSum_ampCorr_binX[ibin] = new TH1F(Form("h_tAveResWSum_ampCorr_binX_%d",ibin),Form("h_tAveResWSum_ampCorr_binX_%d",ibin), nbinsHistoX, dtminAveSum, dtmaxAveSum);
  }
  TH1F*          h_tAveResWSum_ampCorr_binY[1000] ;
  for (int ibin = 0; ibin < NBINSY; ibin++){
    h_tAveResWSum_ampCorr_binY[ibin] = new TH1F(Form("h_tAveResWSum_ampCorr_binY_%d",ibin),Form("h_tAveResWSum_ampCorr_binY_%d",ibin), nbinsHistoY, dtminAveSum, dtmaxAveSum);
  }


  TH1F*          h_tAveResWSum_ampCorr_tDiffCorr = new TH1F("h_tAveResWSum_ampCorr_tDiffCorr","h_tAveResWSum_ampCorr_tDiffCorr",1000, dtminAveSum, dtmaxAveSum);
  TProfile*      p_tAveResWSum_ampCorr_tDiffCorr_vs_posX  = new TProfile("p_tAveResWSum_ampCorr_tDiffCorr_vs_posX","p_tAveResWSum_ampCorr_tDiffCorr_vs_posX", 400, xmin, xmax, dtminAveSum, dtmaxAveSum);
  TProfile*      p_tAveResWSum_ampCorr_tDiffCorr_vs_posY  = new TProfile("p_tAveResWSum_ampCorr_tDiffCorr_vs_posY","p_tAveResWSum_ampCorr_tDiffCorr_vs_posY", 200, ymin, ymax, dtminAveSum, dtmaxAveSum);
  TH1F*          h_tAveResWSum_ampCorr_tDiffCorr_binX[1000];
  for (int ibin = 0; ibin < NBINSX[0]; ibin++){
    h_tAveResWSum_ampCorr_tDiffCorr_binX[ibin] = new TH1F(Form("h_tAveResWSum_ampCorr_tDiffCorr_binX_%d",ibin),Form("h_tAveResWSum_ampCorr_tDiffCorr_binX_%d",ibin), nbinsHistoX, dtminAveSum, dtmaxAveSum);
  }
  TH1F*          h_tAveResWSum_ampCorr_tDiffCorr_binY[1000] ;
  for (int ibin = 0; ibin < NBINSY; ibin++){
    h_tAveResWSum_ampCorr_tDiffCorr_binY[ibin] = new TH1F(Form("h_tAveResWSum_ampCorr_tDiffCorr_binY_%d",ibin),Form("h_tAveResWSum_ampCorr_tDiffCorr_binY_%d",ibin), nbinsHistoY, dtminAveSum, dtmaxAveSum);
  }


  TGraphErrors *g_nEventsRatio_vs_posY = new TGraphErrors();
  g_nEventsRatio_vs_posY->SetName("g_nEventsRatio_vs_posY");


  float ampRef = 0;
  float tRef = 0;

  float ampL  = 0.;
  float ampR  = 0.;
  float tL    = 0.;
  float tR    = 0.;
  float tAve  = 0.;

  float brmsL = 0;
  float brmsR = 0;
  float brmsRef = 0;

  float posX = 0;
  float posY = 0;
  
  float kAdcToV = 1./4096.; // factor to convert amplitudes in V

  //-----------------------
  // zeroth loop over events to find mip peak
  std::cout<<" >>> Zeroth-A loop over events to find mip peak ..." <<std::endl;
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

      // -- beam profile 
      h_beamXY ->Fill(posX, posY);
      h_beamX  ->Fill(posX);
      h_beamY  ->Fill(posY);


      // -- Photek as ref
      ampRef = treeVars.t_amp[(*treeVars.t_channelId)[ampChannelRef]] * kAdcToV ;
      tRef   = treeVars.t_time[(*treeVars.t_channelId)[timeChannelRef]+treeVars.t_CFD];
      h_ampRef_nocuts->Fill(ampRef);
      if ( ampRef > 0.020 ) h_timeRef -> Fill(tRef);

      // -- loop over bars
      for (int iBar = 0; iBar < NBARS; iBar++){

        ampL = treeVars.t_amp[(*treeVars.t_channelId)[ ampChannelsL[iBar] ]] * kAdcToV ;
        ampR = treeVars.t_amp[(*treeVars.t_channelId)[ ampChannelsR[iBar] ]] * kAdcToV ;

	tL   = treeVars.t_time[(*treeVars.t_channelId)[ timeChannelsL[iBar] ]+treeVars.t_LED];
        tR   = treeVars.t_time[(*treeVars.t_channelId)[ timeChannelsR[iBar] ]+treeVars.t_LED];

	// -- select events
        if ( posX < cut_XminRef || posX > cut_XmaxRef ) continue;
        if ( posY < cut_YminRef || posY > cut_YmaxRef ) continue;
	if ( posX < cut_Xmin[iBar] || posX > cut_Xmax[iBar] ) continue;
	if ( posY < cut_Ymin[iBar] || posY > cut_Ymax[iBar] ) continue;
	
	// -- fill amp histograms before any selection
	h_ampL_nocuts[iBar] -> Fill(ampL);
	h_ampR_nocuts[iBar] -> Fill(ampR);

	if ( ampL > 20 * kAdcToV ) h_timeL[iBar] -> Fill(tL);
	if ( ampR > 20 * kAdcToV ) h_timeR[iBar] -> Fill(tR);      
      }
    }

  
  // --  Find mip peak MCP
  TF1 *ffitRef = new TF1("ffitRef","gaus",0, 1000); 
  ffitRef->SetRange(0.03,0.2); 
  h_ampRef_nocuts->Fit("ffitRef","QR");
  float mipPeakRef = ffitRef-> GetParameter(1);  
  float cut_ampMinRef =  mipPeakRef - 2. * ffitRef-> GetParameter(2);
  float cut_ampMaxRef =  mipPeakRef + 2. * ffitRef-> GetParameter(2);
  //  float cut_ampMaxRef = 0.90;

  // --  Find mip peak for each bar
  TF1 *fLandauL[NBARS];
  TF1 *fLandauR[NBARS];
  float mipPeakL[NBARS];
  float mipPeakR[NBARS];
  std::vector<float> cut_ampMinL, cut_ampMinR, cut_ampMaxL, cut_ampMaxR;
  cut_ampMinL.clear();
  cut_ampMinR.clear();
  cut_ampMaxL.clear();
  cut_ampMaxR.clear();
  float  maxAmpSaturation = 0.90;
  if (runMax < 6402) maxAmpSaturation = 0.50;
  cout << " maxAmpSaturation = " << maxAmpSaturation <<endl;
  for (int iBar =0; iBar< NBARS; iBar++){
    fLandauL[iBar] = new TF1(Form("fLandauL_BAR%d",iBar),"landau",0, 1000);
    if (runMax >= 8629 && runMax <= 8922){ // 45 deg
      fLandauL[iBar]->SetRange(0.100, maxAmpSaturation);
      fLandauL[iBar]->SetParameter(1, 0.120);
    }
    if ( runMax >= 8943 ){ // 90, 60, 30 deg, 2mm bar
      fLandauL[iBar]->SetRange(0.060, maxAmpSaturation);
      fLandauL[iBar]->SetParameter(1, 0.100);
    }
    h_ampL_nocuts[iBar] -> Fit(fLandauL[iBar],"QR");  
    mipPeakL[iBar] = fLandauL[iBar]-> GetParameter(1);
    cut_ampMinL.push_back(cut_ampMinFraction * mipPeakL[iBar]);
    float maxamp = min(5*mipPeakL[iBar],maxAmpSaturation);
    //cut_ampMaxL.push_back(maxamp);
    cut_ampMaxL.push_back(maxAmpSaturation);
    fLandauR[iBar] = new TF1(Form("fLandauR_BAR%d",iBar),"landau",0, 1000);
    if (runMax >= 8629 && runMax <= 8922){ // 45 deg
      fLandauR[iBar]->SetRange(0.100, maxAmpSaturation);
      fLandauR[iBar]->SetParameter(1, 0.120);
    }    
    if ( runMax >= 8943 ){ // 90, 60, 30 deg, 2mm bar
      fLandauR[iBar]->SetRange(0.060, maxAmpSaturation);
      fLandauR[iBar]->SetParameter(1, 0.100);
    }
    h_ampR_nocuts[iBar] -> Fit(fLandauR[iBar],"QR");  
    mipPeakR[iBar] = fLandauR[iBar]-> GetParameter(1);
    cut_ampMinR.push_back(cut_ampMinFraction * mipPeakR[iBar]);
    maxamp = min(5*mipPeakR[iBar],maxAmpSaturation);
    //cut_ampMaxR.push_back(maxamp);
    cut_ampMaxR.push_back(maxAmpSaturation);
  }

  cout << "Amplitude selections" <<endl;
  cout << "Photek: " << cut_ampMinRef << "  " << cut_ampMaxRef <<endl;
  for (int iBar = 0; iBar< NBARS; iBar++){
    cout << "BAR"<< iBar << "   Left:   minAmp (V) = " << cut_ampMinL[iBar] << "     maxAmp (V) = " << cut_ampMaxL[iBar] << "         Right:  minAmp (V) " << cut_ampMinR[iBar] << "    maxAmp (V) = " << cut_ampMaxR[iBar] <<endl;
  }


  // -- Fit time to get time selections
  vector<float> tLmin ;
  vector<float> tLmax ;
  vector<float> tRmin ;
  vector<float> tRmax ;  

  float nSigma = 2;
  TF1 *fG = new TF1("fG","gaus",-200, 200);
  fG->SetParameter(1,h_timeRef->GetMean());
  fG->SetParameter(2, 2.);
  fG->SetRange(h_timeRef->GetMean()-10, h_timeRef->GetMean()+10);
  h_timeRef->Fit("fG","QR");
  cut_minTimeRef = fG->GetParameter(1) - nSigma*fG->GetParameter(2);
  cut_maxTimeRef = fG->GetParameter(1) + nSigma*fG->GetParameter(2);

  for (int iBar = 0; iBar < NBARS; iBar++){
    fG->SetParameter(1,h_timeL[iBar]->GetMean()); 
    fG->SetParameter(2, 2.); 
    fG->SetRange(h_timeL[iBar]->GetMean()-10, h_timeL[iBar]->GetMean()+10); 
    h_timeL[iBar]->Fit("fG","QR");
    float meanL = fG->GetParameter(1);
    float sigmaL = fG->GetParameter(2);
    tLmin.push_back(meanL-nSigma*sigmaL);
    tLmax.push_back(meanL+nSigma*sigmaL);
    fG->SetParameter(1,h_timeR[iBar]->GetMean()); 
    fG->SetParameter(2,2.);
    fG->SetRange(h_timeR[iBar]->GetMean()-10, h_timeR[iBar]->GetMean()+10);  
    h_timeR[iBar]->Fit("fG","QR");
    float meanR = fG->GetParameter(1);
    float sigmaR = fG->GetParameter(2);
    tRmin.push_back(meanR-nSigma*sigmaR);
    tRmax.push_back(meanR+nSigma*sigmaR);
  }

  
  cout << "Time selections" <<endl;
  cout << "Photek: " << cut_minTimeRef << "  " << cut_maxTimeRef <<endl;
  for (int iBar = 0; iBar< NBARS; iBar++){
    cout << "BAR"<< iBar << "   Left:   minTime (ns) = " << tLmin[iBar] << "     maxTime (ns) = " << tLmax[iBar] << "         Right:  minTime (ns) " << tRmin[iBar] << "    maxTime (ns) = " << tRmax[iBar] <<endl;
  }


  // -- loop to select time
  std::cout<<" >>> Zeroth-B loop over events" <<std::endl; 
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
      brmsRef = treeVars.t_b_rms[(*treeVars.t_channelId)[ampChannelRef]];

      if ( ampRef > cut_ampMinRef  &&  ampRef < cut_ampMaxRef ){
	h_ampRef->Fill(ampRef);
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
      
      if ( ampRef > cut_ampMinRef  &&  ampRef < cut_ampMaxRef && tRef > cut_minTimeRef && tRef < cut_maxTimeRef ) {
	if (posX > cut_XminRef && posX < cut_XmaxRef && posY < cut_YmaxRef && posY >cut_YminRef &&
	    posX > cut_Xmin[0] && posX < cut_Xmax[0] && posY < cut_Ymax[0] && posY >cut_Ymin[NBARS-1] ){
	  p_timeRef_vs_ampRef->Fill(ampRef, tRef);
	  p_ampRef_vs_posX->Fill(posX,ampRef);
	  p_ampRef_vs_posY->Fill(posY,ampRef);
	  p_timeRef_vs_posX->Fill(posX,tRef);
	  p_timeRef_vs_posY->Fill(posY,tRef);
	}
	p2_ampRef_vs_posXY ->Fill(posX, posY, ampRef);
	p2_timeRef_vs_posXY->Fill(posX, posY, tRef);
	h_brmsRef ->Fill(treeVars.t_b_rms[(*treeVars.t_channelId)[ ampChannelRef] ]);
      }
      

      // -- loop over bars
      for (int iBar = 0; iBar < NBARS; iBar++){
	
        ampL = treeVars.t_amp[(*treeVars.t_channelId)[ ampChannelsL[iBar] ]] * kAdcToV ;
        ampR = treeVars.t_amp[(*treeVars.t_channelId)[ ampChannelsR[iBar] ]] * kAdcToV ;
	
        tL   = treeVars.t_time[(*treeVars.t_channelId)[ timeChannelsL[iBar] ]+treeVars.t_LED];
        tR   = treeVars.t_time[(*treeVars.t_channelId)[ timeChannelsR[iBar] ]+treeVars.t_LED];
	
        brmsL = treeVars.t_b_rms[(*treeVars.t_channelId)[ timeChannelsL[iBar] ] ];
        brmsR = treeVars.t_b_rms[(*treeVars.t_channelId)[ timeChannelsR[iBar] ] ];
	
	if ( ampL > cut_ampMinL[iBar]  && ampR > cut_ampMinR[iBar] ) {
          p2_eff_vs_posXY[iBar] ->Fill(posX, posY, 1.);
          if ( posY > cut_Ymin[iBar] && posY < cut_Ymax[iBar] ) p_eff_vs_posX[iBar] ->Fill(posX, 1.);
          if ( posX > cut_Xmin[iBar] && posX < cut_Xmax[iBar] ) p_eff_vs_posY[iBar] ->Fill(posY, 1.);
        }
        else{
          p2_eff_vs_posXY[iBar] ->Fill(posX, posY, 0.);
          if ( posY > cut_Ymin[iBar] && posY < cut_Ymax[iBar] )   p_eff_vs_posX[iBar] ->Fill(posX, 0.);
          if ( posX > cut_Xmin[iBar] && posX < cut_Xmax[iBar] )   p_eff_vs_posY[iBar] ->Fill(posY, 0.);
	}
	
        // -- select events
        if ( posX < cut_XminRef || posX > cut_XmaxRef ) continue;
        if ( posY < cut_YminRef || posY > cut_YmaxRef ) continue;
        if ( posX < cut_Xmin[iBar] || posX > cut_Xmax[iBar] ) continue;
        if ( posY < cut_Ymin[iBar] || posY > cut_Ymax[iBar] ) continue;

        if ( ampRef < cut_ampMinRef     ||  ampRef > cut_ampMaxRef ) continue;
        if ( ampL   < cut_ampMinL[iBar] ||  ampL   > cut_ampMaxL[iBar] ) continue;
        if ( ampR   < cut_ampMinR[iBar] ||  ampR   > cut_ampMaxR[iBar] ) continue;

        if ( tRef < 0  ||  tRef > 200) continue;
        if ( tL   < 0  ||  tL   > 200) continue;
        if ( tR   < 0  ||  tR   > 200) continue;

        h2_timeL_vs_brms[iBar] -> Fill(brmsL, tL);
        h2_timeR_vs_brms[iBar] -> Fill(brmsR, tR);

	
	// -- remove events with bad reco time
        if ( tRef < cut_minTimeRef || tRef > cut_maxTimeRef ) continue;
        if ( tL < cut_minTime[iBar] || tL > cut_maxTime[iBar]) continue;
        if ( tR < cut_minTime[iBar] || tR > cut_maxTime[iBar] ) continue;
	
	if (tL < tLmin[iBar] || tL > tLmax[iBar]) continue;
        if (tR < tRmin[iBar] || tR > tRmax[iBar]) continue;

	h_brms_timeL[iBar] -> Fill(brmsL);
	h_brms_timeR[iBar] -> Fill(brmsR);

	// -- remove very noisy events
        if (brmsRef > cut_brmsMaxRef) continue;
        if (brmsR > cut_brmsMaxTimeCh[iBar]) continue;
        if (brmsL > cut_brmsMaxTimeCh[iBar]) continue;

        //-- time of Left, Right                                                                                                                            
        tL = tL - tRef;
        tR = tR - tRef;
        
	h_tL[iBar]->Fill(tL);
	h_tR[iBar]->Fill(tR);

      }
    }


  //-----------------------
  // first loop over events
  std::cout<<" >>> First loop over events to get amp walk corrections ..." <<std::endl;
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
      brmsRef = treeVars.t_b_rms[(*treeVars.t_channelId)[ampChannelRef]];


      // -- loop over bars
      for (int iBar = 0; iBar < NBARS; iBar++){

	ampL = treeVars.t_amp[(*treeVars.t_channelId)[ ampChannelsL[iBar] ]] * kAdcToV ;
	ampR = treeVars.t_amp[(*treeVars.t_channelId)[ ampChannelsR[iBar] ]] * kAdcToV ;
	
	tL   = treeVars.t_time[(*treeVars.t_channelId)[ timeChannelsL[iBar] ]+treeVars.t_LED];  
	tR   = treeVars.t_time[(*treeVars.t_channelId)[ timeChannelsR[iBar] ]+treeVars.t_LED];  
	
	brmsL = treeVars.t_b_rms[(*treeVars.t_channelId)[ timeChannelsL[iBar] ] ];
	brmsR = treeVars.t_b_rms[(*treeVars.t_channelId)[ timeChannelsR[iBar] ] ];
	
	// -- select events
        if ( posX < cut_XminRef || posX > cut_XmaxRef ) continue;
        if ( posY < cut_YminRef || posY > cut_YmaxRef ) continue;
	if ( posX < cut_Xmin[iBar] || posX > cut_Xmax[iBar] ) continue;
	if ( posY < cut_Ymin[iBar] || posY > cut_Ymax[iBar] ) continue;

	if ( ampRef < cut_ampMinRef     ||  ampRef > cut_ampMaxRef ) continue;
	if ( ampL   < cut_ampMinL[iBar] ||  ampL   > cut_ampMaxL[iBar] ) continue;
	if ( ampR   < cut_ampMinR[iBar] ||  ampR   > cut_ampMaxR[iBar] ) continue;

	if ( tRef < 0  ||  tRef > 200) continue;
	if ( tL   < 0  ||  tL   > 200) continue;
	if ( tR   < 0  ||  tR   > 200) continue;

	// -- remove very noisy events
	if (brmsRef > cut_brmsMaxRef) continue;
	if (brmsR > cut_brmsMaxTimeCh[iBar]) continue;
	if (brmsL > cut_brmsMaxTimeCh[iBar]) continue;
		
	// -- remove events with bad reco time
	if ( tRef < cut_minTimeRef || tRef > cut_maxTimeRef ) continue;
	if ( tL < cut_minTime[iBar] || tL > cut_maxTime[iBar]) continue;
	if ( tR < cut_minTime[iBar] || tR > cut_maxTime[iBar] ) continue;

	if (tL < tLmin[iBar] || tL > tLmax[iBar]) continue;
	if (tR < tRmin[iBar] || tR > tRmax[iBar]) continue;

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

	h2_tL_vs_amp[iBar]->Fill(ampL,tL);
        if ( tL > h_tL[iBar]->GetMean()-3*h_tL[iBar]->GetRMS() && tL < h_tL[iBar]->GetMean()+3*h_tL[iBar]->GetRMS() ) 
	{
	  p_tL_vs_amp[iBar]->Fill(ampL,tL);
	  int xcbinamp = int ((posX - max(cut_Xmin[iBar], cut_XminRef))/cos(theta[iBar])/xbinWidthForAmpWalk );
          p_tL_vs_amp_binned[iBar][xcbinamp]->Fill(ampL,tL);
	}
	p2_tL_vs_posXY[iBar] ->Fill(posX,posY,tL);
	p_tL_vs_posX[iBar]->Fill(posX,tL);
	p_tL_vs_posY[iBar]->Fill(posY,tL);
	

	// --- SiPM Right
        h_ampR[iBar]  -> Fill(ampR);
        p2_ampR_vs_posXY[iBar] ->Fill(posX,posY,ampR);
        p_ampR_vs_posX[iBar] ->Fill(posX,ampR);
        p_ampR_vs_posXc[iBar] ->Fill(posX/cos(theta[iBar]),ampR);
        p_ampR_vs_posY[iBar] ->Fill(posY,ampR);
        
	h2_tR_vs_amp[iBar]->Fill(ampR,tR);
	if ( tR > h_tR[iBar]->GetMean()-3*h_tR[iBar]->GetRMS() && tR < h_tR[iBar]->GetMean()+3*h_tR[iBar]->GetRMS() ) 
	{
	  p_tR_vs_amp[iBar]->Fill(ampR,tR);
	  int xcbinamp = int ((posX - max(cut_Xmin[iBar], cut_XminRef))/cos(theta[iBar])/xbinWidthForAmpWalk );
          p_tR_vs_amp_binned[iBar][xcbinamp] ->Fill(ampR,tR);
        }
	p2_tR_vs_posXY[iBar] ->Fill(posX,posY,tR);
        p_tR_vs_posX[iBar]->Fill(posX,tR);
        p_tR_vs_posY[iBar]->Fill(posY,tR);
        
	// -- tDiff
	h_tDiff[iBar]->Fill(tR-tL);

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
  TF1*  fitFuncL_ampCorr_binned[NBARS][1000];
  TF1*  fitFuncR_ampCorr_binned[NBARS][1000];

  for (int iBar = 0 ; iBar < NBARS; iBar++){
    fitFuncL_ampCorr[iBar] = new TF1(Form("fitFuncL_ampCorr_%d", iBar),fitFunction.c_str());
    fitFuncL_ampCorr[iBar] -> SetLineColor(kRed);
    float mpv = (h_ampL_nocuts[iBar]->GetFunction(Form("fLandauL_BAR%d",iBar)))->GetParameter(1);
    //fitFuncL_ampCorr[iBar] -> SetRange(0, 10*mpv);
    fitFuncL_ampCorr[iBar] -> SetRange(0, 1.);
    p_tL_vs_amp[iBar] -> Fit(Form("fitFuncL_ampCorr_%d",iBar),"QSR");
    if (fitFuncL_ampCorr[iBar] -> GetChisquare()/fitFuncL_ampCorr[iBar] -> GetNDF() > 5 ){
      p_tL_vs_amp[iBar] -> Fit(Form("fitFuncL_ampCorr_%d",iBar),"QSRW");
    }

    fitFuncR_ampCorr[iBar] = new TF1(Form("fitFuncR_ampCorr_%d", iBar),fitFunction.c_str());
    fitFuncR_ampCorr[iBar] -> SetLineColor(kRed);
    mpv = (h_ampR_nocuts[iBar]->GetFunction(Form("fLandauR_BAR%d",iBar)))->GetParameter(1);
    //fitFuncR_ampCorr[iBar] -> SetRange(0, 10*mpv);
    fitFuncR_ampCorr[iBar] -> SetRange(0, 1.);
    p_tR_vs_amp[iBar] -> Fit(Form("fitFuncR_ampCorr_%d",iBar),"QSR");
    if (fitFuncR_ampCorr[iBar] -> GetChisquare()/fitFuncR_ampCorr[iBar] -> GetNDF() > 5 ){
      p_tR_vs_amp[iBar] -> Fit(Form("fitFuncR_ampCorr_%d",iBar),"QSRW");
    }


    // amp corr binned in X
    for (int ibin =0; ibin < NBINSXAMPWALK[iBar]; ibin++){

      fitFuncL_ampCorr_binned[iBar][ibin] = new TF1(Form("fitFuncL_ampCorr_%d_%d",ibin,  iBar),fitFunction.c_str());
      fitFuncL_ampCorr_binned[iBar][ibin] -> SetLineColor(kRed);
      p_tL_vs_amp_binned[iBar][ibin] -> Fit(Form("fitFuncL_ampCorr_%d_%d",ibin, iBar),"QSR");
      if (fitFuncL_ampCorr_binned[iBar][ibin] -> GetChisquare()/fitFuncL_ampCorr_binned[iBar][ibin] -> GetNDF() > 5 ){
        p_tL_vs_amp_binned[iBar][ibin] -> Fit(Form("fitFuncL_ampCorr_%d_%d",ibin, iBar),"QSRW");
      }

      fitFuncR_ampCorr_binned[iBar][ibin] = new TF1(Form("fitFuncR_ampCorr_%d_%d", ibin, iBar),fitFunction.c_str());
      fitFuncR_ampCorr_binned[iBar][ibin] -> SetLineColor(kRed);
      p_tR_vs_amp_binned[iBar][ibin] -> Fit(Form("fitFuncR_ampCorr_%d_%d",ibin,iBar),"QSR");
      if (fitFuncR_ampCorr_binned[iBar][ibin] -> GetChisquare()/fitFuncR_ampCorr_binned[iBar][ibin] -> GetNDF() > 5 ){
        p_tR_vs_amp_binned[iBar][ibin] -> Fit(Form("fitFuncR_ampCorr_%d_%d",ibin,iBar),"QSRW");
      }
    }

    
  }


  // --- second loop over events -- fill histograms after applying amplitude walk corrections
  std::cout<<" >>> Second loop over events  after amp walk corrections ..." <<std::endl;
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
      brmsRef = treeVars.t_b_rms[(*treeVars.t_channelId)[ampChannelRef]];

      // -- loop over bars
      for (int iBar = 0; iBar < NBARS; iBar++){

	ampL = treeVars.t_amp[(*treeVars.t_channelId)[ ampChannelsL[iBar] ]] * kAdcToV ;
	ampR = treeVars.t_amp[(*treeVars.t_channelId)[ ampChannelsR[iBar] ]] * kAdcToV ;

	tL   = treeVars.t_time[(*treeVars.t_channelId)[ timeChannelsL[iBar] ]+treeVars.t_LED];  
	tR   = treeVars.t_time[(*treeVars.t_channelId)[ timeChannelsR[iBar] ]+treeVars.t_LED];  

	brmsL = treeVars.t_b_rms[(*treeVars.t_channelId)[ timeChannelsL[iBar] ] ];
	brmsR = treeVars.t_b_rms[(*treeVars.t_channelId)[ timeChannelsR[iBar] ] ];

	// -- select events
        if ( posX < cut_XminRef || posX > cut_XmaxRef ) continue;
        if ( posY < cut_YminRef || posY > cut_YmaxRef ) continue;
	if ( posX < cut_Xmin[iBar] || posX > cut_Xmax[iBar] ) continue;
	if ( posY < cut_Ymin[iBar] || posY > cut_Ymax[iBar] ) continue;

	if ( ampRef < cut_ampMinRef     ||  ampRef > cut_ampMaxRef ) continue;
	if ( ampL   < cut_ampMinL[iBar] ||  ampL   > cut_ampMaxL[iBar] ) continue;
	if ( ampR   < cut_ampMinR[iBar] ||  ampR   > cut_ampMaxR[iBar] ) continue;

	if ( tRef < 0  ||  tRef > 200) continue;
	if ( tL   < 0  ||  tL   > 200) continue;
	if ( tR   < 0  ||  tR   > 200) continue;

	if ( tRef < cut_minTimeRef || tRef > cut_maxTimeRef ) continue;
	if ( tL   < cut_minTime[iBar]  || tL   > cut_maxTime[iBar]  ) continue;
	if ( tR   < cut_minTime[iBar]  || tR   > cut_maxTime[iBar]  ) continue;

	if (tL < tLmin[iBar] || tL > tLmax[iBar]) continue;
	if (tR < tRmin[iBar] || tR > tRmax[iBar]) continue;

	// -- remove very noisy events
	if (brmsRef > cut_brmsMaxRef) continue;
        if (brmsR > cut_brmsMaxTimeCh[iBar]) continue;
        if (brmsL > cut_brmsMaxTimeCh[iBar]) continue;

	tL = tL - tRef;
        tR = tR - tRef;

	// -- amplitude walk correction 
	//int xcbinamp = int ((posX - max(cut_Xmin[iBar], cut_XminRef))/cos(theta[iBar])/xbinWidthForAmpWalk );
        //float tL_corr = tL - fitFuncL_ampCorr_binned[iBar][xcbinamp]->Eval(ampL) + fitFuncL_ampCorr_binned[iBar][xcbinamp]->Eval(h_ampL[iBar]->GetMean());
        //float tR_corr = tR - fitFuncR_ampCorr_binned[iBar][xcbinamp]->Eval(ampR) + fitFuncR_ampCorr_binned[iBar][xcbinamp]->Eval(h_ampR[iBar]->GetMean());
	//float tL_corr = tL - fitFuncL_ampCorr_binned[iBar][xcbinamp]->Eval(ampL);
        //float tR_corr = tR - fitFuncR_ampCorr_binned[iBar][xcbinamp]->Eval(ampR);
	//float tL_corr = tL - fitFuncL_ampCorr[iBar]->Eval(ampL) + fitFuncL_ampCorr[iBar]->Eval(h_ampL[iBar]->GetMean()) ; 
	//float tR_corr = tR - fitFuncR_ampCorr[iBar]->Eval(ampR) + fitFuncR_ampCorr[iBar]->Eval(h_ampR[iBar]->GetMean()) ; 
	float tL_corr = tL - fitFuncL_ampCorr[iBar]->Eval(ampL); 
	float tR_corr = tR - fitFuncR_ampCorr[iBar]->Eval(ampR);
	float tAve_ampCorr = ( tL_corr + tR_corr)/2 ;
	float tDiff = tR_corr - tL_corr;

	// -- tDiff vs position X
	h2_tDiff_ampCorr_vs_posX[iBar] ->Fill(posX, tDiff);
	h2_tDiff_vs_posX[iBar] ->Fill(posX, tR-tL);

	// -- SiPM left
	p_ampL_vs_tDiff[iBar] ->Fill(tDiff,ampL);
	p_tL_vs_tDiff[iBar] ->Fill(tDiff,tL);
	h_tL_ampCorr[iBar]->Fill(tL_corr);
        p_tL_ampCorr_vs_amp[iBar]->Fill(ampL,tL_corr);
        p_tL_ampCorr_vs_posX[iBar]->Fill(posX,tL_corr);
	if ( tL > h_tL[iBar]->GetMean()-3*h_tL[iBar]->GetRMS() && tL < h_tL[iBar]->GetMean()+3*h_tL[iBar]->GetRMS() ){
	  p_tL_ampCorr_vs_posXc[iBar]->Fill(posX/cos(theta[iBar]),tL_corr);
	  int xcbinamp = int ((posX - max(cut_Xmin[iBar], cut_XminRef))/cos(theta[iBar])/xbinWidthForAmpWalk );
          p_tL_ampCorr_vs_amp_binned[iBar][xcbinamp] ->Fill(ampL,tL_corr);
	}
        p_tL_ampCorr_vs_posY[iBar]->Fill(posY,tL_corr);
        p_tL_ampCorr_vs_tDiff[iBar]->Fill(tDiff, tL_corr);


	// -- SiPM right
	p_ampR_vs_tDiff[iBar] ->Fill(tDiff,ampR);
	p_tR_vs_tDiff[iBar] ->Fill(tDiff,tR);
	h_tR_ampCorr[iBar]->Fill(tR_corr);
        p_tR_ampCorr_vs_amp[iBar]->Fill(ampR,tR_corr);
        p_tR_ampCorr_vs_posX[iBar]->Fill(posX,tR_corr);
	if ( tR > h_tR[iBar]->GetMean()-3*h_tR[iBar]->GetRMS() && tR < h_tR[iBar]->GetMean()+3*h_tR[iBar]->GetRMS() ) {
	  p_tR_ampCorr_vs_posXc[iBar]->Fill(posX/cos(theta[iBar]),tR_corr);
	  int xcbinamp = int ((posX - max(cut_Xmin[iBar], cut_XminRef))/cos(theta[iBar])/xbinWidthForAmpWalk );
          p_tR_ampCorr_vs_amp_binned[iBar][xcbinamp] ->Fill(ampR,tR_corr);
	}
        p_tR_ampCorr_vs_posY[iBar]->Fill(posY,tR_corr);
        p_tR_ampCorr_vs_tDiff[iBar]->Fill(tDiff, tR_corr);

	// -- tDiff
	h_tDiff_ampCorr[iBar]->Fill(tDiff);
	p_tDiff_ampCorr_vs_posX[iBar]->Fill(posX,tDiff);
        p_tDiff_ampCorr_vs_posXc[iBar]->Fill(posX/cos(theta[iBar]),tDiff);

	// -- simple average
	p_tAve_vs_tDiff[iBar] ->Fill(tDiff, tAve);
	h_tAve_ampCorr[iBar]->Fill(tAve_ampCorr);
	if ( tR > h_tR[iBar]->GetMean()-3*h_tR[iBar]->GetRMS() && tR < h_tR[iBar]->GetMean()+3*h_tR[iBar]->GetRMS() &&
	     tL > h_tL[iBar]->GetMean()-3*h_tL[iBar]->GetRMS() && tL < h_tL[iBar]->GetMean()+3*h_tL[iBar]->GetRMS() ) {
	  p_tAve_ampCorr_vs_posX[iBar]->Fill(posX,tAve_ampCorr);
	}
        p_tAve_ampCorr_vs_posXc[iBar]->Fill(posX/cos(theta[iBar]),tAve_ampCorr);
        p_tAve_ampCorr_vs_posY[iBar]->Fill(posY,tAve_ampCorr);
        p_tAve_ampCorr_vs_tDiff[iBar]->Fill(tDiff, tAve_ampCorr);

	int xbin = int ((posX-cut_Xmin[iBar])/xbinWidth );
	int ybin = int ((posY-cut_Ymin[NBARS-1])/ybinWidth );

	h_tDiff_ampCorr_binX[iBar][xbin]->Fill(tDiff);
	h_ampDiff_binX[iBar][xbin]->Fill(ampL-ampR);
	h_tL_ampCorr_binX[iBar][xbin] ->Fill(tL_corr);
	h_tR_ampCorr_binX[iBar][xbin] ->Fill(tR_corr);
	h_tAve_ampCorr_binX[iBar][xbin] ->Fill(tAve_ampCorr);
	h_tAve_ampCorr_binY[iBar][ybin] ->Fill(tAve_ampCorr);

      } // -- end loop over bars

    }// -- end second loop over events

  
  

  // -- Third loop over events - correction for residual tAve dependence on tDiff or position
  TF1*  fitFuncAve_tDiffCorr[NBARS];
  float mean, rms;

  for (int iBar = 0 ; iBar < NBARS; iBar++){
    fitFuncAve_tDiffCorr[iBar] = new TF1(Form("fitFuncAve_tDiffCorr_%d", iBar),"pol4");
    mean = p_tAve_ampCorr_vs_tDiff[iBar] -> GetMean();
    rms  = p_tAve_ampCorr_vs_tDiff[iBar] -> GetRMS();
    fitFuncAve_tDiffCorr[iBar]->SetRange( mean - 3*rms , mean + 3*rms);
    fitFuncAve_tDiffCorr[iBar] -> SetLineColor(kRed);
    p_tAve_ampCorr_vs_tDiff[iBar] -> Fit(Form("fitFuncAve_tDiffCorr_%d",iBar),"QSR");
  }


  TF1*  fitFuncAve_posCorr[NBARS];
  for (int iBar = 0 ; iBar < NBARS; iBar++){
    fitFuncAve_posCorr[iBar] = new TF1(Form("fitFuncAve_posCorr_%d", iBar),"pol4");
    mean = p_tAve_ampCorr_vs_posX[iBar] -> GetMean();
    rms  = p_tAve_ampCorr_vs_posX[iBar] -> GetRMS();
    fitFuncAve_posCorr[iBar]->SetRange( mean - 5.0*rms , mean + 5.0*rms);
    fitFuncAve_posCorr[iBar] -> SetLineColor(kRed);
    p_tAve_ampCorr_vs_posX[iBar] -> Fit(Form("fitFuncAve_posCorr_%d",iBar),"QSR");
  }

  TF1*  fitFuncDiff_posCorr[NBARS];
  for (int iBar = 0 ; iBar < NBARS; iBar++){
    fitFuncDiff_posCorr[iBar] = new TF1(Form("fitFuncDiff_posCorr_%d", iBar),"pol4");
    mean = p_tDiff_ampCorr_vs_posX[iBar] -> GetMean();
    rms  = p_tDiff_ampCorr_vs_posX[iBar] -> GetRMS();
    fitFuncDiff_posCorr[iBar]->SetRange( mean - 5*rms , mean + 5*rms);
    fitFuncDiff_posCorr[iBar] -> SetLineColor(kRed);
    p_tDiff_ampCorr_vs_posX[iBar] -> Fit(Form("fitFuncDiff_posCorr_%d",iBar),"QSR");
  }



  std::cout<<" >>> Third loop over events - residual corrections vs tDiff or position ..." <<std::endl;
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
      brmsRef = treeVars.t_b_rms[(*treeVars.t_channelId)[ampChannelRef]];

      // -- loop over bars
      for (int iBar = 0; iBar < NBARS; iBar++){
	
        ampL = treeVars.t_amp[(*treeVars.t_channelId)[ ampChannelsL[iBar] ]] * kAdcToV ;
        ampR = treeVars.t_amp[(*treeVars.t_channelId)[ ampChannelsR[iBar] ]] * kAdcToV ;
	
        tL   = treeVars.t_time[(*treeVars.t_channelId)[ timeChannelsL[iBar] ]+treeVars.t_LED];
        tR   = treeVars.t_time[(*treeVars.t_channelId)[ timeChannelsR[iBar] ]+treeVars.t_LED];

	brmsL = treeVars.t_b_rms[(*treeVars.t_channelId)[ timeChannelsL[iBar] ] ];
	brmsR = treeVars.t_b_rms[(*treeVars.t_channelId)[ timeChannelsR[iBar] ] ];

        // -- select events
        if ( posX < cut_XminRef || posX > cut_XmaxRef ) continue;
        if ( posY < cut_YminRef || posY > cut_YmaxRef ) continue;
        if ( posX < cut_Xmin[iBar] || posX > cut_Xmax[iBar] ) continue;
        if ( posY < cut_Ymin[iBar] || posY > cut_Ymax[iBar] ) continue;
	
        if ( ampRef < cut_ampMinRef     ||  ampRef > cut_ampMaxRef ) continue;
        if ( ampL   < cut_ampMinL[iBar] ||  ampL   > cut_ampMaxL[iBar] ) continue;
        if ( ampR   < cut_ampMinR[iBar] ||  ampR   > cut_ampMaxR[iBar] ) continue;

        if ( tRef < 0  ||  tRef > 200) continue;
        if ( tL   < 0  ||  tL   > 200) continue;
        if ( tR   < 0  ||  tR   > 200) continue;

        if ( tRef < cut_minTimeRef || tRef > cut_maxTimeRef ) continue;
        if ( tL   < cut_minTime[iBar]  || tL   > cut_maxTime[iBar]  ) continue;
        if ( tR   < cut_minTime[iBar]  || tR   > cut_maxTime[iBar]  ) continue;

	if (tL < tLmin[iBar] || tL > tLmax[iBar]) continue;
	if (tR < tRmin[iBar] || tR > tRmax[iBar]) continue;

	// -- remove very noisy events
	if (brmsRef > cut_brmsMaxRef) continue;
        if (brmsR > cut_brmsMaxTimeCh[iBar]) continue;
        if (brmsL > cut_brmsMaxTimeCh[iBar]) continue;

        tL = tL - tRef;
        tR = tR - tRef;

        // -- amplitude walk correction
        //int xcbinamp = int ((posX - max(cut_Xmin[iBar], cut_XminRef))/cos(theta[iBar])/xbinWidthForAmpWalk );
        //float tL_corr = tL - fitFuncL_ampCorr_binned[iBar][xcbinamp]->Eval(ampL) + fitFuncL_ampCorr_binned[iBar][xcbinamp]->Eval(h_ampL[iBar]->GetMean());
        //float tR_corr = tR - fitFuncR_ampCorr_binned[iBar][xcbinamp]->Eval(ampR) + fitFuncR_ampCorr_binned[iBar][xcbinamp]->Eval(h_ampR[iBar]->GetMean());
        //float tL_corr = tL - fitFuncL_ampCorr_binned[iBar][xcbinamp]->Eval(ampL) ;
        //float tR_corr = tR - fitFuncR_ampCorr_binned[iBar][xcbinamp]->Eval(ampR) ;
        //float tL_corr = tL - fitFuncL_ampCorr[iBar]->Eval(ampL) + fitFuncL_ampCorr[iBar]->Eval(h_ampL[iBar]->GetMean()) ;
	//float tR_corr = tR - fitFuncR_ampCorr[iBar]->Eval(ampR) + fitFuncR_ampCorr[iBar]->Eval(h_ampR[iBar]->GetMean()) ;
	float tL_corr = tL - fitFuncL_ampCorr[iBar]->Eval(ampL) ;
	float tR_corr = tR - fitFuncR_ampCorr[iBar]->Eval(ampR) ;
        float tAve_ampCorr = ( tL_corr + tR_corr)/2 ;
        float tDiff = tR_corr - tL_corr;
 
	// -- correction for residual dependence on tDiff
	//float tAve_ampCorr_tDiffCorr = tAve_ampCorr - fitFuncAve_tDiffCorr[iBar]->Eval(tDiff) + fitFuncAve_tDiffCorr[iBar]->Eval( p_tAve_ampCorr_vs_tDiff[iBar]->GetMean() ) ;
	float tAve_ampCorr_tDiffCorr = tAve_ampCorr - fitFuncAve_tDiffCorr[iBar]->Eval(tDiff) ;

	// -- correction for residual dependence on posX
	//float tAve_ampCorr_posCorr  = tAve_ampCorr - fitFuncAve_posCorr[iBar]->Eval(posX) + fitFuncAve_posCorr[iBar]->Eval( p_tAve_ampCorr_vs_posX[iBar]->GetMean() ) ;
	//float tDiff_ampCorr_posCorr = tDiff - fitFuncDiff_posCorr[iBar]->Eval(posX) + fitFuncDiff_posCorr[iBar]->Eval( p_tDiff_ampCorr_vs_posX[iBar]->GetMean() ) ;
	float tAve_ampCorr_posCorr  = tAve_ampCorr - fitFuncAve_posCorr[iBar]->Eval(posX);
	float tDiff_ampCorr_posCorr = tDiff - fitFuncDiff_posCorr[iBar]->Eval(posX);


	int xbin = int ((posX-cut_Xmin[iBar])/xbinWidth );
	int ybin = int ((posY-cut_Ymin[NBARS-1])/ybinWidth );

	// -- simple average
        h_tAve_ampCorr_tDiffCorr[iBar]->Fill(tAve_ampCorr_tDiffCorr);
        p_tAve_ampCorr_tDiffCorr_vs_posX[iBar]->Fill(posX,tAve_ampCorr_tDiffCorr);
        p_tAve_ampCorr_tDiffCorr_vs_posXc[iBar]->Fill(posX/cos(theta[iBar]),tAve_ampCorr_tDiffCorr);
        p_tAve_ampCorr_tDiffCorr_vs_posY[iBar]->Fill(posY,tAve_ampCorr_tDiffCorr);
        p_tAve_ampCorr_tDiffCorr_vs_tDiff[iBar]->Fill(tDiff, tAve_ampCorr_tDiffCorr);
	h_tAve_ampCorr_tDiffCorr_binX[iBar][xbin] ->Fill(tAve_ampCorr_tDiffCorr);
        h_tAve_ampCorr_tDiffCorr_binY[iBar][ybin] ->Fill(tAve_ampCorr_tDiffCorr);

	// -- simple average vs posX
        h_tAve_ampCorr_posCorr[iBar]->Fill(tAve_ampCorr_posCorr);
        p_tAve_ampCorr_posCorr_vs_posX[iBar]->Fill(posX,tAve_ampCorr_posCorr);
        p_tAve_ampCorr_posCorr_vs_posXc[iBar]->Fill(posX/cos(theta[iBar]),tAve_ampCorr_posCorr);
        p_tAve_ampCorr_posCorr_vs_posY[iBar]->Fill(posY,tAve_ampCorr_posCorr);
        p_tAve_ampCorr_posCorr_vs_tDiff[iBar]->Fill(tDiff, tAve_ampCorr_posCorr);
	h_tAve_ampCorr_posCorr_binX[iBar][xbin] ->Fill(tAve_ampCorr_posCorr);

	// -- tDiff vs posX
        h_tDiff_ampCorr_posCorr[iBar]->Fill(tDiff_ampCorr_posCorr);
        p_tDiff_ampCorr_posCorr_vs_posX[iBar]->Fill(posX,tDiff_ampCorr_posCorr);
        p_tDiff_ampCorr_posCorr_vs_posXc[iBar]->Fill(posX/cos(theta[iBar]),tDiff_ampCorr_posCorr);
	h_tDiff_ampCorr_posCorr_binX[iBar][xbin] ->Fill(tDiff_ampCorr_posCorr);
      }
      
      
    } //--- end fourth loop over events


  cout << "Calculating time resolution for each bar... " <<endl;
  // -- Compute resolutions (gaussian and effective sigma)
  TF1*  fitFunL_ampCorr[NBARS];
  TF1*  fitFunR_ampCorr[NBARS];
  
  TF1*  fitFunAve_ampCorr[NBARS];
  TF1*  fitFunAve_ampCorr_tDiffCorr[NBARS];
  TF1*  fitFunAve_ampCorr_posCorr[NBARS];
  
  TF1*  fitFunDiff_ampCorr[NBARS];
  TF1*  fitFunDiff_ampCorr_posCorr[NBARS];
  
  TF1*  fitFunL_ampCorr_binX[NBARS][1000];
  TF1*  fitFunR_ampCorr_binX[NBARS][1000];
  
  TF1*  fitFunAve_ampCorr_binX[NBARS][1000];
  TF1*  fitFunAve_ampCorr_tDiffCorr_binX[NBARS][1000];
  TF1*  fitFunAve_ampCorr_posCorr_binX[NBARS][1000];
  
  TF1*  fitFunAve_ampCorr_binY[NBARS][1000];
  TF1*  fitFunAve_ampCorr_tDiffCorr_binY[NBARS][1000];
  TF1*  fitFunAve_ampCorr_posCorr_binY[NBARS][1000];
  
  TF1*  fitFunDiff_ampCorr_binX[NBARS][1000];
  TF1*  fitFunDiff_ampCorr_posCorr_binX[NBARS][1000];
  
  TH1F*  h_effectiveSigmaL[NBARS];
  TH1F*  h_effectiveSigmaL_ampCorr[NBARS];
  TH1F*  h_effectiveSigmaR[NBARS];
  TH1F*  h_effectiveSigmaR_ampCorr[NBARS];
  TH1F*  h_effectiveSigmaAve[NBARS];
  TH1F*  h_effectiveSigmaAve_ampCorr[NBARS];
  TH1F*  h_effectiveSigmaAve_ampCorr_tDiffCorr[NBARS];
  TH1F*  h_effectiveSigmaAve_ampCorr_posCorr[NBARS];
  TH1F*  h_effectiveSigmaDiff_ampCorr[NBARS];
  TH1F*  h_effectiveSigmaDiff_ampCorr_posCorr[NBARS];
  
  
  TGraphErrors *g_tResolGausL_ampCorr[NBARS];
  TGraphErrors *g_tResolGausR_ampCorr[NBARS];
  TGraphErrors *g_tResolGausAve_ampCorr[NBARS];
  TGraphErrors *g_tResolGausAve_ampCorr_tDiffCorr[NBARS];
  TGraphErrors *g_tResolGausAve_ampCorr_posCorr[NBARS];
  TGraphErrors *g_tResolGausDiff_ampCorr[NBARS];
  TGraphErrors *g_tResolGausDiff_ampCorr_posCorr[NBARS];
  TGraphErrors *g_tResolGausAve_ampCorr_vs_posY[NBARS];
  TGraphErrors *g_tResolGausAve_ampCorr_tDiffCorr_vs_posY[NBARS];
  
  float* vals = new float[4];
  float sigmaEff;
  float sigmaGaus;   
  float sigmaGausErr;
  float resolEff;
  float resolGaus;
  float resolGausErr;
  //float resolMCP = 0.;
  //float resolMCP = 0.014; 
  float resolMCP = 0.030; // estimated comparing tAve and tDiff in this run range 
  int nMinEntries = 50;
  
  TF1 *fpol0_tL_ampCorr[NBARS]; 
  TF1 *fpol0_tR_ampCorr[NBARS]; 
  
  TF1 *fpol0_tAve_ampCorr[NBARS]; 
  TF1 *fpol0_tAve_ampCorr_tDiffCorr[NBARS]; 
  TF1 *fpol0_tAve_ampCorr_posCorr[NBARS]; 
  
  TF1 *fpol0_tDiff_ampCorr[NBARS]; 
  TF1 *fpol0_tDiff_ampCorr_posCorr[NBARS]; 
  
  for (int iBar = 0; iBar < NBARS; iBar++){
    h_effectiveSigmaL[iBar]  = new TH1F(Form("h_effectiveSigmaL_BAR%d",iBar),Form("h_effectiveSigmaL_BAR%d",iBar),2000,0.,200.);
    h_effectiveSigmaL_ampCorr[iBar]  = new TH1F(Form("h_effectiveSigmaL_ampCorr_BAR%d",iBar),Form("h_effectiveSigmaL_ampCorr_BAR%d",iBar), 2000,0.,200.);
    
    h_effectiveSigmaR[iBar]  = new TH1F(Form("h_effectiveSigmaR_BAR%d",iBar),Form("h_effectiveSigmaR_BAR%d",iBar),2000,0.,200.);
    h_effectiveSigmaR_ampCorr[iBar]  = new TH1F(Form("h_effectiveSigmaR_ampCorr_BAR%d",iBar),Form("h_effectiveSigmaR_ampCorr_BAR%d",iBar), 2000,0.,200.);
    
    h_effectiveSigmaAve[iBar]  = new TH1F(Form("h_effectiveSigmaAve_BAR%d",iBar),Form("h_effectiveSigmaAve_BAR%d",iBar),2000,0.,200.);
    
    h_effectiveSigmaAve_ampCorr[iBar]  = new TH1F(Form("h_effectiveSigmaAve_ampCorr_BAR%d",iBar),Form("h_effectiveSigmaAve_ampCorr_BAR%d",iBar), 2000,0.,200.);
    h_effectiveSigmaAve_ampCorr_tDiffCorr[iBar]  = new TH1F(Form("h_effectiveSigmaAve_ampCorr_tDiffCorr_BAR%d",iBar),Form("h_effectiveSigmaAve_ampCorr_tDiffCorr_BAR%d",iBar), 2000,0.,200.);
    h_effectiveSigmaAve_ampCorr_posCorr[iBar]  = new TH1F(Form("h_effectiveSigmaAve_ampCorr_posCorr_BAR%d",iBar),Form("h_effectiveSigmaAve_ampCorr_posCorr_BAR%d",iBar), 2000,0.,200.);
    
    h_effectiveSigmaDiff_ampCorr[iBar]  = new TH1F(Form("h_effectiveSigmaDiff_ampCorr_BAR%d",iBar),Form("h_effectiveSigmaDiff_ampCorr_BAR%d",iBar), 2000,0.,200.);
    h_effectiveSigmaDiff_ampCorr_posCorr[iBar]  = new TH1F(Form("h_effectiveSigmaDiff_ampCorr_posCorr_BAR%d",iBar),Form("h_effectiveSigmaDiff_ampCorr_posCorr_BAR%d",iBar), 2000,0.,200.);
    
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
    fitFunL_ampCorr[iBar]= new TF1(Form("fitFunL_ampCorr_BAR%d",iBar),"gaus", -20, 20);
    GetTimeResolution(h_tL_ampCorr[iBar], sigmaEff, sigmaGaus, fitFunL_ampCorr[iBar]);
    h_effectiveSigmaL_ampCorr[iBar]->Fill(sigmaEff);
    
    // -- right
    fitFunR_ampCorr[iBar]= new TF1(Form("fitFunR_ampCorr_BAR%d",iBar),"gaus", -20, 20);
    GetTimeResolution(h_tR_ampCorr[iBar], sigmaEff, sigmaGaus, fitFunR_ampCorr[iBar]);
    h_effectiveSigmaR_ampCorr[iBar]->Fill(sigmaEff);
    
    // -- simple average
    fitFunAve_ampCorr[iBar]= new TF1(Form("fitFunAve_ampCorr_BAR%d",iBar),"gaus", -20, 20);
    GetTimeResolution(h_tAve_ampCorr[iBar], sigmaEff, sigmaGaus, fitFunAve_ampCorr[iBar]);
    h_effectiveSigmaAve_ampCorr[iBar]->Fill(sigmaEff);
    
    // ----- tDiff corr
    // -- simple average
    fitFunAve_ampCorr_tDiffCorr[iBar]= new TF1(Form("fitFunAve_ampCorr_tDiffCorr_BAR%d",iBar),"gaus", -20, 20);
    GetTimeResolution(h_tAve_ampCorr_tDiffCorr[iBar], sigmaEff, sigmaGaus, fitFunAve_ampCorr_tDiffCorr[iBar]);
    h_effectiveSigmaAve_ampCorr_tDiffCorr[iBar]->Fill(sigmaEff);
    
    // ----- position corr
    // -- simple average
    fitFunAve_ampCorr_posCorr[iBar]= new TF1(Form("fitFunAve_ampCorr_posCorr_BAR%d",iBar),"gaus", -20, 20);
    GetTimeResolution(h_tAve_ampCorr_posCorr[iBar], sigmaEff, sigmaGaus, fitFunAve_ampCorr_posCorr[iBar]);
    h_effectiveSigmaAve_ampCorr_posCorr[iBar]->Fill(sigmaEff);
    
    // --- tDiff resolution
    fitFunDiff_ampCorr[iBar]= new TF1(Form("fitFunDiff_ampCorr_BAR%d",iBar),"gaus", -20, 20);
    GetTimeResolution(h_tDiff_ampCorr[iBar], sigmaEff, sigmaGaus, fitFunDiff_ampCorr[iBar]);
    h_effectiveSigmaDiff_ampCorr[iBar]->Fill(sigmaEff);
    
    fitFunDiff_ampCorr_posCorr[iBar]= new TF1(Form("fitFunDiff_ampCorr_posCorr_BAR%d",iBar),"gaus", -20, 20);
    GetTimeResolution(h_tDiff_ampCorr_posCorr[iBar], sigmaEff, sigmaGaus, fitFunDiff_ampCorr_posCorr[iBar]);
    h_effectiveSigmaDiff_ampCorr_posCorr[iBar]->Fill(sigmaEff);
    
    
    // tResol graphs in bins in x
    g_tResolGausL_ampCorr[iBar] = new TGraphErrors();
    g_tResolGausL_ampCorr[iBar]->SetName(Form("g_tResolGausL_ampCorr_BAR%d",iBar));
    
    g_tResolGausR_ampCorr[iBar] = new TGraphErrors();
    g_tResolGausR_ampCorr[iBar]->SetName(Form("g_tResolGausR_ampCorr_BAR%d",iBar));
    
    g_tResolGausAve_ampCorr[iBar] = new TGraphErrors();
    g_tResolGausAve_ampCorr[iBar]->SetName(Form("g_tResolGausAve_ampCorr_BAR%d",iBar));
    
    g_tResolGausAve_ampCorr_tDiffCorr[iBar] = new TGraphErrors();
    g_tResolGausAve_ampCorr_tDiffCorr[iBar]->SetName(Form("g_tResolGausAve_ampCorr_tDiffCorr_BAR%d",iBar));
    
    g_tResolGausAve_ampCorr_posCorr[iBar] = new TGraphErrors();
    g_tResolGausAve_ampCorr_posCorr[iBar]->SetName(Form("g_tResolGausAve_ampCorr_posCorr_BAR%d",iBar));
    
    g_tResolGausDiff_ampCorr[iBar] = new TGraphErrors();
    g_tResolGausDiff_ampCorr[iBar]->SetName(Form("g_tResolGausDiff_ampCorr_BAR%d",iBar));
    
    g_tResolGausDiff_ampCorr_posCorr[iBar] = new TGraphErrors();
    g_tResolGausDiff_ampCorr_posCorr[iBar]->SetName(Form("g_tResolGausDiff_ampCorr_posCorr_BAR%d",iBar));
    
    g_tResolGausAve_ampCorr_vs_posY[iBar] = new TGraphErrors();
    g_tResolGausAve_ampCorr_vs_posY[iBar]->SetName(Form("g_tResolGausAve_ampCorr_vs_posY_BAR%d",iBar));
    
    g_tResolGausAve_ampCorr_tDiffCorr_vs_posY[iBar] = new TGraphErrors();
    g_tResolGausAve_ampCorr_tDiffCorr_vs_posY[iBar]->SetName(Form("g_tResolGausAve_ampCorr_tDiffCorr_vs_posY_BAR%d",iBar));
    
    // --- vs X
    for (int ibin = 0; ibin < NBINSX[iBar]; ibin++){
      
      float xx = cut_Xmin[iBar] + xbinWidth*ibin + xbinWidth/2;
      
      // ----- amp walk corr
      
      // -- Left
      fitFunL_ampCorr_binX[iBar][ibin]= new TF1(Form("fitFunL_ampCorr_binX%d_BAR%d",ibin,iBar),"gaus", -20, 20); 
      GetTimeResolution(h_tL_ampCorr_binX[iBar][ibin], sigmaEff, sigmaGaus, fitFunL_ampCorr_binX[iBar][ibin]);
      
      sigmaGaus    = fitFunL_ampCorr_binX[iBar][ibin]->GetParameter(2);
      sigmaGausErr = fitFunL_ampCorr_binX[iBar][ibin]->GetParError(2);
      if ( h_tL_ampCorr_binX[iBar][ibin]->GetEntries() < nMinEntries){
	sigmaGaus    = h_tL_ampCorr_binX[iBar][ibin]->GetRMS();
	sigmaGausErr = h_tL_ampCorr_binX[iBar][ibin]->GetRMSError();
      }
      resolEff  = sqrt(sigmaEff  * sigmaEff  - resolMCP * resolMCP);
      resolGaus = sqrt(sigmaGaus * sigmaGaus - resolMCP * resolMCP); 
      resolGausErr = sigmaGausErr/resolGaus * sigmaGaus;

      g_tResolGausL_ampCorr[iBar]->SetPoint(ibin, xx, resolGaus*1000.);
      g_tResolGausL_ampCorr[iBar]->SetPointError(ibin, xbinWidth/2., resolGausErr*1000.);

      // -- Right
      fitFunR_ampCorr_binX[iBar][ibin]= new TF1(Form("fitFunR_ampCorr_binX%d_BAR%d",ibin,iBar),"gaus", -20, 20); 
      GetTimeResolution(h_tR_ampCorr_binX[iBar][ibin], sigmaEff, sigmaGaus, fitFunR_ampCorr_binX[iBar][ibin]);

      sigmaGaus    = fitFunR_ampCorr_binX[iBar][ibin]->GetParameter(2);
      sigmaGausErr = fitFunR_ampCorr_binX[iBar][ibin]->GetParError(2);
      if ( h_tR_ampCorr_binX[iBar][ibin]->GetEntries() < nMinEntries){
	sigmaGaus    = h_tR_ampCorr_binX[iBar][ibin]->GetRMS();
	sigmaGausErr = h_tR_ampCorr_binX[iBar][ibin]->GetRMSError();
      }
      resolEff  = sqrt(sigmaEff*sigmaEff - resolMCP*resolMCP);
      resolGaus = sqrt(sigmaGaus*sigmaGaus - resolMCP*resolMCP);
      resolGausErr = sigmaGausErr/resolGaus * sigmaGaus;

      g_tResolGausR_ampCorr[iBar]->SetPoint(ibin, xx, resolGaus*1000.);
      g_tResolGausR_ampCorr[iBar]->SetPointError(ibin, xbinWidth/2., resolGausErr*1000.);

      // -- simple average
      fitFunAve_ampCorr_binX[iBar][ibin]= new TF1(Form("fitFunAve_ampCorr_binX%d_BAR%d",ibin,iBar),"gaus", -20, 20); 
      GetTimeResolution(h_tAve_ampCorr_binX[iBar][ibin], sigmaEff, sigmaGaus, fitFunAve_ampCorr_binX[iBar][ibin]);

      sigmaGaus    = fitFunAve_ampCorr_binX[iBar][ibin]->GetParameter(2);
      sigmaGausErr = fitFunAve_ampCorr_binX[iBar][ibin]->GetParError(2);
      if ( h_tAve_ampCorr_binX[iBar][ibin]->GetEntries() < nMinEntries){
        sigmaGaus    = h_tAve_ampCorr_binX[iBar][ibin]->GetRMS();
        sigmaGausErr = h_tAve_ampCorr_binX[iBar][ibin]->GetRMSError();
      }
      resolEff  = sqrt(sigmaEff*sigmaEff - resolMCP*resolMCP);
      resolGaus = sqrt(sigmaGaus*sigmaGaus - resolMCP*resolMCP);
      resolGausErr = sigmaGausErr/resolGaus * sigmaGaus;

      g_tResolGausAve_ampCorr[iBar]->SetPoint(ibin, xx, resolGaus*1000.);
      g_tResolGausAve_ampCorr[iBar]->SetPointError(ibin, xbinWidth/2., resolGausErr*1000.);


      // -- tDiff resolution
      fitFunDiff_ampCorr_binX[iBar][ibin]= new TF1(Form("fitFunDiff_ampCorr_binX%d_BAR%d",ibin,iBar),"gaus", -20, 20);
      GetTimeResolution(h_tDiff_ampCorr_binX[iBar][ibin], sigmaEff, sigmaGaus, fitFunDiff_ampCorr_binX[iBar][ibin]);

      sigmaGaus    = fitFunDiff_ampCorr_binX[iBar][ibin]->GetParameter(2);
      sigmaGausErr = fitFunDiff_ampCorr_binX[iBar][ibin]->GetParError(2);
      if ( h_tDiff_ampCorr_binX[iBar][ibin]->GetEntries() < nMinEntries){
        sigmaGaus    = h_tDiff_ampCorr_binX[iBar][ibin]->GetRMS();
        sigmaGausErr = h_tDiff_ampCorr_binX[iBar][ibin]->GetRMSError();
      }
      resolEff  = sqrt(sigmaEff*sigmaEff);
      resolGaus = sqrt(sigmaGaus*sigmaGaus);
      resolGausErr = sigmaGausErr/resolGaus * sigmaGaus;

      g_tResolGausDiff_ampCorr[iBar]->SetPoint(ibin, xx, resolGaus*1000.);
      g_tResolGausDiff_ampCorr[iBar]->SetPointError(ibin, xbinWidth/2., resolGausErr*1000.);

      

      // ----- tDiff cor

      // -- simple average
      fitFunAve_ampCorr_tDiffCorr_binX[iBar][ibin]= new TF1(Form("fitFunAve_ampCorr_tDiffCorr_binX%d_BAR%d",ibin,iBar),"gaus", -20, 20);
      GetTimeResolution(h_tAve_ampCorr_tDiffCorr_binX[iBar][ibin], sigmaEff, sigmaGaus, fitFunAve_ampCorr_tDiffCorr_binX[iBar][ibin]);

      sigmaGaus    = fitFunAve_ampCorr_tDiffCorr_binX[iBar][ibin]->GetParameter(2);
      sigmaGausErr = fitFunAve_ampCorr_tDiffCorr_binX[iBar][ibin]->GetParError(2);
      if ( h_tAve_ampCorr_tDiffCorr_binX[iBar][ibin]->GetEntries() < nMinEntries){
        sigmaGaus    = h_tAve_ampCorr_tDiffCorr_binX[iBar][ibin]->GetRMS();
        sigmaGausErr = h_tAve_ampCorr_tDiffCorr_binX[iBar][ibin]->GetRMSError();
      }
      resolEff  = sqrt(sigmaEff*sigmaEff - resolMCP*resolMCP);
      resolGaus = sqrt(sigmaGaus*sigmaGaus - resolMCP*resolMCP);
      resolGausErr = sigmaGausErr/resolGaus * sigmaGaus;

      g_tResolGausAve_ampCorr_tDiffCorr[iBar]->SetPoint(ibin, xx, resolGaus*1000.);
      g_tResolGausAve_ampCorr_tDiffCorr[iBar]->SetPointError(ibin, xbinWidth/2., resolGausErr*1000.);


      //-- ampCorr + pos corr 
      fitFunAve_ampCorr_posCorr_binX[iBar][ibin]= new TF1(Form("fitFunAve_ampCorr_posCorr_binX%d_BAR%d",ibin,iBar),"gaus", -20, 20);
      GetTimeResolution(h_tAve_ampCorr_posCorr_binX[iBar][ibin], sigmaEff, sigmaGaus, fitFunAve_ampCorr_posCorr_binX[iBar][ibin]);

      sigmaGaus    = fitFunAve_ampCorr_posCorr_binX[iBar][ibin]->GetParameter(2);
      sigmaGausErr = fitFunAve_ampCorr_posCorr_binX[iBar][ibin]->GetParError(2);
      if ( h_tAve_ampCorr_posCorr_binX[iBar][ibin]->GetEntries() < nMinEntries){
        sigmaGaus    = h_tAve_ampCorr_posCorr_binX[iBar][ibin]->GetRMS();
        sigmaGausErr = h_tAve_ampCorr_posCorr_binX[iBar][ibin]->GetRMSError();
      }
      resolEff  = sqrt(sigmaEff*sigmaEff - resolMCP*resolMCP);
      resolGaus = sqrt(sigmaGaus*sigmaGaus - resolMCP*resolMCP);
      resolGausErr = sigmaGausErr/resolGaus * sigmaGaus;

      g_tResolGausAve_ampCorr_posCorr[iBar]->SetPoint(ibin, xx, resolGaus*1000.);
      g_tResolGausAve_ampCorr_posCorr[iBar]->SetPointError(ibin, xbinWidth/2., resolGausErr*1000.);


      // -- tDiff resolution (ampCorr+ posCorr)
      fitFunDiff_ampCorr_posCorr_binX[iBar][ibin]= new TF1(Form("fitFunDiff_ampCorr_posCorr_binX%d_BAR%d",ibin,iBar),"gaus", -20, 20);
      GetTimeResolution(h_tDiff_ampCorr_posCorr_binX[iBar][ibin], sigmaEff, sigmaGaus, fitFunDiff_ampCorr_posCorr_binX[iBar][ibin]);

      sigmaGaus    = fitFunDiff_ampCorr_posCorr_binX[iBar][ibin]->GetParameter(2);
      sigmaGausErr = fitFunDiff_ampCorr_posCorr_binX[iBar][ibin]->GetParError(2);
      if ( h_tDiff_ampCorr_posCorr_binX[iBar][ibin]->GetEntries() < nMinEntries){
        sigmaGaus    = h_tDiff_ampCorr_posCorr_binX[iBar][ibin]->GetRMS();
        sigmaGausErr = h_tDiff_ampCorr_posCorr_binX[iBar][ibin]->GetRMSError();
      }
      resolEff  = sqrt(sigmaEff*sigmaEff);
      resolGaus = sqrt(sigmaGaus*sigmaGaus);
      resolGausErr = sigmaGausErr/resolGaus * sigmaGaus;

      g_tResolGausDiff_ampCorr_posCorr[iBar]->SetPoint(ibin, xx, resolGaus*1000.);
      g_tResolGausDiff_ampCorr_posCorr[iBar]->SetPointError(ibin, xbinWidth/2., resolGausErr*1000.);
    }// --  end vs X
    
    
    // --- vs Y 
    int np = 0;
    for (int ibin = 0; ibin < NBINSY; ibin++){
      
      float yy = cut_Ymin[NBARS-1] + ybinWidth*ibin + ybinWidth/2;
      
      if (yy < cut_Ymin[iBar]  || yy > cut_Ymax[iBar]) continue;

      // -- simple average
      fitFunAve_ampCorr_binY[iBar][ibin]= new TF1(Form("fitFunAve_ampCorr_binY%d_BAR%d",ibin,iBar),"gaus", -20, 20);
      GetTimeResolution(h_tAve_ampCorr_binY[iBar][ibin], sigmaEff, sigmaGaus, fitFunAve_ampCorr_binY[iBar][ibin]);

      sigmaGaus    = fitFunAve_ampCorr_binY[iBar][ibin]->GetParameter(2);
      sigmaGausErr = fitFunAve_ampCorr_binY[iBar][ibin]->GetParError(2);
      resolEff  = sqrt(sigmaEff*sigmaEff - resolMCP*resolMCP);
      resolGaus = sqrt(sigmaGaus*sigmaGaus - resolMCP*resolMCP);
      resolGausErr = sigmaGausErr/resolGaus * sigmaGaus;

      if ( !isnan(resolGaus) ) {
        g_tResolGausAve_ampCorr_vs_posY[iBar]->SetPoint(np, yy, resolGaus*1000.);
        g_tResolGausAve_ampCorr_vs_posY[iBar]->SetPointError(np, ybinWidth/2., resolGausErr*1000.);
      }

      // -- simple average   - tDiffCorr
      fitFunAve_ampCorr_tDiffCorr_binY[iBar][ibin]= new TF1(Form("fitFunAve_ampCorr_tDiffCorr_binY%d_BAR%d",ibin,iBar),"gaus", -20, 20);
      GetTimeResolution(h_tAve_ampCorr_tDiffCorr_binY[iBar][ibin], sigmaEff, sigmaGaus, fitFunAve_ampCorr_tDiffCorr_binY[iBar][ibin]);

      sigmaGaus    = fitFunAve_ampCorr_tDiffCorr_binY[iBar][ibin]->GetParameter(2);
      sigmaGausErr = fitFunAve_ampCorr_tDiffCorr_binY[iBar][ibin]->GetParError(2);
      resolEff  = sqrt(sigmaEff*sigmaEff - resolMCP*resolMCP);
      resolGaus = sqrt(sigmaGaus*sigmaGaus - resolMCP*resolMCP);
      resolGausErr = sigmaGausErr/resolGaus * sigmaGaus;

      if ( !isnan(resolGaus) ) {
        g_tResolGausAve_ampCorr_tDiffCorr_vs_posY[iBar]->SetPoint(np, yy, resolGaus*1000.);
        g_tResolGausAve_ampCorr_tDiffCorr_vs_posY[iBar]->SetPointError(np, ybinWidth/2., resolGausErr*1000.);
      }

      np++;
      
    } // -- end vs Y



    /// -- fit with pol0 resol vs X 

    // -- tLeft
    fpol0_tL_ampCorr[iBar] = new TF1(Form("fpol0_tL_ampCorr_BAR%d",iBar),"pol0",-50,50);
    fpol0_tL_ampCorr[iBar]->SetLineStyle(2);
    fpol0_tL_ampCorr[iBar]->SetLineWidth(2);    
    fpol0_tL_ampCorr[iBar]->SetLineColor(4);
    fpol0_tL_ampCorr[iBar]->SetParameter(0, g_tResolGausL_ampCorr[iBar]->GetMean(2));
    g_tResolGausL_ampCorr[iBar]->Fit(fpol0_tL_ampCorr[iBar],"QRS");

    // -- tRight
    fpol0_tR_ampCorr[iBar] = new TF1(Form("fpol0_tR_ampCorr_BAR%d",iBar),"pol0",-50,50);
    fpol0_tR_ampCorr[iBar]->SetLineStyle(2);
    fpol0_tR_ampCorr[iBar]->SetLineWidth(2);    
    fpol0_tR_ampCorr[iBar]->SetLineColor(2);
    fpol0_tR_ampCorr[iBar]->SetParameter(0, g_tResolGausR_ampCorr[iBar]->GetMean(2));
    g_tResolGausR_ampCorr[iBar]->Fit(fpol0_tR_ampCorr[iBar],"QRS");

    // -- tAve
    fpol0_tAve_ampCorr[iBar] = new TF1(Form("fpol0_tAve_ampCorr_BAR%d",iBar),"pol0",-50,50);
    fpol0_tAve_ampCorr[iBar]->SetLineStyle(2);
    fpol0_tAve_ampCorr[iBar]->SetLineWidth(2);    
    fpol0_tAve_ampCorr[iBar]->SetLineColor(1);
    fpol0_tAve_ampCorr[iBar]->SetParameter(0, g_tResolGausAve_ampCorr[iBar]->GetMean(2));
    g_tResolGausAve_ampCorr[iBar]->Fit(fpol0_tAve_ampCorr[iBar],"QRS");

    fpol0_tAve_ampCorr_tDiffCorr[iBar] = new TF1(Form("fpol0_tAve_ampCorr_tDiffCorr_BAR%d",iBar),"pol0",-50,50);
    fpol0_tAve_ampCorr_tDiffCorr[iBar]->SetLineStyle(2);
    fpol0_tAve_ampCorr_tDiffCorr[iBar]->SetLineWidth(2);    
    fpol0_tAve_ampCorr_tDiffCorr[iBar]->SetLineColor(1);
    fpol0_tAve_ampCorr_tDiffCorr[iBar]->SetParameter(0, g_tResolGausAve_ampCorr_tDiffCorr[iBar]->GetMean(2));
    g_tResolGausAve_ampCorr_tDiffCorr[iBar]->Fit(fpol0_tAve_ampCorr_tDiffCorr[iBar],"QRS");

    fpol0_tAve_ampCorr_posCorr[iBar] = new TF1(Form("fpol0_tAve_ampCorr_posCorr_BAR%d",iBar),"pol0",-50,50);
    fpol0_tAve_ampCorr_posCorr[iBar]->SetLineStyle(2);
    fpol0_tAve_ampCorr_posCorr[iBar]->SetLineWidth(2);    
    fpol0_tAve_ampCorr_posCorr[iBar]->SetLineColor(1);
    g_tResolGausAve_ampCorr_posCorr[iBar]->Fit(fpol0_tAve_ampCorr_posCorr[iBar],"QRS");

    // -- tDiff
    fpol0_tDiff_ampCorr[iBar] = new TF1(Form("fpol0_tDiff_ampCorr_BAR%d",iBar),"pol0",-50,50);
    fpol0_tDiff_ampCorr[iBar]->SetLineStyle(2);
    fpol0_tDiff_ampCorr[iBar]->SetLineWidth(2);
    fpol0_tDiff_ampCorr[iBar]->SetLineColor(1);
    g_tResolGausDiff_ampCorr[iBar]->Fit(fpol0_tDiff_ampCorr[iBar],"QRS");

    fpol0_tDiff_ampCorr_posCorr[iBar] = new TF1(Form("fpol0_tDiff_ampCorr_posCorr_BAR%d",iBar),"pol0",-50,50);
    fpol0_tDiff_ampCorr_posCorr[iBar]->SetLineStyle(2);
    fpol0_tDiff_ampCorr_posCorr[iBar]->SetLineWidth(2);
    fpol0_tDiff_ampCorr_posCorr[iBar]->SetLineColor(1);
    g_tResolGausDiff_ampCorr_posCorr[iBar]->Fit(fpol0_tDiff_ampCorr_posCorr[iBar],"QRS");

  }//

 

  // **** Now do the last loop on events to combine bars **** 

  // -- parametrization of the single bar time resolution vs Y
  TF1 *f_tRes[NBARS];
  for (int iBar = 0; iBar < NBARS; iBar++){
    f_tRes[iBar] = new TF1(Form("f_tRes_BAR%d",iBar),"pol6",cut_Ymin[iBar],cut_Ymax[iBar]);
    g_tResolGausAve_ampCorr_tDiffCorr_vs_posY[iBar]->Fit(f_tRes[iBar],"QRN");
  }
    

  std::cout<<" >>> Fourth loop over events - bars combination ..." <<std::endl;
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
      brmsRef = treeVars.t_b_rms[(*treeVars.t_channelId)[ampChannelRef]];

      float w[NBARS] = {0., 0., 0.};
      float wr[NBARS] = {0., 0., 0.};
      float t_ampCorr[NBARS] = {-999., -999., -999.};
      float t_ampCorr_tDiffCorr[NBARS] = {-999., -999., -999.};

      // -- loop over bars
      for (int iBar = 0; iBar < NBARS; iBar++){
	
        ampL = treeVars.t_amp[(*treeVars.t_channelId)[ ampChannelsL[iBar] ]] * kAdcToV ;
        ampR = treeVars.t_amp[(*treeVars.t_channelId)[ ampChannelsR[iBar] ]] * kAdcToV ;
	
        tL   = treeVars.t_time[(*treeVars.t_channelId)[ timeChannelsL[iBar] ]+treeVars.t_LED];
        tR   = treeVars.t_time[(*treeVars.t_channelId)[ timeChannelsR[iBar] ]+treeVars.t_LED];

	brmsL = treeVars.t_b_rms[(*treeVars.t_channelId)[ timeChannelsL[iBar] ] ];
	brmsR = treeVars.t_b_rms[(*treeVars.t_channelId)[ timeChannelsR[iBar] ] ];

        // -- select events
        if ( posX < cut_XminRef || posX > cut_XmaxRef ) continue;
        if ( posY < cut_YminRef || posY > cut_YmaxRef ) continue;
        if ( posX < cut_Xmin[iBar] || posX > cut_Xmax[iBar] ) continue;
        if ( posY < cut_Ymin[iBar] || posY > cut_Ymax[iBar] ) continue;
	
        if ( ampRef < cut_ampMinRef     ||  ampRef > cut_ampMaxRef ) continue;
        if ( ampL   < cut_ampMinL[iBar] ||  ampL   > cut_ampMaxL[iBar] ) continue;
        if ( ampR   < cut_ampMinR[iBar] ||  ampR   > cut_ampMaxR[iBar] ) continue;

        if ( tRef < 0  ||  tRef > 200) continue;
        if ( tL   < 0  ||  tL   > 200) continue;
        if ( tR   < 0  ||  tR   > 200) continue;

        if ( tRef < cut_minTimeRef || tRef > cut_maxTimeRef ) continue;
        if ( tL   < cut_minTime[iBar]  || tL   > cut_maxTime[iBar]  ) continue;
        if ( tR   < cut_minTime[iBar]  || tR   > cut_maxTime[iBar]  ) continue;

	if (tL < tLmin[iBar] || tL > tLmax[iBar]) continue;
	if (tR < tRmin[iBar] || tR > tRmax[iBar]) continue;

	// -- remove very noisy events
	if (brmsRef > cut_brmsMaxRef) continue;
        if (brmsR > cut_brmsMaxTimeCh[iBar]) continue;
        if (brmsL > cut_brmsMaxTimeCh[iBar]) continue;

        tL = tL - tRef;
        tR = tR - tRef;

        // -- amplitude walk correction 
	//int xcbinamp = int ((posX - max(cut_Xmin[iBar], cut_XminRef))/cos(theta[iBar])/xbinWidthForAmpWalk );
        //float tL_corr = tL - fitFuncL_ampCorr_binned[iBar][xcbinamp]->Eval(ampL) + fitFuncL_ampCorr_binned[iBar][xcbinamp]->Eval(h_ampL[iBar]->GetMean());
        //float tR_corr = tR - fitFuncR_ampCorr_binned[iBar][xcbinamp]->Eval(ampR) + fitFuncR_ampCorr_binned[iBar][xcbinamp]->Eval(h_ampR[iBar]->GetMean());
        //float tL_corr = tL - fitFuncL_ampCorr[iBar]->Eval(ampL) + fitFuncL_ampCorr[iBar]->Eval(h_ampL[iBar]->GetMean()) ;
	//float tR_corr = tR - fitFuncR_ampCorr[iBar]->Eval(ampR) + fitFuncR_ampCorr[iBar]->Eval(h_ampR[iBar]->GetMean()) ;
	float tL_corr = tL - fitFuncL_ampCorr[iBar]->Eval(ampL) ;
	float tR_corr = tR - fitFuncR_ampCorr[iBar]->Eval(ampR) ;
        float tAve_ampCorr = ( tL_corr + tR_corr)/2 ;
        float tDiff = tR_corr - tL_corr;
 
	// -- correction for residual dependence on tDiff
	//float tAve_ampCorr_tDiffCorr = tAve_ampCorr - fitFuncAve_tDiffCorr[iBar]->Eval(tDiff) + fitFuncAve_tDiffCorr[iBar]->Eval( p_tAve_ampCorr_vs_tDiff[iBar]->GetMean() ) ;
	float tAve_ampCorr_tDiffCorr = tAve_ampCorr - fitFuncAve_tDiffCorr[iBar]->Eval(tDiff) ;

	// weights and times to combine bars
        w[iBar] =  0.5*(ampL+ampR);
        t_ampCorr[iBar] = tAve_ampCorr;
        t_ampCorr_tDiffCorr[iBar] = tAve_ampCorr_tDiffCorr;

      }

      
      if (w[0] == 0 && w[1] == 0 && w[2] == 0) continue;

      // amp weighted sum            
      float wsum = w[0]+w[1]+w[2];
      if (wsum!=0){
        w[0] = w[0]/wsum;
        w[1] = w[1]/wsum;
        w[2] = w[2]/wsum;
      }

      float tAmpWSum_ampCorr = 0;
      float tAmpWSum_ampCorr_tDiffCorr = 0;
      for (int iBar =0; iBar < NBARS; iBar++){
	tAmpWSum_ampCorr+= w[iBar] * t_ampCorr[iBar];
	tAmpWSum_ampCorr_tDiffCorr+= w[iBar] * t_ampCorr_tDiffCorr[iBar];
      } 


      // resolution weighted sum 
      for ( int iBar = 0; iBar < NBARS; iBar++){
	if ( posY > cut_Ymin[iBar] && posY < cut_Ymax[iBar] && w[iBar] > 0) {
	  float tRes = g_tResolGausAve_ampCorr_tDiffCorr_vs_posY[iBar]->Eval(posY)  ;
	  wr[iBar] = 1./(tRes*tRes);
	}
	else {
	  wr[iBar] = 0;
	}
      }
            
      float wrsum = wr[0]+wr[1]+wr[2];
      if (wrsum!=0){
        wr[0] = wr[0]/wrsum;
        wr[1] = wr[1]/wrsum;
        wr[2] = wr[2]/wrsum;
      }

      float tResWSum_ampCorr = 0;
      float tResWSum_ampCorr_tDiffCorr = 0;
      for (int iBar =0; iBar < NBARS; iBar++){
	tResWSum_ampCorr+= wr[iBar] * t_ampCorr[iBar];
	tResWSum_ampCorr_tDiffCorr+= wr[iBar] * t_ampCorr_tDiffCorr[iBar];
      } 

      
      // unweighted  sum 
      float tSum_ampCorr = 0.;
      float tSum_ampCorr_tDiffCorr = 0.;
      int n1 = 0;
      int n2 = 0;
      for (int iBar =0; iBar < NBARS; iBar++){
	if ( t_ampCorr[iBar]!=-999 ) {
	  tSum_ampCorr+=t_ampCorr[iBar] ;
	  n1++;
	}
	if ( t_ampCorr_tDiffCorr[iBar]!=-999 ) {
	  tSum_ampCorr_tDiffCorr+=t_ampCorr_tDiffCorr[iBar] ;
	  n2++;
	}
      }
      tSum_ampCorr/=n1;
      tSum_ampCorr_tDiffCorr/=n2;
      
      int xbin = int ((posX-cut_Xmin[0])/xbinWidth );
      int ybin = int ((posY-cut_Ymin[NBARS-1])/ybinWidth );
      


      //if ( n1==2 && n2==2 )
      {
	h2_tAve1_vs_tAve2->Fill(t_ampCorr_tDiffCorr[2], t_ampCorr_tDiffCorr[1]);
	h_tAveSum_ampCorr->Fill(tSum_ampCorr);
	h_tAveSum_ampCorr_tDiffCorr->Fill(tSum_ampCorr_tDiffCorr);
	h_tAveSum_ampCorr_binX[xbin] ->Fill(tSum_ampCorr);
	h_tAveSum_ampCorr_tDiffCorr_binX[xbin] ->Fill(tSum_ampCorr_tDiffCorr);
	h_tAveSum_ampCorr_binY[ybin] ->Fill(tSum_ampCorr);
	h_tAveSum_ampCorr_tDiffCorr_binY[ybin] ->Fill(tSum_ampCorr_tDiffCorr);
	p_tAveSum_ampCorr_vs_posX ->Fill(posX,tSum_ampCorr);
	p_tAveSum_ampCorr_tDiffCorr_vs_posX ->Fill(posX,tSum_ampCorr_tDiffCorr);
	p_tAveSum_ampCorr_vs_posY ->Fill(posY,tSum_ampCorr);
	p_tAveSum_ampCorr_tDiffCorr_vs_posY ->Fill(posY,tSum_ampCorr_tDiffCorr);
      }

      //if ( (w[0]!=0 && w[1]!=0) || (w[1]!=0 && w[2]!=0) )
      {
	h_tAveAmpWSum_ampCorr->Fill(tAmpWSum_ampCorr);
	h_tAveAmpWSum_ampCorr_tDiffCorr->Fill(tAmpWSum_ampCorr_tDiffCorr);
	h_tAveAmpWSum_ampCorr_binX[xbin] ->Fill(tAmpWSum_ampCorr);
	h_tAveAmpWSum_ampCorr_tDiffCorr_binX[xbin] ->Fill(tAmpWSum_ampCorr_tDiffCorr);
	h_tAveAmpWSum_ampCorr_binY[ybin] ->Fill(tAmpWSum_ampCorr);
	h_tAveAmpWSum_ampCorr_tDiffCorr_binY[ybin] ->Fill(tAmpWSum_ampCorr_tDiffCorr);
	p_tAveAmpWSum_ampCorr_vs_posX ->Fill(posX,tAmpWSum_ampCorr);
	p_tAveAmpWSum_ampCorr_tDiffCorr_vs_posX ->Fill(posX,tAmpWSum_ampCorr_tDiffCorr);
	p_tAveAmpWSum_ampCorr_vs_posY ->Fill(posY,tAmpWSum_ampCorr);
	p_tAveAmpWSum_ampCorr_tDiffCorr_vs_posY ->Fill(posY,tAmpWSum_ampCorr_tDiffCorr);
      }

      //if ( (w[0]!=0 && w[1]!=0) || (w[1]!=0 && w[2]!=0) )
      {
	h_tAveResWSum_ampCorr->Fill(tResWSum_ampCorr);
	h_tAveResWSum_ampCorr_tDiffCorr->Fill(tResWSum_ampCorr_tDiffCorr);
	h_tAveResWSum_ampCorr_binX[xbin] ->Fill(tResWSum_ampCorr);
	h_tAveResWSum_ampCorr_tDiffCorr_binX[xbin] ->Fill(tResWSum_ampCorr_tDiffCorr);
	h_tAveResWSum_ampCorr_binY[ybin] ->Fill(tResWSum_ampCorr);
	h_tAveResWSum_ampCorr_tDiffCorr_binY[ybin] ->Fill(tResWSum_ampCorr_tDiffCorr);
	p_tAveResWSum_ampCorr_vs_posX ->Fill(posX,tResWSum_ampCorr);
	p_tAveResWSum_ampCorr_tDiffCorr_vs_posX ->Fill(posX,tResWSum_ampCorr_tDiffCorr);
	p_tAveResWSum_ampCorr_vs_posY ->Fill(posY,tResWSum_ampCorr);
	p_tAveResWSum_ampCorr_tDiffCorr_vs_posY ->Fill(posY,tResWSum_ampCorr_tDiffCorr);
      }
      
    } //--- end fourth loop over events
  

  // graphs for bars combination
  TF1*  fitFunSum_ampCorr_binX[1000];
  TF1*  fitFunSum_ampCorr_tDiffCorr_binX[1000];

  TF1*  fitFunSum_ampCorr_binY[1000];
  TF1*  fitFunSum_ampCorr_tDiffCorr_binY[1000];

  TF1*  fitFunAmpWSum_ampCorr_binX[1000];
  TF1*  fitFunAmpWSum_ampCorr_tDiffCorr_binX[1000];

  TF1*  fitFunAmpWSum_ampCorr_binY[1000];
  TF1*  fitFunAmpWSum_ampCorr_tDiffCorr_binY[1000];

  TF1*  fitFunResWSum_ampCorr_binX[1000];
  TF1*  fitFunResWSum_ampCorr_tDiffCorr_binX[1000];

  TF1*  fitFunResWSum_ampCorr_binY[1000];
  TF1*  fitFunResWSum_ampCorr_tDiffCorr_binY[1000];

  TGraphErrors *g_tResolGausSum_ampCorr_vs_posX = new TGraphErrors();
  g_tResolGausSum_ampCorr_vs_posX->SetName("g_tResolGausSum_ampCorr_vs_posX");

  TGraphErrors *g_tResolGausSum_ampCorr_tDiffCorr_vs_posX = new TGraphErrors();
  g_tResolGausSum_ampCorr_tDiffCorr_vs_posX->SetName("g_tResolGausSum_ampCorr_tDiffCorr_vs_posX");

  TGraphErrors *g_tResolGausSum_ampCorr_vs_posY = new TGraphErrors();
  g_tResolGausSum_ampCorr_vs_posY->SetName("g_tResolGausSum_ampCorr_vs_posY");

  TGraphErrors *g_tResolGausSum_ampCorr_tDiffCorr_vs_posY = new TGraphErrors();
  g_tResolGausSum_ampCorr_tDiffCorr_vs_posY->SetName("g_tResolGausSum_ampCorr_tDiffCorr_vs_posY");


  TGraphErrors *g_tResolGausAmpWSum_ampCorr_vs_posX = new TGraphErrors();
  g_tResolGausAmpWSum_ampCorr_vs_posX->SetName("g_tResolGausAmpWSum_ampCorr_vs_posX");

  TGraphErrors *g_tResolGausAmpWSum_ampCorr_tDiffCorr_vs_posX = new TGraphErrors();
  g_tResolGausAmpWSum_ampCorr_tDiffCorr_vs_posX->SetName("g_tResolGausAmpWSum_ampCorr_tDiffCorr_vs_posX");

  TGraphErrors *g_tResolGausAmpWSum_ampCorr_vs_posY = new TGraphErrors();
  g_tResolGausAmpWSum_ampCorr_vs_posY->SetName("g_tResolGausAmpWSum_ampCorr_vs_posY");

  TGraphErrors *g_tResolGausAmpWSum_ampCorr_tDiffCorr_vs_posY = new TGraphErrors();
  g_tResolGausAmpWSum_ampCorr_tDiffCorr_vs_posY->SetName("g_tResolGausAmpWSum_ampCorr_tDiffCorr_vs_posY");


  TGraphErrors *g_tResolGausResWSum_ampCorr_vs_posX = new TGraphErrors();
  g_tResolGausResWSum_ampCorr_vs_posX->SetName("g_tResolGausResWSum_ampCorr_vs_posX");

  TGraphErrors *g_tResolGausResWSum_ampCorr_tDiffCorr_vs_posX = new TGraphErrors();
  g_tResolGausResWSum_ampCorr_tDiffCorr_vs_posX->SetName("g_tResolGausResWSum_ampCorr_tDiffCorr_vs_posX");

  TGraphErrors *g_tResolGausResWSum_ampCorr_vs_posY = new TGraphErrors();
  g_tResolGausResWSum_ampCorr_vs_posY->SetName("g_tResolGausResWSum_ampCorr_vs_posY");

  TGraphErrors *g_tResolGausResWSum_ampCorr_tDiffCorr_vs_posY = new TGraphErrors();
  g_tResolGausResWSum_ampCorr_tDiffCorr_vs_posY->SetName("g_tResolGausResWSum_ampCorr_tDiffCorr_vs_posY");


  //

  TGraphErrors *g_tResolEffSum_ampCorr_vs_posX = new TGraphErrors();
  g_tResolEffSum_ampCorr_vs_posX->SetName("g_tResolEffSum_ampCorr_vs_posX");

  TGraphErrors *g_tResolEffSum_ampCorr_tDiffCorr_vs_posX = new TGraphErrors();
  g_tResolEffSum_ampCorr_tDiffCorr_vs_posX->SetName("g_tResolEffSum_ampCorr_tDiffCorr_vs_posX");

  TGraphErrors *g_tResolEffSum_ampCorr_vs_posY = new TGraphErrors();
  g_tResolEffSum_ampCorr_vs_posY->SetName("g_tResolEffSum_ampCorr_vs_posY");

  TGraphErrors *g_tResolEffSum_ampCorr_tDiffCorr_vs_posY = new TGraphErrors();
  g_tResolEffSum_ampCorr_tDiffCorr_vs_posY->SetName("g_tResolEffSum_ampCorr_tDiffCorr_vs_posY");


  TGraphErrors *g_tResolEffAmpWSum_ampCorr_vs_posX = new TGraphErrors();
  g_tResolEffAmpWSum_ampCorr_vs_posX->SetName("g_tResolEffAmpWSum_ampCorr_vs_posX");

  TGraphErrors *g_tResolEffAmpWSum_ampCorr_tDiffCorr_vs_posX = new TGraphErrors();
  g_tResolEffAmpWSum_ampCorr_tDiffCorr_vs_posX->SetName("g_tResolEffAmpWSum_ampCorr_tDiffCorr_vs_posX");

  TGraphErrors *g_tResolEffAmpWSum_ampCorr_vs_posY = new TGraphErrors();
  g_tResolEffAmpWSum_ampCorr_vs_posY->SetName("g_tResolEffAmpWSum_ampCorr_vs_posY");

  TGraphErrors *g_tResolEffAmpWSum_ampCorr_tDiffCorr_vs_posY = new TGraphErrors();
  g_tResolEffAmpWSum_ampCorr_tDiffCorr_vs_posY->SetName("g_tResolEffAmpWSum_ampCorr_tDiffCorr_vs_posY");


  TGraphErrors *g_tResolEffResWSum_ampCorr_vs_posX = new TGraphErrors();
  g_tResolEffResWSum_ampCorr_vs_posX->SetName("g_tResolEffResWSum_ampCorr_vs_posX");

  TGraphErrors *g_tResolEffResWSum_ampCorr_tDiffCorr_vs_posX = new TGraphErrors();
  g_tResolEffResWSum_ampCorr_tDiffCorr_vs_posX->SetName("g_tResolEffResWSum_ampCorr_tDiffCorr_vs_posX");

  TGraphErrors *g_tResolEffResWSum_ampCorr_vs_posY = new TGraphErrors();
  g_tResolEffResWSum_ampCorr_vs_posY->SetName("g_tResolEffResWSum_ampCorr_vs_posY");

  TGraphErrors *g_tResolEffResWSum_ampCorr_tDiffCorr_vs_posY = new TGraphErrors();
  g_tResolEffResWSum_ampCorr_tDiffCorr_vs_posY->SetName("g_tResolEffResWSum_ampCorr_tDiffCorr_vs_posY");

  
  // -- vs X
  for (int ibin = 0; ibin < NBINSX[0]; ibin++){

    float xx = cut_Xmin[0] + xbinWidth*ibin + xbinWidth/2;

    // -- simple average
    fitFunSum_ampCorr_binX[ibin]= new TF1(Form("fitFunSum_ampCorr_binX%d",ibin),"gaus", -20, 20);
    GetTimeResolution(h_tAveSum_ampCorr_binX[ibin], sigmaEff, sigmaGaus, fitFunSum_ampCorr_binX[ibin]);

    sigmaGaus    = fitFunSum_ampCorr_binX[ibin]->GetParameter(2);
    sigmaGausErr = fitFunSum_ampCorr_binX[ibin]->GetParError(2);
    resolEff  = sqrt(sigmaEff*sigmaEff - resolMCP*resolMCP);
    resolGaus = sqrt(sigmaGaus*sigmaGaus - resolMCP*resolMCP);
    resolGausErr = sigmaGausErr/resolGaus * sigmaGaus;

    if ( !isnan(resolGaus) ) {
      g_tResolGausSum_ampCorr_vs_posX->SetPoint(ibin, xx, resolGaus*1000.);
      g_tResolGausSum_ampCorr_vs_posX->SetPointError(ibin, xbinWidth/2., resolGausErr*1000.);
    }


    fitFunSum_ampCorr_tDiffCorr_binX[ibin]= new TF1(Form("fitFunSum_ampCorr_tDiffCorr_binX%d",ibin),"gaus", -20, 20);
    GetTimeResolution(h_tAveSum_ampCorr_tDiffCorr_binX[ibin], sigmaEff, sigmaGaus, fitFunSum_ampCorr_tDiffCorr_binX[ibin]);

    sigmaGaus    = fitFunSum_ampCorr_tDiffCorr_binX[ibin]->GetParameter(2);
    sigmaGausErr = fitFunSum_ampCorr_tDiffCorr_binX[ibin]->GetParError(2);
    resolEff  = sqrt(sigmaEff*sigmaEff - resolMCP*resolMCP);
    resolGaus = sqrt(sigmaGaus*sigmaGaus - resolMCP*resolMCP);
    resolGausErr = sigmaGausErr/resolGaus * sigmaGaus;

    if ( !isnan(resolGaus) ) {
      g_tResolGausSum_ampCorr_tDiffCorr_vs_posX->SetPoint(ibin, xx, resolGaus*1000.);
      g_tResolGausSum_ampCorr_tDiffCorr_vs_posX->SetPointError(ibin, xbinWidth/2., resolGausErr*1000.);
    }


    // - amp weighted average
    fitFunAmpWSum_ampCorr_binX[ibin]= new TF1(Form("fitFunAmpWSum_ampCorr_binX%d",ibin),"gaus", -20, 20);
    GetTimeResolution(h_tAveAmpWSum_ampCorr_binX[ibin], sigmaEff, sigmaGaus, fitFunAmpWSum_ampCorr_binX[ibin]);

    sigmaGaus    = fitFunAmpWSum_ampCorr_binX[ibin]->GetParameter(2);
    sigmaGausErr = fitFunAmpWSum_ampCorr_binX[ibin]->GetParError(2);
    resolEff  = sqrt(sigmaEff*sigmaEff - resolMCP*resolMCP);
    resolGaus = sqrt(sigmaGaus*sigmaGaus - resolMCP*resolMCP);
    resolGausErr = sigmaGausErr/resolGaus * sigmaGaus;

    if ( !isnan(resolGaus) ) {
      g_tResolGausAmpWSum_ampCorr_vs_posX->SetPoint(ibin, xx, resolGaus*1000.);
      g_tResolGausAmpWSum_ampCorr_vs_posX->SetPointError(ibin, xbinWidth/2., resolGausErr*1000.);
    }


    fitFunAmpWSum_ampCorr_tDiffCorr_binX[ibin]= new TF1(Form("fitFunAmpWSum_ampCorr_tDiffCorr_binX%d",ibin),"gaus", -20, 20);
    GetTimeResolution(h_tAveAmpWSum_ampCorr_tDiffCorr_binX[ibin], sigmaEff, sigmaGaus, fitFunAmpWSum_ampCorr_tDiffCorr_binX[ibin]);

    sigmaGaus    = fitFunAmpWSum_ampCorr_tDiffCorr_binX[ibin]->GetParameter(2);
    sigmaGausErr = fitFunAmpWSum_ampCorr_tDiffCorr_binX[ibin]->GetParError(2);
    resolEff  = sqrt(sigmaEff*sigmaEff - resolMCP*resolMCP);
    resolGaus = sqrt(sigmaGaus*sigmaGaus - resolMCP*resolMCP);
    resolGausErr = sigmaGausErr/resolGaus * sigmaGaus;

    if ( !isnan(resolGaus) ) {
      g_tResolGausAmpWSum_ampCorr_tDiffCorr_vs_posX->SetPoint(ibin, xx, resolGaus*1000.);
      g_tResolGausAmpWSum_ampCorr_tDiffCorr_vs_posX->SetPointError(ibin, xbinWidth/2., resolGausErr*1000.);
    }


    // - res weighted average
    fitFunResWSum_ampCorr_binX[ibin]= new TF1(Form("fitFunResWSum_ampCorr_binX%d",ibin),"gaus", -20, 20);
    GetTimeResolution(h_tAveResWSum_ampCorr_binX[ibin], sigmaEff, sigmaGaus, fitFunResWSum_ampCorr_binX[ibin]);

    sigmaGaus    = fitFunResWSum_ampCorr_binX[ibin]->GetParameter(2);
    sigmaGausErr = fitFunResWSum_ampCorr_binX[ibin]->GetParError(2);
    resolEff  = sqrt(sigmaEff*sigmaEff - resolMCP*resolMCP);
    resolGaus = sqrt(sigmaGaus*sigmaGaus - resolMCP*resolMCP);
    resolGausErr = sigmaGausErr/resolGaus * sigmaGaus;

    if ( !isnan(resolGaus) ) {
      g_tResolGausResWSum_ampCorr_vs_posX->SetPoint(ibin, xx, resolGaus*1000.);
      g_tResolGausResWSum_ampCorr_vs_posX->SetPointError(ibin, xbinWidth/2., resolGausErr*1000.);
    }


    fitFunResWSum_ampCorr_tDiffCorr_binX[ibin]= new TF1(Form("fitFunResWSum_ampCorr_tDiffCorr_binX%d",ibin),"gaus", -20, 20);
    GetTimeResolution(h_tAveResWSum_ampCorr_tDiffCorr_binX[ibin], sigmaEff, sigmaGaus, fitFunResWSum_ampCorr_tDiffCorr_binX[ibin]);

    sigmaGaus    = fitFunResWSum_ampCorr_tDiffCorr_binX[ibin]->GetParameter(2);
    sigmaGausErr = fitFunResWSum_ampCorr_tDiffCorr_binX[ibin]->GetParError(2);
    resolEff  = sqrt(sigmaEff*sigmaEff - resolMCP*resolMCP);
    resolGaus = sqrt(sigmaGaus*sigmaGaus - resolMCP*resolMCP);
    resolGausErr = sigmaGausErr/resolGaus * sigmaGaus;

    if ( !isnan(resolGaus) ) {
      g_tResolGausResWSum_ampCorr_tDiffCorr_vs_posX->SetPoint(ibin, xx, resolGaus*1000.);
      g_tResolGausResWSum_ampCorr_tDiffCorr_vs_posX->SetPointError(ibin, xbinWidth/2., resolGausErr*1000.);
    }
  }

  // -- vs Y
  int np = 0;
  int np1 = 0;
  for (int ibin = 0; ibin < NBINSY; ibin++){

    float yy = cut_Ymin[NBARS-1] + ybinWidth*ibin + ybinWidth/2;

    if (h_tAveSum_ampCorr_binY[ibin]->GetEntries() == 0 || h_tAveSum_ampCorr_tDiffCorr_binY[ibin] == 0) continue;


    // -- simple average
    fitFunSum_ampCorr_binY[ibin]= new TF1(Form("fitFunSum_ampCorr_binY%d",ibin),"gaus", -20, 20);
    GetTimeResolution(h_tAveSum_ampCorr_binY[ibin], sigmaEff, sigmaGaus, fitFunSum_ampCorr_binY[ibin]);

    sigmaGaus    = fitFunSum_ampCorr_binY[ibin]->GetParameter(2);
    sigmaGausErr = fitFunSum_ampCorr_binY[ibin]->GetParError(2);
    resolEff  = sqrt(sigmaEff*sigmaEff - resolMCP*resolMCP);
    resolGaus = sqrt(sigmaGaus*sigmaGaus - resolMCP*resolMCP);
    resolGausErr = sigmaGausErr/resolGaus * sigmaGaus;

    if ( !isnan(resolGaus) ) {
      g_tResolGausSum_ampCorr_vs_posY->SetPoint(np, yy, resolGaus*1000.);
      g_tResolGausSum_ampCorr_vs_posY->SetPointError(np, ybinWidth/2., resolGausErr*1000.);
      g_tResolEffSum_ampCorr_vs_posY->SetPoint(np, yy, resolEff*1000.);
      g_tResolEffSum_ampCorr_vs_posY->SetPointError(np, ybinWidth/2., resolGausErr*1000.);
    }

    
    fitFunSum_ampCorr_tDiffCorr_binY[ibin]= new TF1(Form("fitFunSum_ampCorr_tDiffCorr_binY%d",ibin),"gaus", -20, 20);
    GetTimeResolution(h_tAveSum_ampCorr_tDiffCorr_binY[ibin], sigmaEff, sigmaGaus, fitFunSum_ampCorr_tDiffCorr_binY[ibin]);

    sigmaGaus    = fitFunSum_ampCorr_tDiffCorr_binY[ibin]->GetParameter(2);
    sigmaGausErr = fitFunSum_ampCorr_tDiffCorr_binY[ibin]->GetParError(2);
    resolEff  = sqrt(sigmaEff*sigmaEff - resolMCP*resolMCP);
    resolGaus = sqrt(sigmaGaus*sigmaGaus - resolMCP*resolMCP);
    resolGausErr = sigmaGausErr/resolGaus * sigmaGaus;


    if ( !isnan(resolGaus) ) {
      g_tResolGausSum_ampCorr_tDiffCorr_vs_posY->SetPoint(np, yy, resolGaus*1000.);
      g_tResolGausSum_ampCorr_tDiffCorr_vs_posY->SetPointError(np, ybinWidth/2., resolGausErr*1000.);
      g_tResolEffSum_ampCorr_tDiffCorr_vs_posY->SetPoint(np, yy, resolEff*1000.);
      g_tResolEffSum_ampCorr_tDiffCorr_vs_posY->SetPointError(np, ybinWidth/2., resolGausErr*1000.);
    }

    

    // -- amp weighted average     
    fitFunAmpWSum_ampCorr_binY[ibin]= new TF1(Form("fitFunAmpWSum_ampCorr_binY%d",ibin),"gaus", -20, 20);
    GetTimeResolution(h_tAveAmpWSum_ampCorr_binY[ibin], sigmaEff, sigmaGaus, fitFunAmpWSum_ampCorr_binY[ibin]);

    sigmaGaus    = fitFunAmpWSum_ampCorr_binY[ibin]->GetParameter(2);
    sigmaGausErr = fitFunAmpWSum_ampCorr_binY[ibin]->GetParError(2);
    resolEff  = sqrt(sigmaEff*sigmaEff - resolMCP*resolMCP);
    resolGaus = sqrt(sigmaGaus*sigmaGaus - resolMCP*resolMCP);
    resolGausErr = sigmaGausErr/resolGaus * sigmaGaus;

    if ( !isnan(resolGaus) ) {
      g_tResolGausAmpWSum_ampCorr_vs_posY->SetPoint(np, yy, resolGaus*1000.);
      g_tResolGausAmpWSum_ampCorr_vs_posY->SetPointError(np, ybinWidth/2., resolGausErr*1000.);
      g_tResolEffAmpWSum_ampCorr_vs_posY->SetPoint(np, yy, resolEff*1000.);
      g_tResolEffAmpWSum_ampCorr_vs_posY->SetPointError(np, ybinWidth/2., resolGausErr*1000.);
    }


    fitFunAmpWSum_ampCorr_tDiffCorr_binY[ibin]= new TF1(Form("fitFunAmpWSum_ampCorr_tDiffCorr_binY%d",ibin),"gaus", -20, 20);
    GetTimeResolution(h_tAveAmpWSum_ampCorr_tDiffCorr_binY[ibin], sigmaEff, sigmaGaus, fitFunAmpWSum_ampCorr_tDiffCorr_binY[ibin]);

    sigmaGaus    = fitFunAmpWSum_ampCorr_tDiffCorr_binY[ibin]->GetParameter(2);
    sigmaGausErr = fitFunAmpWSum_ampCorr_tDiffCorr_binY[ibin]->GetParError(2);
    resolEff  = sqrt(sigmaEff*sigmaEff - resolMCP*resolMCP);
    resolGaus = sqrt(sigmaGaus*sigmaGaus - resolMCP*resolMCP);
    resolGausErr = sigmaGausErr/resolGaus * sigmaGaus;

    if ( !isnan(resolGaus) ) {
      g_tResolGausAmpWSum_ampCorr_tDiffCorr_vs_posY->SetPoint(np, yy, resolGaus*1000.);
      g_tResolGausAmpWSum_ampCorr_tDiffCorr_vs_posY->SetPointError(np, ybinWidth/2., resolGausErr*1000.);
      g_tResolEffAmpWSum_ampCorr_tDiffCorr_vs_posY->SetPoint(np, yy, resolEff*1000.);
      g_tResolEffAmpWSum_ampCorr_tDiffCorr_vs_posY->SetPointError(np, ybinWidth/2., resolGausErr*1000.);
    }



    // -- res weighted average     
    fitFunResWSum_ampCorr_binY[ibin]= new TF1(Form("fitFunResWSum_ampCorr_binY%d",ibin),"gaus", -20, 20);
    GetTimeResolution(h_tAveResWSum_ampCorr_binY[ibin], sigmaEff, sigmaGaus, fitFunResWSum_ampCorr_binY[ibin]);

    sigmaGaus    = fitFunResWSum_ampCorr_binY[ibin]->GetParameter(2);
    sigmaGausErr = fitFunResWSum_ampCorr_binY[ibin]->GetParError(2);
    resolEff  = sqrt(sigmaEff*sigmaEff - resolMCP*resolMCP);
    resolGaus = sqrt(sigmaGaus*sigmaGaus - resolMCP*resolMCP);
    resolGausErr = sigmaGausErr/resolGaus * sigmaGaus;

    if ( !isnan(resolGaus) ) {
      g_tResolGausResWSum_ampCorr_vs_posY->SetPoint(np, yy, resolGaus*1000.);
      g_tResolGausResWSum_ampCorr_vs_posY->SetPointError(np, ybinWidth/2., resolGausErr*1000.);
      g_tResolEffResWSum_ampCorr_vs_posY->SetPoint(np, yy, resolEff*1000.);
      g_tResolEffResWSum_ampCorr_vs_posY->SetPointError(np, ybinWidth/2., resolGausErr*1000.);
    }


    fitFunResWSum_ampCorr_tDiffCorr_binY[ibin]= new TF1(Form("fitFunResWSum_ampCorr_tDiffCorr_binY%d",ibin),"gaus", -20, 20);
    GetTimeResolution(h_tAveResWSum_ampCorr_tDiffCorr_binY[ibin], sigmaEff, sigmaGaus, fitFunResWSum_ampCorr_tDiffCorr_binY[ibin]);

    sigmaGaus    = fitFunResWSum_ampCorr_tDiffCorr_binY[ibin]->GetParameter(2);
    sigmaGausErr = fitFunResWSum_ampCorr_tDiffCorr_binY[ibin]->GetParError(2);
    resolEff  = sqrt(sigmaEff*sigmaEff - resolMCP*resolMCP);
    resolGaus = sqrt(sigmaGaus*sigmaGaus - resolMCP*resolMCP);
    resolGausErr = sigmaGausErr/resolGaus * sigmaGaus;

    if ( !isnan(resolGaus) ) {
      g_tResolGausResWSum_ampCorr_tDiffCorr_vs_posY->SetPoint(np, yy, resolGaus*1000.);
      g_tResolGausResWSum_ampCorr_tDiffCorr_vs_posY->SetPointError(np, ybinWidth/2., resolGausErr*1000.);
      g_tResolEffResWSum_ampCorr_tDiffCorr_vs_posY->SetPoint(np, yy, resolEff*1000.);
      g_tResolEffResWSum_ampCorr_tDiffCorr_vs_posY->SetPointError(np, ybinWidth/2., resolGausErr*1000.);
    }

    np++;
    

    // fill graph of number of events vs Y
    float nn[NBARS];
    for (int iBar = 0; iBar< NBARS; iBar++){
      nn[iBar] = h_tAve_ampCorr_tDiffCorr_binY[iBar][ibin]->GetEntries();
    }

    float r = 0;
    float er = 0;
    for (int iBar = 0; iBar< NBARS-1; iBar++){
      if ( nn[iBar] > 0 && nn[iBar+1] > 0) { // overlap 0/1
	r  = nn[iBar+1]/nn[iBar]; 
	if (nn[iBar+1] > nn[iBar] ) r  = nn[iBar]/nn[iBar+1];
	er = r * sqrt(1/nn[iBar+1] + 1/nn[iBar]);
	g_nEventsRatio_vs_posY -> SetPoint(np1, yy, r);
	g_nEventsRatio_vs_posY -> SetPointError(np1, ybinWidth/2., er);
	np1++;
      }
    }
    
  }
  

  // -- Save histograms on file
  std::cout << "Saving histograms in file " << OutputFile.c_str() <<std::endl;
  
  TFile *outfile = new TFile(OutputFile.c_str(),"recreate");
  
  h_beamXY -> Write();
  h_beamX  -> Write();
  h_beamY  -> Write();
 
  
  h_ampRef_nocuts->Write();
  h_ampRef->Write();
  h_timeRef->Write();
  h_brmsRef ->Write();
  p2_eff_vs_posXY_Ref -> Write();
  p_eff_vs_posX_Ref -> Write();
  p_eff_vs_posY_Ref -> Write();
  p_timeRef_vs_ampRef->Write();
  p_ampRef_vs_posX->Write();
  p_ampRef_vs_posY->Write();
  p_timeRef_vs_posX->Write();
  p_timeRef_vs_posY->Write();
  p2_ampRef_vs_posXY->Write();
  p2_timeRef_vs_posXY->Write();

  for (int iBar = 0; iBar < NBARS; iBar++){
    
    p2_eff_vs_posXY[iBar]->Write();
    p_eff_vs_posX[iBar]->Write();
    p_eff_vs_posY[iBar]->Write();
       
    h_ampL_nocuts[iBar]->Write();
    h_ampL[iBar]->Write();
    h_timeL[iBar]->Write();
    h2_timeL_vs_brms[iBar]->Write(); 
    h_brms_timeL[iBar] -> Write();
    p2_ampL_vs_posXY[iBar]->Write();
    p_ampL_vs_posX[iBar]->Write();
    p_ampL_vs_posXc[iBar]->Write();
    p_ampL_vs_posY[iBar]->Write();
    p_ampL_vs_tDiff[iBar]->Write();

    h_ampR_nocuts[iBar]->Write();
    h_ampR[iBar]->Write();
    h_timeR[iBar]->Write();
    h2_timeR_vs_brms[iBar]->Write();
    h_brms_timeR[iBar] -> Write(); 
    p2_ampR_vs_posXY[iBar]->Write();
    p_ampR_vs_posX[iBar]->Write();
    p_ampR_vs_posXc[iBar]->Write();
    p_ampR_vs_posY[iBar]->Write();
    p_ampR_vs_tDiff[iBar]->Write();


    // -- left
    p2_tL_vs_posXY[iBar]->Write();

    h_tL[iBar]->Write();
    h2_tL_vs_amp[iBar]->Write();
    p_tL_vs_amp[iBar]->Write();
    for (int ibin =0; ibin < NBINSXAMPWALK[iBar]; ibin++) {
      p_tL_vs_amp_binned[iBar][ibin]->Write();
      p_tL_ampCorr_vs_amp_binned[iBar][ibin]->Write();
    }
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
    h2_tR_vs_amp[iBar]->Write();
    p_tR_vs_amp[iBar]->Write();
    for (int ibin =0; ibin < NBINSXAMPWALK[iBar]; ibin++) {
      p_tR_vs_amp_binned[iBar][ibin]->Write();
      p_tR_ampCorr_vs_amp_binned[iBar][ibin]->Write();
    }
    p_tR_vs_posX[iBar]->Write();
    p_tR_vs_posY[iBar]->Write();
    p_tR_vs_tDiff[iBar]->Write();
    
    h_tR_ampCorr[iBar]->Write();
    p_tR_ampCorr_vs_amp[iBar]->Write();
    p_tR_ampCorr_vs_posX[iBar]->Write();
    p_tR_ampCorr_vs_posXc[iBar]->Write();
    p_tR_ampCorr_vs_posY[iBar]->Write();
    p_tR_ampCorr_vs_tDiff[iBar]->Write();

    // -- tDiff
    h_tDiff[iBar]->Write();
    h_tDiff_ampCorr[iBar]->Write();
    h_tDiff_ampCorr_posCorr[iBar]->Write();
    p_tDiff_ampCorr_vs_posX[iBar]->Write();
    p_tDiff_ampCorr_vs_posXc[iBar]->Write();
    p_tDiff_ampCorr_posCorr_vs_posX[iBar]->Write();
    p_tDiff_ampCorr_posCorr_vs_posXc[iBar]->Write();
    h2_tDiff_vs_posX[iBar]->Write();
    h2_tDiff_ampCorr_vs_posX[iBar]->Write();


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

    // eff sigmas    
    h_effectiveSigmaL[iBar]->Write();
    h_effectiveSigmaL_ampCorr[iBar]->Write();

    h_effectiveSigmaR[iBar]->Write();
    h_effectiveSigmaR_ampCorr[iBar]->Write();

    h_effectiveSigmaAve[iBar]->Write();

    h_effectiveSigmaAve_ampCorr[iBar]->Write();
    h_effectiveSigmaAve_ampCorr_tDiffCorr[iBar]->Write();
    h_effectiveSigmaAve_ampCorr_posCorr[iBar]->Write();

    h_effectiveSigmaDiff_ampCorr[iBar]->Write();
    h_effectiveSigmaDiff_ampCorr_posCorr[iBar]->Write();

    // graphs 
    g_tResolGausL_ampCorr[iBar]->Write();
    g_tResolGausR_ampCorr[iBar]->Write();

    g_tResolGausAve_ampCorr[iBar]->Write();
    g_tResolGausAve_ampCorr_tDiffCorr[iBar]->Write();
    g_tResolGausAve_ampCorr_posCorr[iBar]->Write();
    g_tResolGausAve_ampCorr_vs_posY[iBar]->Write();
    g_tResolGausAve_ampCorr_tDiffCorr_vs_posY[iBar]->Write();

    g_tResolGausDiff_ampCorr[iBar]->Write();
    g_tResolGausDiff_ampCorr_posCorr[iBar]->Write();
  
  }
  
  h2_tAve1_vs_tAve2->Write();

  h_tAveSum_ampCorr->Write();
  h_tAveSum_ampCorr_tDiffCorr->Write();
  p_tAveSum_ampCorr_vs_posX ->Write();
  p_tAveSum_ampCorr_tDiffCorr_vs_posX ->Write();
  p_tAveSum_ampCorr_vs_posY ->Write();
  p_tAveSum_ampCorr_tDiffCorr_vs_posY ->Write();
  
  h_tAveAmpWSum_ampCorr->Write();
  h_tAveAmpWSum_ampCorr_tDiffCorr->Write();
  p_tAveAmpWSum_ampCorr_vs_posX ->Write();
  p_tAveAmpWSum_ampCorr_tDiffCorr_vs_posX ->Write();
  p_tAveAmpWSum_ampCorr_vs_posY ->Write();
  p_tAveAmpWSum_ampCorr_tDiffCorr_vs_posY ->Write();
  
  g_tResolGausSum_ampCorr_vs_posX->Write();
  g_tResolGausSum_ampCorr_tDiffCorr_vs_posX->Write();
  g_tResolGausSum_ampCorr_vs_posY->Write();
  g_tResolGausSum_ampCorr_tDiffCorr_vs_posY->Write();
  
  g_tResolGausAmpWSum_ampCorr_vs_posX->Write();
  g_tResolGausAmpWSum_ampCorr_tDiffCorr_vs_posX->Write();
  g_tResolGausAmpWSum_ampCorr_vs_posY->Write();
  g_tResolGausAmpWSum_ampCorr_tDiffCorr_vs_posY->Write();

  g_tResolGausResWSum_ampCorr_vs_posX->Write();
  g_tResolGausResWSum_ampCorr_tDiffCorr_vs_posX->Write();
  g_tResolGausResWSum_ampCorr_vs_posY->Write();
  g_tResolGausResWSum_ampCorr_tDiffCorr_vs_posY->Write();



  g_tResolEffSum_ampCorr_vs_posX->Write();
  g_tResolEffSum_ampCorr_tDiffCorr_vs_posX->Write();
  g_tResolEffSum_ampCorr_vs_posY->Write();
  g_tResolEffSum_ampCorr_tDiffCorr_vs_posY->Write();
  
  g_tResolEffAmpWSum_ampCorr_vs_posX->Write();
  g_tResolEffAmpWSum_ampCorr_tDiffCorr_vs_posX->Write();
  g_tResolEffAmpWSum_ampCorr_vs_posY->Write();
  g_tResolEffAmpWSum_ampCorr_tDiffCorr_vs_posY->Write();

  g_tResolEffResWSum_ampCorr_vs_posX->Write();
  g_tResolEffResWSum_ampCorr_tDiffCorr_vs_posX->Write();
  g_tResolEffResWSum_ampCorr_vs_posY->Write();
  g_tResolEffResWSum_ampCorr_tDiffCorr_vs_posY->Write();
    
  g_nEventsRatio_vs_posY -> Write();

  cout << "Saving plots in subdir..."<<endl;

  TDirectory *dir1 = outfile->mkdir("xBins/");  
  TDirectory *dir2 = outfile->mkdir("yBins/");  
  dir1->cd();
  /*for (int iBar = 0; iBar < NBARS; iBar++){
      for (int ibin = 0 ; ibin < NBINSX[iBar]; ibin++){
      h_tL_ampCorr_binX[iBar][ibin] -> Write();
      h_tR_ampCorr_binX[iBar][ibin] -> Write();
      h_tAve_ampCorr_binX[iBar][ibin] -> Write();
      h_tDiff_ampCorr_binX[iBar][ibin]-> Write();
      //h_tAve_ampCorr_tDiffCorr_binX[iBar][ibin] -> Write();
      //h_tAveAmpW_ampCorr_binX[iBar][ibin] -> Write();
      //h_tAveAmpW_ampCorr_tDiffCorr_binX[iBar][ibin] -> Write();
      //h_tAveResW_ampCorr_binX[iBar][ibin] -> Write();
      //h_tAveResW_ampCorr_tDiffCorr_binX[iBar][ibin] -> Write();
    }
    }*/
  
  dir2->cd();
  for (int ibin = 0 ; ibin < NBINSY; ibin++){
    //h_tAveSum_ampCorr_binY[ibin] -> Write();
    //h_tAveAmpWSum_ampCorr_binY[ibin] -> Write();
    //h_tAveResWSum_ampCorr_binY[ibin] -> Write();
    h_tAveSum_ampCorr_tDiffCorr_binY[ibin] -> Write();
    h_tAveAmpWSum_ampCorr_tDiffCorr_binY[ibin] -> Write();
    h_tAveResWSum_ampCorr_tDiffCorr_binY[ibin] -> Write();
    for (int iBar = 0; iBar < NBARS; iBar++){
      //h_tAve_ampCorr_binY[iBar][ibin]->Write();
      h_tAve_ampCorr_tDiffCorr_binY[iBar][ibin]->Write();
    }
  }

  cout << "Closing file..."<<endl;

  outfile->Close();

  delete outfile;
  return 0;

}



// ---- Initialize tree -------------------------------------------------------------------------------------
void InitTreeVars(TTree* chain1,TreeVars& treeVars, float threshold){
  
  treeVars.t_amp = new float[1000];
  treeVars.t_time = new float[1000];
  treeVars.t_b_rms = new float[1000];
  treeVars.t_CFD = 0;
  treeVars.t_LED = 0;

  treeVars.t_channelId = new std::map<std::string,int>;

  treeVars.t_trackFitResult = new std::vector<TrackPar>;
  treeVars.t_ntracks = 0;

  //chain1 -> SetBranchStatus("*",0);

  chain1 -> SetBranchStatus("n_tracks",      1); chain1->SetBranchAddress("n_tracks",  &treeVars.t_ntracks);
  chain1 -> SetBranchStatus("fitResult",     1); chain1->SetBranchAddress("fitResult", &treeVars.t_trackFitResult);

  chain1 -> SetBranchStatus("amp_max",       1); chain1 -> SetBranchAddress("amp_max",      treeVars.t_amp);
  chain1 -> SetBranchStatus("time",          1); chain1 -> SetBranchAddress("time",         treeVars.t_time);
  chain1 -> SetBranchStatus("b_rms",         1); chain1 -> SetBranchAddress("b_rms",         treeVars.t_b_rms);
  chain1 -> SetBranchStatus("CFD",           1); chain1 -> SetBranchAddress("CFD",          &treeVars.t_CFD);
  cout << "Using threshold " << threshold << "  --> time : " << Form("LED%.0f",threshold) <<endl;
  chain1 -> SetBranchStatus(Form("LED%.0f",threshold),        1); chain1 -> SetBranchAddress(Form("LED%.0f",threshold),       &treeVars.t_LED);

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
  //tmin = ht->GetMean()-2.0*sEff;
  //tmax = ht->GetMean()+2.0*sEff;
  tmin = ht->GetMean()-3.0*ht->GetRMS();
  tmax = ht->GetMean()+3.0*ht->GetRMS();
  ht->Fit( fitFun, "QRS", "", tmin, tmax);
  sGaus = fitFun->GetParameter(2);
  
  
}
