#include "FFTAnalyzer.h"

#include "TLinearFitter.h"
#include <algorithm>

//----------Constructor-------------------------------------------------------------------
FFTAnalyzer::FFTAnalyzer():
    n_tot_(0)
{}

//----------Utils-------------------------------------------------------------------------
bool FFTAnalyzer::Begin(map<string, PluginBase*>& plugins, CfgManager& opts, uint64* index)
{

    //---register shared FFTs
    //   nSamples is divided by two if FFT is from time to frequency domain
    srcInstance_ = opts.GetOpt<string>(instanceName_+".srcInstanceName");    
    channelsNames_ = opts.GetOpt<vector<string> >(instanceName_+".channelsNames");    
    fftType_ = opts.OptExist(instanceName_+".FFTType") ?
        opts.GetOpt<string>(instanceName_+".FFTType") : "T2F";
    bool storeFFT = opts.OptExist(instanceName_+".storeFFToutput") ?
        opts.GetOpt<bool>(instanceName_+".storeFFToutput") : false;
    for(auto& channel : channelsNames_)
    {
        if(!opts.OptExist(channel+".tUnit"))
        {
            cout << ">>> FFTAnalyzer ERROR: configuration for channel < " << channel << " > not found." << endl;
            return false;
        }
        if(fftType_ == "T2F")
        {
            FFTs_[channel] = new FFTClass();
            RegisterSharedData(FFTs_[channel], channel, storeFFT);
        }
        if(fftType_ == "F2T")
        {
            WFs_[channel] = new WFClass(1, opts.GetOpt<float>(channel+".tUnit"));
            RegisterSharedData(WFs_[channel], channel, storeFFT);
            if(opts.OptExist(instanceName_+".subtractFFTNoise"))
            {
                noiseTemplateFile_= TFile::Open(opts.GetOpt<string>(instanceName_+".noiseTemplateFile").c_str());
                TString noiseTemplateHistoName (opts.GetOpt<string>(instanceName_+".noiseTemplateHisto"));
                noiseTemplateHistoRe_ = (TH1F*) noiseTemplateFile_->Get(noiseTemplateHistoName+"_Re_tmpl");
                noiseTemplateHistoIm_ = (TH1F*) noiseTemplateFile_->Get(noiseTemplateHistoName+"_Im_tmpl");
                if (!noiseTemplateFile_)
                {
                    cout << ">>> FFTAnalyzer ERROR: noiseTemplateFile not open " << endl;
                    return false;
                }
            }
        }
    }
    
    //---create and register templates istograms
    //   histograms are created with automatic binning alog Y axis
    if(fftType_ == "T2F" && opts.OptExist(instanceName_+".makeTemplates"))
        templatesNames_ =  opts.GetOpt<vector<string> >(instanceName_+".makeTemplates");
    for(auto& channel : channelsNames_)
    {
        for(auto& tmpl : templatesNames_)
        {
            auto nSamples = opts.GetOpt<int>(channel+".nSamples");
            // templates2dHistos_[channel+tmpl] = new TH2F((channel+tmpl).c_str(),
            //                                             ("Template "+channel+" "+tmpl).c_str(),
            //                                             nSamples/2, 0, nSamples/2,
            //                                             10000, 0, 0);
            // templatesHistos_[channel+tmpl] = new TH1F((channel+"_"+tmpl+"_tmpl").c_str(),
            //                                           ("Template "+channel+" "+tmpl).c_str(),
            //                                           nSamples/2, 0, nSamples/2);
            // RegisterSharedData(templatesHistos_[channel+tmpl], channel+"_"+tmpl+"_tmpl", true);
            templatesProfiles_[channel+tmpl] = new TProfile((channel+"_"+tmpl+"_prof").c_str(),
                                                            ("Profile "+channel+" "+tmpl).c_str(),
                                                            nSamples/2+1, -0.5, nSamples/2+1-0.5);
            RegisterSharedData(templatesProfiles_[channel+tmpl], channel+"_"+tmpl+"_prof", true);
        }
    }
    
    //---register output data tree if requested (default true)
    bool storeTree = opts.OptExist(instanceName_+".storeTree") && fftType_ == "T2F" ?
        opts.GetOpt<bool>(instanceName_+".storeTree") : false;
    if(storeTree)
    {
        for(auto& channel : channelsNames_)
        {
            auto point = std::find(channelsNames_.begin(), channelsNames_.end(), channel);
            if(point != channelsNames_.end())
            {
                channelsMap_[channel] = point-channelsNames_.begin();
                n_tot_ += opts.GetOpt<int>(channel+".nSamples")/2+1;
            }
        }
            
        string fftTreeName = opts.OptExist(instanceName_+".fftTreeName") ?
            opts.GetOpt<string>(instanceName_+".fftTreeName") : "fft";
        RegisterSharedData(new TTree(fftTreeName.c_str(), "fft_tree"), "fft_tree", storeTree);
        //---create tree branches:
        //   array size is determined by DigitizerReco channels
        index_ = index;
        n_ch_ = channelsNames_.size();
        current_ch_ = new int[n_tot_];
        re_ = new float[n_tot_];
        im_ = new float[n_tot_];        
        amplitudes_ = new float[n_tot_];
        phases_ = new float[n_tot_];
        phase_adj_ = new float[n_tot_];
        dt_ = new float[n_ch_];        
        fftTree_ = (TTree*)data_.back().obj;
        fftTree_->Branch("index", index_, "index/l");
        fftTree_->Branch("n_tot", &n_tot_, "n_tot/i");
        fftTree_->Branch("n_ch", &n_ch_, "n_ch/i");
        fftTree_->Branch("ch", current_ch_, "ch[n_tot]/I");        
        fftTree_->Branch("re", re_, "re[n_tot]/F");
        fftTree_->Branch("im", im_, "im[n_tot]/F");
        fftTree_->Branch("ampl", amplitudes_, "ampl[n_tot]/F");
        fftTree_->Branch("phi", phases_, "phi[n_tot]/F");
        fftTree_->Branch("phi_adj", phase_adj_, "phi_adj[n_tot]/F");
        fftTree_->Branch("dt", dt_, "dt[n_ch]/F");
        //---set default values
        for(unsigned int i=0; i<n_tot_; ++i)
        {
            current_ch_[i]=-1;
            re_[i] = -10;
            im_[i] = -10;
            amplitudes_[i]=-1;
            phases_[i]=-1;
        }
    }
    else
        fftTree_ = NULL;
    
    return true;
}

bool FFTAnalyzer::ProcessEvent(H4Tree& event, map<string, PluginBase*>& plugins, CfgManager& opts)
{
    for(auto& channel : channelsNames_)
    {
        //---FFT from time to frequency domain /// T2F
        if(fftType_ == "T2F")
        {
            //---get WF from source instance data and reset FFT
            FFTs_[channel]->Reset();
            auto wf = (WFClass*)plugins[srcInstance_]->GetSharedData(srcInstance_+"_"+channel, "", false).at(0).obj;
            auto samples = wf->GetSamples();
            auto samples_norm = *samples;
            // int n_samples = opts.OptExist(channel+".signalWin") ?
            //   opts.GetOpt<int>(channel+".signalWin", 1) - opts.GetOpt<int>(channel+".signalWin", 0) :
            //   samples->size();
            int n_samples = samples->size();
            if(opts.OptExist(instanceName_+".normalizeInput") && opts.GetOpt<bool>(instanceName_+".normalizeInput"))
            {
                float max = *std::max_element(samples_norm.begin(), samples_norm.end());
                for(auto& sample : samples_norm)
                    sample /= max;
            }
            //---build the FFT
            double oRe[n_samples/2+1], oIm[n_samples/2+1];
            // int first_sample = opts.GetOpt<int>(channel+".signalWin", 0);
            // int last_sample = opts.GetOpt<int>(channel+".signalWin", 1);
            int first_sample = 0;
            int last_sample = n_samples;
            
            
            auto fftr2c = TVirtualFFT::FFT(1, &n_samples, "C2CF M");
            for(unsigned int i=first_sample; i<last_sample; ++i)
            {
              fftr2c->SetPoint(i, samples_norm[i]);
              // std::cout << "iSample: " << i-first_sample << "   val: " << samples_norm[i] << std::endl;
            }
            fftr2c->Transform();
            
            for(int i = 0; i < n_samples/2+1; ++i)
            {
              double re, im;
              fftr2c->GetPointComplex(i, re, im);
              
              oRe[i] = re;
              oIm[i] = im;
              // oRe[i] = re / n_samples;
              // oIm[i] = im / n_samples;
            }
            FFTs_[channel]->SetPointsComplex(n_samples/2+1, oRe, oIm);
            
            map<string, const double*> var_map;
            var_map["Re"] = oRe;
            var_map["Im"] = oIm;
            var_map["Ampl"] = FFTs_[channel]->GetAmplitudes()->data();
            var_map["Phase"] = FFTs_[channel]->GetPhases()->data();
            if(fftTree_ || templatesNames_.size() != 0)
            {
                float shift=0;
                vector<double> x(int(opts.GetOpt<float>(channel+".maxPhaseFitFreq")*wf->GetTUnit()*n_samples));
                generate(x.begin(), x.end(), [n=-1, wf, n_samples] () mutable { ++n; return n/(wf->GetTUnit()*n_samples); });   
                TLinearFitter phase_fit(1, "x[0]", "");
                for(int k=0; k<n_samples/2+1; ++k)
                {
                    for(auto& tmpl : templatesNames_)
                    {
                        // templates2dHistos_[channel+tmpl]->Fill(k, var_map[tmpl][k]);
                        templatesProfiles_[channel+tmpl]->Fill(k, var_map[tmpl][k]);
                    }
                    if(fftTree_)
                    {
                        int index =  channelsMap_[channel] * n_samples + k;
                        current_ch_[index] = channelsMap_[channel];
                        re_[index] = oRe[index];
                        im_[index] = oIm[index];
                        amplitudes_[index] = sqrt(oRe[index]*oRe[index]+oIm[index]*oIm[index]);
                        phases_[index] = atan2(oIm[index],oRe[index]);
                        if(k>0 && var_map["Phase"][k] > var_map["Phase"][k-1])
                            shift -= 2*TMath::Pi();
                        phase_adj_[index] = shift;
                        if(k < x.size())
                            phase_fit.AddPoint(&x[k], (phases_[index]+shift)/wf->GetTUnit());
                    }
                }
                if(fftTree_)
                {
                    phase_fit.Eval();
                    dt_[channelsMap_[channel]] = phase_fit.GetParameter(0);
                }
                
            }
            delete fftr2c;
        }
        //---FFT from frequency to time domain /// F2T
        else if(fftType_ == "F2T")
        {
            //---get FFT from source instance data and reset old WF
            WFs_[channel]->Reset();
            auto fft = (FFTClass*)plugins[srcInstance_]->GetSharedData(srcInstance_+"_"+channel, "", false).at(0).obj;

            //---build the FFT
            int n_samples = fft->GetAmplitudes()->size()*2-1;
            double data[n_samples];
            auto Re = fft->GetRe();
            auto Im = fft->GetIm();
            auto fftc2r = TVirtualFFT::FFT(1, &n_samples, "C2R");
            
            for(int i = 0; i < fft->GetAmplitudes()->size(); ++i)
              fftc2r -> SetPoint(i, *(Re->data()+i), *(Im->data()+i) );
            
            
            //---subtract FFT of noise from template before going back to time domain
            if(opts.OptExist(instanceName_+".subtractFFTNoise"))
            {
                double noiseRe=0,noiseIm=0,newRe=0, newIm=0;
                for(int i=0;i<n_samples/2;++i)
                {
                    noiseRe = noiseTemplateHistoRe_->GetBinContent(i);
                    newRe = *(Re->data()+i) - noiseRe; 
                    noiseIm = noiseTemplateHistoIm_->GetBinContent(i);
                    newIm = *(Im->data()+i) - noiseIm; 
                    fftc2r->SetPoint(i,newRe,newIm);
                }
            } 
            if(opts.OptExist(instanceName_+".frequencyCut"))
            {
                for(int i=0;i<n_samples/2;++i)
                {
                    if(opts.OptExist(instanceName_+".frequencyCut"))
                    {
                        if(i<opts.GetOpt<float>(instanceName_+".frequencyCut"))
                            fftc2r->SetPoint(i,*(Re->data()+i),*(Im->data()+i));
                        else
                            fftc2r->SetPoint(i,0,0);
                    }
                }
            }
            if(opts.OptExist(instanceName_+".bandFilter"))
            {
                for(int i = 0; i < fft->GetAmplitudes()->size(); ++i)
                {
                    if( i >= opts.GetOpt<float>(instanceName_+".bandFilter",0) &&
                        i <= opts.GetOpt<float>(instanceName_+".bandFilter",1) )
                      fftc2r->SetPoint(i,0,0);
                }
            }
            if(opts.OptExist(instanceName_+".bandPass"))
            {
                for(int i = 0; i < fft->GetAmplitudes()->size(); ++i)
                {
                    if( i < opts.GetOpt<float>(instanceName_+".bandPass",0) ||
                        i > opts.GetOpt<float>(instanceName_+".bandPass",1) )
                      fftc2r->SetPoint(i,0,0);
                }
            }
            if(opts.OptExist(instanceName_+".lowPass"))
            {
                for(int i = 0; i < fft->GetAmplitudes()->size(); ++i)
                {
                    if( i > opts.GetOpt<float>(instanceName_+".lowPass",0) )
                      fftc2r->SetPoint(i,0,0);
                }
            }
            if(opts.OptExist(instanceName_+".highPass"))
            {
                for(int i = 0; i < fft->GetAmplitudes()->size(); ++i)
                {
                    if( i < opts.GetOpt<float>(instanceName_+".highPass",0) )
                      fftc2r->SetPoint(i,0,0);
                }
            }
            
            fftc2r->Transform();
            fftc2r->GetPoints(data);
            
            
            //---fill new WF
            // std::cout << "\n\nn_samples: " << n_samples << std::endl;
            for(int iSample=0; iSample<n_samples; ++iSample)
            {
              WFs_[channel]->AddSample(data[iSample]/n_samples);
              // std::cout << "iSample: " << iSample << "   val: " << data[iSample]/n_samples << std::endl;
            }
            
            delete fftc2r;
        }
    }
    
    
    //---fill FFT tree
    if(fftTree_)
        fftTree_->Fill();
    
    return true;
}

bool FFTAnalyzer::EndLoop(int iLoop, map<string, PluginBase*>& plugins, CfgManager& opts)
{
    // for(auto& channel : channelsNames_)
    //     for(auto& tmpl : templatesNames_)
    //         GetIterativeProfile(templates2dHistos_[channel+tmpl], templatesHistos_[channel+tmpl]);

    return true;
}
