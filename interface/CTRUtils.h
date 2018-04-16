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
  int t_LED;
  int t_CFD;
  
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
  
  int t_CFD;
  int t_LED;
};

struct TreeVarsCosmics
{
  float* t_ped;
  float* t_amp;
  float* t_charge;
  float* t_time;
  std::map<std::string,int>* t_channelId;
  
  int t_CFD;
  int t_LED;
  std::map<std::string,int>* t_timeTypes;
};

void InitTreeVars(TTree* tree, TreeVars& treeVars, CfgManager& opts);
void InitTreeVarsCosmics(TTree* tree, TreeVarsCosmics& treeVars, CfgManager& opts);

bool AcceptEvent(const std::string& enCh0, const std::string& enCh1, const std::string& timeCh0, const std::string& timeCh1, TreeVars& treeVars,
                 const int& beamCutType, const float& beamXMin, const float& beamXMax, const float& beamYMin, const float& beamYMax,
                 const float& angle);

bool AcceptEventMCP(TreeVars& treeVars, const float& x_MCP, const float& y_MCP, const float& r_MCP);

template <class T>
bool AcceptEventAmp(const std::string& enCh0, const std::string& enCh1, T& treeVars,
                    const float& ampMin1, const float& ampMax1, const float& ampMin2, const float& ampMax2)
{
  if( isnan(treeVars.t_amp[(*treeVars.t_channelId)[enCh0]]) ) return false;
  if( isnan(treeVars.t_amp[(*treeVars.t_channelId)[enCh1]]) ) return false;
  if( (treeVars.t_amp[(*treeVars.t_channelId)[enCh0]]*0.25 < ampMin1) || (treeVars.t_amp[(*treeVars.t_channelId)[enCh0]]*0.25 > ampMax1) ) return false;
  if( (treeVars.t_amp[(*treeVars.t_channelId)[enCh1]]*0.25 < ampMin2) || (treeVars.t_amp[(*treeVars.t_channelId)[enCh1]]*0.25 > ampMax2) ) return false;
  
  return true;
};

template <class T>
bool AcceptEventTime(const std::string& timeCh0, const std::string& timeCh1, const bool& isMCP0, const bool& isMCP1, T& treeVars,
                     const float& timeMin1, const float& timeMax1, const float& timeMin2, const float& timeMax2)
{
  int extraIt0 = isMCP0 ? 14 : 0;
  int extraIt1 = isMCP1 ? 14 : 0;
  
  if( isnan(treeVars.t_time[(*treeVars.t_channelId)[timeCh0]+extraIt0]) ) return false;
  if( isnan(treeVars.t_time[(*treeVars.t_channelId)[timeCh1]+extraIt1]) ) return false;
  if( (treeVars.t_time[(*treeVars.t_channelId)[timeCh0]+extraIt0] < timeMin1) || (treeVars.t_time[(*treeVars.t_channelId)[timeCh0]+extraIt0] > timeMax1) ) return false;
  if( (treeVars.t_time[(*treeVars.t_channelId)[timeCh1]+extraIt1] < timeMin2) || (treeVars.t_time[(*treeVars.t_channelId)[timeCh1]+extraIt1] > timeMax2) ) return false;
  if( fabs( treeVars.t_time[(*treeVars.t_channelId)[timeCh1]+extraIt1] - treeVars.t_time[(*treeVars.t_channelId)[timeCh0]+extraIt0] ) > 1. ) return false;
  return true;
};


bool AcceptEventDur(const std::string& timeCh0, const std::string& timeCh1, TreeVars& treeVars,
                    const float& durMin1, const float& durMax1, const float& durMin2, const float& durMax2);

bool AcceptEventTh(TreeVars& treeVars, const float& thMin, const float& thMax);

bool AcceptEventVbias(const int& VbiasIndex1, const int& VbiasIndex2, TreeVars& treeVars,
                      const float& VbiasMin, const float& VbiasMax);

void drawCTRPlot(TH1F* histo, TCanvas* c1, const int& rebin, const int& isMCP0, const int& isMCP1, const float& MCPIntrinsic,
                 const std::string& channelLabel0, const std::string& channelLabel1, const std::string&extraLabel, TLatex* latexLabel,
                 TH1F* histo_center = NULL, TH1F* histo_border = NULL);
