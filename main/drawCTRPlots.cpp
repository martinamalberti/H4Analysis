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
#include "TApplication.h"


bool popupPlots = false;

int VbiasIndex1;
int VbiasIndex2;

std::vector<std::string> timeChannels;
std::vector<std::string> energyChannels;
std::vector<float> mcpChannels;

/*** tree variables ***/
struct TreeVars
{
  float* t_beamX;
  float* t_beamY;
  float* t_Vbias;
  float t_NINOthr;
  float t_angle;
  float* t_ped;
  float* t_amp;
  float* t_dur;
  float* t_time;
  std::map<std::string,int>* t_channelId;
};

void InitTreeVars(TTree* chain1, TTree* chain2, TTree* chain3,
                  TreeVars& treeVars);

bool AcceptEvent(TreeVars treeVars, const int& beamCutType, const float& beamXMin, const float& beamXMax, const float& beamYMin, const float& beamYMax, const float& angle);
bool AcceptEventAmp(TreeVars treeVars, const float& ampMin1, const float& ampMax1, const float& ampMin2, const float& ampMax2);
bool AcceptEventDur(TreeVars treeVars, const float& durMin1, const float& durMax1, const float& durMin2, const float& durMax2);
bool AcceptEventTh(TreeVars treeVars, const float& thMin, const float& thMax);
bool AcceptEventVbias(TreeVars treeVars, const float& VbiasMin, const float& VbiasMax);



int main(int argc, char** argv)
{
  if( argc < 2 )
  {
    std::cerr << ">>>>> drawCTRPlots.cpp::usage:   " << argv[0] << " configFileName" << std::endl;
    return 1;
  }
  
  
  
  //----------------------
  // parse the config file
  CfgManager opts;
  opts.ParseConfigFile(argv[1]);
  
  std::vector<std::string> inputFiles = opts.GetOpt<std::vector<std::string> >("Input.inputFiles");
  
  timeChannels = opts.GetOpt<std::vector<std::string> >("Input.timeChannels");
  energyChannels = opts.GetOpt<std::vector<std::string> >("Input.energyChannels");
  mcpChannels = opts.GetOpt<std::vector<float> >("Input.mcpChannels");
  VbiasIndex1 = opts.GetOpt<int>("Input.VbiasIndex1");
  VbiasIndex2 = opts.GetOpt<int>("Input.VbiasIndex2");
  int configuration = opts.GetOpt<int>("Input.configuration");
  std::string extra = opts.GetOpt<std::string>("Input.extra");
  int maxStep = opts.GetOpt<int>("Input.maxStep");
  int nEventsPerEnergyBin = opts.GetOpt<int>("Input.nEventsPerEnergyBin");
  float MCPIntrinsic = opts.GetOpt<float>("Input.MCPIntrinsic");
  
  float cut_NINOthrMin = opts.GetOpt<float>("Cuts.minThreshold");
  float cut_NINOthrMax = opts.GetOpt<float>("Cuts.maxThreshold");
  float cut_VbiasMin = opts.GetOpt<float>("Cuts.minVbias");
  float cut_VbiasMax = opts.GetOpt<float>("Cuts.maxVbias");  
  
  std::vector<float> cut_VbiasValues = opts.GetOpt<std::vector<float> >("Cuts.VbiasValues");
  std::vector<float> cut_minAmplitudes1 = opts.GetOpt<std::vector<float> >("Cuts.minAmplitudes1");
  std::vector<float> cut_maxAmplitudes1 = opts.GetOpt<std::vector<float> >("Cuts.maxAmplitudes1");
  std::vector<float> cut_minAmplitudes2 = opts.GetOpt<std::vector<float> >("Cuts.minAmplitudes2");
  std::vector<float> cut_maxAmplitudes2 = opts.GetOpt<std::vector<float> >("Cuts.maxAmplitudes2");
  std::map<float,float> cut_ampMin1;
  std::map<float,float> cut_ampMax1;
  std::map<float,float> cut_ampMin2;
  std::map<float,float> cut_ampMax2;
  for(unsigned int it = 0; it < cut_VbiasValues.size(); ++it)
  {
    cut_ampMin1[cut_VbiasValues.at(it)] = cut_minAmplitudes1.at(it);
    cut_ampMax1[cut_VbiasValues.at(it)] = cut_maxAmplitudes1.at(it);
    cut_ampMin2[cut_VbiasValues.at(it)] = cut_minAmplitudes2.at(it);
    cut_ampMax2[cut_VbiasValues.at(it)] = cut_maxAmplitudes2.at(it);
  }
  
  std::vector<float> cut_minDurations1 = opts.GetOpt<std::vector<float> >("Cuts.minDurations1");
  std::vector<float> cut_maxDurations1 = opts.GetOpt<std::vector<float> >("Cuts.maxDurations1");
  std::vector<float> cut_minDurations2 = opts.GetOpt<std::vector<float> >("Cuts.minDurations2");
  std::vector<float> cut_maxDurations2 = opts.GetOpt<std::vector<float> >("Cuts.maxDurations2");
  std::map<float,float> cut_durMin1;
  std::map<float,float> cut_durMax1;
  std::map<float,float> cut_durMin2;
  std::map<float,float> cut_durMax2;
  for(unsigned int it = 0; it < cut_VbiasValues.size(); ++it)
  {
    cut_durMin1[cut_VbiasValues.at(it)] = cut_minDurations1.at(it);
    cut_durMax1[cut_VbiasValues.at(it)] = cut_maxDurations1.at(it);
    cut_durMin2[cut_VbiasValues.at(it)] = cut_minDurations2.at(it);
    cut_durMax2[cut_VbiasValues.at(it)] = cut_maxDurations2.at(it);
  }
  
  int cut_beamCutType = opts.GetOpt<int>("Cuts.beamCutType");
  float cut_beamXMin = opts.GetOpt<float>("Cuts.beamXMin");
  float cut_beamXMax = opts.GetOpt<float>("Cuts.beamXMax");
  float cut_beamYMin = opts.GetOpt<float>("Cuts.beamYMin");
  float cut_beamYMax = opts.GetOpt<float>("Cuts.beamYMax");
  
  float cut_angle = opts.GetOpt<float>("Cuts.angle");
  
  int rebin = opts.GetOpt<int>("Plots.rebin");
  std::string label1 = opts.GetOpt<std::string>("Plots.label1");
  std::string label2 = opts.GetOpt<std::string>("Plots.label2");
  std::string label12;
  if( label1 == label2 ) label12 = label1;
  else label12 = std::string(Form("%s-%s",label1.c_str(),label2.c_str()));
  
  //------------------------
  // labels and canvas style
  setTDRStyle();
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
  
  std::string baseDir(Form("/afs/cern.ch/user/a/abenagli/www/TIMING/TBatH4June2017_new/config%02d%s/",configuration,extra.c_str()));
  system(Form("mkdir -p %s",baseDir.c_str()));
  system(Form("cp /afs/cern.ch/user/a/abenagli/public/index.php %s",baseDir.c_str()));
  std::string plotDir(Form("%s/%s_beamCutType%d/",baseDir.c_str(),label12.c_str(),cut_beamCutType));
  system(Form("mkdir %s",plotDir.c_str()));
  system(Form("cp /afs/cern.ch/user/a/abenagli/public/index.php %s",plotDir.c_str()));
  
  
  //---------------------------
  // open input files and trees
  TChain* chain1 = new TChain("info","info");
  TChain* chain2 = new TChain("hodo","hodo");
  TChain* chain3 = new TChain("digi","digi");
  for(unsigned int fileIt = 0; fileIt < inputFiles.size(); ++fileIt)
  {
    chain1 -> Add(inputFiles.at(fileIt).c_str());
    chain2 -> Add(inputFiles.at(fileIt).c_str());
    chain3 -> Add(inputFiles.at(fileIt).c_str());
  }
  chain2 -> BuildIndex("index");
  chain1 -> AddFriend("hodo");
  chain3 -> BuildIndex("index");
  chain1 -> AddFriend("digi");
  chain1 -> BuildIndex("index");
  std::cout << " Read " << chain1->GetEntries() << " total events in tree " << chain1->GetName() << std::endl;
  std::cout << " Read " << chain2->GetEntries() << " total events in tree " << chain2->GetName() << std::endl;
  std::cout << " Read " << chain3->GetEntries() << " total events in tree " << chain3->GetName() << std::endl;
  
  // set branches
  TreeVars treeVars;
  InitTreeVars(chain1,chain2,chain1,treeVars);
  
  
  //------------------
  // Define histogram
  TH1F* h_amp1 = new TH1F(Form("h_amp1"),"",1000,0.,1000.);
  TH1F* h_amp2 = new TH1F(Form("h_amp2"),"",1000,0.,1000.);
  TH1F* h_ampRatio = new TH1F(Form("h_ampRatio"),"",1000,0.,3.);
  TH1F* h_amp1_cut = new TH1F(Form("h_amp1_cut"),"",1000,0.,1000.);
  TH1F* h_amp2_cut = new TH1F(Form("h_amp2_cut"),"",1000,0.,1000.);
  TH1F* h_ampRatio_cut = new TH1F(Form("h_ampRatio_cut"),"",1000,0.,3.);
  std::map<std::string,TH1F*> h_amp1_Vbias;
  std::map<std::string,TH1F*> h_amp2_Vbias;
  std::map<std::string,TH1F*> h_ampRatio_Vbias;
  TProfile* p1_amp1_vs_beam_X = new TProfile("p1_amp1_vs_beam_X","",320,-20.,20.);
  TProfile* p1_amp1_vs_beam_Y = new TProfile("p1_amp1_vs_beam_Y","",320,-20.,20.);
  TProfile* p1_amp2_vs_beam_X = new TProfile("p1_amp2_vs_beam_X","",320,-20.,20.);
  TProfile* p1_amp2_vs_beam_Y = new TProfile("p1_amp2_vs_beam_Y","",320,-20.,20.);
  TProfile2D* p2_amp1_vs_beam_Y_vs_X = new TProfile2D("p2_amp1_vs_beam_Y_vs_X","",80,-20.,20.,80,-20.,20.);
  TProfile2D* p2_amp2_vs_beam_Y_vs_X = new TProfile2D("p2_amp2_vs_beam_Y_vs_X","",80,-20.,20.,80,-20.,20.);
  
  std::map<std::string,TH1F*> h_dur1;
  std::map<std::string,TH1F*> h_dur2;
  
  TH1F* h_CTR = new TH1F("h_CTR","",20000,-10.,10.);
  TH1F* h_CTR_corr = new TH1F("h_CTR_corr","",20000,-10.,10.);
  std::map<std::string,TH1F*> map_CTR_vs_Vbias_th;
  std::map<std::string,TH1F*> map_CTR_corr_vs_Vbias_th;
  std::map<int,TH1F*> map_CTR_corr_vs_amp1;
  std::map<int,TH1F*> map_CTR_corr_vs_amp2;
  std::map<int,TH1F*> map_CTR_corr_vs_ampAvg;
  
  TProfile* p_CTR_vs_amp1 = new TProfile("p_CTR_vs_amp1","",100,0.,1000.);
  TProfile* p_CTR_vs_amp2 = new TProfile("p_CTR_vs_amp2","",100,0.,1000.);
  TProfile* p_CTR_vs_ampRatio = new TProfile("p_CTR_vs_ampRatio","",100,0.,5.);  
  std::map<std::string,TProfile*> map_CTR_vs_amp1_vs_Vbias_th;
  std::map<std::string,TProfile*> map_CTR_vs_amp2_vs_Vbias_th;
  std::map<std::string,TProfile*> map_CTR_vs_ampRatio_vs_Vbias_th;
  
  TH2F* h2_beam_Y_vs_X = new TH2F("h2_beam_Y_vs_X","",80,-20.,20.,80,-20.,20.);
  TH2F* h2_beam_Y_vs_X_cut = new TH2F("h2_beam_Y_vs_X_cut","",80,-20.,20.,80,-20.,20.);
  
  
  //-----------------------
  // first loop over events
  int nEntries = chain1 -> GetEntries();
  std::vector<std::pair<float,float> > vec_Vbias;
  std::vector<float> vec_th;
  std::vector<std::pair<std::pair<float,float>,float> > pairs_Vbias_th;
  std::vector<float> vec_amp1;
  std::vector<float> vec_amp2;
  std::vector<float> vec_ampAvg;
  for(int entry = 0; entry < nEntries; ++entry)
  {
    if( entry%1000 == 0 ) std::cout << ">>> loop 1/4: reading entry " << entry << " / " << nEntries << "\r" << std::flush;
    chain1 -> GetEntry(entry);
    
    if( !AcceptEvent(treeVars,cut_beamCutType,cut_beamXMin,cut_beamXMax,cut_beamYMin,cut_beamYMax,cut_angle) ) continue;
    
    h2_beam_Y_vs_X -> Fill(treeVars.t_beamX[0],treeVars.t_beamY[0]);
    
    float Vbias1 = treeVars.t_Vbias[VbiasIndex1];
    float Vbias2 = treeVars.t_Vbias[VbiasIndex2];
    float amp1 = treeVars.t_amp[(*treeVars.t_channelId)[energyChannels.at(0)]] * 0.25;
    float amp2 = treeVars.t_amp[(*treeVars.t_channelId)[energyChannels.at(1)]] * 0.25;
    float dur1 = treeVars.t_dur[(*treeVars.t_channelId)[timeChannels.at(0)]] * 0.2;
    float dur2 = treeVars.t_dur[(*treeVars.t_channelId)[timeChannels.at(1)]] * 0.2;
    int extraIt1 = 0;
    int extraIt2 = 0;
    if( mcpChannels.at(0) == 1 ) extraIt1 = 14;
    if( mcpChannels.at(1) == 1 ) extraIt2 = 14;
    float time1 = treeVars.t_time[(*treeVars.t_channelId)[timeChannels.at(0)]+extraIt1];
    float time2 = treeVars.t_time[(*treeVars.t_channelId)[timeChannels.at(1)]+extraIt2];
    float CTR = time2 - time1;
    
    std::pair<float,float> pair_Vbias(Vbias1,Vbias2);
    std::string label_Vbias = std::string(Form("Vbias%.0fV",Vbias1));
    if( std::find(vec_Vbias.begin(),vec_Vbias.end(),pair_Vbias) == vec_Vbias.end() ) vec_Vbias.push_back(pair_Vbias);
    if( std::find(vec_th.begin(),vec_th.end(),treeVars.t_NINOthr) == vec_th.end() ) vec_th.push_back(treeVars.t_NINOthr);

    if( h_amp1_Vbias[label_Vbias] == NULL )
    {      
      h_amp1_Vbias[label_Vbias] = new TH1F(Form("h_amp1_%s",label_Vbias.c_str()),"",1000,0.,1000.);
      h_amp2_Vbias[label_Vbias] = new TH1F(Form("h_amp2_%s",label_Vbias.c_str()),"",1000,0.,1000.);
      h_ampRatio_Vbias[label_Vbias] = new TH1F(Form("h_ampRatio_%s",label_Vbias.c_str()),"",1000,0.,3.);
      
      h_dur1[label_Vbias] = new TH1F(Form("h_dur1_%s",label_Vbias.c_str()),"",1000,0.,500.);
      h_dur2[label_Vbias] = new TH1F(Form("h_dur2_%s",label_Vbias.c_str()),"",1000,0.,500.);
    }

    h_amp1 -> Fill( amp1 );
    h_amp2 -> Fill( amp2 );
    p1_amp1_vs_beam_X -> Fill( treeVars.t_beamX[0],amp1 );
    p1_amp1_vs_beam_Y -> Fill( treeVars.t_beamY[0],amp1 );
    p1_amp2_vs_beam_X -> Fill( treeVars.t_beamX[0],amp2 );
    p1_amp2_vs_beam_Y -> Fill( treeVars.t_beamY[0],amp2 );
    p2_amp1_vs_beam_Y_vs_X -> Fill( treeVars.t_beamX[0],treeVars.t_beamY[0],amp1 );
    p2_amp2_vs_beam_Y_vs_X -> Fill( treeVars.t_beamX[0],treeVars.t_beamY[0],amp2 );
    
    h_amp1_Vbias[label_Vbias] -> Fill( amp1 );
    h_amp2_Vbias[label_Vbias] -> Fill( amp2 );
    h_ampRatio_Vbias[label_Vbias] -> Fill( amp2/amp1 );
    
    if( !AcceptEventAmp(treeVars,cut_ampMin1[Vbias1],cut_ampMax1[Vbias1],cut_ampMin2[Vbias2],cut_ampMax2[Vbias2]) ) continue;
    
    h_dur1[label_Vbias] -> Fill( dur1 );
    h_dur2[label_Vbias] -> Fill( dur2 );
    
    if( !AcceptEventDur(treeVars,cut_durMin1[Vbias1],cut_durMax1[Vbias1],cut_durMin2[Vbias2],cut_durMax2[Vbias2]) ) continue;
    
    h2_beam_Y_vs_X_cut -> Fill(treeVars.t_beamX[0],treeVars.t_beamY[0]);
    
    std::string label_th = std::string(Form("th%.0fmV",treeVars.t_NINOthr));
    std::string label_Vbias_th = std::string(Form("%s_%s",label_Vbias.c_str(),label_th.c_str()));
    if( map_CTR_vs_Vbias_th[label_Vbias_th] == NULL )
    {
      std::pair<std::pair<float,float>,float> pair_Vbias_th(pair_Vbias,treeVars.t_NINOthr);
      pairs_Vbias_th.push_back(pair_Vbias_th);
      map_CTR_vs_Vbias_th[label_Vbias_th] = new TH1F(Form("h_CTR_%s",label_Vbias_th.c_str()),"",20000,-10.,10.);
    }
    map_CTR_vs_Vbias_th[label_Vbias_th] -> Fill( CTR );
    
    if( !AcceptEventVbias(treeVars,cut_VbiasMin,cut_VbiasMax) ) continue;
    if( !AcceptEventTh(treeVars,cut_NINOthrMin,cut_NINOthrMax) ) continue;
    
    h_amp1_cut -> Fill( amp1 );
    h_amp2_cut -> Fill( amp2 );
    h_ampRatio_cut -> Fill( amp2/amp1 );
    
    vec_amp1.push_back(amp1);
    vec_amp2.push_back(amp2);
    vec_ampAvg.push_back(0.5*(amp1+amp2));
  }
  std::cout << std::endl;
  
  
  // define bins for plots vs energy
  nEventsPerEnergyBin = std::min(nEventsPerEnergyBin,int(vec_amp1.size()));
  std::sort(vec_amp1.begin(),vec_amp1.end());
  std::sort(vec_amp2.begin(),vec_amp2.end());
  std::sort(vec_ampAvg.begin(),vec_ampAvg.end());
  int nBins_vs_amp = -1;
  float* bins_vs_amp1;
  float* bins_vs_amp2 ;
  float* bins_vs_ampAvg;
  if( nEventsPerEnergyBin > 0 )
  {
    nBins_vs_amp = int(vec_amp1.size()) / nEventsPerEnergyBin;
    std::cout << "nBins_vs_amp: " << nBins_vs_amp << std::endl;
    bins_vs_amp1 = new float[nBins_vs_amp+1];
    bins_vs_amp2 = new float[nBins_vs_amp+1];
    bins_vs_ampAvg = new float[nBins_vs_amp+1];
    for(int it = 0; it < nBins_vs_amp; ++it)
    {
      bins_vs_amp1[it]   = vec_amp1[it*nEventsPerEnergyBin];
      bins_vs_amp2[it]   = vec_amp2[it*nEventsPerEnergyBin];
      bins_vs_ampAvg[it] = vec_ampAvg[it*nEventsPerEnergyBin];
    }
    bins_vs_amp1[nBins_vs_amp]   = vec_amp1[nBins_vs_amp*nEventsPerEnergyBin];
    bins_vs_amp2[nBins_vs_amp]   = vec_amp2[nBins_vs_amp*nEventsPerEnergyBin];
    bins_vs_ampAvg[nBins_vs_amp] = vec_ampAvg[nBins_vs_amp*nEventsPerEnergyBin];
  }
  else
  {
    nBins_vs_amp = 1;
    bins_vs_amp1 = new float[nBins_vs_amp+1];
    bins_vs_amp2 = new float[nBins_vs_amp+1];
    bins_vs_ampAvg = new float[nBins_vs_amp+1];
    bins_vs_amp1[0]   = 0.;
    bins_vs_amp2[0]   = 0.;
    bins_vs_ampAvg[0] = 0.;
    bins_vs_amp1[1]   = 1000.;
    bins_vs_amp2[1]   = 1000.;
    bins_vs_ampAvg[1] = 1000.;
  }
  TH1F* h_nEvents_vs_amp1   = new TH1F("h_nEvents_vs_amp1",  "",nBins_vs_amp,bins_vs_amp1);
  TH1F* h_nEvents_vs_amp2   = new TH1F("h_nEvents_vs_amp2",  "",nBins_vs_amp,bins_vs_amp2);
  TH1F* h_nEvents_vs_ampAvg = new TH1F("h_nEvents_vs_ampAvg","",nBins_vs_amp,bins_vs_ampAvg);
  
  // draw plots
  TCanvas* c1;
  
  for(unsigned int it = 0; it < vec_Vbias.size(); ++it)
  {
    float Vbias1 = vec_Vbias.at(it).first;
    float Vbias2 = vec_Vbias.at(it).second;
    std::string label_Vbias = std::string(Form("Vbias%.0fV",Vbias1));
    TLatex* latex3_cuts = new TLatex(0.65,0.89,Form("V_{bias} = %.0fV",Vbias1));
    latex3_cuts -> SetNDC();
    latex3_cuts -> SetTextFont(42);
    latex3_cuts -> SetTextSize(0.03);
    latex3_cuts -> SetTextColor(kBlack);
    
    c1 = new TCanvas(Form("c1_amplitudes_%s",label_Vbias.c_str()),Form("Vbias %s",label_Vbias.c_str()),1800,1400);
    c1 -> Divide(2,2);
    c1 -> cd(1);
    gPad -> SetLogy();
    h_amp1_Vbias[label_Vbias] -> GetXaxis() -> SetRangeUser(0.,1000.);
    h_amp1_Vbias[label_Vbias] -> SetTitle(Form(";%s max. amplitude (mV); events",energyChannels.at(0).c_str()));
    h_amp1_Vbias[label_Vbias] -> Draw();
    TLine* line_cutAmpMin1 = new TLine(cut_ampMin1[Vbias1],0.,cut_ampMin1[Vbias1],1.05*h_amp1_Vbias[label_Vbias]->GetMaximum());
    TLine* line_cutAmpMax1 = new TLine(cut_ampMax1[Vbias1],0.,cut_ampMax1[Vbias1],1.05*h_amp1_Vbias[label_Vbias]->GetMaximum());
    line_cutAmpMin1 -> SetLineColor(kRed);
    line_cutAmpMax1 -> SetLineColor(kRed);
    line_cutAmpMin1 -> Draw("same");
    line_cutAmpMax1 -> Draw("same");
    latexLabel1 -> Draw("same");
    latex3_cuts -> Draw("same");
    c1 -> cd(2);
    gPad -> SetLogy();
    h_amp2_Vbias[label_Vbias] -> GetXaxis() -> SetRangeUser(0.,1000.);
    h_amp2_Vbias[label_Vbias] -> SetTitle(Form(";%s max. amplitude (mV); events",energyChannels.at(1).c_str()));
    h_amp2_Vbias[label_Vbias] -> Draw();
    TLine* line_cutAmpMin2 = new TLine(cut_ampMin2[Vbias2],0.,cut_ampMin2[Vbias2],1.05*h_amp2_Vbias[label_Vbias]->GetMaximum());
    TLine* line_cutAmpMax2 = new TLine(cut_ampMax2[Vbias2],0.,cut_ampMax2[Vbias2],1.05*h_amp2_Vbias[label_Vbias]->GetMaximum());
    line_cutAmpMin2 -> SetLineColor(kRed);
    line_cutAmpMax2 -> SetLineColor(kRed);
    line_cutAmpMin2 -> Draw("same");
    line_cutAmpMax2 -> Draw("same");
    latexLabel2 -> Draw("same");
    latex3_cuts -> Draw("same");
    gPad -> Update();
    
    c1 -> cd(3);
    gPad -> SetLogy();
    h_dur1[label_Vbias] -> GetXaxis() -> SetRangeUser(0.,200.);
    h_dur1[label_Vbias] -> SetTitle(Form(";%s pulse length (ns); events",timeChannels.at(0).c_str()));
    h_dur1[label_Vbias] -> Draw();
    TLine* line_cutDurMin1 = new TLine(cut_durMin1[Vbias1],0.,cut_durMin1[Vbias1],1.05*h_dur1[label_Vbias]->GetMaximum());
    TLine* line_cutDurMax1 = new TLine(cut_durMax1[Vbias1],0.,cut_durMax1[Vbias1],1.05*h_dur1[label_Vbias]->GetMaximum());
    line_cutDurMin1 -> SetLineColor(kRed);
    line_cutDurMax1 -> SetLineColor(kRed);
    line_cutDurMin1 -> Draw("same");
    line_cutDurMax1 -> Draw("same");
    latexLabel1 -> Draw("same");
    latex3_cuts -> Draw("same");
    c1 -> cd(4);
    gPad -> SetLogy();
    h_dur2[label_Vbias] -> GetXaxis() -> SetRangeUser(0.,200.);
    h_dur2[label_Vbias] -> SetTitle(Form(";%s pulse length (ns); events",timeChannels.at(1).c_str()));
    h_dur2[label_Vbias] -> Draw();
    TLine* line_cutDurMin2 = new TLine(cut_durMin2[Vbias2],0.,cut_durMin2[Vbias2],1.05*h_dur2[label_Vbias]->GetMaximum());
    TLine* line_cutDurMax2 = new TLine(cut_durMax2[Vbias2],0.,cut_durMax2[Vbias2],1.05*h_dur2[label_Vbias]->GetMaximum());
    line_cutDurMin2 -> SetLineColor(kRed);
    line_cutDurMax2 -> SetLineColor(kRed);
    line_cutDurMin2 -> Draw("same");
    line_cutDurMax2 -> Draw("same");
    latexLabel2 -> Draw("same");
    latex3_cuts -> Draw("same");
    gPad -> Update();
    
    c1 -> Print(Form("%s/c__%s__amp-dur__config%d__%s.png",plotDir.c_str(),label1.c_str(),configuration,label_Vbias.c_str()));
  }
  
  TCanvas* c1_amp1_beam_X_Y = new TCanvas("c1_amp1_beam_X_Y","amplitude profile",2100,900);
  c1_amp1_beam_X_Y -> Divide(2,1);
  c1_amp1_beam_X_Y -> cd(1);
  gPad -> SetGridx();
  p1_amp1_vs_beam_X -> SetTitle(";beam X (mm);average amp_{xtal1}");
  p1_amp1_vs_beam_X -> Draw();
  c1_amp1_beam_X_Y -> cd(2);
  gPad -> SetGridx();
  p1_amp1_vs_beam_Y -> SetTitle(";beam Y (mm);average amp_{xtal1}");
  p1_amp1_vs_beam_Y -> Draw();
  gPad -> Update();
  c1_amp1_beam_X_Y -> Print(Form("%s/c__%s__amplitude1Profile__config%d.png",plotDir.c_str(),label1.c_str(),configuration));
  
  TCanvas* c1_amp2_beam_X_Y = new TCanvas("c1_amp2_beam_X_Y","amplitude profile",2100,900);
  c1_amp2_beam_X_Y -> Divide(2,1);
  c1_amp2_beam_X_Y -> cd(1);
  gPad -> SetGridx();
  p1_amp2_vs_beam_X -> SetTitle(";beam X (mm);average amp_{xtal2}");
  p1_amp2_vs_beam_X -> Draw();
  c1_amp2_beam_X_Y -> cd(2);
  gPad -> SetGridx();
  p1_amp2_vs_beam_Y -> SetTitle(";beam Y (mm);average amp_{xtal2}");
  p1_amp2_vs_beam_Y -> Draw();
  gPad -> Update();
  c1_amp2_beam_X_Y -> Print(Form("%s/c__%s__amplitude2Profile__config%d.png",plotDir.c_str(),label1.c_str(),configuration));
  
  TCanvas* c1_amp_beam_Y_vs_X = new TCanvas("c1_amp_beam_Y_vs_X","amplitude profile",2100,900);
  c1_amp_beam_Y_vs_X -> Divide(2,1);
  c1_amp_beam_Y_vs_X -> cd(1);
  TH1F* hPad = (TH1F*)( gPad->DrawFrame(-20.,-20.,20.,20.) );
  hPad -> SetTitle(";beam X (mm);beam Y (mm);average amp_{xtal1}");
  hPad -> Draw();
  gPad -> SetLogz();
  p2_amp1_vs_beam_Y_vs_X -> Draw("colz,same");
  c1_amp_beam_Y_vs_X -> cd(2);
  hPad = (TH1F*)( gPad->DrawFrame(-20.,-20.,20.,20.) );
  hPad -> SetTitle(";beam X (mm);beam Y (mm);average amp_{xtal2}");
  hPad -> Draw();
  gPad -> SetLogz();
  p2_amp2_vs_beam_Y_vs_X -> Draw("colz,same");
  gPad -> Update();
  c1_amp_beam_Y_vs_X -> Print(Form("%s/c__%s__amplitudeProfile__config%d.png",plotDir.c_str(),label1.c_str(),configuration));
  
  TCanvas* c1_beam_Y_vs_X = new TCanvas("c1_beam_Y_vs_X","beam profile",2100,900);
  c1_beam_Y_vs_X -> Divide(2,1);
  c1_beam_Y_vs_X -> cd(1);
  hPad = (TH1F*)( gPad->DrawFrame(-20.,-20.,20.,20.) );
  hPad -> SetTitle(";beam X (mm);beam Y (mm);events");
  hPad -> Draw();
  h2_beam_Y_vs_X -> Draw("colz,same");
  c1_beam_Y_vs_X -> cd(2);
  hPad = (TH1F*)( gPad->DrawFrame(-20.,-20.,20.,20.) );
  hPad -> SetTitle(";beam X (mm);beam Y (mm);events");
  hPad -> Draw();
  h2_beam_Y_vs_X_cut -> Draw("colz,same");
  gPad -> Update();
  c1_beam_Y_vs_X -> Print(Form("%s/c__%s__beamProfile__config%d.png",plotDir.c_str(),label1.c_str(),configuration));
  
  
  
  //---------------------------
  // CTR Vbias / threshold scan
  float VbiasMax = -999.;
  float thMax = -999.;
  std::string label_Vbias_th_max = "";
  
  std::map<std::string,float> map_mean_vs_Vbias_th;
  std::map<std::string,TGraph*> g_mean_vs_Vbias;
  std::map<std::string,TGraph*> g_CTR_vs_Vbias;
  std::map<std::string,TGraph*> g_mean_vs_th;
  std::map<std::string,TGraph*> g_CTR_vs_th;
  std::map<std::string,float> map_mean1_MCP_vs_Vbias_th;
  std::map<std::string,TGraph*> g_mean1_MCP_vs_Vbias;
  std::map<std::string,TGraph*> g_CTR1_MCP_vs_Vbias;
  std::map<std::string,TGraph*> g_mean1_MCP_vs_th;
  std::map<std::string,TGraph*> g_CTR1_MCP_vs_th;
  std::map<std::string,float> map_mean2_MCP_vs_Vbias_th;
  std::map<std::string,TGraph*> g_mean2_MCP_vs_Vbias;
  std::map<std::string,TGraph*> g_CTR2_MCP_vs_Vbias;
  std::map<std::string,TGraph*> g_mean2_MCP_vs_th;
  std::map<std::string,TGraph*> g_CTR2_MCP_vs_th;
  
  std::sort(pairs_Vbias_th.begin(),pairs_Vbias_th.end());
  
  std::cout << "size: " <<  pairs_Vbias_th.size() << std::endl;
  for(unsigned int it = 0; it < pairs_Vbias_th.size(); ++it)
  {
    float Vbias1 = pairs_Vbias_th.at(it).first.first;
    float Vbias2 = pairs_Vbias_th.at(it).first.second;
    float th = pairs_Vbias_th.at(it).second;
    std::string label_Vbias = std::string(Form("Vbias%.0fV",Vbias1));
    std::string label_th    = std::string(Form("th%.0fmV",th));
    std::string label_Vbias_th = label_Vbias + "_" + label_th;
    std::cout << label_Vbias_th << std::endl;
    if( th >= thMax )
    {
      thMax = th;
      if( 0.5*(Vbias1+Vbias2) >= VbiasMax )
      {
        VbiasMax = 0.5*(Vbias1+Vbias2);
        label_Vbias_th_max = label_Vbias_th;
      } 
    }
    
    TH1F* histo = map_CTR_vs_Vbias_th[label_Vbias_th];
    float* vals = new float[4];
    FindSmallestInterval(vals,histo,0.68,true); 
    
    float mean = vals[0];      
    float min = vals[2];
    float max = vals[3];
    float delta = max-min;
    float sigma = 0.5*delta;
    float effSigma = sigma;
    
    map_mean_vs_Vbias_th[label_Vbias_th] = mean;
    
    std::cout << "label_th: " << label_th << std::endl;
    if( g_mean_vs_Vbias[label_th] == NULL )
    {
      std::cout << "creating graph vs vbias" << std::endl;
      g_mean_vs_Vbias[label_th] = new TGraph();
      g_CTR_vs_Vbias[label_th] = new TGraph();
    }
    g_mean_vs_Vbias[label_th] -> SetPoint(g_mean_vs_Vbias[label_th]->GetN(),Vbias1,mean);
    
    if( (mcpChannels.at(0)==1 && mcpChannels.at(1)==0) || (mcpChannels.at(0)==0 && mcpChannels.at(1)==1))//reference is the MCP
      g_CTR_vs_Vbias[label_th] -> SetPoint(g_CTR_vs_Vbias[label_th]->GetN(),Vbias1,sqrt(effSigma*effSigma - MCPIntrinsic*MCPIntrinsic)*1000.);
    else
      g_CTR_vs_Vbias[label_th] -> SetPoint(g_CTR_vs_Vbias[label_th]->GetN(),Vbias1,effSigma/sqrt(2)*1000.);
    
    std::cout << "label_Vbias: " << label_Vbias << std::endl;
    if( g_mean_vs_th[label_Vbias] == NULL )
    {
      std::cout << "creating graph vs th" << std::endl;
      g_mean_vs_th[label_Vbias] = new TGraph();
      g_CTR_vs_th[label_Vbias] = new TGraph();
    }
    g_mean_vs_th[label_Vbias] -> SetPoint(g_mean_vs_th[label_Vbias]->GetN(),th,mean);
    if( (mcpChannels.at(0)==1 && mcpChannels.at(1)==0) || (mcpChannels.at(0)==0 && mcpChannels.at(1)==1))//reference is the MCP
      g_CTR_vs_th[label_Vbias] -> SetPoint(g_CTR_vs_th[label_Vbias]->GetN(),th,sqrt(effSigma*effSigma - MCPIntrinsic*MCPIntrinsic)*1000.);
    else
      g_CTR_vs_th[label_Vbias] -> SetPoint(g_CTR_vs_th[label_Vbias]->GetN(),th,effSigma/sqrt(2)*1000.);
  }
  
  
  
  //------------------------
  // second loop over events
  if( maxStep < 2) { if( popupPlots ) theApp -> Run(); return 0; }
  
  for(int entry = 0; entry < nEntries; ++entry)
  {
    if( entry%1000 == 0 ) std::cout << ">>> loop 2/4: reading entry " << entry << " / " << nEntries << "\r" << std::flush;
    chain1 -> GetEntry(entry);
    
    if( !AcceptEvent(treeVars,cut_beamCutType,cut_beamXMin,cut_beamXMax,cut_beamYMin,cut_beamYMax,cut_angle) ) continue;
    
    float Vbias1 = treeVars.t_Vbias[VbiasIndex1];
    float Vbias2 = treeVars.t_Vbias[VbiasIndex2];
    float th = treeVars.t_NINOthr;
    std::string label_Vbias = std::string(Form("Vbias%.0fV",Vbias1));
    std::string label_th    = std::string(Form("th%.0fmV",th));
    std::string label_Vbias_th = label_Vbias + "_" + label_th;
    
    float amp1 = treeVars.t_amp[(*treeVars.t_channelId)[energyChannels.at(0)]] * 0.25;
    float amp2 = treeVars.t_amp[(*treeVars.t_channelId)[energyChannels.at(1)]] * 0.25;
    int extraIt1 = 0;
    int extraIt2 = 0;
    if( mcpChannels.at(0) == 1 ) extraIt1 = 14;
    if( mcpChannels.at(1) == 1 ) extraIt2 = 14;
    float time1 = treeVars.t_time[(*treeVars.t_channelId)[timeChannels.at(0)]+extraIt1];
    float time2 = treeVars.t_time[(*treeVars.t_channelId)[timeChannels.at(1)]+extraIt2];
    float CTR = (time2 - time1) - map_mean_vs_Vbias_th[label_Vbias_th] + map_mean_vs_Vbias_th[label_Vbias_th_max];
    
    if( !AcceptEventAmp(treeVars,cut_ampMin1[Vbias1],cut_ampMax1[Vbias1],cut_ampMin2[Vbias2],cut_ampMax2[Vbias2]) ) continue;
    if( !AcceptEventDur(treeVars,cut_durMin1[Vbias1],cut_durMax1[Vbias1],cut_durMin2[Vbias2],cut_durMax2[Vbias2]) ) continue;

    if( CTR > -5. && CTR < 5. )
    {
      if( map_CTR_vs_amp1_vs_Vbias_th[label_Vbias_th] == NULL )
      {
        map_CTR_vs_amp1_vs_Vbias_th[label_Vbias_th] = new TProfile(Form("p_CTR_vs_amp1_%s",label_Vbias_th.c_str()),"",100,0.,1000.);
        map_CTR_vs_amp2_vs_Vbias_th[label_Vbias_th] = new TProfile(Form("p_CTR_vs_amp2_%s",label_Vbias_th.c_str()),"",100,0.,1000.);
        map_CTR_vs_ampRatio_vs_Vbias_th[label_Vbias_th] = new TProfile(Form("p_CTR_vs_ampRatio_%s",label_Vbias_th.c_str()),"",100,0.,5.);
      }
      map_CTR_vs_amp1_vs_Vbias_th[label_Vbias_th] -> Fill( amp1,CTR );
      map_CTR_vs_amp2_vs_Vbias_th[label_Vbias_th] -> Fill( amp2,CTR );
      map_CTR_vs_ampRatio_vs_Vbias_th[label_Vbias_th] -> Fill( amp2/amp1,CTR );
    }
    
    if( !AcceptEventVbias(treeVars,cut_VbiasMin,cut_VbiasMax) ) continue;
    if( !AcceptEventTh(treeVars,cut_NINOthrMin,cut_NINOthrMax) ) continue;
    
    h_CTR -> Fill( CTR );
    h_ampRatio -> Fill(amp2/amp1);
    
    if( CTR > -5. && CTR < 5. )
    {
      p_CTR_vs_amp1 -> Fill( amp1,CTR );
      p_CTR_vs_amp2 -> Fill( amp2,CTR );
      p_CTR_vs_ampRatio -> Fill( amp2/amp1,CTR );
    }
  }
  std::cout << std::endl;
  
  
  
  //------------------
  // fit CTR histogram
  
  float* vals = new float[4];
  FindSmallestInterval(vals,h_CTR,0.68,true); 
  h_CTR -> Rebin(rebin);
  
  float norm = h_CTR -> GetMaximum();
  float mean = vals[0];
  float min = vals[2];
  float max = vals[3];
  float delta = max-min;
  float sigma = 0.5*delta;
  float effSigma = sigma;
  min = min - 2.*delta;
  max = max + 2.*delta;

  TCanvas* c1_CTR = new TCanvas("c1_CTR","CTR",1400,1200);
  h_CTR -> SetTitle(";#Deltat = time_{xtal2} #minus time_{xtal1} (ns);events");
  h_CTR -> SetMarkerStyle(24);
  h_CTR -> SetMarkerColor(kRed);
  h_CTR -> SetLineColor(kRed);
  h_CTR -> GetXaxis() -> SetRangeUser(min,max);
  h_CTR -> Draw("PE");
  latexLabel12 -> Draw("same");
  
  // gaus fit
  std::string gausName = Form("fitFunc_gaus");
  TF1* fitFunc_gaus = new TF1(gausName.c_str(),"[0]*exp(-1.*(x-[1])*(x-[1])/(2.*[2]*[2]))",mean-sigma,mean+sigma);
  fitFunc_gaus -> SetNpx(10000);
  fitFunc_gaus -> SetParameters(norm,mean,sigma);
  h_CTR -> Fit(gausName.c_str(),"QNRSL");
  norm = fitFunc_gaus -> GetParameter(0);
  mean = fitFunc_gaus -> GetParameter(1);
  sigma = fitFunc_gaus -> GetParameter(2);
  float sigmaErr = fitFunc_gaus -> GetParError(2);
  
  TLatex* latex22;
  if( (mcpChannels.at(0)==1 && mcpChannels.at(1)==0) || (mcpChannels.at(0)==0 && mcpChannels.at(1)==1))//reference is the MCP
    latex22= new TLatex(0.16,0.55,Form("#sigma_{single}^{gaus} = (%.1f #pm %.1f) ps",
    sqrt(sigma*sigma - MCPIntrinsic*MCPIntrinsic)*1000.,
    fabs(sigmaErr*1000)));
  else
    latex22= new TLatex(0.16,0.55,Form("#sigma_{single}^{gaus} = (%.1f #pm %.1f) ps",fabs(sigma*1000)/sqrt(2),fabs(sigmaErr*1000)/sqrt(2)));
  latex22 -> SetNDC();
  latex22 -> SetTextFont(42);
  latex22 -> SetTextSize(0.03);
  latex22 -> SetTextColor(kRed);
  latex22 -> Draw("same");
  
  // crystal ball fit
  std::string cbName = Form("fitFunc_cb");
  TF1* fitFunc_cb = new TF1(cbName.c_str(),crystalBallLowHigh,mean-4.*sigma,mean+4.*sigma,8);
  fitFunc_cb -> SetNpx(10000);
  fitFunc_cb -> SetParameters(norm,mean,sigma,1.5,10.,1.5,10.);
  h_CTR -> Fit(cbName.c_str(),"QNRSL");
  norm = fitFunc_cb -> GetParameter(0);
  mean = fitFunc_cb -> GetParameter(1);
  sigma = fitFunc_cb -> GetParameter(2);
  sigmaErr = fitFunc_cb -> GetParError(2);
  fitFunc_cb -> Draw("same");
  
  TLatex* latex2;
  if( (mcpChannels.at(0)==1 && mcpChannels.at(1)==0) || (mcpChannels.at(0)==0 && mcpChannels.at(1)==1))//reference is the MCP
    latex2 = new TLatex(0.16,0.89,Form("eff. #sigma_{single} = %.1f ps",
    sqrt(effSigma*effSigma - MCPIntrinsic*MCPIntrinsic)*1000.));
  else
    latex2 = new TLatex(0.16,0.89,Form("eff. #sigma_{single} = %.1f ps",fabs(effSigma*1000)/sqrt(2)));
  latex2 -> SetNDC();
  latex2 -> SetTextFont(42);
  latex2 -> SetTextSize(0.05);
  latex2 -> SetTextColor(kRed);
  latex2 -> Draw("same");
  
  TLatex* latex21;
  if( (mcpChannels.at(0)==1 && mcpChannels.at(1)==0) || (mcpChannels.at(0)==0 && mcpChannels.at(1)==1))//reference is the MCP
    latex21 = new TLatex(0.16,0.40,Form("#sigma_{single}^{c.b.} = (%.1f #pm %.1f) ps",
    sqrt(sigma*sigma - MCPIntrinsic*MCPIntrinsic)*1000.,
    fabs(sigmaErr*1000)));
  else
    latex21 = new TLatex(0.16,0.40,Form("#sigma_{single}^{c.b.} = (%.1f #pm %.1f) ps",fabs(sigma*1000)/sqrt(2),fabs(sigmaErr*1000)/sqrt(2)));
  latex21 -> SetNDC();
  latex21 -> SetTextFont(42);
  latex21 -> SetTextSize(0.03);
  latex21 -> SetTextColor(kRed);
  latex21 -> Draw("same");
  
  gPad -> Update();
  
  
  std::map<std::string,TCanvas*> c1_scans;
  for(unsigned int it = 0; it < vec_Vbias.size(); ++it)
  {
    float Vbias1 = vec_Vbias.at(it).first;
    std::string label_Vbias = std::string(Form("Vbias%.0fV",Vbias1));
    
    c1_scans[label_Vbias] = new TCanvas(Form("c1_CTR_vs_th_%s",label_Vbias.c_str()),Form("threshold scan -- %s",label_Vbias.c_str()),2100,900);
    c1_scans[label_Vbias] -> Divide(2,1);
    c1_scans[label_Vbias] -> cd(1);
    hPad = (TH1F*)( gPad->DrawFrame(0.,-0.25,1100.,0.25) );
    hPad -> SetTitle(";NINO th. (mV); #LT #Deltat #GT (ns)");
    hPad -> Draw();
    gPad -> SetGridy();
    g_mean_vs_th[label_Vbias] -> SetMarkerStyle(24);
    g_mean_vs_th[label_Vbias] -> SetMarkerColor(kRed);
    g_mean_vs_th[label_Vbias] -> SetLineColor(kRed);
    g_mean_vs_th[label_Vbias] -> Draw("PL,same");
    latexLabel12 -> Draw("same");
    c1_scans[label_Vbias] -> cd(2);
    hPad = (TH1F*)( gPad->DrawFrame(0.,20,1100.,150.) );
    hPad -> SetTitle(";NINO th. (mV); eff. #sigma_{single} (ps)");
    hPad -> GetYaxis() -> SetNdivisions(520);
    gPad -> SetGridy();
    hPad -> Draw();
    g_CTR_vs_th[label_Vbias] -> SetMarkerStyle(24);
    g_CTR_vs_th[label_Vbias] -> SetMarkerColor(kRed);
    g_CTR_vs_th[label_Vbias] -> SetLineColor(kRed);
    g_CTR_vs_th[label_Vbias] -> Draw("PL,same");
    latexLabel12 -> Draw("same");    
    gPad -> Update();
  }

  for(unsigned int it = 0; it < vec_th.size(); ++it)
  {
    float th = vec_th.at(it);
    std::string label_th = std::string(Form("th%.0fmV",th));
    
    c1_scans[label_th] = new TCanvas(Form("c1_CTR_vs_Vbias_%s",label_th.c_str()),Form("Vbias scan -- %s",label_th.c_str()),2100,900);
    c1_scans[label_th] -> Divide(2,1);
    c1_scans[label_th] -> cd(1);
    hPad = (TH1F*)( gPad->DrawFrame(25.,-0.25,60.,0.25) );
    hPad -> SetTitle(";V_{bias} (V); #LT #Deltat #GT (ns)");
    gPad -> SetGridy();
    hPad -> Draw();
    g_mean_vs_Vbias[label_th] -> SetMarkerStyle(24);
    g_mean_vs_Vbias[label_th] -> SetMarkerColor(kRed);
    g_mean_vs_Vbias[label_th] -> SetLineColor(kRed);
    g_mean_vs_Vbias[label_th] -> Draw("PL,same");
    latexLabel12 -> Draw("same");
    c1_scans[label_th] -> cd(2);
    hPad = (TH1F*)( gPad->DrawFrame(25.,20.,60.,150.) );
    hPad -> SetTitle(";V_{bias} (V); eff. #sigma_{single} (ps)");
    hPad -> GetYaxis() -> SetNdivisions(520);
    gPad -> SetGridy();
    hPad -> Draw();
    g_CTR_vs_Vbias[label_th] -> SetMarkerStyle(24);
    g_CTR_vs_Vbias[label_th] -> SetMarkerColor(kRed);
    g_CTR_vs_Vbias[label_th] -> SetLineColor(kRed);
    g_CTR_vs_Vbias[label_th] -> Draw("PL,same");
    latexLabel12 -> Draw("same");    
    gPad -> Update();
  }
  
  
  std::map<std::string,TF1*> map_fitFunc_corrRatio_vs_Vbias_th;
  std::map<std::string,TF1*> map_fitFunc_corrAmp1_vs_Vbias_th;
  std::map<std::string,TF1*> map_fitFunc_corrAmp2_vs_Vbias_th;
  for(std::map<std::string,TProfile*>::const_iterator mapIt = map_CTR_vs_amp1_vs_Vbias_th.begin(); mapIt != map_CTR_vs_amp1_vs_Vbias_th.end(); ++mapIt)
  {
    std::cout << ">>> " << mapIt -> first << std::endl;
    
    map_fitFunc_corrAmp1_vs_Vbias_th[mapIt->first] = new TF1(Form("fitFunc_corrAmp1_%s",mapIt->first.c_str()),"[0]*log([1]*x)+[2]",0.,1000.);
    map_fitFunc_corrAmp1_vs_Vbias_th[mapIt->first] -> SetParameters(0.05,0.0000001,0.);
    map_CTR_vs_amp1_vs_Vbias_th[mapIt->first] -> Fit(Form("fitFunc_corrAmp1_%s",mapIt->first.c_str()),"QNS+","",
                                                         h_amp1->GetMean()-4.*h_amp1->GetRMS(),
                                                         h_amp1->GetMean()+2.*h_amp1->GetRMS());
    c1 = new TCanvas(Form("c1_CTR_vs_amp1_%s",mapIt->first.c_str()),Form("c1_CTR_vs_amp1_%s",mapIt->first.c_str()));
    if( mcpChannels.at(0)==1)
      map_CTR_vs_amp1_vs_Vbias_th[mapIt->first] -> SetTitle(";amp_{MCP};CTR (ps)");
    else
      map_CTR_vs_amp1_vs_Vbias_th[mapIt->first] -> SetTitle(";amp_{xtal1};CTR (ps)");
    map_CTR_vs_amp1_vs_Vbias_th[mapIt->first] -> Draw();    
    map_fitFunc_corrAmp1_vs_Vbias_th[mapIt->first] -> Draw("same");
    c1 -> Print(Form("%s/c__%s__amplitudeWalk1__config%d__%s.png",plotDir.c_str(),label1.c_str(),configuration,mapIt->first.c_str()));
    

    map_fitFunc_corrAmp2_vs_Vbias_th[mapIt->first] = new TF1(Form("fitFunc_corrAmp2_%s",mapIt->first.c_str()),"[0]*log([1]*x)+[2]",0.,1000.);
    map_fitFunc_corrAmp2_vs_Vbias_th[mapIt->first] -> SetParameters(0.05,0.0000001,0.);
    map_CTR_vs_amp2_vs_Vbias_th[mapIt->first] -> Fit(Form("fitFunc_corrAmp2_%s",mapIt->first.c_str()),"QNS+","",
                                                         h_amp2->GetMean()-4.*h_amp2->GetRMS(),
                                                         h_amp2->GetMean()+2.*h_amp2->GetRMS());
    c1 = new TCanvas(Form("c1_CTR_vs_amp2_%s",mapIt->first.c_str()),Form("c1_CTR_vs_amp2_%s",mapIt->first.c_str()));
    if( mcpChannels.at(1)==1)
      map_CTR_vs_amp2_vs_Vbias_th[mapIt->first] -> SetTitle(";amp_{MCP};CTR (ps)");
    else
      map_CTR_vs_amp2_vs_Vbias_th[mapIt->first] -> SetTitle(";amp_{xtal2};CTR (ps)");
    map_CTR_vs_amp2_vs_Vbias_th[mapIt->first] -> Draw();    
    map_fitFunc_corrAmp2_vs_Vbias_th[mapIt->first] -> Draw("same");
    c1 -> Print(Form("%s/c__%s__amplitudeWalk2__config%d__%s.png",plotDir.c_str(),label1.c_str(),configuration,mapIt->first.c_str()));


    map_fitFunc_corrRatio_vs_Vbias_th[mapIt->first] = new TF1(Form("fitFunc_corrRatio_%s",mapIt->first.c_str()),"[0]*log([1]*x)+[2]",0.,5.);
    map_fitFunc_corrRatio_vs_Vbias_th[mapIt->first] -> SetParameters(-0.2,0.1,0.);
    map_CTR_vs_ampRatio_vs_Vbias_th[mapIt->first] -> Fit(Form("fitFunc_corrRatio_%s",mapIt->first.c_str()),"QNS+","",
                                                         h_ampRatio->GetMean()-2.*h_ampRatio->GetRMS(),
                                                         h_ampRatio->GetMean()+2.*h_ampRatio->GetRMS());
    
    c1 = new TCanvas(Form("c1_CTR_vs_ampRatio_%s",mapIt->first.c_str()),Form("c1_CTR_vs_ampRatio_%s",mapIt->first.c_str()));
    if( mcpChannels.at(0) == 1 && mcpChannels.at(1)== 0 )
      map_CTR_vs_ampRatio_vs_Vbias_th[mapIt->first] -> SetTitle(";amp_{xtal2} / amp_{MCP};CTR (ps)");
    else if( mcpChannels.at(0) == 0 && mcpChannels.at(1)== 1 )
      map_CTR_vs_ampRatio_vs_Vbias_th[mapIt->first] -> SetTitle(";amp_{MCP} / amp_{xtal1};CTR (ps)");
    else if( mcpChannels.at(0) == 0 && mcpChannels.at(1)== 0 )
      map_CTR_vs_ampRatio_vs_Vbias_th[mapIt->first] -> SetTitle(";amp_{xtal2} / amp_{xtal1};CTR (ps)");
    map_CTR_vs_ampRatio_vs_Vbias_th[mapIt->first] -> Draw();    
    map_fitFunc_corrRatio_vs_Vbias_th[mapIt->first] -> Draw("same");
    c1 -> Print(Form("%s/c__%s__amplitudeWalk__config%d__%s.png",plotDir.c_str(),label1.c_str(),configuration,mapIt->first.c_str()));
  }
  gPad -> Update();
  
 
  //------------------------
  // third loop over events
  if( maxStep < 3) { if( popupPlots ) theApp -> Run(); return 0; }
  
  for(int entry = 0; entry < nEntries; ++entry)
  {
    if( entry%1000 == 0 ) std::cout << ">>> loop 3/4: reading entry " << entry << " / " << nEntries << "\r" << std::flush;
    chain1 -> GetEntry(entry);

    if( !AcceptEvent(treeVars,cut_beamCutType,cut_beamXMin,cut_beamXMax,cut_beamYMin,cut_beamYMax,cut_angle) ) continue;
    
    float Vbias1 = treeVars.t_Vbias[VbiasIndex1];
    float Vbias2 = treeVars.t_Vbias[VbiasIndex2];
    float th = treeVars.t_NINOthr;
    std::string label_Vbias = std::string(Form("Vbias%.0fV",Vbias1));
    std::string label_th    = std::string(Form("th%.0fmV",th));
    std::string label_Vbias_th = label_Vbias + "_" + label_th;
    
    float amp1 = treeVars.t_amp[(*treeVars.t_channelId)[energyChannels.at(0)]] * 0.25;
    float amp2 = treeVars.t_amp[(*treeVars.t_channelId)[energyChannels.at(1)]] * 0.25;
    int extraIt1 = 0;
    int extraIt2 = 0;
    if( mcpChannels.at(0) == 1 ) extraIt1 = 14;
    if( mcpChannels.at(1) == 1 ) extraIt2 = 14;
    float time1 = treeVars.t_time[(*treeVars.t_channelId)[timeChannels.at(0)]+extraIt1];
    float time2 = treeVars.t_time[(*treeVars.t_channelId)[timeChannels.at(1)]+extraIt2];
    float CTR = (time2 - time1);
    float CTR_corr = CTR;
    //The presence of one MCP modifies the amp walk correction
      if( mcpChannels.at(0)==1 && mcpChannels.at(1)==0)//Amp Walk correction on the second channel
      // CTR_corr = CTR -
      //            map_fitFunc_corrAmp2_vs_Vbias_th[label_Vbias_th]->Eval(amp2) +
      //            map_fitFunc_corrAmp2_vs_Vbias_th[label_Vbias_th]->Eval(h_amp2_cut->GetMean());
      CTR_corr = CTR -
                 map_CTR_vs_amp2_vs_Vbias_th[label_Vbias_th]->GetBinContent(map_CTR_vs_amp2_vs_Vbias_th[label_Vbias_th]->FindBin(amp2)) +
                 map_CTR_vs_amp2_vs_Vbias_th[label_Vbias_th]->GetBinContent(map_CTR_vs_amp2_vs_Vbias_th[label_Vbias_th]->FindBin(h_amp2_cut->GetMean())) -
                 map_CTR_vs_amp1_vs_Vbias_th[label_Vbias_th]->GetBinContent(map_CTR_vs_amp1_vs_Vbias_th[label_Vbias_th]->FindBin(amp1)) +
                 map_CTR_vs_amp1_vs_Vbias_th[label_Vbias_th]->GetBinContent(map_CTR_vs_amp1_vs_Vbias_th[label_Vbias_th]->FindBin(h_amp1_cut->GetMean()));
     else if( mcpChannels.at(0)==0 && mcpChannels.at(1)==1)//Amp Walk correction on the first channel
       // CTR_corr = CTR -
       //            map_fitFunc_corrAmp1_vs_Vbias_th[label_Vbias_th]->Eval(amp1) +
       //            map_fitFunc_corrAmp1_vs_Vbias_th[label_Vbias_th]->Eval(h_amp1_cut->GetMean());
       CTR_corr = CTR -
         map_CTR_vs_amp1_vs_Vbias_th[label_Vbias_th]->GetBinContent(map_CTR_vs_amp1_vs_Vbias_th[label_Vbias_th]->FindBin(amp1)) +
         map_CTR_vs_amp1_vs_Vbias_th[label_Vbias_th]->GetBinContent(map_CTR_vs_amp1_vs_Vbias_th[label_Vbias_th]->FindBin(h_amp1_cut->GetMean())) -
         map_CTR_vs_amp2_vs_Vbias_th[label_Vbias_th]->GetBinContent(map_CTR_vs_amp2_vs_Vbias_th[label_Vbias_th]->FindBin(amp2)) +
         map_CTR_vs_amp2_vs_Vbias_th[label_Vbias_th]->GetBinContent(map_CTR_vs_amp2_vs_Vbias_th[label_Vbias_th]->FindBin(h_amp2_cut->GetMean()));
     else//Or there are 2 MCP either NO MCP
       if( map_fitFunc_corrRatio_vs_Vbias_th[label_Vbias_th] != NULL )
         CTR_corr = CTR -
                    map_fitFunc_corrRatio_vs_Vbias_th[label_Vbias_th]->Eval(amp2/amp1) +
                    map_fitFunc_corrRatio_vs_Vbias_th[label_Vbias_th]->Eval(h_ampRatio_cut->GetMean());
       // CTR_corr = CTR -
       //            map_CTR_vs_ampRatio_vs_Vbias_th[label_Vbias_th]->GetBinContent(map_CTR_vs_ampRatio_vs_Vbias_th[label_Vbias_th]->FindBin(amp2/amp1)) + 
       //            map_CTR_vs_ampRatio_vs_Vbias_th[label_Vbias_th]->GetBinContent(map_CTR_vs_ampRatio_vs_Vbias_th[label_Vbias_th]->FindBin(h_ampRatio_cut->GetMean()));
       // CTR_corr = CTR -
       //   map_CTR_vs_amp1_vs_Vbias_th[label_Vbias_th]->GetBinContent(map_CTR_vs_amp1_vs_Vbias_th[label_Vbias_th]->FindBin(amp1)) +
       //   map_CTR_vs_amp1_vs_Vbias_th[label_Vbias_th]->GetBinContent(map_CTR_vs_amp1_vs_Vbias_th[label_Vbias_th]->FindBin(h_amp1_cut->GetMean())) -
       //   map_CTR_vs_amp2_vs_Vbias_th[label_Vbias_th]->GetBinContent(map_CTR_vs_amp2_vs_Vbias_th[label_Vbias_th]->FindBin(amp2)) +
       //   map_CTR_vs_amp2_vs_Vbias_th[label_Vbias_th]->GetBinContent(map_CTR_vs_amp2_vs_Vbias_th[label_Vbias_th]->FindBin(h_amp2_cut->GetMean()));
       
    if( !AcceptEventAmp(treeVars,cut_ampMin1[Vbias1],cut_ampMax1[Vbias1],cut_ampMin2[Vbias2],cut_ampMax2[Vbias2]) ) continue;
    if( !AcceptEventDur(treeVars,cut_durMin1[Vbias1],cut_durMax1[Vbias1],cut_durMin2[Vbias2],cut_durMax2[Vbias2]) ) continue;
    
    if( map_CTR_corr_vs_Vbias_th[label_Vbias_th] == NULL )
    {
      map_CTR_corr_vs_Vbias_th[label_Vbias_th] = new TH1F(Form("h_CTR_corr_%s",label_Vbias_th.c_str()),"",20000,-10.,10.);
    }
    map_CTR_corr_vs_Vbias_th[label_Vbias_th] -> Fill( CTR_corr );
  }
  std::cout << std::endl;
  
  
  
  //------------------------
  // CTR corr threshold scan
  
  std::map<std::string,float> map_mean_corr_vs_Vbias_th;
  std::map<std::string,TGraph*> g_mean_corr_vs_Vbias;
  std::map<std::string,TGraph*> g_CTR_corr_vs_Vbias;
  std::map<std::string,TGraph*> g_mean_corr_vs_th;
  std::map<std::string,TGraph*> g_CTR_corr_vs_th;  

  for(unsigned int it = 0; it < pairs_Vbias_th.size(); ++it)
  {
    float Vbias1 = pairs_Vbias_th.at(it).first.first;
    float th = pairs_Vbias_th.at(it).second;
    std::string label_Vbias = std::string(Form("Vbias%.0fV",Vbias1));
    std::string label_th    = std::string(Form("th%.0fmV",th));
    std::string label_Vbias_th = label_Vbias + "_" + label_th;
    std::cout << label_Vbias_th << std::endl;
    
    TH1F* histo = map_CTR_corr_vs_Vbias_th[label_Vbias_th];
    float* vals = new float[4];
    FindSmallestInterval(vals,histo,0.68,true); 
    
    float mean = vals[0];      
    float min = vals[2];
    float max = vals[3];
    float delta = max-min;
    float sigma = 0.5*delta;
    float effSigma = sigma;
    
    map_mean_corr_vs_Vbias_th[label_Vbias_th] = mean;
    
    if( g_mean_corr_vs_Vbias[label_th] == NULL )
    {
      g_mean_corr_vs_Vbias[label_th] = new TGraph();
      g_CTR_corr_vs_Vbias[label_th] = new TGraph();
    }
    
    g_mean_corr_vs_Vbias[label_th] -> SetPoint(g_mean_corr_vs_Vbias[label_th]->GetN(),Vbias1,mean);
    if( (mcpChannels.at(0)==1 && mcpChannels.at(1)==0) || (mcpChannels.at(0)==0 && mcpChannels.at(1)==1))//reference is the MCP
      g_CTR_corr_vs_Vbias[label_th] -> SetPoint(g_CTR_corr_vs_Vbias[label_th]->GetN(),Vbias1,sqrt(effSigma*effSigma - MCPIntrinsic*MCPIntrinsic)*1000.);
    else
      g_CTR_corr_vs_Vbias[label_th] -> SetPoint(g_CTR_corr_vs_Vbias[label_th]->GetN(),Vbias1,effSigma/sqrt(2)*1000.);
    
    if( g_mean_corr_vs_th[label_Vbias] == NULL )
    {
      g_mean_corr_vs_th[label_Vbias] = new TGraph();
      g_CTR_corr_vs_th[label_Vbias] = new TGraph();
    }
    g_mean_corr_vs_th[label_Vbias] -> SetPoint(g_mean_corr_vs_th[label_Vbias]->GetN(),th,mean);
    if( (mcpChannels.at(0)==1 && mcpChannels.at(1)==0) || (mcpChannels.at(0)==0 && mcpChannels.at(1)==1))//reference is the MCP
      g_CTR_corr_vs_th[label_Vbias] -> SetPoint(g_CTR_corr_vs_th[label_Vbias]->GetN(),th,sqrt(effSigma*effSigma - MCPIntrinsic*MCPIntrinsic)*1000.);
    else
      g_CTR_corr_vs_th[label_Vbias] -> SetPoint(g_CTR_corr_vs_th[label_Vbias]->GetN(),th,effSigma/sqrt(2)*1000.);
  }
  
  for(unsigned int it = 0; it < vec_Vbias.size(); ++it)
  {
    float Vbias1 = vec_Vbias.at(it).first;
    std::string label_Vbias = std::string(Form("Vbias%.0fV",Vbias1));
    
    c1_scans[label_Vbias] -> cd(1);
    g_mean_corr_vs_th[label_Vbias] -> SetMarkerStyle(21);
    g_mean_corr_vs_th[label_Vbias] -> SetMarkerColor(kBlue);
    g_mean_corr_vs_th[label_Vbias] -> SetLineColor(kBlue);
    g_mean_corr_vs_th[label_Vbias] -> Draw("PL,same");
    c1_scans[label_Vbias] -> cd(2);
    g_CTR_corr_vs_th[label_Vbias] -> SetMarkerStyle(21);
    g_CTR_corr_vs_th[label_Vbias] -> SetMarkerColor(kBlue);
    g_CTR_corr_vs_th[label_Vbias] -> SetLineColor(kBlue);
    g_CTR_corr_vs_th[label_Vbias] -> Draw("PL,same");
    
    TLatex* latex3_cuts = new TLatex(0.55,0.89,Form("V_{bias} = %.0fV",Vbias1));
    latex3_cuts -> SetNDC();
    latex3_cuts -> SetTextFont(42);
    latex3_cuts -> SetTextSize(0.03);
    latex3_cuts -> SetTextColor(kBlack);
    latex3_cuts -> Draw("same");
    
    gPad -> Update();
    c1_scans[label_Vbias] -> Print(Form("%s/c__%s__CTR_vs_th__config%d__%s.png",plotDir.c_str(),label1.c_str(),configuration,label_Vbias.c_str()));
  }

  for(unsigned int it = 0; it < vec_th.size(); ++it)
  {
    float th = vec_th.at(it);
    std::string label_th = std::string(Form("th%.0fmV",th));
    
    c1_scans[label_th] -> cd(1);
    g_mean_corr_vs_Vbias[label_th] -> SetMarkerStyle(21);
    g_mean_corr_vs_Vbias[label_th] -> SetMarkerColor(kBlue);
    g_mean_corr_vs_Vbias[label_th] -> SetLineColor(kBlue);
    g_mean_corr_vs_Vbias[label_th] -> Draw("PL,same");
    c1_scans[label_th] -> cd(2);
    g_CTR_corr_vs_Vbias[label_th] -> SetMarkerStyle(21);
    g_CTR_corr_vs_Vbias[label_th] -> SetMarkerColor(kBlue);
    g_CTR_corr_vs_Vbias[label_th] -> SetLineColor(kBlue);
    g_CTR_corr_vs_Vbias[label_th] -> Draw("PL,same");
    
    TLatex* latex4_cuts = new TLatex(0.55,0.89,Form("NINO th. = %.0fmV",th));
    latex4_cuts -> SetNDC();
    latex4_cuts -> SetTextFont(42);
    latex4_cuts -> SetTextSize(0.03);
    latex4_cuts -> SetTextColor(kBlack);
    latex4_cuts -> Draw("same");
    
    gPad -> Update();
    c1_scans[label_th] -> Print(Form("%s/c__%s__CTR_vs_Vbias__config%d__%s.png",plotDir.c_str(),label1.c_str(),configuration,label_th.c_str()));
  }
  

  
  //------------------------
  // fourth loop over events
  if( maxStep < 3) { if( popupPlots ) theApp -> Run(); return 0; }
  
  for(int entry = 0; entry < nEntries; ++entry)
  {
    if( entry%1000 == 0 ) std::cout << ">>> loop 4/4: reading entry " << entry << " / " << nEntries << "\r" << std::flush;
    chain1 -> GetEntry(entry);
    
    if( !AcceptEvent(treeVars,cut_beamCutType,cut_beamXMin,cut_beamXMax,cut_beamYMin,cut_beamYMax,cut_angle) ) continue;
    
    float Vbias1 = treeVars.t_Vbias[VbiasIndex1];
    float Vbias2 = treeVars.t_Vbias[VbiasIndex2];
    float th = treeVars.t_NINOthr;
    std::string label_Vbias = std::string(Form("Vbias%.0fV",Vbias1));
    std::string label_th    = std::string(Form("th%.0fmV",th));
    std::string label_Vbias_th = label_Vbias + "_" + label_th;
    
    float amp1 = treeVars.t_amp[(*treeVars.t_channelId)[energyChannels.at(0)]] * 0.25;
    float amp2 = treeVars.t_amp[(*treeVars.t_channelId)[energyChannels.at(1)]] * 0.25;
    int extraIt1 = 0;
    int extraIt2 = 0;
    if( mcpChannels.at(0) == 1 ) extraIt1 = 14;
    if( mcpChannels.at(1) == 1 ) extraIt2 = 14;
    float time1 = treeVars.t_time[(*treeVars.t_channelId)[timeChannels.at(0)]+extraIt1];
    float time2 = treeVars.t_time[(*treeVars.t_channelId)[timeChannels.at(1)]+extraIt2];
    float CTR = (time2 - time1) - map_mean_vs_Vbias_th[label_Vbias_th] + map_mean_vs_Vbias_th[label_Vbias_th_max];
    float CTR_corr = CTR;
    if( mcpChannels.at(0)==1 && mcpChannels.at(1)==0)//Amp Walk correction on the second channel
      // CTR_corr = CTR -
      //            map_fitFunc_corrAmp2_vs_Vbias_th[label_Vbias_th]->Eval(amp2) +
      //            map_fitFunc_corrAmp2_vs_Vbias_th[label_Vbias_th]->Eval(h_amp2_cut->GetMean());
      CTR_corr = CTR -
                 map_CTR_vs_amp2_vs_Vbias_th[label_Vbias_th]->GetBinContent(map_CTR_vs_amp2_vs_Vbias_th[label_Vbias_th]->FindBin(amp2)) +
                 map_CTR_vs_amp2_vs_Vbias_th[label_Vbias_th]->GetBinContent(map_CTR_vs_amp2_vs_Vbias_th[label_Vbias_th]->FindBin(h_amp2_cut->GetMean())) -
                 map_CTR_vs_amp1_vs_Vbias_th[label_Vbias_th]->GetBinContent(map_CTR_vs_amp1_vs_Vbias_th[label_Vbias_th]->FindBin(amp1)) +
                 map_CTR_vs_amp1_vs_Vbias_th[label_Vbias_th]->GetBinContent(map_CTR_vs_amp1_vs_Vbias_th[label_Vbias_th]->FindBin(h_amp1_cut->GetMean()));
    else if( mcpChannels.at(0)==0 && mcpChannels.at(1)==1)//Amp Walk correction on the first channel
      // CTR_corr = CTR -
      //            map_fitFunc_corrAmp1_vs_Vbias_th[label_Vbias_th]->Eval(amp1) +
      //            map_fitFunc_corrAmp1_vs_Vbias_th[label_Vbias_th]->Eval(h_amp1_cut->GetMean());
      CTR_corr = CTR -
                 map_CTR_vs_amp1_vs_Vbias_th[label_Vbias_th]->GetBinContent(map_CTR_vs_amp1_vs_Vbias_th[label_Vbias_th]->FindBin(amp1)) +
                 map_CTR_vs_amp1_vs_Vbias_th[label_Vbias_th]->GetBinContent(map_CTR_vs_amp1_vs_Vbias_th[label_Vbias_th]->FindBin(h_amp1_cut->GetMean())) -
                 map_CTR_vs_amp2_vs_Vbias_th[label_Vbias_th]->GetBinContent(map_CTR_vs_amp2_vs_Vbias_th[label_Vbias_th]->FindBin(amp2)) +
                 map_CTR_vs_amp2_vs_Vbias_th[label_Vbias_th]->GetBinContent(map_CTR_vs_amp2_vs_Vbias_th[label_Vbias_th]->FindBin(h_amp2_cut->GetMean()));
    else//Or there are 2 MCP either NO MCP
      if( map_fitFunc_corrRatio_vs_Vbias_th[label_Vbias_th] != NULL )
        CTR_corr = CTR -
                   map_fitFunc_corrRatio_vs_Vbias_th[label_Vbias_th]->Eval(amp2/amp1) +
                   map_fitFunc_corrRatio_vs_Vbias_th[label_Vbias_th]->Eval(h_ampRatio_cut->GetMean());
      // CTR_corr = CTR -
      //            map_CTR_vs_ampRatio_vs_Vbias_th[label_Vbias_th]->GetBinContent(map_CTR_vs_ampRatio_vs_Vbias_th[label_Vbias_th]->FindBin(amp2/amp1)) + 
      //            map_CTR_vs_ampRatio_vs_Vbias_th[label_Vbias_th]->GetBinContent(map_CTR_vs_ampRatio_vs_Vbias_th[label_Vbias_th]->FindBin(h_ampRatio_cut->GetMean()));
       // CTR_corr = CTR -
       //            map_CTR_vs_amp1_vs_Vbias_th[label_Vbias_th]->GetBinContent(map_CTR_vs_amp1_vs_Vbias_th[label_Vbias_th]->FindBin(amp1)) +
       //            map_CTR_vs_amp1_vs_Vbias_th[label_Vbias_th]->GetBinContent(map_CTR_vs_amp1_vs_Vbias_th[label_Vbias_th]->FindBin(h_amp1_cut->GetMean())) -
       //            map_CTR_vs_amp2_vs_Vbias_th[label_Vbias_th]->GetBinContent(map_CTR_vs_amp2_vs_Vbias_th[label_Vbias_th]->FindBin(amp2)) +
       //            map_CTR_vs_amp2_vs_Vbias_th[label_Vbias_th]->GetBinContent(map_CTR_vs_amp2_vs_Vbias_th[label_Vbias_th]->FindBin(h_amp2_cut->GetMean()));
       
    if( !AcceptEventAmp(treeVars,cut_ampMin1[Vbias1],cut_ampMax1[Vbias1],cut_ampMin2[Vbias2],cut_ampMax2[Vbias2]) ) continue;
    if( !AcceptEventDur(treeVars,cut_durMin1[Vbias1],cut_durMax1[Vbias1],cut_durMin2[Vbias2],cut_durMax2[Vbias2]) ) continue;
    if( !AcceptEventVbias(treeVars,cut_VbiasMin,cut_VbiasMax) ) continue;
    if( !AcceptEventTh(treeVars,cut_NINOthrMin,cut_NINOthrMax) ) continue;
    
    h_CTR_corr -> Fill( CTR_corr );
    
    int bin1   = h_nEvents_vs_amp1   -> Fill(amp1);
    int bin2   = h_nEvents_vs_amp2   -> Fill(amp2);
    int binAvg = h_nEvents_vs_ampAvg -> Fill(0.5*(amp1+amp2));
    
    if( map_CTR_corr_vs_amp1[bin1] == NULL )
      map_CTR_corr_vs_amp1[bin1] = new TH1F(Form("h_CTR_corr_vsAmp1_%d",bin1),"",20000,-10.,10.);
    map_CTR_corr_vs_amp1[bin1] -> Fill( CTR_corr );
    
    if( map_CTR_corr_vs_amp2[bin2] == NULL )
      map_CTR_corr_vs_amp2[bin2] = new TH1F(Form("h_CTR_corr_vs_Amp2_%d",bin2),"",20000,-10.,10.);
    map_CTR_corr_vs_amp2[bin2] -> Fill( CTR_corr );
    
    if( map_CTR_corr_vs_ampAvg[binAvg] == NULL )
      map_CTR_corr_vs_ampAvg[binAvg] = new TH1F(Form("h_CTR_corr_vs_AmpAvg_%d",binAvg),"",20000,-10.,10.);
    map_CTR_corr_vs_ampAvg[binAvg] -> Fill( CTR_corr );
  }
  std::cout << std::endl;

  
  c1_CTR -> cd();
  
  FindSmallestInterval(vals,h_CTR_corr,0.68,true); 
  h_CTR_corr -> Rebin(rebin);
  h_CTR_corr -> SetMarkerStyle(21);
  h_CTR_corr -> SetMarkerColor(kBlue);
  h_CTR_corr -> SetLineColor(kBlue);
  h_CTR -> SetMaximum(1.5*h_CTR_corr->GetMaximum());
  h_CTR_corr -> Draw("PE,same");
  
  gPad -> Update();  
  
  
  norm = h_CTR_corr -> GetMaximum();  
  mean = vals[0];
  min = vals[2];
  max = vals[3];
  delta = max-min;
  sigma = 0.5*delta;
  effSigma = sigma;
  min = min - 2.*delta;
  max = max + 2.*delta;
  
  h_CTR_corr -> GetXaxis() -> SetRangeUser(min,max);
  
  // gaus fit
  gausName = Form("fitFunc_gaus_corr");
  TF1* fitFunc_gaus_corr = new TF1(gausName.c_str(),"[0]*exp(-1.*(x-[1])*(x-[1])/(2.*[2]*[2]))",mean-sigma,mean+sigma);
  fitFunc_gaus_corr -> SetNpx(10000);
  fitFunc_gaus_corr -> SetParameters(norm,mean,sigma);
  fitFunc_gaus_corr -> SetLineColor(kBlue);
  h_CTR_corr -> Fit(gausName.c_str(),"QNRSL");
  norm = fitFunc_gaus_corr -> GetParameter(0);
  mean = fitFunc_gaus_corr -> GetParameter(1);
  sigma = fitFunc_gaus_corr -> GetParameter(2);
  sigmaErr = fitFunc_gaus_corr -> GetParError(2);
 
  TLatex* latex22_corr;
  if( (mcpChannels.at(0)==1 && mcpChannels.at(1)==0) || (mcpChannels.at(0)==0 && mcpChannels.at(1)==1))//reference is the MCP
    latex22_corr = new TLatex(0.16,0.50,Form("#sigma_{single}^{gaus} = (%.1f #pm %.1f) ps",
    sqrt(sigma*sigma - MCPIntrinsic*MCPIntrinsic)*1000.,
    fabs(sigmaErr*1000)));
  else
    latex22_corr = new TLatex(0.16,0.50,Form("#sigma_{single}^{gaus} = (%.1f #pm %.1f) ps",fabs(sigma*1000)/sqrt(2),fabs(sigmaErr*1000)/sqrt(2)));
  latex22_corr -> SetNDC();
  latex22_corr -> SetTextFont(42);
  latex22_corr -> SetTextSize(0.03);
  latex22_corr -> SetTextColor(kBlue);
  latex22_corr -> Draw("same");
  
  // crystal ball fit
  cbName = Form("fitFunc_cb_corr");
  TF1* fitFunc_cb_corr = new TF1(cbName.c_str(),crystalBallLowHigh,mean-4.*sigma,mean+4.*sigma,8);
  fitFunc_cb_corr -> SetNpx(10000);
  fitFunc_cb_corr -> SetParameters(norm,mean,sigma,1.5,10.,1.5,10.);
  fitFunc_cb_corr -> SetLineColor(kBlue);
  h_CTR_corr -> Fit(cbName.c_str(),"QNRSL");
  norm = fitFunc_cb_corr -> GetParameter(0);
  mean = fitFunc_cb_corr -> GetParameter(1);
  sigma = fitFunc_cb_corr -> GetParameter(2);
  sigmaErr = fitFunc_cb_corr -> GetParError(2);
  
  fitFunc_cb_corr -> Draw("same");
  
  TLatex* latex2_corr;
  if( (mcpChannels.at(0)==1 && mcpChannels.at(1)==0) || (mcpChannels.at(0)==0 && mcpChannels.at(1)==1))//reference is the MCP
    latex2_corr = new TLatex(0.16,0.80,Form("eff. #sigma_{single} = %.1f ps",
    sqrt(effSigma*effSigma - MCPIntrinsic*MCPIntrinsic)*1000.));
  else
    latex2_corr = new TLatex(0.16,0.80,Form("eff. #sigma_{single} = %.1f ps",fabs(effSigma*1000)/sqrt(2)));
  latex2_corr -> SetNDC();
  latex2_corr -> SetTextFont(42);
  latex2_corr -> SetTextSize(0.05);
  latex2_corr -> SetTextColor(kBlue);
  latex2_corr -> Draw("same");

  TLatex* latex21_corr;
  if( (mcpChannels.at(0)==1 && mcpChannels.at(1)==0) || (mcpChannels.at(0)==0 && mcpChannels.at(1)==1))//reference is the MCP
    latex21_corr = new TLatex(0.16,0.35,Form("#sigma_{single}^{c.b.} = (%.1f #pm %.1f) ps",
    sqrt(sigma*sigma - MCPIntrinsic*MCPIntrinsic)*1000.,
    fabs(sigmaErr*1000)));
  else
    latex21_corr = new TLatex(0.16,0.35,Form("#sigma_{single}^{c.b.} = (%.1f #pm %.1f) ps",fabs(sigma*1000)/sqrt(2),fabs(sigmaErr*1000)/sqrt(2)));
  latex21_corr -> SetNDC();
  latex21_corr -> SetTextFont(42);
  latex21_corr -> SetTextSize(0.03);
  latex21_corr -> SetTextColor(kBlue);
  latex21_corr -> Draw("same");
  
  TLatex* latex3_cuts = new TLatex(0.55,0.89,Form("V_{bias} #in [%.0f,%.0f]",cut_VbiasMin,cut_VbiasMax));
  latex3_cuts -> SetNDC();
  latex3_cuts -> SetTextFont(42);
  latex3_cuts -> SetTextSize(0.03);
  latex3_cuts -> SetTextColor(kBlack);
  latex3_cuts -> Draw("same");
  TLatex* latex4_cuts = new TLatex(0.55,0.84,Form("NINO th. #in [%.0f,%.0f]",cut_NINOthrMin,cut_NINOthrMax));
  latex4_cuts -> SetNDC();
  latex4_cuts -> SetTextFont(42);
  latex4_cuts -> SetTextSize(0.03);
  latex4_cuts -> SetTextColor(kBlack);
  latex4_cuts -> Draw("same");
  
  gPad -> Update();
  c1_CTR -> Print(Form("%s/c__%s__CTR__config%d__Vbias%.0f-%.0f_th%.0f-%.0f.png",plotDir.c_str(),label1.c_str(),configuration,cut_VbiasMin,cut_VbiasMax,cut_NINOthrMin,cut_NINOthrMax));
  
  
  
  // CTR vs amp plots
  
  TGraphErrors* g_CTR_vs_amp1 = new TGraphErrors();
  for(int bin = 1; bin <= h_nEvents_vs_amp1->GetNbinsX(); ++bin)
  {
    float binCenter = h_nEvents_vs_amp1->GetBinCenter(bin);
    float binError = binCenter - h_nEvents_vs_amp1->GetBinLowEdge(bin);
    
    TH1F* histo = map_CTR_corr_vs_amp1[bin];
    float* vals = new float[4];
    FindSmallestInterval(vals,histo,0.68,true); 
    
    float min = vals[2];
    float max = vals[3];
    float delta = max-min;
    float sigma = 0.5*delta;
    float effSigma = sigma;

    if( (mcpChannels.at(0)==1 && mcpChannels.at(1)==0) || (mcpChannels.at(0)==0 && mcpChannels.at(1)==1))//reference is the MCP
    {
      g_CTR_vs_amp1 -> SetPoint(g_CTR_vs_amp1->GetN(),binCenter,sqrt(effSigma*effSigma - MCPIntrinsic*MCPIntrinsic)*1000.);
      g_CTR_vs_amp1 -> SetPointError(g_CTR_vs_amp1->GetN()-1,binError,effSigma*1000./sqrt(nEventsPerEnergyBin));
    }
    else
    {
      g_CTR_vs_amp1 -> SetPoint(g_CTR_vs_amp1->GetN(),binCenter,effSigma/sqrt(2)*1000.);
      g_CTR_vs_amp1 -> SetPointError(g_CTR_vs_amp1->GetN()-1,binError,effSigma/sqrt(2)*1000./sqrt(nEventsPerEnergyBin));
    }
  }
  
  TGraphErrors* g_CTR_vs_amp2 = new TGraphErrors();
  for(int bin = 1; bin <= h_nEvents_vs_amp2->GetNbinsX(); ++bin)
  {
    float binCenter = h_nEvents_vs_amp2->GetBinCenter(bin);
    float binError = binCenter - h_nEvents_vs_amp2->GetBinLowEdge(bin);
    
    TH1F* histo = map_CTR_corr_vs_amp2[bin];
    float* vals = new float[4];
    FindSmallestInterval(vals,histo,0.68,true); 
    
    float min = vals[2];
    float max = vals[3];
    float delta = max-min;
    float sigma = 0.5*delta;
    float effSigma = sigma;

    if( (mcpChannels.at(0)==1 && mcpChannels.at(1)==0) || (mcpChannels.at(0)==0 && mcpChannels.at(1)==1))//reference is the MCP
    {
      g_CTR_vs_amp2 -> SetPoint(g_CTR_vs_amp2->GetN(),binCenter,sqrt(effSigma*effSigma - MCPIntrinsic*MCPIntrinsic)*1000.);
      g_CTR_vs_amp2 -> SetPointError(g_CTR_vs_amp2->GetN()-1,binError,effSigma*1000./sqrt(nEventsPerEnergyBin));
    }
    else
    {    
      g_CTR_vs_amp2 -> SetPoint(g_CTR_vs_amp2->GetN(),binCenter,effSigma/sqrt(2)*1000.);
      g_CTR_vs_amp2 -> SetPointError(g_CTR_vs_amp2->GetN()-1,binError,effSigma/sqrt(2)*1000./sqrt(nEventsPerEnergyBin));
    }
  }
  
  TGraphErrors* g_CTR_vs_ampAvg = new TGraphErrors();
  for(int bin = 1; bin <= h_nEvents_vs_ampAvg->GetNbinsX(); ++bin)
  {
    float binCenter = h_nEvents_vs_ampAvg->GetBinCenter(bin);
    float binError = binCenter - h_nEvents_vs_ampAvg->GetBinLowEdge(bin);
    
    TH1F* histo = map_CTR_corr_vs_ampAvg[bin];
    float* vals = new float[4];
    FindSmallestInterval(vals,histo,0.68,true); 
    
    float min = vals[2];
    float max = vals[3];
    float delta = max-min;
    float sigma = 0.5*delta;
    float effSigma = sigma;

    if( (mcpChannels.at(0)==1 && mcpChannels.at(1)==0) || (mcpChannels.at(0)==0 && mcpChannels.at(1)==1))//reference is the MCP
    {
      g_CTR_vs_ampAvg -> SetPoint(g_CTR_vs_ampAvg->GetN(),binCenter,sqrt(effSigma*effSigma - MCPIntrinsic*MCPIntrinsic)*1000.);
      g_CTR_vs_ampAvg -> SetPointError(g_CTR_vs_ampAvg->GetN()-1,binError,effSigma*1000./sqrt(nEventsPerEnergyBin));
    }
    else
    {
      g_CTR_vs_ampAvg -> SetPoint(g_CTR_vs_ampAvg->GetN(),binCenter,effSigma/sqrt(2)*1000.);
      g_CTR_vs_ampAvg -> SetPointError(g_CTR_vs_ampAvg->GetN()-1,binError,effSigma/sqrt(2)*1000./sqrt(nEventsPerEnergyBin));
    }
  }
  
  c1 = new TCanvas("c1_CTR_vs_amp","CTR vs amplitude",2100,900);
  c1 -> Divide(2,1);
  c1 -> cd(1);
  hPad = (TH1F*)( gPad->DrawFrame(0.,20.,1000.,80.) );
  hPad -> SetTitle(";amp_{1} or amp_{2} (mV); eff. #sigma_{single} (ps)");
  gPad -> SetGridy();
  hPad -> GetXaxis() -> SetRangeUser(std::min(bins_vs_amp1[0],bins_vs_amp2[0])-50.,std::max(bins_vs_amp1[nBins_vs_amp],bins_vs_amp2[nBins_vs_amp])+50.);
  hPad -> Draw();
  g_CTR_vs_amp1 -> SetMarkerStyle(24);
  g_CTR_vs_amp1 -> SetMarkerColor(kRed);
  g_CTR_vs_amp1 -> SetLineColor(kRed);
  g_CTR_vs_amp1 -> Draw("P,same");
  g_CTR_vs_amp2 -> SetMarkerStyle(20);
  g_CTR_vs_amp2 -> SetMarkerColor(kBlue);
  g_CTR_vs_amp2 -> SetLineColor(kBlue);
  g_CTR_vs_amp2 -> Draw("P,same");
  latexLabel12 -> Draw("same");
  c1 -> cd(2);
  hPad = (TH1F*)( gPad->DrawFrame(0.,20.,1000.,80.) );
  hPad -> SetTitle(";0.5 * ( amp_{1} + amp_{2} ) (mV); eff. #sigma_{single} (ps)");
  gPad -> SetGridy();
  hPad -> GetXaxis() -> SetRangeUser(bins_vs_ampAvg[0]-50.,bins_vs_ampAvg[nBins_vs_amp]+50.);
  hPad -> Draw();
  g_CTR_vs_ampAvg -> SetMarkerStyle(20);
  g_CTR_vs_ampAvg -> SetMarkerColor(kBlack);
  g_CTR_vs_ampAvg -> SetLineColor(kBlack);
  g_CTR_vs_ampAvg -> Draw("P,same");
  latexLabel12 -> Draw("same");
    
  gPad -> Update();
  c1 -> Print(Form("%s/c__%s__CTR_vs_amp__config%d__Vbias%.0f-%.0f_th%.0f-%.0f.png",plotDir.c_str(),label1.c_str(),configuration,cut_VbiasMin,cut_VbiasMax,cut_NINOthrMin,cut_NINOthrMax));  
  
  
  
  if( popupPlots ) theApp -> Run();
  return 0;
}



bool AcceptEvent(TreeVars treeVars, const int& beamCutType, const float& beamXMin, const float& beamXMax, const float& beamYMin, const float& beamYMax, const float& angle)
{
  if( treeVars.t_beamX[1] >= -15.0 && treeVars.t_beamX[1] < -14.5 ) return false;
  if( treeVars.t_beamX[1] >=   8.5 && treeVars.t_beamX[1] <   9.5 ) return false;
  if( treeVars.t_beamY[0] >= -16.0 && treeVars.t_beamY[0] < -15.5 ) return false;
  if( treeVars.t_beamY[1] >= -16.0 && treeVars.t_beamY[1] < -15.5 ) return false;
  
  if( fabs(treeVars.t_beamX[1]-treeVars.t_beamX[0]) > 1. ) return false;
  if( fabs(treeVars.t_beamY[1]-treeVars.t_beamY[0]) > 1. ) return false;
  
  if( treeVars.t_amp[(*treeVars.t_channelId)[energyChannels.at(0)]]*0.25 < 0 ) return false;
  if( treeVars.t_amp[(*treeVars.t_channelId)[energyChannels.at(1)]]*0.25 < 0 ) return false;
  
  if( angle > -1 )
  {
    if( treeVars.t_angle != angle ) return false;
  }
  
  if( beamCutType <= 0 ) return true;
  
  bool inside = false;
  if( treeVars.t_beamX[0] >= beamXMin && treeVars.t_beamX[0] <= beamXMax &&
      treeVars.t_beamY[0] >= beamYMin && treeVars.t_beamY[0] <= beamYMax )
    inside = true;
  
  if     ( beamCutType == 1 && inside == true ) return true;
  else if( beamCutType == 1 && inside == false ) return false;
  else if( beamCutType == 2 && inside == true ) return false;
  else if( beamCutType == 2 && inside == false ) return true;
  
  return true;
}

bool AcceptEventAmp(TreeVars treeVars, const float& ampMin1, const float& ampMax1, const float& ampMin2, const float& ampMax2)
{
  if( (treeVars.t_amp[(*treeVars.t_channelId)[energyChannels.at(0)]]*0.25 < ampMin1) || (treeVars.t_amp[(*treeVars.t_channelId)[energyChannels.at(0)]]*0.25 > ampMax1) ) return false;
  if( (treeVars.t_amp[(*treeVars.t_channelId)[energyChannels.at(1)]]*0.25 < ampMin2) || (treeVars.t_amp[(*treeVars.t_channelId)[energyChannels.at(1)]]*0.25 > ampMax2) ) return false;
  
  return true;
}

bool AcceptEventDur(TreeVars treeVars, const float& durMin1, const float& durMax1, const float& durMin2, const float& durMax2)
{
  if( (treeVars.t_dur[(*treeVars.t_channelId)[timeChannels.at(0)]]*0.2 < durMin1) || (treeVars.t_dur[(*treeVars.t_channelId)[timeChannels.at(0)]]*0.2 > durMax1) ) return false;
  if( (treeVars.t_dur[(*treeVars.t_channelId)[timeChannels.at(1)]]*0.2 < durMin2) || (treeVars.t_dur[(*treeVars.t_channelId)[timeChannels.at(1)]]*0.2 > durMax2) ) return false;
  
  return true;
}

bool AcceptEventTh(TreeVars treeVars, const float& thMin, const float& thMax)
{
  if( (treeVars.t_NINOthr < thMin) || (treeVars.t_NINOthr > thMax) ) return false;
  
  return true;
}

bool AcceptEventVbias(TreeVars treeVars, const float& VbiasMin, const float& VbiasMax)
{
  float Vbias1 = treeVars.t_Vbias[VbiasIndex1];
  float Vbias2 = treeVars.t_Vbias[VbiasIndex2];
  if( (Vbias1 < VbiasMin) || (Vbias1 > VbiasMax) ) return false;
  if( (Vbias2 < VbiasMin) || (Vbias2 > VbiasMax) ) return false;
  
  return true;
}



void InitTreeVars(TTree* chain1, TTree* chain2, TTree* chain3,
                  TreeVars& treeVars)
{
  treeVars.t_Vbias = new float[3];
  treeVars.t_ped = new float[28];
  treeVars.t_amp = new float[28];
  treeVars.t_dur = new float[28];
  treeVars.t_time = new float[28];
  treeVars.t_beamX = new float[2];
  treeVars.t_beamY = new float[2];
  treeVars.t_channelId = new std::map<std::string,int>;
  
  //chain1 -> SetBranchStatus("*",0);
  chain1 -> SetBranchStatus("NINOthr",1); chain1 -> SetBranchAddress("NINOthr",&treeVars.t_NINOthr);
  chain1 -> SetBranchStatus("Vbias1" ,1); chain1 -> SetBranchAddress("Vbias1", &treeVars.t_Vbias[0]);
  chain1 -> SetBranchStatus("Vbias2" ,1); chain1 -> SetBranchAddress("Vbias2", &treeVars.t_Vbias[1]);
  chain1 -> SetBranchStatus("Vbias3" ,1); chain1 -> SetBranchAddress("Vbias3", &treeVars.t_Vbias[2]);
  chain1 -> SetBranchStatus("angle"  ,1); chain1 -> SetBranchAddress("angle",  &treeVars.t_angle);
  
  //chain2 -> SetBranchStatus("*",0);
  chain2 -> SetBranchStatus("X",1); chain2 -> SetBranchAddress("X",treeVars.t_beamX);
  chain2 -> SetBranchStatus("Y",1); chain2 -> SetBranchAddress("Y",treeVars.t_beamY);
  
  //chain3 -> SetBranchStatus("*",0);
  chain3 -> SetBranchStatus("amp_max",   1); chain3 -> SetBranchAddress("amp_max",   treeVars.t_amp);
  chain3 -> SetBranchStatus("charge_sig",1); chain3 -> SetBranchAddress("charge_sig",treeVars.t_dur);
  chain3 -> SetBranchStatus("time",      1); chain3 -> SetBranchAddress("time",      treeVars.t_time);
  for(unsigned int it = 0; it < timeChannels.size(); ++it)
  {
    std::string channelName = timeChannels.at(it);
    chain3 -> SetBranchStatus(channelName.c_str(),1); chain3 -> SetBranchAddress(channelName.c_str(),&((*treeVars.t_channelId)[channelName]));
  }
  for(unsigned int it = 0; it < energyChannels.size(); ++it)
  {
    std::string channelName = energyChannels.at(it);
    chain3 -> SetBranchStatus(channelName.c_str(),1); chain3 -> SetBranchAddress(channelName.c_str(),&((*treeVars.t_channelId)[channelName]));
  }
}
