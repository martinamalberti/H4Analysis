#include "interface/CTRUtils.h"
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
#include "TF1.h"
#include "TCanvas.h"
#include "TLatex.h"
#include "TLine.h"
#include "TBox.h"
#include "TEllipse.h"
#include "TApplication.h"

bool popupPlots = false;



int main(int argc, char** argv)
{
  if( argc < 2 )
  {
    std::cerr << ">>>>> drawCTRPeacock.cpp::usage:   " << argv[0] << " configFileName" << std::endl;
    return 1;
  }
  
  
  
  //--------
  // trg map
  std::map<std::string,std::string> trgMap;
  trgMap["A1"] = "TRG3";
  trgMap["B1"] = "TRG3";
  trgMap["C1"] = "TRG1";
  trgMap["D1"] = "TRG2";
  trgMap["A2"] = "TRG3";
  trgMap["B2"] = "TRG3";
  trgMap["C2"] = "TRG2";
  trgMap["D2"] = "TRG2";
  trgMap["A3"] = "TRG3";
  trgMap["B3"] = "TRG3";
  trgMap["C3"] = "TRG2";
  trgMap["D3"] = "TRG2";
  trgMap["A4"] = "TRG3";
  trgMap["B4"] = "TRG1";
  trgMap["C4"] = "TRG2";
  trgMap["D4"] = "TRG2";
  
  
  
  //----------------------
  // parse the config file
  CfgManager opts;
  opts.ParseConfigFile(argv[1]);
  
  std::vector<std::string> inputFiles = opts.GetOpt<std::vector<std::string> >("Input.inputFiles");
  
  std::string refChannel = opts.GetOpt<std::string>("Input.refChannel");
  std::vector<std::string> channels = opts.GetOpt<std::vector<std::string> >("Input.channels");
  
  float cut_Vbias = opts.GetOpt<float>("Cuts.Vbias");
  float cut_minAmplitudeRef = opts.GetOpt<float>("Cuts.minAmplitudeRef");
  float cut_maxAmplitudeRef = opts.GetOpt<float>("Cuts.maxAmplitudeRef");
  
  std::vector<std::string> cut_channelNames = opts.GetOpt<std::vector<std::string> >("Cuts.channelNames");
  std::vector<float> cut_minAmplitudes = opts.GetOpt<std::vector<float> >("Cuts.minAmplitudes");
  std::vector<float> cut_maxAmplitudes = opts.GetOpt<std::vector<float> >("Cuts.maxAmplitudes");
  std::map<std::string,float> ampMin;
  std::map<std::string,float> ampMax;
  for(unsigned int ii = 0; ii < cut_channelNames.size(); ++ii)
  {
    ampMin[cut_channelNames.at(ii)] = cut_minAmplitudes.at(ii);
    ampMax[cut_channelNames.at(ii)] = cut_maxAmplitudes.at(ii);
  }
  
  float cut_crystalXMin = opts.GetOpt<float>("Cuts.crystalXMin");
  float cut_crystalXMax = opts.GetOpt<float>("Cuts.crystalXMax");
  float cut_crystalYMin = opts.GetOpt<float>("Cuts.crystalYMin");
  float cut_crystalYMax = opts.GetOpt<float>("Cuts.crystalYMax");
  
  float cut_MCPX = opts.GetOpt<float>("Cuts.MCPX");
  float cut_MCPY = opts.GetOpt<float>("Cuts.MCPY");
  float cut_MCPR = opts.GetOpt<float>("Cuts.MCPR");
  
  int rebin = opts.GetOpt<int>("Plots.rebin");
  std::string label1 = opts.GetOpt<std::string>("Plots.label1");
  std::string label2 = opts.GetOpt<std::string>("Plots.label2");
  std::string label12;
  if( label1 == label2 ) label12 = label1;
  else label12 = std::string(Form("%s-%s",label1.c_str(),label2.c_str()));
  
  
  
  //------------------------
  // labels and canvas style
  setTDRStyle();
  gStyle->SetPaintTextFormat("4.1f");
  gStyle->SetPadLeftMargin(0.10);
  gStyle->SetPadRightMargin(0.15);
  
  TApplication* theApp;
  if( popupPlots )
    theApp = new TApplication("App", &argc, argv);
  
  TLatex* latexLabel1 = new TLatex(0.13,0.97,Form("%s",label1.c_str()));
  TLatex* latexLabel2 = new TLatex(0.13,0.97,Form("%s",label2.c_str()));
  TLatex* latexLabel12 = new TLatex(0.13,0.97,Form("%s",label12.c_str()));
  latexLabel1 -> SetNDC();
  latexLabel1 -> SetTextFont(42);
  latexLabel1 -> SetTextSize(0.03);
  latexLabel2 -> SetNDC();
  latexLabel2 -> SetTextFont(42);
  latexLabel2 -> SetTextSize(0.03);
  latexLabel12 -> SetNDC();
  latexLabel12 -> SetTextFont(42);
  latexLabel12 -> SetTextSize(0.03);
  
  std::string baseDir(Form("/afs/cern.ch/user/a/abenagli/www/TIMING/TBatH6August2017_new/peacock/"));
  system(Form("mkdir -p %s",baseDir.c_str()));
  system(Form("cp /afs/cern.ch/user/a/abenagli/public/index.php %s",baseDir.c_str()));
  std::string plotDir(Form("%s/Vbias%.0f/",baseDir.c_str(),cut_Vbias));
  system(Form("mkdir %s",plotDir.c_str()));
  system(Form("cp /afs/cern.ch/user/a/abenagli/public/index.php %s",plotDir.c_str()));
  
  
  //---------------------------
  // open input files and trees
  TChain* chain0 = new TChain("h4","h4");
  for(unsigned int fileIt = 0; fileIt < inputFiles.size(); ++fileIt)
  {
    chain0 -> Add(inputFiles.at(fileIt).c_str());
  }
  std::cout << " Read " << chain0->GetEntries() << " total events in tree " << chain0->GetName() << std::endl;
  
  TreeVars treeVars;
  InitTreeVarsPeacock(chain0,treeVars,opts);
  
  
  //------------------
  // Define histograms
  TH1F* h_ampRef = new TH1F(Form("h_ampRef"),"",250,0.,500.);
  TProfile2D* p2_ampRef_vs_beam_Y_vs_X = new TProfile2D("p2_ampRef_vs_beam_Y_vs_X","",25,-25.,25.,25,-25.,25.);
  
  std::map<std::string,TH1F*> h_amp;
  std::map<std::string,TH1F*> h_amp_cut;
  std::map<std::string,TProfile*> p_CTR_vs_amp;
  std::map<std::string,TProfile*> p_CTR_vs_beam_X;
  std::map<std::string,TProfile*> p_CTR_vs_beam_Y;
  std::map<std::string,TProfile*> p_CTR_vs_beam_X_cut;
  std::map<std::string,TProfile*> p_CTR_vs_beam_Y_cut;
  std::map<std::string,TProfile2D*> p2_amp_vs_beam_Y_vs_X;
  std::map<std::string,TProfile2D*> p2_CTR_vs_beam_Y_vs_X;
  std::map<std::string,TProfile*> p_CTR_ampCorrEff_vs_beam_X;
  std::map<std::string,TProfile*> p_CTR_ampCorrEff_vs_beam_Y;
  std::map<std::string,TProfile*> p_CTR_ampCorrEff_vs_beam_X_cut;
  std::map<std::string,TProfile*> p_CTR_ampCorrEff_vs_beam_Y_cut;
  std::map<std::string,TProfile2D*> p2_CTR_ampCorrEff_vs_beam_Y_vs_X;
  for(unsigned int ii = 0; ii < channels.size(); ++ii)
  {
    std::string ch = channels.at(ii);
    h_amp[ch] = new TH1F(Form("h_amp_%s",ch.c_str()),"",250,0.,1000.);
    h_amp_cut[ch] = new TH1F(Form("h_amp_cut_%s",ch.c_str()),"",250,0.,1000.);
    p_CTR_vs_amp[ch] = new TProfile(Form("p_CTR_vs_amp_%s",ch.c_str()),"",100,0.,1000.);
    p_CTR_vs_beam_X[ch] = new TProfile(Form("p_CTR_vs_beam_X_%s",ch.c_str()),"",1*int(cut_crystalXMax-cut_crystalXMin),cut_crystalXMin,cut_crystalXMax);
    p_CTR_vs_beam_Y[ch] = new TProfile(Form("p_CTR_vs_beam_Y_%s",ch.c_str()),"",1*int(cut_crystalYMax-cut_crystalYMin),cut_crystalYMin,cut_crystalYMax);
    p_CTR_vs_beam_X_cut[ch] = new TProfile(Form("p_CTR_vs_beam_X_cut_%s",ch.c_str()),"",1*int(cut_crystalXMax-cut_crystalXMin),cut_crystalXMin,cut_crystalXMax);
    p_CTR_vs_beam_Y_cut[ch] = new TProfile(Form("p_CTR_vs_beam_Y_cut_%s",ch.c_str()),"",1*int(cut_crystalYMax-cut_crystalYMin),cut_crystalYMin,cut_crystalYMax);
    p2_amp_vs_beam_Y_vs_X[ch] = new TProfile2D(Form("p2_amp_vs_beam_Y_vs_X_%s",ch.c_str()),"",25,-25.,25.,25,-25.,25.);
    p2_CTR_vs_beam_Y_vs_X[ch] = new TProfile2D(Form("p2_CTR_vs_beam_Y_vs_X_%s",ch.c_str()),"",1*int(cut_crystalXMax-cut_crystalXMin),cut_crystalXMin,cut_crystalXMax,1*int(cut_crystalYMax-cut_crystalYMin),cut_crystalYMin,cut_crystalYMax);
    p_CTR_ampCorrEff_vs_beam_X[ch] = new TProfile(Form("p_CTR_ampCorrEff_vs_beam_X_%s",ch.c_str()),"",1*int(cut_crystalXMax-cut_crystalXMin),cut_crystalXMin,cut_crystalXMax);
    p_CTR_ampCorrEff_vs_beam_Y[ch] = new TProfile(Form("p_CTR_ampCorrEff_vs_beam_Y_%s",ch.c_str()),"",1*int(cut_crystalYMax-cut_crystalYMin),cut_crystalYMin,cut_crystalYMax);
    p_CTR_ampCorrEff_vs_beam_X_cut[ch] = new TProfile(Form("p_CTR_ampCorrEff_vs_beam_X_cut_%s",ch.c_str()),"",1*int(cut_crystalXMax-cut_crystalXMin),cut_crystalXMin,cut_crystalXMax);
    p_CTR_ampCorrEff_vs_beam_Y_cut[ch] = new TProfile(Form("p_CTR_ampCorrEff_vs_beam_Y_cut_%s",ch.c_str()),"",1*int(cut_crystalYMax-cut_crystalYMin),cut_crystalYMin,cut_crystalYMax);
    p2_CTR_ampCorrEff_vs_beam_Y_vs_X[ch] = new TProfile2D(Form("p2_CTR_ampCorrEff_vs_beam_Y_vs_X_%s",ch.c_str()),"",1*int(cut_crystalXMax-cut_crystalXMin),cut_crystalXMin,cut_crystalXMax,1*int(cut_crystalYMax-cut_crystalYMin),cut_crystalYMin,cut_crystalYMax);
  }
  
  TH1F* h_CTR_raw = new TH1F("h_CTR_raw","",50000,-100.,100.);
  TH1F* h_CTR_raw_ampCorrEff = new TH1F("h_CTR_raw_ampCorrEff","",50000,-100.,100.);
  
  
  //-----------------------
  // first loop over events
  int nEntries = chain0 -> GetEntries();
  int nEntries_cut = 0;
  for(int entry = 0; entry < nEntries; ++entry)
  {
    if( entry%1000 == 0 ) std::cout << ">>> loop 1/3: reading entry " << entry << " / " << nEntries << "\r" << std::flush;
    chain0 -> GetEntry(entry);
    
    {
      float ampRef = treeVars.t_amp[(*treeVars.t_channelId)[refChannel]] * 0.25;    
      if( ampRef > 0 && ampRef < 1000. )
        p2_ampRef_vs_beam_Y_vs_X -> Fill( treeVars.t_beamX[0],treeVars.t_beamY[0],ampRef );
      
      for(unsigned int ii = 0; ii < channels.size(); ++ii)
      {
        std::string ch = channels.at(ii);
        if( ch == "B3" ) continue;
        
        float amp = treeVars.t_amp[(*treeVars.t_channelId)[ch]] * 0.25;
        if( amp > 0. && amp < 1000. )
          p2_amp_vs_beam_Y_vs_X[ch] -> Fill( treeVars.t_beamX[0],treeVars.t_beamY[0],amp );
      }
    }
    
    
    // if( !AcceptEventMCP(treeVars,cut_MCPX,cut_MCPY,cut_MCPR) ) continue;
    if( treeVars.t_beamX[0] < cut_crystalXMin || treeVars.t_beamX[0] > cut_crystalXMax ) continue;
    if( treeVars.t_beamY[0] < cut_crystalYMin || treeVars.t_beamY[0] > cut_crystalYMax ) continue;
    if( treeVars.t_Vbias[0] != cut_Vbias ) continue;
    
    int extraItRef = treeVars.t_CFD;
    float ampRef = treeVars.t_amp[(*treeVars.t_channelId)[refChannel]] * 0.25;
    float timeRef = treeVars.t_time[(*treeVars.t_channelId)[refChannel]+extraItRef];
    float timeTrgRef = treeVars.t_time[(*treeVars.t_trgChannelId)["TRG1"]+treeVars.t_LED];
    h_ampRef -> Fill( ampRef );
    
    for(unsigned int ii = 0; ii < channels.size(); ++ii)
    {
      std::string ch = channels.at(ii);
      if( ch == "B3" ) continue;
      
      int extraIt = (*treeVars.t_timeTypes)["LED50"];
      float amp = treeVars.t_amp[(*treeVars.t_channelId)[ch]] * 0.25;
      float time = treeVars.t_time[(*treeVars.t_channelId)[ch]+extraIt];
      float timeTrg = treeVars.t_time[(*treeVars.t_trgChannelId)[trgMap[ch]]+treeVars.t_LED];
      float CTR = (time - timeTrg + timeTrgRef) - timeRef;
      if( CTR < 0 || CTR > 5. ) continue;
      
      h_amp[ch] -> Fill( amp );
      
      if( isnan(ampRef) ) continue;
      if( isnan(amp) ) continue;
      if( ampRef < cut_minAmplitudeRef || ampRef > cut_maxAmplitudeRef ) continue;
      if( amp < ampMin[ch] || amp > ampMax[ch] ) continue;
      
      if( isnan(timeRef) ) continue;
      if( isnan(time) ) continue;
      if( timeRef < 0. || timeRef > 200. ) continue;
      if( time < 0. || time > 200. ) continue;
      
      h_amp_cut[ch] -> Fill( amp );
      h_CTR_raw -> Fill( CTR );
      p_CTR_vs_amp[ch] -> Fill( amp,CTR );
      p_CTR_vs_beam_X[ch] -> Fill( treeVars.t_beamX[0],CTR );
      p_CTR_vs_beam_Y[ch] -> Fill( treeVars.t_beamY[0],CTR );
      if( treeVars.t_beamY[0] < cut_crystalYMax-int(ii/4)*(cut_crystalYMax-cut_crystalYMin)/4. &&
          treeVars.t_beamY[0] > cut_crystalYMax-(int(ii/4)+1)*(cut_crystalYMax-cut_crystalYMin)/4. )
        p_CTR_vs_beam_X_cut[ch] -> Fill( treeVars.t_beamX[0],CTR );
      if( treeVars.t_beamX[0] > cut_crystalXMin+(ii%4)*(cut_crystalXMax-cut_crystalXMin)/4. &&
          treeVars.t_beamX[0] < cut_crystalXMin+(ii%4+1)*(cut_crystalXMax-cut_crystalXMin)/4. )
        p_CTR_vs_beam_Y_cut[ch] -> Fill( treeVars.t_beamY[0],CTR );
      p2_CTR_vs_beam_Y_vs_X[ch] -> Fill( treeVars.t_beamX[0],treeVars.t_beamY[0],CTR );
    }
    
    ++nEntries_cut;
  }
  std::cout << std::endl;
  std::cout << ">>> Selected " << nEntries_cut << " / " << nEntries << " events" << std::endl;
  
  
  
  
  //----------------
  // find CTR ranges
  float CTRMean = -1;
  float CTRSigma = -1;
  {
    TH1F* histo = h_CTR_raw;
    float* vals = new float[6];
    FindSmallestInterval(vals,histo,0.68,true); 
    
    float mean = vals[0];
    float min = vals[4];
    float max = vals[5];
    float delta = max-min;
    float sigma = 0.5*delta;
    float effSigma = sigma;
    CTRMean = mean;
    CTRSigma = effSigma;
  }
  
  
  
  //-------------------
  // amplitude-walk fit
  std::map<std::string,TF1*> fitFunc_corrAmp;
  for(unsigned int ii = 0; ii < channels.size(); ++ii)
  {
    std::string ch = channels.at(ii);
    
    fitFunc_corrAmp[ch] = new TF1(Form("fitFunc_corrAmp_%s",ch.c_str()),"[0]*log([1]*x)+[2]",0.,1000.);
    fitFunc_corrAmp[ch] -> SetParameters(-0.8,0.000001,-3.5);
    p_CTR_vs_amp[ch] -> Fit(fitFunc_corrAmp[ch],"QNS+");
  }
  
  
  
  std::map<std::string,TH1F*> h_CTR;
  std::map<std::string,TH1F*> h_CTR_ampCorrEff;
  std::map<std::string,TH1F*> h_CTR_ampCorrEffPosCorr;
  TH1F* h_CTR_sum2 = new TH1F("h_CTR_sum2","",4000,CTRMean-10.*CTRSigma,CTRMean+10.*CTRSigma);
  TH1F* h_CTR_sum4 = new TH1F("h_CTR_sum4","",4000,CTRMean-10.*CTRSigma,CTRMean+10.*CTRSigma);
  TH1F* h_CTR_sumAll = new TH1F("h_CTR_sumAll","",4000,CTRMean-10.*CTRSigma,CTRMean+10.*CTRSigma);
  for(unsigned int ii = 0; ii < channels.size(); ++ii)
  {
    std::string ch = channels.at(ii);
    
    h_CTR[ch] = new TH1F(Form("h_CTR_%s",ch.c_str()),"",4000,CTRMean-10.*CTRSigma,CTRMean+10.*CTRSigma);
    h_CTR_ampCorrEff[ch] = new TH1F(Form("h_CTR_ampCorrEff_%s",ch.c_str()),"",4000,CTRMean-10.*CTRSigma,CTRMean+10.*CTRSigma);
    h_CTR_ampCorrEffPosCorr[ch] = new TH1F(Form("h_CTR_ampCorrEffPosCorr_%s",ch.c_str()),"",4000,CTRMean-10.*CTRSigma,CTRMean+10.*CTRSigma);
  }
  
  
  
  //------------------------
  // second loop over events
  nEntries_cut = 0;
  for(int entry = 0; entry < nEntries; ++entry)
  {
    if( entry%1000 == 0 ) std::cout << ">>> loop 2/3: reading entry " << entry << " / " << nEntries << "\r" << std::flush;
    chain0 -> GetEntry(entry);
    
    // if( !AcceptEventMCP(treeVars,cut_MCPX,cut_MCPY,cut_MCPR) ) continue;
    if( treeVars.t_beamX[0] < cut_crystalXMin || treeVars.t_beamX[0] > cut_crystalXMax ) continue;
    if( treeVars.t_beamY[0] < cut_crystalYMin || treeVars.t_beamY[0] > cut_crystalYMax ) continue;
    if( treeVars.t_Vbias[0] != cut_Vbias ) continue;
    
    int extraItRef = treeVars.t_CFD;
    float ampRef = treeVars.t_amp[(*treeVars.t_channelId)[refChannel]] * 0.25;
    float timeRef = treeVars.t_time[(*treeVars.t_channelId)[refChannel]+extraItRef];
    float timeTrgRef = treeVars.t_time[(*treeVars.t_trgChannelId)["TRG1"]+treeVars.t_LED];
    
    for(unsigned int ii = 0; ii < channels.size(); ++ii)
    {
      std::string ch = channels.at(ii);
      if( ch == "B3" ) continue;
      
      int extraIt = (*treeVars.t_timeTypes)["LED50"];
      float amp = treeVars.t_amp[(*treeVars.t_channelId)[ch]] * 0.25;
      float time = treeVars.t_time[(*treeVars.t_channelId)[ch]+extraIt];
      float timeTrg = treeVars.t_time[(*treeVars.t_trgChannelId)[trgMap[ch]]+treeVars.t_LED];
      float CTR = (time - timeTrg + timeTrgRef) - timeRef;
      float CTR_ampCorrEff = CTR - fitFunc_corrAmp[ch]->Eval(amp) + fitFunc_corrAmp[ch]->Eval(h_amp_cut[ch]->GetMean());
      if( CTR < 0 || CTR > 5. ) continue;
      if( CTR_ampCorrEff < 0 || CTR_ampCorrEff > 5. ) continue;
      
      if( isnan(ampRef) ) continue;
      if( isnan(amp) ) continue;
      if( ampRef < cut_minAmplitudeRef || ampRef > cut_maxAmplitudeRef ) continue;
      if( amp < ampMin[ch] || amp > ampMax[ch] ) continue;
      
      if( isnan(timeRef) ) continue;
      if( isnan(time) ) continue;
      if( timeRef < 0. || timeRef > 200. ) continue;
      if( time < 0. || time > 200. ) continue;
      
      h_CTR_raw_ampCorrEff -> Fill( CTR_ampCorrEff );
      h_CTR[ch] -> Fill( CTR );
      h_CTR_ampCorrEff[ch] -> Fill( CTR_ampCorrEff );
      p_CTR_ampCorrEff_vs_beam_X[ch] -> Fill( treeVars.t_beamX[0],CTR_ampCorrEff );
      p_CTR_ampCorrEff_vs_beam_Y[ch] -> Fill( treeVars.t_beamY[0],CTR_ampCorrEff );
      if( treeVars.t_beamY[0] < cut_crystalYMax-int(ii/4)*(cut_crystalYMax-cut_crystalYMin)/4. &&
          treeVars.t_beamY[0] > cut_crystalYMax-(int(ii/4)+1)*(cut_crystalYMax-cut_crystalYMin)/4. )
        p_CTR_ampCorrEff_vs_beam_X_cut[ch] -> Fill( treeVars.t_beamX[0],CTR_ampCorrEff );
      if( treeVars.t_beamX[0] > cut_crystalXMin+(ii%4)*(cut_crystalXMax-cut_crystalXMin)/4. &&
          treeVars.t_beamX[0] < cut_crystalXMin+(ii%4+1)*(cut_crystalXMax-cut_crystalXMin)/4. )
        p_CTR_ampCorrEff_vs_beam_Y_cut[ch] -> Fill( treeVars.t_beamY[0],CTR_ampCorrEff );
      p2_CTR_ampCorrEff_vs_beam_Y_vs_X[ch] -> Fill( treeVars.t_beamX[0],treeVars.t_beamY[0],CTR_ampCorrEff );
    }
    
    ++nEntries_cut;
  }
  std::cout << std::endl;
  std::cout << ">>> Selected " << nEntries_cut << " / " << nEntries << " events" << std::endl;  
  
  
  
  //----------------
  // find CTR ranges
  float CTRCorrMean = -1;
  float CTRCorrSigma = -1;
  {
    TH1F* histo = h_CTR_raw_ampCorrEff;
    float* vals = new float[6];
    FindSmallestInterval(vals,histo,0.68,true); 
    
    float mean = vals[0];
    float min = vals[4];
    float max = vals[5];
    float delta = max-min;
    float sigma = 0.5*delta;
    float effSigma = sigma;
    CTRCorrMean = mean;
    CTRCorrSigma = effSigma;
  }
  
  
  
  //-----------------------
  // third loop over events
  nEntries_cut = 0;
  for(int entry = 0; entry < nEntries; ++entry)
  {
    if( entry%1000 == 0 ) std::cout << ">>> loop 3/3: reading entry " << entry << " / " << nEntries << "\r" << std::flush;
    chain0 -> GetEntry(entry);
    
    // if( !AcceptEventMCP(treeVars,cut_MCPX,cut_MCPY,cut_MCPR) ) continue;
    if( treeVars.t_beamX[0] < cut_crystalXMin || treeVars.t_beamX[0] > cut_crystalXMax ) continue;
    if( treeVars.t_beamY[0] < cut_crystalYMin || treeVars.t_beamY[0] > cut_crystalYMax ) continue;
    if( treeVars.t_Vbias[0] != cut_Vbias ) continue;
    
    int extraItRef = treeVars.t_CFD;
    float ampRef = treeVars.t_amp[(*treeVars.t_channelId)[refChannel]] * 0.25;
    float timeRef = treeVars.t_time[(*treeVars.t_channelId)[refChannel]+extraItRef];
    float timeTrgRef = treeVars.t_time[(*treeVars.t_trgChannelId)["TRG1"]+treeVars.t_LED];
    
    for(unsigned int ii = 0; ii < channels.size(); ++ii)
    {
      std::string ch = channels.at(ii);
      if( ch == "B3" ) continue;
      
      int extraIt = (*treeVars.t_timeTypes)["LED50"];
      float amp = treeVars.t_amp[(*treeVars.t_channelId)[ch]] * 0.25;
      float time = treeVars.t_time[(*treeVars.t_channelId)[ch]+extraIt];
      float timeTrg = treeVars.t_time[(*treeVars.t_trgChannelId)[trgMap[ch]]+treeVars.t_LED];
      float CTR = (time - timeTrg + timeTrgRef) - timeRef;
      float CTR_ampCorrEffPosCorr = CTR - fitFunc_corrAmp[ch]->Eval(amp) + fitFunc_corrAmp[ch]->Eval(h_amp_cut[ch]->GetMean()) -
        p2_CTR_ampCorrEff_vs_beam_Y_vs_X[ch]->GetBinContent( p2_CTR_ampCorrEff_vs_beam_Y_vs_X[ch]->FindBin(treeVars.t_beamX[0],treeVars.t_beamY[0])) + 
        p2_CTR_ampCorrEff_vs_beam_Y_vs_X[ch]->GetBinContent( p2_CTR_ampCorrEff_vs_beam_Y_vs_X[ch]->FindBin(0.5*(cut_crystalXMin+cut_crystalXMax),0.5*(cut_crystalYMin+cut_crystalYMax)));
      
      if( CTR < 0 || CTR > 5. ) continue;
      if( CTR_ampCorrEffPosCorr < 0 || CTR_ampCorrEffPosCorr > 5. ) continue;
      
      if( isnan(ampRef) ) continue;
      if( isnan(amp) ) continue;
      if( ampRef < cut_minAmplitudeRef || ampRef > cut_maxAmplitudeRef ) continue;
      if( amp < ampMin[ch] || amp > ampMax[ch] ) continue;
      
      if( isnan(timeRef) ) continue;
      if( isnan(time) ) continue;
      if( timeRef < 0. || timeRef > 200. ) continue;
      if( time < 0. || time > 200. ) continue;
      
      h_CTR_ampCorrEffPosCorr[ch] -> Fill( CTR_ampCorrEffPosCorr );
    }
    
    ++nEntries_cut;
  }
  std::cout << std::endl;
  std::cout << ">>> Selected " << nEntries_cut << " / " << nEntries << " events" << std::endl;
  
  
  
  //------------------------
  // fourth loop over events
  nEntries_cut = 0;
  for(int entry = 0; entry < nEntries; ++entry)
  {
    if( entry%1000 == 0 ) std::cout << ">>> loop 4/4: reading entry " << entry << " / " << nEntries << "\r" << std::flush;
    chain0 -> GetEntry(entry);
    
    // if( !AcceptEventMCP(treeVars,cut_MCPX,cut_MCPY,cut_MCPR) ) continue;
    if( treeVars.t_beamX[0] < cut_crystalXMin || treeVars.t_beamX[0] > cut_crystalXMax ) continue;
    if( treeVars.t_beamY[0] < cut_crystalYMin || treeVars.t_beamY[0] > cut_crystalYMax ) continue;
    if( treeVars.t_Vbias[0] != cut_Vbias ) continue;
    
    int extraItRef = treeVars.t_CFD;
    float ampRef = treeVars.t_amp[(*treeVars.t_channelId)[refChannel]] * 0.25;
    float timeRef = treeVars.t_time[(*treeVars.t_channelId)[refChannel]+extraItRef];
    float timeTrgRef = treeVars.t_time[(*treeVars.t_trgChannelId)["TRG1"]+treeVars.t_LED];
    
    int n_sum2 = 0; float CTR_sum2 = 0.;
    int n_sum4 = 0; float CTR_sum4 = 0.;
    int n_sumAll = 0; float CTR_sumAll = 0.;
    for(unsigned int ii = 0; ii < channels.size(); ++ii)
    {
      std::string ch = channels.at(ii);
      if( ch == "B3" ) continue;
      
      int extraIt = (*treeVars.t_timeTypes)["LED50"];
      float amp = treeVars.t_amp[(*treeVars.t_channelId)[ch]] * 0.25;
      float time = treeVars.t_time[(*treeVars.t_channelId)[ch]+extraIt];
      float timeTrg = treeVars.t_time[(*treeVars.t_trgChannelId)[trgMap[ch]]+treeVars.t_LED];
      float CTR = (time - timeTrg + timeTrgRef) - timeRef;
      float CTR_ampCorrEffPosCorr = CTR - fitFunc_corrAmp[ch]->Eval(amp) + fitFunc_corrAmp[ch]->Eval(h_amp_cut[ch]->GetMean()) -
        p2_CTR_ampCorrEff_vs_beam_Y_vs_X[ch]->GetBinContent( p2_CTR_ampCorrEff_vs_beam_Y_vs_X[ch]->FindBin(treeVars.t_beamX[0],treeVars.t_beamY[0])) + 
        p2_CTR_ampCorrEff_vs_beam_Y_vs_X[ch]->GetBinContent( p2_CTR_ampCorrEff_vs_beam_Y_vs_X[ch]->FindBin(0.5*(cut_crystalXMin+cut_crystalXMax),0.5*(cut_crystalYMin+cut_crystalYMax))) -
        h_CTR_ampCorrEffPosCorr[ch]->GetMean() + h_CTR_ampCorrEffPosCorr["C1"]->GetMean();
      
      if( CTR < 0 || CTR > 5. ) continue;
      if( CTR_ampCorrEffPosCorr < 0 || CTR_ampCorrEffPosCorr > 5. ) continue;
      
      if( isnan(ampRef) ) continue;
      if( isnan(amp) ) continue;
      if( ampRef < cut_minAmplitudeRef || ampRef > cut_maxAmplitudeRef ) continue;
      if( amp < ampMin[ch] || amp > ampMax[ch] ) continue;
      
      if( isnan(timeRef) ) continue;
      if( isnan(time) ) continue;
      if( timeRef < 0. || timeRef > 200. ) continue;
      if( time < 0. || time > 200. ) continue;
      
      if( ch == "C1" || ch == "B4" ) { ++n_sum2; CTR_sum2 += CTR_ampCorrEffPosCorr; }
      if( ch == "C1" || ch == "B4" || ch == "D3" || ch == "A2" ) { ++n_sum4; CTR_sum4 += CTR_ampCorrEffPosCorr; }
      { ++n_sumAll; CTR_sumAll += CTR_ampCorrEffPosCorr; }
    }
    
    if( n_sum2 == 2 )    h_CTR_sum2   -> Fill(   CTR_sum2/2. );
    if( n_sum4 == 4 )    h_CTR_sum4   -> Fill(   CTR_sum4/4. );
    if( n_sumAll == 15 ) h_CTR_sumAll -> Fill( CTR_sumAll/15. );
    
    ++nEntries_cut;
  }
  std::cout << std::endl;
  std::cout << ">>> Selected " << nEntries_cut << " / " << nEntries << " events" << std::endl;
  
  
  
  //-----------
  // draw plots
  TCanvas* c1;
  
  
  c1 = new TCanvas(Form("c1_amplitude_ref"),Form("ampRef"));
  gPad -> SetLogy();
  h_ampRef -> GetXaxis() -> SetRangeUser(0.,1000.);
  h_ampRef -> SetTitle(Form(";PHOTEK amp_{max} (mV); events"));
  h_ampRef -> SetLineWidth(2);
  h_ampRef -> Draw();
  TLine* line_cutAmpMinRef = new TLine(cut_minAmplitudeRef,0.,cut_minAmplitudeRef,1.5*h_ampRef->GetMaximum());
  TLine* line_cutAmpMaxRef = new TLine(cut_maxAmplitudeRef,0.,cut_maxAmplitudeRef,1.5*h_ampRef->GetMaximum());
  line_cutAmpMinRef -> SetLineColor(kBlack);
  line_cutAmpMaxRef -> SetLineColor(kBlack);
  line_cutAmpMinRef -> SetLineStyle(9);
  line_cutAmpMaxRef -> SetLineStyle(9);
  line_cutAmpMinRef -> SetLineWidth(2);
  line_cutAmpMaxRef -> SetLineWidth(2);
  line_cutAmpMinRef -> Draw("same");
  line_cutAmpMaxRef -> Draw("same");
  c1 -> Print(Form("%s/c__ampRef.png",plotDir.c_str()));
  
  c1 = new TCanvas(Form("c1_amplitude_ref_vs_beam_Y_vs_X"),Form("ampRef"));
  p2_ampRef_vs_beam_Y_vs_X -> SetTitle(Form(";beam x (mm);beam y (mm);#LT PHOTEK amp_{max} #GT (mV)"));
  p2_ampRef_vs_beam_Y_vs_X -> Draw("colz");
  c1 -> Print(Form("%s/c__ampRef_vs_beam_Y_vs_X.png",plotDir.c_str()));
  
  
  c1 = new TCanvas(Form("c1_amplitudes"),Form("amp"),2000,2000);
  c1 -> Divide(4,4);
  for(unsigned int ii = 0; ii < channels.size(); ++ii)
  {
    std::string ch = channels.at(ii);
    c1 -> cd(ii+1);
    gPad -> SetLogy();
    h_amp[ch] -> GetXaxis() -> SetRangeUser(0.,1000.);
    h_amp[ch] -> SetTitle(Form(";%s amp_{max} (mV); events",ch.c_str()));
    h_amp[ch] -> SetLineWidth(2);
    h_amp[ch] -> Draw();
    TLine* line_cutAmpMinRef = new TLine(ampMin[ch],0.,ampMin[ch],1.5*h_amp[ch]->GetMaximum());
    TLine* line_cutAmpMaxRef = new TLine(ampMax[ch],0.,ampMax[ch],1.5*h_amp[ch]->GetMaximum());
    line_cutAmpMinRef -> SetLineColor(kBlack);
    line_cutAmpMaxRef -> SetLineColor(kBlack);
    line_cutAmpMinRef -> SetLineStyle(9);
    line_cutAmpMaxRef -> SetLineStyle(9);
    line_cutAmpMinRef -> SetLineWidth(2);
    line_cutAmpMaxRef -> SetLineWidth(2);
    line_cutAmpMinRef -> Draw("same");
    line_cutAmpMaxRef -> Draw("same");
  }
  c1 -> Print(Form("%s/c__amplitudes.png",plotDir.c_str()));
  
  
  c1 = new TCanvas(Form("c1_CTR_vs_amp"),Form("CTR_vs_amp"),2000,2000);
  c1 -> Divide(4,4);
  for(unsigned int ii = 0; ii < channels.size(); ++ii)
  {
    std::string ch = channels.at(ii);
    c1 -> cd(ii+1);
    p_CTR_vs_amp[ch] -> GetXaxis() -> SetRangeUser(0.,1000.);
    p_CTR_vs_amp[ch] -> SetTitle(Form(";%s amp_{max} (mV); #LT t_{%s} - t_{MCP} #GT",ch.c_str(),ch.c_str()));
    p_CTR_vs_amp[ch] -> Draw();
    p_CTR_vs_amp[ch] -> SetMinimum(CTRMean-5.*CTRSigma);
    p_CTR_vs_amp[ch] -> SetMaximum(CTRMean+5.*CTRSigma);
    fitFunc_corrAmp[ch] -> Draw("same");
  }
  c1 -> Print(Form("%s/c__CTR_vs_amp.png",plotDir.c_str()));
  
  
  c1 = new TCanvas(Form("c1_CTR_vs_beam_X"),Form("CTR_vs_beam_X"),2000,2000);
  c1 -> Divide(4,4);
  for(unsigned int ii = 0; ii < channels.size(); ++ii)
  {
    std::string ch = channels.at(ii);
    c1 -> cd(ii+1);
    p_CTR_vs_beam_X[ch] -> SetMinimum(CTRMean-3.*CTRSigma);
    p_CTR_vs_beam_X[ch] -> SetMaximum(CTRMean+3.*CTRSigma);
    p_CTR_vs_beam_X[ch] -> SetTitle(";beam X (mm);#LT #Deltat #GT (ns)");
    p_CTR_vs_beam_X[ch] -> Draw();
    p_CTR_vs_beam_X_cut[ch] -> SetMarkerColor(kRed);
    p_CTR_vs_beam_X_cut[ch] -> SetLineColor(kRed);
    p_CTR_vs_beam_X_cut[ch] -> Draw("same");
  }
  c1 -> Print(Form("%s/c__CTR_vs_beam_X.png",plotDir.c_str()));
  
  c1 = new TCanvas(Form("c1_CTR_vs_beam_Y"),Form("CTR_vs_beam_Y"),2000,2000);
  c1 -> Divide(4,4);
  for(unsigned int ii = 0; ii < channels.size(); ++ii)
  {
    std::string ch = channels.at(ii);
    c1 -> cd(ii+1);
    p_CTR_vs_beam_Y[ch] -> SetMinimum(CTRMean-3.*CTRSigma);
    p_CTR_vs_beam_Y[ch] -> SetMaximum(CTRMean+3.*CTRSigma);
    p_CTR_vs_beam_Y[ch] -> SetTitle(";beam Y (mm);#LT #Deltat #GT (ns)");
    p_CTR_vs_beam_Y[ch] -> Draw("");
    p_CTR_vs_beam_Y_cut[ch] -> SetMarkerColor(kBlue);
    p_CTR_vs_beam_Y_cut[ch] -> SetLineColor(kBlue);
    p_CTR_vs_beam_Y_cut[ch] -> Draw("same");
  }
  c1 -> Print(Form("%s/c__CTR_vs_beam_Y.png",plotDir.c_str()));
  
  c1 = new TCanvas(Form("c1_CTR_vs_beam_Y_vs_X"),Form("CTR_vs_beam_Y_vs_X"),2000,2000);
  c1 -> Divide(4,4);
  for(unsigned int ii = 0; ii < channels.size(); ++ii)
  {
    std::string ch = channels.at(ii);
    c1 -> cd(ii+1);
    p2_CTR_vs_beam_Y_vs_X[ch] -> SetMinimum(CTRMean-3.*CTRSigma);
    p2_CTR_vs_beam_Y_vs_X[ch] -> SetMaximum(CTRMean+3.*CTRSigma);
    p2_CTR_vs_beam_Y_vs_X[ch] -> SetTitle(";beam X (mm);beam Y (mm);#LT #Deltat #GT (ns)");
    p2_CTR_vs_beam_Y_vs_X[ch] -> Draw("colz");
  }
  c1 -> Print(Form("%s/c__CTR_vs_beam_Y_vs_X.png",plotDir.c_str()));
  
  
  c1 = new TCanvas(Form("c1_CTR_ampCorrEff_vs_beam_X"),Form("CTR_ampCorrEff_vs_beam_X"),2000,2000);
  c1 -> Divide(4,4);
  for(unsigned int ii = 0; ii < channels.size(); ++ii)
  {
    std::string ch = channels.at(ii);
    c1 -> cd(ii+1);
    p_CTR_ampCorrEff_vs_beam_X[ch] -> SetMinimum(CTRCorrMean-2.*CTRCorrSigma);
    p_CTR_ampCorrEff_vs_beam_X[ch] -> SetMaximum(CTRCorrMean+2.*CTRCorrSigma);
    p_CTR_ampCorrEff_vs_beam_X[ch] -> SetTitle(";beam X (mm);#LT eff. amp-corrected #Deltat #GT (ns)");
    p_CTR_ampCorrEff_vs_beam_X[ch] -> Draw();
    p_CTR_ampCorrEff_vs_beam_X_cut[ch] -> SetMarkerColor(kRed);
    p_CTR_ampCorrEff_vs_beam_X_cut[ch] -> SetLineColor(kRed);
    p_CTR_ampCorrEff_vs_beam_X_cut[ch] -> Draw("same");
  }
  c1 -> Print(Form("%s/c__CTR_ampCorrEff_vs_beam_X.png",plotDir.c_str()));
  
  c1 = new TCanvas(Form("c1_CTR_ampCorrEff_vs_beam_Y"),Form("CTR_ampCorrEff_vs_beam_Y"),2000,2000);
  c1 -> Divide(4,4);
  for(unsigned int ii = 0; ii < channels.size(); ++ii)
  {
    std::string ch = channels.at(ii);
    c1 -> cd(ii+1);
    p_CTR_ampCorrEff_vs_beam_Y[ch] -> SetMinimum(CTRCorrMean-2.*CTRCorrSigma);
    p_CTR_ampCorrEff_vs_beam_Y[ch] -> SetMaximum(CTRCorrMean+2.*CTRCorrSigma);
    p_CTR_ampCorrEff_vs_beam_Y[ch] -> SetTitle(";beam Y (mm);#LT eff. amp-corrected #Deltat #GT (ns)");
    p_CTR_ampCorrEff_vs_beam_Y[ch] -> Draw("");
    p_CTR_ampCorrEff_vs_beam_Y_cut[ch] -> SetMarkerColor(kBlue);
    p_CTR_ampCorrEff_vs_beam_Y_cut[ch] -> SetLineColor(kBlue);
    p_CTR_ampCorrEff_vs_beam_Y_cut[ch] -> Draw("same");
  }
  c1 -> Print(Form("%s/c__CTR_ampCorrEff_vs_beam_Y.png",plotDir.c_str()));
  
  c1 = new TCanvas(Form("c1_CTR_ampCorrEff_vs_beam_Y_vs_X"),Form("CTR_ampCorrEff_vs_beam_Y_vs_X"),2000,2000);
  c1 -> Divide(4,4);
  for(unsigned int ii = 0; ii < channels.size(); ++ii)
  {
    std::string ch = channels.at(ii);
    c1 -> cd(ii+1);
    p2_CTR_ampCorrEff_vs_beam_Y_vs_X[ch] -> SetMinimum(CTRCorrMean-2.*CTRCorrSigma);
    p2_CTR_ampCorrEff_vs_beam_Y_vs_X[ch] -> SetMaximum(CTRCorrMean+2.*CTRCorrSigma);
    p2_CTR_ampCorrEff_vs_beam_Y_vs_X[ch] -> SetTitle(";beam X (mm);beam Y (mm);#LT eff. amp-corrected #Deltat #GT (ns)");
    p2_CTR_ampCorrEff_vs_beam_Y_vs_X[ch] -> Draw("colz");
  }
  c1 -> Print(Form("%s/c__CTR_ampCorrEff_vs_beam_Y_vs_X.png",plotDir.c_str()));
  
  
  c1 = new TCanvas("c1_CTR","CTR",2000,2000);
  c1 -> Divide(4,4);
  for(unsigned int ii = 0; ii < channels.size(); ++ii)
  {
    std::string ch = channels.at(ii);
    c1 -> cd(ii+1);  
    drawCTRPlot(h_CTR[ch],rebin,1,0,0.,ch,"MCP","",latexLabel12);
  }
  c1 -> Print(Form("%s/c__CTR.png",plotDir.c_str()));
  
  c1 = new TCanvas("c1_CTR_ampCorrEff","CTR ampCorrEff",2000,2000);
  c1 -> Divide(4,4);
  for(unsigned int ii = 0; ii < channels.size(); ++ii)
  {
    std::string ch = channels.at(ii);
    c1 -> cd(ii+1);  
    drawCTRPlot(h_CTR_ampCorrEff[ch],rebin,1,0,0.,ch,"MCP","eff. amp-corrected ",latexLabel12);
  }
  c1 -> Print(Form("%s/c__CTR_ampCorrEff.png",plotDir.c_str()));
  
  c1 = new TCanvas("c1_CTR_ampCorrEffPosCorr","CTR ampCorrEffPosCorr",2000,2000);
  c1 -> Divide(4,4);
  for(unsigned int ii = 0; ii < channels.size(); ++ii)
  {
    std::string ch = channels.at(ii);
    c1 -> cd(ii+1);  
    drawCTRPlot(h_CTR_ampCorrEffPosCorr[ch],rebin,1,0,0.,ch,"MCP","eff. amp + pos-corrected ",latexLabel12);
  }
  c1 -> Print(Form("%s/c__CTR_ampCorrEffPosCorr.png",plotDir.c_str()));
  
  c1 = new TCanvas("c1_CTR_sum2","CTR_sum2",2000,2000);
  drawCTRPlot(h_CTR_sum2,rebin,1,0,0.,"#LTC1+B4#GT","MCP","",latexLabel12);
  c1 -> Print(Form("%s/c__CTR_sum2.png",plotDir.c_str()));
  
  c1 = new TCanvas("c1_CTR_sum4","CTR_sum4",2000,2000);
  drawCTRPlot(h_CTR_sum4,rebin,1,0,0.,"#LTC1+D3+B4+A2#GT","MCP","",latexLabel12);
  c1 -> Print(Form("%s/c__CTR_sum4.png",plotDir.c_str()));
  
  c1 = new TCanvas("c1_CTR_sumAll","CTR_sumAll",2000,2000);
  drawCTRPlot(h_CTR_sumAll,rebin,1,0,0.,"#LTall#GT","MCP","",latexLabel12);
  c1 -> Print(Form("%s/c__CTR_sumAll.png",plotDir.c_str()));
  
  
  
  if( popupPlots ) theApp -> Run();
  return 0;
}
