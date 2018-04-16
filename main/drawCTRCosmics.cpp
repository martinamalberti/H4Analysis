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

bool popupPlots = true;



int main(int argc, char** argv)
{
  if( argc < 2 )
  {
    std::cerr << ">>>>> drawCTRCosmics.cpp::usage:   " << argv[0] << " configFileName" << std::endl;
    return 1;
  }
  
  
  
  //----------------------
  // parse the config file
  CfgManager opts;
  opts.ParseConfigFile(argv[1]);
  
  std::vector<std::string> inputFiles = opts.GetOpt<std::vector<std::string> >("Input.inputFiles");
  
  std::vector<std::string> timeChannels = opts.GetOpt<std::vector<std::string> >("Input.timeChannels");
  std::string timeCh0 = timeChannels.at(0);
  std::string timeCh1 = timeChannels.at(1);
  std::string timeType0 = opts.GetOpt<std::string>("Input.timeType0");
  std::string timeType1 = opts.GetOpt<std::string>("Input.timeType1");
  std::vector<std::string> energyChannels = opts.GetOpt<std::vector<std::string> >("Input.energyChannels");
  std::string enCh0 = energyChannels.at(0);
  std::string enCh1 = energyChannels.at(1);
  std::vector<float> isMCP = opts.GetOpt<std::vector<float> >("Input.isMCP");
  bool isMCP0 = bool(isMCP.at(0));
  bool isMCP1 = bool(isMCP.at(1));
  
  float cut_ampMin1 = opts.GetOpt<float>("Cuts.minAmplitude1");
  float cut_ampMax1 = opts.GetOpt<float>("Cuts.maxAmplitude1");
  float cut_ampMin2 = opts.GetOpt<float>("Cuts.minAmplitude2");
  float cut_ampMax2 = opts.GetOpt<float>("Cuts.maxAmplitude2");
  
  int rebin = opts.GetOpt<int>("Plots.rebin");
  std::string label1 = opts.GetOpt<std::string>("Plots.label1");
  std::string label2 = opts.GetOpt<std::string>("Plots.label2");
  std::string label12;
  if( label1 == label2 ) label12 = label1;
  else label12 = std::string(Form("%s-%s",label1.c_str(),label2.c_str()));
  
  std::string xLabel1_en("SiPM_{1}"); if( isMCP0 ) xLabel1_en = label1;
  std::string xLabel2_en("SiPM_{2}"); if( isMCP1 ) xLabel2_en =  label2;
  std::string xLabel1_time("NINO_{1}"); if( isMCP0 ) xLabel1_time = label1;
  std::string xLabel2_time("NINO_{2}"); if( isMCP1 ) xLabel2_time = label2;
  
  
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
  
  std::string baseDir(Form("/gwpool/users/benaglia/web/TIMING/cosmics/"));
  system(Form("mkdir -p %s",baseDir.c_str()));
  system(Form("cp /gwpool/users/benaglia/public/index.php %s",baseDir.c_str()));
  std::string plotDir(Form("%s/%s/",baseDir.c_str(),label12.c_str()));
  system(Form("mkdir %s",plotDir.c_str()));
  system(Form("cp /gwpool/users/benaglia/public/index.php %s",plotDir.c_str()));
  
  
  //---------------------------
  // open input files and trees
  TChain* chain1 = new TChain("info","info");
  TChain* chain2 = new TChain("digi","digi");
  for(unsigned int fileIt = 0; fileIt < inputFiles.size(); ++fileIt)
  {
    chain1 -> Add(inputFiles.at(fileIt).c_str());
    chain2 -> Add(inputFiles.at(fileIt).c_str());
  }
  chain2 -> BuildIndex("index");
  chain1 -> AddFriend("digi");
  chain1 -> BuildIndex("index");
  std::cout << " Read " << chain1->GetEntries() << " total events in tree " << chain1->GetName() << std::endl;
  std::cout << " Read " << chain2->GetEntries() << " total events in tree " << chain2->GetName() << std::endl;
  
  TreeVarsCosmics treeVars;
  InitTreeVarsCosmics(chain1,treeVars,opts);
  
  
  //------------------
  // Define histograms
  TH1F* h_amp1 = new TH1F(Form("h_amp1"),"",1000,0.,1000.);
  TH1F* h_amp2 = new TH1F(Form("h_amp2"),"",1000,0.,1000.);
  TH1F* h_ampRatio = new TH1F(Form("h_ampRatio"),"",1000,0.,3.);
  TH1F* h_amp1_cut = new TH1F(Form("h_amp1_cut"),"",1000,0.,1000.);
  TH1F* h_amp2_cut = new TH1F(Form("h_amp2_cut"),"",1000,0.,1000.);
  TH1F* h_ampRatio_cut = new TH1F(Form("h_ampRatio_cut"),"",1000,0.,3.);
  
  TH1F* h_CTR = new TH1F("h_CTR","",10000,-5.,5.);
  TH1F* h_CTR_ampCorr = new TH1F("h_CTR_ampCorr","",10000,-5.,5.);
  
  TProfile* p1_CTR_vs_amp1 = new TProfile("p1_CTR_vs_amp1","",100,0.,1000.);
  TProfile* p1_CTR_vs_amp2 = new TProfile("p1_CTR_vs_amp2","",100,0.,1000.);
  TProfile* p1_CTR_vs_ampRatio = new TProfile("p1_CTR_vs_ampRatio","",100,0.,5.);
  
  TProfile* p1_CTR_ampCorr_vs_amp1 = new TProfile("p1_CTR_ampCorr_vs_amp1","",100,0.,1000.);
  TProfile* p1_CTR_ampCorr_vs_amp2 = new TProfile("p1_CTR_ampCorr_vs_amp2","",100,0.,1000.);
  TProfile* p1_CTR_ampCorr_vs_ampRatio = new TProfile("p1_CTR_ampCorr_vs_ampRatio","",100,0.,5.);
  
  
  
  //-----------------------
  // first loop over events
  int nEntries = chain1 -> GetEntries();
  int nEntries_cut = 0;
  for(int entry = 0; entry < nEntries; ++entry)
  {
    if( entry%1000 == 0 ) std::cout << ">>> loop 1/2: reading entry " << entry << " / " << nEntries << "\r" << std::flush;
    chain1 -> GetEntry(entry);
    
    float amp1 = treeVars.t_amp[(*treeVars.t_channelId)[enCh0]] * 0.25;
    float amp2 = treeVars.t_amp[(*treeVars.t_channelId)[enCh1]] * 0.25;
    int extraIt1 = (*treeVars.t_timeTypes)[timeType0];
    int extraIt2 = (*treeVars.t_timeTypes)[timeType1];
    float time1 = treeVars.t_time[(*treeVars.t_channelId)[timeCh0]+extraIt1];
    float time2 = treeVars.t_time[(*treeVars.t_channelId)[timeCh1]+extraIt2];
    float CTR = time2 - time1;
    
    h_amp1 -> Fill( amp1 );
    h_amp2 -> Fill( amp2 );
    h_ampRatio -> Fill( amp2/amp1 );
    
    if( !AcceptEventAmp(enCh0,enCh1,treeVars,cut_ampMin1,cut_ampMax1,cut_ampMin2,cut_ampMax2) ) continue;
    // if( !AcceptEventTime(timeCh0,timeCh1,isMCP0,isMCP1,treeVars,0.,200.,0.,200.) ) continue;
    // if( !AcceptEventTime(timeCh0,timeCh1,0,0,treeVars,0.,200.,0.,200.) ) continue;
    
    h_amp1_cut -> Fill( amp1 );
    h_amp2_cut -> Fill( amp2 );
    h_ampRatio_cut -> Fill( amp2/amp1 );
    
    h_CTR -> Fill( CTR );
    
    p1_CTR_vs_amp1 -> Fill( amp1,CTR );
    p1_CTR_vs_amp2 -> Fill( amp2,CTR );
    p1_CTR_vs_ampRatio -> Fill( amp2/amp1,CTR );
    
    ++nEntries_cut;
  }
  std::cout << std::endl;
  std::cout << ">>>>>> Selected " << nEntries_cut << " / " << nEntries << " events" << std::endl;
  
  
  
  //--------------------------
  // amplitude walk correction
  TF1* fitFunc_corrAmp = new TF1(Form("fitFunc_corrAmp"),"[0]*log([1]*x)+[2]",0.,1000.);
  fitFunc_corrAmp -> SetParameters(-0.2,0.0000001,0.);
  if( isMCP0 && !isMCP1)
    p1_CTR_vs_amp2 -> Fit(Form("fitFunc_corrAmp"),"QNS+","",h_amp2_cut->GetMean()+0.5*h_amp2_cut->GetRMS(),h_amp2_cut->GetMean()+10.*h_amp2_cut->GetRMS());
  else if( !isMCP0 && isMCP1)
    p1_CTR_vs_amp1 -> Fit(Form("fitFunc_corrAmp"),"QNS+","",h_amp1_cut->GetMean()+0.5*h_amp1_cut->GetRMS(),h_amp1_cut->GetMean()+10.*h_amp1_cut->GetRMS());
  else
    p1_CTR_vs_ampRatio -> Fit(Form("fitFunc_corrAmp"),"QNS+","");
  
  
  
  
  //------------------------
  // second loop over events
  nEntries_cut = 0;
  for(int entry = 0; entry < nEntries; ++entry)
  {
    if( entry%1000 == 0 ) std::cout << ">>> loop 2/2: reading entry " << entry << " / " << nEntries << "\r" << std::flush;
    chain1 -> GetEntry(entry);
    
    float amp1 = treeVars.t_amp[(*treeVars.t_channelId)[enCh0]] * 0.25;
    float amp2 = treeVars.t_amp[(*treeVars.t_channelId)[enCh1]] * 0.25;
    int extraIt1 = (*treeVars.t_timeTypes)[timeType0];
    int extraIt2 = (*treeVars.t_timeTypes)[timeType1];
    float time1 = treeVars.t_time[(*treeVars.t_channelId)[timeCh0]+extraIt1];
    float time2 = treeVars.t_time[(*treeVars.t_channelId)[timeCh1]+extraIt2];
    float CTR = time2 - time1;
    
    if( !AcceptEventAmp(enCh0,enCh1,treeVars,cut_ampMin1,cut_ampMax1,cut_ampMin2,cut_ampMax2) ) continue;
    // if( !AcceptEventTime(timeCh0,timeCh1,isMCP0,isMCP1,treeVars,0.,200.,0.,200.) ) continue;
    // if( !AcceptEventTime(timeCh0,timeCh1,0,0,treeVars,0.,200.,0.,200.) ) continue;
    
    float CTR_ampCorr;
    if( isMCP0 && !isMCP1 )
      CTR_ampCorr = CTR -
        p1_CTR_vs_amp2->GetBinContent( p1_CTR_vs_amp2->FindBin(amp2) ) + 
        p1_CTR_vs_amp2->GetBinContent( p1_CTR_vs_amp2->FindBin(h_amp2_cut->GetMean()) );
    else if( !isMCP0 && isMCP1 )
      CTR_ampCorr = CTR -
        p1_CTR_vs_amp1->GetBinContent( p1_CTR_vs_amp1->FindBin(amp1) ) + 
        p1_CTR_vs_amp1->GetBinContent( p1_CTR_vs_amp1->FindBin(h_amp1_cut->GetMean()) );
    else
      CTR_ampCorr = CTR -
        fitFunc_corrAmp -> Eval(amp2/amp1) + 
        fitFunc_corrAmp -> Eval(h_ampRatio_cut->GetMean());
    
    h_CTR_ampCorr -> Fill( CTR_ampCorr );
    
    p1_CTR_ampCorr_vs_amp1 -> Fill( amp1,CTR_ampCorr );
    p1_CTR_ampCorr_vs_amp2 -> Fill( amp2,CTR_ampCorr );
    p1_CTR_ampCorr_vs_ampRatio -> Fill( amp2/amp1,CTR_ampCorr );
    
    ++nEntries_cut;
  }
  std::cout << std::endl;
  std::cout << ">>>>>> Selected " << nEntries_cut << " / " << nEntries << " events" << std::endl;
  
  
  /*
  //-----------------------
  // third loop over events
  for(int entry = 0; entry < nEntries; ++entry)
  {
    if( entry%1000 == 0 ) std::cout << ">>> loop 3/3: reading entry " << entry << " / " << nEntries << "\r" << std::flush;
    chain1 -> GetEntry(entry);
    
    if( !AcceptEvent(enCh0,enCh1,timeCh0,timeCh1,treeVars,
                     1,cut_crystalXMin,cut_crystalXMax,cut_crystalYMin,cut_crystalYMax,
                     -1) )
      continue;
    
    if( !AcceptEventVbias(VbiasIndex1,VbiasIndex2,treeVars,cut_Vbias,cut_Vbias) ) continue;
    if( !AcceptEventTh(treeVars,cut_NINOthr,cut_NINOthr) ) continue;
    
    float amp1 = treeVars.t_amp[(*treeVars.t_channelId)[enCh0]] * 0.25;
    float amp2 = treeVars.t_amp[(*treeVars.t_channelId)[enCh1]] * 0.25;
    int extraIt1 = isMCP0 ? 14 : 0;
    int extraIt2 = isMCP1 ? 14 : 0;
    float time1 = treeVars.t_time[(*treeVars.t_channelId)[timeCh0]+extraIt1];
    float time2 = treeVars.t_time[(*treeVars.t_channelId)[timeCh1]+extraIt2];
    float CTR = time2 - time1;
    
    if( !AcceptEventAmp(enCh0,enCh1,treeVars,cut_ampMin1,cut_ampMax1,cut_ampMin2,cut_ampMax2) ) continue;
    if( !AcceptEventDur(timeCh0,timeCh1,treeVars,cut_durMin1,cut_durMax1,cut_durMin2,cut_durMax2) ) continue;
    if( !AcceptEventTime(timeCh0,timeCh1,isMCP0,isMCP1,treeVars,0.,200.,0.,200.) ) continue;
    if( isMCP0 || isMCP1 ) if( !AcceptEventMCP(treeVars,cut_MCPX,cut_MCPY,cut_MCPR) ) continue;
    
    bool isCenter = false;
    bool isBorder = false;
    if( treeVars.t_beamX[0] > cut_crystalXMin &&
        treeVars.t_beamX[0] < cut_crystalXMax &&
        treeVars.t_beamY[0] > cut_crystalYMin &&
        treeVars.t_beamY[0] < cut_crystalYMax )
    {
      isCenter = false;
      isBorder = true;
    }
    if( treeVars.t_beamX[0] > 0.5*(cut_crystalXMin+cut_crystalXMax)-2. &&
        treeVars.t_beamX[0] < 0.5*(cut_crystalXMin+cut_crystalXMax)+2. &&
        treeVars.t_beamY[0] > 0.5*(cut_crystalYMin+cut_crystalYMax)-2. &&
        treeVars.t_beamY[0] < 0.5*(cut_crystalYMin+cut_crystalYMax)+2. )
    {
      isCenter = true;
      isBorder = false;
    }
    
    float ampCorr;
    if( isMCP0 && !isMCP1 )
      ampCorr = 
        -p1_CTR_posCorr_vs_amp2->GetBinContent( p1_CTR_posCorr_vs_amp2->FindBin(amp2) ) + 
        p1_CTR_posCorr_vs_amp2->GetBinContent( p1_CTR_posCorr_vs_amp2->FindBin(h_amp2_cut->GetMean()) );
    else if( !isMCP0 && isMCP1 )
      ampCorr = 
        -p1_CTR_posCorr_vs_amp1->GetBinContent( p1_CTR_posCorr_vs_amp1->FindBin(amp1) ) + 
        p1_CTR_posCorr_vs_amp1->GetBinContent( p1_CTR_posCorr_vs_amp1->FindBin(h_amp1_cut->GetMean()) );
    else
      ampCorr = 
        -p1_CTR_posCorr_vs_ampRatio->GetBinContent( p1_CTR_posCorr_vs_ampRatio->FindBin(amp2/amp1) ) + 
        p1_CTR_posCorr_vs_ampRatio->GetBinContent( p1_CTR_posCorr_vs_ampRatio->FindBin(h_ampRatio_cut->GetMean()) );
    
    float CTR_posCorrAmpCorr = CTR -
      p2_CTR_vs_beam_Y_vs_X->GetBinContent( p2_CTR_vs_beam_Y_vs_X->FindBin(treeVars.t_beamX[0],treeVars.t_beamY[0]) ) + 
      p2_CTR_vs_beam_Y_vs_X->GetBinContent( p2_CTR_vs_beam_Y_vs_X->FindBin(0.5*(cut_crystalXMin+cut_crystalXMax),0.5*(cut_crystalYMin+cut_crystalYMax)) ) +
      ampCorr;
    
    float CTR_ampCorrEffPosCorr;
    if( isMCP0 && !isMCP1 )
      CTR_ampCorrEffPosCorr= CTR -
        fitFunc_corrAmp -> Eval( amp2 ) +
        fitFunc_corrAmp -> Eval( h_amp2_cut->GetMean() ) -
        p2_CTR_ampCorrEff_vs_beam_Y_vs_X->GetBinContent( p2_CTR_vs_beam_Y_vs_X->FindBin(treeVars.t_beamX[0],treeVars.t_beamY[0]) ) +
        p2_CTR_ampCorrEff_vs_beam_Y_vs_X->GetBinContent( p2_CTR_vs_beam_Y_vs_X->FindBin(0.5*(cut_crystalXMin+cut_crystalXMax),0.5*(cut_crystalYMin+cut_crystalYMax)) );
    else if( !isMCP0 && isMCP1 )
      CTR_ampCorrEffPosCorr= CTR -
        fitFunc_corrAmp -> Eval( amp1 ) +
        fitFunc_corrAmp -> Eval( h_amp1_cut->GetMean() ) -
        p2_CTR_ampCorrEff_vs_beam_Y_vs_X->GetBinContent( p2_CTR_vs_beam_Y_vs_X->FindBin(treeVars.t_beamX[0],treeVars.t_beamY[0]) ) +
        p2_CTR_ampCorrEff_vs_beam_Y_vs_X->GetBinContent( p2_CTR_vs_beam_Y_vs_X->FindBin(0.5*(cut_crystalXMin+cut_crystalXMax),0.5*(cut_crystalYMin+cut_crystalYMax)) );
    else
      CTR_ampCorrEffPosCorr= CTR -
        fitFunc_corrAmp -> Eval( amp2/amp1 ) +
        fitFunc_corrAmp -> Eval( h_ampRatio_cut->GetMean() ) -
        p2_CTR_ampCorrEff_vs_beam_Y_vs_X->GetBinContent( p2_CTR_vs_beam_Y_vs_X->FindBin(treeVars.t_beamX[0],treeVars.t_beamY[0]) ) +
        p2_CTR_ampCorrEff_vs_beam_Y_vs_X->GetBinContent( p2_CTR_vs_beam_Y_vs_X->FindBin(0.5*(cut_crystalXMin+cut_crystalXMax),0.5*(cut_crystalYMin+cut_crystalYMax)) );
    
    h_CTR_posCorrAmpCorr -> Fill( CTR_posCorrAmpCorr );
    if( isCenter ) h_CTR_posCorrAmpCorr_center -> Fill( CTR_posCorrAmpCorr );
    if( isBorder ) h_CTR_posCorrAmpCorr_border -> Fill( CTR_posCorrAmpCorr );
    
    h_CTR_ampCorrEffPosCorr -> Fill( CTR_ampCorrEffPosCorr );
    if( isCenter ) h_CTR_ampCorrEffPosCorr_center -> Fill( CTR_ampCorrEffPosCorr );
    if( isBorder ) h_CTR_ampCorrEffPosCorr_border -> Fill( CTR_ampCorrEffPosCorr );
    
    p1_CTR_posCorrAmpCorr_vs_beam_X -> Fill( treeVars.t_beamX[0],CTR_posCorrAmpCorr );
    p1_CTR_posCorrAmpCorr_vs_beam_Y -> Fill( treeVars.t_beamY[0],CTR_posCorrAmpCorr );
    p2_CTR_posCorrAmpCorr_vs_beam_Y_vs_X -> Fill( treeVars.t_beamX[0],treeVars.t_beamY[0],CTR_posCorrAmpCorr );
    
    p1_CTR_ampCorrEffPosCorr_vs_beam_X -> Fill( treeVars.t_beamX[0],CTR_ampCorrEffPosCorr );
    p1_CTR_ampCorrEffPosCorr_vs_beam_Y -> Fill( treeVars.t_beamY[0],CTR_ampCorrEffPosCorr );
    p2_CTR_ampCorrEffPosCorr_vs_beam_Y_vs_X -> Fill( treeVars.t_beamX[0],treeVars.t_beamY[0],CTR_ampCorrEffPosCorr );
    
    if( fabs( treeVars.t_beamY[0] - 0.5*(cut_crystalYMin+cut_crystalYMax) ) < 2. )
    {
      p1_CTR_posCorrAmpCorr_vs_beam_X_cut -> Fill( treeVars.t_beamX[0],CTR_posCorrAmpCorr );
      p1_CTR_ampCorrEffPosCorr_vs_beam_X_cut -> Fill( treeVars.t_beamX[0],CTR_ampCorrEffPosCorr );
    }
    
    if( fabs( treeVars.t_beamX[0] - 0.5*(cut_crystalXMin+cut_crystalXMax) ) < 2. )
    {
      p1_CTR_posCorrAmpCorr_vs_beam_Y_cut -> Fill( treeVars.t_beamY[0],CTR_posCorrAmpCorr );
      p1_CTR_ampCorrEffPosCorr_vs_beam_Y_cut -> Fill( treeVars.t_beamY[0],CTR_ampCorrEffPosCorr );
    }
    
    p1_CTR_posCorrAmpCorr_vs_amp1 -> Fill( amp1,CTR_posCorrAmpCorr );
    p1_CTR_posCorrAmpCorr_vs_amp2 -> Fill( amp2,CTR_posCorrAmpCorr );
    p1_CTR_posCorrAmpCorr_vs_ampRatio -> Fill( amp2/amp1,CTR_posCorrAmpCorr );
    
    p1_CTR_ampCorrEffPosCorr_vs_amp1 -> Fill( amp1,CTR_ampCorrEffPosCorr );
    p1_CTR_ampCorrEffPosCorr_vs_amp2 -> Fill( amp2,CTR_ampCorrEffPosCorr );
    p1_CTR_ampCorrEffPosCorr_vs_ampRatio -> Fill( amp2/amp1,CTR_ampCorrEffPosCorr );
  }
  std::cout << std::endl;
  */
  
  // find CTR ranges
  float CTRMean = -1;
  float CTRSigma = -1;
  {
    TH1F* histo = h_CTR;
    float* vals = new float[4];
    FindSmallestInterval(vals,histo,0.68,true); 
    
    float mean = vals[0];
    float min = vals[2];
    float max = vals[3];
    float delta = max-min;
    float sigma = 0.5*delta;
    float effSigma = sigma;
    CTRMean = mean;
    CTRSigma = effSigma;
  }
  /*
  // float CTRMean_ampCorr = -1;
  // float CTRSigma_ampCorr = -1;
  // {
  //   TH1F* histo = h_CTR_ampCorr;
  //   float* vals = new float[4];
  //   FindSmallestInterval(vals,histo,0.68,true); 
    
  //   float mean = vals[0];      
  //   float min = vals[2];
  //   float max = vals[3];
  //   float delta = max-min;
  //   float sigma = 0.5*delta;
  //   float effSigma = sigma;
  //   CTRMean_ampCorr = mean;
  //   CTRSigma_ampCorr = effSigma;
  // }
  
  // float CTRMean_posCorr = -1;
  // float CTRSigma_posCorr = -1;
  // {
  //   TH1F* histo = h_CTR_posCorr;
  //   float* vals = new float[4];
  //   FindSmallestInterval(vals,histo,0.68,true); 
    
  //   float mean = vals[0];      
  //   float min = vals[2];
  //   float max = vals[3];
  //   float delta = max-min;
  //   float sigma = 0.5*delta;
  //   float effSigma = sigma;
  //   CTRMean_posCorr = mean;
  //   CTRSigma_posCorr = effSigma;
  // }
  
  
  
  //------------------------
  // define rectangles, etc.
  
  TBox* box_crystal = new TBox(cut_crystalXMin,cut_crystalYMin,cut_crystalXMax,cut_crystalYMax);
  box_crystal -> SetFillStyle(0);
  box_crystal -> SetLineColor(kBlack);
  box_crystal -> SetLineStyle(9);
  box_crystal -> SetLineWidth(3);
  
  TEllipse* ell_MCP = new TEllipse(cut_MCPX,cut_MCPY,cut_MCPR); 
  ell_MCP -> SetFillStyle(0);
  ell_MCP -> SetLineColor(kBlack);
  ell_MCP -> SetLineStyle(9);
  ell_MCP -> SetLineWidth(3);
  
  TLine* xBand_low = new TLine(cut_crystalXMin,0.5*(cut_crystalYMin+cut_crystalYMax)-2.,cut_crystalXMax,0.5*(cut_crystalYMin+cut_crystalYMax)-2.);
  TLine* xBand_hig = new TLine(cut_crystalXMin,0.5*(cut_crystalYMin+cut_crystalYMax)+2.,cut_crystalXMax,0.5*(cut_crystalYMin+cut_crystalYMax)+2.);
  xBand_low -> SetLineColor(kRed);
  xBand_hig -> SetLineColor(kRed);
  xBand_low -> SetLineStyle(9);
  xBand_hig -> SetLineStyle(9);
  xBand_low -> SetLineWidth(2);
  xBand_hig -> SetLineWidth(2);
  
  TLine* yBand_lef = new TLine(0.5*(cut_crystalXMin+cut_crystalXMax)-2.,cut_crystalYMin,0.5*(cut_crystalXMin+cut_crystalXMax)-2.,cut_crystalYMax);
  TLine* yBand_rig = new TLine(0.5*(cut_crystalXMin+cut_crystalXMax)+2.,cut_crystalYMin,0.5*(cut_crystalXMin+cut_crystalXMax)+2.,cut_crystalYMax);
  yBand_lef -> SetLineColor(kBlue);
  yBand_rig -> SetLineColor(kBlue);
  yBand_lef -> SetLineStyle(9);
  yBand_rig -> SetLineStyle(9);
  yBand_lef -> SetLineWidth(2);
  yBand_rig -> SetLineWidth(2);
  */
  
  
  //-----------
  // draw plots
  TCanvas* c1 = new TCanvas(Form("c1_amplitudes"),Form("amp-dur"),1430,665);
  c1 -> Divide(2,1);
  c1 -> cd(1);
  gPad -> SetLogy();
  h_amp1 -> GetXaxis() -> SetRangeUser(0.,1000.);
  h_amp1 -> SetTitle(Form(";%s amp_{max} (mV); events",xLabel1_en.c_str()));
  h_amp1 -> SetLineWidth(2);
  h_amp1 -> Draw();
  h_amp1 -> Draw("same");
  TLine* line_cutAmpMin1 = new TLine(cut_ampMin1,0.,cut_ampMin1,1.5*h_amp1->GetMaximum());
  TLine* line_cutAmpMax1 = new TLine(cut_ampMax1,0.,cut_ampMax1,1.5*h_amp1->GetMaximum());
  line_cutAmpMin1 -> SetLineColor(kBlack);
  line_cutAmpMax1 -> SetLineColor(kBlack);
  line_cutAmpMin1 -> SetLineStyle(9);
  line_cutAmpMax1 -> SetLineStyle(9);
  line_cutAmpMin1 -> SetLineWidth(2);
  line_cutAmpMax1 -> SetLineWidth(2);
  line_cutAmpMin1 -> Draw("same");
  line_cutAmpMax1 -> Draw("same");
  latexLabel1 -> Draw("same");
  c1 -> cd(2);
  gPad -> SetLogy();
  h_amp2 -> GetXaxis() -> SetRangeUser(0.,1000.);
  h_amp2 -> SetTitle(Form(";%s amp_{max} (mV); events",xLabel2_en.c_str()));
  h_amp2 -> SetLineWidth(2);
  h_amp2 -> Draw();
  h_amp2 -> Draw("same");
  TLine* line_cutAmpMin2 = new TLine(cut_ampMin2,0.,cut_ampMin2,1.5*h_amp2->GetMaximum());
  TLine* line_cutAmpMax2 = new TLine(cut_ampMax2,0.,cut_ampMax2,1.5*h_amp2->GetMaximum());
  line_cutAmpMin2 -> SetLineColor(kBlack);
  line_cutAmpMax2 -> SetLineColor(kBlack);
  line_cutAmpMin2 -> SetLineStyle(9);
  line_cutAmpMax2 -> SetLineStyle(9);
  line_cutAmpMin2 -> SetLineWidth(2);
  line_cutAmpMax2 -> SetLineWidth(2);
  line_cutAmpMin2 -> Draw("same");
  line_cutAmpMax2 -> Draw("same");
  latexLabel2 -> Draw("same");
  gPad -> Update();
  c1 -> Print(Form("%s/c__amp-dur.png",plotDir.c_str()));
  
  
  c1 = new TCanvas("c1_CTR_vs_amp","DeltaT vs amp",2203,665);
  c1 -> Divide(3,1);
  c1 -> cd(1);
  TH1F* hPad = (TH1F*)( gPad->DrawFrame(0.,CTRMean-5.*CTRSigma,1000.,CTRMean+5.*CTRSigma) );
  hPad -> SetTitle(Form(";%s amp_{max} (mV);#LT #Deltat #GT (ns)",xLabel1_en.c_str()));
  hPad -> SetLineColor(kWhite);
  hPad -> Draw();
  gPad -> SetGridy();
  p1_CTR_vs_amp1 -> Draw("same");
  latexLabel1 -> Draw("same");
  c1 -> cd(2);
  hPad = (TH1F*)( gPad->DrawFrame(0.,CTRMean-5.*CTRSigma,1000.,CTRMean+5.*CTRSigma) );
  hPad -> SetTitle(Form(";%s amp_{max} (mV);#LT #Deltat #GT (ns)",xLabel2_en.c_str()));
  hPad -> SetLineColor(kWhite);
  hPad -> Draw();
  gPad -> SetGridy();
  p1_CTR_vs_amp2 -> Draw("same");
  latexLabel2 -> Draw("same");
  c1 -> cd(3);
  hPad = (TH1F*)( gPad->DrawFrame(0.,CTRMean-5.*CTRSigma,2.,CTRMean+5.*CTRSigma) );
  hPad -> SetTitle(Form(";%s amp_{max} / %s amp_{max};#LT #Deltat #GT (ns)",xLabel2_en.c_str(),xLabel1_en.c_str()));
  hPad -> SetLineColor(kWhite);
  hPad -> Draw();
  gPad -> SetGridy();
  p1_CTR_vs_ampRatio -> Draw("same");
  fitFunc_corrAmp -> Draw("same");
  latexLabel2 -> Draw("same");  
  c1 -> Print(Form("%s/c__deltaT_vs_amp.png",plotDir.c_str()));
  
  c1 = new TCanvas("c1_CTR_ampCorr_vs_amp","amp-corrected DeltaT vs amp",2203,665);
  c1 -> Divide(3,1);
  c1 -> cd(1);
  hPad = (TH1F*)( gPad->DrawFrame(0.,CTRMean-5.*CTRSigma,1000.,CTRMean+5.*CTRSigma) );
  hPad -> SetTitle(Form(";%s amp_{max} (mV);#LT amp-corrected #Deltat #GT (ns)",xLabel1_en.c_str()));
  hPad -> SetLineColor(kWhite);
  hPad -> Draw();
  gPad -> SetGridy();
  p1_CTR_ampCorr_vs_amp1 -> Draw("same");
  latexLabel1 -> Draw("same");
  c1 -> cd(2);
  hPad = (TH1F*)( gPad->DrawFrame(0.,CTRMean-5.*CTRSigma,1000.,CTRMean+5.*CTRSigma) );
  hPad -> SetTitle(Form(";%s amp_{max} (mV);#LT amp-corrected #Deltat #GT (ns)",xLabel2_en.c_str()));
  hPad -> SetLineColor(kWhite);
  hPad -> Draw();
  gPad -> SetGridy();
  p1_CTR_ampCorr_vs_amp2 -> Draw("same");
  latexLabel2 -> Draw("same");
  c1 -> cd(3);
  hPad = (TH1F*)( gPad->DrawFrame(0.,CTRMean-5.*CTRSigma,2.,CTRMean+5.*CTRSigma) );
  hPad -> SetTitle(Form(";%s amp_{max} / %s amp_{max};#LT amp-corrected #Deltat #GT (ns)",xLabel2_en.c_str(),xLabel1_en.c_str()));
  hPad -> SetLineColor(kWhite);
  hPad -> Draw();
  gPad -> SetGridy();
  p1_CTR_ampCorr_vs_ampRatio -> Draw("same");
  latexLabel2 -> Draw("same");  
  c1 -> Print(Form("%s/c__deltaT_ampCorr_vs_amp.png",plotDir.c_str()));
  
  c1 = new TCanvas("c1_CTR","CTR");
  c1 -> cd();
  drawCTRPlot(h_CTR,rebin,isMCP0,isMCP1,0.010,xLabel1_time,xLabel2_time,"",latexLabel12);
  c1 -> Print(Form("%s/c__CTR.png",plotDir.c_str()));
  
  c1 = new TCanvas("c1_CTR_ampCorr","CTR_ampCorr");
  c1 -> cd();
  drawCTRPlot(h_CTR_ampCorr,rebin,isMCP0,isMCP1,0.010,xLabel1_time,xLabel2_time,"amp-corrected ",latexLabel12);
  c1 -> Print(Form("%s/c__CTR_ampCorr.png",plotDir.c_str()));
  
  
  if( popupPlots ) theApp -> Run();
  return 0;
}
