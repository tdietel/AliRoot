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
// $MpId: AliMpSlatSegmentation.cxx,v 1.12 2006/05/24 13:58:50 ivana Exp $

//-----------------------------------------------------------------------------
// Caution !!
// Implementation note.
// The position(s) used in the interface are supposed to be relative
// to the slat center (AliMpSlat::Position()), whereas internally
// the x,y are relative to bottom-left corner.
//-----------------------------------------------------------------------------

#include "AliMpSlatSegmentation.h"

#include "AliLog.h"
#include "AliMpArea.h"
#include "AliMpConnection.h"
#include "AliMpConstants.h"
#include "AliLog.h"
#include "AliMpMotif.h"
#include "AliMpMotifPosition.h"
#include "AliMpMotifType.h"
#include "AliMpSlat.h"
#include "AliMpSlatPadIterator.h"
#include "AliMpEncodePair.h"

/// \cond CLASSIMP
ClassImp(AliMpSlatSegmentation)
/// \endcond

//_____________________________________________________________________________
AliMpSlatSegmentation::AliMpSlatSegmentation() 
: AliMpVSegmentation(),
  fkSlat(0),
  fIsOwner(false)
{
  ///
  /// Default ctor. Not to be used really.
  ///
  AliDebug(1,Form("this=%p Empty ctor",this));
}

//_____________________________________________________________________________
AliMpSlatSegmentation::AliMpSlatSegmentation(const AliMpSlat* slat, Bool_t own) 
: AliMpVSegmentation(), 
  fkSlat(slat),
  fIsOwner(own)
{
  ///
  /// Normal ctor.
  ///
  AliDebug(1,Form("this=%p Normal ctor slat=%p",this,slat));
}

//_____________________________________________________________________________
AliMpSlatSegmentation::~AliMpSlatSegmentation()
{
  ///
  /// Dtor (empty).
  ///
 
  if ( fIsOwner ) delete fkSlat;
 
  // Int_t i(0);//just to be able to put a breakpoint in gdb
  AliDebug(1,Form("this=%p",this));			
}

//_____________________________________________________________________________
AliMpVPadIterator*
AliMpSlatSegmentation::CreateIterator(const AliMpArea& area) const
{
  ///
  /// Returns an iterator to loop over the pad contained within given area.
  ///
  AliMpArea a(area.GetPositionX()+fkSlat->GetPositionX(),
              area.GetPositionY()+fkSlat->GetPositionY(),
              area.GetDimensionX(), 
              area.GetDimensionY());
  AliDebug(3,Form("Converted input area wrt to slat center : "
                  "%7.2f,%7.2f->%7.2f,%7.2f to wrt slat lower-left : "
                  "%7.2f,%7.2f->%7.2f,%7.2f ",
                  area.LeftBorder(),area.DownBorder(),
                  area.RightBorder(),area.UpBorder(),
                  a.LeftBorder(),a.DownBorder(),
                  a.RightBorder(),a.UpBorder()));
                  
  return new AliMpSlatPadIterator(fkSlat,a);
}

//_____________________________________________________________________________
AliMpVPadIterator*
AliMpSlatSegmentation::CreateIterator() const
{
  /// Returns an iterator to loop over all pads of that segmentation
  ///
  /// FIXME: we currently just forward this to the other CreateIterator,
  /// with the proper region. Might be more efficient to write a dedicated
  /// iterator ? Test that idea.
  
  AliMpArea area(0.0,0.0,fkSlat->DX(),fkSlat->DY());
  return CreateIterator(area);
}

//_____________________________________________________________________________
Int_t 
AliMpSlatSegmentation::GetNeighbours(const AliMpPad& pad, 
                                     TObjArray& neighbours,
                                     Bool_t includeSelf,
                                     Bool_t includeVoid) const
{
  /// Uses default implementation
  return AliMpVSegmentation::GetNeighbours(pad,neighbours,includeSelf,includeVoid);
}

//_____________________________________________________________________________
Double_t  
AliMpSlatSegmentation::GetDimensionX() const
{
/// Return slat x dimensions
  return Slat()->DX();
}

//_____________________________________________________________________________
Double_t  
AliMpSlatSegmentation::GetDimensionY() const
{
/// Return slat y dimensions
  return Slat()->DY();
}

//_____________________________________________________________________________
void 
AliMpSlatSegmentation::GetAllElectronicCardIDs(TArrayI& ecn) const
{
  /// Fill the array ecn with all manuIds

  Slat()->GetAllMotifPositionsIDs(ecn);
}

//_____________________________________________________________________________
const char*
AliMpSlatSegmentation::GetName() const
{
  /// The name of this segmentation is "SlatSegmentation"+slatName

  static TString name("SlatSegmentation");
  if ( fkSlat) 
  {
    name += ".";
    name += fkSlat->GetName();
  }
  return name.Data();
}

//_____________________________________________________________________________
Int_t 
AliMpSlatSegmentation::MaxPadIndexX() const
{
  ///
  /// Returns the value of the largest pad index in x-direction.
  ///
  
  return fkSlat->GetMaxPadIndexX();
}

//_____________________________________________________________________________
Int_t 
AliMpSlatSegmentation::MaxPadIndexY() const
{
  ///
  /// Returns the value of the largest pad index in y-direction.
  ///
  
  return fkSlat->GetMaxNofPadsY()-1;
}

//_____________________________________________________________________________
Int_t 
AliMpSlatSegmentation::NofPads() const
{
/// Return number of pads defined in the slat
  
  return fkSlat->NofPads();
}

//_____________________________________________________________________________
AliMpPad
AliMpSlatSegmentation::PadByLocation(Int_t manuId, Int_t manuChannel, 
                                     Bool_t warning) const
{
  ///
  /// Returns the pad specified by its location, where location is the 
  /// pair (ManuID,ManuChannel).
  /// If warning=kTRUE and the pad does not exist, a warning message is 
  /// printed.
  ///
  /// AliMpPad::Invalid() is returned if there's no pad at the given location.
  ///
  AliMpMotifPosition* motifPos = fkSlat->FindMotifPosition(manuId);
	
  if (!motifPos)
  {
    if (warning)
    {
      AliWarning(Form("Manu ID %d not found in slat %s",
                       manuId, fkSlat->GetID()));
    }
    return AliMpPad::Invalid();
  }
  AliMpVMotif* motif = motifPos->GetMotif();
  MpPair_t localIndices = 
    motif->GetMotifType()->FindLocalIndicesByGassiNum(manuChannel);
	
  if ( localIndices < 0 ) 
  {
    if (warning) 
    {
      AliWarning(Form("The pad number %d doesn't exists",
                 manuChannel));
    }
    return AliMpPad::Invalid();
  }

  Double_t posx, posy;
  motif->PadPositionLocal(localIndices, posx, posy);
  posx += motifPos->GetPositionX() - fkSlat->GetPositionX();
  posy += motifPos->GetPositionY() - fkSlat->GetPositionY();

  Double_t dx, dy;
  motif->GetPadDimensionsByIndices(localIndices, dx, dy);
	
  return AliMpPad(manuId, manuChannel,
                  motifPos->GlobalIndices(localIndices),
                  posx, posy, dx, dy);
}

//_____________________________________________________________________________
AliMpPad
AliMpSlatSegmentation::PadByIndices(Int_t ix, Int_t iy, 
                                    Bool_t warning) const
{
  ///
  /// Returns the pad specified by its integer indices.
  /// If warning=kTRUE and the pad does not exist, a warning message is 
  /// printed.
  ///
  /// AliMpPad::Invalid() is returned if there's no pad at the given location.
  ///
  ///  
  /// FIXME: except for the FindMotifPosition below, this method
  /// is exactly as the one in AliMpSectorSegmentation.
  /// See if we can merge them somehow.
	
  AliMpMotifPosition* motifPos = fkSlat->FindMotifPosition(ix,iy);
  
  if (!motifPos)
  {
    if ( warning ) 
    {
      AliWarning(Form("No motif found containing pad location (%d,%d)",ix,iy));	  
    }
    return AliMpPad::Invalid();
  }
	
  AliMpVMotif* motif = motifPos->GetMotif();
  AliMpMotifType* motifType = motif->GetMotifType();
  MpPair_t localIndices = AliMp::Pair(ix,iy) - motifPos->GetLowIndicesLimit();
  AliMpConnection* connection 
    = motifType->FindConnectionByLocalIndices(localIndices);
  
  if (!connection)
  {
    if ( warning )
    {
      AliWarning(Form("No connection for pad location (%d,%d)",ix,iy));
    }
    return AliMpPad::Invalid();
  }

  Double_t posx, posy;
  motif->PadPositionLocal(localIndices, posx, posy);
  posx += motifPos->GetPositionX() - fkSlat->GetPositionX();
  posy += motifPos->GetPositionY() - fkSlat->GetPositionY();

  Double_t dx, dy;
  motif->GetPadDimensionsByIndices(localIndices, dx, dy);

  return AliMpPad(motifPos->GetID(),connection->GetManuChannel(),
                  ix, iy, posx, posy, dx, dy);
}

//_____________________________________________________________________________
AliMpPad
AliMpSlatSegmentation::PadByPosition(Double_t x, Double_t y, 
                                     Bool_t warning) const
{
  ///
  /// Returns the pad specified by its (floating point) position.
  /// If warning=kTRUE and the pad does not exist, a warning message is 
  /// printed.
  ///
  /// AliMpPad::Invalid() is returned if there's no pad at the given location.
  ///
  
  Double_t blPosX(x);
  Double_t blPosY(y);
  
  blPosX += fkSlat->GetPositionX();
  blPosY += fkSlat->GetPositionY(); // position relative to bottom-left of the slat.
  
  AliMpMotifPosition* motifPos = fkSlat->FindMotifPosition(blPosX,blPosY);
	
  if (!motifPos)
  {
    if (warning) 
    {
      AliWarning(Form("Slat %s Position (%e,%e)/center (%e,%e)/bottom-left cm "
                      " outside limits",fkSlat->GetID(),x,y,
                      blPosX,blPosY));
    }
    return AliMpPad::Invalid();
  }
	
  AliMpVMotif* motif =  motifPos->GetMotif();  

  blPosX -= motifPos->GetPositionX();
  blPosY -= motifPos->GetPositionY();
  MpPair_t localIndices = motif->PadIndicesLocal(blPosX, blPosY);
	
  AliMpConnection* connect = 
    motif->GetMotifType()->FindConnectionByLocalIndices(localIndices);
	
  if (!connect)
  {
    if (warning) 
    {
      AliWarning(Form("Slat %s localIndices (%d,%d) outside motif %s limits",
                      fkSlat->GetID(),AliMp::PairFirst(localIndices),
                      AliMp::PairSecond(localIndices),motif->GetID().Data()));
    }
    return AliMpPad::Invalid();
  }

  Double_t posx, posy;
  motif->PadPositionLocal(localIndices, posx, posy);
  posx += motifPos->GetPositionX() - fkSlat->GetPositionX();
  posy += motifPos->GetPositionY() - fkSlat->GetPositionY();

  Double_t dx, dy;
  motif->GetPadDimensionsByIndices(localIndices, dx, dy);
    
  return AliMpPad(motifPos->GetID(),connect->GetManuChannel(),
                  motifPos->GlobalIndices(localIndices),
                  posx, posy, dx, dy);
}

//_____________________________________________________________________________
AliMp::PlaneType
AliMpSlatSegmentation::PlaneType() const
{
  return Slat()->PlaneType();
}

//_____________________________________________________________________________
void
AliMpSlatSegmentation::Print(Option_t* opt) const
{
/// Printing

  fkSlat->Print(opt);
}

//_____________________________________________________________________________
const AliMpSlat* 
AliMpSlatSegmentation::Slat() const
{
  ///
  /// Returns the pointer to the referenced slat.
  ///
  
  return fkSlat;
}

//_____________________________________________________________________________
Bool_t 
AliMpSlatSegmentation::HasPadByIndices(Int_t ix, Int_t iy) const
{
  /// Tell whether we have a pad at indices=(ix,iy)
  
  AliMpMotifPosition* motifPos = Slat()->FindMotifPosition(ix, iy);
  
  if (motifPos) return motifPos->HasPadByIndices(AliMp::Pair(ix, iy));
  
  return kFALSE;
}

//_____________________________________________________________________________
Bool_t 
AliMpSlatSegmentation::HasPadByLocation(Int_t manuId, Int_t manuChannel) const
{
  /// Tell whether we have a pad at location=(manuId,manuChannel)
  
  AliMpMotifPosition* motifPos = Slat()->FindMotifPosition(manuId);
  
  if ( motifPos ) return motifPos->HasPadByManuChannel(manuChannel);
  
  return kFALSE;  
}


//_____________________________________________________________________________
Int_t 
AliMpSlatSegmentation::GetNofElectronicCards() const
{
  /// Get the number of manus of this slat
  return Slat()->GetNofElectronicCards();

}

//_____________________________________________________________________________
Double_t  
AliMpSlatSegmentation::GetPositionX() const
{
/// Return x position of slat origin
  return Slat()->GetPositionX();
}

//_____________________________________________________________________________
Double_t  
AliMpSlatSegmentation::GetPositionY() const
{
/// Return y position of slat origin

  return Slat()->GetPositionY();
}

//_____________________________________________________________________________
Bool_t 
AliMpSlatSegmentation::HasMotifPosition(Int_t manuId) const
{
  /// Use default implementation
  return AliMpVSegmentation::HasMotifPosition(manuId);
}

//_____________________________________________________________________________
AliMpMotifPosition* 
AliMpSlatSegmentation::MotifPosition(Int_t manuId) const
{
  /// Get back a given manu
  return Slat()->FindMotifPosition(manuId);
}

