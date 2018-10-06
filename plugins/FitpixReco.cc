#include "FitpixReco.h"

//**********Utils*************************************************************************
//----------Begin-------------------------------------------------------------------------
bool FitpixReco::Begin(CfgManager& opts, uint64* index)
{
    //---create a position tree
    bool storeTree = opts.OptExist(instanceName_+".storeTree") ?
        opts.GetOpt<bool>(instanceName_+".storeTree") : true;

    //---create a position tree
    string treeName = opts.OptExist(instanceName_+".treeName") ?
        opts.GetOpt<string>(instanceName_+".treeName") : "fitpix_tree";

    //---invert X&Y (depends on orientation, should be swapped when FITPIX is oriented along horizontal line)
    swapCoordinates_ = opts.OptExist(instanceName_+".swapCoordinates") ?
        opts.GetOpt<bool>(instanceName_+".swapCoordinates") : false;

    RegisterSharedData(new TTree(treeName.c_str(), treeName.c_str()), treeName.c_str(), storeTree);

    fitpixTree_ = new FitpixTree(index, (TTree*)data_.back().obj);
    fitpixTree_->Init();

    boardId_ = opts.GetOpt<int>(instanceName_+".boardId"); 

    return true;
}

//----------ProcessEvent------------------------------------------------------------------
bool FitpixReco::ProcessEvent(H4Tree& h4Tree, map<string, PluginBase*>& plugins, CfgManager& opts)
{

    hits_.clear();
    clusters_.clear();
    fitpixTree_->Clear();

    std::vector<FPHit*> clustered_hits;

    for(unsigned int i=0; i<h4Tree.nAdcChannels; ++i)
    {
        if(h4Tree.adcBoard[i] != boardId_)
            continue;

        int x = h4Tree.adcChannel[i] % FITPIX_PIXELS_X;
        int y = floor(h4Tree.adcChannel[i]/FITPIX_PIXELS_Y);
        FPHit hit=FPHit(x,y,h4Tree.adcData[i]);
        if (swapCoordinates_)
            hit.swapCoordinates();
        hits_.push_back(hit);
    }

    for (unsigned int i=0; i<hits_.size();++i)
        clustered_hits.push_back(&hits_[i]);

    while( clustered_hits.size()>0 )
    {
        std::vector<FPHit*>::iterator i_hit = clustered_hits.begin();
        FPCluster this_cluster;
        //---the seed
        this_cluster.add_hit( *(*i_hit) ); 
        clustered_hits.erase( i_hit );

        std::vector<FPHit*>::iterator j_hit = clustered_hits.begin();

        while( j_hit != clustered_hits.end() ) 
        {
            //---add all adjacent hits to the cluster
            if( this_cluster.isAdjacent(*(*j_hit)) )
            {
                //---erase takes care of moving the pointer fwd so no need for ++
                this_cluster.add_hit(*(*j_hit));
                clustered_hits.erase(j_hit); 
            } 
            else
                j_hit++;
        }
        clusters_.push_back(this_cluster);
    }

    fitpixTree_->n_hits=hits_.size();
    for (int i=0;i<fitpixTree_->n_hits;++i)
    {
        fitpixTree_->hitX.push_back(hits_[i].x_);
        fitpixTree_->hitY.push_back(hits_[i].y_);
        fitpixTree_->hitCharge.push_back(hits_[i].c_);
    }
    fitpixTree_->n_clusters=clusters_.size();
    for (int i=0;i<clusters_.size();++i)
    {
        fitpixTree_->clusterX.push_back( (clusters_[i].x() - ((double)FITPIX_PIXELS_X/2.)) * FITPIX_PIXELSIZE );
        fitpixTree_->clusterY.push_back( (clusters_[i].y() - ((double)FITPIX_PIXELS_Y/2.)) * FITPIX_PIXELSIZE );
        fitpixTree_->clusterCharge.push_back(clusters_[i].charge());
        fitpixTree_->clusterSize.push_back(clusters_[i].nhits());
    }

    //---fill output tree
    fitpixTree_->Fill();

    return true;
}