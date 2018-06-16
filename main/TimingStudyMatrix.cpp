//#include "interface/FitUtils.h"
#include "interface/SetTDRStyle.h"

#include "CfgManager/interface/CfgManager.h"
#include "CfgManager/interface/CfgManagerT.h"

#include <iostream>
#include <vector>
#include <map>
#include <algorithm>
#include <cstdlib>
#include <string>


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
  float t_beamX;
  float t_beamY;
  uint t_run;
  float t_Vbias_matrix;
  float t_NINOthr_matrix;

  float* t_amp;
  float* t_dur;
  float* t_time;
  int  t_CFD;
  int  t_LED600;
  std::map<std::string,int> *t_channelId;

};

void InitTreeVars(TTree* chain1, TTree* chain2, TTree* chain3, TreeVars& treeVars);

bool AcceptEvent(TreeVars treeVars);
int GetReadoutGroup(std::string channelName);
int GetAmplitudeBin(float amplitude);

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

  std::vector<float> cut_ampMin = opts.GetOpt<std::vector<float> >("Cuts.ampMin");
  std::vector<float> cut_ampMax = opts.GetOpt<std::vector<float> >("Cuts.ampMax");
  
  std::string label = opts.GetOpt<std::string>("Plots.label");
  
  //------------------------
  // labels and canvas style
  setTDRStyle();
  
  /*
  std::string baseDir(Form("/afs/cern.ch/user/m/malberti/www/PrecisionTiming/TBatT9May2017/config%02d/",configuration));
  system(Form("mkdir -p %s",baseDir.c_str()));
  system(Form("cp /afs/cern.ch/user/m/malberti/www/index.php %s",baseDir.c_str()));
  std::string plotDir(Form("/afs/cern.ch/user/m/malberti/www/PrecisionTiming/TBatT9May2017/config%02d/%s/",configuration,label1.c_str()));
  system(Form("mkdir %s",plotDir.c_str()));
  system(Form("cp /afs/cern.ch/user/m/malberti/www/index.php %s",plotDir.c_str()));
  */  
  

  //---------------------------
  // open input files and trees
  TChain* chain1 = new TChain("info","info");
  TChain* chain2 = new TChain("digi","digi");
  TChain* chain3 = new TChain("h4","h4");
  for(unsigned int fileIt = 0; fileIt < inputFiles.size(); ++fileIt)
  {
    chain1 -> Add(inputFiles.at(fileIt).c_str());
    chain2 -> Add(inputFiles.at(fileIt).c_str());
    chain3 -> Add(inputFiles.at(fileIt).c_str());
  }
  std::cout << " Read " << chain1->GetEntries() << " total events in tree " << chain1->GetName() << std::endl;
  std::cout << " Read " << chain2->GetEntries() << " total events in tree " << chain2->GetName() << std::endl;
  std::cout << " Read " << chain3->GetEntries() << " total events in tree " << chain3->GetName() << std::endl;
  
  // set branches
  TreeVars treeVars;
  InitTreeVars(chain1,chain2,chain3,treeVars);
  
  
  //------------------
  // Book histograms
  const int NCHANNELS = timeChannels.size();
  
  std::map<std::string,TH1F*>  h_amp_nocuts;
  std::map<std::string,TH1F*>  h_dur_nocuts;

  std::map<std::string,TH1F*>  h_amp;
  std::map<std::string,TH1F*>  h_time;

  std::map<std::string,TH1F*>  h_dt;
  std::map<std::string,TH2F*>  h2_dt_vs_amp;
  std::map<std::string,TProfile*>  p_dt_vs_amp;

  std::map<std::string,TH1F*>  h_dtcorr;
  std::map<std::string,TH2F*>  h2_dtcorr_vs_amp;
  std::map<std::string,TProfile*>  p_dtcorr_vs_amp;

  const int nAmpBins = 5;
  float ampBins[nAmpBins+1] = {1500, 1600, 1700, 1800, 2000, 3500};
  std::map<std::string,TH1F*> h_amp_binAmp[nAmpBins];
  std::map<std::string,TH1F*> h_dt_binAmp[nAmpBins];
  std::map<std::string,TH1F*> h_dtcorr_binAmp[nAmpBins];


  TH2F *h2_dt_vs_x = new TH2F("h2_dt_vs_x","h2_dt_vs_x",120, -24, 24, 1000, -25,25);
  TH2F *h2_dt_vs_y = new TH2F("h2_dt_vs_y","h2_dt_vs_y",120, -24, 24, 1000, -25,25);

  for (int i = 0 ; i < NCHANNELS; i++){
    std::string ch = timeChannels.at(i); // es: ch = MCP1, NINOMAT1, etc...
    h_dur_nocuts[ch]  = new TH1F(Form("h_dur_nocuts_%s",ch.c_str()),Form("h_dur_nocuts_%s",ch.c_str()),1000,0.,4000.);
    h_amp_nocuts[ch]  = new TH1F(Form("h_amp_nocuts_%s",ch.c_str()),Form("h_amp_nocuts_%s",ch.c_str()),1000,0.,4000.);
    h_amp[ch]  = new TH1F(Form("h_amp_%s",ch.c_str()),Form("h_amp_%s",ch.c_str()),1000,0.,4000.);
    h_time[ch] = new TH1F(Form("h_time_%s",ch.c_str()),Form("h_time_%s",ch.c_str()),2000,-100.,100.);
    h_dt[ch]   = new TH1F(Form("h_dt_%s",ch.c_str()),Form("h_dt_%s",ch.c_str()),5000,0.,25.);
    h2_dt_vs_amp[ch] = new TH2F(Form("h2_dt_vs_amp_%s",ch.c_str()),Form("h2_dt_vs_amp_%s",ch.c_str()),1000, 0, 4000, 2500,0.,25.);
    p_dt_vs_amp[ch] = new TProfile(Form("p_dt_vs_amp_%s",ch.c_str()),Form("p_dt_vs_amp_%s",ch.c_str()),100, 0, 4000,0.,25.);
    h_dtcorr[ch]   = new TH1F(Form("h_dtcorr_%s",ch.c_str()),Form("h_dtcorr_%s",ch.c_str()),5000,0.,25.);
    h2_dtcorr_vs_amp[ch] = new TH2F(Form("h2_dtcorr_vs_amp_%s",ch.c_str()),Form("h2_dtcorr_vs_amp_%s",ch.c_str()),1000, 0, 4000, 2500, 0.,25.);
    p_dtcorr_vs_amp[ch] = new TProfile(Form("p_dtcorr_vs_amp_%s",ch.c_str()),Form("p_dtcorr_vs_amp_%s",ch.c_str()),100, 0, 4000,0.,25.);
 
    for (int ibin = 0; ibin < nAmpBins ; ibin++){
      h_amp_binAmp[ibin][ch] = new TH1F(Form("h_amp_%s_ampBin_%d",ch.c_str(),ibin), Form("h_amp_%s_ampBin_%d",ch.c_str(),ibin) ,1000,0.,4000.);
      h_dt_binAmp[ibin][ch] = new TH1F(Form("h_dt_%s_ampBin_%d",ch.c_str(),ibin), Form("h_dt_%s_ampBin_%d",ch.c_str(),ibin), 5000,0.,25.);
      h_dtcorr_binAmp[ibin][ch] = new TH1F(Form("h_dtcorr_%s_ampBin_%d",ch.c_str(),ibin), Form("h_dtcorr_%s_ampBin_%d",ch.c_str(),ibin),5000,0.,25.);
    }
  }

  int rog = 0;
  float amp  = -999.;
  float time = -999.;
  float dur  = -999.; 
  float dt   = -999.;
  float dtcorr = -999.;

  std::string channelName;
  std::string channelNameA;

  float x0 = 0.;
  float y0 = 0.;
  float cell_size = 12.;
  std::map<std::string,float> xx;
  std::map<std::string,float> yy;
  xx["NINOMAT10"] = x0 - cell_size/2;
  xx["NINOMAT6"]  = x0 - cell_size/2;
  xx["NINOMAT9"]  = x0 - 3*cell_size/2;
  xx["NINOMAT11"] = x0 + cell_size/2;
  xx["NINOMAT14"] = x0 - cell_size/2;
  yy["NINOMAT10"] = y0 - cell_size/2;
  yy["NINOMAT6"]  = y0 + cell_size/2;
  yy["NINOMAT9"]  = y0 - cell_size/2;
  yy["NINOMAT11"] = y0 - cell_size/2;
  yy["NINOMAT14"] = y0 - 3*cell_size/2;
  

  //-----------------------
  // first loop over events
  std::cout<<"First loop over events ..." <<std::endl;
  int nEntries = chain2 -> GetEntries();
  for(int entry = 0; entry < nEntries; ++entry)
  {
    if( entry%1000 == 0 ) std::cout << ">>> Reading entry " << entry << " / " << nEntries << "\r" << std::flush;
    chain2 -> GetEntry(entry);
        
    //if( !AcceptEvent(treeVars) ) continue;
    
    //float Vbias = treeVars.t_Vbias_matrix;
    //std::cout << "Vbias = " << Vbias <<std::endl;
    
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
      dur  = treeVars.t_dur[(*treeVars.t_channelId)[channelName]];
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

      //std::cout<< ich << "  " << channelName << "   " << channelNameA << "  " << time << std::endl;
      

      
      h_amp_nocuts[channelName] -> Fill(amp);
      h_dur_nocuts[channelName]-> Fill(dur);

      // -- select events
      if ( ampMCP[0] < cut_ampMin[0] || ampMCP[0] > cut_ampMax[0]) continue;
      if ( ampMCP[1] < cut_ampMin[1] || ampMCP[1] > cut_ampMax[1]) continue;
      if ( amp < cut_ampMin[ich]     || amp > cut_ampMax[ich]) continue;

      rog = GetReadoutGroup(channelName);
      dt = time - timeMCP[rog];
      
      // attempt to get the position for some channels (eg NINOMAT10)
      if (strcmp(channelName.c_str(), "NINOMAT10")  == 0 && 
	  (treeVars.t_amp[(*treeVars.t_channelId)["AMPMAT6"]]>600 || treeVars.t_amp[(*treeVars.t_channelId)["AMPMAT9"]]>600 || 
	   treeVars.t_amp[(*treeVars.t_channelId)["AMPMAT11"]]>600 || treeVars.t_amp[(*treeVars.t_channelId)["AMPMAT14"]]>600 )
	  ){
	//	std::cout << treeVars.t_amp[(*treeVars.t_channelId)["AMPMAT10"]] << "  "
	//	  << treeVars.t_amp[(*treeVars.t_channelId)["AMPMAT6"]] << "  "
	//		  << treeVars.t_amp[(*treeVars.t_channelId)["AMPMAT9"]] << "  "
	//	  << treeVars.t_amp[(*treeVars.t_channelId)["AMPMAT11"]] << "  "
	//	  << treeVars.t_amp[(*treeVars.t_channelId)["AMPMAT14"]] << "  "
	//		  <<std::endl;
        float ampSum = treeVars.t_amp[(*treeVars.t_channelId)["AMPMAT10"]]+
          treeVars.t_amp[(*treeVars.t_channelId)["AMPMAT6"]]+
          treeVars.t_amp[(*treeVars.t_channelId)["AMPMAT9"]]+
          treeVars.t_amp[(*treeVars.t_channelId)["AMPMAT11"]]+
	  treeVars.t_amp[(*treeVars.t_channelId)["AMPMAT14"]];
        float x = 1./ampSum*(xx["NINOMAT10"]*treeVars.t_amp[(*treeVars.t_channelId)["AMPMAT10"]]+
                             xx["NINOMAT6"]*treeVars.t_amp[(*treeVars.t_channelId)["AMPMAT6"]]+
                             xx["NINOMAT9"]*treeVars.t_amp[(*treeVars.t_channelId)["AMPMAT9"]]+
                             xx["NINOMAT11"]*treeVars.t_amp[(*treeVars.t_channelId)["AMPMAT11"]]+
                             xx["NINOMAT14"]*treeVars.t_amp[(*treeVars.t_channelId)["AMPMAT14"]]);

        float y = 1./ampSum*(yy["NINOMAT10"]*treeVars.t_amp[(*treeVars.t_channelId)["AMPMAT10"]]+
                             yy["NINOMAT6"]*treeVars.t_amp[(*treeVars.t_channelId)["AMPMAT6"]]+
                             yy["NINOMAT9"]*treeVars.t_amp[(*treeVars.t_channelId)["AMPMAT9"]]+
                             yy["NINOMAT11"]*treeVars.t_amp[(*treeVars.t_channelId)["AMPMAT11"]]+
                             yy["NINOMAT14"]*treeVars.t_amp[(*treeVars.t_channelId)["AMPMAT14"]]);
        //std::cout<< x << "  " << y << "  " << dt <<std::endl;                                                                                                 	
        h2_dt_vs_x->Fill(x,dt);
        h2_dt_vs_y->Fill(y,dt);
      }

  
      // -- fill histograms
      h_amp[channelName]  -> Fill(amp);
      h_time[channelName] -> Fill(time);
      h_dt[channelName]   -> Fill(dt);
      h2_dt_vs_amp[channelName] -> Fill(amp,dt);
      p_dt_vs_amp[channelName] -> Fill(amp,dt);
    } //end loop over channels

  } //end loop over events
 


  // === Amplitude walk corrections
  std::map<std::string,TF1*>  fitFunc_awcorr;
  for (int i = 0 ; i < NCHANNELS; i++){
    channelName = timeChannels.at(i);
    //fitFunc_awcorr[channelName] = new TF1(Form("fitFunc_awcorr_%s",channelName.c_str()),"[0]*log([1]*x)+[2]",0.,4000.);
    //fitFunc_awcorr[channelName] -> SetParameters(-0.5,0.0000001, p_dt_vs_amp[channelName] -> GetMean(2));
    fitFunc_awcorr[channelName] = new TF1(Form("fitFunc_awcorr_%s",channelName.c_str()),"pol4",0,4000);// test
    fitFunc_awcorr[channelName] -> SetLineColor(kRed);
    p_dt_vs_amp[channelName] -> Fit(Form("fitFunc_awcorr_%s",channelName.c_str()),"QS+","",
				    h_amp[channelName]->GetMean()-5.*h_amp[channelName]->GetRMS(),
				    h_amp[channelName]->GetMean()+7.*h_amp[channelName]->GetRMS());
  }


  // second loop over events -- fill histograms after applying timewalk correction
  std::cout<<"Second loop over events ..." <<std::endl;
  for(int entry = 0; entry < nEntries; ++entry)
    {
      if( entry%1000 == 0 ) std::cout << ">>> Reading entry " << entry << " / " << nEntries << "\r" << std::flush;
      chain2 -> GetEntry(entry);

      //if( !AcceptEvent(treeVars) ) continue;

      //--- MCPs amplitude and time ( MCP1 for readout group 0, MCP2 for readout group 1)
      float ampMCP[2]  =  {treeVars.t_amp[(*treeVars.t_channelId)["MCP1"]],
			   treeVars.t_amp[(*treeVars.t_channelId)["MCP2"]]};
      float timeMCP[2] =  {treeVars.t_time[(*treeVars.t_channelId)["MCP1"]+treeVars.t_CFD],
			   treeVars.t_time[(*treeVars.t_channelId)["MCP2"]+treeVars.t_CFD]};      

      for (int ich = 0; ich < NCHANNELS; ich++){
	channelName  = timeChannels[ich];
	channelNameA = ampChannels[ich];

	// -- read amplitude and time for each channel
	dur  = treeVars.t_dur[(*treeVars.t_channelId)[channelName]];
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

	// -- select events
	if ( ampMCP[0] < cut_ampMin[0] || ampMCP[0] > cut_ampMax[0]) continue;
	if ( ampMCP[1] < cut_ampMin[1] || ampMCP[1] > cut_ampMax[1]) continue;
	if ( amp < cut_ampMin[ich]     || amp > cut_ampMax[ich]) continue;

	rog = GetReadoutGroup(channelName);
	dt = time - timeMCP[rog];
	dtcorr = dt - fitFunc_awcorr[channelName]->Eval(amp) + fitFunc_awcorr[channelName]->Eval(h_amp[channelName]->GetMean()) ;  

	// -- fill histograms
	h_dtcorr[channelName]   -> Fill(dtcorr);
	h2_dtcorr_vs_amp[channelName] -> Fill(amp,dtcorr);
	p_dtcorr_vs_amp[channelName] -> Fill(amp,dtcorr);

	int bin = GetAmplitudeBin(amp);
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
  std::map<std::string,TGraphErrors*>  g_resol_vs_amp;
  float tmin = 0;
  float tmax = 0;

  for (int ich = 0; ich < NCHANNELS; ich++){
    channelName = timeChannels[ich];  
   
    if (strcmp(channelName.c_str(), "MCP1") == 0 || strcmp(channelName.c_str(), "MCP2") == 0) continue;
    
    fitFun[channelName] = new TF1(Form("fitFun_%s",channelName.c_str()),"gaus",3);
    fitFun[channelName]->SetLineColor(1);
    fitFun[channelName]->SetLineWidth(1);
    fitFun[channelName]->SetParameter(1,h_dtcorr[channelName]->GetMean());
    fitFun[channelName]->SetParameter(2,h_dtcorr[channelName]->GetRMS());
    tmin = h_dtcorr[channelName]->GetMean()-2.0*h_dtcorr[channelName]->GetRMS();
    tmax = h_dtcorr[channelName]->GetMean()+2.0*h_dtcorr[channelName]->GetRMS();
    h_dtcorr[channelName]->Fit(Form("fitFun_%s",channelName.c_str()),"QS+","", tmin, tmax);
    
    g_resol_vs_amp[channelName] = new TGraphErrors();
    g_resol_vs_amp[channelName]->SetName(Form("g_resol_vs_amp_%s",channelName.c_str()));
    for (int ibin = 0; ibin < nAmpBins; ibin++){
      fitFun2[ibin][channelName] = new TF1(Form("fitFun2_%s_%d",channelName.c_str(),ibin),"gaus",3);
      fitFun2[ibin][channelName]->SetLineColor(1);
      fitFun2[ibin][channelName]->SetLineWidth(1);
      fitFun2[ibin][channelName]->SetParameter(1,h_dtcorr_binAmp[ibin][channelName]->GetMean());
      fitFun2[ibin][channelName]->SetParameter(2,h_dtcorr_binAmp[ibin][channelName]->GetRMS());
      tmin = h_dtcorr_binAmp[ibin][channelName]->GetMean()-2.0*h_dtcorr_binAmp[ibin][channelName]->GetRMS();
      tmax = h_dtcorr_binAmp[ibin][channelName]->GetMean()+2.0*h_dtcorr_binAmp[ibin][channelName]->GetRMS();
      h_dtcorr_binAmp[ibin][channelName]->Fit(Form("fitFun2_%s_%d", channelName.c_str(),ibin),"QS+","", tmin, tmax);

      float sigma = fitFun2[ibin][channelName]->GetParameter(2);
      float resolMCP = 0.015;
      float resol = sqrt(sigma*sigma-resolMCP*resolMCP);
      float binwidth = h_amp_binAmp[ibin][channelName]->GetRMS(); //
      g_resol_vs_amp[channelName]->SetPoint(ibin, h_amp_binAmp[ibin][channelName]->GetMean(), resol);
      g_resol_vs_amp[channelName]->SetPointError(ibin, binwidth,fitFun2[ibin][channelName]->GetParError(2) );
    }
    
  }
  





  // -- Save histograms on file
  TFile *outfile = new TFile(OutputFile.c_str(),"recreate");
  
  TDirectory *outdir = outfile->mkdir(label.c_str());
  outdir->cd();
  
  for (int ich = 0; ich < NCHANNELS; ich++){
    channelName = timeChannels[ich];
    h_dur_nocuts[channelName]->Write();
    h_amp_nocuts[channelName]->Write();
    h_amp[channelName]->Write();
    h_time[channelName]->Write();
    h_dt[channelName]->Write();
    h2_dt_vs_amp[channelName]->Write();
    p_dt_vs_amp[channelName]->Write();
    h_dtcorr[channelName]->Write();
    h2_dtcorr_vs_amp[channelName]->Write();
    p_dtcorr_vs_amp[channelName]->Write();
  }
  
  for (int ich = 0; ich < NCHANNELS; ich++){
    channelName = timeChannels[ich];
    if (strcmp(channelName.c_str(), "MCP1") == 0 || strcmp(channelName.c_str(), "MCP2") == 0) continue;
    for (int ibin = 0; ibin < nAmpBins; ibin++){
      h_amp_binAmp[ibin][channelName]->Write();
      h_dt_binAmp[ibin][channelName]->Write();
      h_dtcorr_binAmp[ibin][channelName]->Write();
    }
    g_resol_vs_amp[channelName]-> Write();
  }


  h2_dt_vs_x->Write();
  h2_dt_vs_y->Write();
  
  outfile->Close();
  return 0;
}



bool AcceptEvent(TreeVars treeVars)
{
  return true;
}

int GetReadoutGroup(std::string channelName){
  int rog = 0;

  if (strcmp(channelName.c_str(), "MCP2") == 0 ||
      strcmp(channelName.c_str(), "NINOMAT1") == 0 ||
      strcmp(channelName.c_str(), "NINOMAT2") == 0 ||
      strcmp(channelName.c_str(), "NINOMAT7") == 0 ||
      strcmp(channelName.c_str(), "NINOMAT9") == 0 ||
      strcmp(channelName.c_str(), "NINOMAT10") == 0 ||
      strcmp(channelName.c_str(), "NINOMAT11") == 0 ||
      strcmp(channelName.c_str(), "NINOMAT12") == 0 )
    rog =1;

  return rog;
}


int GetAmplitudeBin(float amplitude){
  int bin = -1;
  if (amplitude >= 1500 && amplitude < 1600) bin = 0;
  if (amplitude >= 1600 && amplitude < 1700) bin = 1;
  if (amplitude >= 1700 && amplitude < 1800) bin = 2;
  if (amplitude >= 1800 && amplitude < 2000) bin = 3;
  if (amplitude >= 2000 && amplitude < 3000) bin = 4;
  return bin;
}

void InitTreeVars(TTree* chain1, TTree* chain2, TTree* chain3,TreeVars& treeVars)
{
  treeVars.t_Vbias_matrix = -99.;
  treeVars.t_amp = new float[36];
  treeVars.t_dur = new float[36];
  treeVars.t_time = new float[360];
  treeVars.t_CFD = 0;
  treeVars.t_LED600 = 0;
  treeVars.t_channelId = new std::map<std::string,int>;

  chain1 -> SetBranchStatus("*",0);
  chain1 -> SetBranchStatus("NINOthr_matrix",1); chain1 -> SetBranchAddress("NINOthr_matrix",&treeVars.t_NINOthr_matrix);
  chain1 -> SetBranchStatus("Vbias_matrix"  ,1); chain1 -> SetBranchAddress("Vbias_matrix", &treeVars.t_Vbias_matrix);
     
  chain2 -> SetBranchStatus("*",0);
  chain2 -> SetBranchStatus("amp_max",       1); chain2 -> SetBranchAddress("amp_max",      treeVars.t_amp);
  chain2 -> SetBranchStatus("charge_sig",    1); chain2 -> SetBranchAddress("charge_sig",   treeVars.t_dur);
  chain2 -> SetBranchStatus("time",          1); chain2 -> SetBranchAddress("time",         treeVars.t_time);
  chain2 -> SetBranchStatus("CFD",           1); chain2 -> SetBranchAddress("CFD",         &treeVars.t_CFD);
  chain2 -> SetBranchStatus("LED600",        1); chain2 -> SetBranchAddress("LED600",      &treeVars.t_LED600);

  for(unsigned int it = 0; it < timeChannels.size(); ++it)
    {
      std::string channelName = timeChannels.at(it);
      chain2 -> SetBranchStatus(channelName.c_str(),1); chain2 -> SetBranchAddress(channelName.c_str(),&((*treeVars.t_channelId)[channelName]));
    }

  for(unsigned int it = 0; it < ampChannels.size(); ++it)
    {
      std::string channelName = ampChannels.at(it);
      chain2 -> SetBranchStatus(channelName.c_str(),1); chain2 -> SetBranchAddress(channelName.c_str(),&((*treeVars.t_channelId)[channelName]));
      //std::cout<< channelName.c_str() << "  " <<(*treeVars.t_channelId)[channelName]<<std::endl; 
    }

  chain3 -> SetBranchStatus("*",0);
  chain3 -> SetBranchStatus("run"  ,1); chain3 -> SetBranchAddress("run", &treeVars.t_run);
}
