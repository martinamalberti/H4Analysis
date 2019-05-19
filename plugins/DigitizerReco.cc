#include "DigitizerReco.h"

//----------Utils-------------------------------------------------------------------------
bool DigitizerReco::Begin(map<string, PluginBase*>& plugins, CfgManager& opts, uint64* index)
{
    //---inputs---
    channelsNames_ = opts.GetOpt<vector<string> >(instanceName_+".channelsNames");

    //---read dV and dT digitizer calibration from file and register it to the shared data
    if(opts.OptExist(instanceName_+".calibration"))
    {
        TFile* calibFile = TFile::Open(opts.GetOpt<string>(instanceName_+".calibration", 0).c_str(), "READ");
        if(calibFile)
        {
            auto calibPtr = calibFile->Get(opts.GetOpt<string>(instanceName_+".calibration", 1).c_str());
            if(calibPtr)
            {
                cout << ">>>DigiReco INFO: using calibration "
                     << opts.GetOpt<string>(instanceName_+".calibration", 1)
                     << " from " << opts.GetOpt<string>(instanceName_+".calibration", 0) << endl;
                
                digitizerCalib_ = *((DigitizerCalibration*)calibPtr->Clone("dVdTCalibration"));
                RegisterSharedData(&digitizerCalib_, "dVdTCalibration", false);
            }
            else
            {
                cout << ">>>DigiReco WARNING: calibration "
                     << opts.GetOpt<string>(instanceName_+".calibration", 1)
                     << " not found in " << opts.GetOpt<string>(instanceName_+".calibration", 0)
                     << ". No calibration will be applied" << endl;
            }
        }
        else
        {
            cout << ">>>DigiReco WARNING: impossible to open calibration file "
                 << opts.GetOpt<string>(instanceName_+".calibration", 0)
                 << ". No calibration will be applied" << endl;
        }
    }
    
    //---create channel container (WFClass)
    for(auto& channel : channelsNames_)
    {
        nSamples_[channel] = opts.GetOpt<int>(channel+".nSamples");
        auto tUnit = opts.GetOpt<float>(channel+".tUnit");
        if(opts.OptExist(channel+".type"))
        {
            if(opts.GetOpt<string>(channel+".type") == "NINO")
                WFs_[channel] = new WFClassNINO(opts.GetOpt<int>(channel+".polarity"), tUnit);
            else if(opts.GetOpt<string>(channel+".type") == "Clock")
                WFs_[channel] = new WFClassClock(tUnit);
        }
        else
            WFs_[channel] = new WFClass(opts.GetOpt<int>(channel+".polarity"), tUnit);
        
        //---set channel calibration if available
        unsigned int digiBd = opts.GetOpt<unsigned int>(channel+".digiBoard");
        unsigned int digiGr = opts.GetOpt<unsigned int>(channel+".digiGroup");
        unsigned int digiCh = opts.GetOpt<unsigned int>(channel+".digiChannel");
        auto ch_key = make_tuple(digiBd, digiGr, digiCh);
        if(digitizerCalib_.find(ch_key) != digitizerCalib_.end())
            WFs_[channel]->SetCalibration(&digitizerCalib_[ch_key]);

        //---register WF in the shared data
        RegisterSharedData(WFs_[channel], channel, false);
    }
    
    return true;
}

bool DigitizerReco::ProcessEvent(H4Tree& event, map<string, PluginBase*>& plugins, CfgManager& opts)
{        
    //---read the digitizer
    //---set time reference from digitized trigger
    int trigRef=0;
    // FIXME
    // for(int iSample=nSamples_[channel]*8; iSample<nSamples_[channel]*9; ++iSample)
    // {
    //     if(event.digiSampleValue[iSample] < 1000)
    //     {
    //         trigRef = iSample-nSamples_[channel]*8;
    //         break;
    //     }
    // }
    
    //---user channels
    bool evtStatus = true;
    for(auto& channel : channelsNames_)
    {
        //---reset and read new WFs_
        WFs_[channel]->Reset();
        unsigned int digiBd = opts.GetOpt<unsigned int>(channel+".digiBoard");
        unsigned int digiGr = opts.GetOpt<unsigned int>(channel+".digiGroup");
        unsigned int digiCh = opts.GetOpt<unsigned int>(channel+".digiChannel");
        int offset = event.digiMap.at(make_tuple(digiBd, digiGr, digiCh));
        
        std::vector<double> samples_old;
        for(int iSample=offset; iSample<offset+nSamples_[channel]; ++iSample)
        {
            //Set the start index cell
            if (iSample==offset)
                WFs_[channel]->SetStartIndexCell(event.digiStartIndexCell[iSample]);

            //---H4DAQ bug: sometimes ADC value is out of bound.
            //---skip everything if one channel is bad
            if(event.digiSampleValue[iSample] > 1e6)
            {
                evtStatus = false;
		WFs_[channel]->AddSample(4095);
                samples_old.push_back(4095);
            }
            else
            {
	        if( opts.GetOpt<int>(instanceName_+".useDigiSampleTime") == 0 )
                {
                    WFs_[channel]->AddSample(event.digiSampleValue[iSample]);
                    samples_old.push_back(event.digiSampleValue[iSample]);
                }
                else
                {
                    WFs_[channel]->AddSampleWithTime(event.digiSampleValue[iSample], event.digiSampleTime[iSample]);
                    samples_old.push_back(event.digiSampleValue[iSample]);
                }
	    }
	}
        if(opts.OptExist(channel+".useTrigRef") && opts.GetOpt<bool>(channel+".useTrigRef"))
            WFs_[channel]->SetTrigRef(trigRef);
        if(WFs_[channel]->GetCalibration())
            WFs_[channel]->ApplyCalibration();
        if(opts.OptExist(channel+".DLED") && opts.GetOpt<int>(channel+".DLED") != 0)
        {
          auto times = WFs_[channel]->GetTimes();
          auto tUnit = WFs_[channel]->GetTUnit();
          int delay = opts.GetOpt<int>(channel+".DLED");
          
          std::vector<double> samples_new;
          for(int jj = 0; jj < delay+1; ++jj)
            samples_new.push_back(0.);
          
          int currentSample = 0;
          for(unsigned int jSample = delay+1; jSample < samples_old.size(); ++jSample)
          {
            float time1 = jSample * tUnit;
            int sampleBef1 = -1;
            int sampleAft1 = -1;
            
            float time2 = (jSample-delay) * tUnit;
            int sampleBef2 = -1;
            int sampleAft2 = -1;
            
            for(unsigned int it = currentSample; it < samples_old.size(); ++it)
            {
              if( times->at(it) > time2 )
              {
                sampleBef2 = it-1;
                sampleAft2 = it;
                currentSample = it;
                break;
              }
            }
            for(unsigned int it = currentSample; it < samples_old.size(); ++it)
            {
              if( times->at(it) > time1 )
              {
                sampleBef1 = it-1;
                sampleAft1 = it;
                break;
              }
            }

            float val1 = samples_old[sampleBef1] + (samples_old[sampleAft1]-samples_old[sampleBef1])/((*times)[sampleAft1]-(*times)[sampleBef1]) * (time1-(*times)[sampleBef1]);
            float val2 = samples_old[sampleBef2] + (samples_old[sampleAft2]-samples_old[sampleBef2])/((*times)[sampleAft2]-(*times)[sampleBef2]) * (time2-(*times)[sampleBef2]);
            samples_new.push_back(val1-val2);
          }
          
          //---reset and create new WFs_          
          WFs_[channel]->Reset();
          
          for(int jj = 0; jj < samples_new.size(); ++jj)
            WFs_[channel]->AddSample(samples_new.at(jj));
        }
    }
    
    if(!evtStatus)
        cout << ">>>DigiReco WARNING: bad amplitude detected" << endl;

    return evtStatus;
}
