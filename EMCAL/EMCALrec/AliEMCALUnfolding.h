#ifndef ALIEMCALUNFOLDING_H 
#define ALIEMCALUNFOLDING_H 
/* Copyright(c) 1998-1999, ALICE Experiment at CERN, All rights reserved. * 
 * See cxx source for full Copyright notice                               */ 
      
//_________________________________________________________________________
/// \class AliEMCALUnfolding
/// \ingroup EMCALrec
/// \brief Base class for the cluster unfolding algorithm 
///
///  Based on unfolding in clusterizerv1 done by Cynthia Hadjidakis
///   * Unfolding for eta~0: Cynthia Hadjidakis - still in AliEMCALCLusterizerv1
///   * Unfolding extension for whole EMCAL: Adam Matyja (SUBATECH & INP PAN)
///
///  It unfolds the clusters having several local maxima. 
///
/// \author Adam Matyja (SUBATECH & INP PAN)
///
//////////////////////////////////////////////////////////////////////////////
 
// --- ROOT system --- 
#include "TObject.h"  
class AliLog ; 

// --- AliRoot header files --- 
class AliEMCALGeometry ; 
class AliEMCALRecPoint ;  
class AliEMCALDigit ; 
 
class AliEMCALUnfolding : public TObject 
{ 
 
public: 
 
  AliEMCALUnfolding() ;          // default ctor 
  virtual ~AliEMCALUnfolding() ; // dtor
  AliEMCALUnfolding(AliEMCALGeometry* geometry);// constructor 
  AliEMCALUnfolding(AliEMCALGeometry* geometry,Float_t ECALocMaxCut,Double_t *SSPars,Double_t *Par5,Double_t *Par6);// constructor 
 
  virtual void Init() ; 
  virtual void SetInput(Int_t numberOfECAClusters,TObjArray *recPoints,TClonesArray *digitsArr); 
 
  // Setters and getters 
  virtual void    SetNumberOfECAClusters(Int_t n) { fNumberOfECAClusters = n    ; } 
  virtual Int_t   GetNumberOfECAClusters()  const { return fNumberOfECAClusters ; } 
  
  virtual void    SetRecPoints(TObjArray *rec)    { fRecPoints = rec  ; } 
  virtual TObjArray * GetRecPoints()        const { return fRecPoints ; } 
 
  virtual void    SetDigitsArr(TClonesArray *digit) { fDigitsArr = digit; } 
  virtual TClonesArray * GetDigitsArr() const       { return fDigitsArr  ; } 
  
  virtual void    SetECALocalMaxCut(Float_t cut)  { fECALocMaxCut = cut  ; } 
  virtual Float_t GetECALocalMaxCut()       const { return fECALocMaxCut ; } 
  
  virtual void    SetThreshold(Float_t energy)    { fThreshold = energy  ; } 
  virtual Float_t GetThreshold()            const { return fThreshold    ; } 
  
  virtual void    SetRejectBelowThreshold(Bool_t reject) { fRejectBelowThreshold = reject ; }
  virtual Bool_t  GetRejectBelowThreshold()        const { return fRejectBelowThreshold   ; }

  virtual void    SetRange(Float_t range) { fRange = range ; }
  virtual Float_t GetRange()        const { return fRange  ; }

  // Unfolding main methods 
  virtual void      MakeUnfolding(); 
  static Double_t   ShowerShapeV2(Double_t x, Double_t y) ; // Shape of EM shower used in unfolding;  
                                              //class member function (not object member function) 
  static void UnfoldingChiSquareV2(Int_t & nPar, Double_t * Grad, Double_t & fret, Double_t * x, Int_t iflag)  ; 
                                              // Chi^2 of the fit. Should be static to be passes to MINUIT 
  virtual void      SetShowerShapeParams(Double_t *pars) ; 
  virtual Double_t* GetShowerShapeParams() const { return fgSSPars ; } 
  
  virtual void      SetPar5(Double_t *pars) ; 
  virtual Double_t* GetPar5() const { return fgPar5 ; } 
  
  virtual void      SetPar6(Double_t *pars) ; 
  virtual Double_t* GetPar6() const { return fgPar6 ; } 

  virtual Int_t     UnfoldOneCluster(AliEMCALRecPoint * iniTower, 
				 Int_t nMax, 
				 AliEMCALDigit ** maxAt, 
				 Float_t * maxAtEnergy,
				 TObjArray *list);//input one cluster -> output list
 
protected:
  
  Int_t              fNumberOfECAClusters ;   ///<  Number of clusters found in EC section
  Float_t            fECALocMaxCut ;          ///<  Minimum energy difference to distinguish local maxima in a cluster
  Float_t            fThreshold ;             ///<  Minimum energy for cell to be joined to a cluster
  Bool_t             fRejectBelowThreshold ;  ///<  Split (false) or reject (true) cell energy below threshold after UF
  Float_t            fRange ;                 ///<  Range (distance from maximum) of application of unfolding
  AliEMCALGeometry * fGeom;                   //!<! Pointer to geometry for utilities
  TObjArray        * fRecPoints;              ///<  Array with EMCAL clusters
  TClonesArray     * fDigitsArr;              ///<  Array with EMCAL digits
  
private:
  
  AliEMCALUnfolding              (const AliEMCALUnfolding &); //copy ctor
  AliEMCALUnfolding & operator = (const AliEMCALUnfolding &);
  
  Bool_t  UnfoldClusterV2(AliEMCALRecPoint * iniEmc, Int_t Nmax,
                          AliEMCALDigit ** maxAt,
                          Float_t * maxAtEnergy ); //Unfolds cluster using TMinuit package
  
  Bool_t  UnfoldClusterV2old(AliEMCALRecPoint * iniEmc, Int_t Nmax,
                             AliEMCALDigit ** maxAt,
                             Float_t * maxAtEnergy ); //Unfolds cluster using TMinuit package
  
  Bool_t  FindFitV2(AliEMCALRecPoint * emcRP, AliEMCALDigit ** MaxAt, const Float_t * maxAtEnergy,
                    Int_t NPar, Float_t * FitParametres) const; //Used in UnfoldClusters, calls TMinuit 
  
  /// Unfolding shower shape parameters 
  /// function: 
  /// f(r)=exp(-(p0*r)^p1 * (1/(p2+p3*(p0*r)^p1)+p4/(1+p6*(p0*r)^p5) ) ) 
  /// p0,p1,p2,p3,p4 are fixed 
  /// params p5 and p6 are phi-dependent and set in ShowerShapeV2
  static Double_t fgSSPars[8]; ///<  
  
  static Double_t fgPar5[3];   ///< UF SSPar nr 5 = p0 + phi*p1 + phi^2 *p2 
  static Double_t fgPar6[3];   ///< UF SSPar nr 6 = p0 + phi*p1 + phi^2 *p2 
  
  static void EvalPar5(Double_t phi); 
  static void EvalPar6(Double_t phi); 
  static void EvalParsPhiDependence(Int_t absId, const AliEMCALGeometry *geom); 
 
  /// \cond CLASSIMP
  ClassDef(AliEMCALUnfolding,4) ;
  /// \endcond

} ; 
 
#endif // AliEMCALUNFOLDING_H 
