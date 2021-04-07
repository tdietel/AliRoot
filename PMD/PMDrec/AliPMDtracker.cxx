/***************************************************************************
 * Copyright(c) 1998-1999, ALICE Experiment at CERN, All rights reserved. *
 *                                                                        *
 * Author: The ALICE Off-line Project.                                    *
 * Contributors are mentioned in the code where appropriate.              *
 *                                                                        *
 * Permission to use, copy, modify and distribute this software and its   *
 * documentation strictly for non-commercial purposes is hereby granted   *
 * without fee, provided that the above copyright notice appears in all   *
 * copies and that both the copyright notice and this permission notice   *
 * appear in the supporting documentation. The authors make no claims     *
 * about the suitability of this software for any purpose. It is          *
 * provided "as is" without express or implied warranty.                  *
 **************************************************************************/

//-----------------------------------------------------//
//                                                     //
//           Date   : March 25 2004                    //
//  This reads the file PMD.RecPoints.root(TreeR),     //
//  calls the Clustering algorithm and stores the      //
//  clustering output in PMD.RecPoints.root(TreeR)     // 
//                                                     //
//-----------------------------------------------------//

#include <Riostream.h>
#include <TMath.h>
#include <TTree.h>
#include <TObjArray.h>
#include <TClonesArray.h>
#include <TFile.h>
#include <TBranch.h>
#include <TNtuple.h>
#include <TParticle.h>

#include <TGeoMatrix.h>

#include "AliGeomManager.h"

#include "AliPMDcluster.h"
#include "AliPMDclupid.h"
#include "AliPMDrecpoint1.h"
#include "AliPMDrecdata.h"
#include "AliPMDrechit.h"
#include "AliPMDUtility.h"
#include "AliPMDDiscriminator.h"
#include "AliPMDEmpDiscriminator.h"
#include "AliPMDtracker.h"

#include "AliESDPmdTrack.h"
#include "AliESDEvent.h"
#include "AliLog.h"

ClassImp(AliPMDtracker)

AliPMDtracker::AliPMDtracker():
  fTreeR(0),
  fRecpoints(new TClonesArray("AliPMDrecpoint1", 10)),
  fRechits(new TClonesArray("AliPMDrechit", 10)),
  fPMDcontin(new TObjArray()),
  fPMDcontout(new TObjArray()),
  fPMDutil(new AliPMDUtility()),
  fPMDrecpoint(0),
  fPMDclin(0),
  fPMDclout(0),
  fXvertex(0.),
  fYvertex(0.),
  fZvertex(0.),
  fSigmaX(0.),
  fSigmaY(0.),
  fSigmaZ(0.)
{
  //
  // Default Constructor
  //
}
//--------------------------------------------------------------------//
AliPMDtracker:: AliPMDtracker(const AliPMDtracker & /* tracker */):
  TObject(/* tracker */),
  fTreeR(0),
  fRecpoints(NULL),
  fRechits(NULL),
  fPMDcontin(NULL),
  fPMDcontout(NULL),
  fPMDutil(NULL),
  fPMDrecpoint(0),
  fPMDclin(0),
  fPMDclout(0),
  fXvertex(0.),
  fYvertex(0.),
  fZvertex(0.),
  fSigmaX(0.),
  fSigmaY(0.),
  fSigmaZ(0.)
{
  // copy constructor
  AliError("Copy constructor not allowed");
}

//--------------------------------------------------------------------//
AliPMDtracker& AliPMDtracker::operator=(const AliPMDtracker & /* tracker */)
{
 // assignment operator
  AliError("Assignment operator not allowed");
  return *this;
}

//--------------------------------------------------------------------//
AliPMDtracker::~AliPMDtracker()
{
  // Destructor
  if (fRecpoints)
    {
      fRecpoints->Clear();
    }
  if (fRechits)
    {
      fRechits->Clear();
    }

  if (fPMDcontin)
    {
      fPMDcontin->Delete();
      delete fPMDcontin;
      fPMDcontin=0;
      
    }
  if (fPMDcontout)
  {
      fPMDcontout->Delete();
      delete fPMDcontout;
      fPMDcontout=0;

    }
  delete fPMDutil;
}
//--------------------------------------------------------------------//
void AliPMDtracker::LoadClusters(TTree *treein)
{
  // Load the Reconstructed tree
  fTreeR = treein;
}
//--------------------------------------------------------------------//
void AliPMDtracker::Clusters2Tracks(AliESDEvent *event, Int_t gRecoMode)
{
  // Converts digits to recpoints after running clustering
  // algorithm on CPV plane and PREshower plane
  //
  
  /*
    Modification by S. K. Prasad on 05-07-2019
    This   part is   added to pass on the cell 
    level  information   to   ESD  without any 
    change in its raw form for PbPb collisions.
  */
  if(gRecoMode == 2){
    Int_t   idetAA        = 0;
    Int_t   ismnAA        = 0;
    Int_t   tracknoAA     = 1;
    Int_t   trackpidAA    = 0;
    Float_t clusdataAA[6] = {0.,0.,0.,0.,0.,0.};
    
    AliPMDrechit *rechitAA = 0x0;
    
    TBranch *branchRecPoint = fTreeR->GetBranch("PMDRecpoint");
    if (!branchRecPoint)
      {
	AliError("PMDRecpoint branch not found");
	return;
      }
    branchRecPoint->SetAddress(&fRecpoints);  
    
    TBranch *branchRecHit = fTreeR->GetBranch("PMDRechit");
    if (!branchRecHit)
      {
	AliError("PMDRechit branch not found");
	return;
      }
    branchRecHit->SetAddress(&fRechits);  
    
    Int_t ncrhitAA     = 0;
    Int_t   nmodulesAA = (Int_t) branchRecPoint->GetEntries();
    
    AliDebug(1,Form("Number of modules filled in treeR = %d",nmodulesAA));
    const Float_t kzposAA = 361.5;    // middle of the PMD
    Float_t zglobalAA;
    for (Int_t imodule = 0; imodule < nmodulesAA; imodule++)
      {
	branchRecPoint->GetEntry(imodule); 
	Int_t nclusters = fRecpoints->GetLast();
	AliDebug(2,Form("Number of clusters per modules filled in treeR = %d"
			,nclusters));
	//cluster loop
	for(Int_t iclust = 0; iclust < nclusters+1; iclust++)
	  {
	    Int_t irowAA[2]  = {-1,-1};
	    Int_t icolAA[2]  = {-1,-1};
	    Int_t itraAA[2]  = {-1,-1};
	    Int_t ipidAA[2]  = {-1,-1};
	    Float_t cadcAA[2] = {-1.,-1.};;
	    fPMDrecpoint = (AliPMDrecpoint1*)fRecpoints->UncheckedAt(iclust);
	    idetAA        = fPMDrecpoint->GetDetector();
	    ismnAA        = fPMDrecpoint->GetSMNumber();
	    clusdataAA[0] = fPMDrecpoint->GetClusX();
	    clusdataAA[1] = fPMDrecpoint->GetClusY();
	    clusdataAA[2] = fPMDrecpoint->GetClusADC();
	    clusdataAA[3] = fPMDrecpoint->GetClusCells();
	    clusdataAA[4] = fPMDrecpoint->GetClusSigmaX();
	    clusdataAA[5] = fPMDrecpoint->GetClusSigmaY();
	    if (idetAA == 0)
	      {
		zglobalAA = kzposAA + 1.65; // PREshower plane
	      }
	    else if (idetAA == 1)
	      {
		zglobalAA = kzposAA - 1.65; // CPV plane
	      }	    
	    
	    //passing cell information to ESD
	    AliESDPmdTrack *esdpmdtrAA = new  AliESDPmdTrack();
	    esdpmdtrAA -> SetDetector(idetAA);//det 0:PRE 1:CPV
	    esdpmdtrAA -> SetSmn(ismnAA);//smn, varies from 0 to 23 for both PRE and CPV
	    esdpmdtrAA -> SetClusterX(clusdataAA[0]);//cell Row, varies from 0 to 47
	    esdpmdtrAA -> SetClusterY(clusdataAA[1]);//cell Coloumn, varies from 0 to 95
	    esdpmdtrAA -> SetClusterADC(clusdataAA[2]);//cell ADC
	    esdpmdtrAA -> SetClusterCells(clusdataAA[3]);//Number of cells = 1 (in this case)
	    esdpmdtrAA -> SetClusterSigmaX(clusdataAA[4]);//Track number for simulation, -1 for data
	    esdpmdtrAA -> SetClusterSigmaY(clusdataAA[5]);//Track PID for simulation, -1 for data
	    esdpmdtrAA -> SetClusterPID(clusdataAA[5]);//Track PID for simulation, -1 for data	    
	    esdpmdtrAA -> SetClusterTrackNo(clusdataAA[4]);//Track number for simulation, -1 for data
	    esdpmdtrAA -> SetClusterTrackPid(clusdataAA[5]);//Track number for simulation, -1 for data
	    esdpmdtrAA -> SetClusterMatching(-1);//set to -1 as there is no matching 
	    esdpmdtrAA -> SetClusterZ(zglobalAA);//cell z position
	    event->AddPmdTrack(esdpmdtrAA);
	    delete esdpmdtrAA;

	    branchRecHit->GetEntry(ncrhitAA); 
	    Int_t ncellsAA = fRechits->GetLast() + 1;
	    for(int icellAA=0; icellAA<ncellsAA; icellAA++){
	      rechitAA       = (AliPMDrechit*)fRechits->UncheckedAt(icellAA);
	      irowAA[icellAA] = rechitAA->GetCellX();
	      icolAA[icellAA] = rechitAA->GetCellY();
	      itraAA[icellAA] = rechitAA->GetCellTrack();
	      ipidAA[icellAA] = rechitAA->GetCellPid();
	      cadcAA[icellAA] = rechitAA->GetCellAdc();
	    }//for cell loop
	    ncrhitAA++;
	  }//cluster loop
      }//module loop
  }//if(gRecoMode == 2)
  
  else {
    ///*****************
    Int_t   idet = 0;
    Int_t   ismn = 0;
    Int_t   trackno = 1, trackpid = 0;
    Float_t clusdata[6] = {0.,0.,0.,0.,0.,0.};
    
    Int_t *irow;
    Int_t *icol;
    Int_t *itra;
    Int_t *ipid;
    Float_t *cadc;
    
    AliPMDrechit *rechit = 0x0;
    
    TBranch *branch = fTreeR->GetBranch("PMDRecpoint");
    if (!branch)
      {
	AliError("PMDRecpoint branch not found");
	return;
      }
    branch->SetAddress(&fRecpoints);  
    
    TBranch *branch1 = fTreeR->GetBranch("PMDRechit");
    if (!branch1)
      {
	AliError("PMDRechit branch not found");
	return;
      }
    branch1->SetAddress(&fRechits);  
    
    Int_t ncrhit = 0;
    Int_t   nmodules = (Int_t) branch->GetEntries();
    
    AliDebug(1,Form("Number of modules filled in treeR = %d",nmodules));
    for (Int_t imodule = 0; imodule < nmodules; imodule++)
      {
	branch->GetEntry(imodule); 
	Int_t nentries = fRecpoints->GetLast();
	AliDebug(2,Form("Number of clusters per modules filled in treeR = %d"
			,nentries));
	
	for(Int_t ient = 0; ient < nentries+1; ient++)
	  {
	    fPMDrecpoint = (AliPMDrecpoint1*)fRecpoints->UncheckedAt(ient);
	    idet        = fPMDrecpoint->GetDetector();
	    ismn        = fPMDrecpoint->GetSMNumber();
	    clusdata[0] = fPMDrecpoint->GetClusX();
	    clusdata[1] = fPMDrecpoint->GetClusY();
	    clusdata[2] = fPMDrecpoint->GetClusADC();
	    clusdata[3] = fPMDrecpoint->GetClusCells();
	    clusdata[4] = fPMDrecpoint->GetClusSigmaX();
	    clusdata[5] = fPMDrecpoint->GetClusSigmaY();
	    
	    if (clusdata[4] >= 0. && clusdata[5] >= 0.)
	      { 
		// extract the associated cell information
		branch1->GetEntry(ncrhit); 
		Int_t nenbr1 = fRechits->GetLast() + 1;
		
		irow = new Int_t[nenbr1];
		icol = new Int_t[nenbr1];
		itra = new Int_t[nenbr1];
		ipid = new Int_t[nenbr1];
		cadc = new Float_t[nenbr1];
		
		for (Int_t ient1 = 0; ient1 < nenbr1; ient1++)
		  {
		    irow[ient1] = -99;
		    icol[ient1] = -99;
		    itra[ient1] = -99;
		    ipid[ient1] = -99;
		    cadc[ient1] = 0.;
		  }
		for (Int_t ient1 = 0; ient1 < nenbr1; ient1++)
		  {
		    rechit = (AliPMDrechit*)fRechits->UncheckedAt(ient1);
		    //irow[ient1] = rechit->GetCellX();
		    //icol[ient1] = rechit->GetCellY();
		    itra[ient1] = rechit->GetCellTrack();
		    ipid[ient1] = rechit->GetCellPid();
		    cadc[ient1] = rechit->GetCellAdc();
		  }
		if (idet == 0)
		  {
		    AssignTrPidToCluster(nenbr1, itra, ipid, cadc,
					 trackno, trackpid);
		  }
		else if (idet == 1)
		  {
		    trackno  = itra[0];
		    trackpid = ipid[0];
		  }
		
		delete [] irow;
		delete [] icol;
		delete [] itra;
		delete [] ipid;
		delete [] cadc;
		
		fPMDclin = new AliPMDrecdata(idet,ismn,trackno,trackpid,clusdata);
		fPMDcontin->Add(fPMDclin);
		
		ncrhit++;
	      }//if
	  }//cluster loop
      }//module loop
    
    AliPMDEmpDiscriminator pmddiscriminator;
    pmddiscriminator.Discrimination(fPMDcontin,fPMDcontout);
    
    // alignment implemention
    
    Double_t sectr[4][3] = { {0.,0.,0.},{0.,0.,0.},{0.,0.,0.},{0.,0.,0.}};
    TString snsector="PMD/Sector";
    TString symname;
    TGeoHMatrix gpmdor;
    
    for(Int_t isector=1; isector<=4; isector++)
      {
	symname = snsector;
	symname += isector;
	TGeoHMatrix *gpmdal = AliGeomManager::GetMatrix(symname);
	Double_t *tral = gpmdal->GetTranslation();
	
	AliGeomManager::GetOrigGlobalMatrix(symname, gpmdor);
	Double_t *tror = gpmdor.GetTranslation();
	
	for(Int_t ixyz=0; ixyz<3; ixyz++)
	  {
	    sectr[isector-1][ixyz] = tral[ixyz] - tror[ixyz];
	  }
      }
    
    const Float_t kzpos = 361.5;    // middle of the PMD
    
    Int_t   ix = -1, iy = -1;
    Int_t   det = 0, smn = 0, trno = 1, trpid = 0, mstat = 0;
    Float_t xpos = 0., ypos = 0.;
    Float_t adc = 0., ncell = 0., radx = 0., rady = 0.;
    Float_t xglobal = 0., yglobal = 0., zglobal = 0;
    Float_t pid = 0.;
    
    fPMDutil->ApplyAlignment(sectr);
    
    Int_t nentries2 = fPMDcontout->GetEntries();
    AliDebug(1,Form("Number of clusters coming after discrimination = %d"
		    ,nentries2));
    for (Int_t ient1 = 0; ient1 < nentries2; ient1++)
      {
	fPMDclout = (AliPMDclupid*)fPMDcontout->UncheckedAt(ient1);
	
	det   = fPMDclout->GetDetector();
	smn   = fPMDclout->GetSMN();
	trno  = fPMDclout->GetClusTrackNo();
	trpid = fPMDclout->GetClusTrackPid();
	mstat = fPMDclout->GetClusMatching();
	xpos  = fPMDclout->GetClusX();
	ypos  = fPMDclout->GetClusY();
	adc   = fPMDclout->GetClusADC();
	ncell = fPMDclout->GetClusCells();
	radx  = fPMDclout->GetClusSigmaX();
	// Here in the variable "rady" we are keeping the row and col
	// of the single isolated cluster having ncell = 1 for offline
	// calibration
	
	if ((radx > 999. && radx < 1000.) && ncell == 1)
	  {
	    if (smn < 12)
	      {
		ix = (Int_t) (ypos +0.5);
		iy = (Int_t) xpos;
	      }
	    else if (smn > 12 && smn < 24)
	      {
		ix = (Int_t) xpos;
		iy = (Int_t) (ypos +0.5);
	      }
	    rady = (Float_t) (ix*100 + iy);
	  }
	else
	  {
	    rady  = fPMDclout->GetClusSigmaY();
	  }
	pid   = fPMDclout->GetClusPID();
	
	//
	/**********************************************************************
	 *    det   : Detector, 0: PRE & 1:CPV                                *
	 *    smn   : Serial Module Number 0 to 23 for each plane             *
	 *    xpos  : x-position of the cluster                               *
	 *    ypos  : y-position of the cluster                               *
	 *            THESE xpos & ypos are not the true xpos and ypos        *
	 *            for some of the unit modules. They are rotated.         *
	 *    adc   : ADC contained in the cluster                            *
	 *    ncell : Number of cells contained in the cluster                *
	 *    rad   : radius of the cluster (1d fit)                          *
	 **********************************************************************/
	//
	
	
	if (det == 0)
	  {
	    zglobal = kzpos + 1.65; // PREshower plane
	  }
	else if (det == 1)
	  {
	    zglobal = kzpos - 1.65; // CPV plane
	  }
	
	fPMDutil->RectGeomCellPos(smn,xpos,ypos,xglobal,yglobal,zglobal);
	
	// Fill ESD
	
	AliESDPmdTrack *esdpmdtr = new  AliESDPmdTrack();
	
	esdpmdtr->SetDetector(det);
	esdpmdtr->SetSmn(smn);
	esdpmdtr->SetClusterTrackNo(trno);
	esdpmdtr->SetClusterTrackPid(trpid);
	esdpmdtr->SetClusterMatching(mstat);
	
	esdpmdtr->SetClusterX(xglobal);
	esdpmdtr->SetClusterY(yglobal);
	esdpmdtr->SetClusterZ(zglobal);
	esdpmdtr->SetClusterADC(adc);
	esdpmdtr->SetClusterCells(ncell);
	esdpmdtr->SetClusterPID(pid);
	esdpmdtr->SetClusterSigmaX(radx);
	esdpmdtr->SetClusterSigmaY(rady);
	
	event->AddPmdTrack(esdpmdtr);
	delete esdpmdtr;
      }
    fPMDcontin->Delete();
    fPMDcontout->Delete();
  }//else
}
//--------------------------------------------------------------------//
void AliPMDtracker::AssignTrPidToCluster(Int_t nentry, Int_t *itra,
					 Int_t *ipid, Float_t *cadc,
					 Int_t &trackno, Int_t &trackpid)
{
  // assign the track number and the corresponding pid to a cluster
  // split cluster part will be done at the time of calculating eff/pur

  Int_t *phentry = new Int_t [nentry];
  Int_t *hadentry = new Int_t [nentry];
  Int_t *trpid    = 0x0;
  Int_t *sortcoord = 0x0;
  Float_t *trenergy = 0x0;

  Int_t ngtrack = 0;
  Int_t nhtrack = 0;
  for (Int_t i = 0; i < nentry; i++)
    {
      phentry[i] = -1;
      hadentry[i] = -1;

      if (ipid[i] == 22)
	{
	  phentry[ngtrack] = i;
	  ngtrack++;
	}
      else if (ipid[i] != 22)
	{
	  hadentry[nhtrack] = i;
	  nhtrack++;
	}
    }
  
  Int_t nghadtrack = ngtrack + nhtrack;

  if (ngtrack == 0)
    {
      // hadron track
      // no need of track number, set to -1
      trackpid = 8;
      trackno  = -1;
    }
  else if (ngtrack >= 1)
    {
      // one or more than one photon track + charged track
      // find out which track deposits maximum energy and
      // assign that track number and track pid

      trenergy  = new Float_t [nghadtrack];
      trpid     = new Int_t [nghadtrack];
      sortcoord = new Int_t [2*nghadtrack];
      for (Int_t i = 0; i < ngtrack; i++)
	{
	  trenergy[i] = 0.;
	  trpid[i]    = -1;
	  for (Int_t j = 0; j < nentry; j++)
	    {
	      if (ipid[j] == 22 && itra[j] == itra[phentry[i]])
		{
		  trenergy[i] += cadc[j];
		  trpid[i]     = 22;
		}
	    }
	}
      for (Int_t i = ngtrack; i < nghadtrack; i++)
	{
	  trenergy[i] = 0.;
	  trpid[i]    = -1;
	  for (Int_t j = 0; j < nentry; j++)
	    {
	      if (ipid[j] != 22 && itra[j] == itra[hadentry[i-ngtrack]])
		{
		  trenergy[i] += cadc[j];
		  trpid[i]     = ipid[j];
		}
	    }
	}
      
      Bool_t jsort = true;
      TMath::Sort(nghadtrack,trenergy,sortcoord,jsort);
      
      Int_t gtr = sortcoord[0];   
      if (trpid[gtr] == 22)
	{
	  trackpid = 22;
	  trackno  = itra[phentry[gtr]];   // highest adc track
	}
      else
	{
	  trackpid = 8;
	  trackno = -1;
	}
      
      delete [] trenergy;
      delete [] trpid;
      delete [] sortcoord;
      
    }   // end of ngtrack >= 1

  delete [] phentry;
  delete [] hadentry;
  
}
//--------------------------------------------------------------------//
void AliPMDtracker::SetVertex(Double_t vtx[3], Double_t evtx[3])
{
  fXvertex = vtx[0];
  fYvertex = vtx[1];
  fZvertex = vtx[2];
  fSigmaX  = evtx[0];
  fSigmaY  = evtx[1];
  fSigmaZ  = evtx[2];
}
//--------------------------------------------------------------------//
void AliPMDtracker::ResetClusters()
{
  if (fRecpoints) fRecpoints->Clear();
}
//--------------------------------------------------------------------//
