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
  int  t_CFD;
  int  t_LED600;
  std::map<std::string,int> *t_channelId;

};
/**********************/




void InitTreeVars(TTree* chain1, TreeVars& treeVars);
bool AcceptEvent(TreeVars treeVars);
int GetReadoutGroup(std::string channelName, float configId);
int GetAmplitudeBin(float amplitude, std::string channelName, float configId);
bool VetoNeighbourChannels(std::string channelNameA, TreeVars treeVars, std::map<std::string,float>  minAmplitude, float configId);
void GetPositionFromEnergySharing(std::string channelNameA, TreeVars treeVars, std::map<std::string,float>  minAmplitude, float& x, float& y);



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
  
  std::vector<std::string> inputFiles = opts.GetOpt<std::vector<std::string> >("Input.inputFiles");

  timeChannels   = opts.GetOpt<std::vector<std::string> >("Input.timeChannels");
  ampChannels   = opts.GetOpt<std::vector<std::string> >("Input.ampChannels");
  std::string OutputFile = opts.GetOpt<std::string>("Input.OutputFile");
  float config = opts.GetOpt<float>("Input.config");


  std::vector<float> cut_ampMin = opts.GetOpt<std::vector<float> >("Cuts.ampMin");
  std::vector<float> cut_ampMax = opts.GetOpt<std::vector<float> >("Cuts.ampMax");
  std::map<std::string,float>  minAmplitude, maxAmplitude;
  for (int ich = 0; ich < cut_ampMin.size(); ich++){
    minAmplitude[ampChannels[ich]] = cut_ampMin[ich];
    maxAmplitude[ampChannels[ich]] = cut_ampMax[ich];
    std::cout << ich << "  " << ampChannels[ich].c_str() << "  " << minAmplitude[ampChannels[ich]]  << "  " <<maxAmplitude[ampChannels[ich]] <<std::endl;
  } 
  
  std::cout << "Ciao"<<std::endl;

  std::string label = opts.GetOpt<std::string>("Plots.label");

  std::cout << " Setting TDRStyle..."<<std::endl;
  //--------------------------
  // labels and canvas style
  setTDRStyle();

  
  //---------------------------
  // open input files and trees
  std::cout << "Loading tree ..." << std::endl; 
  TChain* chain1 = new TChain("digi","digi");
  for(unsigned int fileIt = 0; fileIt < inputFiles.size(); ++fileIt)
  {
    chain1 -> Add(inputFiles.at(fileIt).c_str());
  }
  std::cout << " Read " << chain1->GetEntries() << " total events in tree " << chain1->GetName() << std::endl;
  
  //--------------------------
  // set branches
  TreeVars treeVars;
  InitTreeVars(chain1,treeVars);
  
  
  //--------------------------
  // Book histograms
  const int NCHANNELS = timeChannels.size();
  
  std::map<std::string,std::map<std::string,TH2F*> >  h2_amp_nocuts_vs_neighbours;

  std::map<std::string,TH1F*>  h_amp_nocuts;
  std::map<std::string,TH1F*>  h_amp_vetoNeighbours;

  std::map<std::string,TH1F*>  h_amp;
  std::map<std::string,TH1F*>  h_time;

  std::map<std::string,TH1F*>  h_dt;
  std::map<std::string,TH2F*>  h2_dt_vs_amp;
  std::map<std::string,TProfile*>  p_dt_vs_amp;


  std::map<std::string,TH1F*>  h_effectiveSigma;

  std::map<std::string,TH1F*>  h_dtcorr;
  std::map<std::string,TH2F*>  h2_dtcorr_vs_amp;
  std::map<std::string,TProfile*>  p_dtcorr_vs_amp;

  const int nAmpBins = 5;
  std::map<std::string,TH1F*> h_amp_binAmp[nAmpBins];
  std::map<std::string,TH1F*> h_dt_binAmp[nAmpBins];
  std::map<std::string,TH1F*> h_dtcorr_binAmp[nAmpBins];
  std::map<std::string,TH1F*> h_effectiveSigma_binAmp[nAmpBins];

  TGraphErrors *g_timeResolGaus_vs_averageAmplitude = new TGraphErrors();
  g_timeResolGaus_vs_averageAmplitude->SetName("g_timeResolGaus_vs_averageAmplitude");

  TGraphErrors *g_timeResolEffSigma_vs_averageAmplitude = new TGraphErrors();
  g_timeResolEffSigma_vs_averageAmplitude->SetName("g_timeResolEffSigma_vs_averageAmplitude");
  

  std::map<std::string,TH2F*>  h2_dt_vs_x;
  std::map<std::string,TH2F*>  h2_dt_vs_y;


  for (int i = 0 ; i < NCHANNELS; i++){
    std::string channelName = timeChannels.at(i); // es: ch = MCP1, NINOMAT1, etc...
    h_amp_nocuts[channelName]  = new TH1F(Form("h_amp_nocuts_%s",channelName.c_str()),Form("h_amp_nocuts_%s",channelName.c_str()),4000,0.,4000.);
    h_amp_vetoNeighbours[channelName]  = new TH1F(Form("h_amp_vetoNeighbours_%s",channelName.c_str()),Form("h_amp_vetoNeighbours_%s",channelName.c_str()),4000,0.,4000.);
    h_amp[channelName]  = new TH1F(Form("h_amp_%s",channelName.c_str()),Form("h_amp_%s",channelName.c_str()),4000,0.,4000.);
    h_time[channelName] = new TH1F(Form("h_time_%s",channelName.c_str()),Form("h_time_%s",channelName.c_str()),2000,-100.,100.);
    h_dt[channelName]   = new TH1F(Form("h_dt_%s",channelName.c_str()),Form("h_dt_%s",channelName.c_str()),12500,0.,25.);
    h2_dt_vs_amp[channelName] = new TH2F(Form("h2_dt_vs_amp_%s",channelName.c_str()),Form("h2_dt_vs_amp_%s",channelName.c_str()),1000, 0, 4000, 2500,0.,25.);
    if (strcmp(channelName.c_str(), "NINOMAT5") == 0 && config == float(5.1)){ 
      std::cout << "ciao" <<std::endl;
      p_dt_vs_amp[channelName] = new TProfile(Form("p_dt_vs_amp_%s",channelName.c_str()),Form("p_dt_vs_amp_%s",channelName.c_str()),1000, 0, 4000,0.,25.);
      p_dtcorr_vs_amp[channelName] = new TProfile(Form("p_dtcorr_vs_amp_%s",channelName.c_str()),Form("p_dtcorr_vs_amp_%s",channelName.c_str()),1000, 0, 4000,0.,25.);
    }
    else{
      p_dt_vs_amp[channelName] = new TProfile(Form("p_dt_vs_amp_%s",channelName.c_str()),Form("p_dt_vs_amp_%s",channelName.c_str()),50, 0, 4000,0.,25.);
      p_dtcorr_vs_amp[channelName] = new TProfile(Form("p_dtcorr_vs_amp_%s",channelName.c_str()),Form("p_dtcorr_vs_amp_%s",channelName.c_str()),50, 0, 4000,0.,25.);
    }

    h_dtcorr[channelName]   = new TH1F(Form("h_dtcorr_%s",channelName.c_str()),Form("h_dtcorr_%s",channelName.c_str()),12500,0.,25.);
    h2_dtcorr_vs_amp[channelName] = new TH2F(Form("h2_dtcorr_vs_amp_%s",channelName.c_str()),Form("h2_dtcorr_vs_amp_%s",channelName.c_str()),1000, 0, 4000, 2500, 0.,25.);
    h_effectiveSigma[channelName]   = new TH1F(Form("h_effectiveSigma_%s",channelName.c_str()),Form("h_effectiveSigma_%s",channelName.c_str()),2000,0.,200.);

    h2_dt_vs_x[channelName] = new TH2F(Form("h2_dt_vs_x_%s",channelName.c_str()),Form("h2_dt_vs_x_%s",channelName.c_str()),200, -10, 10, 2500,0.,25.);
    h2_dt_vs_y[channelName] = new TH2F(Form("h2_dt_vs_y_%s",channelName.c_str()),Form("h2_dt_vs_y_%s",channelName.c_str()),200, -10, 10, 2500,0.,25.);

    for (int ibin = 0; ibin < nAmpBins ; ibin++){
      h_amp_binAmp[ibin][channelName] = new TH1F(Form("h_amp_%s_ampBin_%d",channelName.c_str(),ibin), Form("h_amp_%s_ampBin_%d",channelName.c_str(),ibin) ,1000,0.,4000.);
      h_dt_binAmp[ibin][channelName] = new TH1F(Form("h_dt_%s_ampBin_%d",channelName.c_str(),ibin), Form("h_dt_%s_ampBin_%d",channelName.c_str(),ibin), 12500,0.,25.);
      h_dtcorr_binAmp[ibin][channelName] = new TH1F(Form("h_dtcorr_%s_ampBin_%d",channelName.c_str(),ibin), Form("h_dtcorr_%s_ampBin_%d",channelName.c_str(),ibin),12500,0.,25.);
      h_effectiveSigma_binAmp[ibin][channelName] = new TH1F(Form("h_effectiveSigma_%s_ampBin_%d",channelName.c_str(),ibin),Form("h_effectiveSigma_%s_ampBin_%d",channelName.c_str(),ibin),2000,0.,200.);
    }


    for (int j=0; j < NCHANNELS; j++){
      std::string channelName2 = timeChannels.at(j);
      h2_amp_nocuts_vs_neighbours[channelName][channelName2]  = new TH2F(Form("h2_amp_nocuts_%s_vs_%s",channelName.c_str(),channelName2.c_str()),Form("h2_amp_nocuts_%s_vs_%s",channelName.c_str(),channelName2.c_str()),1000,0.,4000.,1000, 0, 4000);
      h2_amp_nocuts_vs_neighbours[channelName][channelName2]->GetXaxis()->SetTitle(Form("amplitude %s (ADC)", channelName2.c_str()));
      h2_amp_nocuts_vs_neighbours[channelName][channelName2]->GetYaxis()->SetTitle(Form("amplitude %s (ADC)", channelName.c_str()));
    }
    
  }



  int rog = 0;
  float amp  = -999.;
  float time = -999.;
  float dt   = -999.;
  float dtcorr = -999.;

  std::string channelName;
  std::string channelNameA;
  std::string channelName2;
  std::string channelNameA2;

  
  //-----------------------
  // first loop over events
  std::cout<<"First loop over events ..." <<std::endl;
  int nEntries = chain1 -> GetEntries();
  for(int entry = 0; entry < nEntries; ++entry)
  {
    if( entry%1000 == 0 ) std::cout << ">>> Reading entry " << entry << " / " << nEntries << "\r" << std::flush;
    chain1 -> GetEntry(entry);
    

    //--- MCPs amplitude and time ( MCP1 for readout group 0, MCP2 for readout group 1)
    float ampMCP[2]  =  {treeVars.t_amp[(*treeVars.t_channelId)["MCP1"]], 
			 treeVars.t_amp[(*treeVars.t_channelId)["MCP2"]]};
    float timeMCP[2] =  {treeVars.t_time[(*treeVars.t_channelId)["MCP1"]+treeVars.t_CFD], 
			 treeVars.t_time[(*treeVars.t_channelId)["MCP2"]+treeVars.t_CFD]};



    //--- loop over channels
    for (int ich = 0; ich < NCHANNELS; ich++){

      channelName  = timeChannels[ich];
      channelNameA = ampChannels[ich];
      
      // -- read amplitude and time for each channel
      amp  = treeVars.t_amp[(*treeVars.t_channelId)[channelNameA]];
      time = treeVars.t_time[(*treeVars.t_channelId)[channelName]+treeVars.t_LED600];

      if (strcmp(channelName.c_str(), "MCP1") == 0 ) {
	time = timeMCP[0];
	amp  = ampMCP[0];
      }
      if (strcmp(channelName.c_str(), "MCP2") == 0 ) {
	time = timeMCP[1];
      	amp  = ampMCP[1];
      }


      // -- plot correlaation between amplitude in channel i and j
      for (int jch=0; jch < NCHANNELS; jch++){
	channelName2 = timeChannels.at(jch);
	channelNameA2 = ampChannels.at(jch);
	h2_amp_nocuts_vs_neighbours[channelName][channelName2] ->Fill(treeVars.t_amp[(*treeVars.t_channelId)[channelNameA2]], amp );
      }


      // -- fill histograms before any selection
      h_amp_nocuts[channelName] -> Fill(amp);

      // -- veto events if amplitude compatible with coming from neighbour channels 
      if ( VetoNeighbourChannels(channelNameA,treeVars, minAmplitude, config) ) continue;

      h_amp_vetoNeighbours[channelName] -> Fill(amp);
      
      // -- select events
      if ( ampMCP[0] < minAmplitude["MCP1"] || ampMCP[0] > maxAmplitude["MCP1"]) continue;
      if ( ampMCP[1] < minAmplitude["MCP2"] || ampMCP[1] > maxAmplitude["MCP2"]) continue;
      if ( amp < minAmplitude[channelNameA]  || amp > maxAmplitude[channelNameA]) continue;
      if ( time < 0 ) continue;

      ///std::cout<< entry << "    ch = " << channelName << "   amp = " << amp << std::endl; 

      // test
      float x = 0;
      float y = 0;
      GetPositionFromEnergySharing(channelNameA, treeVars, minAmplitude, x, y);

      // -- get time difference wrt to MCP1 or MCP2 according to the readout group
      rog = GetReadoutGroup(channelName,config);
      dt = time - timeMCP[rog];
    
      // -- fill histograms
      h_amp[channelName]  -> Fill(amp);
      h_time[channelName] -> Fill(time);
      h_dt[channelName]   -> Fill(dt);
      h2_dt_vs_amp[channelName] -> Fill(amp,dt);
      p_dt_vs_amp[channelName] -> Fill(amp,dt);
      h2_dt_vs_x[channelName]->Fill(x,dt);
      h2_dt_vs_y[channelName]->Fill(y,dt);
  
    } //end loop over channels
    
  } //end loop over events
  


  // === Amplitude walk corrections
  std::map<std::string,TF1*>  fitFunc_awcorr;
  for (int i = 0 ; i < NCHANNELS; i++){
    channelName = timeChannels.at(i);
    //fitFunc_awcorr[channelName] = new TF1(Form("fitFunc_awcorr_%s",channelName.c_str()),"[0]*log([1]*x)+[2]",0.,4000.);
    //fitFunc_awcorr[channelName] -> SetParameters(-0.5,0.0000001, p_dt_vs_amp[channelName] -> GetMean(2));
    fitFunc_awcorr[channelName] = new TF1(Form("fitFunc_awcorr_%s",channelName.c_str()),"pol6",0,4000);// test
    fitFunc_awcorr[channelName] -> SetLineColor(kRed);
    //p_dt_vs_amp[channelName] -> Fit(Form("fitFunc_awcorr_%s",channelName.c_str()),"QS+","",
    //				    h_amp[channelName]->GetMean()-5.*h_amp[channelName]->GetRMS(),
    //				    h_amp[channelName]->GetMean()+7.*h_amp[channelName]->GetRMS());
    p_dt_vs_amp[channelName] -> Fit(Form("fitFunc_awcorr_%s",channelName.c_str()),"QS+");
  }


  // second loop over events -- fill histograms after applying timewalk correction
  std::cout<<"Second loop over events ..." <<std::endl;
  for(int entry = 0; entry < nEntries; ++entry)
    {
      if( entry%1000 == 0 ) std::cout << ">>> Reading entry " << entry << " / " << nEntries << "\r" << std::flush;
      chain1 -> GetEntry(entry);

      //--- MCPs amplitude and time ( MCP1 for readout group 0, MCP2 for readout group 1)
      float ampMCP[2]  =  {treeVars.t_amp[(*treeVars.t_channelId)["MCP1"]],
			   treeVars.t_amp[(*treeVars.t_channelId)["MCP2"]]};
      float timeMCP[2] =  {treeVars.t_time[(*treeVars.t_channelId)["MCP1"]+treeVars.t_CFD],
			   treeVars.t_time[(*treeVars.t_channelId)["MCP2"]+treeVars.t_CFD]};      

      // -- loop over channels
      for (int ich = 0; ich < NCHANNELS; ich++){

	channelName  = timeChannels[ich];
	channelNameA = ampChannels[ich];

	// -- read amplitude and time for each channel
	amp  = treeVars.t_amp[(*treeVars.t_channelId)[channelNameA]];
	time = treeVars.t_time[(*treeVars.t_channelId)[channelName]+treeVars.t_LED600];
	
	if (strcmp(channelName.c_str(), "MCP1") == 0 ) {
	  time = timeMCP[0];
	  amp  = ampMCP[0];
	}
	if (strcmp(channelName.c_str(), "MCP2") == 0 ) {
	  time = timeMCP[1];
	  amp  = ampMCP[1];
	}


	// -- veto events if amplitude compatible with coming from neighbour channels
	if ( VetoNeighbourChannels(channelNameA,treeVars, minAmplitude, config) ) continue;

	// -- select events
	if ( ampMCP[0] < minAmplitude["MCP1"] || ampMCP[0] > maxAmplitude["MCP1"]) continue;
	if ( ampMCP[1] < minAmplitude["MCP2"] || ampMCP[1] > maxAmplitude["MCP2"]) continue;
	if ( amp < minAmplitude[channelNameA]  || amp > maxAmplitude[channelNameA]) continue;
	if ( time < 0 ) continue;

	// -- get time difference wrt to MCP1 or MCP2 according to the readout group
	rog = GetReadoutGroup(channelName,config);
	dt = time - timeMCP[rog];
	
	// -- amplitude walk correction from fitted funtion 
	dtcorr = dt - fitFunc_awcorr[channelName]->Eval(amp) + fitFunc_awcorr[channelName]->Eval(h_amp[channelName]->GetMean()) ;  

	// -- fill histograms
	h_dtcorr[channelName]   -> Fill(dtcorr);
	h2_dtcorr_vs_amp[channelName] -> Fill(amp,dtcorr);
	p_dtcorr_vs_amp[channelName] -> Fill(amp,dtcorr);

	int bin = GetAmplitudeBin(amp,channelName, config);
	if (bin > -1){
	  h_amp_binAmp[bin][channelName] -> Fill(amp);
	  h_dt_binAmp[bin][channelName] -> Fill(dt);
	  h_dtcorr_binAmp[bin][channelName] -> Fill(dtcorr);
	}
	
      } //end loop over channels       

    }// end second loop over events
  

  // --  Get time resolution for each channel 
  std::map<std::string,TF1*>  fitFun;
  std::map<std::string,TF1*>  fitFun2[nAmpBins];
  std::map<std::string,TGraphErrors*>  g_timeResolEffSigma_vs_amp;
  std::map<std::string,TGraphErrors*>  g_timeResolGaus_vs_amp;
  float tmin = 0;
  float tmax = 0;

  int n  = 0;
  float sigmaGaus = 0;
  float resolMCP = 0.015; //ns
  float resol = 0;
  float binwidth ;
  double xl, xh, sigmaEff;
  float* vals = new float[6];
  float sigmaEff_error  = 0;

  for (int ich = 0; ich < NCHANNELS; ich++){

    channelName = timeChannels[ich];  
   
    // -- graphs for time resolution in ampl bins
    g_timeResolGaus_vs_amp[channelName] = new TGraphErrors();
    g_timeResolGaus_vs_amp[channelName]->SetName(Form("g_timeResolGaus_vs_amp_%s",channelName.c_str()));

    g_timeResolEffSigma_vs_amp[channelName] = new TGraphErrors();
    g_timeResolEffSigma_vs_amp[channelName]->SetName(Form("g_timeResolEffSigma_vs_amp_%s",channelName.c_str()));


    // -- skip MCPs and channels with no entries
    if (strcmp(channelName.c_str(), "MCP1") == 0 || strcmp(channelName.c_str(), "MCP2") == 0) continue;
    if (h_dtcorr[channelName] -> GetEntries() == 0) continue;

    // -- get the effective (68%) sigma
    FindSmallestInterval(vals,h_dtcorr[channelName],0.68,true); 
    sigmaEff = 0.5*(vals[5]-vals[4]); 
    h_effectiveSigma[channelName]->Fill(sigmaEff);
    resol = sqrt(sigmaEff*sigmaEff-resolMCP*resolMCP);
    n = g_timeResolEffSigma_vs_averageAmplitude->GetN();
    g_timeResolEffSigma_vs_averageAmplitude->SetPoint(n, h_amp[channelName]->GetMean(), resol*1000.); // ps
    g_timeResolEffSigma_vs_averageAmplitude->SetPointError(n, h_amp[channelName]->GetMeanError(), sigmaEff_error*1000.); //ps 

    // -- get the gaussian sigma
    h_dt[channelName]->Rebin(5);
    h_dtcorr[channelName]->Rebin(5);
    fitFun[channelName] = new TF1(Form("fitFun_%s",channelName.c_str()),"gaus",3);
    fitFun[channelName]->SetLineColor(1);
    fitFun[channelName]->SetLineWidth(1);
    fitFun[channelName]->SetParameter(1,h_dtcorr[channelName]->GetMean());
    fitFun[channelName]->SetParameter(2,h_dtcorr[channelName]->GetRMS());
    tmin = h_dtcorr[channelName]->GetMean()-2.0*h_dtcorr[channelName]->GetRMS();
    tmax = h_dtcorr[channelName]->GetMean()+2.0*h_dtcorr[channelName]->GetRMS();
    h_dtcorr[channelName]->Fit(Form("fitFun_%s",channelName.c_str()),"QS+","", tmin, tmax);
    
    sigmaGaus = fitFun[channelName]->GetParameter(2);
    resol = sqrt(sigmaGaus*sigmaGaus-resolMCP*resolMCP);
    n = g_timeResolGaus_vs_averageAmplitude->GetN();
    g_timeResolGaus_vs_averageAmplitude->SetPoint(n, h_amp[channelName]->GetMean(), resol*1000.); // ps
    g_timeResolGaus_vs_averageAmplitude->SetPointError(n, h_amp[channelName]->GetMeanError(), fitFun[channelName]->GetParError(2)*1000.); //ps 

    std::cout << "  sigma_gaus = " << fitFun[channelName]->GetParameter(2) 
	      << "  sigma_eff = " << sigmaEff 
	      << "  RMS = " << h_dtcorr[channelName]->GetRMS() << std::endl;


    // -- loop over amplitude bins
    for (int ibin = 0; ibin < nAmpBins; ibin++){

      if (h_dtcorr_binAmp[ibin][channelName] -> GetEntries() == 0) continue;

      //binwidth = h_amp_binAmp[ibin][channelName]->GetRMS(); 
      binwidth = h_amp_binAmp[ibin][channelName]->GetMeanError(); 
  
      // -- get the effective (68%) sigma           
      FindSmallestInterval(vals,h_dtcorr_binAmp[ibin][channelName],0.68,true);
      sigmaEff = 0.5*(vals[5]-vals[4]);    
      h_effectiveSigma_binAmp[ibin][channelName]->Fill(sigmaEff);  
      resol = sqrt(sigmaEff*sigmaEff-resolMCP*resolMCP);
      n = g_timeResolEffSigma_vs_amp[channelName] -> GetN();
      g_timeResolEffSigma_vs_amp[channelName] -> SetPoint(n, h_amp_binAmp[ibin][channelName]->GetMean(), resol*1000); // ps   
      g_timeResolEffSigma_vs_amp[channelName]->SetPointError(n, binwidth,sigmaEff_error*1000); //ps

      // -- get the gaussian sigma  
      h_dtcorr_binAmp[ibin][channelName]->Rebin(5); 
      fitFun2[ibin][channelName] = new TF1(Form("fitFun2_%s_%d",channelName.c_str(),ibin),"gaus",3);
      fitFun2[ibin][channelName]->SetLineColor(2);
      fitFun2[ibin][channelName]->SetLineWidth(1);
      fitFun2[ibin][channelName]->SetParameter(1,h_dtcorr_binAmp[ibin][channelName]->GetMean());
      fitFun2[ibin][channelName]->SetParameter(2,h_dtcorr_binAmp[ibin][channelName]->GetRMS());
      tmin = h_dtcorr_binAmp[ibin][channelName]->GetMean()-2.0*h_dtcorr_binAmp[ibin][channelName]->GetRMS();
      tmax = h_dtcorr_binAmp[ibin][channelName]->GetMean()+2.0*h_dtcorr_binAmp[ibin][channelName]->GetRMS();
      h_dtcorr_binAmp[ibin][channelName]->Fit(Form("fitFun2_%s_%d", channelName.c_str(),ibin),"QS+","", tmin, tmax);

      sigmaGaus = fitFun2[ibin][channelName]->GetParameter(2);
      resol = sqrt(sigmaGaus*sigmaGaus-resolMCP*resolMCP);
      n = g_timeResolGaus_vs_amp[channelName] -> GetN();
      g_timeResolGaus_vs_amp[channelName]->SetPoint(n, h_amp_binAmp[ibin][channelName]->GetMean(), resol*1000); // ps
      g_timeResolGaus_vs_amp[channelName]->SetPointError(n, binwidth,fitFun2[ibin][channelName]->GetParError(2)*1000); //ps

    }
    
  }
  
  std::cout << "Saving histograms in file " << OutputFile.c_str() <<std::endl;

  // -- Save histograms on file
  TFile *outfile = new TFile(OutputFile.c_str(),"recreate");
  
  TDirectory *outdir = outfile->mkdir(label.c_str());
  outdir->cd();
  
  for (int ich = 0; ich < NCHANNELS; ich++){
    channelName = timeChannels[ich];
    h_amp_nocuts[channelName]->Write();
    h_amp_vetoNeighbours[channelName]->Write();
    h_amp[channelName]->Write();
    h_time[channelName]->Write();
    h_dt[channelName]->Write();
    h2_dt_vs_amp[channelName]->Write();
    p_dt_vs_amp[channelName]->Write();
    h_dtcorr[channelName]->Write();
    h2_dtcorr_vs_amp[channelName]->Write();
    p_dtcorr_vs_amp[channelName]->Write();

    h2_dt_vs_x[channelName]->Write();
    h2_dt_vs_y[channelName]->Write();
  }
 
  for (int ich = 0; ich < NCHANNELS; ich++){
    channelName = timeChannels[ich];
    if (strcmp(channelName.c_str(), "MCP1") == 0 || strcmp(channelName.c_str(), "MCP2") == 0) continue;
    
    for (int ibin = 0; ibin < nAmpBins; ibin++){
      h_amp_binAmp[ibin][channelName]->Write();
      h_dt_binAmp[ibin][channelName]->Write();
      h_dtcorr_binAmp[ibin][channelName]->Write();
      //h_effectiveSigma_binAmp[ibin][channelName]->Write();
    }
       
    h_effectiveSigma[channelName]->Write();
    g_timeResolGaus_vs_amp[channelName]-> Write();
    g_timeResolEffSigma_vs_amp[channelName]-> Write();
    
  }


  /*  for (int ich = 0; ich < NCHANNELS; ich++){
    channelName = timeChannels[ich];
    for (int jch=0; jch < NCHANNELS; jch++){
      std::string channelName2 = timeChannels.at(jch);
      h2_amp_nocuts_vs_neighbours[channelName][channelName2] ->Write();
    }
    }*/

  g_timeResolGaus_vs_averageAmplitude-> Write();
  g_timeResolEffSigma_vs_averageAmplitude-> Write();

  
  outfile->Close();
  return 0;
}



bool AcceptEvent(TreeVars treeVars)
{
  return true;
}

int GetReadoutGroup(std::string channelName, float configId)
{
  int rog = 0;
  //std::cout << "config = " << configId <<std::endl;
  if (configId == float(2.1) || configId == float(2.2) || configId == float(5.1) ){ // e' senza senso che debba mettere float(2.x)
    if (strcmp(channelName.c_str(), "MCP2") == 0 || 
	strcmp(channelName.c_str(), "NINOMAT1") == 0 ||
	strcmp(channelName.c_str(), "NINOMAT2") == 0 ||
	strcmp(channelName.c_str(), "NINOMAT7") == 0 ||
	strcmp(channelName.c_str(), "NINOMAT9") == 0 ||
	strcmp(channelName.c_str(), "NINOMAT10") == 0 ||
	strcmp(channelName.c_str(), "NINOMAT11") == 0 ||
	strcmp(channelName.c_str(), "NINOMAT12") == 0 ){
      rog =1;
    }
  }
  //std::cout << "channelName = " << channelName.c_str() << "  rog = " << rog << std::endl; 

  //Pizzi matrix
  if (configId == float(7.1)){
    if (strcmp(channelName.c_str(), "MCP2") == 0 ||
        strcmp(channelName.c_str(), "NINOMAT2") == 0 ||
        strcmp(channelName.c_str(), "NINOMAT3") == 0 ||
        strcmp(channelName.c_str(), "NINOMAT7") == 0 ||
        strcmp(channelName.c_str(), "NINOMAT11") == 0 ||
        strcmp(channelName.c_str(), "NINOMAT12") == 0 ||
        strcmp(channelName.c_str(), "NINOMAT16") == 0 )
      rog =1;
  }

  
  return rog;
}


int GetAmplitudeBin(float amplitude, std::string channelName, float configId){
  int bin = -1;
  if (configId == float(2.2)){
    if (amplitude >= 1400 && amplitude < 1600) bin = 0;
    if (amplitude >= 1600 && amplitude < 1700) bin = 1;
    if (amplitude >= 1700 && amplitude < 1800) bin = 2;
    if (amplitude >= 1800 && amplitude < 2000) bin = 3;
    if (amplitude >= 2000 && amplitude < 3500) bin = 4;
    
    // channels with 25um SiPM pixels have lower amplitude
    if  ( strcmp(channelName.c_str(), "NINOMAT1") == 0 ||
	  strcmp(channelName.c_str(), "NINOMAT2") == 0 ||
	  strcmp(channelName.c_str(), "NINOMAT15") == 0 ||
	  strcmp(channelName.c_str(), "NINOMAT16") == 0){
      if (amplitude >= 200 && amplitude < 300) bin = 0;
      if (amplitude >= 300 && amplitude < 400) bin = 1;
      if (amplitude >= 400 && amplitude < 500) bin = 2;
      if (amplitude >= 500 && amplitude < 600) bin = 3;
      if (amplitude >= 600 && amplitude < 3500) bin = 4;
    }
  }

  if (configId == float(2.1)){
    if (amplitude >= 600 && amplitude < 800) bin = 0;
    if (amplitude >= 800 && amplitude < 900) bin = 1;
    if (amplitude >= 900 && amplitude < 1000) bin = 2;
    if (amplitude >= 1000 && amplitude < 1200) bin = 3;
    if (amplitude >= 1200 && amplitude < 3500) bin = 4;

    // channels with 25um SiPM pixels have lower amplitude                                                                                      
    if  ( strcmp(channelName.c_str(), "NINOMAT1") == 0 ||
	  strcmp(channelName.c_str(), "NINOMAT2") == 0 ||
	  strcmp(channelName.c_str(), "NINOMAT15") == 0 ||
          strcmp(channelName.c_str(), "NINOMAT16") == 0){
      if (amplitude >= 100 && amplitude < 200) bin = 0;
      if (amplitude >= 200 && amplitude < 300) bin = 1;
      if (amplitude >= 300 && amplitude < 400) bin = 2;
      if (amplitude >= 400 && amplitude < 500) bin = 3;
      if (amplitude >= 500 && amplitude < 3500) bin = 4;
    }
  }

  if (configId == float(5.1)){
    bin = 0;
  }

  return bin;
}


bool VetoNeighbourChannels(std::string channelNameA, TreeVars treeVars, std::map<std::string,float>  minAmplitude, float configId){

  bool veto = false;

  if ( configId == float(5.1) ) { 
 
    if (channelNameA == "AMPMAT5"){ // ch5 --> neighbours are ch6 and ch9
      //if (treeVars.t_amp[(*treeVars.t_channelId)["AMPMAT6"]] > minAmplitude["AMPMAT6"] && !(treeVars.t_amp[(*treeVars.t_channelId)["AMPMAT10"]] > minAmplitude["AMPMAT10"]) ) veto = true;
      //if (treeVars.t_amp[(*treeVars.t_channelId)["AMPMAT9"]] > minAmplitude["AMPMAT9"]) veto = true;
      if (treeVars.t_amp[(*treeVars.t_channelId)["AMPMAT6"]] > 10 ) veto = true;
      if (treeVars.t_amp[(*treeVars.t_channelId)["AMPMAT9"]] > 10 ) veto = true;
    }
    
    if (channelNameA == "AMPMAT6"){ // ch6 --> neighbours are ch5 and ch10
      //if (treeVars.t_amp[(*treeVars.t_channelId)["AMPMAT5"]] > minAmplitude["AMPMAT5"]) veto = true;
      if (treeVars.t_amp[(*treeVars.t_channelId)["AMPMAT10"]] > minAmplitude["AMPMAT10"]) veto = true;
    }
    
    if (channelNameA == "AMPMAT9"){ // ch9 --> neighbours are ch5, ch10, ch13
      //if (treeVars.t_amp[(*treeVars.t_channelId)["AMPMAT5"]] > minAmplitude["AMPMAT5"]) veto = true;
      if (treeVars.t_amp[(*treeVars.t_channelId)["AMPMAT10"]] > minAmplitude["AMPMAT10"]) veto = true;
      if (treeVars.t_amp[(*treeVars.t_channelId)["AMPMAT13"]] > minAmplitude["AMPMAT13"]) veto = true;
    }
    
    if (channelNameA == "AMPMAT10"){ // ch10 --> neighbours are ch6, ch9, ch14
      //if (treeVars.t_amp[(*treeVars.t_channelId)["AMPMAT6"]] > minAmplitude["AMPMAT6"]) veto = true;
      if (treeVars.t_amp[(*treeVars.t_channelId)["AMPMAT9"]] > minAmplitude["AMPMAT9"]) veto = true;
      //if (treeVars.t_amp[(*treeVars.t_channelId)["AMPMAT14"]] > minAmplitude["AMPMAT14"]) veto = true;
    }
    
    if (channelNameA == "AMPMAT13"){ // ch13 --> neighbours are ch9, ch14
      if (treeVars.t_amp[(*treeVars.t_channelId)["AMPMAT9"]] > minAmplitude["AMPMAT9"]) veto = true;
      //if (treeVars.t_amp[(*treeVars.t_channelId)["AMPMAT14"]] > minAmplitude["AMPMAT14"]) veto = true;
    }
    
    if (channelNameA == "AMPMAT14"){ // ch14 --> neighbours are ch10, ch13
      if (treeVars.t_amp[(*treeVars.t_channelId)["AMPMAT10"]] > minAmplitude["AMPMAT10"]) veto = true;
      if (treeVars.t_amp[(*treeVars.t_channelId)["AMPMAT13"]] > minAmplitude["AMPMAT13"]) veto = true;
    }
  }


  // -- config 7.1: veto in ALL other channels
  if (configId == float(7.1)){
    for (int ich = 0; ich < 16; ich++){
      std::string chName = Form("AMPMAT%d",ich+1);
      if (chName ==  "AMPMAT15"  ) continue;
      if (chName == channelNameA ) continue;
      if (treeVars.t_amp[(*treeVars.t_channelId)[chName]] > minAmplitude[chName] ) {
	veto = true;
	break;
      }
    }
  }

  return veto;

}


void GetPositionFromEnergySharing(std::string channelNameA, TreeVars treeVars, std::map<std::string,float>  minAmplitude, float& x, float& y)
{
  float xx = 0;
  float yy = 0;

  float a0  = 0.;
  float axp = 0.;
  float axm = 0.;
  float ayp = 0.;
  float aym = 0.;
  float dx = 11.; //mm
  float dy = 11.; //mm
  float x0 = 0;
  float y0 = 0;

  std::string newstring = channelNameA.erase(0, 6);
  int ch = atoi(newstring.c_str());

  a0  = treeVars.t_amp[(*treeVars.t_channelId)[Form("AMPMAT%d",ch)]];
  if (     ch%4 !=0 ) axp = treeVars.t_amp[(*treeVars.t_channelId)[Form("AMPMAT%d",ch+1)]];
  if ( (ch-1)%4 !=0 ) axm = treeVars.t_amp[(*treeVars.t_channelId)[Form("AMPMAT%d",ch-1)]];
  if ( (ch-4) > 0  ) ayp = treeVars.t_amp[(*treeVars.t_channelId)[Form("AMPMAT%d",ch-4)]];
  if ( (ch+4) < 16 ) aym = treeVars.t_amp[(*treeVars.t_channelId)[Form("AMPMAT%d",ch+4)]];

  
  xx = (x0 * a0 + (x0+dx) * axp + (x0-dx) * axm)/(a0+axp+axm);
  yy = (y0 * a0 + (y0+dy) * ayp + (y0-dy) * aym)/(a0+ayp+aym);

  x = xx;
  y = yy;

}



void InitTreeVars(TTree* chain1,TreeVars& treeVars)
{
  treeVars.t_amp = new float[36];
  treeVars.t_time = new float[360];
  treeVars.t_CFD = 0;
  treeVars.t_LED600 = 0;
  treeVars.t_channelId = new std::map<std::string,int>;

  chain1 -> SetBranchStatus("*",0);
  chain1 -> SetBranchStatus("amp_max",       1); chain1 -> SetBranchAddress("amp_max",      treeVars.t_amp);
  chain1 -> SetBranchStatus("time",          1); chain1 -> SetBranchAddress("time",         treeVars.t_time);
  chain1 -> SetBranchStatus("CFD",           1); chain1 -> SetBranchAddress("CFD",         &treeVars.t_CFD);
  chain1 -> SetBranchStatus("LED600",        1); chain1 -> SetBranchAddress("LED600",      &treeVars.t_LED600);

  for(unsigned int it = 0; it < timeChannels.size(); ++it)
    {
      std::string channelName = timeChannels.at(it);
      chain1 -> SetBranchStatus(channelName.c_str(),1); chain1 -> SetBranchAddress(channelName.c_str(),&((*treeVars.t_channelId)[channelName]));
    }

  for(unsigned int it = 0; it < ampChannels.size(); ++it)
    {
      std::string channelName = ampChannels.at(it);
      chain1 -> SetBranchStatus(channelName.c_str(),1); chain1 -> SetBranchAddress(channelName.c_str(),&((*treeVars.t_channelId)[channelName]));
      //std::cout<< channelName.c_str() << "  " <<(*treeVars.t_channelId)[channelName]<<std::endl; 
    }

}
