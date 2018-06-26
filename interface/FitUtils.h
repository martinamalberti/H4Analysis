#include <iostream>
#include <cmath>

#include "TH1.h"

/*** find effective sigma ***/
void EffectiveSigma( TH1F *h , double &xl, double &xh, double &sigmaeff, double &sigmaeff_error);


/*** find effective sigma (Andrea) ***/
void FindSmallestInterval(float* ret, TH1F* histo, const float& fraction, const bool& verbosity);
