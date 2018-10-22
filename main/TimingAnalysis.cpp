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
  std::map<std::string,TH1F*>  h_rt;
  std::map<std::string,TH1F*>  h_posX;
  std::map<std::string,TH1F*>  h_posY;
  std::map<std::string,TH2F*>  h2_posXY;

  std::map<std::string,TProfile2D*>    p2_eff_vs_posXY;
  std::map<std::string,TProfile2D*>    p2_dt_vs_posXY;
  std::map<std::string,TProfile2D*>    p2_amp_vs_posXY;
  std::map<std::string,TProfile*>      p_eff_vs_posX;
  std::map<std::string,TProfile*>      p_eff_vs_posY;
  std::map<std::string,TProfile*>      p_amp_vs_posX;
  std::map<std::string,TProfile*>      p_amp_vs_posY;

  std::map<std::string,TH1F*>  h_dt;
  std::map<std::string,TH1F*>  h_dt_central;
  std::map<std::string,TH1F*>  h_dt_border;

  std::map<std::string,TH1F*>  h_dt_ampCorr;
  std::map<std::string,TH1F*>  h_dt_ampCorr_central;
  std::map<std::string,TH1F*>  h_dt_ampCorr_border;
  std::map<std::string,TH1F*>  h_dt_ampCorr_posCorr;
  std::map<std::string,TH1F*>  h_dt_rtCorr;
  std::map<std::string,TH1F*>  h_dt_rtCorr_posCorr;

  std::map<std::string,TProfile*>      p_dt_vs_amp;
  std::map<std::string,TProfile*>      p_dt_vs_rt;
  std::map<std::string,TProfile*>      p_dt_vs_posX;
  std::map<std::string,TProfile*>      p_dt_vs_posY;

  std::map<std::string,TProfile*>      p_dt_ampCorr_vs_amp;
  std::map<std::string,TProfile*>      p_dt_ampCorr_vs_posX;
  std::map<std::string,TProfile*>      p_dt_ampCorr_vs_posY;
  std::map<std::string,TProfile*>      p_dt_rtCorr_vs_amp;
  std::map<std::string,TProfile*>      p_dt_rtCorr_vs_posX;
  std::map<std::string,TProfile*>      p_dt_rtCorr_vs_posY;

  std::map<std::string,TProfile*>      p_dt_ampCorr_posCorr_vs_amp;
  std::map<std::string,TProfile*>      p_dt_ampCorr_posCorr_vs_posX;
  std::map<std::string,TProfile*>      p_dt_ampCorr_posCorr_vs_posY;
  std::map<std::string,TProfile*>      p_dt_rtCorr_posCorr_vs_amp;
  std::map<std::string,TProfile*>      p_dt_rtCorr_posCorr_vs_posX;
  std::map<std::string,TProfile*>      p_dt_rtCorr_posCorr_vs_posY;


  std::string channelName;
  std::string channelNameA;

  float dtmin = 5;
  float dtmax = 15;
  //  if (ninoThr == 20){
  //  dtmin = -20;
  //  dtmax = 20;
  //}

  std::cout << treeVars.t_digiConf <<std::endl;
  // if (treeVars.t_digiConf == 2){
  if (runs[0]>=12510 && runs[0]<=12538)  {
    dtmin = 0;
    dtmax = 10;
  }

  for (int i = 0 ; i < NCHANNELS; i++){

    channelName = timeChannels.at(i); // es: ch = MCP1, NINOMAT1, etc...

    h_amp_nocuts[channelName]  = new TH1F(Form("h_amp_nocuts_%s", channelName.c_str()), Form("h_amp_nocuts_%s",channelName.c_str()), 1000, 0., 1.0);
    h_amp[channelName]  = new TH1F(Form("h_amp_%s", channelName.c_str()), Form("h_amp_%s",channelName.c_str()), 1000, 0., 1.0);
    h_time[channelName] = new TH1F(Form("h_time_%s",channelName.c_str()),Form("h_time_%s",channelName.c_str()),200,0.,200.);
    h_rt[channelName] = new TH1F(Form("h_rt_%s",channelName.c_str()),Form("h_rt_%s",channelName.c_str()),100,0.,50.);
    h_posX[channelName] = new TH1F(Form("h_posX_%s",channelName.c_str()),Form("h_posX_%s",channelName.c_str()), 80,-20.,20.);
    h_posY[channelName] = new TH1F(Form("h_posY_%s",channelName.c_str()),Form("h_posY_%s",channelName.c_str()),80,-20.,20.);
    h2_posXY[channelName] = new TH2F(Form("h2_posXY_%s",channelName.c_str()),Form("h2_posXY_%s",channelName.c_str()),80,-20.,20.,80, -20.,20.);
    p2_eff_vs_posXY[channelName] = new TProfile2D(Form("p2_eff_vs_posXY_%s",channelName.c_str()),Form("p2_eff_vs_posXY_%s",channelName.c_str()), 80, -20., 20., 80, -20., 20., 0, 1);
    p2_dt_vs_posXY[channelName] = new TProfile2D(Form("p2_dt_vs_posXY_%s",channelName.c_str()),Form("p2_dt_vs_posXY_%s",channelName.c_str()), 80, -20., 20., 80, -20., 20., dtmin, dtmax);
    p2_amp_vs_posXY[channelName] = new TProfile2D(Form("p2_amp_vs_posXY_%s",channelName.c_str()),Form("p2_amp_vs_posXY_%s",channelName.c_str()), 80, -20., 20., 80, -20., 20., 0, 1);
    p_eff_vs_posX[channelName] = new TProfile(Form("p_eff_vs_posX_%s",channelName.c_str()),Form("p_eff_vs_posX_%s",channelName.c_str()), 80, -20., 20., 0, 1);
    p_eff_vs_posY[channelName] = new TProfile(Form("p_eff_vs_posY_%s",channelName.c_str()),Form("p_eff_vs_posY_%s",channelName.c_str()), 80, -20., 20., 0, 1);
    p_amp_vs_posX[channelName] = new TProfile(Form("p_amp_vs_posX_%s",channelName.c_str()),Form("p_amp_vs_posX_%s",channelName.c_str()), 80, -20., 20., 0, 1);
    p_amp_vs_posY[channelName] = new TProfile(Form("p_amp_vs_posY_%s",channelName.c_str()),Form("p_amp_vs_posY_%s",channelName.c_str()), 80, -20., 20., 0, 1);
    
    h_dt[channelName]   = new TH1F(Form("h_dt_%s",channelName.c_str()),Form("h_dt_%s",channelName.c_str()),8000, dtmin, dtmax);
    h_dt_central[channelName]   = new TH1F(Form("h_dt_central_%s",channelName.c_str()),Form("h_dt_central_%s",channelName.c_str()),8000, dtmin, dtmax);
    h_dt_border[channelName]   = new TH1F(Form("h_dt_border_%s",channelName.c_str()),Form("h_dt_border_%s",channelName.c_str()),8000, dtmin, dtmax);

    h_dt_ampCorr[channelName]   = new TH1F(Form("h_dt_ampCorr_%s",channelName.c_str()),Form("h_dt_ampCorr_%s",channelName.c_str()),8000, dtmin, dtmax);
    h_dt_ampCorr_central[channelName]   = new TH1F(Form("h_dt_ampCorr_central_%s",channelName.c_str()),Form("h_dt_ampCorr_central_%s",channelName.c_str()),8000, dtmin, dtmax);
    h_dt_ampCorr_border[channelName]   = new TH1F(Form("h_dt_ampCorr_border_%s",channelName.c_str()),Form("h_dt_ampCorr_border_%s",channelName.c_str()),8000, dtmin, dtmax);

    h_dt_ampCorr_posCorr[channelName]   = new TH1F(Form("h_dt_ampCorr_posCorr_%s",channelName.c_str()),Form("h_dt_ampCorr_posCorr_%s",channelName.c_str()),8000, dtmin, dtmax);
    h_dt_rtCorr[channelName]   = new TH1F(Form("h_dt_rtCorr_%s",channelName.c_str()),Form("h_dt_rtCorr_%s",channelName.c_str()),8000, dtmin, dtmax);
    h_dt_rtCorr_posCorr[channelName]   = new TH1F(Form("h_dt_rtCorr_posCorr_%s",channelName.c_str()),Form("h_dt_rtCorr_posCorr_%s",channelName.c_str()),8000, dtmin, dtmax);
    
    p_dt_vs_amp[channelName]  = new TProfile(Form("p_dt_vs_amp_%s",channelName.c_str()),Form("p_dt_vs_amp_%s",channelName.c_str()), 200, 0., 1., dtmin, dtmax);
    p_dt_vs_rt[channelName]   = new TProfile(Form("p_dt_vs_rt_%s",channelName.c_str()),Form("p_dt_vs_rt_%s",channelName.c_str()), 500, 0., 50., dtmin, dtmax);
    p_dt_vs_posX[channelName] = new TProfile(Form("p_dt_vs_posX_%s",channelName.c_str()),Form("p_dt_vs_posX_%s",channelName.c_str()), 80, -20., 20., dtmin, dtmax);
    p_dt_vs_posY[channelName] = new TProfile(Form("p_dt_vs_posY_%s",channelName.c_str()),Form("p_dt_vs_posY_%s",channelName.c_str()), 80, -20., 20., dtmin, dtmax);
    
    p_dt_ampCorr_vs_amp[channelName]  = new TProfile(Form("p_dt_ampCorr_vs_amp_%s",channelName.c_str()),Form("p_dt_ampCorr_vs_amp_%s",channelName.c_str()), 200, 0., 1., dtmin, dtmax);
    p_dt_ampCorr_vs_posX[channelName] = new TProfile(Form("p_dt_ampCorr_vs_posX_%s",channelName.c_str()),Form("p_dt_ampCorr_vs_posX_%s",channelName.c_str()), 80, -20., 20., dtmin, dtmax);
    p_dt_ampCorr_vs_posY[channelName] = new TProfile(Form("p_dt_ampCorr_vs_posY_%s",channelName.c_str()),Form("p_dt_ampCorr_vs_posY_%s",channelName.c_str()), 80, -20., 20., dtmin, dtmax);

    p_dt_ampCorr_posCorr_vs_amp[channelName]  = new TProfile(Form("p_dt_ampCorr_posCorr_vs_amp_%s",channelName.c_str()),Form("p_dt_ampCorr_posCorr_vs_amp_%s",channelName.c_str()), 200, 0., 1., dtmin, dtmax);
    p_dt_ampCorr_posCorr_vs_posX[channelName] = new TProfile(Form("p_dt_ampCorr_posCorr_vs_posX_%s",channelName.c_str()),Form("p_dt_ampCorr_posCorr_vs_posX_%s",channelName.c_str()), 80, -20., 20., dtmin, dtmax);
    p_dt_ampCorr_posCorr_vs_posY[channelName] = new TProfile(Form("p_dt_ampCorr_posCorr_vs_posY_%s",channelName.c_str()),Form("p_dt_ampCorr_posCorr_vs_posY_%s",channelName.c_str()), 80, -20., 20., dtmin, dtmax);
    
    p_dt_rtCorr_vs_amp[channelName]  = new TProfile(Form("p_dt_rtCorr_vs_amp_%s",channelName.c_str()),Form("p_dt_rtCorr_vs_amp_%s",channelName.c_str()), 200, 0., 1., dtmin, dtmax);
    p_dt_rtCorr_vs_posX[channelName] = new TProfile(Form("p_dt_rtCorr_vs_posX_%s",channelName.c_str()),Form("p_dt_rtCorr_vs_posX_%s",channelName.c_str()), 80, -20., 20., dtmin, dtmax);
    p_dt_rtCorr_vs_posY[channelName] = new TProfile(Form("p_dt_rtCorr_vs_posY_%s",channelName.c_str()),Form("p_dt_rtCorr_vs_posY_%s",channelName.c_str()), 80, -20., 20., dtmin, dtmax);

    p_dt_rtCorr_posCorr_vs_amp[channelName]  = new TProfile(Form("p_dt_rtCorr_posCorr_vs_amp_%s",channelName.c_str()),Form("p_dt_rtCorr_posCorr_vs_amp_%s",channelName.c_str()), 200, 0., 1., dtmin, dtmax);
    p_dt_rtCorr_posCorr_vs_posX[channelName] = new TProfile(Form("p_dt_rtCorr_posCorr_vs_posX_%s",channelName.c_str()),Form("p_dt_rtCorr_posCorr_vs_posX_%s",channelName.c_str()), 80, -20., 20., dtmin, dtmax);
    p_dt_rtCorr_posCorr_vs_posY[channelName] = new TProfile(Form("p_dt_rtCorr_posCorr_vs_posY_%s",channelName.c_str()),Form("p_dt_rtCorr_posCorr_vs_posY_%s",channelName.c_str()), 80, -20., 20., dtmin, dtmax);
    
    
  }



  float amp  = 0.;
  float time = 0.;
  float ampRef = 0;
  float timeRef = 0;
  float dt   = 0.;
  float dtcorr = 0.;
  float rt = 0;
  float timeTrg =0;
  float posX = 0;
  float posY = 0;
  
  float k = 1./4096.; // factor to convert amplitudes in V
  float trgOffset = 160; // just for plotting times in a range 0-200
  float tMaxCh  = 40;
  float tMaxRef = 35;
  float bRmsMax = 200;
  float xtalSize = 11.5; // mm

 
  float ampCorrFactor = 1.0;

  //-----------------------
  // first loop over events
  std::cout<<"First loop over events ..." <<std::endl;
  int nEntries = chain1 -> GetEntries();
  for(int entry = 0; entry < nEntries; ++entry)
    {
      if( entry%1000 == 0 ) std::cout << ">>> Reading entry " << entry << " / " << nEntries << "\r" << std::flush;
      chain1 -> GetEntry(entry);

      // -- trig time
      timeTrg = treeVars.t_time[(*treeVars.t_channelId)["TRG0"]+treeVars.t_LED] - trgOffset;
      
      // -- Photek as ref
      ampRef = treeVars.t_amp[(*treeVars.t_channelId)["PTK1"]] * k;
      timeRef   = treeVars.t_time[(*treeVars.t_channelId)["PTK1"]+treeVars.t_CFD] - timeTrg;
      
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
	if (channelNameA=="AMP1") amp = amp * ampCorrFactor;
        time = treeVars.t_time[(*treeVars.t_channelId)[channelName]+treeVars.t_LED];
	if (channelName=="PTK1") 
	  time = treeVars.t_time[(*treeVars.t_channelId)[channelName]+treeVars.t_CFD];

       	// -- fill histograms before any selection
	h_amp_nocuts[channelName] -> Fill(amp);
	
	if ( amp > minAmplitude[channelNameA] ) {
	  p2_eff_vs_posXY[channelName] ->Fill(posX, posY, 1.);
	  p_eff_vs_posX[channelName] ->Fill(posX, 1.);
	  p_eff_vs_posY[channelName] ->Fill(posY, 1.);
	}
	else{
	  p2_eff_vs_posXY[channelName] ->Fill(posX, posY, 0.);
	  p_eff_vs_posX[channelName] ->Fill(posX, 0.);
	  p_eff_vs_posY[channelName] ->Fill(posY, 0.);
	}

	// -- select events
	if ( ampRef < minAmplitude["PTK1"] || ampRef > maxAmplitude["PTK1"]) continue;
	if ( amp < minAmplitude[channelNameA]  || amp > maxAmplitude[channelNameA]) continue;
	if ( timeRef < 0 || timeRef > 200) continue;
	if ( time < 0 || time > 200) continue;
	if ( treeVars.t_b_rms[(*treeVars.t_channelId)["PTK1"]] > bRmsMax) continue; // cleaning reco failures... 

	// subtract TRG time for each digi group 
	time = time - timeTrg;
	dt   = time - timeRef; 
	rt   = treeVars.t_time[(*treeVars.t_channelId)[channelNameA]+treeVars.t_LED200] - treeVars.t_time[(*treeVars.t_channelId)[channelNameA]+treeVars.t_LED50] ; 
	//std::cout << channelNameA << "  " << treeVars.t_time[(*treeVars.t_channelId)[channelNameA]+treeVars.t_LED200] << "  " << treeVars.t_time[(*treeVars.t_channelId)[channelNameA]+treeVars.t_LED50] <<"  " <<rt <<std::endl;

	// -- fill histograms
	h_time[channelName] -> Fill(time);
	h2_posXY[channelName]->Fill(posX,posY);

	// -- remove events with bad reco time (second peak in the time distribution)
	if ( time    > tMaxCh  ) continue;
	if ( timeRef > tMaxRef ) continue;

	// -- keep events on the crystal
	if ( posX < cut_hodoXmin[ich] || posX > cut_hodoXmax[ich] ) continue;
	if ( posY < cut_hodoYmin[ich] || posY > cut_hodoYmax[ich] ) continue;

	h_amp[channelName]  -> Fill(amp);
	h_posX[channelName]->Fill(posX);
	h_posY[channelName]->Fill(posY);
	p2_amp_vs_posXY[channelName] ->Fill(posX,posY,amp);
	p_amp_vs_posX[channelName] ->Fill(posX,amp);
	p_amp_vs_posY[channelName] ->Fill(posY,amp);
	h_rt[channelName] ->Fill(rt);

	// -- fill time resol plots only for NINO channels
	if (! (channelName.find("NINO") != std::string::npos) ) continue;
		
	h_dt[channelName]->Fill(dt);
	p_dt_vs_amp[channelName]->Fill(amp,dt);
	p_dt_vs_rt[channelName]->Fill(rt,dt);
	p_dt_vs_posX[channelName]->Fill(posX,dt);
	p_dt_vs_posY[channelName]->Fill(posY,dt);
	p2_dt_vs_posXY[channelName] ->Fill(posX,posY,dt);

	float xcenter = cut_hodoXmin[ich] + (cut_hodoXmax[ich] - cut_hodoXmin[ich])/2;
	float ycenter = cut_hodoYmin[ich] + (cut_hodoYmax[ich] - cut_hodoYmin[ich])/2;
	if ( fabs(posX-xcenter) < 1.5 && fabs(posY-ycenter) < 1.5   ){
	  h_dt_central[channelName]->Fill(dt);
	}
	if ( fabs(posX-xcenter) > (xtalSize/2-3.) && fabs(posY-ycenter) > (xtalSize/2-3.) ){
	  h_dt_border[channelName]->Fill(dt);
	} 

      } // -- end loop over channels
      
    }// -- end first loop over events
  
  
  // --- Amplitude walk corrections
  std::map<std::string,TF1*>  fitFunc_ampCorr;  
  std::map<std::string,TF1*>  fitFunc_rtCorr;

  for (int i = 0 ; i < NCHANNELS; i++){
    channelName = timeChannels.at(i);

    fitFunc_ampCorr[channelName] = new TF1(Form("fitFunc_ampCorr_%s",channelName.c_str()),"pol6");
    fitFunc_ampCorr[channelName] -> SetLineColor(kRed);
    p_dt_vs_amp[channelName] -> Fit(Form("fitFunc_ampCorr_%s",channelName.c_str()),"QSR",0,1);

    fitFunc_rtCorr[channelName] = new TF1(Form("fitFunc_rtCorr_%s",channelName.c_str()),"pol6",0,200);
    fitFunc_rtCorr[channelName] -> SetLineColor(kBlue);
    fitFunc_rtCorr[channelName] -> SetParameter(0, p_dt_vs_rt[channelName] -> GetMean(2) );
    p_dt_vs_rt[channelName] -> Fit(Form("fitFunc_rtCorr_%s",channelName.c_str()),"QSR",0,50);

  }


  // --- second loop over events -- fill histograms after applying amplitude walk corrections
  std::cout<<"Second loop over events ..." <<std::endl;
  for(int entry = 0; entry < nEntries; ++entry)
    {
      if( entry%1000 == 0 ) std::cout << ">>> Reading entry " << entry << " / " << nEntries << "\r" << std::flush;
      chain1 -> GetEntry(entry);

      // -- trig time
      timeTrg = treeVars.t_time[(*treeVars.t_channelId)["TRG0"]+treeVars.t_LED] - trgOffset;

      // -- Photek as ref
      ampRef = treeVars.t_amp[(*treeVars.t_channelId)["PTK1"]] * k;
      timeRef   = treeVars.t_time[(*treeVars.t_channelId)["PTK1"]+treeVars.t_CFD] - timeTrg;

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
	if (channelNameA == "AMP1") amp = amp*ampCorrFactor;
        time = treeVars.t_time[(*treeVars.t_channelId)[channelName]+treeVars.t_LED];
	if (channelNameA=="PTK")
          time = treeVars.t_time[(*treeVars.t_channelId)[channelName]+treeVars.t_CFD];

        // -- select events
        if ( ampRef < minAmplitude["PTK1"] || ampRef > maxAmplitude["PTK1"]) continue;
        if ( amp < minAmplitude[channelNameA]  || amp > maxAmplitude[channelNameA]) continue;
	if ( timeRef < 0 || timeRef > 200) continue;
        if ( time < 0 || time > 200) continue;
        if ( treeVars.t_b_rms[(*treeVars.t_channelId)["PTK1"]] > bRmsMax) continue; // cleaning reco failures... 

	// subtract TRG time for each digi group
        time = time - timeTrg;
        dt   = time - timeRef;
	rt   = treeVars.t_time[(*treeVars.t_channelId)[channelNameA]+treeVars.t_LED200] - treeVars.t_time[(*treeVars.t_channelId)[channelNameA]+treeVars.t_LED50] ;

	// -- remove events with bad reco time
        if ( time    > tMaxCh  ) continue;
        if ( timeRef > tMaxRef ) continue;

	// -- keep events on the crystal
        if ( posX < cut_hodoXmin[ich] || posX > cut_hodoXmax[ich] ) continue;
        if ( posY < cut_hodoYmin[ich] || posY > cut_hodoYmax[ich] ) continue;

	// -- fill time resol plots only for NINO channels                
	if (! (channelName.find("NINO") != std::string::npos) ) continue;
        
	// -- amplitude walk correction from ampl
	dtcorr = dt - fitFunc_ampCorr[channelName]->Eval(amp) + fitFunc_ampCorr[channelName]->Eval(h_amp[channelName]->GetMean()) ; 

	h_dt_ampCorr[channelName]->Fill(dtcorr);
        p_dt_ampCorr_vs_amp[channelName]->Fill(amp,dtcorr);
        p_dt_ampCorr_vs_posX[channelName]->Fill(posX,dtcorr);
        p_dt_ampCorr_vs_posY[channelName]->Fill(posY,dtcorr);


	// -- beam spot selection
	float xcenter = cut_hodoXmin[ich] + (cut_hodoXmax[ich] - cut_hodoXmin[ich])/2;
	float ycenter = cut_hodoYmin[ich] + (cut_hodoYmax[ich] - cut_hodoYmin[ich])/2;
	if ( fabs(posX-xcenter) < 1.5 && fabs(posY-ycenter) < 1.5   ){
	  h_dt_ampCorr_central[channelName]->Fill(dtcorr);
	}
	if ( fabs(posX-xcenter) > (xtalSize/2-1.5) && fabs(posY-ycenter) > (xtalSize/2-1.5) ){
	  h_dt_ampCorr_border[channelName]->Fill(dtcorr);
	} 


	// -- amplitude walk correction from rise time
	dtcorr = dt - fitFunc_rtCorr[channelName]->Eval(rt) + fitFunc_rtCorr[channelName]->Eval(h_rt[channelName]->GetMean()) ; 
	h_dt_rtCorr[channelName]->Fill(dtcorr);
        p_dt_rtCorr_vs_amp[channelName]->Fill(amp,dtcorr);
        p_dt_rtCorr_vs_posX[channelName]->Fill(posX,dtcorr);
        p_dt_rtCorr_vs_posY[channelName]->Fill(posY,dtcorr);


      } // -- end loop over channels

    }// -- end second loop over events



  // --- third loop over events -- fill histograms after applying amplitude walk corrections
  std::cout<<"Third loop over events ..." <<std::endl;
  for(int entry = 0; entry < nEntries; ++entry)
    {
      if( entry%1000 == 0 ) std::cout << ">>> Reading entry " << entry << " / " << nEntries << "\r" << std::flush;
      chain1 -> GetEntry(entry);

      // -- trig time 
      timeTrg = treeVars.t_time[(*treeVars.t_channelId)["TRG0"]+treeVars.t_LED] - trgOffset;

      // -- Photek as ref
      ampRef = treeVars.t_amp[(*treeVars.t_channelId)["PTK1"]] * k;
      timeRef   = treeVars.t_time[(*treeVars.t_channelId)["PTK1"]+treeVars.t_CFD] - timeTrg;

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
	if (channelNameA == "AMP1") amp = amp*ampCorrFactor;
        time = treeVars.t_time[(*treeVars.t_channelId)[channelName]+treeVars.t_LED];
        if (channelName=="PTK1")
          time = treeVars.t_time[(*treeVars.t_channelId)[channelName]+treeVars.t_CFD];

        // -- select events
        if ( ampRef < minAmplitude["PTK1"] || ampRef > maxAmplitude["PTK1"]) continue;
        if ( amp < minAmplitude[channelNameA]  || amp > maxAmplitude[channelNameA]) continue;
	if ( timeRef < 0 || timeRef > 200) continue;
        if ( time < 0 || time > 200) continue;
        if ( treeVars.t_b_rms[(*treeVars.t_channelId)["PTK1"]] > bRmsMax) continue; // cleaning reco failures... 

	// subtract TRG time for each digi group
        time = time - timeTrg;
        dt   = time - timeRef;
	rt   = treeVars.t_time[(*treeVars.t_channelId)[channelNameA]+treeVars.t_LED200] - treeVars.t_time[(*treeVars.t_channelId)[channelNameA]+treeVars.t_LED50] ;

	// -- remove events with bad reco time
        if ( time    > tMaxCh  ) continue;
        if ( timeRef > tMaxRef ) continue;

	// -- keep events on the crystal
        if ( posX < cut_hodoXmin[ich] || posX > cut_hodoXmax[ich] ) continue;
        if ( posY < cut_hodoYmin[ich] || posY > cut_hodoYmax[ich] ) continue;

	// -- fill time resol plots only for NINO channels                
	if (! (channelName.find("NINO") != std::string::npos) ) continue;

        // -- position corrections on top of amp walk correction
	dtcorr = dt - fitFunc_ampCorr[channelName]->Eval(amp) + fitFunc_ampCorr[channelName]->Eval(h_amp[channelName]->GetMean()) ;
	// -- vs X
	int bin = p_dt_ampCorr_vs_posX[channelName]->FindBin(posX);
	dtcorr = dtcorr - p_dt_ampCorr_vs_posX[channelName]->GetBinContent(bin) + h_dt_ampCorr[channelName]->GetMean();
	// -- vs Y
	bin = p_dt_ampCorr_vs_posY[channelName]->FindBin(posY);
        dtcorr = dtcorr - p_dt_ampCorr_vs_posY[channelName]->GetBinContent(bin) + h_dt_ampCorr[channelName]->GetMean();

        h_dt_ampCorr_posCorr[channelName]->Fill(dtcorr);
        p_dt_ampCorr_posCorr_vs_amp[channelName]->Fill(amp,dtcorr);
        p_dt_ampCorr_posCorr_vs_posX[channelName]->Fill(posX,dtcorr);
        p_dt_ampCorr_posCorr_vs_posY[channelName]->Fill(posY,dtcorr);

        // -- position corrections on top of risetime correction
	dtcorr = dt - fitFunc_rtCorr[channelName]->Eval(rt) + fitFunc_rtCorr[channelName]->Eval(h_rt[channelName]->GetMean()) ;
	// -- vs X
	bin = p_dt_rtCorr_vs_posX[channelName]->FindBin(posX);
	dtcorr = dtcorr - p_dt_rtCorr_vs_posX[channelName]->GetBinContent(bin) + h_dt_rtCorr[channelName]->GetMean();
	// -- vs Y
	bin = p_dt_rtCorr_vs_posY[channelName]->FindBin(posY);
        dtcorr = dtcorr - p_dt_rtCorr_vs_posY[channelName]->GetBinContent(bin) + h_dt_rtCorr[channelName]->GetMean();

        h_dt_rtCorr_posCorr[channelName]->Fill(dtcorr);
        p_dt_rtCorr_posCorr_vs_amp[channelName]->Fill(amp,dtcorr);
        p_dt_rtCorr_posCorr_vs_posX[channelName]->Fill(posX,dtcorr);
        p_dt_rtCorr_posCorr_vs_posY[channelName]->Fill(posY,dtcorr);

      } // -- end loop over channels                                                                                                                                                                                                                                     
    }// -- end third loop over events   


  // -- Compute resolutions (gaussian and effective sigma)
  std::map<std::string,TF1*>  fitFun_ampCorr;
  std::map<std::string,TF1*>  fitFun_ampCorr_posCorr;
  std::map<std::string,TF1*>  fitFun_rtCorr;
  std::map<std::string,TF1*>  fitFun_rtCorr_posCorr;
  std::map<std::string,TF1*>  fitFun_ampCorr_central;
  std::map<std::string,TF1*>  fitFun_ampCorr_border;

  float tmin = 0;
  float tmax = 0;
  double sigmaEff;
  float* vals = new float[6];

  std::map<std::string,TH1F*>  h_effectiveSigma;
  std::map<std::string,TH1F*>  h_effectiveSigma_ampCorr;
  std::map<std::string,TH1F*>  h_effectiveSigma_ampCorr_posCorr;
  std::map<std::string,TH1F*>  h_effectiveSigma_rtCorr;
  std::map<std::string,TH1F*>  h_effectiveSigma_rtCorr_posCorr;

  std::map<std::string,TH1F*>  h_effectiveSigma_central;
  std::map<std::string,TH1F*>  h_effectiveSigma_border;
  std::map<std::string,TH1F*>  h_effectiveSigma_ampCorr_central;
  std::map<std::string,TH1F*>  h_effectiveSigma_ampCorr_border;

  for (int ich = 0; ich < NCHANNELS; ich++){
    channelName = timeChannels[ich];
    h_effectiveSigma[channelName]  = new TH1F(Form("h_effectiveSigma_%s",channelName.c_str()),Form("h_effectiveSigma_%s",channelName.c_str()),2000,0.,200.);
    h_effectiveSigma_ampCorr[channelName]  = new TH1F(Form("h_effectiveSigma_ampCorr_%s",channelName.c_str()),Form("h_effectiveSigma_ampCorr_%s",channelName.c_str()), 2000,0.,200.);
    h_effectiveSigma_ampCorr_posCorr[channelName]  = new TH1F(Form("h_effectiveSigma_ampCorr_posCorr_%s",channelName.c_str()),Form("h_effectiveSigma_ampCorr_posCorr_%s",channelName.c_str()),2000,0.,200.);
    h_effectiveSigma_rtCorr[channelName]  = new TH1F(Form("h_effectiveSigma_rtCorr_%s",channelName.c_str()),Form("h_effectiveSigma_rtCorr_%s",channelName.c_str()), 2000,0.,200.);
    h_effectiveSigma_rtCorr_posCorr[channelName]  = new TH1F(Form("h_effectiveSigma_rtCorr_posCorr_%s",channelName.c_str()),Form("h_effectiveSigma_rtCorr_posCorr_%s",channelName.c_str()),2000,0.,200.);
    
    h_effectiveSigma_central[channelName]  = new TH1F(Form("h_effectiveSigma_central_%s",channelName.c_str()),Form("h_effectiveSigma_central_%s",channelName.c_str()),2000,0.,200.);
    h_effectiveSigma_ampCorr_central[channelName]  = new TH1F(Form("h_effectiveSigma_ampCorr_central_%s",channelName.c_str()),Form("h_effectiveSigma_ampCorr_central_%s",channelName.c_str()), 2000,0.,200.);
    h_effectiveSigma_border[channelName]  = new TH1F(Form("h_effectiveSigma_border_%s",channelName.c_str()),Form("h_effectiveSigma_border_%s",channelName.c_str()),2000,0.,200.);
    h_effectiveSigma_ampCorr_border[channelName]  = new TH1F(Form("h_effectiveSigma_ampCorr_border_%s",channelName.c_str()),Form("h_effectiveSigma_ampCorr_border_%s",channelName.c_str()), 2000,0.,200.);
  }

  int nRe = 4;
  for (int ich = 0; ich < NCHANNELS; ich++){

    channelName = timeChannels[ich];  

    // -- skip MCPs and channels with no entries
    if (channelName == "PTK1" || channelName == "TRG0") continue;


    // -- no corrections
    FindSmallestInterval(vals,h_dt[channelName],0.68,true);
    sigmaEff = 0.5*(vals[5]-vals[4]);
    h_effectiveSigma[channelName]->Fill(sigmaEff);
    h_dt[channelName]->Rebin(nRe);    

    FindSmallestInterval(vals,h_dt_central[channelName],0.68,true);
    sigmaEff = 0.5*(vals[5]-vals[4]);
    h_effectiveSigma_central[channelName]->Fill(sigmaEff);
    h_dt_central[channelName]->Rebin(nRe);    

    FindSmallestInterval(vals,h_dt_border[channelName],0.68,true);
    sigmaEff = 0.5*(vals[5]-vals[4]);
    h_effectiveSigma_border[channelName]->Fill(sigmaEff);
    h_dt_border[channelName]->Rebin(nRe);    


    // ----- amp corr
    
    // -- get the effective (68%) sigma
    FindSmallestInterval(vals,h_dt_ampCorr[channelName],0.68,true); 
    sigmaEff = 0.5*(vals[5]-vals[4]); 
    h_effectiveSigma_ampCorr[channelName]->Fill(sigmaEff);
    h_dt_ampCorr[channelName]->Rebin(nRe);    

    // -- get the gaussian sigma
    fitFun_ampCorr[channelName]= new TF1(Form("fitFun_ampCorr_%s",channelName.c_str()),"gaus");
    fitFun_ampCorr[channelName]->SetLineColor(2);
    fitFun_ampCorr[channelName]->SetLineWidth(1);
    fitFun_ampCorr[channelName]->SetParameter(1,h_dt_ampCorr[channelName]->GetMean());
    fitFun_ampCorr[channelName]->SetParameter(2,h_dt_ampCorr[channelName]->GetRMS());
    tmin = h_dt_ampCorr[channelName]->GetMean()-1.5*sigmaEff;
    tmax = h_dt_ampCorr[channelName]->GetMean()+1.5*sigmaEff;
    h_dt_ampCorr[channelName]->Fit(Form("fitFun_ampCorr_%s",channelName.c_str()),"QRS","", tmin, tmax);


    // -- only central impact point
    // -- get the effective (68%) sigma
    FindSmallestInterval(vals,h_dt_ampCorr_central[channelName],0.68,true); 
    sigmaEff = 0.5*(vals[5]-vals[4]); 
    h_effectiveSigma_ampCorr_central[channelName]->Fill(sigmaEff);
    h_dt_ampCorr_central[channelName]->Rebin(nRe);    

    // -- get the gaussian sigma
    fitFun_ampCorr_central[channelName]= new TF1(Form("fitFun_ampCorr_central_%s",channelName.c_str()),"gaus");
    fitFun_ampCorr_central[channelName]->SetLineColor(2);
    fitFun_ampCorr_central[channelName]->SetLineWidth(1);
    fitFun_ampCorr_central[channelName]->SetParameter(1,h_dt_ampCorr_central[channelName]->GetMean());
    fitFun_ampCorr_central[channelName]->SetParameter(2,h_dt_ampCorr_central[channelName]->GetRMS());
    tmin = h_dt_ampCorr_central[channelName]->GetMean()-1.5*sigmaEff;
    tmax = h_dt_ampCorr_central[channelName]->GetMean()+1.5*sigmaEff;
    h_dt_ampCorr_central[channelName]->Fit(Form("fitFun_ampCorr_central_%s",channelName.c_str()),"QRS","", tmin, tmax);


    // -- only corner impact point
    // -- get the effective (68%) sigma
    FindSmallestInterval(vals,h_dt_ampCorr_border[channelName],0.68,true); 
    sigmaEff = 0.5*(vals[5]-vals[4]); 
    h_effectiveSigma_ampCorr_border[channelName]->Fill(sigmaEff);
    h_dt_ampCorr_border[channelName]->Rebin(nRe);    

    // -- get the gaussian sigma
    fitFun_ampCorr_border[channelName]= new TF1(Form("fitFun_ampCorr_border_%s",channelName.c_str()),"gaus");
    fitFun_ampCorr_border[channelName]->SetLineColor(2);
    fitFun_ampCorr_border[channelName]->SetLineWidth(1);
    fitFun_ampCorr_border[channelName]->SetParameter(1,h_dt_ampCorr_border[channelName]->GetMean());
    fitFun_ampCorr_border[channelName]->SetParameter(2,h_dt_ampCorr_border[channelName]->GetRMS());
    tmin = h_dt_ampCorr_border[channelName]->GetMean()-1.5*sigmaEff;
    tmax = h_dt_ampCorr_border[channelName]->GetMean()+1.5*sigmaEff;
    h_dt_ampCorr_border[channelName]->Fit(Form("fitFun_ampCorr_border_%s",channelName.c_str()),"QRS","", tmin, tmax);
    

    // ---- amp & pos corrections
    
    // -- get the effective (68%) sigma
    FindSmallestInterval(vals,h_dt_ampCorr_posCorr[channelName],0.68,true); 
    sigmaEff = 0.5*(vals[5]-vals[4]); 
    h_effectiveSigma_ampCorr_posCorr[channelName]->Fill(sigmaEff);
    h_dt_ampCorr_posCorr[channelName]->Rebin(nRe);    

    // -- get the gaussian sigma
    fitFun_ampCorr_posCorr[channelName]= new TF1(Form("fitFun_ampCorr_posCorr_%s",channelName.c_str()),"gaus");
    fitFun_ampCorr_posCorr[channelName]->SetLineColor(1);
    fitFun_ampCorr_posCorr[channelName]->SetLineWidth(1);
    fitFun_ampCorr_posCorr[channelName]->SetParameter(1,h_dt_ampCorr_posCorr[channelName]->GetMean());
    fitFun_ampCorr_posCorr[channelName]->SetParameter(2,h_dt_ampCorr_posCorr[channelName]->GetRMS());
    tmin = h_dt_ampCorr_posCorr[channelName]->GetMean()-1.5*sigmaEff;
    tmax = h_dt_ampCorr_posCorr[channelName]->GetMean()+1.5*sigmaEff;
    h_dt_ampCorr_posCorr[channelName]->Fit(Form("fitFun_ampCorr_posCorr_%s",channelName.c_str()),"QRS","", tmin, tmax);


    

    // ----- risetime corr
    
    // -- get the effective (68%) sigma
    FindSmallestInterval(vals,h_dt_rtCorr[channelName],0.68,true); 
    sigmaEff = 0.5*(vals[5]-vals[4]); 
    h_effectiveSigma_rtCorr[channelName]->Fill(sigmaEff);
    h_dt_rtCorr[channelName]->Rebin(nRe);    

    // -- get the gaussian sigma
    fitFun_rtCorr[channelName]= new TF1(Form("fitFun_rtCorr_%s",channelName.c_str()),"gaus");
    fitFun_rtCorr[channelName]->SetLineColor(2);
    fitFun_rtCorr[channelName]->SetLineWidth(1);
    fitFun_rtCorr[channelName]->SetParameter(1,h_dt_rtCorr[channelName]->GetMean());
    fitFun_rtCorr[channelName]->SetParameter(2,h_dt_rtCorr[channelName]->GetRMS());
    tmin = h_dt_rtCorr[channelName]->GetMean()-1.5*sigmaEff;
    tmax = h_dt_rtCorr[channelName]->GetMean()+1.5*sigmaEff;
    h_dt_rtCorr[channelName]->Fit(Form("fitFun_rtCorr_%s",channelName.c_str()),"QRS","", tmin, tmax);
    

    // ---- rt & pos corrections
    
    // -- get the effective (68%) sigma
    FindSmallestInterval(vals,h_dt_rtCorr_posCorr[channelName],0.68,true); 
    sigmaEff = 0.5*(vals[5]-vals[4]); 
    h_effectiveSigma_rtCorr_posCorr[channelName]->Fill(sigmaEff);
    h_dt_rtCorr_posCorr[channelName]->Rebin(nRe);    

    // -- get the gaussian sigma
    fitFun_rtCorr_posCorr[channelName]= new TF1(Form("fitFun_rtCorr_posCorr_%s",channelName.c_str()),"gaus");
    fitFun_rtCorr_posCorr[channelName]->SetLineColor(1);
    fitFun_rtCorr_posCorr[channelName]->SetLineWidth(1);
    fitFun_rtCorr_posCorr[channelName]->SetParameter(1,h_dt_rtCorr_posCorr[channelName]->GetMean());
    fitFun_rtCorr_posCorr[channelName]->SetParameter(2,h_dt_rtCorr_posCorr[channelName]->GetRMS());
    tmin = h_dt_rtCorr_posCorr[channelName]->GetMean()-1.5*sigmaEff;
    tmax = h_dt_rtCorr_posCorr[channelName]->GetMean()+1.5*sigmaEff;
    h_dt_rtCorr_posCorr[channelName]->Fit(Form("fitFun_rtCorr_posCorr_%s",channelName.c_str()),"QRS","", tmin, tmax);
    

  }//

  


  // -- Save histograms on file
  std::cout << "Saving histograms in file " << OutputFile.c_str() <<std::endl;
  
  TFile *outfile = new TFile(OutputFile.c_str(),"recreate");
  
  for (int ich = 0; ich < NCHANNELS; ich++){
    channelName = timeChannels[ich];
    std::cout << channelName <<std::endl;
    
    
    h_amp_nocuts[channelName]->Write();
    h_amp[channelName]->Write();
    h_time[channelName]->Write();
    h_rt[channelName]->Write();
    h_posX[channelName]->Write();
    h_posY[channelName]->Write();
    h2_posXY[channelName]->Write();
    p2_eff_vs_posXY[channelName]->Write();
    p2_dt_vs_posXY[channelName]->Write();
    p2_amp_vs_posXY[channelName]->Write();
    p_amp_vs_posX[channelName]->Write();
    p_amp_vs_posY[channelName]->Write();
    p_eff_vs_posX[channelName]->Write();
    p_eff_vs_posY[channelName]->Write();
    
    h_dt[channelName]->Write();
    h_dt_central[channelName]->Write();
    h_dt_border[channelName]->Write();
    h_dt_ampCorr[channelName]->Write();
    h_dt_ampCorr_central[channelName]->Write();
    h_dt_ampCorr_border[channelName]->Write();
    h_dt_ampCorr_posCorr[channelName]->Write();
    h_dt_rtCorr[channelName]->Write();
    h_dt_rtCorr_posCorr[channelName]->Write();
    
    p_dt_vs_amp[channelName]->Write();
    p_dt_vs_rt[channelName]->Write();
    p_dt_vs_posX[channelName]->Write();
    p_dt_vs_posY[channelName]->Write();
    
    p_dt_ampCorr_vs_amp[channelName]->Write();
    p_dt_ampCorr_vs_posX[channelName]->Write();
    p_dt_ampCorr_vs_posY[channelName]->Write();
    
    p_dt_ampCorr_posCorr_vs_amp[channelName]->Write();
    p_dt_ampCorr_posCorr_vs_posX[channelName]->Write();
    p_dt_ampCorr_posCorr_vs_posY[channelName]->Write();

    p_dt_rtCorr_vs_amp[channelName]->Write();
    p_dt_rtCorr_vs_posX[channelName]->Write();
    p_dt_rtCorr_vs_posY[channelName]->Write();
    
    p_dt_rtCorr_posCorr_vs_amp[channelName]->Write();
    p_dt_rtCorr_posCorr_vs_posX[channelName]->Write();
    p_dt_rtCorr_posCorr_vs_posY[channelName]->Write();
    
    
    h_effectiveSigma[channelName]->Write();
    h_effectiveSigma_ampCorr[channelName]->Write();
    h_effectiveSigma_ampCorr_posCorr[channelName]->Write();

    h_effectiveSigma_rtCorr[channelName]->Write();
    h_effectiveSigma_rtCorr_posCorr[channelName]->Write();
    
    h_effectiveSigma_central[channelName]->Write();
    h_effectiveSigma_ampCorr_central[channelName]->Write();

    h_effectiveSigma_border[channelName]->Write();
    h_effectiveSigma_ampCorr_border[channelName]->Write();
    
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

