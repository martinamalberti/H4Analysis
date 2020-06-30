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
  unsigned int t_event;
  unsigned int t_run;
  float* t_amp;
  float* t_time;
  float *t_time_max;
  float *t_chi2_max;
  float* t_b_rms;
  int  t_CFD;
  int  t_LED;
  float* t_gaus_sigma;

  std::vector<TrackPar> *t_trackFitResult;
  int t_ntracks;

  std::map<std::string,int> *t_channelId;

  float* t_charge_sig;
};
/**********************/

void InitTreeVars(TTree* chain1, TreeVars& treeVars, float threshold);
void GetTimeResolution( TH1F *ht, TF1* fitFun, float &resolEff, float &resolGaus, float &resolGausErr, float resolRef);
int GetPointFromX(TGraphErrors *g, double x);
float GetCorrectedMCPTime(float time, float amp, float x);

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

  ofstream selectedEventsFile;
  selectedEventsFile.open("selectedEvents.txt");

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

  int applyMCPTimeCorrection = opts.GetOpt<int>("Input.applyMCPTimeCorrection");
  int useAverageAmplitudeForTimeWalkCorr = opts.GetOpt<int>("Input.useAverageAmplitudeForTimeWalkCorr");

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


  bool useTimeMaxMCP = true;

  // -- list of runs without tracking desync
  //ifstream goodRunsFile("/afs/cern.ch/user/a/abenagli/public/MTDTB_FNAL_Apr2019_good_runs.txt");
  ifstream goodRunsFile("/afs/cern.ch/user/m/malberti/public/MTDTB_FNAL_Apr2019_good_runs.txt");
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
    // -- check if file exists
    std::string fileName = "/eos/cms/store/group/dpg_mtd/comm_mtd/TB/MTDTB_FNAL_Apr2019/ntuples/v11/"+ std::to_string(int(iRun))+".root";
    //std::string fileName = "/afs/cern.ch/work/m/malberti/MTD/TBatFNALApril2019/H4Analysis/ntuples/v11/"+ std::to_string(int(iRun))+".root";
    if (gSystem->AccessPathName(fileName.c_str()) == 1) continue;
    // -- check if this is a good run
    if ( !goodRuns.empty() && std::count(goodRuns.begin(), goodRuns.end(), int(iRun)) == false) {
      cout << "Run " << iRun << " not in the list of GOOD runs" << endl;
      continue;
    }
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
   
  float ybinWidth = 0.25 ; // mm
  int NBINSY = ceil ((cut_Ymax[0]-cut_Ymin[NBARS-1])/ybinWidth );

  float xbinWidthForAmpWalk =  999; // mm 
  //float xbinWidthForAmpWalk =  5.; // mm 
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

  float ampBinWidth = 0.1;

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
  TH2F *h2_ampRef_vs_event = new TH2F("h2_ampRef_vs_event", "h2_ampRef_vs_event", 3000, 0., 300000., 1000, 0., 1.);
  TProfile2D *p2_ampRef_vs_run_vs_event = new TProfile2D("p2_ampRef_vs_run_vs_event", "p2_ampRef_vs_run_vs_event", 500, 8500., 9000., 5000, 0., 5000.);
  TH1F *h_ampRef_nocuts = new TH1F("h_ampRef_nocuts", "h_ampRef_nocuts", 500, 0., 1.);
  TH1F *h_ampRef        = new TH1F("h_ampRef", "h_ampRef", 500, 0., 1.);
  TH1F *h_timeRef       = new TH1F("h_timeRef", "h_timeRef", 200, 0., 100.);;
  TH1F *h_brmsRef       = new TH1F("h_brmsRef", "h_brmsRef", 200, 0., 100.);;
  TH1F *h_chi2Ref       = new TH1F("h_chi2Ref", "h_chi2Ref", 1000, 0., 50.);;
  TH1F *h_sigmaRef      = new TH1F("h_sigmaRef", "h_sigmaRef", 1000, 0., 1.);;
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

  TH1F*  h_ampSum_nocuts = new TH1F("h_ampSum_nocuts","h_ampSum_nocuts", 1000, 0., 1.);
  TH1F*  h_pulseIntSum_nocuts = new TH1F("h_pulseIntSum_nocuts","h_pulseIntSum_nocuts", 2500, 0., 250.);

  TH1F*  h_ampL_nocuts[NBARS];
  TH1F*  h_ampL[NBARS];
  TH1F*  h_pulseIntL[NBARS];
  TH1F*  h_timeL[NBARS];
  TH1F*  h_brms_timeL[NBARS];
  TH2F*  h2_timeL_vs_brms[NBARS];
  TH2F*  h2_pulseIntL_vs_amp[NBARS];

  TProfile2D*  p2_ampL_vs_posXY[NBARS];
  TProfile*    p_ampL_vs_posX[NBARS];
  TProfile*    p_ampL_vs_posXc[NBARS];
  TProfile*    p_ampL_vs_posY[NBARS];
  TProfile*    p_ampL_vs_tDiff[NBARS];

  TH1F*  h_ampR_nocuts[NBARS];
  TH1F*  h_ampR[NBARS];
  TH1F*  h_pulseIntR[NBARS];
  TH1F*  h_timeR[NBARS];
  TH1F*  h_brms_timeR[NBARS];
  TH2F*  h2_timeR_vs_brms[NBARS];
  TH2F*  h2_pulseIntR_vs_amp[NBARS];
 
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
  TProfile*      p_tL_vs_amp_binned[NBARS][1000];                                                 
  TProfile*      p_tL_vs_posX[NBARS];
  TProfile*      p_tL_vs_posY[NBARS];
  TProfile*      p_tL_vs_tDiff[NBARS];
  TProfile*      p_tL_vs_pulseInt[NBARS];
  TProfile*      p_tL_vs_pulseInt_binned[NBARS][1000];
  TProfile2D*    p2_tL_vs_amp_vs_posXc[NBARS];

  TH1F*          h_tL_ampCorr[NBARS];
  TProfile*      p_tL_ampCorr_vs_amp[NBARS];
  TProfile*      p_tL_ampCorr_vs_amp_binned[NBARS][1000];  
  TProfile*      p_tL_ampCorr_vs_posX[NBARS];
  TProfile*      p_tL_ampCorr_vs_posY[NBARS];
  TProfile*      p_tL_ampCorr_vs_tDiff[NBARS];
  TH1F*          h_tL_ampCorr_binX[NBARS][1000];
  TProfile*      p_tL_ampCorr_vs_posXc[NBARS];

  TH1F*          h_tL_pulseIntCorr[NBARS];
  TProfile*      p_tL_pulseIntCorr_vs_pulseInt[NBARS];
  TProfile*      p_tL_pulseIntCorr_vs_posX[NBARS];
  TProfile*      p_tL_pulseIntCorr_vs_posY[NBARS];
  TProfile*      p_tL_pulseIntCorr_vs_tDiff[NBARS];
  TH1F*          h_tL_pulseIntCorr_binX[NBARS][1000];
  TProfile*      p_tL_pulseIntCorr_vs_posXc[NBARS];

  // -- right   
  TProfile2D*    p2_tR_vs_posXY[NBARS];

  TH1F*          h_tR[NBARS];
  TH2F*          h2_tR_vs_amp[NBARS];
  TProfile*      p_tR_vs_amp[NBARS];
  TProfile*      p_tR_vs_amp_binned[NBARS][1000];
  TProfile*      p_tR_vs_posX[NBARS];
  TProfile*      p_tR_vs_posY[NBARS];
  TProfile*      p_tR_vs_tDiff[NBARS];
  TProfile*      p_tR_vs_pulseInt[NBARS];
  TProfile*      p_tR_vs_pulseInt_binned[NBARS][1000];
  TProfile2D*    p2_tR_vs_amp_vs_posXc[NBARS];

  TH1F*          h_tR_ampCorr[NBARS];
  TProfile*      p_tR_ampCorr_vs_amp[NBARS];
  TProfile*      p_tR_ampCorr_vs_amp_binned[NBARS][1000];  
  TProfile*      p_tR_ampCorr_vs_posX[NBARS];
  TProfile*      p_tR_ampCorr_vs_posY[NBARS];
  TProfile*      p_tR_ampCorr_vs_tDiff[NBARS];
  TH1F*          h_tR_ampCorr_binX[NBARS][1000];
  TProfile*      p_tR_ampCorr_vs_posXc[NBARS];

  TH1F*          h_tR_pulseIntCorr[NBARS];
  TProfile*      p_tR_pulseIntCorr_vs_pulseInt[NBARS];
  TProfile*      p_tR_pulseIntCorr_vs_posX[NBARS];
  TProfile*      p_tR_pulseIntCorr_vs_posY[NBARS];
  TProfile*      p_tR_pulseIntCorr_vs_tDiff[NBARS];
  TH1F*          h_tR_pulseIntCorr_binX[NBARS][1000];
  TProfile*      p_tR_pulseIntCorr_vs_posXc[NBARS];

  // -- tDiff
  TH1F*          h_tDiff[NBARS];
  TProfile*      p_tDiff_vs_posX[NBARS];
  TProfile*      p_tDiff_vs_posXc[NBARS];
  TH1F*          h_tDiff_posCorr[NBARS];
  TProfile*      p_tDiff_posCorr_vs_posX[NBARS];
  TProfile*      p_tDiff_posCorr_vs_posXc[NBARS];
  TH1F*          h_tDiff_ampCorr[NBARS];
  TProfile*      p_tDiff_ampCorr_vs_posX[NBARS];
  TProfile*      p_tDiff_ampCorr_vs_posXc[NBARS];
  TH1F*          h_tDiff_ampCorr_posCorr[NBARS];
  TProfile*      p_tDiff_ampCorr_posCorr_vs_posX[NBARS];
  TProfile*      p_tDiff_ampCorr_posCorr_vs_posXc[NBARS];
  TProfile*      p_tDiff_ampCorr_posCorr_vs_posY[NBARS];
  TH1F*          h_tDiff_pulseIntCorr[NBARS];
  TProfile*      p_tDiff_pulseIntCorr_vs_posX[NBARS];
  TProfile*      p_tDiff_pulseIntCorr_vs_posXc[NBARS];
  TH1F*          h_tDiff_pulseIntCorr_posCorr[NBARS];
  TProfile*      p_tDiff_pulseIntCorr_posCorr_vs_posX[NBARS];
  TProfile*      p_tDiff_pulseIntCorr_posCorr_vs_posXc[NBARS];

  TH1F* h_tDiff_binX[NBARS][1000];
  TH1F* h_tDiff_posCorr_binX[NBARS][1000];

  TH1F* h_tDiff_ampCorr_binX[NBARS][1000];
  TH1F* h_tDiff_ampCorr_posCorr_binX[NBARS][1000];

  TH1F* h_tDiff_pulseIntCorr_binX[NBARS][1000];
  TH1F* h_tDiff_pulseIntCorr_posCorr_binX[NBARS][1000];

  TH1F* h_tDiff_binY[NBARS][1000];
  TH1F* h_tDiff_posCorr_binY[NBARS][1000];

  TH1F* h_tDiff_ampCorr_binY[NBARS][1000];
  TH1F* h_tDiff_ampCorr_posCorr_binY[NBARS][1000];

  TH1F* h_tDiff_pulseIntCorr_binY[NBARS][1000];
  TH1F* h_tDiff_pulseIntCorr_posCorr_binY[NBARS][1000];

  TH1F* h_tDiff_pulseIntCorr_posCorr_binAmp[NBARS][1000];

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

  TH1F*          h_tAve_ampCorr_posCorr[NBARS];
  TProfile*      p_tAve_ampCorr_posCorr_vs_posX[NBARS];
  TProfile*      p_tAve_ampCorr_posCorr_vs_posY[NBARS];
  TProfile*      p_tAve_ampCorr_posCorr_vs_tDiff[NBARS];
  TH1F*          h_tAve_ampCorr_posCorr_binX[NBARS][1000];
  TH1F*          h_tAve_ampCorr_posCorr_binY[NBARS][1000];
  TProfile*      p_tAve_ampCorr_posCorr_vs_posXc[NBARS];

  TH1F*          h_tAve_pulseIntCorr[NBARS];
  TProfile*      p_tAve_pulseIntCorr_vs_posX[NBARS];
  TProfile*      p_tAve_pulseIntCorr_vs_posY[NBARS];
  TProfile*      p_tAve_pulseIntCorr_vs_tDiff[NBARS];
  TH1F*          h_tAve_pulseIntCorr_binX[NBARS][1000];
  TH1F*          h_tAve_pulseIntCorr_binY[NBARS][1000];
  TProfile*      p_tAve_pulseIntCorr_vs_posXc[NBARS];

  TH1F*          h_tAve_pulseIntCorr_posCorr[NBARS];
  TProfile*      p_tAve_pulseIntCorr_posCorr_vs_posX[NBARS];
  TProfile*      p_tAve_pulseIntCorr_posCorr_vs_posY[NBARS];
  TProfile*      p_tAve_pulseIntCorr_posCorr_vs_tDiff[NBARS];
  TH1F*          h_tAve_pulseIntCorr_posCorr_binX[NBARS][1000];
  TH1F*          h_tAve_pulseIntCorr_posCorr_binY[NBARS][1000];
  TProfile*      p_tAve_pulseIntCorr_posCorr_vs_posXc[NBARS];


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
    h_pulseIntL[iBar] = new TH1F(Form("h_pulseIntL_BAR%d", iBar), Form("h_pulseIntL_BAR%d",iBar), 2500, 0., 250);
    h_timeL[iBar]       = new TH1F(Form("h_timeL_BAR%d",iBar),Form("h_timeL_BAR%d",iBar),400,0.,200.);
    h_brms_timeL[iBar]   = new TH1F(Form("h_brms_timeL_BAR%d",iBar),Form("h_brms_timeL_BAR%d",iBar),400,0.,200.);
    h2_timeL_vs_brms[iBar] = new TH2F(Form("h2_timeL_vs_brms_BAR%d",iBar),Form("h2_timeL_vs_brms_BAR%d",iBar),500,0.,500., 2000, 0, 200);
    h2_pulseIntL_vs_amp[iBar] = new TH2F(Form("h2_pulseIntL_vs_amp_BAR%d",iBar),Form("h2_pulseIntL_vs_amp_BAR%d",iBar),500,0.,1., 2500, 0, 250);

    h_ampR_nocuts[iBar] = new TH1F(Form("h_ampR_nocuts_BAR%d", iBar), Form("h_ampR_nocuts_BAR%d",iBar), 500, 0., 1.0);
    h_ampR[iBar]        = new TH1F(Form("h_ampR_BAR%d", iBar), Form("h_ampR_BAR%d",iBar), 500, 0., 1.0);
    h_pulseIntR[iBar] = new TH1F(Form("h_pulseIntR_BAR%d", iBar), Form("h_pulseIntR_BAR%d",iBar), 2500, 0., 250);
    h_timeR[iBar]       = new TH1F(Form("h_timeR_BAR%d",iBar),Form("h_timeR_BAR%d",iBar),400,0.,200.);
    h_brms_timeR[iBar]   = new TH1F(Form("h_brms_timeR_BAR%d",iBar),Form("h_brms_timeR_BAR%d",iBar),400,0.,200.);
    h2_timeR_vs_brms[iBar] = new TH2F(Form("h2_timeR_vs_brms_BAR%d",iBar),Form("h2_timeR_vs_brms_BAR%d",iBar),500,0.,500., 2000, 0, 200);
    h2_pulseIntR_vs_amp[iBar] = new TH2F(Form("h2_pulseIntR_vs_amp_BAR%d",iBar),Form("h2_pulseIntR_vs_amp_BAR%d",iBar),500,0.,1., 2500, 0, 250);

    p2_ampL_vs_posXY[iBar] = new TProfile2D(Form("p2_ampL_vs_posXY_BAR%d",iBar),Form("p2_ampL_vs_posXY_BAR%d",iBar), 100, xmin, xmax, 100, ymin, ymax, 0, 1);
    p_ampL_vs_posX[iBar]   = new TProfile(Form("p_ampL_vs_posX_BAR%d",iBar),Form("p_ampL_vs_posX_BAR%d",iBar), 100, xmin, xmax, 0, 1);
    p_ampL_vs_posXc[iBar]  = new TProfile(Form("p_ampL_vs_posXc_BAR%d",iBar),Form("p_ampL_vs_posXc_BAR%d",iBar), 100, xmin/cos(theta[iBar]), xmax/cos(theta[iBar]), 0, 1);
    p_ampL_vs_posY[iBar]   = new TProfile(Form("p_ampL_vs_posY_BAR%d",iBar),Form("p_ampL_vs_posY_BAR%d",iBar), 500, ymin, ymax, 0, 1);
    p_ampL_vs_tDiff[iBar]  = new TProfile(Form("p_ampL_vs_tDiff_BAR%d",iBar),Form("p_ampL_vs_tDiff_BAR%d",iBar), 400, -4, 4, 0, 1);

    p2_ampR_vs_posXY[iBar] = new TProfile2D(Form("p2_ampR_vs_posXY_BAR%d",iBar),Form("p2_ampR_vs_posXY_BAR%d",iBar), 100, xmin, xmax, 100, ymin, ymax, 0, 1);
    p_ampR_vs_posX[iBar]   = new TProfile(Form("p_ampR_vs_posX_BAR%d",iBar),Form("p_ampR_vs_posX_BAR%d",iBar), 100, xmin, xmax, 0, 1);
    p_ampR_vs_posXc[iBar]  = new TProfile(Form("p_ampR_vs_posXc_BAR%d",iBar),Form("p_ampR_vs_posXc_BAR%d",iBar), 100, xmin/cos(theta[iBar]), xmax/cos(theta[iBar]), 0, 1);
    p_ampR_vs_posY[iBar]   = new TProfile(Form("p_ampR_vs_posY_BAR%d",iBar),Form("p_ampR_vs_posY_BAR%d",iBar), 500, ymin, ymax, 0, 1);
    p_ampR_vs_tDiff[iBar]  = new TProfile(Form("p_ampR_vs_tDiff_BAR%d",iBar),Form("p_ampR_vs_tDiff_BAR%d",iBar), 400, -4, 4, 0, 1);

    // -- tLeft
    p2_tL_vs_posXY[iBar]        = new TProfile2D(Form("p2_tL_vs_posXY_BAR%d",iBar),Form("p2_tL_vs_posXY_BAR%d",iBar), 400, xmin, xmax, 200, ymin, ymax, dtminL[iBar], dtmaxL[iBar]);

    h_tL[iBar]                  = new TH1F(Form("h_tL_BAR%d",iBar),Form("h_tL_BAR%d",iBar), nTimeBins, dtminL[iBar], dtmaxL[iBar]);
    h2_tL_vs_amp[iBar]          = new TH2F(Form("h2_tL_vs_amp_BAR%d",iBar),Form("h2_tL_vs_amp_BAR%d",iBar), nAmpBins, 0., 1., 2000, dtminL[iBar], dtmaxL[iBar]);
    p_tL_vs_amp[iBar]           = new TProfile(Form("p_tL_vs_amp_BAR%d",iBar),Form("p_tL_vs_amp_BAR%d",iBar), nAmpBins, 0., 1.,dtminL[iBar], dtmaxL[iBar]);
    p_tL_vs_posX[iBar]          = new TProfile(Form("p_tL_vs_posX_BAR%d",iBar),Form("p_tL_vs_posX_BAR%d",iBar), 100, xmin, xmax,dtminL[iBar], dtmaxL[iBar]);
    p_tL_vs_posY[iBar]          = new TProfile(Form("p_tL_vs_posY_BAR%d",iBar),Form("p_tL_vs_posY_BAR%d",iBar), 100, ymin, ymax,dtminL[iBar], dtmaxL[iBar]);
    p_tL_vs_tDiff[iBar]         = new TProfile(Form("p_tL_vs_tDiff_BAR%d",iBar),Form("p_tL_vs_tDiff_BAR%d",iBar), 100, -4, 4, dtminL[iBar], dtmaxL[iBar]);
    p_tL_vs_pulseInt[iBar]      = new TProfile(Form("p_tL_vs_pulseInt_BAR%d",iBar),Form("p_tL_vs_pulseInt_BAR%d",iBar), nAmpBins*2, 0., 250.,dtminL[iBar], dtmaxL[iBar]);
    p2_tL_vs_amp_vs_posXc[iBar]  = new TProfile2D(Form("p2_tL_vs_amp_vs_posXc_BAR%d",iBar),Form("p2_tL_vs_amp_vs_posXc_BAR%d",iBar), nAmpBins, 0., 1., 100, xmin/cos(theta[iBar]), xmax/cos(theta[iBar]), dtminL[iBar], dtmaxL[iBar]);

    h_tL_ampCorr[iBar]          = new TH1F(Form("h_tL_ampCorr_BAR%d",iBar),Form("h_tL_ampCorr_BAR%d",iBar), nTimeBins, dtminL[iBar], dtmaxL[iBar]);
    p_tL_ampCorr_vs_amp[iBar]   = new TProfile(Form("p_tL_ampCorr_vs_amp_BAR%d",iBar),Form("p_tL_ampCorr_vs_amp_BAR%d",iBar), nAmpBins, 0., 1.,dtminL[iBar], dtmaxL[iBar]);
    p_tL_ampCorr_vs_posX[iBar]  = new TProfile(Form("p_tL_ampCorr_vs_posX_BAR%d",iBar),Form("p_tL_ampCorr_vs_posX_BAR%d",iBar), 100, xmin, xmax,dtminL[iBar], dtmaxL[iBar]);
    p_tL_ampCorr_vs_posY[iBar]  = new TProfile(Form("p_tL_ampCorr_vs_posY_BAR%d",iBar),Form("p_tL_ampCorr_vs_posY_BAR%d",iBar), 100, ymin, ymax,dtminL[iBar], dtmaxL[iBar]);
    p_tL_ampCorr_vs_tDiff[iBar] = new TProfile(Form("p_tL_ampCorr_vs_tDiff_BAR%d",iBar),Form("p_tL_ampCorr_vs_tDiff_BAR%d",iBar), 100, -4, 4,dtminL[iBar], dtmaxL[iBar]);
    p_tL_ampCorr_vs_posXc[iBar] = new TProfile(Form("p_tL_ampCorr_vs_posXc_BAR%d",iBar),Form("p_tL_ampCorr_vs_posXc_BAR%d",iBar), 100, xmin/cos(theta[iBar]), xmax/cos(theta[iBar]),dtminL[iBar], dtmaxL[iBar]);

    h_tL_pulseIntCorr[iBar]          = new TH1F(Form("h_tL_pulseIntCorr_BAR%d",iBar),Form("h_tL_pulseIntCorr_BAR%d",iBar), nTimeBins, dtminL[iBar], dtmaxL[iBar]);
    p_tL_pulseIntCorr_vs_pulseInt[iBar]   = new TProfile(Form("p_tL_pulseIntCorr_vs_pulseInt_BAR%d",iBar),Form("p_tL_pulseIntCorr_vs_pulseInt_BAR%d",iBar), nAmpBins*2, 0., 250.,dtminL[iBar], dtmaxL[iBar]);
    p_tL_pulseIntCorr_vs_posX[iBar]  = new TProfile(Form("p_tL_pulseIntCorr_vs_posX_BAR%d",iBar),Form("p_tL_pulseIntCorr_vs_posX_BAR%d",iBar), 100, xmin, xmax,dtminL[iBar], dtmaxL[iBar]);
    p_tL_pulseIntCorr_vs_posY[iBar]  = new TProfile(Form("p_tL_pulseIntCorr_vs_posY_BAR%d",iBar),Form("p_tL_pulseIntCorr_vs_posY_BAR%d",iBar), 100, ymin, ymax,dtminL[iBar], dtmaxL[iBar]);
    p_tL_pulseIntCorr_vs_tDiff[iBar] = new TProfile(Form("p_tL_pulseIntCorr_vs_tDiff_BAR%d",iBar),Form("p_tL_pulseIntCorr_vs_tDiff_BAR%d",iBar), 100, -4, 4,dtminL[iBar], dtmaxL[iBar]);
    p_tL_pulseIntCorr_vs_posXc[iBar] = new TProfile(Form("p_tL_pulseIntCorr_vs_posXc_BAR%d",iBar),Form("p_tL_pulseIntCorr_vs_posXc_BAR%d",iBar), 100, xmin/cos(theta[iBar]), xmax/cos(theta[iBar]),dtminL[iBar], dtmaxL[iBar]);

   
    // -- tRight
    p2_tR_vs_posXY[iBar]        = new TProfile2D(Form("p2_tR_vs_posXY_BAR%d",iBar),Form("p2_tR_vs_posXY_BAR%d",iBar), 400, xmin, xmax, 200, ymin, ymax,dtminR[iBar], dtmaxR[iBar]);

    h_tR[iBar]                  = new TH1F(Form("h_tR_BAR%d",iBar),Form("h_tR_BAR%d",iBar), nTimeBins, dtminR[iBar], dtmaxR[iBar]);
    h2_tR_vs_amp[iBar]          = new TH2F(Form("h2_tR_vs_amp_BAR%d",iBar),Form("h2_tR_vs_amp_BAR%d",iBar), nAmpBins, 0., 1., 2000, dtminR[iBar], dtmaxR[iBar]);
    p_tR_vs_amp[iBar]           = new TProfile(Form("p_tR_vs_amp_BAR%d",iBar),Form("p_tR_vs_amp_BAR%d",iBar), nAmpBins, 0., 1.,dtminR[iBar], dtmaxR[iBar]);
    p_tR_vs_posX[iBar]          = new TProfile(Form("p_tR_vs_posX_BAR%d",iBar),Form("p_tR_vs_posX_BAR%d",iBar), 100, xmin, xmax,dtminR[iBar], dtmaxR[iBar]);
    p_tR_vs_posY[iBar]          = new TProfile(Form("p_tR_vs_posY_BAR%d",iBar),Form("p_tR_vs_posY_BAR%d",iBar), 100, ymin, ymax,dtminR[iBar], dtmaxR[iBar]);
    p_tR_vs_tDiff[iBar]         = new TProfile(Form("p_tR_vs_tDiff_BAR%d",iBar),Form("p_tR_vs_tDiff_BAR%d",iBar), 100, -4, 4,dtminR[iBar], dtmaxR[iBar]);
    p_tR_vs_pulseInt[iBar]      = new TProfile(Form("p_tR_vs_pulseInt_BAR%d",iBar),Form("p_tR_vs_pulseInt_BAR%d",iBar), nAmpBins*2, 0., 250.,dtminR[iBar], dtmaxR[iBar]);
    p2_tR_vs_amp_vs_posXc[iBar]  = new TProfile2D(Form("p2_tR_vs_amp_vs_posXc_BAR%d",iBar),Form("p2_tR_vs_amp_vs_posXc_BAR%d",iBar), nAmpBins, 0., 1., 100, xmin/cos(theta[iBar]), xmax/cos(theta[iBar]), dtminR[iBar], dtmaxR[iBar]);

    h_tR_ampCorr[iBar]          = new TH1F(Form("h_tR_ampCorr_BAR%d",iBar),Form("h_tR_ampCorr_BAR%d",iBar), nTimeBins, dtminR[iBar], dtmaxR[iBar]);
    p_tR_ampCorr_vs_amp[iBar]   = new TProfile(Form("p_tR_ampCorr_vs_amp_BAR%d",iBar),Form("p_tR_ampCorr_vs_amp_BAR%d",iBar), nAmpBins, 0., 1., dtminR[iBar], dtmaxR[iBar]);
    p_tR_ampCorr_vs_posX[iBar]  = new TProfile(Form("p_tR_ampCorr_vs_posX_BAR%d",iBar),Form("p_tR_ampCorr_vs_posX_BAR%d",iBar), 100, xmin, xmax, dtminR[iBar], dtmaxR[iBar]);
    p_tR_ampCorr_vs_posY[iBar]  = new TProfile(Form("p_tR_ampCorr_vs_posY_BAR%d",iBar),Form("p_tR_ampCorr_vs_posY_BAR%d",iBar), 100, ymin, ymax, dtminR[iBar], dtmaxR[iBar]);
    p_tR_ampCorr_vs_tDiff[iBar] = new TProfile(Form("p_tR_ampCorr_vs_tDiff_BAR%d",iBar),Form("p_tR_ampCorr_vs_tDiff_BAR%d",iBar), 100, -4, 4, dtminR[iBar], dtmaxR[iBar]);
    p_tR_ampCorr_vs_posXc[iBar] = new TProfile(Form("p_tR_ampCorr_vs_posXc_BAR%d",iBar),Form("p_tR_ampCorr_vs_posXc_BAR%d",iBar), 100, xmin/cos(theta[iBar]), xmax/cos(theta[iBar]), dtminR[iBar], dtmaxR[iBar]);

    h_tR_pulseIntCorr[iBar]          = new TH1F(Form("h_tR_pulseIntCorr_BAR%d",iBar),Form("h_tR_pulseIntCorr_BAR%d",iBar), nTimeBins, dtminR[iBar], dtmaxR[iBar]);
    p_tR_pulseIntCorr_vs_pulseInt[iBar]   = new TProfile(Form("p_tR_pulseIntCorr_vs_pulseInt_BAR%d",iBar),Form("p_tR_pulseIntCorr_vs_pulseInt_BAR%d",iBar), nAmpBins*2, 0., 250.,dtminR[iBar], dtmaxR[iBar]);
    p_tR_pulseIntCorr_vs_posX[iBar]  = new TProfile(Form("p_tR_pulseIntCorr_vs_posX_BAR%d",iBar),Form("p_tR_pulseIntCorr_vs_posX_BAR%d",iBar), 100, xmin, xmax, dtminR[iBar], dtmaxR[iBar]);
    p_tR_pulseIntCorr_vs_posY[iBar]  = new TProfile(Form("p_tR_pulseIntCorr_vs_posY_BAR%d",iBar),Form("p_tR_pulseIntCorr_vs_posY_BAR%d",iBar), 100, ymin, ymax, dtminR[iBar], dtmaxR[iBar]);
    p_tR_pulseIntCorr_vs_tDiff[iBar] = new TProfile(Form("p_tR_pulseIntCorr_vs_tDiff_BAR%d",iBar),Form("p_tR_pulseIntCorr_vs_tDiff_BAR%d",iBar), 100, -4, 4, dtminR[iBar], dtmaxR[iBar]);
    p_tR_pulseIntCorr_vs_posXc[iBar] = new TProfile(Form("p_tR_pulseIntCorr_vs_posXc_BAR%d",iBar),Form("p_tR_pulseIntCorr_vs_posXc_BAR%d",iBar), 100, xmin/cos(theta[iBar]), xmax/cos(theta[iBar]), dtminR[iBar], dtmaxR[iBar]);

    // -- tDiff
    h_tDiff[iBar]                  = new TH1F(Form("h_tDiff_BAR%d",iBar),Form("h_tDiff_BAR%d",iBar),nTimeBins/2, -4, 4);
    p_tDiff_vs_posX[iBar]    = new TProfile(Form("p_tDiff_vs_posX_BAR%d",iBar),Form("p_tDiff_vs_posX_BAR%d",iBar), 100, xmin, xmax, -4, 4);
    p_tDiff_vs_posXc[iBar]   = new TProfile(Form("p_tDiff_vs_posXc_BAR%d",iBar),Form("p_tDiff_vs_posXc_BAR%d",iBar), 100, xmin/cos(theta[iBar]), xmax/cos(theta[iBar]), -4, 4);
    h_tDiff_posCorr[iBar]          = new TH1F(Form("h_tDiff_posCorr_BAR%d",iBar),Form("h_tDiff_posCorr_BAR%d",iBar),nTimeBins/2, -4, 4);
    p_tDiff_posCorr_vs_posX[iBar]  = new TProfile(Form("p_tDiff_posCorr_vs_posX_BAR%d",iBar),Form("p_tDiff_posCorr_vs_posX_BAR%d",iBar), 100, xmin, xmax, -4, 4);
    p_tDiff_posCorr_vs_posXc[iBar] = new TProfile(Form("p_tDiff_posCorr_vs_posXc_BAR%d",iBar),Form("p_tDiff_posCorr_vs_posXc_BAR%d",iBar), 200, xmin/cos(theta[iBar]), xmax/cos(theta[iBar]), -4, 4);
    h_tDiff_ampCorr[iBar]          = new TH1F(Form("h_tDiff_ampCorr_BAR%d",iBar),Form("h_tDiff_ampCorr_BAR%d",iBar),nTimeBins/2, -4, 4);
    h_tDiff_ampCorr_posCorr[iBar]  = new TH1F(Form("h_tDiff_ampCorr_posCorr_BAR%d",iBar),Form("h_tDiff_ampCorr_posCorr_BAR%d",iBar),nTimeBins/2, -4, 4);
    p_tDiff_ampCorr_vs_posX[iBar]  = new TProfile(Form("p_tDiff_ampCorr_vs_posX_BAR%d",iBar),Form("p_tDiff_ampCorr_vs_posX_BAR%d",iBar), 100, xmin, xmax, -4, 4);
    p_tDiff_ampCorr_vs_posXc[iBar] = new TProfile(Form("p_tDiff_ampCorr_vs_posXc_BAR%d",iBar),Form("p_tDiff_ampCorr_vs_posXc_BAR%d",iBar), 100, xmin/cos(theta[iBar]), xmax/cos(theta[iBar]), -4, 4);
    p_tDiff_ampCorr_posCorr_vs_posX[iBar]  = new TProfile(Form("p_tDiff_ampCorr_posCorr_vs_posX_BAR%d",iBar),Form("p_tDiff_ampCorr_posCorr_vs_posX_BAR%d",iBar), 100, xmin, xmax, -4, 4);
    p_tDiff_ampCorr_posCorr_vs_posXc[iBar] = new TProfile(Form("p_tDiff_ampCorr_posCorr_vs_posXc_BAR%d",iBar),Form("p_tDiff_ampCorr_posCorr_vs_posXc_BAR%d",iBar), 200, xmin/cos(theta[iBar]), xmax/cos(theta[iBar]), -4, 4);
    p_tDiff_ampCorr_posCorr_vs_posY[iBar] = new TProfile(Form("p_tDiff_ampCorr_posCorr_vs_posY_BAR%d",iBar),Form("p_tDiff_ampCorr_posCorr_vs_posY_BAR%d",iBar), 200, ymin, ymax, -4, 4);

    h2_tDiff_vs_posX[iBar] = new TH2F(Form("h2_tDiff_vs_posX_BAR%d",iBar), Form("h2_tDiff_vs_posX_BAR%d",iBar), 100, xmin, xmax, 200, -4, 4);
    h2_tDiff_ampCorr_vs_posX[iBar] = new TH2F(Form("h2_tDiff_ampCorr_vs_posX_BAR%d",iBar), Form("h2_tDiff_ampCorr_vs_posX_BAR%d",iBar), 400, xmin, xmax, 400, -4, 4);

    h_tDiff_pulseIntCorr[iBar]          = new TH1F(Form("h_tDiff_pulseIntCorr_BAR%d",iBar),Form("h_tDiff_pulseIntCorr_BAR%d",iBar),nTimeBins/2, -4, 4);
    p_tDiff_pulseIntCorr_vs_posX[iBar]  = new TProfile(Form("p_tDiff_pulseIntCorr_vs_posX_BAR%d",iBar),Form("p_tDiff_pulseIntCorr_vs_posX_BAR%d",iBar), 100, xmin, xmax, -4, 4);
    p_tDiff_pulseIntCorr_vs_posXc[iBar] = new TProfile(Form("p_tDiff_pulseIntCorr_vs_posXc_BAR%d",iBar),Form("p_tDiff_pulseIntCorr_vs_posXc_BAR%d",iBar), 100, xmin/cos(theta[iBar]), xmax/cos(theta[iBar]), -4, 4);

    h_tDiff_pulseIntCorr_posCorr[iBar]  = new TH1F(Form("h_tDiff_pulseIntCorr_posCorr_BAR%d",iBar),Form("h_tDiff_pulseIntCorr_posCorr_BAR%d",iBar),nTimeBins/2, -4, 4);
    p_tDiff_pulseIntCorr_posCorr_vs_posX[iBar]  = new TProfile(Form("p_tDiff_pulseIntCorr_posCorr_vs_posX_BAR%d",iBar),Form("p_tDiff_pulseIntCorr_posCorr_vs_posX_BAR%d",iBar), 100, xmin, xmax, -4, 4);
    p_tDiff_pulseIntCorr_posCorr_vs_posXc[iBar] = new TProfile(Form("p_tDiff_pulseIntCorr_posCorr_vs_posXc_BAR%d",iBar),Form("p_tDiff_pulseIntCorr_posCorr_vs_posXc_BAR%d",iBar),200, xmin/cos(theta[iBar]), xmax/cos(theta[iBar]), -4, 4);

    // -- tAverage
    p2_tAve_vs_posXY[iBar]        = new TProfile2D(Form("p2_tAve_vs_posXY_BAR%d",iBar),Form("p2_tAve_vs_posXY_BAR%d",iBar), 400, xmin, xmax, 200, ymin, ymax, dtminAve[iBar], dtmaxAve[iBar]);

    h_tAve[iBar]                  = new TH1F(Form("h_tAve_BAR%d",iBar),Form("h_tAve_BAR%d",iBar),nTimeBins, dtminAve[iBar], dtmaxAve[iBar]);
    p_tAve_vs_posX[iBar]          = new TProfile(Form("p_tAve_vs_posX_BAR%d",iBar),Form("p_tAve_vs_posX_BAR%d",iBar), 100, xmin, xmax, dtminAve[iBar], dtmaxAve[iBar]);
    p_tAve_vs_posY[iBar]          = new TProfile(Form("p_tAve_vs_posY_BAR%d",iBar),Form("p_tAve_vs_posY_BAR%d",iBar), 100, ymin, ymax, dtminAve[iBar], dtmaxAve[iBar]);
    p_tAve_vs_tDiff[iBar]         = new TProfile(Form("p_tAve_vs_tDiff_BAR%d",iBar),Form("p_tAve_vs_tDiff_BAR%d",iBar), 100, -4, 4, dtminAve[iBar], dtmaxAve[iBar]);


    // -- plain tAve
    h_tAve_ampCorr[iBar]          = new TH1F(Form("h_tAve_ampCorr_BAR%d",iBar),Form("h_tAve_ampCorr_BAR%d",iBar), nTimeBins, dtminAve[iBar], dtmaxAve[iBar]);
    p_tAve_ampCorr_vs_posX[iBar]  = new TProfile(Form("p_tAve_ampCorr_vs_posX_BAR%d",iBar),Form("p_tAve_ampCorr_vs_posX_BAR%d",iBar), 50, xmin, xmax, dtminAve[iBar], dtmaxAve[iBar]);
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




    // -- plain tAve after correction vs position dependence
    h_tAve_ampCorr_posCorr[iBar]          = new TH1F(Form("h_tAve_ampCorr_posCorr_BAR%d",iBar),Form("h_tAve_ampCorr_posCorr_BAR%d",iBar), nTimeBins, dtminAve[iBar], dtmaxAve[iBar]);
    p_tAve_ampCorr_posCorr_vs_posX[iBar]  = new TProfile(Form("p_tAve_ampCorr_posCorr_vs_posX_BAR%d",iBar),Form("p_tAve_ampCorr_posCorr_vs_posX_BAR%d",iBar), 100, xmin, xmax, dtminAve[iBar], dtmaxAve[iBar]);
    p_tAve_ampCorr_posCorr_vs_posY[iBar]  = new TProfile(Form("p_tAve_ampCorr_posCorr_vs_posY_BAR%d",iBar),Form("p_tAve_ampCorr_posCorr_vs_posY_BAR%d",iBar), 100, ymin, ymax, dtminAve[iBar], dtmaxAve[iBar]);
    p_tAve_ampCorr_posCorr_vs_tDiff[iBar] = new TProfile(Form("p_tAve_ampCorr_posCorr_vs_tDiff_BAR%d",iBar),Form("p_tAve_ampCorr_posCorr_vs_tDiff_BAR%d",iBar), 100, -4, 4, dtminAve[iBar], dtmaxAve[iBar]);
    p_tAve_ampCorr_posCorr_vs_posXc[iBar]  = new TProfile(Form("p_tAve_ampCorr_posCorr_vs_posXc_BAR%d",iBar),Form("p_tAve_ampCorr_posCorr_vs_posXc_BAR%d",iBar), 100, xmin/cos(theta[iBar]), xmax/cos(theta[iBar]), dtminAve[iBar], dtmaxAve[iBar]);
    for (int ibin = 0; ibin < NBINSX[iBar]; ibin++){
      h_tAve_ampCorr_posCorr_binX[iBar][ibin] = new TH1F(Form("h_tAve_ampCorr_posCorr_binX_%d_BAR%d",ibin, iBar),Form("h_tAve_ampCorr_posCorr_binX_%d_BAR%d",ibin,iBar), nbinsHistoX, dtminAve[iBar], dtmaxAve[iBar]);
    }
    for (int ibin = 0; ibin < NBINSY; ibin++){
      h_tAve_ampCorr_posCorr_binY[iBar][ibin] = new TH1F(Form("h_tAve_ampCorr_posCorr_binY_%d_BAR%d",ibin, iBar),Form("h_tAve_ampCorr_posCorr_binY_%d_BAR%d",ibin,iBar), nbinsHistoY, dtminAve[iBar], dtmaxAve[iBar]);
    }



    // -- plain tAve, amp walk corrction vs integral
    h_tAve_pulseIntCorr[iBar]          = new TH1F(Form("h_tAve_pulseIntCorr_BAR%d",iBar),Form("h_tAve_pulseIntCorr_BAR%d",iBar), nTimeBins, dtminAve[iBar], dtmaxAve[iBar]);
    p_tAve_pulseIntCorr_vs_posX[iBar]  = new TProfile(Form("p_tAve_pulseIntCorr_vs_posX_BAR%d",iBar),Form("p_tAve_pulseIntCorr_vs_posX_BAR%d",iBar), 50, xmin, xmax, dtminAve[iBar], dtmaxAve[iBar]);
    p_tAve_pulseIntCorr_vs_posY[iBar]  = new TProfile(Form("p_tAve_pulseIntCorr_vs_posY_BAR%d",iBar),Form("p_tAve_pulseIntCorr_vs_posY_BAR%d",iBar), 100, ymin, ymax, dtminAve[iBar], dtmaxAve[iBar]);
    p_tAve_pulseIntCorr_vs_tDiff[iBar] = new TProfile(Form("p_tAve_pulseIntCorr_vs_tDiff_BAR%d",iBar),Form("p_tAve_pulseIntCorr_vs_tDiff_BAR%d",iBar), 100, -4, 4, dtminAve[iBar], dtmaxAve[iBar]);
    p_tAve_pulseIntCorr_vs_posXc[iBar]  = new TProfile(Form("p_tAve_pulseIntCorr_vs_posXc_BAR%d",iBar),Form("p_tAve_pulseIntCorr_vs_posXc_BAR%d",iBar), 100, xmin/cos(theta[iBar]), xmax/cos(theta[iBar]), dtminAve[iBar], dtmaxAve[iBar]);
    for (int ibin = 0; ibin < NBINSX[iBar]; ibin++){
      h_tL_pulseIntCorr_binX[iBar][ibin] = new TH1F(Form("h_tL_pulseIntCorr_binX_%d_BAR%d",ibin, iBar),Form("h_tL_pulseIntCorr_binX_%d_BAR%d",ibin,iBar), nbinsHistoX, dtminL[iBar], dtmaxL[iBar]);
      h_tR_pulseIntCorr_binX[iBar][ibin] = new TH1F(Form("h_tR_pulseIntCorr_binX_%d_BAR%d",ibin, iBar),Form("h_tR_pulseIntCorr_binX_%d_BAR%d",ibin,iBar), nbinsHistoX, dtminR[iBar], dtmaxR[iBar]);
      h_tAve_pulseIntCorr_binX[iBar][ibin] = new TH1F(Form("h_tAve_pulseIntCorr_binX_%d_BAR%d",ibin, iBar),Form("h_tAve_pulseIntCorr_binX_%d_BAR%d",ibin,iBar), nbinsHistoX, dtminAve[iBar], dtmaxAve[iBar]);
    }
    for (int ibin = 0; ibin < NBINSY; ibin++){
      h_tAve_pulseIntCorr_binY[iBar][ibin] = new TH1F(Form("h_tAve_pulseIntCorr_binY_%d_BAR%d",ibin, iBar),Form("h_tAve_pulseIntCorr_binY_%d_BAR%d",ibin,iBar), nbinsHistoY, dtminAve[iBar], dtmaxAve[iBar]);
    }




    // -- plain tAve after correction vs integral, position dependence
    h_tAve_pulseIntCorr_posCorr[iBar]          = new TH1F(Form("h_tAve_pulseIntCorr_posCorr_BAR%d",iBar),Form("h_tAve_pulseIntCorr_posCorr_BAR%d",iBar), nTimeBins, dtminAve[iBar], dtmaxAve[iBar]);
    p_tAve_pulseIntCorr_posCorr_vs_posX[iBar]  = new TProfile(Form("p_tAve_pulseIntCorr_posCorr_vs_posX_BAR%d",iBar),Form("p_tAve_pulseIntCorr_posCorr_vs_posX_BAR%d",iBar), 100,xmin, xmax, dtminAve[iBar], dtmaxAve[iBar]);
    p_tAve_pulseIntCorr_posCorr_vs_posY[iBar]  = new TProfile(Form("p_tAve_pulseIntCorr_posCorr_vs_posY_BAR%d",iBar),Form("p_tAve_pulseIntCorr_posCorr_vs_posY_BAR%d",iBar), 100,ymin, ymax, dtminAve[iBar], dtmaxAve[iBar]);
    p_tAve_pulseIntCorr_posCorr_vs_tDiff[iBar] = new TProfile(Form("p_tAve_pulseIntCorr_posCorr_vs_tDiff_BAR%d",iBar),Form("p_tAve_pulseIntCorr_posCorr_vs_tDiff_BAR%d",iBar), 100, -4, 4, dtminAve[iBar], dtmaxAve[iBar]);
    p_tAve_pulseIntCorr_posCorr_vs_posXc[iBar]  = new TProfile(Form("p_tAve_pulseIntCorr_posCorr_vs_posXc_BAR%d",iBar),Form("p_tAve_pulseIntCorr_posCorr_vs_posXc_BAR%d",iBar), 100, xmin/cos(theta[iBar]), xmax/cos(theta[iBar]), dtminAve[iBar], dtmaxAve[iBar]);
    for (int ibin = 0; ibin < NBINSX[iBar]; ibin++){
      h_tAve_pulseIntCorr_posCorr_binX[iBar][ibin] = new TH1F(Form("h_tAve_pulseIntCorr_posCorr_binX_%d_BAR%d",ibin, iBar),Form("h_tAve_pulseIntCorr_posCorr_binX_%d_BAR%d",ibin,iBar), nbinsHistoX, dtminAve[iBar], dtmaxAve[iBar]);
    }
    for (int ibin = 0; ibin < NBINSY; ibin++){
      h_tAve_pulseIntCorr_posCorr_binY[iBar][ibin] = new TH1F(Form("h_tAve_pulseIntCorr_posCorr_binY_%d_BAR%d",ibin, iBar),Form("h_tAve_pulseIntCorr_posCorr_binY_%d_BAR%d",ibin,iBar), nbinsHistoY, dtminAve[iBar], dtmaxAve[iBar]);
    }

    // -- tDiff 
    for (int ibin = 0; ibin < NBINSX[iBar]; ibin++){
      h_tDiff_binX[iBar][ibin] = new TH1F(Form("h_tDiff_binX_%d_BAR%d",ibin, iBar),Form("h_tDiff_binX_%d_BAR%d",ibin,iBar), int(nbinsHistoX/2), -4, 4);
      h_tDiff_posCorr_binX[iBar][ibin] = new TH1F(Form("h_tDiff_posCorr_binX_%d_BAR%d",ibin, iBar),Form("h_tDiff_posCorr_binX_%d_BAR%d",ibin,iBar), int(nbinsHistoX/2), -4, 4);
      h_tDiff_ampCorr_binX[iBar][ibin] = new TH1F(Form("h_tDiff_ampCorr_binX_%d_BAR%d",ibin, iBar),Form("h_tDiff_ampCorr_binX_%d_BAR%d",ibin,iBar), int(nbinsHistoX/2), -4, 4);
      h_tDiff_ampCorr_posCorr_binX[iBar][ibin] = new TH1F(Form("h_tDiff_ampCorr_posCorr_binX_%d_BAR%d",ibin, iBar),Form("h_tDiff_ampCorr_posCorr_binX_%d_BAR%d",ibin,iBar), int(nbinsHistoX/2), -4, 4);
      h_tDiff_pulseIntCorr_binX[iBar][ibin] = new TH1F(Form("h_tDiff_pulseIntCorr_binX_%d_BAR%d",ibin, iBar),Form("h_tDiff_pulseIntCorr_binX_%d_BAR%d",ibin,iBar), int(nbinsHistoX/2), -4, 4);
      h_tDiff_pulseIntCorr_posCorr_binX[iBar][ibin] = new TH1F(Form("h_tDiff_pulseIntCorr_posCorr_binX_%d_BAR%d",ibin, iBar),Form("h_tDiff_pulseIntCorr_posCorr_binX_%d_BAR%d",ibin,iBar), int(nbinsHistoX/2), -4, 4);
    }

    for (int ibin = 0; ibin < NBINSY; ibin++){
      h_tDiff_binY[iBar][ibin] = new TH1F(Form("h_tDiff_binY_%d_BAR%d",ibin, iBar),Form("h_tDiff_binY_%d_BAR%d",ibin,iBar), int(nbinsHistoY/2), -5, 5);
      h_tDiff_posCorr_binY[iBar][ibin] = new TH1F(Form("h_tDiff_posCorr_binY_%d_BAR%d",ibin, iBar),Form("h_tDiff_posCorr_binY_%d_BAR%d",ibin,iBar), int(nbinsHistoY/2), -5, 5);
      h_tDiff_ampCorr_binY[iBar][ibin] = new TH1F(Form("h_tDiff_ampCorr_binY_%d_BAR%d",ibin, iBar),Form("h_tDiff_ampCorr_binY_%d_BAR%d",ibin,iBar), int(nbinsHistoY/2), -5, 5);
      h_tDiff_ampCorr_posCorr_binY[iBar][ibin] = new TH1F(Form("h_tDiff_ampCorr_posCorr_binY_%d_BAR%d",ibin, iBar),Form("h_tDiff_ampCorr_posCorr_binY_%d_BAR%d",ibin,iBar), int(nbinsHistoY/2), -5, 5);
      h_tDiff_pulseIntCorr_binY[iBar][ibin] = new TH1F(Form("h_tDiff_pulseIntCorr_binY_%d_BAR%d",ibin, iBar),Form("h_tDiff_pulseIntCorr_binY_%d_BAR%d",ibin,iBar), int(nbinsHistoY/2), -5, 5);
      h_tDiff_pulseIntCorr_posCorr_binY[iBar][ibin] = new TH1F(Form("h_tDiff_pulseIntCorr_posCorr_binY_%d_BAR%d",ibin, iBar),Form("h_tDiff_pulseIntCorr_posCorr_binY_%d_BAR%d",ibin,iBar), int(nbinsHistoY/2), -5, 5);
    }

    for (int ibin = 0; ibin < 100; ibin++){
      h_tDiff_pulseIntCorr_posCorr_binAmp[iBar][ibin] = new TH1F(Form("h_tDiff_pulseIntCorr_posCorr_binAmp_%d_BAR%d",ibin, iBar),Form("h_tDiff_pulseIntCorr_posCorr_binAmp_%d_BAR%d",ibin,iBar), int(nbinsHistoY/5), -5, 5);
    }
                                                                                                            
    // binned amp walk corr                                                                                                                     
    for (int ibin = 0; ibin < NBINSXAMPWALK[iBar]; ibin++){
      p_tL_vs_amp_binned[iBar][ibin] = new TProfile(Form("p_tL_vs_amp_binXc_%d_BAR%d",ibin,iBar),Form("p_tL_vs_amp_binXc_%d_BAR%d",ibin,iBar), nAmpBins, 0., 1.,dtminL[iBar], dtmaxL[iBar]);
      p_tR_vs_amp_binned[iBar][ibin] = new TProfile(Form("p_tR_vs_amp_binXc_%d_BAR%d",ibin,iBar),Form("p_tR_vs_amp_binXc_%d_BAR%d",ibin,iBar),  nAmpBins, 0., 1.,dtminR[iBar], dtmaxR[iBar]);
      p_tL_ampCorr_vs_amp_binned[iBar][ibin] = new TProfile(Form("p_tL_ampCorr_vs_amp_binXc_%d_BAR%d",ibin,iBar),Form("p_tL_ampCorr_vs_amp_binXc_%d_BAR%d",ibin,iBar), nAmpBins, 0., 1.,dtminL[iBar], dtmaxL[iBar]);
      p_tR_ampCorr_vs_amp_binned[iBar][ibin] = new TProfile(Form("p_tR_ampCorr_vs_amp_binXc_%d_BAR%d",ibin,iBar),Form("p_tR_ampCorr_vs_amp_binXc_%d_BAR%d",ibin,iBar), nAmpBins, 0., 1.,dtminR[iBar], dtmaxR[iBar]);
   
      p_tL_vs_pulseInt_binned[iBar][ibin] = new TProfile(Form("p_tL_vs_pulseInt_binXc_%d_BAR%d",ibin,iBar),Form("p_tL_vs_pulseInt_binXc_%d_BAR%d",ibin,iBar), nAmpBins*2, 0., 250., dtminL[iBar], dtmaxL[iBar]);
      p_tR_vs_pulseInt_binned[iBar][ibin] = new TProfile(Form("p_tR_vs_pulseInt_binXc_%d_BAR%d",ibin,iBar),Form("p_tR_vs_pulseInt_binXc_%d_BAR%d",ibin,iBar), nAmpBins*2, 0., 250., dtminR[iBar], dtmaxR[iBar]);

    }

  }



  // -- combination of the two bars ---
  float dtminAveSum   = -20;
  float dtmaxAveSum   = 20;

  TH2F *h2_tAve1_vs_tAve2 = new TH2F("h2_tAve1_vs_tAve2","h2_tAve1_vs_tAve2", 200, -1, 1, 200, -1, 1);
  TH2F *h2_amp1_vs_amp2 = new TH2F("h2_amp1_vs_amp2","h2_amp1_vs_amp2", 200, 0, 1, 200, 0, 1);

  TGraphErrors *g_nEventsRatio_vs_posY = new TGraphErrors();
  g_nEventsRatio_vs_posY->SetName("g_nEventsRatio_vs_posY");

  TH2F *h2_ampRatio = new TH2F("h2_ampRatio","h2_ampRatio", 1000, -50, 50, 1000, -2, 2);


  // -- simple average 

  // - amp corr
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

  TH1F*          h_tAveSum_ampCorr_posCorr = new TH1F("h_tAveSum_ampCorr_posCorr","h_tAveSum_ampCorr_posCorr",1000, dtminAveSum, dtmaxAveSum);
  TProfile*      p_tAveSum_ampCorr_posCorr_vs_posX  = new TProfile("p_tAveSum_ampCorr_posCorr_vs_posX","p_tAveSum_ampCorr_posCorr_vs_posX", 400, xmin, xmax, dtminAveSum, dtmaxAveSum);
  TProfile*      p_tAveSum_ampCorr_posCorr_vs_posY  = new TProfile("p_tAveSum_ampCorr_posCorr_vs_posY","p_tAveSum_ampCorr_posCorr_vs_posY", 200, ymin, ymax, dtminAveSum, dtmaxAveSum);
  TH1F*          h_tAveSum_ampCorr_posCorr_binX[1000];
  for (int ibin = 0; ibin < NBINSX[0]; ibin++){
    h_tAveSum_ampCorr_posCorr_binX[ibin] = new TH1F(Form("h_tAveSum_ampCorr_posCorr_binX_%d",ibin),Form("h_tAveSum_ampCorr_posCorr_binX_%d",ibin), nbinsHistoX, dtminAveSum, dtmaxAveSum);
  }
  TH1F*          h_tAveSum_ampCorr_posCorr_binY[1000] ;
  for (int ibin = 0; ibin < NBINSY; ibin++){
    h_tAveSum_ampCorr_posCorr_binY[ibin] = new TH1F(Form("h_tAveSum_ampCorr_posCorr_binY_%d",ibin),Form("h_tAveSum_ampCorr_posCorr_binY_%d",ibin), nbinsHistoY, dtminAveSum, dtmaxAveSum);
  }

  // -- pulse integral corr
  TH1F*          h_tAveSum_pulseIntCorr = new TH1F("h_tAveSum_pulseIntCorr","h_tAveSum_pulseIntCorr",1000, dtminAveSum, dtmaxAveSum);
  TProfile*      p_tAveSum_pulseIntCorr_vs_posX  = new TProfile("p_tAveSum_pulseIntCorr_vs_posX","p_tAveSum_pulseIntCorr_vs_posX", 400, xmin, xmax, dtminAveSum, dtmaxAveSum);
  TProfile*      p_tAveSum_pulseIntCorr_vs_posY  = new TProfile("p_tAveSum_pulseIntCorr_vs_posY","p_tAveSum_pulseIntCorr_vs_posY", 200, ymin, ymax, dtminAveSum, dtmaxAveSum);
  TH1F*          h_tAveSum_pulseIntCorr_binX[1000];
  for (int ibin = 0; ibin < NBINSX[0]; ibin++){
    h_tAveSum_pulseIntCorr_binX[ibin] = new TH1F(Form("h_tAveSum_pulseIntCorr_binX_%d",ibin),Form("h_tAveSum_pulseIntCorr_binX_%d",ibin), nbinsHistoX, dtminAveSum, dtmaxAveSum);
  }
  TH1F*          h_tAveSum_pulseIntCorr_binY[1000] ;
  for (int ibin = 0; ibin < NBINSY; ibin++){
    h_tAveSum_pulseIntCorr_binY[ibin] = new TH1F(Form("h_tAveSum_pulseIntCorr_binY_%d",ibin),Form("h_tAveSum_pulseIntCorr_binY_%d",ibin), nbinsHistoY, dtminAveSum, dtmaxAveSum);
  }

  TH1F*          h_tAveSum_pulseIntCorr_posCorr = new TH1F("h_tAveSum_pulseIntCorr_posCorr","h_tAveSum_pulseIntCorr_posCorr",1000, dtminAveSum, dtmaxAveSum);
  TProfile*      p_tAveSum_pulseIntCorr_posCorr_vs_posX  = new TProfile("p_tAveSum_pulseIntCorr_posCorr_vs_posX","p_tAveSum_pulseIntCorr_posCorr_vs_posX", 400, xmin, xmax, dtminAveSum, dtmaxAveSum);
  TProfile*      p_tAveSum_pulseIntCorr_posCorr_vs_posY  = new TProfile("p_tAveSum_pulseIntCorr_posCorr_vs_posY","p_tAveSum_pulseIntCorr_posCorr_vs_posY", 200, ymin, ymax, dtminAveSum, dtmaxAveSum);
  TH1F*          h_tAveSum_pulseIntCorr_posCorr_binX[1000];
  for (int ibin = 0; ibin < NBINSX[0]; ibin++){
    h_tAveSum_pulseIntCorr_posCorr_binX[ibin] = new TH1F(Form("h_tAveSum_pulseIntCorr_posCorr_binX_%d",ibin),Form("h_tAveSum_pulseIntCorr_posCorr_binX_%d",ibin), nbinsHistoX, dtminAveSum, dtmaxAveSum);
  }
  TH1F*          h_tAveSum_pulseIntCorr_posCorr_binY[1000] ;
  for (int ibin = 0; ibin < NBINSY; ibin++){
    h_tAveSum_pulseIntCorr_posCorr_binY[ibin] = new TH1F(Form("h_tAveSum_pulseIntCorr_posCorr_binY_%d",ibin),Form("h_tAveSum_pulseIntCorr_posCorr_binY_%d",ibin), nbinsHistoY, dtminAveSum, dtmaxAveSum);
  }



  // -- amp weighted average
  // - amp corr
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


  TH1F*          h_tAveAmpWSum_ampCorr_posCorr = new TH1F("h_tAveAmpWSum_ampCorr_posCorr","h_tAveAmpWSum_ampCorr_posCorr",1000, dtminAveSum, dtmaxAveSum);
  TProfile*      p_tAveAmpWSum_ampCorr_posCorr_vs_posX  = new TProfile("p_tAveAmpWSum_ampCorr_posCorr_vs_posX","p_tAveAmpWSum_ampCorr_posCorr_vs_posX", 400, xmin, xmax, dtminAveSum, dtmaxAveSum);
  TProfile*      p_tAveAmpWSum_ampCorr_posCorr_vs_posY  = new TProfile("p_tAveAmpWSum_ampCorr_posCorr_vs_posY","p_tAveAmpWSum_ampCorr_posCorr_vs_posY", 200, ymin, ymax, dtminAveSum, dtmaxAveSum);
  TH1F*          h_tAveAmpWSum_ampCorr_posCorr_binX[1000];
  for (int ibin = 0; ibin < NBINSX[0]; ibin++){
    h_tAveAmpWSum_ampCorr_posCorr_binX[ibin] = new TH1F(Form("h_tAveAmpWSum_ampCorr_posCorr_binX_%d",ibin),Form("h_tAveAmpWSum_ampCorr_posCorr_binX_%d",ibin), nbinsHistoX, dtminAveSum, dtmaxAveSum);
  }
  TH1F*          h_tAveAmpWSum_ampCorr_posCorr_binY[1000] ;
  for (int ibin = 0; ibin < NBINSY; ibin++){
    h_tAveAmpWSum_ampCorr_posCorr_binY[ibin] = new TH1F(Form("h_tAveAmpWSum_ampCorr_posCorr_binY_%d",ibin),Form("h_tAveAmpWSum_ampCorr_posCorr_binY_%d",ibin), nbinsHistoY, dtminAveSum, dtmaxAveSum);
  }

  // - pulse int corr
  TH1F*          h_tAveAmpWSum_pulseIntCorr = new TH1F("h_tAveAmpWSum_pulseIntCorr","h_tAveAmpWSum_pulseIntCorr",1000, dtminAveSum, dtmaxAveSum);
  TProfile*      p_tAveAmpWSum_pulseIntCorr_vs_posX  = new TProfile("p_tAveAmpWSum_pulseIntCorr_vs_posX","p_tAveAmpWSum_pulseIntCorr_vs_posX", 400, xmin, xmax, dtminAveSum, dtmaxAveSum);
  TProfile*      p_tAveAmpWSum_pulseIntCorr_vs_posY  = new TProfile("p_tAveAmpWSum_pulseIntCorr_vs_posY","p_tAveAmpWSum_pulseIntCorr_vs_posY", 200, ymin, ymax, dtminAveSum, dtmaxAveSum);
  TH1F*          h_tAveAmpWSum_pulseIntCorr_binX[1000];
  for (int ibin = 0; ibin < NBINSX[0]; ibin++){
    h_tAveAmpWSum_pulseIntCorr_binX[ibin] = new TH1F(Form("h_tAveAmpWSum_pulseIntCorr_binX_%d",ibin),Form("h_tAveAmpWSum_pulseIntCorr_binX_%d",ibin), nbinsHistoX, dtminAveSum, dtmaxAveSum);
  }
  TH1F*          h_tAveAmpWSum_pulseIntCorr_binY[1000] ;
  for (int ibin = 0; ibin < NBINSY; ibin++){
    h_tAveAmpWSum_pulseIntCorr_binY[ibin] = new TH1F(Form("h_tAveAmpWSum_pulseIntCorr_binY_%d",ibin),Form("h_tAveAmpWSum_pulseIntCorr_binY_%d",ibin), nbinsHistoY, dtminAveSum, dtmaxAveSum);
  }


  TH1F*          h_tAveAmpWSum_pulseIntCorr_posCorr = new TH1F("h_tAveAmpWSum_pulseIntCorr_posCorr","h_tAveAmpWSum_pulseIntCorr_posCorr",1000, dtminAveSum, dtmaxAveSum);
  TProfile*      p_tAveAmpWSum_pulseIntCorr_posCorr_vs_posX  = new TProfile("p_tAveAmpWSum_pulseIntCorr_posCorr_vs_posX","p_tAveAmpWSum_pulseIntCorr_posCorr_vs_posX", 400, xmin, xmax, dtminAveSum, dtmaxAveSum);
  TProfile*      p_tAveAmpWSum_pulseIntCorr_posCorr_vs_posY  = new TProfile("p_tAveAmpWSum_pulseIntCorr_posCorr_vs_posY","p_tAveAmpWSum_pulseIntCorr_posCorr_vs_posY", 200, ymin, ymax, dtminAveSum, dtmaxAveSum);
  TH1F*          h_tAveAmpWSum_pulseIntCorr_posCorr_binX[1000];
  for (int ibin = 0; ibin < NBINSX[0]; ibin++){
    h_tAveAmpWSum_pulseIntCorr_posCorr_binX[ibin] = new TH1F(Form("h_tAveAmpWSum_pulseIntCorr_posCorr_binX_%d",ibin),Form("h_tAveAmpWSum_pulseIntCorr_posCorr_binX_%d",ibin), nbinsHistoX, dtminAveSum, dtmaxAveSum);
  }
  TH1F*          h_tAveAmpWSum_pulseIntCorr_posCorr_binY[1000] ;
  for (int ibin = 0; ibin < NBINSY; ibin++){
    h_tAveAmpWSum_pulseIntCorr_posCorr_binY[ibin] = new TH1F(Form("h_tAveAmpWSum_pulseIntCorr_posCorr_binY_%d",ibin),Form("h_tAveAmpWSum_pulseIntCorr_posCorr_binY_%d",ibin), nbinsHistoY, dtminAveSum, dtmaxAveSum);
  }


  // -- time resolution weighted average
  // - amp corr
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


  TH1F*          h_tAveResWSum_ampCorr_posCorr = new TH1F("h_tAveResWSum_ampCorr_posCorr","h_tAveResWSum_ampCorr_posCorr",1000, dtminAveSum, dtmaxAveSum);
  TProfile*      p_tAveResWSum_ampCorr_posCorr_vs_posX  = new TProfile("p_tAveResWSum_ampCorr_posCorr_vs_posX","p_tAveResWSum_ampCorr_posCorr_vs_posX", 400, xmin, xmax, dtminAveSum, dtmaxAveSum);
  TProfile*      p_tAveResWSum_ampCorr_posCorr_vs_posY  = new TProfile("p_tAveResWSum_ampCorr_posCorr_vs_posY","p_tAveResWSum_ampCorr_posCorr_vs_posY", 200, ymin, ymax, dtminAveSum, dtmaxAveSum);
  TH1F*          h_tAveResWSum_ampCorr_posCorr_binX[1000];
  for (int ibin = 0; ibin < NBINSX[0]; ibin++){
    h_tAveResWSum_ampCorr_posCorr_binX[ibin] = new TH1F(Form("h_tAveResWSum_ampCorr_posCorr_binX_%d",ibin),Form("h_tAveResWSum_ampCorr_posCorr_binX_%d",ibin), nbinsHistoX, dtminAveSum, dtmaxAveSum);
  }
  TH1F*          h_tAveResWSum_ampCorr_posCorr_binY[1000] ;
  for (int ibin = 0; ibin < NBINSY; ibin++){
    h_tAveResWSum_ampCorr_posCorr_binY[ibin] = new TH1F(Form("h_tAveResWSum_ampCorr_posCorr_binY_%d",ibin),Form("h_tAveResWSum_ampCorr_posCorr_binY_%d",ibin), nbinsHistoY, dtminAveSum, dtmaxAveSum);
  }


  // -- pulse int corr
  TH1F*          h_tAveResWSum_pulseIntCorr = new TH1F("h_tAveResWSum_pulseIntCorr","h_tAveResWSum_pulseIntCorr",1000, dtminAveSum, dtmaxAveSum);
  TProfile*      p_tAveResWSum_pulseIntCorr_vs_posX  = new TProfile("p_tAveResWSum_pulseIntCorr_vs_posX","p_tAveResWSum_pulseIntCorr_vs_posX", 400, xmin, xmax, dtminAveSum, dtmaxAveSum);
  TProfile*      p_tAveResWSum_pulseIntCorr_vs_posY  = new TProfile("p_tAveResWSum_pulseIntCorr_vs_posY","p_tAveResWSum_pulseIntCorr_vs_posY", 200, ymin, ymax, dtminAveSum, dtmaxAveSum);
  TH1F*          h_tAveResWSum_pulseIntCorr_binX[1000];
  for (int ibin = 0; ibin < NBINSX[0]; ibin++){
    h_tAveResWSum_pulseIntCorr_binX[ibin] = new TH1F(Form("h_tAveResWSum_pulseIntCorr_binX_%d",ibin),Form("h_tAveResWSum_pulseIntCorr_binX_%d",ibin), nbinsHistoX, dtminAveSum, dtmaxAveSum);
  }
  TH1F*          h_tAveResWSum_pulseIntCorr_binY[1000] ;
  for (int ibin = 0; ibin < NBINSY; ibin++){
    h_tAveResWSum_pulseIntCorr_binY[ibin] = new TH1F(Form("h_tAveResWSum_pulseIntCorr_binY_%d",ibin),Form("h_tAveResWSum_pulseIntCorr_binY_%d",ibin), nbinsHistoY, dtminAveSum, dtmaxAveSum);
  }

  TH1F*          h_tAveResWSum_pulseIntCorr_posCorr = new TH1F("h_tAveResWSum_pulseIntCorr_posCorr","h_tAveResWSum_pulseIntCorr_posCorr",1000, dtminAveSum, dtmaxAveSum);
  TProfile*      p_tAveResWSum_pulseIntCorr_posCorr_vs_posX  = new TProfile("p_tAveResWSum_pulseIntCorr_posCorr_vs_posX","p_tAveResWSum_pulseIntCorr_posCorr_vs_posX", 400, xmin, xmax, dtminAveSum, dtmaxAveSum);
  TProfile*      p_tAveResWSum_pulseIntCorr_posCorr_vs_posY  = new TProfile("p_tAveResWSum_pulseIntCorr_posCorr_vs_posY","p_tAveResWSum_pulseIntCorr_posCorr_vs_posY", 200, ymin, ymax, dtminAveSum, dtmaxAveSum);
  TH1F*          h_tAveResWSum_pulseIntCorr_posCorr_binX[1000];
  for (int ibin = 0; ibin < NBINSX[0]; ibin++){
    h_tAveResWSum_pulseIntCorr_posCorr_binX[ibin] = new TH1F(Form("h_tAveResWSum_pulseIntCorr_posCorr_binX_%d",ibin),Form("h_tAveResWSum_pulseIntCorr_posCorr_binX_%d",ibin), nbinsHistoX, dtminAveSum, dtmaxAveSum);
  }
  TH1F*          h_tAveResWSum_pulseIntCorr_posCorr_binY[1000] ;
  for (int ibin = 0; ibin < NBINSY; ibin++){
    h_tAveResWSum_pulseIntCorr_posCorr_binY[ibin] = new TH1F(Form("h_tAveResWSum_pulseIntCorr_posCorr_binY_%d",ibin),Form("h_tAveResWSum_pulseIntCorr_posCorr_binY_%d",ibin), nbinsHistoY, dtminAveSum, dtmaxAveSum);
  }




  // -- tDiff between two bars Ave
  // - amp corr
  TH1F*          h_tAveDiff_ampCorr = new TH1F("h_tAveDiff_ampCorr","h_tAveDiff_ampCorr",1000, dtminAveSum, dtmaxAveSum);
  TProfile*      p_tAveDiff_ampCorr_vs_posX  = new TProfile("p_tAveDiff_ampCorr_vs_posX","p_tAveDiff_ampCorr_vs_posX", 400, xmin, xmax, dtminAveSum, dtmaxAveSum);
  TProfile*      p_tAveDiff_ampCorr_vs_posY  = new TProfile("p_tAveDiff_ampCorr_vs_posY","p_tAveDiff_ampCorr_vs_posY", 200, ymin, ymax, dtminAveSum, dtmaxAveSum);
  TH1F*          h_tAveDiff_ampCorr_binX[1000];
  for (int ibin = 0; ibin < NBINSX[0]; ibin++){
    h_tAveDiff_ampCorr_binX[ibin] = new TH1F(Form("h_tAveDiff_ampCorr_binX_%d",ibin),Form("h_tAveDiff_ampCorr_binX_%d",ibin), nbinsHistoX, dtminAveSum, dtmaxAveSum);
  }
  TH1F*          h_tAveDiff_ampCorr_binY[1000] ;
  for (int ibin = 0; ibin < NBINSY; ibin++){
    h_tAveDiff_ampCorr_binY[ibin] = new TH1F(Form("h_tAveDiff_ampCorr_binY_%d",ibin),Form("h_tAveDiff_ampCorr_binY_%d",ibin), nbinsHistoY, dtminAveSum, dtmaxAveSum);
  }

  TH1F*          h_tAveDiff_ampCorr_posCorr = new TH1F("h_tAveDiff_ampCorr_posCorr","h_tAveDiff_ampCorr_posCorr",1000, dtminAveSum, dtmaxAveSum);
  TProfile*      p_tAveDiff_ampCorr_posCorr_vs_posX  = new TProfile("p_tAveDiff_ampCorr_posCorr_vs_posX","p_tAveDiff_ampCorr_posCorr_vs_posX", 400, xmin, xmax, dtminAveSum, dtmaxAveSum);
  TProfile*      p_tAveDiff_ampCorr_posCorr_vs_posY  = new TProfile("p_tAveDiff_ampCorr_posCorr_vs_posY","p_tAveDiff_ampCorr_posCorr_vs_posY", 200, ymin, ymax, dtminAveSum, dtmaxAveSum);
  TH1F*          h_tAveDiff_ampCorr_posCorr_binX[1000];
  for (int ibin = 0; ibin < NBINSX[0]; ibin++){
    h_tAveDiff_ampCorr_posCorr_binX[ibin] = new TH1F(Form("h_tAveDiff_ampCorr_posCorr_binX_%d",ibin),Form("h_tAveDiff_ampCorr_posCorr_binX_%d",ibin), nbinsHistoX, dtminAveSum, dtmaxAveSum);
  }
  TH1F*          h_tAveDiff_ampCorr_posCorr_binY[1000] ;
  for (int ibin = 0; ibin < NBINSY; ibin++){
    h_tAveDiff_ampCorr_posCorr_binY[ibin] = new TH1F(Form("h_tAveDiff_ampCorr_posCorr_binY_%d",ibin),Form("h_tAveDiff_ampCorr_posCorr_binY_%d",ibin), nbinsHistoY, dtminAveSum, dtmaxAveSum);
  }

  // -- pulse integral corr
  TH1F*          h_tAveDiff_pulseIntCorr = new TH1F("h_tAveDiff_pulseIntCorr","h_tAveDiff_pulseIntCorr",1000, dtminAveSum, dtmaxAveSum);
  TProfile*      p_tAveDiff_pulseIntCorr_vs_posX  = new TProfile("p_tAveDiff_pulseIntCorr_vs_posX","p_tAveDiff_pulseIntCorr_vs_posX", 400, xmin, xmax, dtminAveSum, dtmaxAveSum);
  TProfile*      p_tAveDiff_pulseIntCorr_vs_posY  = new TProfile("p_tAveDiff_pulseIntCorr_vs_posY","p_tAveDiff_pulseIntCorr_vs_posY", 200, ymin, ymax, dtminAveSum, dtmaxAveSum);
  TH1F*          h_tAveDiff_pulseIntCorr_binX[1000];
  for (int ibin = 0; ibin < NBINSX[0]; ibin++){
    h_tAveDiff_pulseIntCorr_binX[ibin] = new TH1F(Form("h_tAveDiff_pulseIntCorr_binX_%d",ibin),Form("h_tAveDiff_pulseIntCorr_binX_%d",ibin), nbinsHistoX, dtminAveSum, dtmaxAveSum);
  }
  TH1F*          h_tAveDiff_pulseIntCorr_binY[1000] ;
  for (int ibin = 0; ibin < NBINSY; ibin++){
    h_tAveDiff_pulseIntCorr_binY[ibin] = new TH1F(Form("h_tAveDiff_pulseIntCorr_binY_%d",ibin),Form("h_tAveDiff_pulseIntCorr_binY_%d",ibin), nbinsHistoY, dtminAveSum, dtmaxAveSum);
  }

  TH1F*          h_tAveDiff_pulseIntCorr_posCorr = new TH1F("h_tAveDiff_pulseIntCorr_posCorr","h_tAveDiff_pulseIntCorr_posCorr",1000, dtminAveSum, dtmaxAveSum);
  TProfile*      p_tAveDiff_pulseIntCorr_posCorr_vs_posX  = new TProfile("p_tAveDiff_pulseIntCorr_posCorr_vs_posX","p_tAveDiff_pulseIntCorr_posCorr_vs_posX", 400, xmin, xmax, dtminAveSum, dtmaxAveSum);
  TProfile*      p_tAveDiff_pulseIntCorr_posCorr_vs_posY  = new TProfile("p_tAveDiff_pulseIntCorr_posCorr_vs_posY","p_tAveDiff_pulseIntCorr_posCorr_vs_posY", 200, ymin, ymax, dtminAveSum, dtmaxAveSum);
  TH1F*          h_tAveDiff_pulseIntCorr_posCorr_binX[1000];
  for (int ibin = 0; ibin < NBINSX[0]; ibin++){
    h_tAveDiff_pulseIntCorr_posCorr_binX[ibin] = new TH1F(Form("h_tAveDiff_pulseIntCorr_posCorr_binX_%d",ibin),Form("h_tAveDiff_pulseIntCorr_posCorr_binX_%d",ibin), nbinsHistoX, dtminAveSum, dtmaxAveSum);
  }
  TH1F*          h_tAveDiff_pulseIntCorr_posCorr_binY[1000] ;
  for (int ibin = 0; ibin < NBINSY; ibin++){
    h_tAveDiff_pulseIntCorr_posCorr_binY[ibin] = new TH1F(Form("h_tAveDiff_pulseIntCorr_posCorr_binY_%d",ibin),Form("h_tAveDiff_pulseIntCorr_posCorr_binY_%d",ibin), nbinsHistoY, dtminAveSum, dtmaxAveSum);
  }



  // -- tDiff between two bars tDiff
  // - amp corr
  TH1F*          h_tDiffDiff_ampCorr = new TH1F("h_tDiffDiff_ampCorr","h_tDiffDiff_ampCorr",1000, dtminAveSum, dtmaxAveSum);
  TProfile*      p_tDiffDiff_ampCorr_vs_posX  = new TProfile("p_tDiffDiff_ampCorr_vs_posX","p_tDiffDiff_ampCorr_vs_posX", 400, xmin, xmax, dtminAveSum, dtmaxAveSum);
  TProfile*      p_tDiffDiff_ampCorr_vs_posY  = new TProfile("p_tDiffDiff_ampCorr_vs_posY","p_tDiffDiff_ampCorr_vs_posY", 200, ymin, ymax, dtminAveSum, dtmaxAveSum);
  TH1F*          h_tDiffDiff_ampCorr_binX[1000];
  for (int ibin = 0; ibin < NBINSX[0]; ibin++){
    h_tDiffDiff_ampCorr_binX[ibin] = new TH1F(Form("h_tDiffDiff_ampCorr_binX_%d",ibin),Form("h_tDiffDiff_ampCorr_binX_%d",ibin), nbinsHistoX, dtminAveSum, dtmaxAveSum);
  }
  TH1F*          h_tDiffDiff_ampCorr_binY[1000] ;
  for (int ibin = 0; ibin < NBINSY; ibin++){
    h_tDiffDiff_ampCorr_binY[ibin] = new TH1F(Form("h_tDiffDiff_ampCorr_binY_%d",ibin),Form("h_tDiffDiff_ampCorr_binY_%d",ibin), nbinsHistoY/4, dtminAveSum, dtmaxAveSum);
  }

  TH1F*          h_tDiffDiff_ampCorr_posCorr = new TH1F("h_tDiffDiff_ampCorr_posCorr","h_tDiffDiff_ampCorr_posCorr",1000, dtminAveSum, dtmaxAveSum);
  TProfile*      p_tDiffDiff_ampCorr_posCorr_vs_posX  = new TProfile("p_tDiffDiff_ampCorr_posCorr_vs_posX","p_tDiffDiff_ampCorr_posCorr_vs_posX", 400, xmin, xmax, dtminAveSum, dtmaxAveSum);
  TProfile*      p_tDiffDiff_ampCorr_posCorr_vs_posY  = new TProfile("p_tDiffDiff_ampCorr_posCorr_vs_posY","p_tDiffDiff_ampCorr_posCorr_vs_posY", 200, ymin, ymax, dtminAveSum, dtmaxAveSum);
  TH1F*          h_tDiffDiff_ampCorr_posCorr_binX[1000];
  for (int ibin = 0; ibin < NBINSX[0]; ibin++){
    h_tDiffDiff_ampCorr_posCorr_binX[ibin] = new TH1F(Form("h_tDiffDiff_ampCorr_posCorr_binX_%d",ibin),Form("h_tDiffDiff_ampCorr_posCorr_binX_%d",ibin), nbinsHistoX, dtminAveSum, dtmaxAveSum);
  }
  TH1F*          h_tDiffDiff_ampCorr_posCorr_binY[1000] ;
  for (int ibin = 0; ibin < NBINSY; ibin++){
    h_tDiffDiff_ampCorr_posCorr_binY[ibin] = new TH1F(Form("h_tDiffDiff_ampCorr_posCorr_binY_%d",ibin),Form("h_tDiffDiff_ampCorr_posCorr_binY_%d",ibin), nbinsHistoY/2, dtminAveSum, dtmaxAveSum);
  }

  // -- pulse integral corr
  TH1F*          h_tDiffDiff_pulseIntCorr = new TH1F("h_tDiffDiff_pulseIntCorr","h_tDiffDiff_pulseIntCorr",1000, dtminAveSum, dtmaxAveSum);
  TProfile*      p_tDiffDiff_pulseIntCorr_vs_posX  = new TProfile("p_tDiffDiff_pulseIntCorr_vs_posX","p_tDiffDiff_pulseIntCorr_vs_posX", 400, xmin, xmax, dtminAveSum, dtmaxAveSum);
  TProfile*      p_tDiffDiff_pulseIntCorr_vs_posY  = new TProfile("p_tDiffDiff_pulseIntCorr_vs_posY","p_tDiffDiff_pulseIntCorr_vs_posY", 200, ymin, ymax, dtminAveSum, dtmaxAveSum);
  TH1F*          h_tDiffDiff_pulseIntCorr_binX[1000];
  for (int ibin = 0; ibin < NBINSX[0]; ibin++){
    h_tDiffDiff_pulseIntCorr_binX[ibin] = new TH1F(Form("h_tDiffDiff_pulseIntCorr_binX_%d",ibin),Form("h_tDiffDiff_pulseIntCorr_binX_%d",ibin), nbinsHistoX, dtminAveSum, dtmaxAveSum);
  }
  TH1F*          h_tDiffDiff_pulseIntCorr_binY[1000] ;
  for (int ibin = 0; ibin < NBINSY; ibin++){
    h_tDiffDiff_pulseIntCorr_binY[ibin] = new TH1F(Form("h_tDiffDiff_pulseIntCorr_binY_%d",ibin),Form("h_tDiffDiff_pulseIntCorr_binY_%d",ibin), nbinsHistoY/2, dtminAveSum, dtmaxAveSum);
  }

  TH1F*          h_tDiffDiff_pulseIntCorr_posCorr = new TH1F("h_tDiffDiff_pulseIntCorr_posCorr","h_tDiffDiff_pulseIntCorr_posCorr",1000, dtminAveSum, dtmaxAveSum);
  TProfile*      p_tDiffDiff_pulseIntCorr_posCorr_vs_posX  = new TProfile("p_tDiffDiff_pulseIntCorr_posCorr_vs_posX","p_tDiffDiff_pulseIntCorr_posCorr_vs_posX", 400, xmin, xmax, dtminAveSum, dtmaxAveSum);
  TProfile*      p_tDiffDiff_pulseIntCorr_posCorr_vs_posY  = new TProfile("p_tDiffDiff_pulseIntCorr_posCorr_vs_posY","p_tDiffDiff_pulseIntCorr_posCorr_vs_posY", 200, ymin, ymax, dtminAveSum, dtmaxAveSum);
  TH1F*          h_tDiffDiff_pulseIntCorr_posCorr_binX[1000];
  for (int ibin = 0; ibin < NBINSX[0]; ibin++){
    h_tDiffDiff_pulseIntCorr_posCorr_binX[ibin] = new TH1F(Form("h_tDiffDiff_pulseIntCorr_posCorr_binX_%d",ibin),Form("h_tDiffDiff_pulseIntCorr_posCorr_binX_%d",ibin), nbinsHistoX, dtminAveSum, dtmaxAveSum);
  }
  TH1F*          h_tDiffDiff_pulseIntCorr_posCorr_binY[1000] ;
  for (int ibin = 0; ibin < NBINSY; ibin++){
    h_tDiffDiff_pulseIntCorr_posCorr_binY[ibin] = new TH1F(Form("h_tDiffDiff_pulseIntCorr_posCorr_binY_%d",ibin),Form("h_tDiffDiff_pulseIntCorr_posCorr_binY_%d",ibin), nbinsHistoY/2, dtminAveSum, dtmaxAveSum);
  }



  float ampRef = 0;
  float tRef = 0;
  float chi2Ref = 0;

  float ampL  = 0.;
  float ampR  = 0.;
  float pulseIntL = 0;
  float pulseIntR = 0;
  float pulseIntAve = 0;
  float tL    = 0.;
  float tR    = 0.;
  float tAve  = 0.;
  
  float brmsL = 0;
  float brmsR = 0;
  float brmsRef = 0;
  float sigmaRef = 0;

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
      if (useTimeMaxMCP) tRef   = treeVars.t_time_max[(*treeVars.t_channelId)[timeChannelRef]];

      h_ampRef_nocuts->Fill(ampRef);
      p2_ampRef_vs_run_vs_event->Fill(treeVars.t_run, treeVars.t_event, ampRef);
      h2_ampRef_vs_event->Fill(entry, ampRef);
      if ( ampRef > 0.020 ) h_timeRef -> Fill(tRef);

      float ampSum = 0;
      float pulseIntSum = 0;

      // -- loop over bars
      for (int iBar = 0; iBar < NBARS; iBar++){

        ampL = treeVars.t_amp[(*treeVars.t_channelId)[ ampChannelsL[iBar] ]] * kAdcToV ;
        ampR = treeVars.t_amp[(*treeVars.t_channelId)[ ampChannelsR[iBar] ]] * kAdcToV ;
	pulseIntL = treeVars.t_charge_sig[(*treeVars.t_channelId)[ ampChannelsL[iBar] ]] * kAdcToV;
        pulseIntR = treeVars.t_charge_sig[(*treeVars.t_channelId)[ ampChannelsR[iBar] ]] * kAdcToV;

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
	
	ampSum+=(ampL+ampR)/2;
	pulseIntSum+=(pulseIntL+pulseIntR)/2;
	
	if ( ampL > 40 * kAdcToV && ampL < 0.9 ) h_timeL[iBar] -> Fill(tL);
	if ( ampR > 40 * kAdcToV && ampR < 0.9 ) h_timeR[iBar] -> Fill(tR);      
      }

      h_ampSum_nocuts-> Fill(ampSum);
      h_pulseIntSum_nocuts-> Fill(pulseIntSum);

    }

  
  // --  Find mip peak MCP
  TF1 *ffitRef = new TF1("ffitRef","gaus",0, 1000); 
  ffitRef->SetRange(0.030,0.250); 
  h_ampRef_nocuts->Fit("ffitRef","QR");
  float mipPeakRef = ffitRef-> GetParameter(1);  
  float cut_ampMinRef =  mipPeakRef - 3. * ffitRef-> GetParameter(2);
  float cut_ampMaxRef =  mipPeakRef + 3. * ffitRef-> GetParameter(2);

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
    cut_ampMaxL.push_back(maxamp);
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
    cut_ampMaxR.push_back(maxamp);
  }

  // find mip peak on sum of 3 bars
  TF1 *fLandauAmpSum = new TF1("fLandauAmpSum","landau",0., 1.);
  fLandauAmpSum->SetRange(0.1, 0.2);
  h_ampSum_nocuts->Fit(fLandauAmpSum,"QR");

  cut_ampMinL.clear();
  cut_ampMinR.clear();
  cut_ampMaxL.clear();
  cut_ampMaxR.clear();
  for (int iBar = 0; iBar< NBARS; iBar++){
    float mipPeak = fLandauAmpSum->GetParameter(1);
    cut_ampMinL.push_back(cut_ampMinFraction * mipPeak);
    cut_ampMinR.push_back(cut_ampMinFraction * mipPeak);
    cut_ampMaxL.push_back(min(float(5. * mipPeak), maxAmpSaturation));
    cut_ampMaxR.push_back(min(float(5. * mipPeak), maxAmpSaturation));
  }

  TF1 *fLandauIntSum = new TF1("fLandauIntSum","landau",0., 1000000.);
  fLandauIntSum->SetRange(15, 30);
  h_pulseIntSum_nocuts->Fit(fLandauIntSum,"QR");
  float pulseIntMPV = fLandauIntSum->GetParameter(1);
  std::vector<float> cut_pulseIntMinL, cut_pulseIntMinR, cut_pulseIntMaxL, cut_pulseIntMaxR;
  cut_pulseIntMinL.clear();
  cut_pulseIntMinR.clear();
  cut_pulseIntMaxL.clear();
  cut_pulseIntMaxR.clear();
  for (int iBar = 0; iBar< NBARS; iBar++){
    float mipPeak = fLandauIntSum->GetParameter(1);
    maxAmpSaturation = 60.;
    cut_pulseIntMinL.push_back(cut_ampMinFraction * mipPeak);
    cut_pulseIntMinR.push_back(cut_ampMinFraction * mipPeak);
    cut_pulseIntMaxL.push_back(min(float(5. * mipPeak), maxAmpSaturation));
    cut_pulseIntMaxR.push_back(min(float(5. * mipPeak), maxAmpSaturation));
  }

  cout << "Amplitude selections" <<endl;
  cout << "Photek: " << cut_ampMinRef << "  " << cut_ampMaxRef <<endl;
  for (int iBar = 0; iBar< NBARS; iBar++){
    cout << "BAR"<< iBar << "   Left:   minAmp (V) = " << cut_ampMinL[iBar] << "     maxAmp (V) = " << cut_ampMaxL[iBar] << "         Right:  minAmp (V) " << cut_ampMinR[iBar] << "    maxAmp (V) = " << cut_ampMaxR[iBar] <<endl;

    cout << "BAR"<< iBar << "   Left:   minPulseIntegral (V) = " << cut_pulseIntMinL[iBar] << "     maxPulseIntegral (V) = " << cut_pulseIntMaxL[iBar] << "         Right:  minPulseIntegral (V) " << cut_pulseIntMinR[iBar] << "    maxPulseIntegral (V) = " << cut_pulseIntMaxR[iBar] <<endl;
  }


  // -- Fit time to get time selections
  vector<float> tLmin ;
  vector<float> tLmax ;
  vector<float> tRmin ;
  vector<float> tRmax ;  

  float nSigma = 3;
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
      if (useTimeMaxMCP) tRef   = treeVars.t_time_max[(*treeVars.t_channelId)[timeChannelRef]];
      brmsRef = treeVars.t_b_rms[(*treeVars.t_channelId)[timeChannelRef]];
      chi2Ref = treeVars.t_chi2_max[(*treeVars.t_channelId)[timeChannelRef]];
      sigmaRef = treeVars.t_gaus_sigma[(*treeVars.t_channelId)[timeChannelRef]];
      
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
	h_brmsRef ->Fill(brmsRef);
	if (brmsRef < cut_brmsMaxRef) {
	  h_chi2Ref ->Fill(chi2Ref);
	  h_sigmaRef ->Fill(sigmaRef);
	}
      }
      

      // -- loop over bars
      for (int iBar = 0; iBar < NBARS; iBar++){
	
        ampL = treeVars.t_amp[(*treeVars.t_channelId)[ ampChannelsL[iBar] ]] * kAdcToV ;
        ampR = treeVars.t_amp[(*treeVars.t_channelId)[ ampChannelsR[iBar] ]] * kAdcToV ;
	
        tL   = treeVars.t_time[(*treeVars.t_channelId)[ timeChannelsL[iBar] ]+treeVars.t_LED];
        tR   = treeVars.t_time[(*treeVars.t_channelId)[ timeChannelsR[iBar] ]+treeVars.t_LED];
	
        brmsL = treeVars.t_b_rms[(*treeVars.t_channelId)[ timeChannelsL[iBar] ] ];
        brmsR = treeVars.t_b_rms[(*treeVars.t_channelId)[ timeChannelsR[iBar] ] ];
	
	pulseIntL = treeVars.t_charge_sig[(*treeVars.t_channelId)[ ampChannelsL[iBar] ]] * kAdcToV;
        pulseIntR = treeVars.t_charge_sig[(*treeVars.t_channelId)[ ampChannelsR[iBar] ]] * kAdcToV;
        pulseIntAve = 0.5 *(pulseIntL+pulseIntR);

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
        //if ( ampL   < cut_ampMinL[iBar] ||  ampL   > cut_ampMaxL[iBar] ) continue;
        //if ( ampR   < cut_ampMinR[iBar] ||  ampR   > cut_ampMaxR[iBar] ) continue;
	if ( pulseIntL   < cut_pulseIntMinL[iBar] ||  pulseIntL   > cut_pulseIntMaxL[iBar] ) continue;
        if ( pulseIntR   < cut_pulseIntMinR[iBar] ||  pulseIntR   > cut_pulseIntMaxR[iBar] ) continue;

    	// -- remove events with bad reco time
        if ( tRef < cut_minTimeRef || tRef > cut_maxTimeRef ) continue;
        if ( tL < cut_minTime[iBar] || tL > cut_maxTime[iBar]) continue;
        if ( tR < cut_minTime[iBar] || tR > cut_maxTime[iBar] ) continue;
	
	if (tL < tLmin[iBar] || tL > tLmax[iBar]) continue;
        if (tR < tRmin[iBar] || tR > tRmax[iBar]) continue;

        h2_timeL_vs_brms[iBar] -> Fill(brmsL, tL);
        h2_timeR_vs_brms[iBar] -> Fill(brmsR, tR);

	h_brms_timeL[iBar] -> Fill(brmsL);
	h_brms_timeR[iBar] -> Fill(brmsR);

	// -- remove very noisy events
        if (brmsRef > cut_brmsMaxRef) continue;
        if (brmsR > cut_brmsMaxTimeCh[iBar]) continue;
        if (brmsL > cut_brmsMaxTimeCh[iBar]) continue;

        //-- time of Left, Right                                                                                                               
	if (applyMCPTimeCorrection) tRef = GetCorrectedMCPTime(tRef, ampRef*1000., posX);  
        tL = tL - tRef;
        tR = tR - tRef;
        
	h_tL[iBar]->Fill(tL);
	h_tR[iBar]->Fill(tR);


	if (iBar==1){
	  selectedEventsFile << treeVars.t_run << "   " << treeVars.t_event <<endl;
	}
      }
    }

  
  selectedEventsFile.close();


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
      if (useTimeMaxMCP) tRef   = treeVars.t_time_max[(*treeVars.t_channelId)[timeChannelRef]];
      brmsRef = treeVars.t_b_rms[(*treeVars.t_channelId)[timeChannelRef]];

      float amp0 = 0.5*(treeVars.t_amp[(*treeVars.t_channelId)[ ampChannelsL[0] ]] + treeVars.t_amp[(*treeVars.t_channelId)[ ampChannelsR[0] ]] );
      float amp1 = 0.5*(treeVars.t_amp[(*treeVars.t_channelId)[ ampChannelsL[1] ]] + treeVars.t_amp[(*treeVars.t_channelId)[ ampChannelsR[1] ]] );
      h2_ampRatio->Fill(posY, (amp0-amp1)/(amp0+amp1));

      // -- loop over bars
      for (int iBar = 0; iBar < NBARS; iBar++){
	
	ampL = treeVars.t_amp[(*treeVars.t_channelId)[ ampChannelsL[iBar] ]] * kAdcToV ;
	ampR = treeVars.t_amp[(*treeVars.t_channelId)[ ampChannelsR[iBar] ]] * kAdcToV ;
	
	tL   = treeVars.t_time[(*treeVars.t_channelId)[ timeChannelsL[iBar] ]+treeVars.t_LED];  
	tR   = treeVars.t_time[(*treeVars.t_channelId)[ timeChannelsR[iBar] ]+treeVars.t_LED];  
	
	brmsL = treeVars.t_b_rms[(*treeVars.t_channelId)[ timeChannelsL[iBar] ] ];
	brmsR = treeVars.t_b_rms[(*treeVars.t_channelId)[ timeChannelsR[iBar] ] ];
	
	pulseIntL = treeVars.t_charge_sig[(*treeVars.t_channelId)[ ampChannelsL[iBar] ]] * kAdcToV;
        pulseIntR = treeVars.t_charge_sig[(*treeVars.t_channelId)[ ampChannelsR[iBar] ]] * kAdcToV;
        pulseIntAve = 0.5 *(pulseIntL+pulseIntR);

	// -- select events
        if ( posX < cut_XminRef || posX > cut_XmaxRef ) continue;
        if ( posY < cut_YminRef || posY > cut_YmaxRef ) continue;
	if ( posX < cut_Xmin[iBar] || posX > cut_Xmax[iBar] ) continue;
	if ( posY < cut_Ymin[iBar] || posY > cut_Ymax[iBar] ) continue;

	if ( ampRef < cut_ampMinRef     ||  ampRef > cut_ampMaxRef ) continue;
	//if ( ampL   < cut_ampMinL[iBar] ||  ampL   > cut_ampMaxL[iBar] ) continue;
	//if ( ampR   < cut_ampMinR[iBar] ||  ampR   > cut_ampMaxR[iBar] ) continue;
	if ( pulseIntL   < cut_pulseIntMinL[iBar] ||  pulseIntL   > cut_pulseIntMaxL[iBar] ) continue;
        if ( pulseIntR   < cut_pulseIntMinR[iBar] ||  pulseIntR   > cut_pulseIntMaxR[iBar] ) continue;

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
	if (applyMCPTimeCorrection) tRef = GetCorrectedMCPTime(tRef, ampRef*1000., posX);       
	tL = tL - tRef;
	tR = tR - tRef;
	tAve  = (tL + tR)/2;

	// --- SiPM Left
	h_ampL[iBar]  -> Fill(ampL);
	h_pulseIntL[iBar]  -> Fill(pulseIntL);
        h2_pulseIntL_vs_amp[iBar] ->Fill(ampL, pulseIntL);
	p2_ampL_vs_posXY[iBar] ->Fill(posX,posY,ampL);
	p_ampL_vs_posX[iBar] ->Fill(posX,ampL);
	p_ampL_vs_posXc[iBar] ->Fill(posX/cos(theta[iBar]),ampL);
	p_ampL_vs_posY[iBar] ->Fill(posY,ampL);

	h2_tL_vs_amp[iBar]->Fill(ampL,tL);
	if ( tL > h_tL[iBar]->GetMean()-3*h_tL[iBar]->GetRMS() && tL < h_tL[iBar]->GetMean()+3*h_tL[iBar]->GetRMS() ) {
	  p_tL_vs_amp[iBar]->Fill(ampL,tL);
	  int xcbinamp = int ((posX - max(cut_Xmin[iBar], cut_XminRef))/cos(theta[iBar])/xbinWidthForAmpWalk );
	  p_tL_vs_amp_binned[iBar][xcbinamp]->Fill(ampL,tL);
	  if (useAverageAmplitudeForTimeWalkCorr==0) {
	    p_tL_vs_pulseInt[iBar]->Fill(pulseIntL,tL);
	    p_tL_vs_pulseInt_binned[iBar][xcbinamp]->Fill(pulseIntL,tL);
	  }
	  else{
	    p_tL_vs_pulseInt[iBar]->Fill(pulseIntAve,tL);
	    p_tL_vs_pulseInt_binned[iBar][xcbinamp]->Fill(pulseIntAve,tL);
	  }
	}
	p2_tL_vs_amp_vs_posXc[iBar]  ->Fill(ampL, posX/cos(theta[iBar]), tL);
	p2_tL_vs_posXY[iBar] ->Fill(posX,posY,tL);
	p_tL_vs_posX[iBar]->Fill(posX,tL);
	p_tL_vs_posY[iBar]->Fill(posY,tL);
	
	// --- SiPM Right
        h_ampR[iBar]  -> Fill(ampR);
	h_pulseIntR[iBar]  -> Fill(pulseIntR);
        h2_pulseIntR_vs_amp[iBar] ->Fill(ampR, pulseIntR);
        p2_ampR_vs_posXY[iBar] ->Fill(posX,posY,ampR);
        p_ampR_vs_posX[iBar] ->Fill(posX,ampR);
        p_ampR_vs_posXc[iBar] ->Fill(posX/cos(theta[iBar]),ampR);
        p_ampR_vs_posY[iBar] ->Fill(posY,ampR);
        
	h2_tR_vs_amp[iBar]->Fill(ampR,tR);
       	if ( tR > h_tR[iBar]->GetMean()-3*h_tR[iBar]->GetRMS() && tR < h_tR[iBar]->GetMean()+3*h_tR[iBar]->GetRMS() ) {
	  p_tR_vs_amp[iBar]->Fill(ampR,tR);
	  int xcbinamp = int ((posX - max(cut_Xmin[iBar], cut_XminRef))/cos(theta[iBar])/xbinWidthForAmpWalk );
          p_tR_vs_amp_binned[iBar][xcbinamp] ->Fill(ampR,tR);
	  if (useAverageAmplitudeForTimeWalkCorr==0) {
	    p_tR_vs_pulseInt[iBar]->Fill(pulseIntR,tR);
	    p_tR_vs_pulseInt_binned[iBar][xcbinamp]->Fill(pulseIntR,tR);
	  }
	  else{
	    p_tR_vs_pulseInt[iBar]->Fill(pulseIntAve,tR);
	    p_tR_vs_pulseInt_binned[iBar][xcbinamp]->Fill(pulseIntAve,tR);
	  }
        }
	p2_tR_vs_amp_vs_posXc[iBar]  ->Fill(ampR, posX/cos(theta[iBar]), tR);
	p2_tR_vs_posXY[iBar] ->Fill(posX,posY,tR);
        p_tR_vs_posX[iBar]->Fill(posX,tR);
        p_tR_vs_posY[iBar]->Fill(posY,tR);
        
	// -- tDiff
	h_tDiff[iBar]->Fill(tR-tL);
	p_tDiff_vs_posXc[iBar]->Fill(posX/cos(theta[iBar]),tR-tL);
	int xbin = int ((posX - max(cut_Xmin[iBar], cut_XminRef)) /xbinWidth );
	int ybin = int ((posY-cut_Ymin[NBARS-1])/ybinWidth );
        h_tDiff_binX[iBar][xbin]->Fill(tR-tL);
        h_tDiff_binY[iBar][ybin]->Fill(tR-tL);


	// -- average
        h_tAve[iBar]->Fill(tAve);
	p2_tAve_vs_posXY[iBar] ->Fill(posX,posY,tAve);
        p_tAve_vs_posX[iBar]->Fill(posX,tAve);
        p_tAve_vs_posY[iBar]->Fill(posY,tAve);
                                                    
      } // -- end loop over bars


    }// -- end first loop over events
  std::cout<<" >>> End first loop over events" <<std::endl;

  
  // --- Amplitude walk corrections
  TF1*  fitFuncL_ampCorr[NBARS];  
  TF1*  fitFuncR_ampCorr[NBARS];  
  TF1*  fitFuncL_ampCorr_binned[NBARS][1000];
  TF1*  fitFuncR_ampCorr_binned[NBARS][1000];
  
  float mpvL[NBARS];
  float mpvR[NBARS];

  for (int iBar = 0 ; iBar < NBARS; iBar++){
    fitFuncL_ampCorr[iBar] = new TF1(Form("fitFuncL_ampCorr_%d", iBar),fitFunction.c_str());
    fitFuncL_ampCorr[iBar] -> SetLineColor(kRed);
    mpvL[iBar] = (h_ampSum_nocuts->GetFunction("fLandauAmpSum"))->GetParameter(1);
    fitFuncL_ampCorr[iBar] -> SetRange(0, 5*mpvL[iBar]);
    p_tL_vs_amp[iBar] -> Fit(Form("fitFuncL_ampCorr_%d",iBar),"QSRW");
    p_tL_vs_amp[iBar] -> Fit(Form("fitFuncL_ampCorr_%d",iBar),"QSR");
    if (fitFuncL_ampCorr[iBar] -> GetChisquare()/fitFuncL_ampCorr[iBar] -> GetNDF() > 5 ){
      fitFuncL_ampCorr[iBar] -> SetRange(0, 4*mpvL[iBar]);
      p_tL_vs_amp[iBar] -> Fit(Form("fitFuncL_ampCorr_%d",iBar),"QSR");
      if (fitFuncL_ampCorr[iBar] -> GetChisquare()/fitFuncL_ampCorr[iBar] -> GetNDF() > 5 ){
	fitFuncL_ampCorr[iBar] -> SetRange(0, 3*mpvL[iBar]);
	p_tL_vs_amp[iBar] -> Fit(Form("fitFuncL_ampCorr_%d",iBar),"QSR");
      }
    }
    
    fitFuncR_ampCorr[iBar] = new TF1(Form("fitFuncR_ampCorr_%d", iBar),fitFunction.c_str());
    fitFuncR_ampCorr[iBar] -> SetLineColor(kRed);
    mpvR[iBar] = (h_ampSum_nocuts->GetFunction("fLandauAmpSum"))->GetParameter(1);
    fitFuncR_ampCorr[iBar] -> SetRange(0, 5*mpvR[iBar]);
    p_tR_vs_amp[iBar] -> Fit(Form("fitFuncR_ampCorr_%d",iBar),"QSRW");
    p_tR_vs_amp[iBar] -> Fit(Form("fitFuncR_ampCorr_%d",iBar),"QSR");
    if (fitFuncR_ampCorr[iBar] -> GetChisquare()/fitFuncR_ampCorr[iBar] -> GetNDF() > 5 ){
      fitFuncR_ampCorr[iBar] -> SetRange(0, 4*mpvR[iBar]);
      p_tR_vs_amp[iBar] -> Fit(Form("fitFuncR_ampCorr_%d",iBar),"QSR");
      if (fitFuncR_ampCorr[iBar] -> GetChisquare()/fitFuncR_ampCorr[iBar] -> GetNDF() > 5 ){
	fitFuncR_ampCorr[iBar] -> SetRange(0, 3*mpvR[iBar]);
	p_tR_vs_amp[iBar] -> Fit(Form("fitFuncR_ampCorr_%d",iBar),"QSR");
      }
    }

    // amp corr binned in X
    for (int ibin =0; ibin < NBINSXAMPWALK[iBar]; ibin++){
      fitFuncL_ampCorr_binned[iBar][ibin] = new TF1(Form("fitFuncL_ampCorr_%d_%d",ibin,  iBar),fitFunction.c_str());
      fitFuncL_ampCorr_binned[iBar][ibin] -> SetLineColor(kRed);
      fitFuncL_ampCorr_binned[iBar][ibin] -> SetRange(0, 5*mpvL[iBar]);
      p_tL_vs_amp_binned[iBar][ibin] -> Fit(Form("fitFuncL_ampCorr_%d_%d",ibin, iBar),"QSRW");
      p_tL_vs_amp_binned[iBar][ibin] -> Fit(Form("fitFuncL_ampCorr_%d_%d",ibin, iBar),"QSR");
      if (fitFuncL_ampCorr_binned[iBar][ibin] -> GetChisquare()/fitFuncL_ampCorr_binned[iBar][ibin] -> GetNDF() > 5 ){
	fitFuncL_ampCorr_binned[iBar][ibin] -> SetRange(0, 4*mpvL[iBar]);
        p_tL_vs_amp_binned[iBar][ibin] -> Fit(Form("fitFuncL_ampCorr_%d_%d",ibin, iBar),"QSR");
	if (fitFuncL_ampCorr_binned[iBar][ibin] -> GetChisquare()/fitFuncL_ampCorr_binned[iBar][ibin] -> GetNDF() > 5 ){
	  fitFuncL_ampCorr_binned[iBar][ibin] -> SetRange(0, 3*mpvL[iBar]);
	  p_tL_vs_amp_binned[iBar][ibin] -> Fit(Form("fitFuncL_ampCorr_%d_%d",ibin, iBar),"QSR");
	}
      }

      fitFuncR_ampCorr_binned[iBar][ibin] = new TF1(Form("fitFuncR_ampCorr_%d_%d", ibin, iBar),fitFunction.c_str());
      fitFuncR_ampCorr_binned[iBar][ibin] -> SetLineColor(kRed);
      fitFuncR_ampCorr_binned[iBar][ibin] -> SetRange(0, 5*mpvR[iBar]);
      p_tR_vs_amp_binned[iBar][ibin] -> Fit(Form("fitFuncR_ampCorr_%d_%d",ibin,iBar),"QSRW");
      p_tR_vs_amp_binned[iBar][ibin] -> Fit(Form("fitFuncR_ampCorr_%d_%d",ibin,iBar),"QSR");
      if (fitFuncR_ampCorr_binned[iBar][ibin] -> GetChisquare()/fitFuncR_ampCorr_binned[iBar][ibin] -> GetNDF() > 5){
	fitFuncR_ampCorr_binned[iBar][ibin] -> SetRange(0, 4*mpvR[iBar]);
	p_tR_vs_amp_binned[iBar][ibin] -> Fit(Form("fitFuncR_ampCorr_%d_%d",ibin,iBar),"QSR");
	if (fitFuncR_ampCorr_binned[iBar][ibin] -> GetChisquare()/fitFuncR_ampCorr_binned[iBar][ibin] -> GetNDF() > 5){
	  fitFuncR_ampCorr_binned[iBar][ibin] -> SetRange(0, 3*mpvR[iBar]);
	  p_tR_vs_amp_binned[iBar][ibin] -> Fit(Form("fitFuncR_ampCorr_%d_%d",ibin,iBar),"QSR");
	}
      }
    }


  }


  // -- amp walk corrections vs pulse integral
  TF1*  fitFuncL_pulseIntCorr[NBARS];
  TF1*  fitFuncR_pulseIntCorr[NBARS];
  TF1*  fitFuncL_pulseIntCorr_binned[NBARS][1000];
  TF1*  fitFuncR_pulseIntCorr_binned[NBARS][1000];
  
  TF1 *fLandauIntL[NBARS];
  TF1 *fLandauIntR[NBARS];
  float mpvIntL[NBARS];
  float mpvIntR[NBARS];

  for (int iBar = 0 ; iBar < NBARS; iBar++){
    fLandauIntL[iBar] = new TF1(Form("fLandauIntL_BAR%d",iBar),"landau", 0, 1000);
    h_pulseIntL[iBar]->Fit(fLandauIntL[iBar],"QR");
    mpvIntL[iBar] = (h_pulseIntSum_nocuts->GetFunction("fLandauIntSum"))->GetParameter(1);
    fitFuncL_pulseIntCorr[iBar] = new TF1(Form("fitFuncL_pulseIntCorr_%d", iBar),fitFunction.c_str(), 0, 1000);
    fitFuncL_pulseIntCorr[iBar] -> SetLineColor(kRed);
    fitFuncL_pulseIntCorr[iBar] -> SetRange(0, 5*mpvIntL[iBar]);
    p_tL_vs_pulseInt[iBar] -> Fit(Form("fitFuncL_pulseIntCorr_%d",iBar),"QSR");
    if (fitFuncL_pulseIntCorr[iBar] -> GetChisquare()/fitFuncL_pulseIntCorr[iBar] -> GetNDF() > 5 ){
      fitFuncL_pulseIntCorr[iBar] -> SetRange(0, 4*mpvIntL[iBar]);
      p_tL_vs_pulseInt[iBar] -> Fit(Form("fitFuncL_pulseIntCorr_%d",iBar),"QSR");
      if (fitFuncL_pulseIntCorr[iBar] -> GetChisquare()/fitFuncL_pulseIntCorr[iBar] -> GetNDF() > 5 ){
	fitFuncL_pulseIntCorr[iBar] -> SetRange(0, 3*mpvIntL[iBar]);
	p_tL_vs_pulseInt[iBar] -> Fit(Form("fitFuncL_pulseIntCorr_%d",iBar),"QSR");
      }
    }

    fLandauIntR[iBar] = new TF1(Form("fLandauIntR_BAR%d",iBar),"landau", 0, 1000);
    h_pulseIntR[iBar]->Fit(fLandauIntR[iBar],"QR");
    mpvIntR[iBar] = (h_pulseIntSum_nocuts->GetFunction("fLandauIntSum"))->GetParameter(1);
    fitFuncR_pulseIntCorr[iBar] = new TF1(Form("fitFuncR_pulseIntCorr_%d", iBar), fitFunction.c_str(), 0, 1000);
    fitFuncR_pulseIntCorr[iBar] -> SetLineColor(kRed);
    fitFuncR_pulseIntCorr[iBar] -> SetRange(0, 5*mpvIntR[iBar]);
    p_tR_vs_pulseInt[iBar] -> Fit(Form("fitFuncR_pulseIntCorr_%d",iBar),"QSR");
    if (fitFuncR_pulseIntCorr[iBar] -> GetChisquare()/fitFuncR_pulseIntCorr[iBar] -> GetNDF() > 5 ){
      fitFuncR_pulseIntCorr[iBar] -> SetRange(0, 4*mpvIntR[iBar]);
      p_tR_vs_pulseInt[iBar] -> Fit(Form("fitFuncR_pulseIntCorr_%d",iBar),"QSR");
      if (fitFuncR_pulseIntCorr[iBar] -> GetChisquare()/fitFuncR_pulseIntCorr[iBar] -> GetNDF() > 5 ){
	fitFuncR_pulseIntCorr[iBar] -> SetRange(0, 3*mpvIntR[iBar]);
	p_tR_vs_pulseInt[iBar] -> Fit(Form("fitFuncR_pulseIntCorr_%d",iBar),"QSR");
      }
    }

    // amp corr binned in X
    for (int ibin =0; ibin < NBINSXAMPWALK[iBar]; ibin++){
      fitFuncL_pulseIntCorr_binned[iBar][ibin] = new TF1(Form("fitFuncL_pulseIntCorr_%d_%d",ibin,  iBar),fitFunction.c_str(), 0, 1000);
      fitFuncL_pulseIntCorr_binned[iBar][ibin] -> SetLineColor(kRed);
      fitFuncL_pulseIntCorr_binned[iBar][ibin] -> SetRange(0, 5*mpvIntL[iBar]);
      p_tL_vs_pulseInt_binned[iBar][ibin] -> Fit(Form("fitFuncL_pulseIntCorr_%d_%d",ibin, iBar),"QSR");
      if (fitFuncL_pulseIntCorr_binned[iBar][ibin] -> GetChisquare()/fitFuncL_pulseIntCorr_binned[iBar][ibin] -> GetNDF() > 5 ){
	fitFuncL_pulseIntCorr_binned[iBar][ibin] -> SetRange(0, 4*mpvIntL[iBar]);
	p_tL_vs_pulseInt_binned[iBar][ibin] -> Fit(Form("fitFuncL_pulseIntCorr_%d_%d",ibin, iBar),"QSR");
	if (fitFuncL_pulseIntCorr_binned[iBar][ibin] -> GetChisquare()/fitFuncL_pulseIntCorr_binned[iBar][ibin] -> GetNDF() > 5 ){
	  fitFuncL_pulseIntCorr_binned[iBar][ibin] -> SetRange(0, 3*mpvIntL[iBar]);
	  p_tL_vs_pulseInt_binned[iBar][ibin] -> Fit(Form("fitFuncL_pulseIntCorr_%d_%d",ibin, iBar),"QSR");
	}
      }
      
      fitFuncR_pulseIntCorr_binned[iBar][ibin] = new TF1(Form("fitFuncR_pulseIntCorr_%d_%d", ibin, iBar),fitFunction.c_str(), 0, 1000);
      fitFuncR_pulseIntCorr_binned[iBar][ibin] -> SetLineColor(kRed);
      fitFuncR_pulseIntCorr_binned[iBar][ibin] -> SetRange(0, 5*mpvIntR[iBar]);
      p_tR_vs_pulseInt_binned[iBar][ibin] -> Fit(Form("fitFuncR_pulseIntCorr_%d_%d",ibin,iBar),"QSR");
      if (fitFuncR_pulseIntCorr_binned[iBar][ibin] -> GetChisquare()/fitFuncR_pulseIntCorr_binned[iBar][ibin] -> GetNDF() > 5 ){
	fitFuncR_pulseIntCorr_binned[iBar][ibin] -> SetRange(0, 4*mpvIntR[iBar]);
	p_tR_vs_pulseInt_binned[iBar][ibin] -> Fit(Form("fitFuncR_pulseIntCorr_%d_%d",ibin,iBar),"QSR");
	if (fitFuncR_pulseIntCorr_binned[iBar][ibin] -> GetChisquare()/fitFuncR_pulseIntCorr_binned[iBar][ibin] -> GetNDF() > 5 ){
	  fitFuncR_pulseIntCorr_binned[iBar][ibin] -> SetRange(0, 3*mpvIntR[iBar]);
	  p_tR_vs_pulseInt_binned[iBar][ibin] -> Fit(Form("fitFuncR_pulseIntCorr_%d_%d",ibin,iBar),"QSR");
	}
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
      if (useTimeMaxMCP) tRef   = treeVars.t_time_max[(*treeVars.t_channelId)[timeChannelRef]];
      brmsRef = treeVars.t_b_rms[(*treeVars.t_channelId)[timeChannelRef]];

      // -- loop over bars
      for (int iBar = 0; iBar < NBARS; iBar++){

	ampL = treeVars.t_amp[(*treeVars.t_channelId)[ ampChannelsL[iBar] ]] * kAdcToV ;
	ampR = treeVars.t_amp[(*treeVars.t_channelId)[ ampChannelsR[iBar] ]] * kAdcToV ;

	tL   = treeVars.t_time[(*treeVars.t_channelId)[ timeChannelsL[iBar] ]+treeVars.t_LED];  
	tR   = treeVars.t_time[(*treeVars.t_channelId)[ timeChannelsR[iBar] ]+treeVars.t_LED];  

	brmsL = treeVars.t_b_rms[(*treeVars.t_channelId)[ timeChannelsL[iBar] ] ];
	brmsR = treeVars.t_b_rms[(*treeVars.t_channelId)[ timeChannelsR[iBar] ] ];

        pulseIntL = treeVars.t_charge_sig[(*treeVars.t_channelId)[ ampChannelsL[iBar] ]] * kAdcToV;
        pulseIntR = treeVars.t_charge_sig[(*treeVars.t_channelId)[ ampChannelsR[iBar] ]] * kAdcToV;
        pulseIntAve = 0.5 *(pulseIntL+pulseIntR);

	// -- select events
        if ( posX < cut_XminRef || posX > cut_XmaxRef ) continue;
        if ( posY < cut_YminRef || posY > cut_YmaxRef ) continue;
	if ( posX < cut_Xmin[iBar] || posX > cut_Xmax[iBar] ) continue;
	if ( posY < cut_Ymin[iBar] || posY > cut_Ymax[iBar] ) continue;

	if ( ampRef < cut_ampMinRef     ||  ampRef > cut_ampMaxRef ) continue;
	//if ( ampL   < cut_ampMinL[iBar] ||  ampL   > cut_ampMaxL[iBar] ) continue;
	//if ( ampR   < cut_ampMinR[iBar] ||  ampR   > cut_ampMaxR[iBar] ) continue;
	if ( pulseIntL   < cut_pulseIntMinL[iBar] ||  pulseIntL   > cut_pulseIntMaxL[iBar] ) continue;
        if ( pulseIntR   < cut_pulseIntMinR[iBar] ||  pulseIntR   > cut_pulseIntMaxR[iBar] ) continue;

	if ( tRef < cut_minTimeRef || tRef > cut_maxTimeRef ) continue;
	if ( tL   < cut_minTime[iBar]  || tL   > cut_maxTime[iBar]  ) continue;
	if ( tR   < cut_minTime[iBar]  || tR   > cut_maxTime[iBar]  ) continue;

	if (tL < tLmin[iBar] || tL > tLmax[iBar]) continue;
	if (tR < tRmin[iBar] || tR > tRmax[iBar]) continue;

	// -- remove very noisy events
	if (brmsRef > cut_brmsMaxRef) continue;
        if (brmsR > cut_brmsMaxTimeCh[iBar]) continue;
        if (brmsL > cut_brmsMaxTimeCh[iBar]) continue;

	if (applyMCPTimeCorrection) tRef = GetCorrectedMCPTime(tRef, ampRef*1000., posX);
	tL = tL - tRef;
        tR = tR - tRef;

	// -- amplitude walk correction 
	int xcbinamp = int ((posX - max(cut_Xmin[iBar], cut_XminRef))/cos(theta[iBar])/xbinWidthForAmpWalk );
        float tL_corr = tL - fitFuncL_ampCorr_binned[iBar][xcbinamp]->Eval(ampL);
        float tR_corr = tR - fitFuncR_ampCorr_binned[iBar][xcbinamp]->Eval(ampR); 
	float tAve_ampCorr  = ( tL_corr + tR_corr)/2 ;
	float tDiff_ampCorr = tR_corr - tL_corr;

	float tL_pulseIntCorr = tL - fitFuncL_pulseIntCorr_binned[iBar][xcbinamp]->Eval(pulseIntL);
        float tR_pulseIntCorr = tR - fitFuncR_pulseIntCorr_binned[iBar][xcbinamp]->Eval(pulseIntR);
        if (useAverageAmplitudeForTimeWalkCorr){
	  tL_pulseIntCorr = tL - fitFuncL_pulseIntCorr_binned[iBar][xcbinamp]->Eval(pulseIntAve);
	  tR_pulseIntCorr = tR - fitFuncR_pulseIntCorr_binned[iBar][xcbinamp]->Eval(pulseIntAve);
	  }
	float tAve_pulseIntCorr = ( tL_pulseIntCorr + tR_pulseIntCorr)/2 ;
        float tDiff_pulseIntCorr = tR_pulseIntCorr - tL_pulseIntCorr;


	// -- tDiff vs position X
	h2_tDiff_ampCorr_vs_posX[iBar] ->Fill(posX, tDiff_ampCorr);
	h2_tDiff_vs_posX[iBar] ->Fill(posX, tR-tL);

	// -- SiPM left
	p_ampL_vs_tDiff[iBar] ->Fill(tDiff_ampCorr,ampL);
	p_tL_vs_tDiff[iBar] ->Fill(tDiff_ampCorr,tL);
	h_tL_ampCorr[iBar]->Fill(tL_corr);
        p_tL_ampCorr_vs_amp[iBar]->Fill(ampL,tL_corr);
        p_tL_ampCorr_vs_posX[iBar]->Fill(posX,tL_corr);
	if ( tL > h_tL[iBar]->GetMean()-3*h_tL[iBar]->GetRMS() && tL < h_tL[iBar]->GetMean()+3*h_tL[iBar]->GetRMS() ){
	  p_tL_ampCorr_vs_posXc[iBar]->Fill(posX/cos(theta[iBar]),tL_corr);
	  int xcbinamp = int ((posX - max(cut_Xmin[iBar], cut_XminRef))/cos(theta[iBar])/xbinWidthForAmpWalk );
          p_tL_ampCorr_vs_amp_binned[iBar][xcbinamp] ->Fill(ampL,tL_corr);
	}
        p_tL_ampCorr_vs_posY[iBar]->Fill(posY,tL_corr);
        p_tL_ampCorr_vs_tDiff[iBar]->Fill(tDiff_ampCorr, tL_corr);

	h_tL_pulseIntCorr[iBar]->Fill(tL_pulseIntCorr);
        p_tL_pulseIntCorr_vs_pulseInt[iBar]->Fill(pulseIntL,tL_pulseIntCorr);
        p_tL_pulseIntCorr_vs_posX[iBar]->Fill(posX,tL_pulseIntCorr);
        if ( tL > h_tL[iBar]->GetMean()-3*h_tL[iBar]->GetRMS() && tL < h_tL[iBar]->GetMean()+3*h_tL[iBar]->GetRMS() ) {
          p_tL_pulseIntCorr_vs_posXc[iBar]->Fill(posX/cos(theta[iBar]),tL_pulseIntCorr);
        }
        p_tL_pulseIntCorr_vs_posY[iBar]->Fill(posY,tL_pulseIntCorr);
        p_tL_pulseIntCorr_vs_tDiff[iBar]->Fill(tDiff_pulseIntCorr, tL_pulseIntCorr);

	// -- SiPM right
	p_ampR_vs_tDiff[iBar] ->Fill(tDiff_ampCorr,ampR);
	p_tR_vs_tDiff[iBar] ->Fill(tDiff_ampCorr,tR);
	h_tR_ampCorr[iBar]->Fill(tR_corr);
        p_tR_ampCorr_vs_amp[iBar]->Fill(ampR,tR_corr);
        p_tR_ampCorr_vs_posX[iBar]->Fill(posX,tR_corr);
	if ( tR > h_tR[iBar]->GetMean()-3*h_tR[iBar]->GetRMS() && tR < h_tR[iBar]->GetMean()+3*h_tR[iBar]->GetRMS() ) {
	  p_tR_ampCorr_vs_posXc[iBar]->Fill(posX/cos(theta[iBar]),tR_corr);
	  int xcbinamp = int ((posX - max(cut_Xmin[iBar], cut_XminRef))/cos(theta[iBar])/xbinWidthForAmpWalk );
          p_tR_ampCorr_vs_amp_binned[iBar][xcbinamp] ->Fill(ampR,tR_corr);
	}
        p_tR_ampCorr_vs_posY[iBar]->Fill(posY,tR_corr);
        p_tR_ampCorr_vs_tDiff[iBar]->Fill(tDiff_ampCorr, tR_corr);

        h_tR_pulseIntCorr[iBar]->Fill(tR_pulseIntCorr);
        p_tR_pulseIntCorr_vs_pulseInt[iBar]->Fill(pulseIntR,tR_pulseIntCorr);
        p_tR_pulseIntCorr_vs_posX[iBar]->Fill(posX,tR_pulseIntCorr);
        if ( tR > h_tR[iBar]->GetMean()-3*h_tR[iBar]->GetRMS() && tR < h_tR[iBar]->GetMean()+3*h_tR[iBar]->GetRMS() ) {
          p_tR_pulseIntCorr_vs_posXc[iBar]->Fill(posX/cos(theta[iBar]),tR_pulseIntCorr);
        }
        p_tR_pulseIntCorr_vs_posY[iBar]->Fill(posY,tR_pulseIntCorr);
        p_tR_pulseIntCorr_vs_tDiff[iBar]->Fill(tDiff_ampCorr, tR_pulseIntCorr);

	// -- tDiff
	h_tDiff_ampCorr[iBar]->Fill(tDiff_ampCorr);
	p_tDiff_ampCorr_vs_posX[iBar]->Fill(posX,tDiff_ampCorr);
        p_tDiff_ampCorr_vs_posXc[iBar]->Fill(posX/cos(theta[iBar]),tDiff_ampCorr);

	h_tDiff_pulseIntCorr[iBar]->Fill(tDiff_pulseIntCorr);
        p_tDiff_pulseIntCorr_vs_posX[iBar]->Fill(posX,tDiff_pulseIntCorr);
        p_tDiff_pulseIntCorr_vs_posXc[iBar]->Fill(posX/cos(theta[iBar]),tDiff_pulseIntCorr);


	// -- simple average
	p_tAve_vs_tDiff[iBar] ->Fill(tDiff_ampCorr, tAve);
	h_tAve_ampCorr[iBar]->Fill(tAve_ampCorr);
	p_tAve_ampCorr_vs_posX[iBar]->Fill(posX,tAve_ampCorr);
	p_tAve_ampCorr_vs_posXc[iBar]->Fill(posX/cos(theta[iBar]),tAve_ampCorr);
        p_tAve_ampCorr_vs_posY[iBar]->Fill(posY,tAve_ampCorr);
        p_tAve_ampCorr_vs_tDiff[iBar]->Fill(tDiff_ampCorr, tAve_ampCorr);


	h_tAve_pulseIntCorr[iBar]->Fill(tAve_pulseIntCorr);
        p_tAve_pulseIntCorr_vs_posX[iBar]->Fill(posX,tAve_pulseIntCorr);
        p_tAve_pulseIntCorr_vs_posXc[iBar]->Fill(posX/cos(theta[iBar]),tAve_pulseIntCorr);
        p_tAve_pulseIntCorr_vs_posY[iBar]->Fill(posY,tAve_pulseIntCorr);
        p_tAve_pulseIntCorr_vs_tDiff[iBar]->Fill(tDiff_pulseIntCorr, tAve_pulseIntCorr);

	int xbin = int ((posX-cut_Xmin[iBar])/xbinWidth );
	int ybin = int ((posY-cut_Ymin[NBARS-1])/ybinWidth );

	h_ampDiff_binX[iBar][xbin]->Fill(ampL-ampR);
	h_tL_ampCorr_binX[iBar][xbin] ->Fill(tL_corr);
	h_tR_ampCorr_binX[iBar][xbin] ->Fill(tR_corr);
	h_tAve_ampCorr_binX[iBar][xbin] ->Fill(tAve_ampCorr);
	h_tAve_ampCorr_binY[iBar][ybin] ->Fill(tAve_ampCorr);
	h_tDiff_ampCorr_binX[iBar][xbin]->Fill(tDiff_ampCorr);
	h_tDiff_ampCorr_binY[iBar][ybin]->Fill(tDiff_ampCorr);

	h_tL_pulseIntCorr_binX[iBar][xbin] ->Fill(tL_pulseIntCorr);
        h_tR_pulseIntCorr_binX[iBar][xbin] ->Fill(tR_pulseIntCorr);
        h_tAve_pulseIntCorr_binX[iBar][xbin] ->Fill(tAve_pulseIntCorr);
        h_tAve_pulseIntCorr_binY[iBar][ybin] ->Fill(tAve_pulseIntCorr);
        h_tDiff_pulseIntCorr_binX[iBar][xbin]->Fill(tDiff_pulseIntCorr);
        h_tDiff_pulseIntCorr_binY[iBar][ybin]->Fill(tDiff_pulseIntCorr);


      } // -- end loop over bars

    }// -- end second loop over events
  std::cout<<" >>> End second loop over events" <<std::endl;
  
  

  // -- Third loop over events - correction for residual tAve dependence on tDiff or position
  float mean, rms;

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
    fitFuncDiff_posCorr[iBar] = new TF1(Form("fitFuncDiff_posCorr_%d", iBar),"pol1");
    mean = p_tDiff_ampCorr_vs_posX[iBar] -> GetMean();
    rms  = p_tDiff_ampCorr_vs_posX[iBar] -> GetRMS();
    fitFuncDiff_posCorr[iBar]->SetRange( mean - 5*rms , mean + 5*rms);
    fitFuncDiff_posCorr[iBar] -> SetLineColor(kRed);
    p_tDiff_ampCorr_vs_posX[iBar] -> Fit(Form("fitFuncDiff_posCorr_%d",iBar),"QSR");
  }

  TF1*  fitFuncDiff_posCorr_noAmpCorr[NBARS];
  for (int iBar = 0 ; iBar < NBARS; iBar++){
    fitFuncDiff_posCorr_noAmpCorr[iBar] = new TF1(Form("fitFuncDiff_posCorr_noAmpCorr_%d", iBar),"pol1");
    mean = p_tDiff_vs_posX[iBar] -> GetMean();
    rms  = p_tDiff_vs_posX[iBar] -> GetRMS();
    fitFuncDiff_posCorr_noAmpCorr[iBar]->SetRange( mean - 5*rms , mean + 5*rms);
    fitFuncDiff_posCorr_noAmpCorr[iBar] -> SetLineColor(kRed);
    p_tDiff_vs_posX[iBar] -> Fit(Form("fitFuncDiff_posCorr_noAmpCorr_%d",iBar),"QSR");
  }
  
  TF1*  fitFuncAve_pulseIntCorr_posCorr[NBARS];
  for (int iBar = 0 ; iBar < NBARS; iBar++){
    fitFuncAve_pulseIntCorr_posCorr[iBar] = new TF1(Form("fitFuncAve_pulseIntCorr_posCorr_%d", iBar),"pol4");
    mean = p_tAve_pulseIntCorr_vs_posX[iBar] -> GetMean();
    rms  = p_tAve_pulseIntCorr_vs_posX[iBar] -> GetRMS();
    fitFuncAve_pulseIntCorr_posCorr[iBar]->SetRange( mean - 5.0*rms , mean + 5.0*rms);
    fitFuncAve_pulseIntCorr_posCorr[iBar] -> SetLineColor(kRed);
    p_tAve_pulseIntCorr_vs_posX[iBar] -> Fit(Form("fitFuncAve_pulseIntCorr_posCorr_%d",iBar),"QSR");
  }

  TF1*  fitFuncDiff_pulseIntCorr_posCorr[NBARS];
  for (int iBar = 0 ; iBar < NBARS; iBar++){
    fitFuncDiff_pulseIntCorr_posCorr[iBar] = new TF1(Form("fitFuncDiff_pulseIntCorr_posCorr_%d", iBar),"pol1");
    mean = p_tDiff_pulseIntCorr_vs_posX[iBar] -> GetMean();
    rms  = p_tDiff_pulseIntCorr_vs_posX[iBar] -> GetRMS();
    fitFuncDiff_pulseIntCorr_posCorr[iBar]->SetRange( mean - 5*rms , mean + 5*rms);
    fitFuncDiff_pulseIntCorr_posCorr[iBar] -> SetLineColor(kRed);
    p_tDiff_pulseIntCorr_vs_posX[iBar] -> Fit(Form("fitFuncDiff_pulseIntCorr_posCorr_%d",iBar),"QSR");
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
      if (useTimeMaxMCP) tRef   = treeVars.t_time_max[(*treeVars.t_channelId)[timeChannelRef]];
      brmsRef = treeVars.t_b_rms[(*treeVars.t_channelId)[timeChannelRef]];

      // -- loop over bars
      for (int iBar = 0; iBar < NBARS; iBar++){
	
        ampL = treeVars.t_amp[(*treeVars.t_channelId)[ ampChannelsL[iBar] ]] * kAdcToV ;
        ampR = treeVars.t_amp[(*treeVars.t_channelId)[ ampChannelsR[iBar] ]] * kAdcToV ;
	
        tL   = treeVars.t_time[(*treeVars.t_channelId)[ timeChannelsL[iBar] ]+treeVars.t_LED];
        tR   = treeVars.t_time[(*treeVars.t_channelId)[ timeChannelsR[iBar] ]+treeVars.t_LED];

	brmsL = treeVars.t_b_rms[(*treeVars.t_channelId)[ timeChannelsL[iBar] ] ];
	brmsR = treeVars.t_b_rms[(*treeVars.t_channelId)[ timeChannelsR[iBar] ] ];

	pulseIntL = treeVars.t_charge_sig[(*treeVars.t_channelId)[ ampChannelsL[iBar] ]] * kAdcToV;
        pulseIntR = treeVars.t_charge_sig[(*treeVars.t_channelId)[ ampChannelsR[iBar] ]] * kAdcToV;
        pulseIntAve = 0.5 * (pulseIntL + pulseIntR);

	// -- select events
        if ( posX < cut_XminRef || posX > cut_XmaxRef ) continue;
        if ( posY < cut_YminRef || posY > cut_YmaxRef ) continue;
        if ( posX < cut_Xmin[iBar] || posX > cut_Xmax[iBar] ) continue;
        if ( posY < cut_Ymin[iBar] || posY > cut_Ymax[iBar] ) continue;
	
        if ( ampRef < cut_ampMinRef     ||  ampRef > cut_ampMaxRef ) continue;
        //if ( ampL   < cut_ampMinL[iBar] ||  ampL   > cut_ampMaxL[iBar] ) continue;
        //if ( ampR   < cut_ampMinR[iBar] ||  ampR   > cut_ampMaxR[iBar] ) continue;
	if ( pulseIntL   < cut_pulseIntMinL[iBar] ||  pulseIntL   > cut_pulseIntMaxL[iBar] ) continue;
        if ( pulseIntR   < cut_pulseIntMinR[iBar] ||  pulseIntR   > cut_pulseIntMaxR[iBar] ) continue;

        if ( tRef < cut_minTimeRef || tRef > cut_maxTimeRef ) continue;
        if ( tL   < cut_minTime[iBar]  || tL   > cut_maxTime[iBar]  ) continue;
        if ( tR   < cut_minTime[iBar]  || tR   > cut_maxTime[iBar]  ) continue;

	if (tL < tLmin[iBar] || tL > tLmax[iBar]) continue;
	if (tR < tRmin[iBar] || tR > tRmax[iBar]) continue;

	// -- remove very noisy events
	if (brmsRef > cut_brmsMaxRef) continue;
        if (brmsR > cut_brmsMaxTimeCh[iBar]) continue;
        if (brmsL > cut_brmsMaxTimeCh[iBar]) continue;

	if (applyMCPTimeCorrection) tRef = GetCorrectedMCPTime(tRef, ampRef*1000., posX);
        tL = tL - tRef;
        tR = tR - tRef;

        // -- amplitude walk correction
	int xcbinamp = int ((posX - max(cut_Xmin[iBar], cut_XminRef))/cos(theta[iBar])/xbinWidthForAmpWalk );
        float tL_corr = tL - fitFuncL_ampCorr_binned[iBar][xcbinamp]->Eval(ampL);
        float tR_corr = tR - fitFuncR_ampCorr_binned[iBar][xcbinamp]->Eval(ampR);
        float tAve_ampCorr  = ( tL_corr + tR_corr)/2 ;
        float tDiff = tR_corr - tL_corr;
	float tAve_ampCorr_posCorr  = tAve_ampCorr - fitFuncAve_posCorr[iBar]->Eval(posX);
	float tDiff_ampCorr_posCorr = tDiff - fitFuncDiff_posCorr[iBar]->Eval(posX);
	
	// -- amp walk correction vs pulse integral
	float tL_pulseIntCorr = tL - fitFuncL_pulseIntCorr_binned[iBar][xcbinamp]->Eval(pulseIntL);
        float tR_pulseIntCorr = tR - fitFuncR_pulseIntCorr_binned[iBar][xcbinamp]->Eval(pulseIntR);
	if (useAverageAmplitudeForTimeWalkCorr){
          tL_pulseIntCorr = tL - fitFuncL_pulseIntCorr_binned[iBar][xcbinamp]->Eval(pulseIntAve);
          tR_pulseIntCorr = tR - fitFuncR_pulseIntCorr_binned[iBar][xcbinamp]->Eval(pulseIntAve);
	}
	float tAve_pulseIntCorr = ( tL_pulseIntCorr + tR_pulseIntCorr )/2 ;
        float tDiff_pulseIntCorr = tR_pulseIntCorr - tL_pulseIntCorr;
	float tAve_pulseIntCorr_posCorr  = tAve_pulseIntCorr - fitFuncAve_pulseIntCorr_posCorr[iBar]->Eval(posX);
        float tDiff_pulseIntCorr_posCorr = tDiff_pulseIntCorr - fitFuncDiff_pulseIntCorr_posCorr[iBar]->Eval(posX);
	
	// -- pos correction only
	float tDiff_posCorr = tR - tL - fitFuncDiff_posCorr_noAmpCorr[iBar]->Eval(posX);
	
	int xbin = int ((posX - max(cut_Xmin[iBar], cut_XminRef)) /xbinWidth );
	int ybin = int ((posY-cut_Ymin[NBARS-1])/ybinWidth );
	
	// -- simple average
        h_tAve_ampCorr_posCorr[iBar]->Fill(tAve_ampCorr_posCorr);
        p_tAve_ampCorr_posCorr_vs_posX[iBar]->Fill(posX,tAve_ampCorr_posCorr);
        p_tAve_ampCorr_posCorr_vs_posXc[iBar]->Fill(posX/cos(theta[iBar]),tAve_ampCorr_posCorr);
        p_tAve_ampCorr_posCorr_vs_posY[iBar]->Fill(posY,tAve_ampCorr_posCorr);
        p_tAve_ampCorr_posCorr_vs_tDiff[iBar]->Fill(tDiff, tAve_ampCorr_posCorr);
	h_tAve_ampCorr_posCorr_binX[iBar][xbin] ->Fill(tAve_ampCorr_posCorr);
	h_tAve_ampCorr_posCorr_binY[iBar][ybin] ->Fill(tAve_ampCorr_posCorr);

        h_tAve_pulseIntCorr_posCorr[iBar]->Fill(tAve_pulseIntCorr_posCorr);
        p_tAve_pulseIntCorr_posCorr_vs_posX[iBar]->Fill(posX,tAve_pulseIntCorr_posCorr);
        p_tAve_pulseIntCorr_posCorr_vs_posXc[iBar]->Fill(posX/cos(theta[iBar]),tAve_pulseIntCorr_posCorr);
        p_tAve_pulseIntCorr_posCorr_vs_posY[iBar]->Fill(posY,tAve_pulseIntCorr_posCorr);
        p_tAve_pulseIntCorr_posCorr_vs_tDiff[iBar]->Fill(tDiff_pulseIntCorr, tAve_pulseIntCorr_posCorr);
        h_tAve_pulseIntCorr_posCorr_binX[iBar][xbin] ->Fill(tAve_pulseIntCorr_posCorr);
        h_tAve_pulseIntCorr_posCorr_binY[iBar][ybin] ->Fill(tAve_pulseIntCorr_posCorr);

	// -- tDiff vs posX
        h_tDiff_posCorr[iBar]->Fill(tDiff_posCorr);
        h_tDiff_posCorr_binX[iBar][xbin]->Fill(tDiff_posCorr);
        h_tDiff_posCorr_binY[iBar][ybin]->Fill(tDiff_posCorr);
        p_tDiff_posCorr_vs_posX[iBar]->Fill(posX,tDiff_posCorr);
        p_tDiff_posCorr_vs_posXc[iBar]->Fill(posX/cos(theta[iBar]),tDiff_posCorr);

        h_tDiff_ampCorr_posCorr[iBar]->Fill(tDiff_ampCorr_posCorr);
        p_tDiff_ampCorr_posCorr_vs_posX[iBar]->Fill(posX,tDiff_ampCorr_posCorr);
        p_tDiff_ampCorr_posCorr_vs_posXc[iBar]->Fill(posX/cos(theta[iBar]),tDiff_ampCorr_posCorr);
        p_tDiff_ampCorr_posCorr_vs_posY[iBar]->Fill(posY,tDiff_ampCorr_posCorr);
        h_tDiff_ampCorr_posCorr_binX[iBar][xbin] ->Fill(tDiff_ampCorr_posCorr);
        h_tDiff_ampCorr_posCorr_binY[iBar][ybin] ->Fill(tDiff_ampCorr_posCorr);

        h_tDiff_pulseIntCorr_posCorr[iBar]->Fill(tDiff_pulseIntCorr_posCorr);
        p_tDiff_pulseIntCorr_posCorr_vs_posX[iBar]->Fill(posX,tDiff_pulseIntCorr_posCorr);
        p_tDiff_pulseIntCorr_posCorr_vs_posXc[iBar]->Fill(posX/cos(theta[iBar]),tDiff_pulseIntCorr_posCorr);
        h_tDiff_pulseIntCorr_posCorr_binX[iBar][xbin] ->Fill(tDiff_pulseIntCorr_posCorr);
        h_tDiff_pulseIntCorr_posCorr_binY[iBar][ybin] ->Fill(tDiff_pulseIntCorr_posCorr);

	int ampBin = int ((pulseIntAve/pulseIntMPV)/ampBinWidth);
	if (pulseIntAve>0)
	  h_tDiff_pulseIntCorr_posCorr_binAmp[iBar][ampBin] ->Fill(tDiff_pulseIntCorr_posCorr);

      }
      
    } //--- end third loop over events

  std::cout<<" >>> End third loop over events " <<std::endl;

  cout << "Calculating time resolution for each bar... " <<endl;
  // -- Compute resolutions (gaussian and effective sigma)
  TF1*  fitFunL_ampCorr[NBARS];
  TF1*  fitFunR_ampCorr[NBARS];
  TF1*  fitFunL_pulseIntCorr[NBARS];
  TF1*  fitFunR_pulseIntCorr[NBARS];
  
  TF1*  fitFunAve_ampCorr[NBARS];
  TF1*  fitFunAve_ampCorr_posCorr[NBARS];
  TF1*  fitFunAve_pulseIntCorr[NBARS];
  TF1*  fitFunAve_pulseIntCorr_posCorr[NBARS];

  TF1*  fitFunDiff_ampCorr[NBARS];
  TF1*  fitFunDiff_ampCorr_posCorr[NBARS];
  TF1*  fitFunDiff_pulseIntCorr[NBARS];
  TF1*  fitFunDiff_pulseIntCorr_posCorr[NBARS];
  
  TF1*  fitFunL_ampCorr_binX[NBARS][1000];
  TF1*  fitFunR_ampCorr_binX[NBARS][1000];
  TF1*  fitFunL_pulseIntCorr_binX[NBARS][1000];
  TF1*  fitFunR_pulseIntCorr_binX[NBARS][1000];
  
  TF1*  fitFunAve_ampCorr_binX[NBARS][1000];
  TF1*  fitFunAve_ampCorr_posCorr_binX[NBARS][1000];
  TF1*  fitFunAve_pulseIntCorr_binX[NBARS][1000];
  TF1*  fitFunAve_pulseIntCorr_posCorr_binX[NBARS][1000];
  
  TF1*  fitFunAve_ampCorr_binY[NBARS][1000];
  TF1*  fitFunAve_ampCorr_posCorr_binY[NBARS][1000];
  TF1*  fitFunAve_pulseIntCorr_binY[NBARS][1000];
  TF1*  fitFunAve_pulseIntCorr_posCorr_binY[NBARS][1000];

  TF1*  fitFunDiff_binX[NBARS][1000];
  TF1*  fitFunDiff_posCorr_binX[NBARS][1000];
  TF1*  fitFunDiff_ampCorr_binX[NBARS][1000];
  TF1*  fitFunDiff_ampCorr_posCorr_binX[NBARS][1000];
  TF1*  fitFunDiff_pulseIntCorr_binX[NBARS][1000];
  TF1*  fitFunDiff_pulseIntCorr_posCorr_binX[NBARS][1000];

  TF1*  fitFunDiff_ampCorr_binY[NBARS][1000];
  TF1*  fitFunDiff_ampCorr_posCorr_binY[NBARS][1000];
  TF1*  fitFunDiff_pulseIntCorr_binY[NBARS][1000];
  TF1*  fitFunDiff_pulseIntCorr_posCorr_binY[NBARS][1000];
  TF1*  fitFunDiff_pulseIntCorr_posCorr_binAmp[NBARS][1000];

  TH1F*  h_effectiveSigmaL[NBARS];
  TH1F*  h_effectiveSigmaL_ampCorr[NBARS];
  TH1F*  h_effectiveSigmaL_pulseIntCorr[NBARS];
  TH1F*  h_effectiveSigmaR[NBARS];
  TH1F*  h_effectiveSigmaR_ampCorr[NBARS];
  TH1F*  h_effectiveSigmaR_pulseIntCorr[NBARS];
  TH1F*  h_effectiveSigmaAve[NBARS];
  TH1F*  h_effectiveSigmaAve_ampCorr[NBARS];
  TH1F*  h_effectiveSigmaAve_ampCorr_posCorr[NBARS];
  TH1F*  h_effectiveSigmaAve_pulseIntCorr[NBARS];
  TH1F*  h_effectiveSigmaAve_pulseIntCorr_posCorr[NBARS];
  TH1F*  h_effectiveSigmaDiff_ampCorr[NBARS];
  TH1F*  h_effectiveSigmaDiff_ampCorr_posCorr[NBARS];
  TH1F*  h_effectiveSigmaDiff_pulseIntCorr[NBARS];
  TH1F*  h_effectiveSigmaDiff_pulseIntCorr_posCorr[NBARS];
  
  
  TGraphErrors *g_tResolGausL_ampCorr[NBARS];
  TGraphErrors *g_tResolGausR_ampCorr[NBARS];

  TGraphErrors *g_tResolGausAve_ampCorr[NBARS];
  TGraphErrors *g_tResolGausAve_ampCorr_posCorr[NBARS];
  TGraphErrors *g_tResolGausDiff_ampCorr[NBARS];
  TGraphErrors *g_tResolGausDiff_ampCorr_posCorr[NBARS];
  TGraphErrors *g_tResolGausAve_ampCorr_vs_posY[NBARS];
  TGraphErrors *g_tResolGausAve_ampCorr_posCorr_vs_posY[NBARS];
  TGraphErrors *g_tResolGausDiff_ampCorr_vs_posY[NBARS];
  TGraphErrors *g_tResolGausDiff_ampCorr_posCorr_vs_posY[NBARS];

  TGraphErrors *g_tResolGausL_pulseIntCorr[NBARS];
  TGraphErrors *g_tResolGausR_pulseIntCorr[NBARS];

  TGraphErrors *g_tResolGausAve_pulseIntCorr[NBARS];
  TGraphErrors *g_tResolGausAve_pulseIntCorr_posCorr[NBARS];
  TGraphErrors *g_tResolGausDiff_pulseIntCorr[NBARS];
  TGraphErrors *g_tResolGausDiff_pulseIntCorr_posCorr[NBARS];
  TGraphErrors *g_tResolGausAve_pulseIntCorr_vs_posY[NBARS];
  TGraphErrors *g_tResolGausAve_pulseIntCorr_posCorr_vs_posY[NBARS];
  TGraphErrors *g_tResolGausDiff_pulseIntCorr_vs_posY[NBARS];
  TGraphErrors *g_tResolGausDiff_pulseIntCorr_posCorr_vs_posY[NBARS];
  TGraphErrors *g_tResolGausDiff_pulseIntCorr_posCorr_vs_amp[NBARS];

  TGraphErrors *g_tResolGausDiff[NBARS];
  TGraphErrors *g_tResolGausDiff_posCorr[NBARS];
  
  float* vals = new float[4];
  float sigmaEff;
  float sigmaGaus;   
  float sigmaGausErr;
  float resolEff;
  float resolGaus;
  float resolGausErr;
  float resolMCP = 0.;
  //float resolMCP = 0.0123; // after amp/position walk corrections 

  TF1 *fpol0_tL_ampCorr[NBARS]; 
  TF1 *fpol0_tR_ampCorr[NBARS]; 
  
  TF1 *fpol0_tAve_ampCorr[NBARS]; 
  TF1 *fpol0_tAve_ampCorr_posCorr[NBARS]; 
  
  TF1 *fpol0_tDiff_ampCorr[NBARS]; 
  TF1 *fpol0_tDiff_ampCorr_posCorr[NBARS]; 
  
  TF1 *fpol0_tL_pulseIntCorr[NBARS];
  TF1 *fpol0_tR_pulseIntCorr[NBARS];

  TF1 *fpol0_tAve_pulseIntCorr[NBARS];
  TF1 *fpol0_tAve_pulseIntCorr_posCorr[NBARS];

  TF1 *fpol0_tDiff_pulseIntCorr[NBARS];
  TF1 *fpol0_tDiff_pulseIntCorr_posCorr[NBARS];

  for (int iBar = 0; iBar < NBARS; iBar++){
    h_effectiveSigmaL[iBar]  = new TH1F(Form("h_effectiveSigmaL_BAR%d",iBar),Form("h_effectiveSigmaL_BAR%d",iBar),2000,0.,200.);
    h_effectiveSigmaL_ampCorr[iBar]  = new TH1F(Form("h_effectiveSigmaL_ampCorr_BAR%d",iBar),Form("h_effectiveSigmaL_ampCorr_BAR%d",iBar), 2000,0.,200.);
    
    h_effectiveSigmaR[iBar]  = new TH1F(Form("h_effectiveSigmaR_BAR%d",iBar),Form("h_effectiveSigmaR_BAR%d",iBar),2000,0.,200.);
    h_effectiveSigmaR_ampCorr[iBar]  = new TH1F(Form("h_effectiveSigmaR_ampCorr_BAR%d",iBar),Form("h_effectiveSigmaR_ampCorr_BAR%d",iBar), 2000,0.,200.);
    
    h_effectiveSigmaAve[iBar]  = new TH1F(Form("h_effectiveSigmaAve_BAR%d",iBar),Form("h_effectiveSigmaAve_BAR%d",iBar),2000,0.,200.);
    h_effectiveSigmaAve_ampCorr[iBar]  = new TH1F(Form("h_effectiveSigmaAve_ampCorr_BAR%d",iBar),Form("h_effectiveSigmaAve_ampCorr_BAR%d",iBar), 2000,0.,200.);
    h_effectiveSigmaAve_ampCorr_posCorr[iBar]  = new TH1F(Form("h_effectiveSigmaAve_ampCorr_posCorr_BAR%d",iBar),Form("h_effectiveSigmaAve_ampCorr_posCorr_BAR%d",iBar), 2000,0.,200.);
    h_effectiveSigmaDiff_ampCorr[iBar]  = new TH1F(Form("h_effectiveSigmaDiff_ampCorr_BAR%d",iBar),Form("h_effectiveSigmaDiff_ampCorr_BAR%d",iBar), 2000,0.,200.);
    h_effectiveSigmaDiff_ampCorr_posCorr[iBar]  = new TH1F(Form("h_effectiveSigmaDiff_ampCorr_posCorr_BAR%d",iBar),Form("h_effectiveSigmaDiff_ampCorr_posCorr_BAR%d",iBar), 2000,0.,200.);
  
    h_effectiveSigmaL_pulseIntCorr[iBar]  = new TH1F(Form("h_effectiveSigmaL_pulseIntCorr_BAR%d",iBar),Form("h_effectiveSigmaL_pulseIntCorr_BAR%d",iBar), 2000,0.,200.);
    h_effectiveSigmaR_pulseIntCorr[iBar]  = new TH1F(Form("h_effectiveSigmaR_pulseIntCorr_BAR%d",iBar),Form("h_effectiveSigmaR_pulseIntCorr_BAR%d",iBar), 2000,0.,200.);
    h_effectiveSigmaAve_pulseIntCorr[iBar]  = new TH1F(Form("h_effectiveSigmaAve_pulseIntCorr_BAR%d",iBar),Form("h_effectiveSigmaAve_pulseIntCorr_BAR%d",iBar), 2000,0.,200.);
    h_effectiveSigmaAve_pulseIntCorr_posCorr[iBar]  = new TH1F(Form("h_effectiveSigmaAve_pulseIntCorr_posCorr_BAR%d",iBar),Form("h_effectiveSigmaAve_pulseIntCorr_posCorr_BAR%d",iBar), 2000,0.,200.);
    h_effectiveSigmaDiff_pulseIntCorr[iBar]  = new TH1F(Form("h_effectiveSigmaDiff_pulseIntCorr_BAR%d",iBar),Form("h_effectiveSigmaDiff_pulseIntCorr_BAR%d",iBar), 2000,0.,200.);
    h_effectiveSigmaDiff_pulseIntCorr_posCorr[iBar]  = new TH1F(Form("h_effectiveSigmaDiff_pulseIntCorr_posCorr_BAR%d",iBar),Form("h_effectiveSigmaDiff_pulseIntCorr_posCorr_BAR%d",iBar), 2000,0.,200.);
  
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
    GetTimeResolution(h_tL_ampCorr[iBar], fitFunL_ampCorr[iBar], sigmaEff, sigmaGaus, sigmaGausErr, -1);
    h_effectiveSigmaL_ampCorr[iBar]->Fill(sigmaEff);
    
    // -- right
    fitFunR_ampCorr[iBar]= new TF1(Form("fitFunR_ampCorr_BAR%d",iBar),"gaus", -20, 20);
    GetTimeResolution(h_tR_ampCorr[iBar], fitFunR_ampCorr[iBar], sigmaEff, sigmaGaus, sigmaGausErr, -1);
    h_effectiveSigmaR_ampCorr[iBar]->Fill(sigmaEff);
    
    // -- simple average
    fitFunAve_ampCorr[iBar]= new TF1(Form("fitFunAve_ampCorr_BAR%d",iBar),"gaus", -20, 20);
    GetTimeResolution(h_tAve_ampCorr[iBar], fitFunAve_ampCorr[iBar], sigmaEff, sigmaGaus, sigmaGausErr, -1);
    h_effectiveSigmaAve_ampCorr[iBar]->Fill(sigmaEff);
    
    // ----- amp walk corr vs pulse integral
    // -- Left
    fitFunL_pulseIntCorr[iBar]= new TF1(Form("fitFunL_pulseIntCorr_BAR%d",iBar),"gaus", -20, 20);
    GetTimeResolution(h_tL_pulseIntCorr[iBar], fitFunL_pulseIntCorr[iBar], sigmaEff, sigmaGaus, sigmaGausErr, -1);
    h_effectiveSigmaL_pulseIntCorr[iBar]->Fill(sigmaEff);

    // -- right
    fitFunR_pulseIntCorr[iBar]= new TF1(Form("fitFunR_pulseIntCorr_BAR%d",iBar),"gaus", -20, 20);
    GetTimeResolution(h_tR_pulseIntCorr[iBar], fitFunR_pulseIntCorr[iBar], sigmaEff, sigmaGaus, sigmaGausErr, -1);
    h_effectiveSigmaR_pulseIntCorr[iBar]->Fill(sigmaEff);

    // -- simple average
    fitFunAve_pulseIntCorr[iBar]= new TF1(Form("fitFunAve_pulseIntCorr_BAR%d",iBar),"gaus", -20, 20);
    GetTimeResolution(h_tAve_pulseIntCorr[iBar], fitFunAve_pulseIntCorr[iBar], sigmaEff, sigmaGaus, sigmaGausErr, -1);
    h_effectiveSigmaAve_pulseIntCorr[iBar]->Fill(sigmaEff);
    
    
    // ----- position corr
    // -- simple average
    fitFunAve_ampCorr_posCorr[iBar]= new TF1(Form("fitFunAve_ampCorr_posCorr_BAR%d",iBar),"gaus", -20, 20);
    GetTimeResolution(h_tAve_ampCorr_posCorr[iBar], fitFunAve_ampCorr_posCorr[iBar], sigmaEff, sigmaGaus, sigmaGausErr, -1);
    h_effectiveSigmaAve_ampCorr_posCorr[iBar]->Fill(sigmaEff);

    fitFunAve_pulseIntCorr_posCorr[iBar]= new TF1(Form("fitFunAve_pulseIntCorr_posCorr_BAR%d",iBar),"gaus", -20, 20);
    GetTimeResolution(h_tAve_pulseIntCorr_posCorr[iBar], fitFunAve_pulseIntCorr_posCorr[iBar], sigmaEff, sigmaGaus, sigmaGausErr, -1);
    h_effectiveSigmaAve_pulseIntCorr_posCorr[iBar]->Fill(sigmaEff);
    
    // --- tDiff resolution
    fitFunDiff_ampCorr[iBar]= new TF1(Form("fitFunDiff_ampCorr_BAR%d",iBar),"gaus", -20, 20);
    GetTimeResolution(h_tDiff_ampCorr[iBar], fitFunDiff_ampCorr[iBar], sigmaEff, sigmaGaus, sigmaGausErr, -1);
    h_effectiveSigmaDiff_ampCorr[iBar]->Fill(sigmaEff);
    
    fitFunDiff_ampCorr_posCorr[iBar]= new TF1(Form("fitFunDiff_ampCorr_posCorr_BAR%d",iBar),"gaus", -20, 20);
    GetTimeResolution(h_tDiff_ampCorr_posCorr[iBar], fitFunDiff_ampCorr_posCorr[iBar], sigmaEff, sigmaGaus, sigmaGausErr, -1);
    h_effectiveSigmaDiff_ampCorr_posCorr[iBar]->Fill(sigmaEff);
    
    fitFunDiff_pulseIntCorr[iBar]= new TF1(Form("fitFunDiff_pulseIntCorr_BAR%d",iBar),"gaus", -20, 20);
    GetTimeResolution(h_tDiff_pulseIntCorr[iBar], fitFunDiff_pulseIntCorr[iBar], sigmaEff, sigmaGaus, sigmaGausErr, -1);
    h_effectiveSigmaDiff_pulseIntCorr[iBar]->Fill(sigmaEff);

    fitFunDiff_pulseIntCorr_posCorr[iBar]= new TF1(Form("fitFunDiff_pulseIntCorr_posCorr_BAR%d",iBar),"gaus", -20, 20);
    GetTimeResolution(h_tDiff_pulseIntCorr_posCorr[iBar], fitFunDiff_pulseIntCorr_posCorr[iBar], sigmaEff, sigmaGaus, sigmaGausErr, -1);
    h_effectiveSigmaDiff_pulseIntCorr_posCorr[iBar]->Fill(sigmaEff);
    
    // tResol graphs in bins in x
    g_tResolGausL_ampCorr[iBar] = new TGraphErrors();
    g_tResolGausL_ampCorr[iBar]->SetName(Form("g_tResolGausL_ampCorr_BAR%d",iBar));
    
    g_tResolGausR_ampCorr[iBar] = new TGraphErrors();
    g_tResolGausR_ampCorr[iBar]->SetName(Form("g_tResolGausR_ampCorr_BAR%d",iBar));
    
    g_tResolGausAve_ampCorr[iBar] = new TGraphErrors();
    g_tResolGausAve_ampCorr[iBar]->SetName(Form("g_tResolGausAve_ampCorr_BAR%d",iBar));
    
    g_tResolGausAve_ampCorr_posCorr[iBar] = new TGraphErrors();
    g_tResolGausAve_ampCorr_posCorr[iBar]->SetName(Form("g_tResolGausAve_ampCorr_posCorr_BAR%d",iBar));
    
    g_tResolGausAve_ampCorr_vs_posY[iBar] = new TGraphErrors();
    g_tResolGausAve_ampCorr_vs_posY[iBar]->SetName(Form("g_tResolGausAve_ampCorr_vs_posY_BAR%d",iBar));
    
    g_tResolGausAve_ampCorr_posCorr_vs_posY[iBar] = new TGraphErrors();
    g_tResolGausAve_ampCorr_posCorr_vs_posY[iBar]->SetName(Form("g_tResolGausAve_ampCorr_posCorr_vs_posY_BAR%d",iBar));

    g_tResolGausDiff[iBar] = new TGraphErrors();
    g_tResolGausDiff[iBar]->SetName(Form("g_tResolGausDiff_BAR%d",iBar));

    g_tResolGausDiff_posCorr[iBar] = new TGraphErrors();
    g_tResolGausDiff_posCorr[iBar]->SetName(Form("g_tResolGausDiff_posCorr_BAR%d",iBar));

    g_tResolGausDiff_ampCorr[iBar] = new TGraphErrors();
    g_tResolGausDiff_ampCorr[iBar]->SetName(Form("g_tResolGausDiff_ampCorr_BAR%d",iBar));
    
    g_tResolGausDiff_ampCorr_posCorr[iBar] = new TGraphErrors();
    g_tResolGausDiff_ampCorr_posCorr[iBar]->SetName(Form("g_tResolGausDiff_ampCorr_posCorr_BAR%d",iBar));
    
    g_tResolGausDiff_ampCorr_vs_posY[iBar] = new TGraphErrors();
    g_tResolGausDiff_ampCorr_vs_posY[iBar]->SetName(Form("g_tResolGausDiff_ampCorr_vs_posY_BAR%d",iBar));
    
    g_tResolGausDiff_ampCorr_posCorr_vs_posY[iBar] = new TGraphErrors();
    g_tResolGausDiff_ampCorr_posCorr_vs_posY[iBar]->SetName(Form("g_tResolGausDiff_ampCorr_posCorr_vs_posY_BAR%d",iBar));


    g_tResolGausL_pulseIntCorr[iBar] = new TGraphErrors();
    g_tResolGausL_pulseIntCorr[iBar]->SetName(Form("g_tResolGausL_pulseIntCorr_BAR%d",iBar));

    g_tResolGausR_pulseIntCorr[iBar] = new TGraphErrors();
    g_tResolGausR_pulseIntCorr[iBar]->SetName(Form("g_tResolGausR_pulseIntCorr_BAR%d",iBar));

    g_tResolGausAve_pulseIntCorr[iBar] = new TGraphErrors();
    g_tResolGausAve_pulseIntCorr[iBar]->SetName(Form("g_tResolGausAve_pulseIntCorr_BAR%d",iBar));

    g_tResolGausAve_pulseIntCorr_posCorr[iBar] = new TGraphErrors();
    g_tResolGausAve_pulseIntCorr_posCorr[iBar]->SetName(Form("g_tResolGausAve_pulseIntCorr_posCorr_BAR%d",iBar));

    g_tResolGausAve_pulseIntCorr_vs_posY[iBar] = new TGraphErrors();
    g_tResolGausAve_pulseIntCorr_vs_posY[iBar]->SetName(Form("g_tResolGausAve_pulseIntCorr_vs_posY_BAR%d",iBar));

    g_tResolGausAve_pulseIntCorr_posCorr_vs_posY[iBar] = new TGraphErrors();
    g_tResolGausAve_pulseIntCorr_posCorr_vs_posY[iBar]->SetName(Form("g_tResolGausAve_pulseIntCorr_posCorr_vs_posY_BAR%d",iBar));

    g_tResolGausDiff_pulseIntCorr[iBar] = new TGraphErrors();
    g_tResolGausDiff_pulseIntCorr[iBar]->SetName(Form("g_tResolGausDiff_pulseIntCorr_BAR%d",iBar));

    g_tResolGausDiff_pulseIntCorr_posCorr[iBar] = new TGraphErrors();
    g_tResolGausDiff_pulseIntCorr_posCorr[iBar]->SetName(Form("g_tResolGausDiff_pulseIntCorr_posCorr_BAR%d",iBar));

    g_tResolGausDiff_pulseIntCorr_vs_posY[iBar] = new TGraphErrors();
    g_tResolGausDiff_pulseIntCorr_vs_posY[iBar]->SetName(Form("g_tResolGausDiff_pulseIntCorr_vs_posY_BAR%d",iBar));

    g_tResolGausDiff_pulseIntCorr_posCorr_vs_posY[iBar] = new TGraphErrors();
    g_tResolGausDiff_pulseIntCorr_posCorr_vs_posY[iBar]->SetName(Form("g_tResolGausDiff_pulseIntCorr_posCorr_vs_posY_BAR%d",iBar));

    g_tResolGausDiff_pulseIntCorr_posCorr_vs_amp[iBar] = new TGraphErrors();
    g_tResolGausDiff_pulseIntCorr_posCorr_vs_amp[iBar]->SetName(Form("g_tResolGausDiff_pulseIntCorr_posCorr_vs_amp_BAR%d",iBar));

    // --- vs X
    for (int ibin = 0; ibin < NBINSX[iBar]; ibin++){
      
      float xx = cut_Xmin[iBar] + xbinWidth*ibin + xbinWidth/2;
      
      // ----- amp walk corr
      // -- Left
      fitFunL_ampCorr_binX[iBar][ibin]= new TF1(Form("fitFunL_ampCorr_binX%d_BAR%d",ibin,iBar),"gaus", -20, 20); 
      GetTimeResolution(h_tL_ampCorr_binX[iBar][ibin], fitFunL_ampCorr_binX[iBar][ibin], resolEff, resolGaus, resolGausErr, resolMCP);
      g_tResolGausL_ampCorr[iBar]->SetPoint(ibin, xx, resolGaus*1000.);
      g_tResolGausL_ampCorr[iBar]->SetPointError(ibin, xbinWidth/2., resolGausErr*1000.);

      // -- Right
      fitFunR_ampCorr_binX[iBar][ibin]= new TF1(Form("fitFunR_ampCorr_binX%d_BAR%d",ibin,iBar),"gaus", -20, 20); 
      GetTimeResolution(h_tR_ampCorr_binX[iBar][ibin], fitFunR_ampCorr_binX[iBar][ibin], resolEff, resolGaus, resolGausErr, resolMCP);
      g_tResolGausR_ampCorr[iBar]->SetPoint(ibin, xx, resolGaus*1000.);
      g_tResolGausR_ampCorr[iBar]->SetPointError(ibin, xbinWidth/2., resolGausErr*1000.);

      // -- simple average
      fitFunAve_ampCorr_binX[iBar][ibin]= new TF1(Form("fitFunAve_ampCorr_binX%d_BAR%d",ibin,iBar),"gaus", -20, 20); 
      GetTimeResolution(h_tAve_ampCorr_binX[iBar][ibin], fitFunAve_ampCorr_binX[iBar][ibin], resolEff, resolGaus, resolGausErr, resolMCP);
      g_tResolGausAve_ampCorr[iBar]->SetPoint(ibin, xx, resolGaus*1000.);
      g_tResolGausAve_ampCorr[iBar]->SetPointError(ibin, xbinWidth/2., resolGausErr*1000.);

      // --- amp walk corrections vs integral
      // -- Left
      fitFunL_pulseIntCorr_binX[iBar][ibin]= new TF1(Form("fitFunL_pulseIntCorr_binX%d_BAR%d",ibin,iBar),"gaus", -20, 20);
      GetTimeResolution(h_tL_pulseIntCorr_binX[iBar][ibin], fitFunL_pulseIntCorr_binX[iBar][ibin], resolEff, resolGaus, resolGausErr, resolMCP);
      g_tResolGausL_pulseIntCorr[iBar]->SetPoint(ibin, xx, resolGaus*1000.);
      g_tResolGausL_pulseIntCorr[iBar]->SetPointError(ibin, xbinWidth/2., resolGausErr*1000.);

      // -- Right
      fitFunR_pulseIntCorr_binX[iBar][ibin]= new TF1(Form("fitFunR_pulseIntCorr_binX%d_BAR%d",ibin,iBar),"gaus", -20, 20);
      GetTimeResolution(h_tR_pulseIntCorr_binX[iBar][ibin], fitFunR_pulseIntCorr_binX[iBar][ibin], resolEff, resolGaus, resolGausErr, resolMCP);
      g_tResolGausR_pulseIntCorr[iBar]->SetPoint(ibin, xx, resolGaus*1000.);
      g_tResolGausR_pulseIntCorr[iBar]->SetPointError(ibin, xbinWidth/2., resolGausErr*1000.);

      // -- simple average
      fitFunAve_pulseIntCorr_binX[iBar][ibin]= new TF1(Form("fitFunAve_pulseIntCorr_binX%d_BAR%d",ibin,iBar),"gaus", -20, 20);
      GetTimeResolution(h_tAve_pulseIntCorr_binX[iBar][ibin], fitFunAve_pulseIntCorr_binX[iBar][ibin], resolEff, resolGaus, resolGausErr, resolMCP);
      g_tResolGausAve_pulseIntCorr[iBar]->SetPoint(ibin, xx, resolGaus*1000.);
      g_tResolGausAve_pulseIntCorr[iBar]->SetPointError(ibin, xbinWidth/2., resolGausErr*1000.);

      // -- tDiff 
      fitFunDiff_binX[iBar][ibin]= new TF1(Form("fitFunDiff_binX%d_BAR%d",ibin,iBar),"gaus", -20, 20);
      GetTimeResolution(h_tDiff_binX[iBar][ibin], fitFunDiff_binX[iBar][ibin], resolEff, resolGaus, resolGausErr, -1);
      g_tResolGausDiff[iBar]->SetPoint(ibin, xx, resolGaus/2.*1000.);
      g_tResolGausDiff[iBar]->SetPointError(ibin, xbinWidth/2., resolGausErr/2.*1000.);

      fitFunDiff_ampCorr_binX[iBar][ibin]= new TF1(Form("fitFunDiff_ampCorr_binX%d_BAR%d",ibin,iBar),"gaus", -20, 20);
      GetTimeResolution(h_tDiff_ampCorr_binX[iBar][ibin], fitFunDiff_ampCorr_binX[iBar][ibin], resolEff, resolGaus, resolGausErr, -1);
      g_tResolGausDiff_ampCorr[iBar]->SetPoint(ibin, xx, resolGaus/2.*1000.);
      g_tResolGausDiff_ampCorr[iBar]->SetPointError(ibin, xbinWidth/2., resolGausErr/2*1000.);

      fitFunDiff_pulseIntCorr_binX[iBar][ibin]= new TF1(Form("fitFunDiff_pulseIntCorr_binX%d_BAR%d",ibin,iBar),"gaus", -20, 20);
      GetTimeResolution(h_tDiff_pulseIntCorr_binX[iBar][ibin], fitFunDiff_pulseIntCorr_binX[iBar][ibin], resolEff, resolGaus, resolGausErr, -1);
      g_tResolGausDiff_pulseIntCorr[iBar]->SetPoint(ibin, xx, resolGaus/2*1000.);
      g_tResolGausDiff_pulseIntCorr[iBar]->SetPointError(ibin, xbinWidth/2., resolGausErr/2*1000.);      


      // ----- average
      //-- ampCorr + pos corr 
      fitFunAve_ampCorr_posCorr_binX[iBar][ibin]= new TF1(Form("fitFunAve_ampCorr_posCorr_binX%d_BAR%d",ibin,iBar),"gaus", -20, 20);
      GetTimeResolution(h_tAve_ampCorr_posCorr_binX[iBar][ibin], fitFunAve_ampCorr_posCorr_binX[iBar][ibin], resolEff, resolGaus, resolGausErr, resolMCP);
      g_tResolGausAve_ampCorr_posCorr[iBar]->SetPoint(ibin, xx, resolGaus*1000.);
      g_tResolGausAve_ampCorr_posCorr[iBar]->SetPointError(ibin, xbinWidth/2., resolGausErr*1000.);

      // -- amp walk corr vs integral + pos corr 
      fitFunAve_pulseIntCorr_posCorr_binX[iBar][ibin]= new TF1(Form("fitFunAve_pulseIntCorr_posCorr_binX%d_BAR%d",ibin,iBar),"gaus", -20, 20);
      GetTimeResolution(h_tAve_pulseIntCorr_posCorr_binX[iBar][ibin], fitFunAve_pulseIntCorr_posCorr_binX[iBar][ibin], resolEff, resolGaus, resolGausErr, resolMCP);
      g_tResolGausAve_pulseIntCorr_posCorr[iBar]->SetPoint(ibin, xx, resolGaus*1000.);
      g_tResolGausAve_pulseIntCorr_posCorr[iBar]->SetPointError(ibin, xbinWidth/2., resolGausErr*1000.);

      // -- tDiff pos corr
      fitFunDiff_posCorr_binX[iBar][ibin]= new TF1(Form("fitFunDiff_posCorr_binX%d_BAR%d",ibin,iBar),"gaus", -20, 20);
      GetTimeResolution(h_tDiff_posCorr_binX[iBar][ibin], fitFunDiff_posCorr_binX[iBar][ibin], resolEff, resolGaus, resolGausErr, -1);
      g_tResolGausDiff_posCorr[iBar]->SetPoint(ibin, xx, resolGaus/2*1000.);
      g_tResolGausDiff_posCorr[iBar]->SetPointError(ibin, xbinWidth/2., resolGausErr/2*1000.);

      // -- tDiff resolution (ampCorr+ posCorr)
      fitFunDiff_ampCorr_posCorr_binX[iBar][ibin]= new TF1(Form("fitFunDiff_ampCorr_posCorr_binX%d_BAR%d",ibin,iBar),"gaus", -20, 20);
      GetTimeResolution(h_tDiff_ampCorr_posCorr_binX[iBar][ibin], fitFunDiff_ampCorr_posCorr_binX[iBar][ibin], resolEff, resolGaus, resolGausErr, -1);
      g_tResolGausDiff_ampCorr_posCorr[iBar]->SetPoint(ibin, xx, resolGaus/2*1000.);
      g_tResolGausDiff_ampCorr_posCorr[iBar]->SetPointError(ibin, xbinWidth/2., resolGausErr/2*1000.);

      // -- tDiff resolution (pulseIntCorr+ posCorr)
      fitFunDiff_pulseIntCorr_posCorr_binX[iBar][ibin]= new TF1(Form("fitFunDiff_pulseIntCorr_posCorr_binX%d_BAR%d",ibin,iBar),"gaus", -20, 20);
      GetTimeResolution(h_tDiff_pulseIntCorr_posCorr_binX[iBar][ibin], fitFunDiff_pulseIntCorr_posCorr_binX[iBar][ibin], resolEff, resolGaus, resolGausErr, -1);
      g_tResolGausDiff_pulseIntCorr_posCorr[iBar]->SetPoint(ibin, xx, resolGaus/2*1000.);
      g_tResolGausDiff_pulseIntCorr_posCorr[iBar]->SetPointError(ibin, xbinWidth/2., resolGausErr/2*1000.);

    }// --  end vs X
    
    
    // --- vs Y 
    int np = 0;
    for (int ibin = 0; ibin < NBINSY; ibin++){
      
      float yy = cut_Ymin[NBARS-1] + ybinWidth*ibin + ybinWidth/2;
      
      if (yy < cut_Ymin[iBar]  || yy > cut_Ymax[iBar]) continue;

      // -- simple average - amp corr
      fitFunAve_ampCorr_binY[iBar][ibin]= new TF1(Form("fitFunAve_ampCorr_binY%d_BAR%d",ibin,iBar),"gaus", -20, 20);
      GetTimeResolution(h_tAve_ampCorr_binY[iBar][ibin], fitFunAve_ampCorr_binY[iBar][ibin], resolEff, resolGaus, resolGausErr, resolMCP);
      g_tResolGausAve_ampCorr_vs_posY[iBar]->SetPoint(np, yy, resolGaus*1000.);
      g_tResolGausAve_ampCorr_vs_posY[iBar]->SetPointError(np, ybinWidth/2., resolGausErr*1000.);
      
      // -- simple average - amp+pos corr
      fitFunAve_ampCorr_posCorr_binY[iBar][ibin]= new TF1(Form("fitFunAve_ampCorr_posCorr_binY%d_BAR%d",ibin,iBar),"gaus", -20, 20);
      GetTimeResolution(h_tAve_ampCorr_posCorr_binY[iBar][ibin], fitFunAve_ampCorr_posCorr_binY[iBar][ibin], resolEff, resolGaus, resolGausErr, resolMCP);
      g_tResolGausAve_ampCorr_posCorr_vs_posY[iBar]->SetPoint(np, yy, resolGaus*1000.);
      g_tResolGausAve_ampCorr_posCorr_vs_posY[iBar]->SetPointError(np, ybinWidth/2., resolGausErr*1000.);
      
      // -- simple average, pulse int corr
      fitFunAve_pulseIntCorr_binY[iBar][ibin]= new TF1(Form("fitFunAve_pulseIntCorr_binY%d_BAR%d",ibin,iBar),"gaus", -20, 20);
      GetTimeResolution(h_tAve_pulseIntCorr_binY[iBar][ibin], fitFunAve_pulseIntCorr_binY[iBar][ibin], resolEff, resolGaus, resolGausErr, resolMCP);
      g_tResolGausAve_pulseIntCorr_vs_posY[iBar]->SetPoint(np, yy, resolGaus*1000.);
      g_tResolGausAve_pulseIntCorr_vs_posY[iBar]->SetPointError(np, ybinWidth/2., resolGausErr*1000.);
      
      // -- simple average, pulse int + pos corr
      fitFunAve_pulseIntCorr_posCorr_binY[iBar][ibin]= new TF1(Form("fitFunAve_pulseIntCorr_posCorr_binY%d_BAR%d",ibin,iBar),"gaus", -20, 20);
      GetTimeResolution(h_tAve_pulseIntCorr_posCorr_binY[iBar][ibin], fitFunAve_pulseIntCorr_posCorr_binY[iBar][ibin], resolEff, resolGaus, resolGausErr, resolMCP);
      g_tResolGausAve_pulseIntCorr_posCorr_vs_posY[iBar]->SetPoint(np, yy, resolGaus*1000.);
      g_tResolGausAve_pulseIntCorr_posCorr_vs_posY[iBar]->SetPointError(np, ybinWidth/2., resolGausErr*1000.);
      
      // -- tDiff
      // -- amp corr
      fitFunDiff_ampCorr_binY[iBar][ibin]= new TF1(Form("fitFunDiff_ampCorr_binY%d_BAR%d",ibin,iBar),"gaus", -20, 20);
      GetTimeResolution(h_tDiff_ampCorr_binY[iBar][ibin], fitFunDiff_ampCorr_binY[iBar][ibin], resolEff, resolGaus, resolGausErr, -1);
      g_tResolGausDiff_ampCorr_vs_posY[iBar]->SetPoint(np, yy, resolGaus/2*1000.);
      g_tResolGausDiff_ampCorr_vs_posY[iBar]->SetPointError(np, ybinWidth/2., resolGausErr/2*1000.);
      
      // -- pusleInt corr
      fitFunDiff_pulseIntCorr_binY[iBar][ibin]= new TF1(Form("fitFunDiff_pulseIntCorr_binY%d_BAR%d",ibin,iBar),"gaus", -20, 20);
      GetTimeResolution(h_tDiff_pulseIntCorr_binY[iBar][ibin], fitFunDiff_pulseIntCorr_binY[iBar][ibin], resolEff, resolGaus, resolGausErr, -1);
      g_tResolGausDiff_pulseIntCorr_vs_posY[iBar]->SetPoint(np, yy, resolGaus/2*1000.);
      g_tResolGausDiff_pulseIntCorr_vs_posY[iBar]->SetPointError(np, ybinWidth/2., resolGausErr/2*1000.);
    
      // amp + pos corr
      fitFunDiff_ampCorr_posCorr_binY[iBar][ibin]= new TF1(Form("fitFunDiff_ampCorr_posCorr_binY%d_BAR%d",ibin,iBar),"gaus", -20, 20);
      GetTimeResolution(h_tDiff_ampCorr_posCorr_binY[iBar][ibin], fitFunDiff_ampCorr_posCorr_binY[iBar][ibin], resolEff, resolGaus, resolGausErr, -1);
      g_tResolGausDiff_ampCorr_posCorr_vs_posY[iBar]->SetPoint(np, yy, resolGaus/2*1000.);
      g_tResolGausDiff_ampCorr_posCorr_vs_posY[iBar]->SetPointError(np, ybinWidth/2., resolGausErr/2*1000.);
    
      // pulseInt + pos corr
      fitFunDiff_pulseIntCorr_posCorr_binY[iBar][ibin]= new TF1(Form("fitFunDiff_pulseIntCorr_posCorr_binY%d_BAR%d",ibin,iBar),"gaus", -20, 20);
      GetTimeResolution(h_tDiff_pulseIntCorr_posCorr_binY[iBar][ibin], fitFunDiff_pulseIntCorr_posCorr_binY[iBar][ibin], resolEff, resolGaus, resolGausErr, -1);
      g_tResolGausDiff_pulseIntCorr_posCorr_vs_posY[iBar]->SetPoint(np, yy, resolGaus/2*1000.);
      g_tResolGausDiff_pulseIntCorr_posCorr_vs_posY[iBar]->SetPointError(np, ybinWidth/2., resolGausErr/2*1000.);
      
      np++;
      
    } // -- end vs Y


    // -- vs amplitude
    for (int abin = 0 ; abin < 20; abin++){
      float amp =  abin * ampBinWidth + ampBinWidth/2;
      // pulseInt + pos corr
      fitFunDiff_pulseIntCorr_posCorr_binAmp[iBar][abin]= new TF1(Form("fitFunDiff_pulseIntCorr_posCorr_binAmp%d_BAR%d",abin,iBar),"gaus", -20, 20);
      GetTimeResolution(h_tDiff_pulseIntCorr_posCorr_binAmp[iBar][abin], fitFunDiff_pulseIntCorr_posCorr_binAmp[iBar][abin], resolEff, resolGaus, resolGausErr, -1);
      g_tResolGausDiff_pulseIntCorr_posCorr_vs_amp[iBar]->SetPoint(abin, amp, resolGaus/2*1000.);
      g_tResolGausDiff_pulseIntCorr_posCorr_vs_amp[iBar]->SetPointError(abin, ampBinWidth/2, resolGausErr/2*1000.);
    }




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

    // -- pulse int corr

    // -- tLeft
    fpol0_tL_pulseIntCorr[iBar] = new TF1(Form("fpol0_tL_pulseIntCorr_BAR%d",iBar),"pol0",-50,50);
    fpol0_tL_pulseIntCorr[iBar]->SetLineStyle(2);
    fpol0_tL_pulseIntCorr[iBar]->SetLineWidth(2);
    fpol0_tL_pulseIntCorr[iBar]->SetLineColor(4);
    fpol0_tL_pulseIntCorr[iBar]->SetParameter(0, g_tResolGausL_pulseIntCorr[iBar]->GetMean(2));
    g_tResolGausL_pulseIntCorr[iBar]->Fit(fpol0_tL_pulseIntCorr[iBar],"QRS");

    // -- tRight
    fpol0_tR_pulseIntCorr[iBar] = new TF1(Form("fpol0_tR_pulseIntCorr_BAR%d",iBar),"pol0",-50,50);
    fpol0_tR_pulseIntCorr[iBar]->SetLineStyle(2);
    fpol0_tR_pulseIntCorr[iBar]->SetLineWidth(2);
    fpol0_tR_pulseIntCorr[iBar]->SetLineColor(2);
    fpol0_tR_pulseIntCorr[iBar]->SetParameter(0, g_tResolGausR_pulseIntCorr[iBar]->GetMean(2));
    g_tResolGausR_pulseIntCorr[iBar]->Fit(fpol0_tR_pulseIntCorr[iBar],"QRS");

    // -- tAve
    fpol0_tAve_pulseIntCorr[iBar] = new TF1(Form("fpol0_tAve_pulseIntCorr_BAR%d",iBar),"pol0",-50,50);
    fpol0_tAve_pulseIntCorr[iBar]->SetLineStyle(2);
    fpol0_tAve_pulseIntCorr[iBar]->SetLineWidth(2);
    fpol0_tAve_pulseIntCorr[iBar]->SetLineColor(1);
    fpol0_tAve_pulseIntCorr[iBar]->SetParameter(0, g_tResolGausAve_pulseIntCorr[iBar]->GetMean(2));
    g_tResolGausAve_pulseIntCorr[iBar]->Fit(fpol0_tAve_pulseIntCorr[iBar],"QRS");

    fpol0_tAve_pulseIntCorr_posCorr[iBar] = new TF1(Form("fpol0_tAve_pulseIntCorr_posCorr_BAR%d",iBar),"pol0",-50,50);
    fpol0_tAve_pulseIntCorr_posCorr[iBar]->SetLineStyle(2);
    fpol0_tAve_pulseIntCorr_posCorr[iBar]->SetLineWidth(2);
    fpol0_tAve_pulseIntCorr_posCorr[iBar]->SetLineColor(1);
    fpol0_tAve_pulseIntCorr_posCorr[iBar]->SetParameter(0, g_tResolGausAve_pulseIntCorr[iBar]->GetMean(2));
    g_tResolGausAve_pulseIntCorr[iBar]->Fit(fpol0_tAve_pulseIntCorr_posCorr[iBar],"QRS");

    // -- tdiff
    fpol0_tDiff_pulseIntCorr[iBar] = new TF1(Form("fpol0_tDiff_pulseIntCorr_BAR%d",iBar),"pol0",-50,50);
    fpol0_tDiff_pulseIntCorr[iBar]->SetLineStyle(2);
    fpol0_tDiff_pulseIntCorr[iBar]->SetLineWidth(2);
    fpol0_tDiff_pulseIntCorr[iBar]->SetLineColor(1);
    g_tResolGausDiff_pulseIntCorr[iBar]->Fit(fpol0_tDiff_pulseIntCorr[iBar],"QRS");

    fpol0_tDiff_pulseIntCorr_posCorr[iBar] = new TF1(Form("fpol0_tDiff_pulseIntCorr_posCorr_BAR%d",iBar),"pol0",-50,50);
    fpol0_tDiff_pulseIntCorr_posCorr[iBar]->SetLineStyle(2);
    fpol0_tDiff_pulseIntCorr_posCorr[iBar]->SetLineWidth(2);
    fpol0_tDiff_pulseIntCorr_posCorr[iBar]->SetLineColor(1);
    g_tResolGausDiff_pulseIntCorr_posCorr[iBar]->Fit(fpol0_tDiff_pulseIntCorr_posCorr[iBar],"QRS");
  }//

 

  // **** Now do the last loop on events to combine bars **** 

  // -- parametrization of the single bar time resolution vs Y
  TF1 *f_tRes[NBARS];
  for (int iBar = 0; iBar < NBARS; iBar++){
    f_tRes[iBar] = new TF1(Form("f_tRes_BAR%d",iBar),"pol6",cut_Ymin[iBar],cut_Ymax[iBar]);
    g_tResolGausAve_ampCorr_posCorr_vs_posY[iBar]->Fit(f_tRes[iBar],"QRN");
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
      if (useTimeMaxMCP) tRef   = treeVars.t_time_max[(*treeVars.t_channelId)[timeChannelRef]];
      brmsRef = treeVars.t_b_rms[(*treeVars.t_channelId)[timeChannelRef]];

      float w[NBARS] = {0., 0., 0.};
      float wr[NBARS] = {0., 0., 0.};
      float tA_ampCorr[NBARS] = {-999., -999., -999.};
      float tA_ampCorr_posCorr[NBARS] = {-999., -999., -999.};
      float tA_pulseIntCorr[NBARS] = {-999., -999., -999.};
      float tA_pulseIntCorr_posCorr[NBARS] = {-999., -999., -999.};
      float tD_ampCorr[NBARS] = {-999., -999., -999.};
      float tD_ampCorr_posCorr[NBARS] = {-999., -999., -999.};
      float tD_pulseIntCorr[NBARS] = {-999., -999., -999.};
      float tD_pulseIntCorr_posCorr[NBARS] = {-999., -999., -999.};

      // -- loop over bars
      for (int iBar = 0; iBar < NBARS; iBar++){
	
        ampL = treeVars.t_amp[(*treeVars.t_channelId)[ ampChannelsL[iBar] ]] * kAdcToV ;
        ampR = treeVars.t_amp[(*treeVars.t_channelId)[ ampChannelsR[iBar] ]] * kAdcToV ;
	
        tL   = treeVars.t_time[(*treeVars.t_channelId)[ timeChannelsL[iBar] ]+treeVars.t_LED];
        tR   = treeVars.t_time[(*treeVars.t_channelId)[ timeChannelsR[iBar] ]+treeVars.t_LED];

	brmsL = treeVars.t_b_rms[(*treeVars.t_channelId)[ timeChannelsL[iBar] ] ];
	brmsR = treeVars.t_b_rms[(*treeVars.t_channelId)[ timeChannelsR[iBar] ] ];

	pulseIntL = treeVars.t_charge_sig[(*treeVars.t_channelId)[ ampChannelsL[iBar] ]] * kAdcToV;
        pulseIntR = treeVars.t_charge_sig[(*treeVars.t_channelId)[ ampChannelsR[iBar] ]] * kAdcToV;
        pulseIntAve = 0.5 * (pulseIntL + pulseIntR);

	// -- select events
        if ( posX < cut_XminRef || posX > cut_XmaxRef ) continue;
        if ( posY < cut_YminRef || posY > cut_YmaxRef ) continue;
        if ( posX < cut_Xmin[iBar] || posX > cut_Xmax[iBar] ) continue;
        if ( posY < cut_Ymin[iBar] || posY > cut_Ymax[iBar] ) continue;
	
        if ( ampRef < cut_ampMinRef     ||  ampRef > cut_ampMaxRef ) continue;
        //if ( ampL   < cut_ampMinL[iBar] ||  ampL   > cut_ampMaxL[iBar] ) continue;
        //if ( ampR   < cut_ampMinR[iBar] ||  ampR   > cut_ampMaxR[iBar] ) continue;
	if ( pulseIntL   < cut_pulseIntMinL[iBar] ||  pulseIntL   > cut_pulseIntMaxL[iBar] ) continue;
        if ( pulseIntR   < cut_pulseIntMinR[iBar] ||  pulseIntR   > cut_pulseIntMaxR[iBar] ) continue;

        if ( tRef < cut_minTimeRef || tRef > cut_maxTimeRef ) continue;
        if ( tL   < cut_minTime[iBar]  || tL   > cut_maxTime[iBar]  ) continue;
        if ( tR   < cut_minTime[iBar]  || tR   > cut_maxTime[iBar]  ) continue;

	if (tL < tLmin[iBar] || tL > tLmax[iBar]) continue;
	if (tR < tRmin[iBar] || tR > tRmax[iBar]) continue;

	// -- remove very noisy events
	if (brmsRef > cut_brmsMaxRef) continue;
        if (brmsR > cut_brmsMaxTimeCh[iBar]) continue;
        if (brmsL > cut_brmsMaxTimeCh[iBar]) continue;

	if (applyMCPTimeCorrection) tRef = GetCorrectedMCPTime(tRef, ampRef*1000., posX);
        tL = tL - tRef;
        tR = tR - tRef;


	// -- amplitude walk correction
        int xcbinamp = int ((posX - max(cut_Xmin[iBar], cut_XminRef))/cos(theta[iBar])/xbinWidthForAmpWalk );
        float tL_corr = tL - fitFuncL_ampCorr_binned[iBar][xcbinamp]->Eval(ampL);
        float tR_corr = tR - fitFuncR_ampCorr_binned[iBar][xcbinamp]->Eval(ampR);
	float tAve_ampCorr = ( tL_corr + tR_corr)/2 ;
	float tAve_ampCorr_posCorr  = tAve_ampCorr - fitFuncAve_posCorr[iBar]->Eval(posX);
        float tDiff_ampCorr = tR_corr - tL_corr;
	float tDiff_ampCorr_posCorr = tDiff_ampCorr - fitFuncDiff_posCorr[iBar]->Eval(posX);

        // -- amp walk correction vs pulse integral
        float tL_pulseIntCorr = tL - fitFuncL_pulseIntCorr_binned[iBar][xcbinamp]->Eval(pulseIntL);
        float tR_pulseIntCorr = tR - fitFuncR_pulseIntCorr_binned[iBar][xcbinamp]->Eval(pulseIntR);
	if (useAverageAmplitudeForTimeWalkCorr){
          tL_pulseIntCorr = tL - fitFuncL_pulseIntCorr_binned[iBar][xcbinamp]->Eval(pulseIntAve);
	  tR_pulseIntCorr = tR - fitFuncR_pulseIntCorr_binned[iBar][xcbinamp]->Eval(pulseIntAve);
	}
        float tAve_pulseIntCorr = ( tL_pulseIntCorr + tR_pulseIntCorr )/2 ;
        float tAve_pulseIntCorr_posCorr  = tAve_pulseIntCorr - fitFuncAve_pulseIntCorr_posCorr[iBar]->Eval(posX);
        float tDiff_pulseIntCorr = tR_pulseIntCorr - tL_pulseIntCorr;
        float tDiff_pulseIntCorr_posCorr = tDiff_pulseIntCorr - fitFuncDiff_pulseIntCorr_posCorr[iBar]->Eval(posX);


	// weights and times to combine bars
        //w[iBar] =  0.5*(ampL+ampR);
	w[iBar] =  pulseIntAve;

        tA_ampCorr[iBar] = tAve_ampCorr;
        tA_ampCorr_posCorr[iBar] = tAve_ampCorr_posCorr;
	tA_pulseIntCorr[iBar] = tAve_pulseIntCorr;
        tA_pulseIntCorr_posCorr[iBar] = tAve_pulseIntCorr_posCorr;

        tD_ampCorr[iBar] = tDiff_ampCorr;
        tD_ampCorr_posCorr[iBar] = tDiff_ampCorr_posCorr;
	tD_pulseIntCorr[iBar] = tDiff_pulseIntCorr;
        tD_pulseIntCorr_posCorr[iBar] = tDiff_pulseIntCorr_posCorr;

      }// end loop over bars

      
      if (w[0] == 0 && w[1] == 0 && w[2] == 0) continue;


      if (posY > 25.6 && posY<26.0) { // overlap 50% bar 0/bar1
        h2_tAve1_vs_tAve2->Fill(tA_ampCorr_posCorr[0], tA_ampCorr_posCorr[1]);
        h2_amp1_vs_amp2->Fill(w[0],w[1]);
      }


      // amp weighted sum   
      float ww[NBARS] = {0., 0., 0.};         
      float wsum = w[0]+w[1]+w[2];
      if (wsum!=0){
        ww[0] = w[0]/wsum;
        ww[1] = w[1]/wsum;
        ww[2] = w[2]/wsum;
      }

      float tAmpWSum_ampCorr = 0;
      float tAmpWSum_ampCorr_posCorr = 0;
      float tAmpWSum_pulseIntCorr = 0;
      float tAmpWSum_pulseIntCorr_posCorr = 0;
      for (int iBar =0; iBar < NBARS; iBar++){
	tAmpWSum_ampCorr+= ww[iBar] * tA_ampCorr[iBar];
	tAmpWSum_ampCorr_posCorr+= ww[iBar] * tA_ampCorr_posCorr[iBar];
	tAmpWSum_pulseIntCorr+= ww[iBar] * tA_pulseIntCorr[iBar];
	tAmpWSum_pulseIntCorr_posCorr+= ww[iBar] * tA_pulseIntCorr_posCorr[iBar];
      } 


      // resolution weighted sum 
      for ( int iBar = 0; iBar < NBARS; iBar++){
	if ( posY > cut_Ymin[iBar] && posY < cut_Ymax[iBar] && ww[iBar] > 0) {
	  float tRes = g_tResolGausAve_pulseIntCorr_posCorr_vs_posY[iBar]->Eval(posY)  ;
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
      float tResWSum_ampCorr_posCorr = 0;
      float tResWSum_pulseIntCorr = 0;
      float tResWSum_pulseIntCorr_posCorr = 0;
      for (int iBar =0; iBar < NBARS; iBar++){
	tResWSum_ampCorr+= wr[iBar] * tA_ampCorr[iBar];
	tResWSum_ampCorr_posCorr+= wr[iBar] * tA_ampCorr_posCorr[iBar];
	tResWSum_pulseIntCorr+= wr[iBar] * tA_pulseIntCorr[iBar];
	tResWSum_pulseIntCorr_posCorr+= wr[iBar] * tA_pulseIntCorr_posCorr[iBar];
      } 

      
      // unweighted  sum 
      float tAveSum_ampCorr = 0.;
      float tAveSum_ampCorr_posCorr = 0.;
      float tAveSum_pulseIntCorr = 0.;
      float tAveSum_pulseIntCorr_posCorr = 0.;
      int n1 = 0;
      int n2 = 0;
      int nn1 = 0;
      int nn2 = 0;
      for (int iBar =0; iBar < NBARS; iBar++){
	// sum bars
	if ( tA_ampCorr[iBar]!=-999 ) {
	  tAveSum_ampCorr+=tA_ampCorr[iBar] ;
	  n1++;
	}
	if ( tA_ampCorr_posCorr[iBar]!=-999 ) {
	  tAveSum_ampCorr_posCorr+=tA_ampCorr_posCorr[iBar] ;
	  n2++;
	}
	if ( tA_pulseIntCorr[iBar]!=-999 ) {
	  tAveSum_pulseIntCorr+=tA_pulseIntCorr[iBar] ;
	  nn1++;
	}
	if ( tA_pulseIntCorr_posCorr[iBar]!=-999 ) {
	  tAveSum_pulseIntCorr_posCorr+=tA_pulseIntCorr_posCorr[iBar] ;
	  nn2++;
	}

      }
      if (n1>2 || n2>2) cout << "@@@@@@@@@   Event " << entry << ": more than 2 bars hit!!!!!" <<endl;
      tAveSum_ampCorr/=n1;
      tAveSum_ampCorr_posCorr/=n2;
      tAveSum_pulseIntCorr/=nn1;
      tAveSum_pulseIntCorr_posCorr/=nn2;

      // tAve1 - tAve2 bars 
      float tAveDiff_ampCorr = -999.;
      float tAveDiff_ampCorr_posCorr = -999.;
      float tAveDiff_pulseIntCorr = -999.;
      float tAveDiff_pulseIntCorr_posCorr = -999.;
      for (int iBar = 0; iBar < NBARS-1; iBar++){

	if (tA_ampCorr[iBar]!=-999 && tA_ampCorr[iBar+1]!=-999)	      tAveDiff_ampCorr = tA_ampCorr[iBar] - tA_ampCorr[iBar+1] ;
	else if (tA_ampCorr[iBar]!=-999 && tA_ampCorr[iBar+1]==-999)  tAveDiff_ampCorr = tA_ampCorr[iBar] ;
	else if (tA_ampCorr[iBar]==-999 && tA_ampCorr[iBar+1]!=-999)  tAveDiff_ampCorr = tA_ampCorr[iBar+1] ;

	if (tA_ampCorr_posCorr[iBar]!=-999 && tA_ampCorr_posCorr[iBar+1]!=-999)	      tAveDiff_ampCorr_posCorr = tA_ampCorr_posCorr[iBar] - tA_ampCorr_posCorr[iBar+1];
	else if (tA_ampCorr_posCorr[iBar]!=-999 && tA_ampCorr_posCorr[iBar+1]==-999)  tAveDiff_ampCorr_posCorr = tA_ampCorr_posCorr[iBar] ;
	else if (tA_ampCorr_posCorr[iBar]==-999 && tA_ampCorr_posCorr[iBar+1]!=-999)  tAveDiff_ampCorr_posCorr = tA_ampCorr_posCorr[iBar+1] ;

	if (tA_pulseIntCorr[iBar]!=-999 && tA_pulseIntCorr[iBar+1]!=-999)       tAveDiff_pulseIntCorr = tA_pulseIntCorr[iBar] - tA_pulseIntCorr[iBar+1];
	else if (tA_pulseIntCorr[iBar]!=-999 && tA_pulseIntCorr[iBar+1]==-999)  tAveDiff_pulseIntCorr = tA_pulseIntCorr[iBar] ;
	else if (tA_pulseIntCorr[iBar]==-999 && tA_pulseIntCorr[iBar+1]!=-999)  tAveDiff_pulseIntCorr = tA_pulseIntCorr[iBar+1] ;

	if (tA_pulseIntCorr_posCorr[iBar]!=-999 && tA_pulseIntCorr_posCorr[iBar+1]!=-999)       tAveDiff_pulseIntCorr_posCorr = tA_pulseIntCorr_posCorr[iBar] - tA_pulseIntCorr_posCorr[iBar+1];
	else if (tA_pulseIntCorr_posCorr[iBar]!=-999 && tA_pulseIntCorr_posCorr[iBar+1]==-999)  tAveDiff_pulseIntCorr_posCorr = tA_pulseIntCorr_posCorr[iBar] ;
	else if (tA_pulseIntCorr_posCorr[iBar]==-999 && tA_pulseIntCorr_posCorr[iBar+1]!=-999)  tAveDiff_pulseIntCorr_posCorr = tA_pulseIntCorr_posCorr[iBar+1] ;
	
      }

      // tDiff1 -tDiff2
      float tDiffDiff_ampCorr = -999.;
      float tDiffDiff_ampCorr_posCorr = -999.;
      float tDiffDiff_pulseIntCorr = -999.;
      float tDiffDiff_pulseIntCorr_posCorr = -999.;
      for (int iBar = 0; iBar < NBARS-1; iBar++){

	if (tD_ampCorr[iBar]!=-999 && tD_ampCorr[iBar+1]!=-999)	      tDiffDiff_ampCorr = tD_ampCorr[iBar] - tD_ampCorr[iBar+1] ;
	else if (tD_ampCorr[iBar]!=-999 && tD_ampCorr[iBar+1]==-999)  tDiffDiff_ampCorr = tD_ampCorr[iBar] ;
	else if (tD_ampCorr[iBar]==-999 && tD_ampCorr[iBar+1]!=-999)  tDiffDiff_ampCorr = tD_ampCorr[iBar+1] ;

	if (tD_ampCorr_posCorr[iBar]!=-999 && tD_ampCorr_posCorr[iBar+1]!=-999)	      tDiffDiff_ampCorr_posCorr = tD_ampCorr_posCorr[iBar] - tD_ampCorr_posCorr[iBar+1];
	else if (tD_ampCorr_posCorr[iBar]!=-999 && tD_ampCorr_posCorr[iBar+1]==-999)  tDiffDiff_ampCorr_posCorr = tD_ampCorr_posCorr[iBar] ;
	else if (tD_ampCorr_posCorr[iBar]==-999 && tD_ampCorr_posCorr[iBar+1]!=-999)  tDiffDiff_ampCorr_posCorr = tD_ampCorr_posCorr[iBar+1] ;

	if (tD_pulseIntCorr[iBar]!=-999 && tD_pulseIntCorr[iBar+1]!=-999)       tDiffDiff_pulseIntCorr = tD_pulseIntCorr[iBar] - tD_pulseIntCorr[iBar+1];
	else if (tD_pulseIntCorr[iBar]!=-999 && tD_pulseIntCorr[iBar+1]==-999)  tDiffDiff_pulseIntCorr = tD_pulseIntCorr[iBar] ;
	else if (tD_pulseIntCorr[iBar]==-999 && tD_pulseIntCorr[iBar+1]!=-999)  tDiffDiff_pulseIntCorr = tD_pulseIntCorr[iBar+1] ;

	if (tD_pulseIntCorr_posCorr[iBar]!=-999 && tD_pulseIntCorr_posCorr[iBar+1]!=-999)       tDiffDiff_pulseIntCorr_posCorr = tD_pulseIntCorr_posCorr[iBar] - tD_pulseIntCorr_posCorr[iBar+1];
	else if (tD_pulseIntCorr_posCorr[iBar]!=-999 && tD_pulseIntCorr_posCorr[iBar+1]==-999)  tDiffDiff_pulseIntCorr_posCorr = tD_pulseIntCorr_posCorr[iBar] ;
	else if (tD_pulseIntCorr_posCorr[iBar]==-999 && tD_pulseIntCorr_posCorr[iBar+1]!=-999)  tDiffDiff_pulseIntCorr_posCorr = tD_pulseIntCorr_posCorr[iBar+1] ;
	
      }
      

      
      
      int xbin = int ((posX-cut_Xmin[0])/xbinWidth );
      int ybin = int ((posY-cut_Ymin[NBARS-1])/ybinWidth );
            
      // -- plain average
      h_tAveSum_ampCorr->Fill(tAveSum_ampCorr);
      h_tAveSum_ampCorr_posCorr->Fill(tAveSum_ampCorr_posCorr);
      h_tAveSum_ampCorr_binX[xbin] ->Fill(tAveSum_ampCorr);
      h_tAveSum_ampCorr_posCorr_binX[xbin] ->Fill(tAveSum_ampCorr_posCorr);
      h_tAveSum_ampCorr_binY[ybin] ->Fill(tAveSum_ampCorr);
      h_tAveSum_ampCorr_posCorr_binY[ybin] ->Fill(tAveSum_ampCorr_posCorr);
      p_tAveSum_ampCorr_vs_posX ->Fill(posX,tAveSum_ampCorr);
      p_tAveSum_ampCorr_posCorr_vs_posX ->Fill(posX,tAveSum_ampCorr_posCorr);
      p_tAveSum_ampCorr_vs_posY ->Fill(posY,tAveSum_ampCorr);
      p_tAveSum_ampCorr_posCorr_vs_posY ->Fill(posY,tAveSum_ampCorr_posCorr);
      
      h_tAveSum_pulseIntCorr->Fill(tAveSum_pulseIntCorr);
      h_tAveSum_pulseIntCorr_posCorr->Fill(tAveSum_pulseIntCorr_posCorr);
      h_tAveSum_pulseIntCorr_binX[xbin] ->Fill(tAveSum_pulseIntCorr);
      h_tAveSum_pulseIntCorr_posCorr_binX[xbin] ->Fill(tAveSum_pulseIntCorr_posCorr);
      h_tAveSum_pulseIntCorr_binY[ybin] ->Fill(tAveSum_pulseIntCorr);
      h_tAveSum_pulseIntCorr_posCorr_binY[ybin] ->Fill(tAveSum_pulseIntCorr_posCorr);
      p_tAveSum_pulseIntCorr_vs_posX ->Fill(posX,tAveSum_pulseIntCorr);
      p_tAveSum_pulseIntCorr_posCorr_vs_posX ->Fill(posX,tAveSum_pulseIntCorr_posCorr);
      p_tAveSum_pulseIntCorr_vs_posY ->Fill(posY,tAveSum_pulseIntCorr);
      p_tAveSum_pulseIntCorr_posCorr_vs_posY ->Fill(posY,tAveSum_pulseIntCorr_posCorr);
      
      // -- amp weighted average
      h_tAveAmpWSum_ampCorr->Fill(tAmpWSum_ampCorr);
      h_tAveAmpWSum_ampCorr_posCorr->Fill(tAmpWSum_ampCorr_posCorr);
      h_tAveAmpWSum_ampCorr_binX[xbin] ->Fill(tAmpWSum_ampCorr);
      h_tAveAmpWSum_ampCorr_posCorr_binX[xbin] ->Fill(tAmpWSum_ampCorr_posCorr);
      h_tAveAmpWSum_ampCorr_binY[ybin] ->Fill(tAmpWSum_ampCorr);
      h_tAveAmpWSum_ampCorr_posCorr_binY[ybin] ->Fill(tAmpWSum_ampCorr_posCorr);
      p_tAveAmpWSum_ampCorr_vs_posX ->Fill(posX,tAmpWSum_ampCorr);
      p_tAveAmpWSum_ampCorr_posCorr_vs_posX ->Fill(posX,tAmpWSum_ampCorr_posCorr);
      p_tAveAmpWSum_ampCorr_vs_posY ->Fill(posY,tAmpWSum_ampCorr);
      p_tAveAmpWSum_ampCorr_posCorr_vs_posY ->Fill(posY,tAmpWSum_ampCorr_posCorr);

      h_tAveAmpWSum_pulseIntCorr->Fill(tAmpWSum_pulseIntCorr);
      h_tAveAmpWSum_pulseIntCorr_posCorr->Fill(tAmpWSum_pulseIntCorr_posCorr);
      h_tAveAmpWSum_pulseIntCorr_binX[xbin] ->Fill(tAmpWSum_pulseIntCorr);
      h_tAveAmpWSum_pulseIntCorr_posCorr_binX[xbin] ->Fill(tAmpWSum_pulseIntCorr_posCorr);
      h_tAveAmpWSum_pulseIntCorr_binY[ybin] ->Fill(tAmpWSum_pulseIntCorr);
      h_tAveAmpWSum_pulseIntCorr_posCorr_binY[ybin] ->Fill(tAmpWSum_pulseIntCorr_posCorr);
      p_tAveAmpWSum_pulseIntCorr_vs_posX ->Fill(posX,tAmpWSum_pulseIntCorr);
      p_tAveAmpWSum_pulseIntCorr_posCorr_vs_posX ->Fill(posX,tAmpWSum_pulseIntCorr_posCorr);
      p_tAveAmpWSum_pulseIntCorr_vs_posY ->Fill(posY,tAmpWSum_pulseIntCorr);
      p_tAveAmpWSum_pulseIntCorr_posCorr_vs_posY ->Fill(posY,tAmpWSum_pulseIntCorr_posCorr);
   

      // -- time resol weighted average
      h_tAveResWSum_ampCorr->Fill(tResWSum_ampCorr);
      h_tAveResWSum_ampCorr_posCorr->Fill(tResWSum_ampCorr_posCorr);
      h_tAveResWSum_ampCorr_binX[xbin] ->Fill(tResWSum_ampCorr);
      h_tAveResWSum_ampCorr_posCorr_binX[xbin] ->Fill(tResWSum_ampCorr_posCorr);
      h_tAveResWSum_ampCorr_binY[ybin] ->Fill(tResWSum_ampCorr);
      h_tAveResWSum_ampCorr_posCorr_binY[ybin] ->Fill(tResWSum_ampCorr_posCorr);
      p_tAveResWSum_ampCorr_vs_posX ->Fill(posX,tResWSum_ampCorr);
      p_tAveResWSum_ampCorr_posCorr_vs_posX ->Fill(posX,tResWSum_ampCorr_posCorr);
      p_tAveResWSum_ampCorr_vs_posY ->Fill(posY,tResWSum_ampCorr);
      p_tAveResWSum_ampCorr_posCorr_vs_posY ->Fill(posY,tResWSum_ampCorr_posCorr);
      
      h_tAveResWSum_pulseIntCorr->Fill(tResWSum_pulseIntCorr);
      h_tAveResWSum_pulseIntCorr_posCorr->Fill(tResWSum_pulseIntCorr_posCorr);
      h_tAveResWSum_pulseIntCorr_binX[xbin] ->Fill(tResWSum_pulseIntCorr);
      h_tAveResWSum_pulseIntCorr_posCorr_binX[xbin] ->Fill(tResWSum_pulseIntCorr_posCorr);
      h_tAveResWSum_pulseIntCorr_binY[ybin] ->Fill(tResWSum_pulseIntCorr);
      h_tAveResWSum_pulseIntCorr_posCorr_binY[ybin] ->Fill(tResWSum_pulseIntCorr_posCorr);
      p_tAveResWSum_pulseIntCorr_vs_posX ->Fill(posX,tResWSum_pulseIntCorr);
      p_tAveResWSum_pulseIntCorr_posCorr_vs_posX ->Fill(posX,tResWSum_pulseIntCorr_posCorr);
      p_tAveResWSum_pulseIntCorr_vs_posY ->Fill(posY,tResWSum_pulseIntCorr);
      p_tAveResWSum_pulseIntCorr_posCorr_vs_posY ->Fill(posY,tResWSum_pulseIntCorr_posCorr);
      
      // -- diff between tAve of the two bars
      h_tAveDiff_ampCorr->Fill(tAveDiff_ampCorr);
      h_tAveDiff_ampCorr_posCorr->Fill(tAveDiff_ampCorr_posCorr);
      h_tAveDiff_ampCorr_binX[xbin] ->Fill(tAveDiff_ampCorr);
      h_tAveDiff_ampCorr_posCorr_binX[xbin] ->Fill(tAveDiff_ampCorr_posCorr);
      h_tAveDiff_ampCorr_binY[ybin] ->Fill(tAveDiff_ampCorr);
      h_tAveDiff_ampCorr_posCorr_binY[ybin] ->Fill(tAveDiff_ampCorr_posCorr);
      p_tAveDiff_ampCorr_vs_posX ->Fill(posX,tAveDiff_ampCorr);
      p_tAveDiff_ampCorr_posCorr_vs_posX ->Fill(posX,tAveDiff_ampCorr_posCorr);
      p_tAveDiff_ampCorr_vs_posY ->Fill(posY,tAveDiff_ampCorr);
      p_tAveDiff_ampCorr_posCorr_vs_posY ->Fill(posY,tAveDiff_ampCorr_posCorr);
      
      h_tAveDiff_pulseIntCorr->Fill(tAveDiff_pulseIntCorr);
      h_tAveDiff_pulseIntCorr_posCorr->Fill(tAveDiff_pulseIntCorr_posCorr);
      h_tAveDiff_pulseIntCorr_binX[xbin] ->Fill(tAveDiff_pulseIntCorr);
      h_tAveDiff_pulseIntCorr_posCorr_binX[xbin] ->Fill(tAveDiff_pulseIntCorr_posCorr);
      h_tAveDiff_pulseIntCorr_binY[ybin] ->Fill(tAveDiff_pulseIntCorr);
      h_tAveDiff_pulseIntCorr_posCorr_binY[ybin] ->Fill(tAveDiff_pulseIntCorr_posCorr);
      p_tAveDiff_pulseIntCorr_vs_posX ->Fill(posX,tAveDiff_pulseIntCorr);
      p_tAveDiff_pulseIntCorr_posCorr_vs_posX ->Fill(posX,tAveDiff_pulseIntCorr_posCorr);
      p_tAveDiff_pulseIntCorr_vs_posY ->Fill(posY,tAveDiff_pulseIntCorr);
      p_tAveDiff_pulseIntCorr_posCorr_vs_posY ->Fill(posY,tAveDiff_pulseIntCorr_posCorr);

      // -- diff between the tDiff of the two bars
      h_tDiffDiff_ampCorr->Fill(tDiffDiff_ampCorr);
      h_tDiffDiff_ampCorr_posCorr->Fill(tDiffDiff_ampCorr_posCorr);
      h_tDiffDiff_ampCorr_binX[xbin] ->Fill(tDiffDiff_ampCorr);
      h_tDiffDiff_ampCorr_posCorr_binX[xbin] ->Fill(tDiffDiff_ampCorr_posCorr);
      h_tDiffDiff_ampCorr_binY[ybin] ->Fill(tDiffDiff_ampCorr);
      h_tDiffDiff_ampCorr_posCorr_binY[ybin] ->Fill(tDiffDiff_ampCorr_posCorr);
      p_tDiffDiff_ampCorr_vs_posX ->Fill(posX,tDiffDiff_ampCorr);
      p_tDiffDiff_ampCorr_posCorr_vs_posX ->Fill(posX,tDiffDiff_ampCorr_posCorr);
      p_tDiffDiff_ampCorr_vs_posY ->Fill(posY,tDiffDiff_ampCorr);
      p_tDiffDiff_ampCorr_posCorr_vs_posY ->Fill(posY,tDiffDiff_ampCorr_posCorr);
      
      h_tDiffDiff_pulseIntCorr->Fill(tDiffDiff_pulseIntCorr);
      h_tDiffDiff_pulseIntCorr_posCorr->Fill(tDiffDiff_pulseIntCorr_posCorr);
      h_tDiffDiff_pulseIntCorr_binX[xbin] ->Fill(tDiffDiff_pulseIntCorr);
      h_tDiffDiff_pulseIntCorr_posCorr_binX[xbin] ->Fill(tDiffDiff_pulseIntCorr_posCorr);
      h_tDiffDiff_pulseIntCorr_binY[ybin] ->Fill(tDiffDiff_pulseIntCorr);
      h_tDiffDiff_pulseIntCorr_posCorr_binY[ybin] ->Fill(tDiffDiff_pulseIntCorr_posCorr);
      p_tDiffDiff_pulseIntCorr_vs_posX ->Fill(posX,tDiffDiff_pulseIntCorr);
      p_tDiffDiff_pulseIntCorr_posCorr_vs_posX ->Fill(posX,tDiffDiff_pulseIntCorr_posCorr);
      p_tDiffDiff_pulseIntCorr_vs_posY ->Fill(posY,tDiffDiff_pulseIntCorr);
      p_tDiffDiff_pulseIntCorr_posCorr_vs_posY ->Fill(posY,tDiffDiff_pulseIntCorr_posCorr);
      
    } //--- end fourth loop over events

  cout << "End fourth loop over events "  <<endl;

  // graphs for bars combination
  TF1*  fitFunAveSum_ampCorr_binX[1000];
  TF1*  fitFunAveSum_ampCorr_posCorr_binX[1000];

  TF1*  fitFunAveSum_ampCorr_binY[1000];
  TF1*  fitFunAveSum_ampCorr_posCorr_binY[1000];

  TF1*  fitFunAmpWSum_ampCorr_binX[1000];
  TF1*  fitFunAmpWSum_ampCorr_posCorr_binX[1000];

  TF1*  fitFunAmpWSum_ampCorr_binY[1000];
  TF1*  fitFunAmpWSum_ampCorr_posCorr_binY[1000];

  TF1*  fitFunResWSum_ampCorr_binX[1000];
  TF1*  fitFunResWSum_ampCorr_posCorr_binX[1000];

  TF1*  fitFunResWSum_ampCorr_binY[1000];
  TF1*  fitFunResWSum_ampCorr_posCorr_binY[1000];

  TF1*  fitFunAveDiff_ampCorr_binX[1000];
  TF1*  fitFunAveDiff_ampCorr_posCorr_binX[1000];

  TF1*  fitFunAveDiff_ampCorr_binY[1000];
  TF1*  fitFunAveDiff_ampCorr_posCorr_binY[1000];

  TF1*  fitFunDiffDiff_ampCorr_binX[1000];
  TF1*  fitFunDiffDiff_ampCorr_posCorr_binX[1000];

  TF1*  fitFunDiffDiff_ampCorr_binY[1000];
  TF1*  fitFunDiffDiff_ampCorr_posCorr_binY[1000];


  TF1*  fitFunAveSum_pulseIntCorr_binX[1000];
  TF1*  fitFunAveSum_pulseIntCorr_posCorr_binX[1000];

  TF1*  fitFunAveSum_pulseIntCorr_binY[1000];
  TF1*  fitFunAveSum_pulseIntCorr_posCorr_binY[1000];

  TF1*  fitFunAmpWSum_pulseIntCorr_binX[1000];
  TF1*  fitFunAmpWSum_pulseIntCorr_posCorr_binX[1000];

  TF1*  fitFunAmpWSum_pulseIntCorr_binY[1000];
  TF1*  fitFunAmpWSum_pulseIntCorr_posCorr_binY[1000];

  TF1*  fitFunResWSum_pulseIntCorr_binX[1000];
  TF1*  fitFunResWSum_pulseIntCorr_posCorr_binX[1000];

  TF1*  fitFunResWSum_pulseIntCorr_binY[1000];
  TF1*  fitFunResWSum_pulseIntCorr_posCorr_binY[1000];

  TF1*  fitFunAveDiff_pulseIntCorr_binX[1000];
  TF1*  fitFunAveDiff_pulseIntCorr_posCorr_binX[1000];

  TF1*  fitFunAveDiff_pulseIntCorr_binY[1000];
  TF1*  fitFunAveDiff_pulseIntCorr_posCorr_binY[1000];

  TF1*  fitFunDiffDiff_pulseIntCorr_binX[1000];
  TF1*  fitFunDiffDiff_pulseIntCorr_posCorr_binX[1000];

  TF1*  fitFunDiffDiff_pulseIntCorr_binY[1000];
  TF1*  fitFunDiffDiff_pulseIntCorr_posCorr_binY[1000];

  TGraphErrors *g_tResolGausAveSum_ampCorr_vs_posX = new TGraphErrors();
  g_tResolGausAveSum_ampCorr_vs_posX->SetName("g_tResolGausAveSum_ampCorr_vs_posX");

  TGraphErrors *g_tResolGausAveSum_ampCorr_posCorr_vs_posX = new TGraphErrors();
  g_tResolGausAveSum_ampCorr_posCorr_vs_posX->SetName("g_tResolGausAveSum_ampCorr_posCorr_vs_posX");

  TGraphErrors *g_tResolGausAveSum_ampCorr_vs_posY = new TGraphErrors();
  g_tResolGausAveSum_ampCorr_vs_posY->SetName("g_tResolGausAveSum_ampCorr_vs_posY");

  TGraphErrors *g_tResolGausAveSum_ampCorr_posCorr_vs_posY = new TGraphErrors();
  g_tResolGausAveSum_ampCorr_posCorr_vs_posY->SetName("g_tResolGausAveSum_ampCorr_posCorr_vs_posY");

  TGraphErrors *g_tResolGausAmpWSum_ampCorr_vs_posX = new TGraphErrors();
  g_tResolGausAmpWSum_ampCorr_vs_posX->SetName("g_tResolGausAmpWSum_ampCorr_vs_posX");

  TGraphErrors *g_tResolGausAmpWSum_ampCorr_posCorr_vs_posX = new TGraphErrors();
  g_tResolGausAmpWSum_ampCorr_posCorr_vs_posX->SetName("g_tResolGausAmpWSum_ampCorr_posCorr_vs_posX");

  TGraphErrors *g_tResolGausAmpWSum_ampCorr_vs_posY = new TGraphErrors();
  g_tResolGausAmpWSum_ampCorr_vs_posY->SetName("g_tResolGausAmpWSum_ampCorr_vs_posY");

  TGraphErrors *g_tResolGausAmpWSum_ampCorr_posCorr_vs_posY = new TGraphErrors();
  g_tResolGausAmpWSum_ampCorr_posCorr_vs_posY->SetName("g_tResolGausAmpWSum_ampCorr_posCorr_vs_posY");

  TGraphErrors *g_tResolGausResWSum_ampCorr_vs_posX = new TGraphErrors();
  g_tResolGausResWSum_ampCorr_vs_posX->SetName("g_tResolGausResWSum_ampCorr_vs_posX");

  TGraphErrors *g_tResolGausResWSum_ampCorr_posCorr_vs_posX = new TGraphErrors();
  g_tResolGausResWSum_ampCorr_posCorr_vs_posX->SetName("g_tResolGausResWSum_ampCorr_posCorr_vs_posX");

  TGraphErrors *g_tResolGausResWSum_ampCorr_vs_posY = new TGraphErrors();
  g_tResolGausResWSum_ampCorr_vs_posY->SetName("g_tResolGausResWSum_ampCorr_vs_posY");

  TGraphErrors *g_tResolGausResWSum_ampCorr_posCorr_vs_posY = new TGraphErrors();
  g_tResolGausResWSum_ampCorr_posCorr_vs_posY->SetName("g_tResolGausResWSum_ampCorr_posCorr_vs_posY");

  TGraphErrors *g_tResolGausAveDiff_ampCorr_vs_posX = new TGraphErrors();
  g_tResolGausAveDiff_ampCorr_vs_posX->SetName("g_tResolGausAveDiff_ampCorr_vs_posX");

  TGraphErrors *g_tResolGausAveDiff_ampCorr_posCorr_vs_posX = new TGraphErrors();
  g_tResolGausAveDiff_ampCorr_posCorr_vs_posX->SetName("g_tResolGausAveDiff_ampCorr_posCorr_vs_posX");

  TGraphErrors *g_tResolGausAveDiff_ampCorr_vs_posY = new TGraphErrors();
  g_tResolGausAveDiff_ampCorr_vs_posY->SetName("g_tResolGausAveDiff_ampCorr_vs_posY");

  TGraphErrors *g_tResolGausAveDiff_ampCorr_posCorr_vs_posY = new TGraphErrors();
  g_tResolGausAveDiff_ampCorr_posCorr_vs_posY->SetName("g_tResolGausAveDiff_ampCorr_posCorr_vs_posY");


  TGraphErrors *g_tResolGausDiffDiff_ampCorr_vs_posX = new TGraphErrors();
  g_tResolGausDiffDiff_ampCorr_vs_posX->SetName("g_tResolGausDiffDiff_ampCorr_vs_posX");

  TGraphErrors *g_tResolGausDiffDiff_ampCorr_posCorr_vs_posX = new TGraphErrors();
  g_tResolGausDiffDiff_ampCorr_posCorr_vs_posX->SetName("g_tResolGausDiffDiff_ampCorr_posCorr_vs_posX");

  TGraphErrors *g_tResolGausDiffDiff_ampCorr_vs_posY = new TGraphErrors();
  g_tResolGausDiffDiff_ampCorr_vs_posY->SetName("g_tResolGausDiffDiff_ampCorr_vs_posY");

  TGraphErrors *g_tResolGausDiffDiff_ampCorr_posCorr_vs_posY = new TGraphErrors();
  g_tResolGausDiffDiff_ampCorr_posCorr_vs_posY->SetName("g_tResolGausDiffDiff_ampCorr_posCorr_vs_posY");

  // vs pulseInt
  TGraphErrors *g_tResolGausAveSum_pulseIntCorr_vs_posX = new TGraphErrors();
  g_tResolGausAveSum_pulseIntCorr_vs_posX->SetName("g_tResolGausAveSum_pulseIntCorr_vs_posX");

  TGraphErrors *g_tResolGausAveSum_pulseIntCorr_posCorr_vs_posX = new TGraphErrors();
  g_tResolGausAveSum_pulseIntCorr_posCorr_vs_posX->SetName("g_tResolGausAveSum_pulseIntCorr_posCorr_vs_posX");

  TGraphErrors *g_tResolGausAveSum_pulseIntCorr_vs_posY = new TGraphErrors();
  g_tResolGausAveSum_pulseIntCorr_vs_posY->SetName("g_tResolGausAveSum_pulseIntCorr_vs_posY");

  TGraphErrors *g_tResolGausAveSum_pulseIntCorr_posCorr_vs_posY = new TGraphErrors();
  g_tResolGausAveSum_pulseIntCorr_posCorr_vs_posY->SetName("g_tResolGausAveSum_pulseIntCorr_posCorr_vs_posY");

  TGraphErrors *g_tResolGausAmpWSum_pulseIntCorr_vs_posX = new TGraphErrors();
  g_tResolGausAmpWSum_pulseIntCorr_vs_posX->SetName("g_tResolGausAmpWSum_pulseIntCorr_vs_posX");

  TGraphErrors *g_tResolGausAmpWSum_pulseIntCorr_posCorr_vs_posX = new TGraphErrors();
  g_tResolGausAmpWSum_pulseIntCorr_posCorr_vs_posX->SetName("g_tResolGausAmpWSum_pulseIntCorr_posCorr_vs_posX");

  TGraphErrors *g_tResolGausAmpWSum_pulseIntCorr_vs_posY = new TGraphErrors();
  g_tResolGausAmpWSum_pulseIntCorr_vs_posY->SetName("g_tResolGausAmpWSum_pulseIntCorr_vs_posY");

  TGraphErrors *g_tResolGausAmpWSum_pulseIntCorr_posCorr_vs_posY = new TGraphErrors();
  g_tResolGausAmpWSum_pulseIntCorr_posCorr_vs_posY->SetName("g_tResolGausAmpWSum_pulseIntCorr_posCorr_vs_posY");

  TGraphErrors *g_tResolGausResWSum_pulseIntCorr_vs_posX = new TGraphErrors();
  g_tResolGausResWSum_pulseIntCorr_vs_posX->SetName("g_tResolGausResWSum_pulseIntCorr_vs_posX");

  TGraphErrors *g_tResolGausResWSum_pulseIntCorr_posCorr_vs_posX = new TGraphErrors();
  g_tResolGausResWSum_pulseIntCorr_posCorr_vs_posX->SetName("g_tResolGausResWSum_pulseIntCorr_posCorr_vs_posX");

  TGraphErrors *g_tResolGausResWSum_pulseIntCorr_vs_posY = new TGraphErrors();
  g_tResolGausResWSum_pulseIntCorr_vs_posY->SetName("g_tResolGausResWSum_pulseIntCorr_vs_posY");

  TGraphErrors *g_tResolGausResWSum_pulseIntCorr_posCorr_vs_posY = new TGraphErrors();
  g_tResolGausResWSum_pulseIntCorr_posCorr_vs_posY->SetName("g_tResolGausResWSum_pulseIntCorr_posCorr_vs_posY");


  // diff of Tave
  TGraphErrors *g_tResolGausAveDiff_pulseIntCorr_vs_posX = new TGraphErrors();
  g_tResolGausAveDiff_pulseIntCorr_vs_posX->SetName("g_tResolGausAveDiff_pulseIntCorr_vs_posX");

  TGraphErrors *g_tResolGausAveDiff_pulseIntCorr_posCorr_vs_posX = new TGraphErrors();
  g_tResolGausAveDiff_pulseIntCorr_posCorr_vs_posX->SetName("g_tResolGausAveDiff_pulseIntCorr_posCorr_vs_posX");

  TGraphErrors *g_tResolGausAveDiff_pulseIntCorr_vs_posY = new TGraphErrors();
  g_tResolGausAveDiff_pulseIntCorr_vs_posY->SetName("g_tResolGausAveDiff_pulseIntCorr_vs_posY");

  TGraphErrors *g_tResolGausAveDiff_pulseIntCorr_posCorr_vs_posY = new TGraphErrors();
  g_tResolGausAveDiff_pulseIntCorr_posCorr_vs_posY->SetName("g_tResolGausAveDiff_pulseIntCorr_posCorr_vs_posY");


  // diff of diffs
  TGraphErrors *g_tResolGausDiffDiff_pulseIntCorr_vs_posX = new TGraphErrors();
  g_tResolGausDiffDiff_pulseIntCorr_vs_posX->SetName("g_tResolGausDiffDiff_pulseIntCorr_vs_posX");

  TGraphErrors *g_tResolGausDiffDiff_pulseIntCorr_posCorr_vs_posX = new TGraphErrors();
  g_tResolGausDiffDiff_pulseIntCorr_posCorr_vs_posX->SetName("g_tResolGausDiffDiff_pulseIntCorr_posCorr_vs_posX");

  TGraphErrors *g_tResolGausDiffDiff_pulseIntCorr_vs_posY = new TGraphErrors();
  g_tResolGausDiffDiff_pulseIntCorr_vs_posY->SetName("g_tResolGausDiffDiff_pulseIntCorr_vs_posY");

  TGraphErrors *g_tResolGausDiffDiff_pulseIntCorr_posCorr_vs_posY = new TGraphErrors();
  g_tResolGausDiffDiff_pulseIntCorr_posCorr_vs_posY->SetName("g_tResolGausDiffDiff_pulseIntCorr_posCorr_vs_posY");


  // -- vs X
  for (int ibin = 0; ibin < NBINSX[0]; ibin++){

    float xx = cut_Xmin[0] + xbinWidth*ibin + xbinWidth/2;

    // == amp corr

    // -- simple average
    fitFunAveSum_ampCorr_binX[ibin]= new TF1(Form("fitFunAveSum_ampCorr_binX%d",ibin),"gaus", -20, 20);
    GetTimeResolution(h_tAveSum_ampCorr_binX[ibin], fitFunAveSum_ampCorr_binX[ibin], resolEff, resolGaus, resolGausErr, resolMCP);
    g_tResolGausAveSum_ampCorr_vs_posX->SetPoint(ibin, xx, resolGaus*1000.);
    g_tResolGausAveSum_ampCorr_vs_posX->SetPointError(ibin, xbinWidth/2., resolGausErr*1000.);
    
    fitFunAveSum_ampCorr_posCorr_binX[ibin]= new TF1(Form("fitFunAveSum_ampCorr_posCorr_binX%d",ibin),"gaus", -20, 20);
    GetTimeResolution(h_tAveSum_ampCorr_posCorr_binX[ibin], fitFunAveSum_ampCorr_posCorr_binX[ibin], resolEff, resolGaus, resolGausErr, resolMCP);
    g_tResolGausAveSum_ampCorr_posCorr_vs_posX->SetPoint(ibin, xx, resolGaus*1000.);
    g_tResolGausAveSum_ampCorr_posCorr_vs_posX->SetPointError(ibin, xbinWidth/2., resolGausErr*1000.);

    // - amp weighted average
    fitFunAmpWSum_ampCorr_binX[ibin]= new TF1(Form("fitFunAmpWSum_ampCorr_binX%d",ibin),"gaus", -20, 20);
    GetTimeResolution(h_tAveAmpWSum_ampCorr_binX[ibin], fitFunAmpWSum_ampCorr_binX[ibin], resolEff, resolGaus, resolGausErr, resolMCP);
    g_tResolGausAmpWSum_ampCorr_vs_posX->SetPoint(ibin, xx, resolGaus*1000.);
    g_tResolGausAmpWSum_ampCorr_vs_posX->SetPointError(ibin, xbinWidth/2., resolGausErr*1000.);
    
    fitFunAmpWSum_ampCorr_posCorr_binX[ibin]= new TF1(Form("fitFunAmpWSum_ampCorr_posCorr_binX%d",ibin),"gaus", -20, 20);
    GetTimeResolution(h_tAveAmpWSum_ampCorr_posCorr_binX[ibin], fitFunAmpWSum_ampCorr_posCorr_binX[ibin], resolEff, resolGaus, resolGausErr, resolMCP);
    g_tResolGausAmpWSum_ampCorr_posCorr_vs_posX->SetPoint(ibin, xx, resolGaus*1000.);
    g_tResolGausAmpWSum_ampCorr_posCorr_vs_posX->SetPointError(ibin, xbinWidth/2., resolGausErr*1000.);
    
    // - res weighted average
    fitFunResWSum_ampCorr_binX[ibin]= new TF1(Form("fitFunResWSum_ampCorr_binX%d",ibin),"gaus", -20, 20);
    GetTimeResolution(h_tAveResWSum_ampCorr_binX[ibin], fitFunResWSum_ampCorr_binX[ibin], resolEff, resolGaus, resolGausErr, resolMCP);
    g_tResolGausResWSum_ampCorr_vs_posX->SetPoint(ibin, xx, resolGaus*1000.);
    g_tResolGausResWSum_ampCorr_vs_posX->SetPointError(ibin, xbinWidth/2., resolGausErr*1000.);
    
    fitFunResWSum_ampCorr_posCorr_binX[ibin]= new TF1(Form("fitFunResWSum_ampCorr_posCorr_binX%d",ibin),"gaus", -20, 20);
    GetTimeResolution(h_tAveResWSum_ampCorr_posCorr_binX[ibin], fitFunResWSum_ampCorr_posCorr_binX[ibin], resolEff, resolGaus, resolGausErr, resolMCP);
    g_tResolGausResWSum_ampCorr_posCorr_vs_posX->SetPoint(ibin, xx, resolGaus*1000.);
    g_tResolGausResWSum_ampCorr_posCorr_vs_posX->SetPointError(ibin, xbinWidth/2., resolGausErr*1000.);
        
    // -- bars tAve diff
    fitFunAveDiff_ampCorr_binX[ibin]= new TF1(Form("fitFunAveDiff_ampCorr_binX%d",ibin),"gaus", -20, 20);
    GetTimeResolution(h_tAveDiff_ampCorr_binX[ibin], fitFunAveDiff_ampCorr_binX[ibin], resolEff, resolGaus, resolGausErr, -1);
    g_tResolGausAveDiff_ampCorr_vs_posX->SetPoint(ibin, xx, resolGaus*1000.);
    g_tResolGausAveDiff_ampCorr_vs_posX->SetPointError(ibin, xbinWidth/2., resolGausErr*1000.);
    
    fitFunAveDiff_ampCorr_posCorr_binX[ibin]= new TF1(Form("fitFunAveDiff_ampCorr_posCorr_binX%d",ibin),"gaus", -20, 20);
    GetTimeResolution(h_tAveDiff_ampCorr_posCorr_binX[ibin], fitFunAveDiff_ampCorr_posCorr_binX[ibin], resolEff, resolGaus, resolGausErr, -1);
    g_tResolGausAveDiff_ampCorr_posCorr_vs_posX->SetPoint(ibin, xx, resolGaus*1000.);
    g_tResolGausAveDiff_ampCorr_posCorr_vs_posX->SetPointError(ibin, xbinWidth/2., resolGausErr*1000.);
    
    // -- bars tDiff diff
    fitFunDiffDiff_ampCorr_binX[ibin]= new TF1(Form("fitFunDiffDiff_ampCorr_binX%d",ibin),"gaus", -20, 20);
    GetTimeResolution(h_tDiffDiff_ampCorr_binX[ibin], fitFunDiffDiff_ampCorr_binX[ibin], resolEff, resolGaus, resolGausErr, -1);
    g_tResolGausDiffDiff_ampCorr_vs_posX->SetPoint(ibin, xx, resolGaus*1000.);
    g_tResolGausDiffDiff_ampCorr_vs_posX->SetPointError(ibin, xbinWidth/2., resolGausErr*1000.);
    
    fitFunDiffDiff_ampCorr_posCorr_binX[ibin]= new TF1(Form("fitFunDiffDiff_ampCorr_posCorr_binX%d",ibin),"gaus", -20, 20);
    GetTimeResolution(h_tDiffDiff_ampCorr_posCorr_binX[ibin], fitFunDiffDiff_ampCorr_posCorr_binX[ibin], resolEff, resolGaus, resolGausErr, -1);
    g_tResolGausDiffDiff_ampCorr_posCorr_vs_posX->SetPoint(ibin, xx, resolGaus*1000.);
    g_tResolGausDiffDiff_ampCorr_posCorr_vs_posX->SetPointError(ibin, xbinWidth/2., resolGausErr*1000.);
    



    // === pulse int corr

    // -- simple average
    fitFunAveSum_pulseIntCorr_binX[ibin]= new TF1(Form("fitFunAveSum_pulseIntCorr_binX%d",ibin),"gaus", -20, 20);
    GetTimeResolution(h_tAveSum_pulseIntCorr_binX[ibin], fitFunAveSum_pulseIntCorr_binX[ibin], resolEff, resolGaus, resolGausErr, resolMCP);
    g_tResolGausAveSum_pulseIntCorr_vs_posX->SetPoint(ibin, xx, resolGaus*1000.);
    g_tResolGausAveSum_pulseIntCorr_vs_posX->SetPointError(ibin, xbinWidth/2., resolGausErr*1000.);
    
    fitFunAveSum_pulseIntCorr_posCorr_binX[ibin]= new TF1(Form("fitFunAveSum_pulseIntCorr_posCorr_binX%d",ibin),"gaus", -20, 20);
    GetTimeResolution(h_tAveSum_pulseIntCorr_posCorr_binX[ibin], fitFunAveSum_pulseIntCorr_posCorr_binX[ibin], resolEff, resolGaus, resolGausErr, resolMCP);
    g_tResolGausAveSum_pulseIntCorr_posCorr_vs_posX->SetPoint(ibin, xx, resolGaus*1000.);
    g_tResolGausAveSum_pulseIntCorr_posCorr_vs_posX->SetPointError(ibin, xbinWidth/2., resolGausErr*1000.);
    
    // - amp weighted average
    fitFunAmpWSum_pulseIntCorr_binX[ibin]= new TF1(Form("fitFunAmpWSum_pulseIntCorr_binX%d",ibin),"gaus", -20, 20);
    GetTimeResolution(h_tAveAmpWSum_pulseIntCorr_binX[ibin], fitFunAmpWSum_pulseIntCorr_binX[ibin], resolEff, resolGaus, resolGausErr, resolMCP);
    g_tResolGausAmpWSum_pulseIntCorr_vs_posX->SetPoint(ibin, xx, resolGaus*1000.);
    g_tResolGausAmpWSum_pulseIntCorr_vs_posX->SetPointError(ibin, xbinWidth/2., resolGausErr*1000.);
    
    fitFunAmpWSum_pulseIntCorr_posCorr_binX[ibin]= new TF1(Form("fitFunAmpWSum_pulseIntCorr_posCorr_binX%d",ibin),"gaus", -20, 20);
    GetTimeResolution(h_tAveAmpWSum_pulseIntCorr_posCorr_binX[ibin], fitFunAmpWSum_pulseIntCorr_posCorr_binX[ibin], resolEff, resolGaus, resolGausErr, resolMCP);
    g_tResolGausAmpWSum_pulseIntCorr_posCorr_vs_posX->SetPoint(ibin, xx, resolGaus*1000.);
    g_tResolGausAmpWSum_pulseIntCorr_posCorr_vs_posX->SetPointError(ibin, xbinWidth/2., resolGausErr*1000.);
    
    // - res weighted average
    fitFunResWSum_pulseIntCorr_binX[ibin]= new TF1(Form("fitFunResWSum_pulseIntCorr_binX%d",ibin),"gaus", -20, 20);
    GetTimeResolution(h_tAveResWSum_pulseIntCorr_binX[ibin], fitFunResWSum_pulseIntCorr_binX[ibin], resolEff, resolGaus, resolGausErr, resolMCP);
    g_tResolGausResWSum_pulseIntCorr_vs_posX->SetPoint(ibin, xx, resolGaus*1000.);
    g_tResolGausResWSum_pulseIntCorr_vs_posX->SetPointError(ibin, xbinWidth/2., resolGausErr*1000.);
    
    fitFunResWSum_pulseIntCorr_posCorr_binX[ibin]= new TF1(Form("fitFunResWSum_pulseIntCorr_posCorr_binX%d",ibin),"gaus", -20, 20);
    GetTimeResolution(h_tAveResWSum_pulseIntCorr_posCorr_binX[ibin], fitFunResWSum_pulseIntCorr_posCorr_binX[ibin], resolEff, resolGaus, resolGausErr, resolMCP);
    g_tResolGausResWSum_pulseIntCorr_posCorr_vs_posX->SetPoint(ibin, xx, resolGaus*1000.);
    g_tResolGausResWSum_pulseIntCorr_posCorr_vs_posX->SetPointError(ibin, xbinWidth/2., resolGausErr*1000.);
    
    // -- bars tAve diff
    fitFunAveDiff_pulseIntCorr_binX[ibin]= new TF1(Form("fitFunAveDiff_pulseIntCorr_binX%d",ibin),"gaus", -20, 20);
    GetTimeResolution(h_tAveDiff_pulseIntCorr_binX[ibin], fitFunAveDiff_pulseIntCorr_binX[ibin], resolEff, resolGaus, resolGausErr, -1);
    g_tResolGausAveDiff_pulseIntCorr_vs_posX->SetPoint(ibin, xx, resolGaus*1000.);
    g_tResolGausAveDiff_pulseIntCorr_vs_posX->SetPointError(ibin, xbinWidth/2., resolGausErr*1000.);
    
    fitFunAveDiff_pulseIntCorr_posCorr_binX[ibin]= new TF1(Form("fitFunAveDiff_pulseIntCorr_posCorr_binX%d",ibin),"gaus", -20, 20);
    GetTimeResolution(h_tAveDiff_pulseIntCorr_posCorr_binX[ibin], fitFunAveDiff_pulseIntCorr_posCorr_binX[ibin], resolEff, resolGaus, resolGausErr, -1);
    g_tResolGausAveDiff_pulseIntCorr_posCorr_vs_posX->SetPoint(ibin, xx, resolGaus*1000.);
    g_tResolGausAveDiff_pulseIntCorr_posCorr_vs_posX->SetPointError(ibin, xbinWidth/2., resolGausErr*1000.);
    
    // -- bars tDiff  diff
    fitFunDiffDiff_pulseIntCorr_binX[ibin]= new TF1(Form("fitFunDiffDiff_pulseIntCorr_binX%d",ibin),"gaus", -20, 20);
    GetTimeResolution(h_tDiffDiff_pulseIntCorr_binX[ibin], fitFunDiffDiff_pulseIntCorr_binX[ibin], resolEff, resolGaus, resolGausErr, -1);
    g_tResolGausDiffDiff_pulseIntCorr_vs_posX->SetPoint(ibin, xx, resolGaus*1000.);
    g_tResolGausDiffDiff_pulseIntCorr_vs_posX->SetPointError(ibin, xbinWidth/2., resolGausErr*1000.);
    
    fitFunDiffDiff_pulseIntCorr_posCorr_binX[ibin]= new TF1(Form("fitFunDiffDiff_pulseIntCorr_posCorr_binX%d",ibin),"gaus", -20, 20);
    GetTimeResolution(h_tDiffDiff_pulseIntCorr_posCorr_binX[ibin], fitFunDiffDiff_pulseIntCorr_posCorr_binX[ibin], resolEff, resolGaus, resolGausErr, -1);
    g_tResolGausDiffDiff_pulseIntCorr_posCorr_vs_posX->SetPoint(ibin, xx, resolGaus*1000.);
    g_tResolGausDiffDiff_pulseIntCorr_posCorr_vs_posX->SetPointError(ibin, xbinWidth/2., resolGausErr*1000.);
  
  }

  // -- vs Y
  int np = 0;
  int np1 = 0;
  for (int ibin = 0; ibin < NBINSY; ibin++){

    float yy = cut_Ymin[NBARS-1] + ybinWidth*ibin + ybinWidth/2;

    if (h_tAveSum_ampCorr_binY[ibin]->GetEntries() == 0 || h_tAveSum_ampCorr_posCorr_binY[ibin] == 0) continue;

    // corr vs amp
    // -- simple average
    fitFunAveSum_ampCorr_binY[ibin]= new TF1(Form("fitFunAveSum_ampCorr_binY%d",ibin),"gaus", -20, 20);
    GetTimeResolution(h_tAveSum_ampCorr_binY[ibin], fitFunAveSum_ampCorr_binY[ibin], resolEff, resolGaus, resolGausErr, resolMCP);
    g_tResolGausAveSum_ampCorr_vs_posY->SetPoint(np, yy, resolGaus*1000.);
    g_tResolGausAveSum_ampCorr_vs_posY->SetPointError(np, ybinWidth/2., resolGausErr*1000.);
        
    fitFunAveSum_ampCorr_posCorr_binY[ibin]= new TF1(Form("fitFunAveSum_ampCorr_posCorr_binY%d",ibin),"gaus", -20, 20);
    GetTimeResolution(h_tAveSum_ampCorr_posCorr_binY[ibin], fitFunAveSum_ampCorr_posCorr_binY[ibin], resolEff, resolGaus, resolGausErr, resolMCP);
    g_tResolGausAveSum_ampCorr_posCorr_vs_posY->SetPoint(np, yy, resolGaus*1000.);
    g_tResolGausAveSum_ampCorr_posCorr_vs_posY->SetPointError(np, ybinWidth/2., resolGausErr*1000.);

    // -- amp weighted average     
    fitFunAmpWSum_ampCorr_binY[ibin]= new TF1(Form("fitFunAmpWSum_ampCorr_binY%d",ibin),"gaus", -20, 20);
    GetTimeResolution(h_tAveAmpWSum_ampCorr_binY[ibin], fitFunAmpWSum_ampCorr_binY[ibin], resolEff, resolGaus, resolGausErr, resolMCP);
    g_tResolGausAmpWSum_ampCorr_vs_posY->SetPoint(np, yy, resolGaus*1000.);
    g_tResolGausAmpWSum_ampCorr_vs_posY->SetPointError(np, ybinWidth/2., resolGausErr*1000.);
    
    fitFunAmpWSum_ampCorr_posCorr_binY[ibin]= new TF1(Form("fitFunAmpWSum_ampCorr_posCorr_binY%d",ibin),"gaus", -20, 20);
    GetTimeResolution(h_tAveAmpWSum_ampCorr_posCorr_binY[ibin], fitFunAmpWSum_ampCorr_posCorr_binY[ibin], resolEff, resolGaus, resolGausErr, resolMCP);
    g_tResolGausAmpWSum_ampCorr_posCorr_vs_posY->SetPoint(np, yy, resolGaus*1000.);
    g_tResolGausAmpWSum_ampCorr_posCorr_vs_posY->SetPointError(np, ybinWidth/2., resolGausErr*1000.);
    
    // -- res weighted average     
    fitFunResWSum_ampCorr_binY[ibin]= new TF1(Form("fitFunResWSum_ampCorr_binY%d",ibin),"gaus", -20, 20);
    GetTimeResolution(h_tAveResWSum_ampCorr_binY[ibin], fitFunResWSum_ampCorr_binY[ibin], resolEff, resolGaus, resolGausErr, resolMCP);
    g_tResolGausResWSum_ampCorr_vs_posY->SetPoint(np, yy, resolGaus*1000.);
    g_tResolGausResWSum_ampCorr_vs_posY->SetPointError(np, ybinWidth/2., resolGausErr*1000.);
  
    fitFunResWSum_ampCorr_posCorr_binY[ibin]= new TF1(Form("fitFunResWSum_ampCorr_posCorr_binY%d",ibin),"gaus", -20, 20);
    GetTimeResolution(h_tAveResWSum_ampCorr_posCorr_binY[ibin], fitFunResWSum_ampCorr_posCorr_binY[ibin], resolEff, resolGaus, resolGausErr, resolMCP);
    g_tResolGausResWSum_ampCorr_posCorr_vs_posY->SetPoint(np, yy, resolGaus*1000.);
    g_tResolGausResWSum_ampCorr_posCorr_vs_posY->SetPointError(np, ybinWidth/2., resolGausErr*1000.);
    
    // -- tAve diff
    fitFunAveDiff_ampCorr_binY[ibin]= new TF1(Form("fitFunAveDiff_ampCorr_binY%d",ibin),"gaus", -20, 20);
    GetTimeResolution(h_tAveDiff_ampCorr_binY[ibin], fitFunAveDiff_ampCorr_binY[ibin], resolEff, resolGaus, resolGausErr, -1);
    g_tResolGausAveDiff_ampCorr_vs_posY->SetPoint(np, yy, resolGaus*1000.);
    g_tResolGausAveDiff_ampCorr_vs_posY->SetPointError(np, ybinWidth/2., resolGausErr*1000.);
        
    fitFunAveDiff_ampCorr_posCorr_binY[ibin]= new TF1(Form("fitFunAveDiff_ampCorr_posCorr_binY%d",ibin),"gaus", -20, 20);
    GetTimeResolution(h_tAveDiff_ampCorr_posCorr_binY[ibin], fitFunAveDiff_ampCorr_posCorr_binY[ibin], resolEff, resolGaus, resolGausErr, -1);
    g_tResolGausAveDiff_ampCorr_posCorr_vs_posY->SetPoint(np, yy, resolGaus*1000.);
    g_tResolGausAveDiff_ampCorr_posCorr_vs_posY->SetPointError(np, ybinWidth/2., resolGausErr*1000.);


    // -- tDiff diff
    fitFunDiffDiff_ampCorr_binY[ibin]= new TF1(Form("fitFunDiffDiff_ampCorr_binY%d",ibin),"gaus", -20, 20);
    GetTimeResolution(h_tDiffDiff_ampCorr_binY[ibin], fitFunDiffDiff_ampCorr_binY[ibin], resolEff, resolGaus, resolGausErr, -1);
    g_tResolGausDiffDiff_ampCorr_vs_posY->SetPoint(np, yy, resolGaus*1000.);
    g_tResolGausDiffDiff_ampCorr_vs_posY->SetPointError(np, ybinWidth/2., resolGausErr*1000.);
        
    fitFunDiffDiff_ampCorr_posCorr_binY[ibin]= new TF1(Form("fitFunDiffDiff_ampCorr_posCorr_binY%d",ibin),"gaus", -20, 20);
    GetTimeResolution(h_tDiffDiff_ampCorr_posCorr_binY[ibin], fitFunDiffDiff_ampCorr_posCorr_binY[ibin], resolEff, resolGaus, resolGausErr, -1);
    g_tResolGausDiffDiff_ampCorr_posCorr_vs_posY->SetPoint(np, yy, resolGaus*1000.);
    g_tResolGausDiffDiff_ampCorr_posCorr_vs_posY->SetPointError(np, ybinWidth/2., resolGausErr*1000.);
    

    // ===  corr vs pulse int

    // -- simple average
    fitFunAveSum_pulseIntCorr_binY[ibin]= new TF1(Form("fitFunAveSum_pulseIntCorr_binY%d",ibin),"gaus", -20, 20);
    GetTimeResolution(h_tAveSum_pulseIntCorr_binY[ibin], fitFunAveSum_pulseIntCorr_binY[ibin], resolEff, resolGaus, resolGausErr, resolMCP);
    g_tResolGausAveSum_pulseIntCorr_vs_posY->SetPoint(np, yy, resolGaus*1000.);
    g_tResolGausAveSum_pulseIntCorr_vs_posY->SetPointError(np, ybinWidth/2., resolGausErr*1000.);
      
    fitFunAveSum_pulseIntCorr_posCorr_binY[ibin]= new TF1(Form("fitFunAveSum_pulseIntCorr_posCorr_binY%d",ibin),"gaus", -20, 20);
    GetTimeResolution(h_tAveSum_pulseIntCorr_posCorr_binY[ibin], fitFunAveSum_pulseIntCorr_posCorr_binY[ibin], resolEff, resolGaus, resolGausErr, resolMCP);
    g_tResolGausAveSum_pulseIntCorr_posCorr_vs_posY->SetPoint(np, yy, resolGaus*1000.);
    g_tResolGausAveSum_pulseIntCorr_posCorr_vs_posY->SetPointError(np, ybinWidth/2., resolGausErr*1000.);

    // -- amp weighted average     
    fitFunAmpWSum_pulseIntCorr_binY[ibin]= new TF1(Form("fitFunAmpWSum_pulseIntCorr_binY%d",ibin),"gaus", -20, 20);
    GetTimeResolution(h_tAveAmpWSum_pulseIntCorr_binY[ibin], fitFunAmpWSum_pulseIntCorr_binY[ibin], resolEff, resolGaus, resolGausErr, resolMCP);
    g_tResolGausAmpWSum_pulseIntCorr_vs_posY->SetPoint(np, yy, resolGaus*1000.);
    g_tResolGausAmpWSum_pulseIntCorr_vs_posY->SetPointError(np, ybinWidth/2., resolGausErr*1000.);
    
    fitFunAmpWSum_pulseIntCorr_posCorr_binY[ibin]= new TF1(Form("fitFunAmpWSum_pulseIntCorr_posCorr_binY%d",ibin),"gaus", -20, 20);
    GetTimeResolution(h_tAveAmpWSum_pulseIntCorr_posCorr_binY[ibin], fitFunAmpWSum_pulseIntCorr_posCorr_binY[ibin], resolEff, resolGaus, resolGausErr, resolMCP);
    g_tResolGausAmpWSum_pulseIntCorr_posCorr_vs_posY->SetPoint(np, yy, resolGaus*1000.);
    g_tResolGausAmpWSum_pulseIntCorr_posCorr_vs_posY->SetPointError(np, ybinWidth/2., resolGausErr*1000.);
    
    // -- res weighted average     
    fitFunResWSum_pulseIntCorr_binY[ibin]= new TF1(Form("fitFunResWSum_pulseIntCorr_binY%d",ibin),"gaus", -20, 20);
    GetTimeResolution(h_tAveResWSum_pulseIntCorr_binY[ibin], fitFunResWSum_pulseIntCorr_binY[ibin], resolEff, resolGaus, resolGausErr, resolMCP);
    g_tResolGausResWSum_pulseIntCorr_vs_posY->SetPoint(np, yy, resolGaus*1000.);
    g_tResolGausResWSum_pulseIntCorr_vs_posY->SetPointError(np, ybinWidth/2., resolGausErr*1000.);
    
    fitFunResWSum_pulseIntCorr_posCorr_binY[ibin]= new TF1(Form("fitFunResWSum_pulseIntCorr_posCorr_binY%d",ibin),"gaus", -20, 20);
    GetTimeResolution(h_tAveResWSum_pulseIntCorr_posCorr_binY[ibin], fitFunResWSum_pulseIntCorr_posCorr_binY[ibin], resolEff, resolGaus, resolGausErr, resolMCP);
    g_tResolGausResWSum_pulseIntCorr_posCorr_vs_posY->SetPoint(np, yy, resolGaus*1000.);
    g_tResolGausResWSum_pulseIntCorr_posCorr_vs_posY->SetPointError(np, ybinWidth/2., resolGausErr*1000.);
    
    // -- tAve diff
    fitFunAveDiff_pulseIntCorr_binY[ibin]= new TF1(Form("fitFunAveDiff_pulseIntCorr_binY%d",ibin),"gaus", -20, 20);
    GetTimeResolution(h_tAveDiff_pulseIntCorr_binY[ibin], fitFunAveDiff_pulseIntCorr_binY[ibin], resolEff, resolGaus, resolGausErr, -1);
    g_tResolGausAveDiff_pulseIntCorr_vs_posY->SetPoint(np, yy, resolGaus*1000.);
    g_tResolGausAveDiff_pulseIntCorr_vs_posY->SetPointError(np, ybinWidth/2., resolGausErr*1000.);
        
    fitFunAveDiff_pulseIntCorr_posCorr_binY[ibin]= new TF1(Form("fitFunAveDiff_pulseIntCorr_posCorr_binY%d",ibin),"gaus", -20, 20);
    GetTimeResolution(h_tAveDiff_pulseIntCorr_posCorr_binY[ibin], fitFunAveDiff_pulseIntCorr_posCorr_binY[ibin], resolEff, resolGaus, resolGausErr, -1);
    g_tResolGausAveDiff_pulseIntCorr_posCorr_vs_posY->SetPoint(np, yy, resolGaus*1000.);
    g_tResolGausAveDiff_pulseIntCorr_posCorr_vs_posY->SetPointError(np, ybinWidth/2., resolGausErr*1000.);
    
    // -- tDiff diff
    fitFunDiffDiff_pulseIntCorr_binY[ibin]= new TF1(Form("fitFunDiffDiff_pulseIntCorr_binY%d",ibin),"gaus", -20, 20);
    GetTimeResolution(h_tDiffDiff_pulseIntCorr_binY[ibin], fitFunDiffDiff_pulseIntCorr_binY[ibin], resolEff, resolGaus, resolGausErr, -1);
    g_tResolGausDiffDiff_pulseIntCorr_vs_posY->SetPoint(np, yy, resolGaus*1000.);
    g_tResolGausDiffDiff_pulseIntCorr_vs_posY->SetPointError(np, ybinWidth/2., resolGausErr*1000.);
        
    fitFunDiffDiff_pulseIntCorr_posCorr_binY[ibin]= new TF1(Form("fitFunDiffDiff_pulseIntCorr_posCorr_binY%d",ibin),"gaus", -20, 20);
    GetTimeResolution(h_tDiffDiff_pulseIntCorr_posCorr_binY[ibin], fitFunDiffDiff_pulseIntCorr_posCorr_binY[ibin], resolEff, resolGaus, resolGausErr, -1);
    g_tResolGausDiffDiff_pulseIntCorr_posCorr_vs_posY->SetPoint(np, yy, resolGaus*1000.);
    g_tResolGausDiffDiff_pulseIntCorr_posCorr_vs_posY->SetPointError(np, ybinWidth/2., resolGausErr*1000.);

    np++;
    
    
    // fill graph of number of events vs Y
    float nn[NBARS];
    for (int iBar = 0; iBar< NBARS; iBar++){
      nn[iBar] = h_tAve_ampCorr_posCorr_binY[iBar][ibin]->GetEntries();
    }

    float r = 0;
    float er = 0;
    for (int iBar = 0; iBar < NBARS-1; iBar++){
      if ( nn[iBar] == 0 && nn[iBar+1] == 0 ) continue;
      else if ( nn[iBar] > 0 && nn[iBar+1] > 0) {
	float nU = h_tAveSum_ampCorr_posCorr_binY[ibin]->GetEntries();
	float nI = nn[iBar] + nn[iBar+1]-nU;
	r = nI/nU;
	er = 0;
	g_nEventsRatio_vs_posY -> SetPoint(np1, yy, r);
        g_nEventsRatio_vs_posY -> SetPointError(np1, ybinWidth/2., er);
        np1++;                                                      
      }


      /*else if ( nn[iBar] > 0 && nn[iBar+1] > 0) { // overlap 0/1
	r  = nn[iBar+1]/nn[iBar]; 
	if (nn[iBar+1] > nn[iBar] ) r  = nn[iBar]/nn[iBar+1];
	er = r * sqrt(1/nn[iBar+1] + 1/nn[iBar]);
	g_nEventsRatio_vs_posY -> SetPoint(np1, yy, r);
	g_nEventsRatio_vs_posY -> SetPointError(np1, ybinWidth/2., er);
	np1++;
      }
       */
    }
    
  }


  // -- Save histograms on file
  std::cout << "Saving histograms in file " << OutputFile.c_str() <<std::endl;
  
  TFile *outfile = new TFile(OutputFile.c_str(),"recreate");
  
  h_beamXY -> Write();
  h_beamX  -> Write();
  h_beamY  -> Write();
 
  
  p2_ampRef_vs_run_vs_event->Write();
  h2_ampRef_vs_event->Write();
  h_ampRef_nocuts->Write();
  h_ampRef->Write();
  h_timeRef->Write();
  h_chi2Ref->Write();
  h_sigmaRef->Write();
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

  h2_ampRatio-> Write();

  h_ampSum_nocuts->Write();
  h_pulseIntSum_nocuts->Write();

  for (int iBar = 0; iBar < NBARS; iBar++){
    
    p2_eff_vs_posXY[iBar]->Write();
    p_eff_vs_posX[iBar]->Write();
    p_eff_vs_posY[iBar]->Write();
       
    h_ampL_nocuts[iBar]->Write();
    h_ampL[iBar]->Write();
    h_pulseIntL[iBar]->Write();
    h2_pulseIntL_vs_amp[iBar]->Write();
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
    h_pulseIntR[iBar]->Write();
    h2_pulseIntR_vs_amp[iBar]->Write();
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
    p_tL_vs_pulseInt[iBar]->Write();
    for (int ibin =0; ibin < NBINSXAMPWALK[iBar]; ibin++) {
      p_tL_vs_amp_binned[iBar][ibin]->Write();
      p_tL_ampCorr_vs_amp_binned[iBar][ibin]->Write();
      p_tL_vs_pulseInt_binned[iBar][ibin]->Write();
    }
    p_tL_vs_posX[iBar]->Write();
    p_tL_vs_posY[iBar]->Write();
    p_tL_vs_tDiff[iBar]->Write();
    p2_tL_vs_amp_vs_posXc[iBar] -> Write();

    h_tL_ampCorr[iBar]->Write();
    p_tL_ampCorr_vs_amp[iBar]->Write();
    p_tL_ampCorr_vs_posX[iBar]->Write();
    p_tL_ampCorr_vs_posXc[iBar]->Write();
    p_tL_ampCorr_vs_posY[iBar]->Write();
    p_tL_ampCorr_vs_tDiff[iBar]->Write();

    h_tL_pulseIntCorr[iBar]->Write();
    p_tL_pulseIntCorr_vs_pulseInt[iBar]->Write();
    p_tL_pulseIntCorr_vs_posX[iBar]->Write();
    p_tL_pulseIntCorr_vs_posXc[iBar]->Write();
    p_tL_pulseIntCorr_vs_posY[iBar]->Write();
    p_tL_pulseIntCorr_vs_tDiff[iBar]->Write();

    // -- right
    p2_tR_vs_posXY[iBar]->Write();

    h_tR[iBar]->Write();
    h2_tR_vs_amp[iBar]->Write();
    p_tR_vs_amp[iBar]->Write();
    p_tR_vs_pulseInt[iBar]->Write();
    for (int ibin =0; ibin < NBINSXAMPWALK[iBar]; ibin++) {
      p_tR_vs_amp_binned[iBar][ibin]->Write();
      p_tR_ampCorr_vs_amp_binned[iBar][ibin]->Write();
      p_tR_vs_pulseInt_binned[iBar][ibin]->Write();
    }
    p_tR_vs_posX[iBar]->Write();
    p_tR_vs_posY[iBar]->Write();
    p_tR_vs_tDiff[iBar]->Write();
    p2_tR_vs_amp_vs_posXc[iBar] -> Write();

    h_tR_ampCorr[iBar]->Write();
    p_tR_ampCorr_vs_amp[iBar]->Write();
    p_tR_ampCorr_vs_posX[iBar]->Write();
    p_tR_ampCorr_vs_posXc[iBar]->Write();
    p_tR_ampCorr_vs_posY[iBar]->Write();
    p_tR_ampCorr_vs_tDiff[iBar]->Write();

    h_tR_pulseIntCorr[iBar]->Write();
    p_tR_pulseIntCorr_vs_pulseInt[iBar]->Write();
    p_tR_pulseIntCorr_vs_posX[iBar]->Write();
    p_tR_pulseIntCorr_vs_posXc[iBar]->Write();
    p_tR_pulseIntCorr_vs_posY[iBar]->Write();
    p_tR_pulseIntCorr_vs_tDiff[iBar]->Write();

    // -- tDiff
    h_tDiff[iBar]->Write();
    h_tDiff_posCorr[iBar]->Write();
    h_tDiff_ampCorr[iBar]->Write();
    h_tDiff_ampCorr_posCorr[iBar]->Write();
    p_tDiff_vs_posX[iBar]->Write();
    p_tDiff_vs_posXc[iBar]->Write();
    p_tDiff_posCorr_vs_posX[iBar]->Write();
    p_tDiff_posCorr_vs_posXc[iBar]->Write();
    p_tDiff_ampCorr_vs_posX[iBar]->Write();
    p_tDiff_ampCorr_vs_posXc[iBar]->Write();
    p_tDiff_ampCorr_posCorr_vs_posX[iBar]->Write();
    p_tDiff_ampCorr_posCorr_vs_posXc[iBar]->Write();
    p_tDiff_ampCorr_posCorr_vs_posY[iBar]->Write();

    h2_tDiff_vs_posX[iBar]->Write();
    h2_tDiff_ampCorr_vs_posX[iBar]->Write();

    h_tDiff_pulseIntCorr[iBar]->Write();
    h_tDiff_pulseIntCorr_posCorr[iBar]->Write();
    p_tDiff_pulseIntCorr_vs_posX[iBar]->Write();
    p_tDiff_pulseIntCorr_vs_posXc[iBar]->Write();
    p_tDiff_pulseIntCorr_posCorr_vs_posX[iBar]->Write();
    p_tDiff_pulseIntCorr_posCorr_vs_posXc[iBar]->Write();



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

    h_tAve_ampCorr_posCorr[iBar]->Write();
    p_tAve_ampCorr_posCorr_vs_posX[iBar]->Write();
    p_tAve_ampCorr_posCorr_vs_posXc[iBar]->Write();
    p_tAve_ampCorr_posCorr_vs_posY[iBar]->Write();
    p_tAve_ampCorr_posCorr_vs_tDiff[iBar]->Write();

    h_tAve_pulseIntCorr[iBar]->Write();
    p_tAve_pulseIntCorr_vs_posX[iBar]->Write();
    p_tAve_pulseIntCorr_vs_posXc[iBar]->Write();
    p_tAve_pulseIntCorr_vs_posY[iBar]->Write();
    p_tAve_pulseIntCorr_vs_tDiff[iBar]->Write();

    h_tAve_pulseIntCorr_posCorr[iBar]->Write();
    p_tAve_pulseIntCorr_posCorr_vs_posX[iBar]->Write();
    p_tAve_pulseIntCorr_posCorr_vs_posXc[iBar]->Write();
    p_tAve_pulseIntCorr_posCorr_vs_posY[iBar]->Write();
    p_tAve_pulseIntCorr_posCorr_vs_tDiff[iBar]->Write();

    // eff sigmas    
    h_effectiveSigmaL[iBar]->Write();
    h_effectiveSigmaL_ampCorr[iBar]->Write();

    h_effectiveSigmaR[iBar]->Write();
    h_effectiveSigmaR_ampCorr[iBar]->Write();

    h_effectiveSigmaAve[iBar]->Write();
    h_effectiveSigmaAve_ampCorr[iBar]->Write();
    h_effectiveSigmaAve_ampCorr_posCorr[iBar]->Write();
    h_effectiveSigmaDiff_ampCorr[iBar]->Write();
    h_effectiveSigmaDiff_ampCorr_posCorr[iBar]->Write();

    h_effectiveSigmaL_pulseIntCorr[iBar]->Write();
    h_effectiveSigmaR_pulseIntCorr[iBar]->Write();
    h_effectiveSigmaAve_pulseIntCorr[iBar]->Write();
    h_effectiveSigmaAve_pulseIntCorr_posCorr[iBar]->Write();
    h_effectiveSigmaDiff_pulseIntCorr[iBar]->Write();
    h_effectiveSigmaDiff_pulseIntCorr_posCorr[iBar]->Write();

    // graphs 
    g_tResolGausL_ampCorr[iBar]->Write();
    g_tResolGausR_ampCorr[iBar]->Write();

    g_tResolGausAve_ampCorr[iBar]->Write();
    g_tResolGausAve_ampCorr_posCorr[iBar]->Write();
    g_tResolGausAve_ampCorr_vs_posY[iBar]->Write();
    g_tResolGausAve_ampCorr_posCorr_vs_posY[iBar]->Write();

    g_tResolGausDiff_ampCorr[iBar]->Write();
    g_tResolGausDiff_ampCorr_posCorr[iBar]->Write();
    g_tResolGausDiff_ampCorr_vs_posY[iBar]->Write();
    g_tResolGausDiff_ampCorr_posCorr_vs_posY[iBar]->Write();

    g_tResolGausAve_pulseIntCorr[iBar]->Write();
    g_tResolGausAve_pulseIntCorr_posCorr[iBar]->Write();
    g_tResolGausAve_pulseIntCorr_vs_posY[iBar]->Write();
    g_tResolGausAve_pulseIntCorr_posCorr_vs_posY[iBar]->Write();

    g_tResolGausDiff_pulseIntCorr[iBar]->Write();
    g_tResolGausDiff_pulseIntCorr_posCorr[iBar]->Write();
    g_tResolGausDiff_pulseIntCorr_vs_posY[iBar]->Write();
    g_tResolGausDiff_pulseIntCorr_posCorr_vs_posY[iBar]->Write();
    g_tResolGausDiff_pulseIntCorr_posCorr_vs_amp[iBar]->Write();

  }
  
  h2_tAve1_vs_tAve2->Write();
  h2_amp1_vs_amp2->Write();

  h_tAveSum_ampCorr->Write();
  h_tAveSum_ampCorr_posCorr->Write();
  p_tAveSum_ampCorr_vs_posX ->Write();
  p_tAveSum_ampCorr_posCorr_vs_posX ->Write();
  p_tAveSum_ampCorr_vs_posY ->Write();
  p_tAveSum_ampCorr_posCorr_vs_posY ->Write();
  
  h_tAveAmpWSum_ampCorr->Write();
  h_tAveAmpWSum_ampCorr_posCorr->Write();
  p_tAveAmpWSum_ampCorr_vs_posX ->Write();
  p_tAveAmpWSum_ampCorr_posCorr_vs_posX ->Write();
  p_tAveAmpWSum_ampCorr_vs_posY ->Write();
  p_tAveAmpWSum_ampCorr_posCorr_vs_posY ->Write();

  h_tAveResWSum_ampCorr->Write();
  h_tAveResWSum_ampCorr_posCorr->Write();
  p_tAveResWSum_ampCorr_vs_posX ->Write();
  p_tAveResWSum_ampCorr_posCorr_vs_posX ->Write();
  p_tAveResWSum_ampCorr_vs_posY ->Write();
  p_tAveResWSum_ampCorr_posCorr_vs_posY ->Write();

  h_tAveDiff_ampCorr->Write();
  h_tAveDiff_ampCorr_posCorr->Write();
  p_tAveDiff_ampCorr_vs_posX ->Write();
  p_tAveDiff_ampCorr_posCorr_vs_posX ->Write();
  p_tAveDiff_ampCorr_vs_posY ->Write();
  p_tAveDiff_ampCorr_posCorr_vs_posY ->Write();

  h_tDiffDiff_ampCorr->Write();
  h_tDiffDiff_ampCorr_posCorr->Write();
  p_tDiffDiff_ampCorr_vs_posX ->Write();
  p_tDiffDiff_ampCorr_posCorr_vs_posX ->Write();
  p_tDiffDiff_ampCorr_vs_posY ->Write();
  p_tDiffDiff_ampCorr_posCorr_vs_posY ->Write();

  h_tAveSum_pulseIntCorr->Write();
  h_tAveSum_pulseIntCorr_posCorr->Write();
  p_tAveSum_pulseIntCorr_vs_posX ->Write();
  p_tAveSum_pulseIntCorr_posCorr_vs_posX ->Write();
  p_tAveSum_pulseIntCorr_vs_posY ->Write();
  p_tAveSum_pulseIntCorr_posCorr_vs_posY ->Write();
  
  h_tAveAmpWSum_pulseIntCorr->Write();
  h_tAveAmpWSum_pulseIntCorr_posCorr->Write();
  p_tAveAmpWSum_pulseIntCorr_vs_posX ->Write();
  p_tAveAmpWSum_pulseIntCorr_posCorr_vs_posX ->Write();
  p_tAveAmpWSum_pulseIntCorr_vs_posY ->Write();
  p_tAveAmpWSum_pulseIntCorr_posCorr_vs_posY ->Write();

  h_tAveResWSum_pulseIntCorr->Write();
  h_tAveResWSum_pulseIntCorr_posCorr->Write();
  p_tAveResWSum_pulseIntCorr_vs_posX ->Write();
  p_tAveResWSum_pulseIntCorr_posCorr_vs_posX ->Write();
  p_tAveResWSum_pulseIntCorr_vs_posY ->Write();
  p_tAveResWSum_pulseIntCorr_posCorr_vs_posY ->Write();

  h_tAveDiff_pulseIntCorr->Write();
  h_tAveDiff_pulseIntCorr_posCorr->Write();
  p_tAveDiff_pulseIntCorr_vs_posX ->Write();
  p_tAveDiff_pulseIntCorr_posCorr_vs_posX ->Write();
  p_tAveDiff_pulseIntCorr_vs_posY ->Write();
  p_tAveDiff_pulseIntCorr_posCorr_vs_posY ->Write();

  h_tDiffDiff_pulseIntCorr->Write();
  h_tDiffDiff_pulseIntCorr_posCorr->Write();
  p_tDiffDiff_pulseIntCorr_vs_posX ->Write();
  p_tDiffDiff_pulseIntCorr_posCorr_vs_posX ->Write();
  p_tDiffDiff_pulseIntCorr_vs_posY ->Write();
  p_tDiffDiff_pulseIntCorr_posCorr_vs_posY ->Write();

  g_tResolGausAveSum_ampCorr_vs_posX->Write();
  g_tResolGausAveSum_ampCorr_posCorr_vs_posX->Write();
  g_tResolGausAveSum_ampCorr_vs_posY->Write();
  g_tResolGausAveSum_ampCorr_posCorr_vs_posY->Write();
  
  g_tResolGausAmpWSum_ampCorr_vs_posX->Write();
  g_tResolGausAmpWSum_ampCorr_posCorr_vs_posX->Write();
  g_tResolGausAmpWSum_ampCorr_vs_posY->Write();
  g_tResolGausAmpWSum_ampCorr_posCorr_vs_posY->Write();

  g_tResolGausResWSum_ampCorr_vs_posX->Write();
  g_tResolGausResWSum_ampCorr_posCorr_vs_posX->Write();
  g_tResolGausResWSum_ampCorr_vs_posY->Write();
  g_tResolGausResWSum_ampCorr_posCorr_vs_posY->Write();

  g_tResolGausAveDiff_ampCorr_vs_posX->Write();
  g_tResolGausAveDiff_ampCorr_posCorr_vs_posX->Write();
  g_tResolGausAveDiff_ampCorr_vs_posY->Write();
  g_tResolGausAveDiff_ampCorr_posCorr_vs_posY->Write();

  g_tResolGausDiffDiff_ampCorr_vs_posX->Write();
  g_tResolGausDiffDiff_ampCorr_posCorr_vs_posX->Write();
  g_tResolGausDiffDiff_ampCorr_vs_posY->Write();
  g_tResolGausDiffDiff_ampCorr_posCorr_vs_posY->Write();

  g_tResolGausAveSum_pulseIntCorr_vs_posX->Write();
  g_tResolGausAveSum_pulseIntCorr_posCorr_vs_posX->Write();
  g_tResolGausAveSum_pulseIntCorr_vs_posY->Write();
  g_tResolGausAveSum_pulseIntCorr_posCorr_vs_posY->Write();
  
  g_tResolGausAmpWSum_pulseIntCorr_vs_posX->Write();
  g_tResolGausAmpWSum_pulseIntCorr_posCorr_vs_posX->Write();
  g_tResolGausAmpWSum_pulseIntCorr_vs_posY->Write();
  g_tResolGausAmpWSum_pulseIntCorr_posCorr_vs_posY->Write();

  g_tResolGausResWSum_pulseIntCorr_vs_posX->Write();
  g_tResolGausResWSum_pulseIntCorr_posCorr_vs_posX->Write();
  g_tResolGausResWSum_pulseIntCorr_vs_posY->Write();
  g_tResolGausResWSum_pulseIntCorr_posCorr_vs_posY->Write();

  g_tResolGausAveDiff_pulseIntCorr_vs_posX->Write();
  g_tResolGausAveDiff_pulseIntCorr_posCorr_vs_posX->Write();
  g_tResolGausAveDiff_pulseIntCorr_vs_posY->Write();
  g_tResolGausAveDiff_pulseIntCorr_posCorr_vs_posY->Write();

  g_tResolGausDiffDiff_pulseIntCorr_vs_posX->Write();
  g_tResolGausDiffDiff_pulseIntCorr_posCorr_vs_posX->Write();
  g_tResolGausDiffDiff_pulseIntCorr_vs_posY->Write();
  g_tResolGausDiffDiff_pulseIntCorr_posCorr_vs_posY->Write();

  g_nEventsRatio_vs_posY -> Write();

  cout << "Saving plots in subdir..."<<endl;

  TDirectory *dir1 = outfile->mkdir("xBins/");  
  TDirectory *dir2 = outfile->mkdir("yBins/");  
  TDirectory *dir3 = outfile->mkdir("ampBins/");  
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
    h_tAveSum_pulseIntCorr_binY[ibin] -> Write();
    h_tAveAmpWSum_pulseIntCorr_binY[ibin] -> Write();
    h_tDiffDiff_pulseIntCorr_posCorr_binY[ibin] -> Write();
    h_tAveDiff_pulseIntCorr_binY[ibin] -> Write();
    
    for (int iBar = 0; iBar < NBARS; iBar++){
      h_tAve_pulseIntCorr_binY[iBar][ibin]->Write();
      h_tDiff_pulseIntCorr_posCorr_binY[iBar][ibin]->Write();
    }
  }

  dir3->cd();
  for (int iBar = 0; iBar < NBARS; iBar++){
    for (int abin = 0 ; abin < 20; abin++){
      h_tDiff_pulseIntCorr_posCorr_binAmp[iBar][abin]->Write();
    }
  }
	      
  



  cout << "Closing file..."<<endl;

  outfile->Close();

  delete outfile;
  return 0;

}



// ---- Initialize tree -------------------------------------------------------------------------------------
void InitTreeVars(TTree* chain1,TreeVars& treeVars, float threshold){
  treeVars.t_run = 0;
  treeVars.t_event = 0;
  treeVars.t_amp = new float[1000];
  treeVars.t_charge_sig = new float[1000];
  treeVars.t_time = new float[1000];
  treeVars.t_time_max = new float[1000];
  treeVars.t_chi2_max = new float[1000];
  treeVars.t_gaus_sigma = new float[1000];
  treeVars.t_b_rms = new float[1000];
  treeVars.t_CFD = 0;
  treeVars.t_LED = 0;

  treeVars.t_channelId = new std::map<std::string,int>;

  treeVars.t_trackFitResult = new std::vector<TrackPar>;
  treeVars.t_ntracks = 0;

  //chain1 -> SetBranchStatus("*",0);

  chain1 -> SetBranchStatus("n_tracks",      1); chain1->SetBranchAddress("n_tracks",  &treeVars.t_ntracks);
  chain1 -> SetBranchStatus("fitResult",     1); chain1->SetBranchAddress("fitResult", &treeVars.t_trackFitResult);

  chain1 -> SetBranchStatus("run",           1); chain1 -> SetBranchAddress("run",        &treeVars.t_run);
  chain1 -> SetBranchStatus("event",         1); chain1 -> SetBranchAddress("event",      &treeVars.t_event);
  chain1 -> SetBranchStatus("amp_max",       1); chain1 -> SetBranchAddress("amp_max",     treeVars.t_amp);
  chain1 -> SetBranchStatus("charge_sig",    1); chain1 -> SetBranchAddress("charge_sig",  treeVars.t_charge_sig);
  chain1 -> SetBranchStatus("time",          1); chain1 -> SetBranchAddress("time",        treeVars.t_time);
  chain1 -> SetBranchStatus("time_max",      1); chain1 -> SetBranchAddress("time_max",    treeVars.t_time_max);
  chain1 -> SetBranchStatus("chi2_max",      1); chain1 -> SetBranchAddress("chi2_max",    treeVars.t_chi2_max);
  chain1 -> SetBranchStatus("gaus_sigma",      1); chain1 -> SetBranchAddress("gaus_sigma",    treeVars.t_gaus_sigma);
  chain1 -> SetBranchStatus("b_rms",         1); chain1 -> SetBranchAddress("b_rms",       treeVars.t_b_rms);
  chain1 -> SetBranchStatus("CFD",           1); chain1 -> SetBranchAddress("CFD",         &treeVars.t_CFD);
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

void GetTimeResolution( TH1F *ht, TF1* fitFun, float &resolEff, float &resolGaus, float &resolGausErr, float resolRef)
{
  int nMinEntries = 50;

  float sGaus = 0;
  float sEff  = 0;
  float sGausErr = 0;
  
  float tmin = 0;
  float tmax = 0;
  float* vals = new float[4];

  FindSmallestInterval(vals,ht,0.68,true);
  sEff = 0.5*(vals[3]-vals[2]);

  fitFun->SetLineColor(2);
  fitFun->SetLineWidth(1);
  fitFun->SetParameter(1,ht->GetMean());
  fitFun->SetParameter(2,ht->GetRMS());
  tmin = ht->GetMean()-3.0*ht->GetRMS();
  tmax = ht->GetMean()+3.0*ht->GetRMS();
  ht->Fit( fitFun, "QRS", "", tmin, tmax);
  if (fitFun->GetChisquare()/fitFun->GetNDF()>10){
    tmin = ht->GetMean()-2.5*ht->GetRMS();
    tmax = ht->GetMean()+2.5*ht->GetRMS();
    ht->Fit( fitFun, "QRS", "", tmin, tmax);
  }
  sGaus = fitFun->GetParameter(2);
  sGausErr = fitFun->GetParError(2);
  
  if ( ht->GetEntries() < nMinEntries || sGaus > ht->GetRMS()){ // troppo poche entries per un fit decente oppure fit balordo
    sGaus    = ht->GetRMS();
    sGausErr = ht->GetRMSError();
  }

  if (resolRef>-1){
    resolEff  = sqrt(sEff  * sEff  - resolRef * resolRef);
    resolGaus = sqrt(sGaus * sGaus - resolRef * resolRef);
  }   
  else {
    resolEff  = sEff;
    resolGaus = sGaus;
  }
  
  resolGausErr = sGausErr/resolGaus * sGaus; // assuming no uncertainty on resolRef
    
}



// ==========

int GetPointFromX(TGraphErrors *g, double x){
  
  int index = -1;
  double d = 9999999;
  for (int i = 0; i < g->GetN(); i++) {
    if (fabs(g->GetX()[i]-x)<d ){
      index = i;
      d = fabs(g->GetX()[i]-x);
    }
  }
  
  return index;
}




// ========
float GetCorrectedMCPTime(float time, float amp, float x){


  //-- from Sasha
  
  // - corr vs amplitude of MCP
  float dt_A = 8.758 - 0.06954 * amp;
  dt_A*=0.001; // ns

  // - corr vs position x
  float p0 = 1.58298E+01;
  float p1 = -5.90089;
  float p2 = -4.05435E-05;
  float p3 = -4.31364E-08;
  float dt_posX = p2 * pow(x - p0, 2) + p3 * pow(x - p0, 4);

  float time_corr = time + dt_A + dt_posX ;
    
  //  cout << time << "  " << time + dt_A << "  " << time_corr <<endl;
  
  return time_corr;    
}
