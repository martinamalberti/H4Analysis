#ifndef __SIPM_FUNCTIONS__
#define __SIPM_FUNCTIONS__

#include <iostream>

#include "TF1.h"



/*** get SiPM PDE based on model ***/
TF1* GetPDE(const std::string& model);


/*** get SiPM response function ***/
double SinglePhotonSiPMResponse(double* xx, double* par);
double SiPMResponse(double* xx, double* par);

#endif
