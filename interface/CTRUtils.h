#include "interface/FitUtils.h"

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



/*** tree variables ***/
struct TreeVars
{
  unsigned int t_run;
  unsigned int t_spill;
  
  int t_LED;
  int t_CFD;
  std::map<std::string,int>* t_timeTypes;
  
  float* t_beamX;
  float* t_beamY;
  float* t_Vbias;
  float t_NINOthr;
  float t_angle;
  float* t_b_rms;
  float* t_ped;
  float* t_amp;
  float* t_dur;
  float* t_time;
  float* t_time_slope;
  std::map<std::string,int>* t_trgChannelId;
  std::map<std::string,int>* t_channelId;
};

void InitTreeVars(TTree* tree, TreeVars& treeVars, CfgManager& opts, const bool& hodo = false);
void InitTreeVarsPeacock(TTree* tree, TreeVars& treeVars, CfgManager& opts);

bool AcceptEvent(const std::string& enCh0, const std::string& enCh1, const std::string& timeCh0, const std::string& timeCh1, TreeVars& treeVars,
                 const int& beamCutType, const float& beamXMin, const float& beamXMax, const float& beamYMin, const float& beamYMax,
                 const float& angle);

bool AcceptEventMCP(TreeVars& treeVars, const float& x_MCP, const float& y_MCP, const float& r_MCP);

bool AcceptEventAmp(const std::string& enCh0, const std::string& enCh1, TreeVars& treeVars,
                    const float& ampMin1, const float& ampMax1, const float& ampMin2, const float& ampMax2);

bool AcceptEventTime(const std::string& timeCh0, const std::string& timeCh1, const bool& isMCP0, const bool& isMCP1, TreeVars& treeVars,
                     const float& timeMin1, const float& timeMax1, const float& timeMin2, const float& timeMax2);

bool AcceptEventDur(const std::string& timeCh0, const std::string& timeCh1, TreeVars& treeVars,
                    const float& durMin1, const float& durMax1, const float& durMin2, const float& durMax2);

bool AcceptEventTh(TreeVars& treeVars, const float& thMin, const float& thMax);

bool AcceptEventVbias(const int& VbiasIndex1, const int& VbiasIndex2, const int& isMCP0, const int& isMCP1, TreeVars& treeVars,
                      const float& VbiasMin1, const float& VbiasMax1, const float& VbiasMin2, const float& VbiasMax2);

void drawCTRPlot(TH1F* histo, const int& rebin, const int& isMCP0, const int& isMCP1, const float& MCPIntrinsic,
                 const std::string& channelLabel0, const std::string& channelLabel1, const std::string&extraLabel, TLatex* latexLabel,
                 TH1F* histo_center = NULL, TH1F* histo_border = NULL);
