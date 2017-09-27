#include "interface/CTRUtils.h"



void InitTreeVars(TTree* tree, TreeVars& treeVars, CfgManager& opts)
{
  treeVars.t_Vbias = new float[3];
  treeVars.t_amp = new float[32];
  treeVars.t_dur = new float[32];
  treeVars.t_time = new float[64];
  treeVars.t_beamX = new float[2];
  treeVars.t_beamY = new float[2];
  treeVars.t_channelId = new std::map<std::string,int>;
  
  //tree -> SetBranchStatus("*",0);
  
  tree -> SetBranchStatus("LED",1);       tree -> SetBranchAddress("LED",      &treeVars.t_LED);
  tree -> SetBranchStatus("CFD",1);       tree -> SetBranchAddress("CFD",      &treeVars.t_CFD);
  
  // tree -> SetBranchStatus("NINOthr",1); tree -> SetBranchAddress("NINOthr",&treeVars.t_NINOthr);
  // tree -> SetBranchStatus("Vbias1" ,1); tree -> SetBranchAddress("Vbias1", &treeVars.t_Vbias[0]);
  // tree -> SetBranchStatus("Vbias2" ,1); tree -> SetBranchAddress("Vbias2", &treeVars.t_Vbias[1]);
  // tree -> SetBranchStatus("Vbias3" ,1); tree -> SetBranchAddress("Vbias3", &treeVars.t_Vbias[2]);

  float cut_angle = opts.GetOpt<float>("Cuts.angle");
  if( cut_angle != -1 )
  {
    tree -> SetBranchStatus("angle"  ,1); tree -> SetBranchAddress("angle",  &treeVars.t_angle);
  }
  
  tree -> SetBranchStatus("X",1); tree -> SetBranchAddress("X",treeVars.t_beamX);
  tree -> SetBranchStatus("Y",1); tree -> SetBranchAddress("Y",treeVars.t_beamY);
  
  tree -> SetBranchStatus("amp_max",   1); tree -> SetBranchAddress("amp_max",   treeVars.t_amp);
  tree -> SetBranchStatus("charge_sig",1); tree -> SetBranchAddress("charge_sig",treeVars.t_dur);
  tree -> SetBranchStatus("time",      1); tree -> SetBranchAddress("time",      treeVars.t_time);
  
  std::vector<std::string> timeChannels = opts.GetOpt<std::vector<std::string> >("Input.timeChannels");
  for(unsigned int it = 0; it < timeChannels.size(); ++it)
  {
    std::string channelName = timeChannels.at(it);
    tree -> SetBranchStatus(channelName.c_str(),1); tree -> SetBranchAddress(channelName.c_str(),&((*treeVars.t_channelId)[channelName]));
  }
  std::vector<std::string> energyChannels = opts.GetOpt<std::vector<std::string> >("Input.energyChannels");
  for(unsigned int it = 0; it < energyChannels.size(); ++it)
  {
    std::string channelName = energyChannels.at(it);
    tree -> SetBranchStatus(channelName.c_str(),1); tree -> SetBranchAddress(channelName.c_str(),&((*treeVars.t_channelId)[channelName]));
  }
}



bool AcceptEvent(const std::string& enCh0, const std::string& enCh1, const std::string& timeCh0, const std::string& timeCh1, TreeVars& treeVars,
                 const int& beamCutType, const float& beamXMin, const float& beamXMax, const float& beamYMin, const float& beamYMax,
                 const float& angle)
{
  //---------------------------
  // selection on beam position
  
  // if( treeVars.t_beamX[1] >= -15.0 && treeVars.t_beamX[1] < -14.5 ) return false;
  // if( treeVars.t_beamX[1] >=   8.5 && treeVars.t_beamX[1] <   9.5 ) return false;
  // if( treeVars.t_beamY[0] >= -16.0 && treeVars.t_beamY[0] < -15.5 ) return false;
  // if( treeVars.t_beamY[1] >= -16.0 && treeVars.t_beamY[1] < -15.5 ) return false;
  
  // if( fabs(treeVars.t_beamX[1]-treeVars.t_beamX[0]) > 1. ) return false;
  // if( fabs(treeVars.t_beamY[1]-treeVars.t_beamY[0]) > 1. ) return false;
  
  
  //-------------------------------
  // selection on channel amplitude
  
  if( treeVars.t_amp[(*treeVars.t_channelId)[enCh0]]*0.25 < 0.    ) return false;
  if( treeVars.t_amp[(*treeVars.t_channelId)[enCh1]]*0.25 < 0.    ) return false;
  if( treeVars.t_amp[(*treeVars.t_channelId)[enCh0]]*0.25 > 1000. ) return false;
  if( treeVars.t_amp[(*treeVars.t_channelId)[enCh1]]*0.25 > 1000. ) return false;
  if( isnan(treeVars.t_amp[(*treeVars.t_channelId)[enCh0]]) ) return false;
  if( isnan(treeVars.t_amp[(*treeVars.t_channelId)[enCh1]]) ) return false;
  
  
  //--------------------------
  // selection on channel time
  
  // if( treeVars.t_time[(*treeVars.t_channelId)[timeCh0]] < 0.   ) return false;
  // if( treeVars.t_time[(*treeVars.t_channelId)[timeCh1]] < 0.   ) return false;
  // if( treeVars.t_time[(*treeVars.t_channelId)[timeCh0]] > 200. ) return false;
  // if( treeVars.t_time[(*treeVars.t_channelId)[timeCh1]] > 200. ) return false;
  // if( isnan(treeVars.t_time[(*treeVars.t_channelId)[timeCh0]]) ) return false;
  // if( isnan(treeVars.t_time[(*treeVars.t_channelId)[timeCh1]]) ) return false;
  
  
  //-------------------
  // selection on angle
  
  if( angle > -1 )
  {
    if( treeVars.t_angle != angle ) return false;
  }
  
  
  //-----------------------
  // selection on beam spot
  
  if( beamCutType <= 0 ) return true;
  
  bool inside = false;
  if( treeVars.t_beamX[0] >= beamXMin && treeVars.t_beamX[0] < beamXMax &&
      treeVars.t_beamY[0] >= beamYMin && treeVars.t_beamY[0] < beamYMax )
    inside = true;
  
  if     ( beamCutType == 1 && inside == true ) return true;
  else if( beamCutType == 1 && inside == false ) return false;
  else if( beamCutType == 2 && inside == true ) return false;
  else if( beamCutType == 2 && inside == false ) return true;
  
  
  return true;
}



bool AcceptEventMCP(TreeVars& treeVars, const float& x_MCP, const float& y_MCP, const float& r_MCP)
{
  //-----------------------
  // selection on beam spot
  
  if( ( (treeVars.t_beamX[0]-x_MCP)*(treeVars.t_beamX[0]-x_MCP) +
        (treeVars.t_beamY[0]-y_MCP)*(treeVars.t_beamY[0]-y_MCP) ) > (r_MCP*r_MCP) )
    return false;
  
  return true;
}



bool AcceptEventAmp(const std::string& enCh0, const std::string& enCh1, TreeVars& treeVars,
                    const float& ampMin1, const float& ampMax1, const float& ampMin2, const float& ampMax2)
{
  if( isnan(treeVars.t_amp[(*treeVars.t_channelId)[enCh0]]) ) return false;
  if( isnan(treeVars.t_amp[(*treeVars.t_channelId)[enCh1]]) ) return false;
  if( (treeVars.t_amp[(*treeVars.t_channelId)[enCh0]]*0.25 < ampMin1) || (treeVars.t_amp[(*treeVars.t_channelId)[enCh0]]*0.25 > ampMax1) ) return false;
  if( (treeVars.t_amp[(*treeVars.t_channelId)[enCh1]]*0.25 < ampMin2) || (treeVars.t_amp[(*treeVars.t_channelId)[enCh1]]*0.25 > ampMax2) ) return false;
  
  return true;
}



bool AcceptEventTime(const std::string& timeCh0, const std::string& timeCh1, const bool& isMCP0, const bool& isMCP1, TreeVars& treeVars,
                    const float& timeMin1, const float& timeMax1, const float& timeMin2, const float& timeMax2)
{
  int extraIt0 = isMCP0 ? treeVars.t_CFD : treeVars.t_LED;
  int extraIt1 = isMCP1 ? treeVars.t_CFD : treeVars.t_LED;
  
  if( isnan(treeVars.t_time[(*treeVars.t_channelId)[timeCh0]+extraIt0]) ) return false;
  if( isnan(treeVars.t_time[(*treeVars.t_channelId)[timeCh1]+extraIt1]) ) return false;
  if( (treeVars.t_time[(*treeVars.t_channelId)[timeCh0]+extraIt0] < timeMin1) || (treeVars.t_time[(*treeVars.t_channelId)[timeCh0]+extraIt0] > timeMax1) ) return false;
  if( (treeVars.t_time[(*treeVars.t_channelId)[timeCh1]+extraIt1] < timeMin2) || (treeVars.t_time[(*treeVars.t_channelId)[timeCh1]+extraIt1] > timeMax2) ) return false;
  if( fabs( treeVars.t_time[(*treeVars.t_channelId)[timeCh1]+extraIt1] - treeVars.t_time[(*treeVars.t_channelId)[timeCh0]+extraIt0] ) > 25. ) return false;
  return true;
}



bool AcceptEventDur(const std::string& timeCh0, const std::string& timeCh1, TreeVars& treeVars,
                    const float& durMin1, const float& durMax1, const float& durMin2, const float& durMax2)
{
  if( (treeVars.t_dur[(*treeVars.t_channelId)[timeCh0]]*0.2 < durMin1) || (treeVars.t_dur[(*treeVars.t_channelId)[timeCh0]]*0.2 > durMax1) ) return false;
  if( (treeVars.t_dur[(*treeVars.t_channelId)[timeCh1]]*0.2 < durMin2) || (treeVars.t_dur[(*treeVars.t_channelId)[timeCh1]]*0.2 > durMax2) ) return false;
  
  return true;
}



bool AcceptEventTh(TreeVars& treeVars, const float& thMin, const float& thMax)
{
  if( (treeVars.t_NINOthr < thMin) || (treeVars.t_NINOthr > thMax) ) return false;
  
  return true;
}



bool AcceptEventVbias(const int& VbiasIndex1, const int& VbiasIndex2, TreeVars& treeVars,
                      const float& VbiasMin, const float& VbiasMax)
{
  float Vbias1 = treeVars.t_Vbias[VbiasIndex1];
  float Vbias2 = treeVars.t_Vbias[VbiasIndex2];
  if( (Vbias1 < VbiasMin) || (Vbias1 > VbiasMax) ) return false;
  if( (Vbias2 < VbiasMin) || (Vbias2 > VbiasMax) ) return false;
  
  return true;
}



void drawCTRPlot(TH1F* histo, TCanvas* c1, const int& rebin, const int& isMCP0, const int& isMCP1, const float& MCPIntrinsic,
                 const std::string& channelLabel0, const std::string& channelLabel1, const std::string& extraLabel, TLatex* latexLabel,
                 TH1F* histo_center, TH1F* histo_border)
{
  c1 -> cd();
  
  float* vals = new float[4];
  FindSmallestInterval(vals,histo,0.68,true);
  histo -> Rebin(rebin);
  histo -> SetMarkerStyle(20);
  histo -> SetMarkerColor(kBlack);
  histo -> SetLineColor(kBlack);
  histo -> SetMaximum(1.5*histo->GetMaximum());
  histo -> SetTitle(Form(";%s#Deltat = t_{%s} - t_{%s} (ns);events",extraLabel.c_str(),channelLabel0.c_str(),channelLabel1.c_str()));
  histo -> Draw("PE");
  
  float norm = histo -> GetMaximum();
  float mean = vals[0];
  float min = vals[2];
  float max = vals[3];
  float delta = max-min;
  float sigma = 0.5*delta;
  float effSigma = sigma;
  min = min - 2.*delta;
  max = max + 2.*delta;
  
  histo -> GetXaxis() -> SetRangeUser(min,max);  

  TLatex* latex;
  if( (isMCP0 && !isMCP1) || (!isMCP0 && isMCP1) )
    latex = new TLatex(0.16,0.85,Form("#sigma_{single}^{eff} = %.1f ps",
                                      sqrt(effSigma*effSigma - MCPIntrinsic*MCPIntrinsic)*1000.));
  else
    latex = new TLatex(0.16,0.85,Form("#sigma_{single}^{eff} = %.1f ps",fabs(effSigma*1000)/sqrt(2)));
  latex -> SetNDC();
  latex -> SetTextFont(42);
  latex -> SetTextSize(0.05);
  latex -> SetTextColor(kBlack);
  latex -> Draw("same");
  
  // gaus fit
  std::string gausName(Form("fitFunc_gaus_corr"));
  TF1* fitFunc_gaus_corr = new TF1(gausName.c_str(),"[0]*exp(-1.*(x-[1])*(x-[1])/(2.*[2]*[2]))",mean-2.*sigma,mean+2.*sigma);
  fitFunc_gaus_corr -> SetNpx(10000);
  fitFunc_gaus_corr -> SetParameters(norm,mean,sigma);
  fitFunc_gaus_corr -> SetLineColor(kBlack);
  histo -> Fit(gausName.c_str(),"QNRSL");
  fitFunc_gaus_corr -> Draw("same");
  
  norm = fitFunc_gaus_corr -> GetParameter(0);
  mean = fitFunc_gaus_corr -> GetParameter(1);
  sigma = fitFunc_gaus_corr -> GetParameter(2);
  float sigmaErr = fitFunc_gaus_corr -> GetParError(2);
  
  TLatex* latex_gaus;
  if( (isMCP0 && !isMCP1) || (!isMCP0 && isMCP1) )
    latex_gaus = new TLatex(0.16,0.75,Form("#sigma_{single}^{gaus} = (%.1f #pm %.1f) ps",
                                             sqrt(sigma*sigma - MCPIntrinsic*MCPIntrinsic)*1000.,
                                             fabs(sigmaErr*1000)));
  else
    latex_gaus = new TLatex(0.16,0.75,Form("#sigma_{single}^{gaus} = (%.1f #pm %.1f) ps",fabs(sigma*1000)/sqrt(2),fabs(sigmaErr*1000)/sqrt(2)));
  latex_gaus -> SetNDC();
  latex_gaus -> SetTextFont(42);
  latex_gaus -> SetTextSize(0.03);
  latex_gaus -> SetTextColor(kBlack);
  latex_gaus -> Draw("same");  
  
  
  if( histo_center)
  {
    vals = new float[4];
    FindSmallestInterval(vals,histo_center,0.68,true);
    histo_center -> Rebin(rebin);
    histo_center -> SetMarkerStyle(22);
    histo_center -> SetMarkerSize(0.7);
    histo_center -> SetMarkerColor(kRed);
    histo_center -> SetLineColor(kRed);
    histo_center -> Draw("PE,same");
    
    norm = histo -> GetMaximum();
    mean = vals[0];
    min = vals[2];
    max = vals[3];
    delta = max-min;
    sigma = 0.5*delta;
    effSigma = sigma;
    min = min - 2.*delta;
    max = max + 2.*delta;
    
    TLatex* latex_center;
  if( (isMCP0 && !isMCP1) || (!isMCP0 && isMCP1) )
    latex_center = new TLatex(0.65,0.30,Form("#splitline{central events:}{#sigma_{single}^{eff} = %.1f ps}",
                                             sqrt(effSigma*effSigma - MCPIntrinsic*MCPIntrinsic)*1000.));
  else
    latex_center = new TLatex(0.65,0.30,Form("#splitline{central events:}{#sigma_{single}^{eff} = %.1f ps}",fabs(effSigma*1000)/sqrt(2)));
  latex_center -> SetNDC();
  latex_center -> SetTextFont(42);
  latex_center -> SetTextSize(0.03);
  latex_center -> SetTextColor(kRed);
  latex_center -> Draw("same");  
  }
  
  
  if( histo_border)
  {
    vals = new float[4];
    FindSmallestInterval(vals,histo_border,0.68,true);
    histo_border -> Rebin(rebin);
    histo_border -> SetMarkerStyle(23);
    histo_border -> SetMarkerSize(0.7);
    histo_border -> SetMarkerColor(kBlue);
    histo_border -> SetLineColor(kBlue);
    histo_border -> Draw("PE,same");
    
    norm = histo -> GetMaximum();
    mean = vals[0];
    min = vals[2];
    max = vals[3];
    delta = max-min;
    sigma = 0.5*delta;
    effSigma = sigma;
    min = min - 2.*delta;
    max = max + 2.*delta;
    
    TLatex* latex_border;
  if( (isMCP0 && !isMCP1) || (!isMCP0 && isMCP1) )
    latex_border = new TLatex(0.65,0.20,Form("#splitline{border events:}{#sigma_{single}^{eff} = %.1f ps}",
                                             sqrt(effSigma*effSigma - MCPIntrinsic*MCPIntrinsic)*1000.));
  else
    latex_border = new TLatex(0.65,0.20,Form("#splitline{border events:}{#sigma_{single}^{eff} = %.1f ps}",fabs(effSigma*1000)/sqrt(2)));
  latex_border -> SetNDC();
  latex_border -> SetTextFont(42);
  latex_border -> SetTextSize(0.03);
  latex_border -> SetTextColor(kBlue);
  latex_border -> Draw("same");  
  }
  
  
  latexLabel -> Draw("same");
  
  
  gPad -> Update();
}
