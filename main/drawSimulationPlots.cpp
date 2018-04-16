#include "interface/FitUtils.h"
#include "interface/SetTDRStyle.h"
#include "interface/SiPMFunctions.h"

#include "CfgManager/interface/CfgManager.h"
#include "CfgManager/interface/CfgManagerT.h"

#include <iostream>
#include <vector>
#include <map>
#include <algorithm>
#include <cstdlib>
#include <numeric>

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
#include "TRandom3.h"



/*** tree variables ***/
struct TreeVars
{
  std::vector<float>* inputInitialPosition;
  float depositedEnergyCrystal;
  int tot_phot_sci;
  int tot_phot_cer;
  int tot_det_phot_sci;
  int tot_det_phot_cer;
  
  std::vector<float>* time_phot_sci;
  std::vector<float>* time_phot_cer;
  std::vector<float>* lambda_phot_sci;  
  std::vector<float>* lambda_phot_cer;
  std::vector<float>* time_det_phot_sci;
  std::vector<float>* time_det_phot_cer;
  std::vector<float>* lambda_det_phot_sci;  
  std::vector<float>* lambda_det_phot_cer;
};

void InitTreeVars(TTree* chain, TreeVars& treeVars);



int main(int argc, char** argv)
{
  if( argc < 2 )
  {
    std::cerr << ">>>>> drawSimulationPlots.cpp::usage:   " << argv[0] << " configFileName" << std::endl;
    return 1;
  }
  
  
  
  //----------------------
  // parse the config file
  CfgManager opts;
  opts.ParseConfigFile(argv[1]);
  
  std::string inputFolder = opts.GetOpt<std::string>("Input.inputFolder");
  std::vector<std::string> labels = opts.GetOpt<std::vector<std::string> >("Input.labels");
  int maxEntries = opts.GetOpt<int>("Input.maxEntries");
  
  std::string SiPM = opts.GetOpt<std::string>("SiPM.model");
  float singleCellAmplitude = opts.GetOpt<float>("SiPM.singleCellAmplitude");
  float riseTime = opts.GetOpt<float>("SiPM.riseTime");
  float decayTime = opts.GetOpt<float>("SiPM.decayTime");
  float SPTR = opts.GetOpt<float>("SiPM.SPTR");
  int doResponseFunction = opts.GetOpt<int>("SiPM.doResponseFunction");
  
  
  
  //------------------------
  // labels and canvas style
  setTDRStyle();
  
  // TLatex* latexLabel1 = new TLatex(0.13,0.97,Form("%s",label1.c_str()));
  // TLatex* latexLabel2 = new TLatex(0.13,0.97,Form("%s",label2.c_str()));
  // TLatex* latexLabel12 = new TLatex(0.13,0.97,Form("%s -- %s",label1.c_str(),label2.c_str()));
  // latexLabel1 -> SetNDC();
  // latexLabel1 -> SetTextFont(42);
  // latexLabel1 -> SetTextSize(0.03);
  // latexLabel2 -> SetNDC();
  // latexLabel2 -> SetTextFont(42);
  // latexLabel2 -> SetTextSize(0.03);
  // latexLabel12 -> SetNDC();
  // latexLabel12 -> SetTextFont(42);
  // latexLabel12 -> SetTextSize(0.03);
  
  // std::string baseDir(Form("/afs/cern.ch/user/a/abenagli/www/TIMING/Simulation/%s/",label.c_str()));
  // system(Form("mkdir -p %s",baseDir.c_str()));
  // system(Form("cp /afs/cern.ch/user/a/abenagli/public/index.php %s",baseDir.c_str()));
  
  
  
  for(unsigned int labelIt = 0; labelIt < labels.size(); ++labelIt)
  {
    std::string label = labels.at(labelIt);
    
    
    //---------------------------
    // open input files and trees
    TChain* chain = new TChain("tree","tree");
    
    std::string ls_command = std::string("eos ls "+inputFolder+"/"+label+" | grep 'root' > tmp.list");
    system(ls_command.c_str());
    std::ifstream waveList("tmp.list", std::ios::in);
    std::string file;
    int nFiles = 0;
    while( waveList >> file )
    {
      std::cout << "+++ Adding file " << "root://eoscms.cern.ch//eos/cms/" << (inputFolder+"/"+label+"/"+file).c_str() << std::endl;
      chain -> AddFile(std::string("root://eoscms.cern.ch//eos/cms/"+(inputFolder+"/"+label+"/"+file)).c_str());
      ++nFiles;
    }
    std::cout << "+++ Added " << nFiles << " files with " << chain->GetEntries() << " events" << std::endl;
    
    
    // set branches
    TreeVars treeVars;
    InitTreeVars(chain,treeVars);
    
    
    
    //-----------------
    // open output file
    
    TFile* outFile = TFile::Open(Form("%s__SPTR%.0fps_plots.root",label.c_str(),SPTR),"RECREATE");
    
    
    
    //------------------
    // Define histograms
    outFile -> cd();
    
    TH1F* h1_depositedEnergyCrystal = new TH1F("depositedEnergyCrystal","",1000,0.,0.05);
    
    TH1F* h1_lightColl_sci = new TH1F("h1_lightColl_sci","",50000,0.,50000);
    TH1F* h1_lightColl_cer = new TH1F("h1_lightColl_cer","",50000,0.,50000);
    TH1F* h1_lightColl_all = new TH1F("h1_lightColl_all","",50000,0.,50000);
    TH1F* h1_lightCollEff_sci = new TH1F("h1_lightCollEff_sci","",50000,0.,1.);
    TH1F* h1_lightCollEff_cer = new TH1F("h1_lightCollEff_cer","",50000,0.,1.);
    TH1F* h1_lightCollEff_all = new TH1F("h1_lightCollEff_all","",50000,0.,1.);
    TProfile2D* p2_lightColl_sci_vs_xy = new TProfile2D("p2_lightColl_sci_vs_xy","",35,-7.,7.,35,-7.,7.);
    TProfile2D* p2_lightColl_cer_vs_xy = new TProfile2D("p2_lightColl_cer_vs_xy","",35,-7.,7.,35,-7.,7.);
    TProfile2D* p2_lightColl_all_vs_xy = new TProfile2D("p2_lightColl_all_vs_xy","",35,-7.,7.,35,-7.,7.);
    TProfile2D* p2_lightCollEff_sci_vs_xy = new TProfile2D("p2_lightCollEff_sci_vs_xy","",35,-7.,7.,35,-7.,7.);
    TProfile2D* p2_lightCollEff_cer_vs_xy = new TProfile2D("p2_lightCollEff_cer_vs_xy","",35,-7.,7.,35,-7.,7.);
    TProfile2D* p2_lightCollEff_all_vs_xy = new TProfile2D("p2_lightCollEff_all_vs_xy","",35,-7.,7.,35,-7.,7.);
    TProfile* p1_lightColl_sci_vs_x_ally = new TProfile("p1_lightColl_sci_vs_x_ally","",70,-7.,7.);
    TProfile* p1_lightColl_sci_vs_x_cuty = new TProfile("p1_lightColl_sci_vs_x_cuty","",70,-7.,7.);
    TProfile* p1_lightColl_cer_vs_x_ally = new TProfile("p1_lightColl_cer_vs_x_ally","",70,-7.,7.);
    TProfile* p1_lightColl_cer_vs_x_cuty = new TProfile("p1_lightColl_cer_vs_x_cuty","",70,-7.,7.);
    TProfile* p1_lightColl_all_vs_x_ally = new TProfile("p1_lightColl_all_vs_x_ally","",70,-7.,7.);
    TProfile* p1_lightColl_all_vs_x_cuty = new TProfile("p1_lightColl_all_vs_x_cuty","",70,-7.,7.);
    TProfile* p1_lightCollEff_sci_vs_x_ally = new TProfile("p1_lightCollEff_sci_vs_x_ally","",70,-7.,7.);
    TProfile* p1_lightCollEff_sci_vs_x_cuty = new TProfile("p1_lightCollEff_sci_vs_x_cuty","",70,-7.,7.);
    TProfile* p1_lightCollEff_cer_vs_x_ally = new TProfile("p1_lightCollEff_cer_vs_x_ally","",70,-7.,7.);
    TProfile* p1_lightCollEff_cer_vs_x_cuty = new TProfile("p1_lightCollEff_cer_vs_x_cuty","",70,-7.,7.);
    TProfile* p1_lightCollEff_all_vs_x_ally = new TProfile("p1_lightCollEff_all_vs_x_ally","",70,-7.,7.);
    TProfile* p1_lightCollEff_all_vs_x_cuty = new TProfile("p1_lightCollEff_all_vs_x_cuty","",70,-7.,7.);
    
    TH1F* h1_timeProfileColl_sci = new TH1F("h1_timeProfileColl_sci","",20000,-500.,19500.);
    TH1F* h1_timeProfileColl_cer = new TH1F("h1_timeProfileColl_cer","",20000,-500.,19500.);
    TH1F* h1_timeProfileColl_all = new TH1F("h1_timeProfileColl_all","",20000,-500.,19500.);
    
    TH1F* h1_lightDet_sci = new TH1F("h1_lightDet_sci","",50000,0.,50000);
    TH1F* h1_lightDet_cer = new TH1F("h1_lightDet_cer","",50000,0.,50000);
    TH1F* h1_lightDet_all = new TH1F("h1_lightDet_all","",50000,0.,50000);
    TH1F* h1_lightDetEff_sci = new TH1F("h1_lightDetEff_sci","",50000,0.,1.);
    TH1F* h1_lightDetEff_cer = new TH1F("h1_lightDetEff_cer","",50000,0.,1.);
    TH1F* h1_lightDetEff_all = new TH1F("h1_lightDetEff_all","",50000,0.,1.);
    TProfile2D* p2_lightDet_sci_vs_xy = new TProfile2D("p2_lightDet_sci_vs_xy","",35,-7.,7.,35,-7.,7.);
    TProfile2D* p2_lightDet_cer_vs_xy = new TProfile2D("p2_lightDet_cer_vs_xy","",35,-7.,7.,35,-7.,7.);
    TProfile2D* p2_lightDet_all_vs_xy = new TProfile2D("p2_lightDet_all_vs_xy","",35,-7.,7.,35,-7.,7.);
    TProfile2D* p2_lightDetEff_sci_vs_xy = new TProfile2D("p2_lightDetEff_sci_vs_xy","",35,-7.,7.,35,-7.,7.);
    TProfile2D* p2_lightDetEff_cer_vs_xy = new TProfile2D("p2_lightDetEff_cer_vs_xy","",35,-7.,7.,35,-7.,7.);
    TProfile2D* p2_lightDetEff_all_vs_xy = new TProfile2D("p2_lightDetEff_all_vs_xy","",35,-7.,7.,35,-7.,7.);
    TProfile* p1_lightDet_sci_vs_x_ally = new TProfile("p1_lightDet_sci_vs_x_ally","",70,-7.,7.);
    TProfile* p1_lightDet_sci_vs_x_cuty = new TProfile("p1_lightDet_sci_vs_x_cuty","",70,-7.,7.);
    TProfile* p1_lightDet_cer_vs_x_ally = new TProfile("p1_lightDet_cer_vs_x_ally","",70,-7.,7.);
    TProfile* p1_lightDet_cer_vs_x_cuty = new TProfile("p1_lightDet_cer_vs_x_cuty","",70,-7.,7.);
    TProfile* p1_lightDet_all_vs_x_ally = new TProfile("p1_lightDet_all_vs_x_ally","",70,-7.,7.);
    TProfile* p1_lightDet_all_vs_x_cuty = new TProfile("p1_lightDet_all_vs_x_cuty","",70,-7.,7.);
    TProfile* p1_lightDetEff_sci_vs_x_ally = new TProfile("p1_lightDetEff_sci_vs_x_ally","",70,-7.,7.);
    TProfile* p1_lightDetEff_sci_vs_x_cuty = new TProfile("p1_lightDetEff_sci_vs_x_cuty","",70,-7.,7.);
    TProfile* p1_lightDetEff_cer_vs_x_ally = new TProfile("p1_lightDetEff_cer_vs_x_ally","",70,-7.,7.);
    TProfile* p1_lightDetEff_cer_vs_x_cuty = new TProfile("p1_lightDetEff_cer_vs_x_cuty","",70,-7.,7.);
    TProfile* p1_lightDetEff_all_vs_x_ally = new TProfile("p1_lightDetEff_all_vs_x_ally","",70,-7.,7.);
    TProfile* p1_lightDetEff_all_vs_x_cuty = new TProfile("p1_lightDetEff_all_vs_x_cuty","",70,-7.,7.);
    
    TH1F* h1_timeProfileDet_sci = new TH1F("h1_timeProfileDet_sci","",20000,-500.,19500.);
    TH1F* h1_timeProfileDet_cer = new TH1F("h1_timeProfileDet_cer","",20000,-500.,19500.);
    TH1F* h1_timeProfileDet_all = new TH1F("h1_timeProfileDet_all","",20000,-500.,19500.);
    
    TH1F* h1_signalAmp_sci = new TH1F("h1_signalAmp_sci","",10000,0.,10000*singleCellAmplitude);
    TH1F* h1_signalAmp_cer = new TH1F("h1_signalAmp_cer","",10000,0.,10000*singleCellAmplitude);
    TH1F* h1_signalAmp_all = new TH1F("h1_signalAmp_all","",10000,0.,10000*singleCellAmplitude);
    TProfile* p1_signalDet_sci = new TProfile("p1_signalDet_sci","",20000,-500.,19500.);
    TProfile* p1_signalDet_cer = new TProfile("p1_signalDet_cer","",20000,-500.,19500.);
    TProfile* p1_signalDet_all = new TProfile("p1_signalDet_all","",20000,-500.,19500.);
    
    std::vector<float> thresholds;
    thresholds.push_back(1);
    thresholds.push_back(2);
    thresholds.push_back(3);
    thresholds.push_back(5);
    thresholds.push_back(10);
    thresholds.push_back(20);
    thresholds.push_back(30);
    thresholds.push_back(50);
    thresholds.push_back(100);
    std::map<float,TH1F*> h1_timeRes_timeIthPhoton_sci;
    std::map<float,TH1F*> h1_timeRes_timeIthPhoton_cer;
    std::map<float,TH1F*> h1_timeRes_timeIthPhoton_all;
    std::map<float,TH1F*> h1_timeRes_avgTimeNPhotons_sci;
    std::map<float,TH1F*> h1_timeRes_avgTimeNPhotons_cer;
    std::map<float,TH1F*> h1_timeRes_avgTimeNPhotons_all;
    std::map<float,TH1F*> h1_timeRes_signalNPhotons_sci;
    std::map<float,TH1F*> h1_timeRes_signalNPhotons_cer;
    std::map<float,TH1F*> h1_timeRes_signalNPhotons_all;
    std::map<float,TH1F*> h1_timeRes_signalNPhotonsCorr_sci;
    std::map<float,TH1F*> h1_timeRes_signalNPhotonsCorr_cer;
    std::map<float,TH1F*> h1_timeRes_signalNPhotonsCorr_all;
    std::map<float,TProfile*> p1_timeRes_signalNPhotons_vs_amp_sci;
    std::map<float,TProfile*> p1_timeRes_signalNPhotons_vs_amp_cer;
    std::map<float,TProfile*> p1_timeRes_signalNPhotons_vs_amp_all;
    std::map<float,std::vector<std::pair<float,float> > > amp_timeRes_sci;
    std::map<float,std::vector<std::pair<float,float> > > amp_timeRes_cer;
    std::map<float,std::vector<std::pair<float,float> > > amp_timeRes_all;
    for(unsigned int thIt = 0; thIt < thresholds.size(); ++thIt)
    {
      float th = thresholds.at(thIt);
      h1_timeRes_timeIthPhoton_sci[th] = new TH1F(Form("h1_timeRes_time%.0fthPhoton_sci",th),"",2000,-500.,1500.);
      h1_timeRes_timeIthPhoton_cer[th] = new TH1F(Form("h1_timeRes_time%.0fthPhoton_cer",th),"",2000,-500.,1500.);
      h1_timeRes_timeIthPhoton_all[th] = new TH1F(Form("h1_timeRes_time%.0fthPhoton_all",th),"",2000,-500.,1500.);
      h1_timeRes_avgTimeNPhotons_sci[th] = new TH1F(Form("h1_timeRes_avgTime%.0fPhotons_sci",th),"",2000,-500.,1500.);
      h1_timeRes_avgTimeNPhotons_cer[th] = new TH1F(Form("h1_timeRes_avgTime%.0fPhotons_cer",th),"",2000,-500.,1500.);
      h1_timeRes_avgTimeNPhotons_all[th] = new TH1F(Form("h1_timeRes_avgTime%.0fPhotons_all",th),"",2000,-500.,1500.);
      h1_timeRes_signalNPhotons_sci[th] = new TH1F(Form("h1_timeRes_signal%.0fPhotons_sci",th),"",2000,-500.,1500.);
      h1_timeRes_signalNPhotons_cer[th] = new TH1F(Form("h1_timeRes_signal%.0fPhotons_cer",th),"",2000,-500.,1500.);
      h1_timeRes_signalNPhotons_all[th] = new TH1F(Form("h1_timeRes_signal%.0fPhotons_all",th),"",2000,-500.,1500.);
      h1_timeRes_signalNPhotonsCorr_sci[th] = new TH1F(Form("h1_timeRes_signal%.0fPhotonsCorr_sci",th),"",2000,-500.,1500.);
      h1_timeRes_signalNPhotonsCorr_cer[th] = new TH1F(Form("h1_timeRes_signal%.0fPhotonsCorr_cer",th),"",2000,-500.,1500.);
      h1_timeRes_signalNPhotonsCorr_all[th] = new TH1F(Form("h1_timeRes_signal%.0fPhotonsCorr_all",th),"",2000,-500.,1500.);
      p1_timeRes_signalNPhotons_vs_amp_sci[th] = new TProfile(Form("p1_timeRes_signal%.0fPhotons_vs_amp_sci",th),"",10000,0.,10000.*singleCellAmplitude);
      p1_timeRes_signalNPhotons_vs_amp_cer[th] = new TProfile(Form("p1_timeRes_signal%.0fPhotons_vs_amp_cer",th),"",10000,0.,10000.*singleCellAmplitude);
      p1_timeRes_signalNPhotons_vs_amp_all[th] = new TProfile(Form("p1_timeRes_signal%.0fPhotons_vs_amp_all",th),"",10000,0.,10000.*singleCellAmplitude);
    }
    
    
    //-----------------
    // loop over events
    TRandom3 r;
    int nEntries = chain -> GetEntries();
    if( maxEntries < 0 ) maxEntries = nEntries;
    for(int entry = 0; entry < maxEntries; ++entry)
    {
      //if( entry%10 == 0 ) std::cout << ">>> loop 1/1: reading entry " << entry << " / " << maxEntries << "\r" << std::flush;
      if( entry%10 == 0 ) std::cout << ">>> loop 1/1: reading entry " << entry << " / " << maxEntries << std::endl;
      chain -> GetEntry(entry);
      
      float beamX = treeVars.inputInitialPosition -> at(0);
      float beamY = treeVars.inputInitialPosition -> at(1);
      
      if( treeVars.depositedEnergyCrystal == 0. ) continue;
      //if( treeVars.tot_phot_sci == 0 && treeVars.tot_phot_cer == 0 ) continue;
      
      h1_depositedEnergyCrystal -> Fill(treeVars.depositedEnergyCrystal);
      
      h1_lightColl_sci -> Fill( treeVars.tot_det_phot_sci );
      h1_lightColl_cer -> Fill( treeVars.tot_det_phot_cer );
      h1_lightColl_all -> Fill( treeVars.tot_det_phot_sci+treeVars.tot_det_phot_cer );
      if( treeVars.tot_phot_sci > 0. )
        h1_lightCollEff_sci -> Fill( 1.*treeVars.tot_det_phot_sci/treeVars.tot_phot_sci );
      if( treeVars.tot_phot_cer > 0. )
        h1_lightCollEff_cer -> Fill( 1.*treeVars.tot_det_phot_cer/treeVars.tot_phot_cer );
      if( (treeVars.tot_phot_sci+treeVars.tot_phot_cer) > 0. )
        h1_lightCollEff_all -> Fill( 1.*(treeVars.tot_det_phot_sci+treeVars.tot_det_phot_cer)/(treeVars.tot_phot_sci+treeVars.tot_phot_cer) );
      
      p2_lightColl_sci_vs_xy -> Fill( beamX,beamY,treeVars.tot_det_phot_sci );
      p2_lightColl_cer_vs_xy -> Fill( beamX,beamY,treeVars.tot_det_phot_cer );
      p2_lightColl_all_vs_xy -> Fill( beamX,beamY,treeVars.tot_det_phot_sci+treeVars.tot_det_phot_cer );
      if( treeVars.tot_phot_sci > 0. )
        p2_lightCollEff_sci_vs_xy -> Fill( beamX,beamY,1.*treeVars.tot_det_phot_sci/treeVars.tot_phot_sci );
      if( treeVars.tot_phot_cer > 0. )
        p2_lightCollEff_cer_vs_xy -> Fill( beamX,beamY,1.*treeVars.tot_det_phot_cer/treeVars.tot_phot_cer );
      if( treeVars.tot_phot_sci > 0. || treeVars.tot_phot_cer > 0. )
        p2_lightCollEff_all_vs_xy -> Fill( beamX,beamY,1.*(treeVars.tot_det_phot_sci+treeVars.tot_det_phot_cer)/(treeVars.tot_phot_sci+treeVars.tot_phot_cer) );
      
      p1_lightColl_sci_vs_x_ally -> Fill( beamX,treeVars.tot_det_phot_sci );
      p1_lightColl_cer_vs_x_ally -> Fill( beamX,treeVars.tot_det_phot_cer );
      p1_lightColl_all_vs_x_ally -> Fill( beamX,treeVars.tot_det_phot_sci+treeVars.tot_det_phot_cer );
      if( treeVars.tot_phot_sci > 0. )
        p1_lightCollEff_sci_vs_x_ally -> Fill( beamX,1.*treeVars.tot_det_phot_sci/treeVars.tot_phot_sci );
      if( treeVars.tot_phot_cer > 0. )
        p1_lightCollEff_cer_vs_x_ally -> Fill( beamX,1.*treeVars.tot_det_phot_cer/treeVars.tot_phot_cer );
      if( treeVars.tot_phot_sci > 0. || treeVars.tot_phot_cer > 0. )
        p1_lightCollEff_all_vs_x_ally -> Fill( beamX,1.*(treeVars.tot_det_phot_sci+treeVars.tot_det_phot_cer)/(treeVars.tot_phot_sci+treeVars.tot_phot_cer) );
      
      if( beamY >= -1. && beamY <= +1. )
      {
        p1_lightColl_sci_vs_x_cuty -> Fill( beamX,treeVars.tot_det_phot_sci );
        p1_lightColl_cer_vs_x_cuty -> Fill( beamX,treeVars.tot_det_phot_cer );
        p1_lightColl_all_vs_x_cuty -> Fill( beamX,treeVars.tot_det_phot_sci+treeVars.tot_det_phot_cer );
        if( treeVars.tot_phot_sci > 0. )
          p1_lightCollEff_sci_vs_x_cuty -> Fill( beamX,1.*treeVars.tot_det_phot_sci/treeVars.tot_phot_sci );
        if( treeVars.tot_phot_cer > 0. )
          p1_lightCollEff_cer_vs_x_cuty -> Fill( beamX,1.*treeVars.tot_det_phot_cer/treeVars.tot_phot_cer );
        if( treeVars.tot_phot_sci > 0. || treeVars.tot_phot_cer > 0. )
          p1_lightCollEff_all_vs_x_cuty -> Fill( beamX,1.*(treeVars.tot_det_phot_sci+treeVars.tot_det_phot_cer)/(treeVars.tot_phot_sci+treeVars.tot_phot_cer) );
      }
      
      
      // apply PDE on scintillation and cherenkov photons
      int pde_det_phot_sci = 0;
      int pde_det_phot_cer = 0;
      int pde_det_phot_all = 0;
      std::vector<double> time_pde_det_phot_sci;
      std::vector<double> time_pde_det_phot_cer;
      std::vector<double> time_pde_det_phot_all;
      
      for(unsigned int it = 0; it < treeVars.time_det_phot_sci->size(); ++it)
      {
        float time = treeVars.time_det_phot_sci->at(it);
        float lambda = treeVars.lambda_det_phot_sci->at(it);
        
        h1_timeProfileColl_sci -> Fill( time );
        h1_timeProfileColl_all -> Fill( time );
        
        float PDE = GetPDE(SiPM)->Eval(lambda);
        if( r.Uniform(0.,1.) > PDE ) continue;
        
        h1_timeProfileDet_sci -> Fill( time );
        h1_timeProfileDet_all -> Fill( time );
        
        time_pde_det_phot_sci.push_back(time);
        time_pde_det_phot_all.push_back(time);
        ++pde_det_phot_sci;
        ++pde_det_phot_all;
      }
      
      for(unsigned int it = 0; it < treeVars.time_det_phot_cer->size(); ++it)
      {
        float time = treeVars.time_det_phot_cer->at(it);
        float lambda = treeVars.lambda_det_phot_cer->at(it);
        
        h1_timeProfileColl_cer -> Fill( time );
        h1_timeProfileColl_all -> Fill( time );
        
        float PDE = GetPDE(SiPM)->Eval(lambda);
        if( r.Uniform(0.,1.) > PDE ) continue;
        
        h1_timeProfileDet_cer -> Fill( time );
        h1_timeProfileDet_all -> Fill( time );
        
        time_pde_det_phot_cer.push_back(time);
        time_pde_det_phot_all.push_back(time);
        ++pde_det_phot_cer;
      }
      
      std::sort(time_pde_det_phot_sci.begin(),time_pde_det_phot_sci.end());
      std::sort(time_pde_det_phot_cer.begin(),time_pde_det_phot_cer.end());
      std::sort(time_pde_det_phot_all.begin(),time_pde_det_phot_all.end());
      
      
      // fill distributions
      h1_lightDet_sci -> Fill( pde_det_phot_sci );
      h1_lightDet_cer -> Fill( pde_det_phot_cer );
      h1_lightDet_all -> Fill( pde_det_phot_sci+pde_det_phot_cer );
      if( treeVars.tot_phot_sci > 0. )
        h1_lightDetEff_sci -> Fill( 1.*pde_det_phot_sci/treeVars.tot_phot_sci );
      if( treeVars.tot_phot_cer > 0. )
      h1_lightDetEff_cer -> Fill( 1.*pde_det_phot_cer/treeVars.tot_phot_cer );
      if( treeVars.tot_phot_sci+treeVars.tot_phot_cer > 0. )
        h1_lightDetEff_all -> Fill( 1.*(pde_det_phot_sci+pde_det_phot_cer)/(treeVars.tot_phot_sci+treeVars.tot_phot_cer) );
      
      p2_lightDet_sci_vs_xy -> Fill( beamX,beamY,pde_det_phot_sci );
      p2_lightDet_cer_vs_xy -> Fill( beamX,beamY,pde_det_phot_cer );
      p2_lightDet_all_vs_xy -> Fill( beamX,beamY,pde_det_phot_sci+pde_det_phot_cer );
      if( treeVars.time_det_phot_sci->size() > 0. )
        p2_lightDetEff_sci_vs_xy -> Fill( beamX,beamY,1.*pde_det_phot_sci/treeVars.time_phot_sci->size() );
      if( treeVars.time_det_phot_cer->size() > 0. )
        p2_lightDetEff_cer_vs_xy -> Fill( beamX,beamY,1.*pde_det_phot_cer/treeVars.time_phot_cer->size() );
      if( treeVars.time_det_phot_sci->size() > 0. || treeVars.time_det_phot_cer->size() > 0. )
        p2_lightDetEff_all_vs_xy -> Fill( beamX,beamY,1.*(pde_det_phot_sci+pde_det_phot_cer)/(treeVars.time_phot_sci->size()+treeVars.time_phot_cer->size()) );
      
      p1_lightDet_sci_vs_x_ally -> Fill( beamX,pde_det_phot_sci );
      p1_lightDet_cer_vs_x_ally -> Fill( beamX,pde_det_phot_cer );
      p1_lightDet_all_vs_x_ally -> Fill( beamX,pde_det_phot_sci+pde_det_phot_cer );
      if( treeVars.time_det_phot_sci->size() > 0. )
        p1_lightDetEff_sci_vs_x_ally -> Fill( beamX,1.*pde_det_phot_sci/treeVars.time_phot_sci->size() );
      if( treeVars.time_det_phot_cer->size() > 0. )
        p1_lightDetEff_cer_vs_x_ally -> Fill( beamX,1.*pde_det_phot_cer/treeVars.time_phot_cer->size() );
      if( treeVars.time_det_phot_sci->size() > 0. || treeVars.time_det_phot_cer->size() > 0. )
        p1_lightDetEff_all_vs_x_ally -> Fill( beamX,1.*(pde_det_phot_sci+pde_det_phot_cer)/(treeVars.time_phot_sci->size()+treeVars.time_phot_cer->size()) );
      
      if( beamY >= -1. && beamY <= +1. )
      {
        p1_lightDet_sci_vs_x_cuty -> Fill( beamX,pde_det_phot_sci );
        p1_lightDet_cer_vs_x_cuty -> Fill( beamX,pde_det_phot_cer );
        p1_lightDet_all_vs_x_cuty -> Fill( beamX,pde_det_phot_sci+pde_det_phot_cer );
        if( treeVars.time_det_phot_sci->size() > 0. )
          p1_lightDetEff_sci_vs_x_cuty -> Fill( beamX,1.*pde_det_phot_sci/treeVars.time_phot_sci->size() );
        if( treeVars.time_det_phot_cer->size() > 0. )
          p1_lightDetEff_cer_vs_x_cuty -> Fill( beamX,1.*pde_det_phot_cer/treeVars.time_phot_cer->size() );
        if( treeVars.time_det_phot_sci->size() > 0. || treeVars.time_det_phot_cer->size() > 0. )
          p1_lightDetEff_all_vs_x_cuty -> Fill( beamX,1.*(pde_det_phot_sci+pde_det_phot_cer)/(treeVars.time_phot_sci->size()+treeVars.time_phot_cer->size()) );
      }
      
      
      // build SiPM response function
      std::map<float,float> time_signalNPhotons_sci;
      std::map<float,float> time_signalNPhotons_cer;
      std::map<float,float> time_signalNPhotons_all;
      float amp_signal_sci = -9999.;
      float amp_signal_cer = -9999.;
      float amp_signal_all = -9999.;
      for(unsigned int thIt = 0; thIt < thresholds.size(); ++thIt)
      {
        float th = thresholds.at(thIt);
        time_signalNPhotons_sci[th] = -9999.;
        time_signalNPhotons_cer[th] = -9999.;
        time_signalNPhotons_all[th] = -9999.;
      }
      
      if( doResponseFunction )
      {        
        double* x = new double[1];
        
        double* SiPMResponseParameters_sci = new double[5+pde_det_phot_sci*2];
        double* SiPMResponseParameters_cer = new double[5+pde_det_phot_cer*2];
        SiPMResponseParameters_sci[0] = -9999.;
        SiPMResponseParameters_sci[1] = 0.;
        SiPMResponseParameters_sci[2] = riseTime;
        SiPMResponseParameters_sci[3] = decayTime;
        SiPMResponseParameters_sci[4] = pde_det_phot_sci;
        SiPMResponseParameters_cer[0] = -9999.;
        SiPMResponseParameters_cer[1] = 0.;
        SiPMResponseParameters_cer[2] = riseTime;
        SiPMResponseParameters_cer[3] = decayTime;
        SiPMResponseParameters_cer[4] = pde_det_phot_cer;
        
        for(unsigned int it = 0; it < time_pde_det_phot_sci.size(); ++it)
        {
          if( SPTR > 0. ) SiPMResponseParameters_sci[5+it*2] = time_pde_det_phot_sci.at(it) + r.Gaus(0.,SPTR);
          else            SiPMResponseParameters_sci[5+it*2] = time_pde_det_phot_sci.at(it);            
          SiPMResponseParameters_sci[6+it*2] = singleCellAmplitude + r.Gaus(0.,0.10*singleCellAmplitude);
        }
        for(unsigned int it = 0; it < time_pde_det_phot_cer.size(); ++it)
        {
          if( SPTR > 0. ) SiPMResponseParameters_cer[5+it*2] = time_pde_det_phot_cer.at(it) + r.Gaus(0.,SPTR);
          else            SiPMResponseParameters_cer[5+it*2] = time_pde_det_phot_cer.at(it);
          SiPMResponseParameters_cer[6+it*2] = singleCellAmplitude + r.Gaus(0.,0.10*singleCellAmplitude);
          //std::cout << "photon it: " << it << "   time: " << time_pde_det_phot_cer.at(it) << "   time + SPTR: " << SiPMResponseParameters_cer[5+it*2] << std::endl;
        }
        
          
        for(int bin = 1; bin <= p1_signalDet_sci->GetNbinsX(); ++bin)
        {
          x[0] = p1_signalDet_sci -> GetBinCenter(bin);
          
          double val_sci = SiPMResponse(x,SiPMResponseParameters_sci);
          double val_cer = SiPMResponse(x,SiPMResponseParameters_cer);
          double val_all = val_sci + val_cer;
          
          if( val_sci > amp_signal_sci ) amp_signal_sci = val_sci;
          if( val_cer > amp_signal_cer ) amp_signal_cer = val_cer;
          if( val_all > amp_signal_all ) amp_signal_all = val_all;
          
          p1_signalDet_sci -> Fill( x[0],val_sci );
          p1_signalDet_cer -> Fill( x[0],val_cer );
          p1_signalDet_all -> Fill( x[0],val_all );
          
          for(unsigned int thIt = 0; thIt < thresholds.size(); ++thIt)
          {
            float th = thresholds.at(thIt);
            float th_signal = 0.5 * (th*singleCellAmplitude);
            
            if( val_sci >= th_signal && time_signalNPhotons_sci[th] == -9999. ) time_signalNPhotons_sci[th] = x[0];
            if( val_cer >= th_signal && time_signalNPhotons_cer[th] == -9999. ) time_signalNPhotons_cer[th] = x[0];
            if( val_all >= th_signal && time_signalNPhotons_all[th] == -9999. ) time_signalNPhotons_all[th] = x[0];
            //std::cout << "x: " << x[0] << "   val: " << val_cer << "   th: " << th << "   th_signal: " << th_signal << "   time: " << time_signalNPhotons_cer[th] << std::endl;
          }
        }
        
        
        h1_signalAmp_sci -> Fill( amp_signal_sci );
        h1_signalAmp_cer -> Fill( amp_signal_cer );
        h1_signalAmp_all -> Fill( amp_signal_all );
        
        
        delete x;
        delete SiPMResponseParameters_sci;
        delete SiPMResponseParameters_cer;
      }
      
      
      // time resolution plots
      for(unsigned int thIt = 0; thIt < thresholds.size(); ++thIt)
      {
        float th = thresholds.at(thIt);
        
        if( time_pde_det_phot_sci.size() >= th ) h1_timeRes_timeIthPhoton_sci[th] -> Fill( time_pde_det_phot_sci.at(th-1) );
        if( time_pde_det_phot_cer.size() >= th ) h1_timeRes_timeIthPhoton_cer[th] -> Fill( time_pde_det_phot_cer.at(th-1) );
        if( time_pde_det_phot_all.size() >= th ) h1_timeRes_timeIthPhoton_all[th] -> Fill( time_pde_det_phot_all.at(th-1) );
        
        if( time_pde_det_phot_sci.size() >= th ) h1_timeRes_avgTimeNPhotons_sci[th] -> Fill( std::accumulate(time_pde_det_phot_sci.begin(),time_pde_det_phot_sci.begin()+th,0.)/th );
        if( time_pde_det_phot_cer.size() >= th ) h1_timeRes_avgTimeNPhotons_cer[th] -> Fill( std::accumulate(time_pde_det_phot_cer.begin(),time_pde_det_phot_cer.begin()+th,0.)/th );
        if( time_pde_det_phot_all.size() >= th ) h1_timeRes_avgTimeNPhotons_all[th] -> Fill( std::accumulate(time_pde_det_phot_all.begin(),time_pde_det_phot_all.begin()+th,0.)/th );
        
        if( time_signalNPhotons_sci[th] > -9999. )
        {
          h1_timeRes_signalNPhotons_sci[th] -> Fill( time_signalNPhotons_sci[th] );
          p1_timeRes_signalNPhotons_vs_amp_sci[th] -> Fill( amp_signal_sci,time_signalNPhotons_sci[th] );
          
          std::pair<float,float> dummy(amp_signal_sci,time_signalNPhotons_sci[th]);
          amp_timeRes_sci[th].push_back(dummy);
        }
        if( time_signalNPhotons_cer[th] > -9999. )
        {
          h1_timeRes_signalNPhotons_cer[th] -> Fill( time_signalNPhotons_cer[th] );
          p1_timeRes_signalNPhotons_vs_amp_cer[th] -> Fill( amp_signal_cer,time_signalNPhotons_cer[th] );
          
          std::pair<float,float> dummy(amp_signal_cer,time_signalNPhotons_cer[th]);
          amp_timeRes_cer[th].push_back(dummy);
        }
        if( time_signalNPhotons_all[th] > -9999. )
        {
          h1_timeRes_signalNPhotons_all[th] -> Fill( time_signalNPhotons_all[th] );
          p1_timeRes_signalNPhotons_vs_amp_all[th] -> Fill( amp_signal_all,time_signalNPhotons_all[th] );
          
          std::pair<float,float> dummy(amp_signal_all,time_signalNPhotons_all[th]);
          amp_timeRes_all[th].push_back(dummy);
        }
      }
    }
    
    
    
    // amplitude walk correction
    if( doResponseFunction )
    {        
      for(unsigned int thIt = 0; thIt < thresholds.size(); ++thIt)
      {
        float th = thresholds.at(thIt);
        
        TF1* fitFunc_sci = new TF1(Form("fitFunc_%.0fPhotons_sci",th),"[0]*log([1]*x)+[2]",0.,10000.);
        fitFunc_sci -> SetParameters(-100.,0.008,0.);
        fitFunc_sci -> SetLineColor(kRed);
        TF1* fitFunc_cer = new TF1(Form("fitFunc_%.0fPhotons_cer",th),"[0]*log([1]*x)+[2]",0.,10000.);
        fitFunc_cer -> SetParameters(-100.,0.008,0.);
        fitFunc_cer -> SetLineColor(kRed);
        TF1* fitFunc_all = new TF1(Form("fitFunc_%.0fPhotons_all",th),"[0]*log([1]*x)+[2]",0.,10000.);
        fitFunc_all -> SetParameters(-100.,0.008,0.);
        fitFunc_all -> SetLineColor(kRed);
        
        if( p1_timeRes_signalNPhotons_vs_amp_sci[th]->GetEntries() > 0 )
        {
          std::cout << ">>> Fitting " << p1_timeRes_signalNPhotons_vs_amp_sci[th]->GetName() << std::endl;
          p1_timeRes_signalNPhotons_vs_amp_sci[th] -> Fit(Form("fitFunc_%.0fPhotons_sci",th),"RS+");
        }
        if( p1_timeRes_signalNPhotons_vs_amp_cer[th]->GetEntries() > 0 )
        {
          std::cout << ">>> Fitting " << p1_timeRes_signalNPhotons_vs_amp_cer[th]->GetName() << std::endl;
          p1_timeRes_signalNPhotons_vs_amp_cer[th] -> Fit(Form("fitFunc_%.0fPhotons_cer",th),"RS+");
        }
        if( p1_timeRes_signalNPhotons_vs_amp_all[th]->GetEntries() > 0 )
        {
          std::cout << ">>> Fitting " << p1_timeRes_signalNPhotons_vs_amp_all[th]->GetName() << std::endl;
          p1_timeRes_signalNPhotons_vs_amp_all[th] -> Fit(Form("fitFunc_%.0fPhotons_all",th),"RS+");
        }
      }
      
      
      for(unsigned int thIt = 0; thIt < thresholds.size(); ++thIt)
      {
        float th = thresholds.at(thIt);      
        
        std::vector<std::pair<float,float> > vals_sci = amp_timeRes_sci[th];
        for(unsigned int it = 0; it < vals_sci.size(); ++it)
        {
          float avgAmp = h1_signalAmp_sci -> GetMean();
          float amp = vals_sci.at(it).first;
          float timeRes = vals_sci.at(it).second;
          
          h1_timeRes_signalNPhotonsCorr_sci[th] -> Fill( timeRes -
                                                         p1_timeRes_signalNPhotons_vs_amp_sci[th]->GetBinContent(p1_timeRes_signalNPhotons_vs_amp_sci[th]->FindBin(amp)) +
                                                         p1_timeRes_signalNPhotons_vs_amp_sci[th]->GetBinContent(p1_timeRes_signalNPhotons_vs_amp_sci[th]->FindBin(avgAmp)) );
        }
        
        std::vector<std::pair<float,float> > vals_cer = amp_timeRes_cer[th];
        for(unsigned int it = 0; it < vals_cer.size(); ++it)
        {
          float avgAmp = h1_signalAmp_cer -> GetMean();
          float amp = vals_cer.at(it).first;
          float timeRes = vals_cer.at(it).second;
          
          h1_timeRes_signalNPhotonsCorr_cer[th] -> Fill( timeRes -
                                                         p1_timeRes_signalNPhotons_vs_amp_cer[th]->GetBinContent(p1_timeRes_signalNPhotons_vs_amp_cer[th]->FindBin(amp)) +
                                                         p1_timeRes_signalNPhotons_vs_amp_cer[th]->GetBinContent(p1_timeRes_signalNPhotons_vs_amp_cer[th]->FindBin(avgAmp)) );
        }
        
        std::vector<std::pair<float,float> > vals_all = amp_timeRes_all[th];
        for(unsigned int it = 0; it < vals_all.size(); ++it)
        {
          float avgAmp = h1_signalAmp_all -> GetMean();
          float amp = vals_all.at(it).first;
          float timeRes = vals_all.at(it).second;
          
          h1_timeRes_signalNPhotonsCorr_all[th] -> Fill( timeRes -
                                                         p1_timeRes_signalNPhotons_vs_amp_all[th]->GetBinContent(p1_timeRes_signalNPhotons_vs_amp_all[th]->FindBin(amp)) +
                                                         p1_timeRes_signalNPhotons_vs_amp_all[th]->GetBinContent(p1_timeRes_signalNPhotons_vs_amp_all[th]->FindBin(avgAmp)) );
        }
      }
    }    
    
    
    // scale histograms and get cumulatives
    h1_timeProfileColl_sci -> Scale(1./nEntries);
    h1_timeProfileColl_cer -> Scale(1./nEntries);
    h1_timeProfileColl_all -> Scale(1./nEntries);
    h1_timeProfileDet_sci  -> Scale(1./nEntries);
    h1_timeProfileDet_cer  -> Scale(1./nEntries);
    h1_timeProfileDet_all  -> Scale(1./nEntries);
    
    h1_timeProfileColl_sci -> GetCumulative();
    h1_timeProfileColl_cer -> GetCumulative();
    h1_timeProfileColl_all -> GetCumulative();
    h1_timeProfileDet_sci -> GetCumulative();
    h1_timeProfileDet_cer -> GetCumulative();
    h1_timeProfileDet_all -> GetCumulative();
    
    outFile -> cd();
    
    int bytes = outFile -> Write();
    std::cout << "============================================"  << std::endl;
    std::cout << "nr of  B written:  " << int(bytes)             << std::endl;
    std::cout << "nr of KB written:  " << int(bytes/1024.)       << std::endl;
    std::cout << "nr of MB written:  " << int(bytes/1024./1024.) << std::endl;
    std::cout << "============================================"  << std::endl;
    
    outFile -> Close();
  }
  
  
  
  return 0;
}



void InitTreeVars(TTree* chain, TreeVars& treeVars)
{
  treeVars.inputInitialPosition = new std::vector<float>;
  
  treeVars.time_phot_sci = new std::vector<float>;
  treeVars.time_phot_cer = new std::vector<float>;
  treeVars.lambda_phot_sci = new std::vector<float>;
  treeVars.lambda_phot_cer = new std::vector<float>;
  treeVars.time_det_phot_sci = new std::vector<float>;
  treeVars.time_det_phot_cer = new std::vector<float>;
  treeVars.lambda_det_phot_sci = new std::vector<float>;
  treeVars.lambda_det_phot_cer = new std::vector<float>;
  
  chain -> SetBranchStatus("*",0);
  
  chain -> SetBranchStatus("depositedEnergyCrystal",1); chain -> SetBranchAddress("depositedEnergyCrystal",&treeVars.depositedEnergyCrystal);
  
  chain -> SetBranchStatus("tot_phot_sci",    1); chain -> SetBranchAddress("tot_phot_sci",    &treeVars.tot_phot_sci);
  chain -> SetBranchStatus("tot_phot_cer",    1); chain -> SetBranchAddress("tot_phot_cer",    &treeVars.tot_phot_cer);
  chain -> SetBranchStatus("tot_det_phot_sci",1); chain -> SetBranchAddress("tot_det_phot_sci",&treeVars.tot_det_phot_sci);
  chain -> SetBranchStatus("tot_det_phot_cer",1); chain -> SetBranchAddress("tot_det_phot_cer",&treeVars.tot_det_phot_cer);
  
  chain -> SetBranchStatus("inputInitialPosition",1); chain -> SetBranchAddress("inputInitialPosition",&treeVars.inputInitialPosition);
  
  chain -> SetBranchStatus("time_phot_sci",  1); chain -> SetBranchAddress("time_phot_sci",  &treeVars.time_phot_sci);
  chain -> SetBranchStatus("time_phot_cer",  1); chain -> SetBranchAddress("time_phot_cer",  &treeVars.time_phot_cer);
  chain -> SetBranchStatus("lambda_phot_sci",1); chain -> SetBranchAddress("lambda_phot_sci",&treeVars.lambda_phot_sci);
  chain -> SetBranchStatus("lambda_phot_cer",1); chain -> SetBranchAddress("lambda_phot_cer",&treeVars.lambda_phot_cer);
  chain -> SetBranchStatus("time_det_phot_sci",  1); chain -> SetBranchAddress("time_det_phot_sci",  &treeVars.time_det_phot_sci);
  chain -> SetBranchStatus("time_det_phot_cer",  1); chain -> SetBranchAddress("time_det_phot_cer",  &treeVars.time_det_phot_cer);
  chain -> SetBranchStatus("lambda_det_phot_sci",1); chain -> SetBranchAddress("lambda_det_phot_sci",&treeVars.lambda_det_phot_sci);
  chain -> SetBranchStatus("lambda_det_phot_cer",1); chain -> SetBranchAddress("lambda_det_phot_cer",&treeVars.lambda_det_phot_cer);
}
