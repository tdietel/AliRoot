#ifndef ALITPCCHEBDIST_H
#define ALITPCCHEBDIST_H


/*****************************************************************************
 *         Wrapper for 2D->ND Chebishev parameterizations in TPC volume      *
 *                                                                           *
 *  Class is similar to its base class AliTPCChebCorr, but the evaluation    *
 *  is obtained as a linear interpolation of the evaluations on the slices   *
 *  encompassing the queried X (in sector coordinates). These slices are     *
 *  are not the TPC rows but just a grid for evaluation, their number        *
 *  should be in general > kNRows of the TPC                                 *
 *                                                                           *
 *         Author: ruben.shahoyan@cern.ch                                    *
 *****************************************************************************/

#include <TNamed.h>
#include "AliTPCChebCorr.h"

class AliTPCChebDist : public AliTPCChebCorr
{
 public:
  //
 public:
  //
  AliTPCChebDist();
  AliTPCChebDist(const char* name, const char* title, int nps=1,int nzs=1, float zmaxAbs=250);
  virtual ~AliTPCChebDist() {}
  //
  Float_t  GetXMin()                             const {return fXMin;}
  Float_t  GetXMax()                             const {return fXMax;}  
  //
  void     Eval(int sector, float x, float y2x, float z,float *distortion) const;
  void     Eval(int sector, float xtz[3], float *distortion)               const;
  //
  virtual  Bool_t   IsCorrection()               const {return kFALSE;}
  virtual  Bool_t   IsDistortion()               const {return kTRUE;}
  virtual  void     Init();
  //
  Int_t    X2Slice(float x) const;
  Float_t  Slice2X(int ix)  const;
  //
  Float_t  GetScaleDnDeta2pp13TeV() const  {return fScaleDnDeta2pp13TeV;}
  void     SetScaleDnDeta2pp13TeV(float v) {fScaleDnDeta2pp13TeV = v;}
  //
  static void SetExtraCVDistortions(float amp=0.7, float scaleY=0.3333333, float scaleZ=0.6, Bool_t incr = kTRUE);

 protected:
  //
  Float_t  fXMin;                                       // min X
  Float_t  fXMax;                                       // max X
  Float_t  fDX;                                         // X step
  Float_t  fDXInv;                                      // inverse of X step
  Float_t  fScaleDnDeta2pp13TeV;                        // dndeta_current / dndeta_pp@13TeV
  //
  static Float_t fgRMinTPC;                             // def. min radius
  static Float_t fgRMaxTPC;                             // def. max radius
  static Int_t   fgNSlices;                             // def. number of slices in X

  // extra distotion on the IROC edges fgExtraCVDistAmp*exp(-|y-ymax|*fgExtraCVDistScale)
  static Float_t fgExtraCVDistScaleI;
  static Float_t fgExtraCVDistRange;                           // 5/fgExtraCVDistScale : range to edge to activate distortions
  static Float_t fgExtraCVDistAmp;                             // amplitude
  static Float_t fgExtraCVZRangeI2;                            // inverse Z range squared
  static Bool_t  fgExtraCVDistAddIncrementally;                // if true, apply extra distortions after applyin regular ones
 private:
  mutable Bool_t  fCacheValid;                                  //! flag cache validity
  mutable Float_t fCacheDistLow[4];                             //! cache value
  mutable Float_t fCacheDistUp[4];                              //! cache value
  mutable Float_t fCacheY2X;                                    //! cache value
  mutable Float_t fCacheZ2X;                                    //! cache value
  mutable Int_t   fCacheSector;                                 //! cache value
  mutable Int_t   fCacheIXLow;                                  //! cache value
  //
  AliTPCChebDist(const AliTPCChebDist& src);            // dummy
  AliTPCChebDist& operator=(const AliTPCChebDist& rhs); // dummy
  //
  ClassDef(AliTPCChebDist,2)
};

//_________________________________________________________________
inline Float_t AliTPCChebDist::Slice2X(Int_t ix) const
{
  // get the lower slice encompacing given X, except if the X is outside of the fid. range
  return fXMin + ix*fDX;
}

//_________________________________________________________________
inline Int_t AliTPCChebDist::X2Slice(float x) const
{
  // get the lower slize covering given X
  int ix = (x-fXMin)*fDXInv;
  if      (ix<0)         ix = 0;          
  else if (ix>=fNRows)   ix = fNRows-1;
  return ix;
}

//____________________________________________________________________
inline void AliTPCChebDist::Eval(int sector, float xtz[3], float *distortion) const
{
  // Calculate distortion for point with x,y,z sector corrdinates
  // Sector is in 0-71 ROC convention, to check Zs outlying from the sector
  Eval(xtz[0],&xtz[1],distortion);
  //
}


#endif
