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
  float* t_amp;
  float* t_time;
  float* t_time_max;
  float* t_b_rms;
  int  t_CFD;
  int  t_LED;
  std::map<std::string,int> *t_channelId;

};
/**********************/




void InitTreeVars(TTree* chain1, TreeVars& treeVars);



// ====== *** MAIN *** ======
int main(int argc, char** argv)
{
  gSystem -> Load("CfgManager/lib/libCFGMan.so");
  
  if( argc < 2 )
    {
      std::cerr << ">>>>> TimingStudyMatrix.cpp::usage:   " << argv[0] << " configFileName" << std::endl;
      return 1;
    }
  
  //----------------------
  // parse the config file
  CfgManager opts;
  opts.ParseConfigFile(argv[1]);
  
  // std::vector<std::string> inputFiles = opts.GetOpt<std::vector<std::string> >("Input.inputFiles");
  std::vector<float> runs = opts.GetOpt<std::vector<float> >("Input.runList");

  timeChannels   = opts.GetOpt<std::vector<std::string> >("Input.timeChannels");
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
  

  std::cout << " Setting TDRStyle..."<<std::endl;
  //--------------------------
  // labels and canvas style
  setTDRStyle();

  
  //---------------------------
  // open input files and trees
  std::cout << "Loading tree ..." << std::endl; 
  TChain* chain1 = new TChain("digi","digi");
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
  std::map<std::string,TH1F*>  h_ampRatio_MCP1;
  std::map<std::string,TH1F*>  h_ampRatio_MCP2;
  std::map<std::string,TH1F*>  h_time;
  

  // MCP1 ref
  std::map<std::string,TH1F*>  h_dt_MCP1;
  std::map<std::string,TH1F*>  h_dt_MCP1_ampCorr;
  std::map<std::string,TH1F*>  h_dt_MCP1_ampRatioCorr;

  std::map<std::string,TH2F*>  h2_dt_MCP1_vs_amp;
  std::map<std::string,TH2F*>  h2_dt_MCP1_vs_ampRatio;

  std::map<std::string,TH2F*>  h2_dt_MCP1_ampCorr_vs_amp;
  std::map<std::string,TH2F*>  h2_dt_MCP1_ampRatioCorr_vs_amp;

  std::map<std::string,TProfile*>  p_dt_MCP1_vs_amp;
  std::map<std::string,TProfile*>  p_dt_MCP1_vs_ampRatio;

  std::map<std::string,TProfile*>  p_dt_MCP1_ampCorr_vs_amp;
  std::map<std::string,TProfile*>  p_dt_MCP1_ampRatioCorr_vs_amp;

  //MCP2 ref
  std::map<std::string,TH1F*>  h_dt_MCP2;
  std::map<std::string,TH1F*>  h_dt_MCP2_ampCorr;
  std::map<std::string,TH1F*>  h_dt_MCP2_ampRatioCorr;

  std::map<std::string,TH2F*>  h2_dt_MCP2_vs_amp;
  std::map<std::string,TH2F*>  h2_dt_MCP2_vs_ampRatio;

  std::map<std::string,TH2F*>  h2_dt_MCP2_ampCorr_vs_amp;
  std::map<std::string,TH2F*>  h2_dt_MCP2_ampRatioCorr_vs_amp;

  std::map<std::string,TProfile*>  p_dt_MCP2_vs_amp;
  std::map<std::string,TProfile*>  p_dt_MCP2_vs_ampRatio;

  std::map<std::string,TProfile*>  p_dt_MCP2_ampCorr_vs_amp;
  std::map<std::string,TProfile*>  p_dt_MCP2_ampRatioCorr_vs_amp;


  std::string channelName;
  std::string channelNameA;

  for (int i = 0 ; i < NCHANNELS; i++){
    channelName = timeChannels.at(i); // es: ch = MCP1, NINOMAT1, etc...
    h_amp_nocuts[channelName]  = new TH1F(Form("h_amp_nocuts_%s",channelName.c_str()),Form("h_amp_nocuts_%s", channelName.c_str()),1000, 0., 1.0);
    h_amp[channelName]  = new TH1F(Form("h_amp_%s",channelName.c_str()),Form("h_amp_%s",channelName.c_str()), 1000, 0., 1.0);
    h_ampRatio_MCP1[channelName]  = new TH1F(Form("h_ampRatio_%s_MCP1",channelName.c_str()),Form("h_amp_%s_MCP1",channelName.c_str()), 200, 0., 20.);
    h_ampRatio_MCP2[channelName]  = new TH1F(Form("h_ampRatio_%s_MCP2",channelName.c_str()),Form("h_amp_%s_MCP2",channelName.c_str()), 200, 0., 20.);
    h_time[channelName] = new TH1F(Form("h_time_%s",channelName.c_str()),Form("h_time_%s",channelName.c_str()),200,0.,200.);

    // MCP1 ref
    h_dt_MCP1[channelName]   = new TH1F(Form("h_dt_%s_MCP1",channelName.c_str()),Form("h_dt_%s_MCP1",channelName.c_str()),10000, -20., 20.);
    h_dt_MCP1_ampCorr[channelName]   = new TH1F(Form("h_dt_%s_MCP1_ampCorr",channelName.c_str()),Form("h_dt_%s_MCP1_ampCorr",channelName.c_str()),10000, -20., 20.);
    h_dt_MCP1_ampRatioCorr[channelName]   = new TH1F(Form("h_dt_%s_MCP1_ampRatioCorr",channelName.c_str()),Form("h_dt_%s_MCP1_ampRatioCorr",channelName.c_str()), 10000, -20., 20.);

    h2_dt_MCP1_vs_amp[channelName] = new TH2F(Form("h2_dt_%s_MCP1_vs_amp",channelName.c_str()),Form("h2_dt_%s_MCP1_vs_amp",channelName.c_str()), 200, 0., 1., 200, -20., 20.);
    h2_dt_MCP1_vs_ampRatio[channelName] = new TH2F(Form("h2_dt_%s_MCP1_vs_ampRatio",channelName.c_str()),Form("h2_dt_%s_MCP1_vs_ampRatio",channelName.c_str()), 200, 0, 20, 200, -20., 20.);

    h2_dt_MCP1_ampCorr_vs_amp[channelName] = new TH2F(Form("h2_dt_%s_MCP1_ampCorr_vs_amp",channelName.c_str()),Form("h2_dt_%s_MCP1_ampCorr_vs_amp",channelName.c_str()), 200, 0., 1., 200, -20., 20.);
    h2_dt_MCP1_ampRatioCorr_vs_amp[channelName] = new TH2F(Form("h2_dt_%s_MCP1_ampRatioCorr_vs_amp",channelName.c_str()),Form("h2_dt_%s_MCP1_ampRatioCorr_vs_amp",channelName.c_str()), 200, 0., 1., 200, -20., 20.);

    p_dt_MCP1_vs_amp[channelName] = new TProfile(Form("p_dt_%s_MCP1_vs_amp",channelName.c_str()),Form("p_dt_%s_MCP1_vs_amp",channelName.c_str()), 200, 0., 1., -20., 20.);
    p_dt_MCP1_vs_ampRatio[channelName] = new TProfile(Form("p_dt_%s_MCP1_vs_ampRatio",channelName.c_str()),Form("p_dt_%s_MCP1_vs_ampRatio",channelName.c_str()), 200, 0, 20, -20., 20.);

    p_dt_MCP1_ampCorr_vs_amp[channelName] = new TProfile(Form("p_dt_%s_MCP1_ampCorr_vs_amp",channelName.c_str()),Form("p_dt_%s_MCP1_vs_ampCorr_amp",channelName.c_str()), 200, 0, 1., -20., 20.);
    p_dt_MCP1_ampRatioCorr_vs_amp[channelName] = new TProfile(Form("p_dt_%s_MCP1_ampRatioCorr_vs_amp",channelName.c_str()),Form("p_dt_%s_MCP1_vs_ampRatioCorr_amp",channelName.c_str()), 200, 0., 1., -20., 20.);

    // MCP2 ref
    h_dt_MCP2[channelName]   = new TH1F(Form("h_dt_%s_MCP2",channelName.c_str()),Form("h_dt_%s_MCP2",channelName.c_str()),10000,-20.,20.);
    h_dt_MCP2_ampCorr[channelName]   = new TH1F(Form("h_dt_%s_MCP2_ampCorr",channelName.c_str()),Form("h_dt_%s_MCP2_ampCorr",channelName.c_str()),10000, -20., 20.);
    h_dt_MCP2_ampRatioCorr[channelName]   = new TH1F(Form("h_dt_%s_MCP2_ampRatioCorr",channelName.c_str()),Form("h_dt_%s_MCP2_ampRatioCorr",channelName.c_str()),10000, -20.,20.);

    h2_dt_MCP2_vs_amp[channelName] = new TH2F(Form("h2_dt_%s_MCP2_vs_amp",channelName.c_str()),Form("h2_dt_%s_MCP2_vs_amp",channelName.c_str()), 200, 0., 1., 200, -20., 20.);
    h2_dt_MCP2_vs_ampRatio[channelName] = new TH2F(Form("h2_dt_%s_MCP2_vs_ampRatio",channelName.c_str()),Form("h2_dt_%s_MCP2_vs_ampRatio",channelName.c_str()), 200, 0, 20, 200, -20., 20.);

    h2_dt_MCP2_ampCorr_vs_amp[channelName] = new TH2F(Form("h2_dt_%s_MCP2_ampCorr_vs_amp",channelName.c_str()),Form("h2_dt_%s_MCP2_ampCorr_vs_amp",channelName.c_str()), 200, 0., 1., 200, -20., 20.);
    h2_dt_MCP2_ampRatioCorr_vs_amp[channelName] = new TH2F(Form("h2_dt_%s_MCP2_ampRatioCorr_vs_amp",channelName.c_str()),Form("h2_dt_%s_MCP2_ampRatioCorr_vs_amp",channelName.c_str()), 200, 0., 1., 200, -20., 20.);

    p_dt_MCP2_vs_amp[channelName] = new TProfile(Form("p_dt_%s_MCP2_vs_amp",channelName.c_str()),Form("p_dt_%s_MCP2_vs_amp",channelName.c_str()), 200, 0., 1., -20., 20.);
    p_dt_MCP2_vs_ampRatio[channelName] = new TProfile(Form("p_dt_%s_MCP2_vs_ampRatio",channelName.c_str()),Form("p_dt_%s_MCP2_vs_ampRatio",channelName.c_str()), 200, 0, 20, -20., 20.);

    p_dt_MCP2_ampCorr_vs_amp[channelName] = new TProfile(Form("p_dt_%s_MCP2_ampCorr_vs_amp",channelName.c_str()),Form("p_dt_%s_MCP2_vs_ampCorr_amp",channelName.c_str()), 200, 0., 1., -20., 20.);
    p_dt_MCP2_ampRatioCorr_vs_amp[channelName] = new TProfile(Form("p_dt_%s_MCP2_ampRatioCorr_vs_amp",channelName.c_str()),Form("p_dt_%s_MCP2_vs_ampRatioCorr_amp",channelName.c_str()), 200, 0., 1., -20., 20.);
  }




  float amp  = 0.;
  float time = 0.;
  float dt1   = 0.;
  float dt2   = 0.;
  float dt1corr = 0.;
  float dt2corr = 0.;
  float ampMCP1 = 0.;
  float ampMCP2 = 0.;
  float timeMCP1 = 0.;
  float timeMCP2 = 0.;
  float timeTrg0 =0;
  float timeTrg1 =0;

  float k = 1./4096.; // factor to convert amplitudes in V
  float trgOffset = 160; // just for plotting times in a range 0-200

  float tMaxMCP = 35;
  float tMaxPTK = 25;
  float bRmsMax = 240;

  //-----------------------
  // first loop over events
  std::cout<<"First loop over events ..." <<std::endl;
  int nEntries = chain1 -> GetEntries();
  for(int entry = 0; entry < nEntries; ++entry)
    {
      if( entry%1000 == 0 ) std::cout << ">>> Reading entry " << entry << " / " << nEntries << "\r" << std::flush;
      chain1 -> GetEntry(entry);


      // -- trig time
      timeTrg0 = treeVars.t_time[(*treeVars.t_channelId)["TRG0"]+treeVars.t_LED] - trgOffset;
      timeTrg1 = treeVars.t_time[(*treeVars.t_channelId)["TRG1"]+treeVars.t_LED] - trgOffset;

      
      
      // -- MCPs amplitude and time ( MCP1 for readout group 0, MCP2 for readout group 1)
      ampMCP1 =  treeVars.t_amp[(*treeVars.t_channelId)["MCP1"]] * k;
      ampMCP2 =  treeVars.t_amp[(*treeVars.t_channelId)["MCP2"]] * k;
      timeMCP1 =  treeVars.t_time[(*treeVars.t_channelId)["MCP1"]+treeVars.t_CFD] - timeTrg1;
      timeMCP2 =  treeVars.t_time[(*treeVars.t_channelId)["MCP2"]+treeVars.t_CFD] - timeTrg1;

      

      //--- loop over channels
      for (int ich = 0; ich < NCHANNELS; ich++){

	channelName = timeChannels[ich];
	channelNameA = ampChannels[ich];
	
	// -- skip TRG channels
	if (channelName == "TRG0" || channelName == "TRG1") continue;

	// -- read amplitude and time for each channel
	amp  = treeVars.t_amp[(*treeVars.t_channelId)[channelNameA]] * k;
	if (channelName == "MCP1"  || channelName == "MCP2" || channelName == "PTK1" || channelName == "PTK2"){
	  time = treeVars.t_time[(*treeVars.t_channelId)[channelName]+treeVars.t_CFD];
	}
	else{
	  time = treeVars.t_time[(*treeVars.t_channelId)[channelName]+treeVars.t_LED];
	}
      

	
	// -- fill histograms before any selection
	h_amp_nocuts[channelName] -> Fill(amp);

	// -- select events
	if ( ampMCP1 < minAmplitude["MCP1"] || ampMCP1 > maxAmplitude["MCP1"]) continue;
	if ( ampMCP2 < minAmplitude["MCP2"] || ampMCP2 > maxAmplitude["MCP2"]) continue;
	if ( amp < minAmplitude[channelNameA]  || amp > maxAmplitude[channelNameA]) continue;
	if ( time < 0 || time > 200) continue;
	if ( treeVars.t_b_rms[(*treeVars.t_channelId)["PTK1"]] > bRmsMax) continue; // cleaning reco failures... 
	if ( treeVars.t_b_rms[(*treeVars.t_channelId)["PTK2"]] > bRmsMax) continue; // cleaning reco failures... 


	// subtract TRG time for each digi group 
	if (channelName == "MCP1"  || channelName == "MCP2" || channelName == "PTK2") time = time - timeTrg1;
	if (channelName == "PTK1") time = time - timeTrg0;


	dt1 = time - timeMCP1;
	dt2 = time - timeMCP2;

	  
	// -- fill histograms
	h_amp[channelName]  -> Fill(amp);
	h_ampRatio_MCP1[channelName] -> Fill(amp/ampMCP1);
	h_ampRatio_MCP2[channelName] -> Fill(amp/ampMCP2);
	h_time[channelName] -> Fill(time);

	if ( (channelName == "MCP1" || channelName == "MCP2") &&  time > tMaxMCP ) continue;
	if ( (channelName == "PTK1" || channelName == "PTK2") &&  time > tMaxPTK ) continue;

	h_dt_MCP1[channelName]->Fill(dt1);
	h2_dt_MCP1_vs_amp[channelName]->Fill(amp,dt1);
	h2_dt_MCP1_vs_ampRatio[channelName]->Fill(amp/ampMCP1,dt1);
	p_dt_MCP1_vs_amp[channelName]->Fill(amp,dt1);
	p_dt_MCP1_vs_ampRatio[channelName]->Fill(amp/ampMCP1,dt1);
	
	h_dt_MCP2[channelName]->Fill(dt2);
	h2_dt_MCP2_vs_amp[channelName]->Fill(amp,dt2);
	h2_dt_MCP2_vs_ampRatio[channelName]->Fill(amp/ampMCP2,dt2);
	p_dt_MCP2_vs_amp[channelName]->Fill(amp,dt2);
	p_dt_MCP2_vs_ampRatio[channelName]->Fill(amp/ampMCP2,dt2);
		

      } // -- end loop over channels

    }// -- end first loop over events
  
  


  // --- Amplitude walk corrections
  std::map<std::string,TF1*>  fitFunc1_ampCorr;
  std::map<std::string,TF1*>  fitFunc2_ampCorr;
  std::map<std::string,TF1*>  fitFunc1_ampRatioCorr;
  std::map<std::string,TF1*>  fitFunc2_ampRatioCorr;

  for (int i = 0 ; i < NCHANNELS; i++){
    channelName = timeChannels.at(i);
    fitFunc1_ampCorr[channelName] = new TF1(Form("fitFunc1_ampCorr_%s",channelName.c_str()),"pol6",0,1);
    fitFunc1_ampCorr[channelName] -> SetLineColor(kRed);
    p_dt_MCP1_vs_amp[channelName] -> Fit(Form("fitFunc1_ampCorr_%s",channelName.c_str()),"QS+");

    fitFunc2_ampCorr[channelName] = new TF1(Form("fitFunc2_ampCorr_%s",channelName.c_str()),"pol6",0,1);
    fitFunc2_ampCorr[channelName] -> SetLineColor(kRed);
    p_dt_MCP2_vs_amp[channelName] -> Fit(Form("fitFunc2_ampCorr_%s",channelName.c_str()),"QS+");

    fitFunc1_ampRatioCorr[channelName] = new TF1(Form("fitFunc1_ampRatioCorr_%s",channelName.c_str()),"pol6",0,10);
    fitFunc1_ampRatioCorr[channelName] -> SetLineColor(kRed);
    p_dt_MCP1_vs_ampRatio[channelName] -> Fit(Form("fitFunc1_ampRatioCorr_%s",channelName.c_str()),"QS+");

    fitFunc2_ampRatioCorr[channelName] = new TF1(Form("fitFunc2_ampRatioCorr_%s",channelName.c_str()),"pol6",0,10);
    fitFunc2_ampRatioCorr[channelName] -> SetLineColor(kRed);
    p_dt_MCP2_vs_ampRatio[channelName] -> Fit(Form("fitFunc2_ampRatioCorr_%s",channelName.c_str()),"QS+");
  }




  // --- second loop over events -- fill histograms after applying amplitude walk corrections
  std::cout<<"Second loop over events ..." <<std::endl;
  for(int entry = 0; entry < nEntries; ++entry)
    {
      if( entry%1000 == 0 ) std::cout << ">>> Reading entry " << entry << " / " << nEntries << "\r" << std::flush;
      chain1 -> GetEntry(entry);

      // -- trig time
      timeTrg0 = treeVars.t_time[(*treeVars.t_channelId)["TRG0"]+treeVars.t_LED] - trgOffset ;
      timeTrg1 = treeVars.t_time[(*treeVars.t_channelId)["TRG1"]+treeVars.t_LED] - trgOffset ;

      //--- MCPs amplitude and time
      ampMCP1 =  treeVars.t_amp[(*treeVars.t_channelId)["MCP1"]] * k;
      ampMCP2 =  treeVars.t_amp[(*treeVars.t_channelId)["MCP2"]] * k ;
      timeMCP1 =  treeVars.t_time[(*treeVars.t_channelId)["MCP1"]+treeVars.t_CFD] - timeTrg1;
      timeMCP2 =  treeVars.t_time[(*treeVars.t_channelId)["MCP2"]+treeVars.t_CFD] - timeTrg1;

      //--- loop over channels
      for (int ich = 0; ich < NCHANNELS; ich++){

        channelName  = timeChannels[ich];
        channelNameA = ampChannels[ich];

        // -- skip TRG channels
        if (channelName == "TRG0" || channelName == "TRG1") continue;

        // -- read amplitude and time for each channel
        amp  = treeVars.t_amp[(*treeVars.t_channelId)[channelNameA]] * k;
	if (channelName == "MCP1"  || channelName == "MCP2" || channelName == "PTK1" || channelName == "PTK2"){
          time = treeVars.t_time[(*treeVars.t_channelId)[channelName]+treeVars.t_CFD];
        }
        else{
          time = treeVars.t_time[(*treeVars.t_channelId)[channelName]+treeVars.t_LED];
        }


        // -- select events
        if ( ampMCP1 < minAmplitude["MCP1"] || ampMCP1 > maxAmplitude["MCP1"]) continue;
        if ( ampMCP2 < minAmplitude["MCP2"] || ampMCP2 > maxAmplitude["MCP2"]) continue;
        if ( amp < minAmplitude[channelNameA]  || amp > maxAmplitude[channelNameA]) continue;
        if ( time < 0 || time > 200) continue;
	if ( treeVars.t_b_rms[(*treeVars.t_channelId)["PTK1"]] > bRmsMax) continue; // cleaning reco failures...
	if ( treeVars.t_b_rms[(*treeVars.t_channelId)["PTK2"]] > bRmsMax) continue; // cleaning reco failures...     

	if ( (channelName == "MCP1" || channelName == "MCP2") &&  time > tMaxMCP ) continue;
        if ( (channelName == "PTK1" || channelName == "PTK2") &&  time > tMaxPTK ) continue;

	// subtract TRG time for each digi group          
	if (channelName == "MCP1"  || channelName == "MCP2" || channelName == "PTK2") time = time - timeTrg1;
        if (channelName == "PTK1") time = time - timeTrg0;

        dt1 = time - timeMCP1;
        dt2 = time - timeMCP2;

	// -- amplitude walk correction from fitted funtion 
	dt1corr = dt1 - fitFunc1_ampCorr[channelName]->Eval(amp) + fitFunc1_ampCorr[channelName]->Eval(h_amp[channelName]->GetMean()) ; 
	dt2corr = dt2 - fitFunc2_ampCorr[channelName]->Eval(amp) + fitFunc2_ampCorr[channelName]->Eval(h_amp[channelName]->GetMean()) ; 

	// -- fill histograms
        h_dt_MCP1_ampCorr[channelName]->Fill(dt1corr);
        h2_dt_MCP1_ampCorr_vs_amp[channelName]->Fill(amp,dt1corr);
        p_dt_MCP1_ampCorr_vs_amp[channelName]->Fill(amp,dt1corr);

        h_dt_MCP2_ampCorr[channelName]->Fill(dt2corr);
        h2_dt_MCP2_ampCorr_vs_amp[channelName]->Fill(amp,dt2corr);
        p_dt_MCP2_ampCorr_vs_amp[channelName]->Fill(amp,dt2corr);


	// -- amplitude walk correction from fitted function 
	dt1corr = dt1 - fitFunc1_ampRatioCorr[channelName]->Eval(amp/ampMCP1) + fitFunc1_ampRatioCorr[channelName]->Eval(h_ampRatio_MCP1[channelName]->GetMean()) ; 
	dt2corr = dt2 - fitFunc2_ampRatioCorr[channelName]->Eval(amp/ampMCP2) + fitFunc2_ampRatioCorr[channelName]->Eval(h_ampRatio_MCP2[channelName]->GetMean()) ; 

	// -- fill histograms
        h_dt_MCP1_ampRatioCorr[channelName]->Fill(dt1corr);
        h2_dt_MCP1_ampRatioCorr_vs_amp[channelName]->Fill(amp,dt1corr);
        p_dt_MCP1_ampRatioCorr_vs_amp[channelName]->Fill(amp,dt1corr);

        h_dt_MCP2_ampRatioCorr[channelName]->Fill(dt2corr);
        h2_dt_MCP2_ampRatioCorr_vs_amp[channelName]->Fill(amp,dt2corr);
        p_dt_MCP2_ampRatioCorr_vs_amp[channelName]->Fill(amp,dt2corr);
      } // -- end loop over channels


    }// -- end first loop over events
      
  

  // --  Get time resolution for each channel 
  std::map<std::string,TF1*>  fitFun1;
  std::map<std::string,TF1*>  fitFun2;

  std::map<std::string,float> sigmaGaus1;
  std::map<std::string,float> sigmaGaus2;
  std::map<std::string,float> sigmaEff1;
  std::map<std::string,float> sigmaEff2;

  std::map<std::string,TF1*>  fitFun1_ampRatioCorr;
  std::map<std::string,TF1*>  fitFun2_ampRatioCorr;

  std::map<std::string,float> sigmaGaus1_ampRatioCorr;
  std::map<std::string,float> sigmaGaus2_ampRatioCorr;
  std::map<std::string,float> sigmaEff1_ampRatioCorr;
  std::map<std::string,float> sigmaEff2_ampRatioCorr;

  float tmin, tmax;
  float* vals = new float[6];


  for (int ich = 0; ich < NCHANNELS; ich++){

    channelName = timeChannels[ich];  

    // -- skip  channels with no entries
    if (h_dt_MCP1_ampCorr[channelName] -> GetEntries() == 0) continue;
    if (h_dt_MCP2_ampCorr[channelName] -> GetEntries() == 0) continue;

    // -- get the effective sigma (68%)
    FindSmallestInterval(vals,h_dt_MCP1_ampCorr[channelName],0.68,true); 
    sigmaEff1[channelName] = 0.5*(vals[5]-vals[4]);   

    FindSmallestInterval(vals,h_dt_MCP2_ampCorr[channelName],0.68,true); 
    sigmaEff2[channelName] = 0.5*(vals[5]-vals[4]);   

    // -- get the gaussian sigma
    fitFun1[channelName] = new TF1(Form("fitFun1_%s",channelName.c_str()),"gaus",3);
    fitFun1[channelName]->SetLineColor(1);
    fitFun1[channelName]->SetLineWidth(1);
    fitFun1[channelName]->SetParameter(1,h_dt_MCP1_ampCorr[channelName]->GetMean());
    fitFun1[channelName]->SetParameter(2,h_dt_MCP1_ampCorr[channelName]->GetRMS());
    //tmin = h_dt_MCP1_ampCorr[channelName]->GetMean()-2.0*h_dt_MCP1_ampCorr[channelName]->GetRMS();
    //tmax = h_dt_MCP1_ampCorr[channelName]->GetMean()+2.0*h_dt_MCP1_ampCorr[channelName]->GetRMS();
    tmin = h_dt_MCP1_ampCorr[channelName]->GetMean()-1.5*sigmaEff1[channelName];
    tmax = h_dt_MCP1_ampCorr[channelName]->GetMean()+1.5*sigmaEff1[channelName];
    h_dt_MCP1_ampCorr[channelName]->Fit(Form("fitFun1_%s",channelName.c_str()),"QS+","", tmin, tmax);
    
    sigmaGaus1[channelName] = fitFun1[channelName]->GetParameter(2);
    std::cout << " sigma(" << channelName << " - MCP1) = " << sigmaGaus1[channelName] <<std::endl;

    fitFun2[channelName] = new TF1(Form("fitFun2_%s",channelName.c_str()),"gaus",3);
    fitFun2[channelName]->SetLineColor(1);
    fitFun2[channelName]->SetLineWidth(1);
    fitFun2[channelName]->SetParameter(1,h_dt_MCP2_ampCorr[channelName]->GetMean());
    fitFun2[channelName]->SetParameter(2,h_dt_MCP2_ampCorr[channelName]->GetRMS());
    //tmin = h_dt_MCP2_ampCorr[channelName]->GetMean()-2.0*h_dt_MCP2_ampCorr[channelName]->GetRMS();
    //tmax = h_dt_MCP2_ampCorr[channelName]->GetMean()+2.0*h_dt_MCP2_ampCorr[channelName]->GetRMS();
    tmin = h_dt_MCP2_ampCorr[channelName]->GetMean()-1.5*sigmaEff2[channelName];
    tmax = h_dt_MCP2_ampCorr[channelName]->GetMean()+1.5*sigmaEff2[channelName];
    h_dt_MCP2_ampCorr[channelName]->Fit(Form("fitFun2_%s",channelName.c_str()),"QS+","", tmin, tmax);
    
    sigmaGaus2[channelName] = fitFun2[channelName]->GetParameter(2);
    std::cout << " sigma(" << channelName << " - MCP2) = " << sigmaGaus2[channelName] <<std::endl;


    // ampRatio corrected

    // -- get the effective sigma (68%)
    FindSmallestInterval(vals,h_dt_MCP1_ampRatioCorr[channelName],0.68,true); 
    sigmaEff1_ampRatioCorr[channelName] = 0.5*(vals[5]-vals[4]);   

    FindSmallestInterval(vals,h_dt_MCP2_ampRatioCorr[channelName],0.68,true); 
    sigmaEff2_ampRatioCorr[channelName] = 0.5*(vals[5]-vals[4]);   

    // -- get the gaussian sigma
    fitFun1_ampRatioCorr[channelName] = new TF1(Form("fitFun1_ampRatioCorr_%s",channelName.c_str()),"gaus",3);
    fitFun1_ampRatioCorr[channelName]->SetLineColor(1);
    fitFun1_ampRatioCorr[channelName]->SetLineWidth(1);
    fitFun1_ampRatioCorr[channelName]->SetParameter(1,h_dt_MCP1_ampRatioCorr[channelName]->GetMean());
    fitFun1_ampRatioCorr[channelName]->SetParameter(2,h_dt_MCP1_ampRatioCorr[channelName]->GetRMS());
    //tmin = h_dt_MCP1_ampRatioCorr[channelName]->GetMean()-2.0*h_dt_MCP1_ampRatioCorr[channelName]->GetRMS();
    //tmax = h_dt_MCP1_ampRatioCorr[channelName]->GetMean()+2.0*h_dt_MCP1_ampRatioCorr[channelName]->GetRMS();
    tmin = h_dt_MCP1_ampRatioCorr[channelName]->GetMean()-1.5*sigmaEff1_ampRatioCorr[channelName];
    tmax = h_dt_MCP1_ampRatioCorr[channelName]->GetMean()+1.5*sigmaEff1_ampRatioCorr[channelName];
    h_dt_MCP1_ampRatioCorr[channelName]->Fit(Form("fitFun1_ampRatioCorr_%s",channelName.c_str()),"QS+","", tmin, tmax);
    
    sigmaGaus1_ampRatioCorr[channelName] = fitFun1_ampRatioCorr[channelName]->GetParameter(2);
    std::cout << " sigma(" << channelName << " - MCP1) = " << sigmaGaus1_ampRatioCorr[channelName] <<std::endl;

    fitFun2_ampRatioCorr[channelName] = new TF1(Form("fitFun2_ampRatioCorr_%s",channelName.c_str()),"gaus",3);
    fitFun2_ampRatioCorr[channelName]->SetLineColor(1);
    fitFun2_ampRatioCorr[channelName]->SetLineWidth(1);
    fitFun2_ampRatioCorr[channelName]->SetParameter(1,h_dt_MCP2_ampRatioCorr[channelName]->GetMean());
    fitFun2_ampRatioCorr[channelName]->SetParameter(2,h_dt_MCP2_ampRatioCorr[channelName]->GetRMS());
    //tmin = h_dt_MCP2_ampRatioCorr[channelName]->GetMean()-2.0*h_dt_MCP2_ampRatioCorr[channelName]->GetRMS();
    //tmax = h_dt_MCP2_ampRatioCorr[channelName]->GetMean()+2.0*h_dt_MCP2_ampRatioCorr[channelName]->GetRMS();
    tmin = h_dt_MCP2_ampRatioCorr[channelName]->GetMean()-1.5*sigmaEff2_ampRatioCorr[channelName];
    tmax = h_dt_MCP2_ampRatioCorr[channelName]->GetMean()+1.5*sigmaEff2_ampRatioCorr[channelName];
    h_dt_MCP2_ampRatioCorr[channelName]->Fit(Form("fitFun2_ampRatioCorr_%s",channelName.c_str()),"QS+","", tmin, tmax);
    
    sigmaGaus2_ampRatioCorr[channelName] = fitFun2_ampRatioCorr[channelName]->GetParameter(2);
    std::cout << " sigma(" << channelName << " - MCP2) = " << sigmaGaus2_ampRatioCorr[channelName] <<std::endl;


  }

  delete vals;

  std::cout << "*** With ampl. correction " << std::endl;
  std::cout << "sigma(MCP2-MCP1) = " << sigmaGaus1["MCP2"] <<std::endl;
  std::cout << "sigma(PTK2-MCP1) = " << sigmaGaus1["PTK2"] <<std::endl;
  std::cout << "sigma(PTK2-MCP2) = " << sigmaGaus2["PTK2"] <<std::endl;
  float sigmaGausPTK2 = sqrt (0.5 * (sigmaGaus1["PTK2"]*sigmaGaus1["PTK2"] + sigmaGaus2["PTK2"]*sigmaGaus2["PTK2"] - sigmaGaus1["MCP2"]*sigmaGaus1["MCP2"]) );
  float sigmaGausMCP1 = sqrt(sigmaGaus1["PTK2"]*sigmaGaus1["PTK2"]-sigmaGausPTK2*sigmaGausPTK2 );
  float sigmaGausMCP2 = sqrt(sigmaGaus2["PTK2"]*sigmaGaus2["PTK2"]-sigmaGausPTK2*sigmaGausPTK2 );
  std::cout << "Gaussian sigma: " << std::endl; 
  std::cout << "sigma(PTK2) = " << sigmaGausPTK2 << "    sigma(MCP1) = " << sigmaGausMCP1 << "  sigma(MCP2) = " <<sigmaGausMCP2 <<std::endl;

  float sigmaEffPTK2 = sqrt (0.5 * (sigmaEff1["PTK2"]*sigmaEff1["PTK2"] + sigmaEff2["PTK2"]*sigmaEff2["PTK2"] - sigmaEff1["MCP2"]*sigmaEff1["MCP2"]) );
  float sigmaEffMCP1 = sqrt(sigmaEff1["PTK2"]*sigmaEff1["PTK2"]-sigmaEffPTK2*sigmaEffPTK2 );
  float sigmaEffMCP2 = sqrt(sigmaEff2["PTK2"]*sigmaEff2["PTK2"]-sigmaEffPTK2*sigmaEffPTK2 );
  std::cout << "Effective sigma: " << std::endl; 
  std::cout << "sigma(PTK2) = " << sigmaEffPTK2 << "    sigma(MCP1) = " << sigmaEffMCP1 << "  sigma(MCP2) = " <<sigmaEffMCP2 <<std::endl;



  std::cout << "*** With ampl. ratio correction " << std::endl;
  sigmaGausPTK2 = sqrt (0.5 * (sigmaGaus1_ampRatioCorr["PTK2"]*sigmaGaus1_ampRatioCorr["PTK2"] + sigmaGaus2_ampRatioCorr["PTK2"]*sigmaGaus2_ampRatioCorr["PTK2"] - sigmaGaus1_ampRatioCorr["MCP2"]*sigmaGaus1_ampRatioCorr["MCP2"]) );
  sigmaGausMCP1 = sqrt(sigmaGaus1_ampRatioCorr["PTK2"]*sigmaGaus1["PTK2"]-sigmaGausPTK2*sigmaGausPTK2 );
  sigmaGausMCP2 = sqrt(sigmaGaus2_ampRatioCorr["PTK2"]*sigmaGaus2_ampRatioCorr["PTK2"]-sigmaGausPTK2*sigmaGausPTK2 );
  std::cout << "Gaussian sigma: " << std::endl; 
  std::cout << "sigma(PTK2) = " << sigmaGausPTK2 << "    sigma(MCP1) = " << sigmaGausMCP1 << "  sigma(MCP2) = " <<sigmaGausMCP2 <<std::endl;

  sigmaEffPTK2 = sqrt (0.5 * (sigmaEff1_ampRatioCorr["PTK2"]*sigmaEff1_ampRatioCorr["PTK2"] + sigmaEff2_ampRatioCorr["PTK2"]*sigmaEff2_ampRatioCorr["PTK2"] - sigmaEff1_ampRatioCorr["MCP2"]*sigmaEff1_ampRatioCorr["MCP2"]) );
  sigmaEffMCP1 = sqrt(sigmaEff1_ampRatioCorr["PTK2"]*sigmaEff1_ampRatioCorr["PTK2"]-sigmaEffPTK2*sigmaEffPTK2 );
  sigmaEffMCP2 = sqrt(sigmaEff2_ampRatioCorr["PTK2"]*sigmaEff2_ampRatioCorr["PTK2"]-sigmaEffPTK2*sigmaEffPTK2 );
  std::cout << "Effective sigma: " << std::endl; 
  std::cout << "sigma(PTK2) = " << sigmaEffPTK2 << "    sigma(MCP1) = " << sigmaEffMCP1 << "  sigma(MCP2) = " <<sigmaEffMCP2 <<std::endl;



  // -- Save histograms on file
  std::cout << "Saving histograms in file " << OutputFile.c_str() <<std::endl;
  
  TFile *outfile = new TFile(OutputFile.c_str(),"recreate");
  
  //TDirectory *outdir = outfile->mkdir(label.c_str());
  //outdir->cd();
  
  for (int ich = 0; ich < NCHANNELS; ich++){
    channelName = timeChannels[ich];

    if ( channelName == "TRG0" || channelName == "TRG1") continue;

    h_amp_nocuts[channelName]->Write();
    h_amp[channelName]->Write();
    h_ampRatio_MCP1[channelName]->Write();
    h_ampRatio_MCP2[channelName]->Write();
    h_time[channelName]->Write();

    h_dt_MCP1[channelName]->Write();
    h2_dt_MCP1_vs_amp[channelName]->Write();
    h2_dt_MCP1_vs_ampRatio[channelName]->Write();
    p_dt_MCP1_vs_amp[channelName]->Write();
    p_dt_MCP1_vs_ampRatio[channelName]->Write();

    h_dt_MCP1_ampCorr[channelName]->Write();
    h_dt_MCP1_ampRatioCorr[channelName]->Write();
    h2_dt_MCP1_ampCorr_vs_amp[channelName]->Write();
    h2_dt_MCP1_ampRatioCorr_vs_amp[channelName]->Write();
    p_dt_MCP1_ampCorr_vs_amp[channelName]->Write();
    p_dt_MCP1_ampRatioCorr_vs_amp[channelName]->Write();

    h_dt_MCP2[channelName]->Write();
    h2_dt_MCP2_vs_amp[channelName]->Write();
    h2_dt_MCP2_vs_ampRatio[channelName]->Write();
    p_dt_MCP2_vs_amp[channelName]->Write();
    p_dt_MCP2_vs_ampRatio[channelName]->Write();

    h_dt_MCP2_ampCorr[channelName]->Write();
    h_dt_MCP2_ampRatioCorr[channelName]->Write();
    h2_dt_MCP2_ampCorr_vs_amp[channelName]->Write();
    h2_dt_MCP2_ampRatioCorr_vs_amp[channelName]->Write();
    p_dt_MCP2_ampCorr_vs_amp[channelName]->Write();
    p_dt_MCP2_ampRatioCorr_vs_amp[channelName]->Write();

  }

  outfile->Close();
  return 0;

}



// ---- Initialize tree --------------------------------------------------------------------------------------
void InitTreeVars(TTree* chain1,TreeVars& treeVars)
{
  treeVars.t_amp = new float[36];
  treeVars.t_b_rms = new float[36];
  treeVars.t_time_max = new float[36];
  treeVars.t_time = new float[360];
  treeVars.t_CFD = 0;
  treeVars.t_LED = 0;
  treeVars.t_channelId = new std::map<std::string,int>;

  chain1 -> SetBranchStatus("*",0);
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

