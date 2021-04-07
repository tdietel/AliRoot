/**************************************************************************
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

// $Id$
// $MpId: AliMpSlat.cxx,v 1.6 2006/05/24 13:58:50 ivana Exp $

#include "AliMpSlat.h"

#include "AliMpExMapIterator.h"
#include "AliLog.h"
#include "AliMpMotifPosition.h"
#include "AliMpPCB.h"

#include "Riostream.h"

#include "TArrayI.h"

#include <cassert>

//-----------------------------------------------------------------------------
/// Representation of a slat cathode (bending or non-bending).
///
/// A slat can be viewed as a "collection" of PCBs of various densities
/// (the density is defined by the size of the pads composing the PCB).
///
/// All the PCBs have a least the same height, if not the same width. In most
/// of the case, height=width=40 cm, at least for St345 (for trigger,
/// width varies)
// 
/// \author Laurent Aphecetche
//-----------------------------------------------------------------------------

using std::cout;
using std::endl;
/// \cond CLASSIMP
ClassImp(AliMpSlat)
/// \endcond

//_____________________________________________________________________________
AliMpSlat::AliMpSlat(TRootIOCtor* ioCtor) 
: TObject(), 
  fId(""), 
  fPlaneType(AliMp::kNonBendingPlane),
  fDX(0), 
  fDY(0),
  fNofPadsX(0), 
  fMaxNofPadsY(0),
  fManuMap(ioCtor),
  fPCBs(),
  fPositionX(0.),
  fPositionY(0.),
  fNofPads(0)
{
    ///
    /// Empty ctor.
    ///
  AliDebug(1,Form("this=%p Empty ctor",this));

    fPCBs.SetOwner(kTRUE);
    fManuMap.SetOwner(kFALSE);
}

//_____________________________________________________________________________
AliMpSlat::AliMpSlat(const char* id, AliMp::PlaneType bendingOrNonBending)
: TObject(), 
  fId(id), 
  fPlaneType(bendingOrNonBending),
  fDX(0), 
  fDY(0),
  fNofPadsX(0), 
  fMaxNofPadsY(0),
  fManuMap(),
  fPCBs(),
  fPositionX(0.),
  fPositionY(0.),
  fNofPads(0)
{
    ///
    /// Normal ctor
    ///
  AliDebug(1,Form("this=%p id=%s",this,id));			

    fPCBs.SetOwner(kTRUE);
    fManuMap.SetOwner(kFALSE);
}

//_____________________________________________________________________________
AliMpSlat::~AliMpSlat()
{
  ///
  /// Dtor.
  ///
  AliDebug(1,Form("this=%p fId=%s",this,fId.Data()));			

  fPCBs.Delete();
}

//_____________________________________________________________________________
void
AliMpSlat::Add(const AliMpPCB& pcbType, const TArrayI& manuList) 
{
  ///
  /// Adds a PCB to this slat. The manuList specifies the ids of the manu
  /// that compose the PCB. The manuList ordering is important, as the 
  /// assumption is that it's ordered counter-clockwise, starting from
  /// the lower-left of the PCB.
  ///
  Int_t ixOffset = 0;
  if ( GetSize() )
	{
		ixOffset = GetPCB(GetSize()-1)->Ixmax()+1;
	}
  else
  {
    ixOffset = pcbType.Ixmin();
  }
  Double_t xOffset = DX()*2;
  AliMpPCB* pcb = pcbType.Clone(manuList,ixOffset,xOffset);
  fPCBs.AddLast(pcb);
  fDY = TMath::Max(pcb->DY(),fDY);
  fDX += pcb->DX();
  fNofPadsX += pcb->GetNofPadsX();
  fMaxNofPadsY = TMath::Max(fMaxNofPadsY,pcb->GetNofPadsY());
  Int_t n(0);
  for ( Int_t i = 0; i < pcb->GetSize(); ++i )
	{
		AliMpMotifPosition* mp = pcb->GetMotifPosition(i);
		Int_t manuID = mp->GetID();
    // Before inserting a new key, check if it's already there
    TObject* there = fManuMap.GetValue(manuID);
    if ( there == 0 )
    {
      ++n;
      AliDebug(1,Form("Adding %d-th manuId=%d (%d) to ManuMap (size=%d)",n,manuID,mp->GetID(),fManuMap.GetSize()));
      fManuMap.Add(manuID,(TObject*)mp);
    }
    else
    {
      AliError(Form("ManuID %d is duplicated for PCB %s",manuID,pcbType.GetID()));      
    }
  }
  fPositionX = DX();
  fPositionY = DY();
  fNofPads += pcb->NofPads();
}

//_____________________________________________________________________________
Double_t
AliMpSlat::DX() const
{
  ///
  /// Returns the x-half-size of the slat.
  ///
  return fDX;
}

//_____________________________________________________________________________
Double_t
AliMpSlat::DY() const
{
  ///
  /// Returns the y-half-size of the slat.
  ///
  return fDY;
}

//_____________________________________________________________________________
AliMpMotifPosition*
AliMpSlat::FindMotifPosition(Int_t manuID) const
{
  ///
  /// Returns the motifPosition referenced by it manuID
  ///
  return static_cast<AliMpMotifPosition*>(fManuMap.GetValue(manuID));
}

//_____________________________________________________________________________
AliMpMotifPosition*
AliMpSlat::FindMotifPosition(Int_t ix, Int_t iy) const
{
  ///
  /// - 1. Find the PCB containing ix (iy not needed for this)
  /// - 2. Forward the request to the PCB, using pcb local indices.
	//
  const AliMpPCB* pcb = FindPCB(ix);
  if ( pcb )
	{
		return pcb->FindMotifPosition(ix,iy);
	}
  else
	{
		return 0;
	}
}

//_____________________________________________________________________________
AliMpMotifPosition*
AliMpSlat::FindMotifPosition(Double_t x, Double_t y) const
{
  ///
  /// Returns the motifPosition containing position (x,y)
  ///
  const AliMpPCB* pcb = FindPCB(x,y);
  if (pcb)
	{
		return pcb->FindMotifPosition(x,y);
	}
  else
	{
		return 0;
	}
}

//_____________________________________________________________________________
AliMpPCB*
AliMpSlat::FindPCB(Int_t ix) const
{
  ///
  /// Returns the PCB containing x-integer-position ix
  ///
  for ( Int_t i = 0; i < GetSize(); ++i ) 
	{
		AliMpPCB* pcb = GetPCB(i);
		if ( ix >= pcb->Ixmin() && ix <= pcb->Ixmax() )
		{
			return pcb;
		}
	}
  return 0;
}

//_____________________________________________________________________________
Int_t
AliMpSlat::FindPCBIndex(Int_t ix) const
{
  ///
  /// Returns the index of the PCB containing x-integer-position ix.
  ///
  for ( Int_t i = 0; i < GetSize(); ++i ) 
	{
		AliMpPCB* pcb = GetPCB(i);
		if ( ix >= pcb->Ixmin() && ix <= pcb->Ixmax() )
		{
			return i;
		}
	}
  return -1;
}

//_____________________________________________________________________________
AliMpPCB*
AliMpSlat::FindPCB(Double_t x, Double_t y) const
{
  ///
  /// Returns the PCB containing position (x,y)
  ///
  for ( Int_t i = 0; i < GetSize(); ++i ) 
	{
		AliMpPCB* pcb = GetPCB(i);
//		if ( x >= pcb->Xmin() && x < pcb->Xmax() &&
//				 y >= pcb->Ymin() && y < pcb->Ymax() )
//		{
//			return pcb;
//		}
    if ( x < pcb->Xmin() || x >= pcb->Xmax() ||
         y < pcb->Ymin() || y >= pcb->Ymax() )
    {
      continue;
    }
    return pcb;
	}
  return 0;
}

//_____________________________________________________________________________
Int_t
AliMpSlat::FindPCBIndex(Double_t x, Double_t y) const
{
  ///
  /// Returns the index of the PCB containing position (x,y)
  ///
  for ( Int_t i = 0; i < GetSize(); ++i ) 
	{
		AliMpPCB* pcb = GetPCB(i);
		if ( x >= pcb->Xmin() && x < pcb->Xmax() &&
				 y >= pcb->Ymin() && y < pcb->Ymax() )
		{
			return i;
		}
	}
  return -1;
}

//_____________________________________________________________________________
Int_t 
AliMpSlat::FindPCBIndexByMotifPositionID(Int_t manuId) const
{
  /// Find the index of the PCB containing a given manu
  for ( Int_t i = 0; i< GetSize(); ++i )
  {
    AliMpPCB* pcb = GetPCB(i);
    if ( pcb->HasMotifPositionID(manuId) ) return i;
  }
  return -1;
}

//_____________________________________________________________________________
void
AliMpSlat::ForcePosition(Double_t x, Double_t y)
{
  ///
  /// Force the position to be different from (DX(),DY()).
  /// Normally only used by triggerSlats (for layers).
  /// Beware that this method must be called once all PCB have been added,
  /// as the Add() method resets the position.
  ///
  fPositionX = x;
  fPositionY = y;
}

//_____________________________________________________________________________
void
AliMpSlat::GetAllMotifPositionsIDs(TArrayI& ecn) const
{
  ///
  /// Return all the manuIds (=MotifPositionIDs) of this slat
  ///
  Int_t nofElectronicCards(GetNofElectronicCards());
  assert(nofElectronicCards>0);
  ecn.Set(nofElectronicCards);
  TIter next(fManuMap.CreateIterator());
  AliMpMotifPosition* mp;
  Int_t n(0);
  while ( ( mp = static_cast<AliMpMotifPosition*>(next()) ) )
  {
    ecn.AddAt(mp->GetID(),n);
    ++n;
  }
  assert(n==nofElectronicCards);
}

//_____________________________________________________________________________
const char*
AliMpSlat::GetID() const
{
  ///
  /// Returns the name of this slat.
  ///
  return fId.Data();
}

//_____________________________________________________________________________
Int_t 
AliMpSlat::GetMaxNofPadsY() const
{
  ///
  /// Returns the maximum number of pads to be found in this slat y-direction.
  /// 
  return fMaxNofPadsY;
}

//_____________________________________________________________________________
Int_t 
AliMpSlat::GetMaxPadIndexX() const
{
  ///
  /// Returns the max ix that is valid for this slat.
  ///
  AliMpPCB* last = GetPCB(GetSize()-1);
  if (last)
  {
    return last->Ixmax();
  }
  return 0;
}

//_____________________________________________________________________________
const char*
AliMpSlat::GetName() const
{
  ///
  /// Returns the name of this slat, which is composed of its ID with
  /// the plane type as a suffix.
  ///
  static TString name(GetID());
  if ( fPlaneType == AliMp::kBendingPlane )
  {
    name += ".Bending";
  }
  else if ( fPlaneType == AliMp::kNonBendingPlane )
  {
    name += ".NonBending";
  }
  else
  {
    name += ".Invalid";
  }
  return name.Data();  
}

//_____________________________________________________________________________
Int_t
AliMpSlat::GetNofElectronicCards() const
{
  ///
  /// Returns the number of manus that compose the readout of this slat.
  ///
  return fManuMap.GetSize();
}

//_____________________________________________________________________________
Int_t
AliMpSlat::GetNofPadsX() const
{
  ///
  /// Returns the number of pad in x-direction.
  ///
  return fNofPadsX;
}

//_____________________________________________________________________________
AliMpPCB*
AliMpSlat::GetPCB(Int_t i) const
{
  ///
  /// Returns the i-th PCB of this slat.
  ///
  if ( i >= fPCBs.GetEntriesFast() ) return 0;
  return (AliMpPCB*)fPCBs[i];
}

//_____________________________________________________________________________
Int_t
AliMpSlat::GetSize() const
{
  ///
  /// Returns the number of PCB in this slat.
  ///
  return fPCBs.GetEntriesFast();
}

//_____________________________________________________________________________
void
AliMpSlat::Print(Option_t* option) const
{
  ///
  /// Prints the slat characteristics.
  ///
  cout << "SLAT " << GetID() <<  " 1/2 DIM = (" << DX() << "," << DY() << ")"
  << " POS = " << GetPositionX() << "," << GetPositionY()
	<< " NPADSX = " << GetNofPadsX() 
  << " MAXNPADSY = " << GetMaxNofPadsY()
  << " NPCBs=" << GetSize() << endl;
  
  TString soption(option);
  
  if ( soption.Contains("P") )
	{
    for ( Int_t i = 0; i < GetSize() ; ++i )
		{
      cout << "    ";
			if ( option )
	    {
				fPCBs[i]->Print(option+1);
	    }
			else
	    {
	      fPCBs[i]->Print();
	    }
		}
	}
  
  if ( soption.Contains("M") || soption.Contains("L") )
  {
    cout << fManuMap.GetSize() << " ";
    cout << "Electronic card (manu or local board) Ids : ";
    TIter next(fManuMap.CreateIterator());
    AliMpMotifPosition* mp;
    while ( ( mp = static_cast<AliMpMotifPosition*>(next())) )
    {
      cout << mp->GetID() << " ";
    }
    cout << endl;
  }
}
