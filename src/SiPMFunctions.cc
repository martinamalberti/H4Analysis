#include "interface/SiPMFunctions.h"



/*** double crystalBall ***/
TF1* GetPDE(const std::string& model)
{
  TF1* func = new TF1("func","[0]*((1-exp(-1.*(x-[4])/[1]))*exp(-1.*(x-[4])/[2]))",0.,1000.);
  func -> SetNpx(10000);
  
  if( model == "Ketek_25um_3x3mm2" )
  {
    func -> SetParameters(3382.33,211166,85.3503,331.968);
  }
  
  return func;
}



double SinglePhotonSiPMResponse(double* x, double* par)
{
  double xx = x[0];
  
  double t_0 = par[0];
  double A   = par[1];
  double t_r = par[2];
  double t_d = par[3];
  double b = t_d / t_r;
  
  if( (xx-t_0) <= 0. ) return 0.;
  return A / ( pow(b,1./(1.-b)) - pow(b,1./(1./b-1.)) ) * ( exp(-1.*(xx-t_0)/t_d) - exp(-1.*(xx-t_0)/t_r) );
}



double SiPMResponse(double* x, double* par)
{
  // par[0] --> t_0
  // par[1] --> A
  // par[2] --> t_r
  // par[3] -->  t_d
  // par[4] --> total number of photons
  // par[5] --> t_0 of 1st photon
  // par[6] --> A of 1st photon
  // par[7] --> t_0 of 2nd photon
  // par[8] --> A of 2nd photon
  // ... etc
  
  double N = par[4]; // total number of photons
  
  double val = 0.;
  for(int photonIt = 0; photonIt < N; ++photonIt)
  {
    par[0] = par[5+photonIt*2];
    par[1] = par[6+photonIt*2];
    val += SinglePhotonSiPMResponse(x,par);
  }
  
  return val;
}
