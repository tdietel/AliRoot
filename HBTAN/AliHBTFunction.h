#ifndef ALIHBTFUNCTION_H
#define ALIHBTFUNCTION_H
//____________________
///////////////////////////////////////////////////////
//                                                   //
// AliHBTFunction                                    //
//                                                   //
// Abstract Base Calss for all the function classes  //
//                                                   //
// Piotr.Skowronski@cern.ch                          //
// http://alisoft.cern.ch/people/skowron/analyzer    //
//                                                   //
///////////////////////////////////////////////////////

#include <TH1.h>

#include "AliHBTPairCut.h"
#include "AliHBTPair.h"

class TH2D;
class TH3D;

class AliHBTAnalysis;
class AliHBTParticleCut;

class AliHBTFunction: public TNamed
{
  public:
    AliHBTFunction();
    AliHBTFunction(const char* name,const char* title);
    virtual ~AliHBTFunction();
    
    virtual TH1* GetNumerator() const = 0;
    virtual TH1* GetDenominator() const = 0;
    virtual TH1* GetResult() = 0;

    virtual void Write();
    virtual void Init();
    
    TH1* GetRatio(Double_t normfactor = 1.0);
    void Rename(const Char_t * name); //renames the function and histograms ==title is the same that name
    void Rename(const Char_t * name, const Char_t * title); //renames and retitle the function and histograms
    
    void SetPairCut(AliHBTPairCut* cut);
    
    virtual AliHBTPair* CheckPair(AliHBTPair* pair);
    
  protected:
    virtual void BuildHistos() = 0;//builds default histograms
    AliHBTPairCut*   fPairCut;     //pair cut
    
  public:  
   ClassDef(AliHBTFunction,2)
};
/******************************************************************/
inline AliHBTPair* AliHBTFunction::CheckPair(AliHBTPair* pair)
{
  //check if pair and both particles meets the cut criteria
  if(fPairCut->Pass(pair)) //if the pair is BAD
   {//it is BAD 
    pair = pair->GetSwapedPair();
    if(pair)
     if(fPairCut->Pass(pair)) //so try reverse combination
       { 
        return 0x0;//it is BAD as well - so return
       }
   }
  return pair; 
}

/******************************************************************/
/******************************************************************/
/******************************************************************/

//____________________
///////////////////////////////////////////////////////
//                                                   //
// AliHBTOnePairFctn                                 //
//                                                   //
// Abstract Base Calss for Functions that need       //
// one pair to fill function                         //
//                                                   //
// Piotr.Skowronski@cern.ch                          //
// http://alisoft.cern.ch/people/skowron/analyzer    //
//                                                   //
///////////////////////////////////////////////////////

class AliHBTOnePairFctn
{
  public:
    AliHBTOnePairFctn(){}
    virtual ~AliHBTOnePairFctn(){}
    
    virtual void ProcessSameEventParticles(AliHBTPair* pair) = 0;
    virtual void ProcessDiffEventParticles(AliHBTPair* pair) = 0;

    virtual void Init() = 0;
    virtual void Write() = 0;
    
  protected:
  public:  
   ClassDef(AliHBTOnePairFctn,2)
};
/******************************************************************/
/******************************************************************/
/******************************************************************/

//____________________
///////////////////////////////////////////////////////
//                                                   //
// AliHBTTwoPairFctn                                 //
//                                                   //
// Abstract Base Calss for Functions that need       //
// two pairs to fill function,                       //
// one reconstructed track and corresponding         //
// simulated pair                                    //
// Basically resolution functions                    //
// Lednicky's algorithm uses that as well            //
//                                                   //
// Piotr.Skowronski@cern.ch                          //
// http://alisoft.cern.ch/people/skowron/analyzer    //
//                                                   //
///////////////////////////////////////////////////////

class AliHBTTwoPairFctn
{
  public:
    AliHBTTwoPairFctn(){};
    virtual ~AliHBTTwoPairFctn(){};
    
    virtual void 
    ProcessSameEventParticles(AliHBTPair* trackpair, AliHBTPair* partpair) = 0;
    virtual void 
    ProcessDiffEventParticles(AliHBTPair* trackpair, AliHBTPair* partpair) = 0;
    
    virtual void Init() = 0;
    virtual void Write() = 0;
	     
  protected:
  public:  
   ClassDef(AliHBTTwoPairFctn,2)
  
};
/******************************************************************/
/******************************************************************/
/******************************************************************/

//____________________
///////////////////////////////////////////////////////
//                                                   //
// AliHBTFunction1D                                  //
//                                                   //
// Base Calss for 1-dimensinal Functions             //
//                                                   //
// Piotr.Skowronski@cern.ch                          //
// http://alisoft.cern.ch/people/skowron/analyzer    //
//                                                   //
///////////////////////////////////////////////////////


class AliHBTFunction1D: public AliHBTFunction
{
 private:
  //this must be declared before constructors because they are used as a default arguments
  static const Int_t fgkDefaultNBins;//default number of Bins in histograms
  static const Float_t fgkDefaultMin;//Default min value of histograms
  static const Float_t fgkDefaultMax;//Default max value of histograms
  static const UInt_t fgkDefaultNBinsToScale;//Default number of bins used for scaling to tale

 public:
  AliHBTFunction1D();//default conmstructor
  AliHBTFunction1D(Int_t nbins, Float_t maxXval, Float_t minXval);
  AliHBTFunction1D(const Char_t *name, const Char_t *title);
  AliHBTFunction1D(const Char_t *name, const Char_t *title,
                   Int_t nbins, Float_t maxXval, Float_t minXval);
  virtual ~AliHBTFunction1D();
  
  TH1* GetNumerator() const {return fNumerator;}//returns numerator histogram
  TH1* GetDenominator() const {return fDenominator;}//returns denominator histogram

  Double_t Scale();
  void SetNumberOfBinsToScale(Int_t n = fgkDefaultNBinsToScale){fNBinsToScale = n;}

 protected:
  //retruns velue to be histogrammed
  virtual void BuildHistos(Int_t nbins, Float_t max, Float_t min);
  virtual void BuildHistos();
  Double_t Scale(TH1D* num,TH1D* den);
  
  TH1D* fNumerator;
  TH1D* fDenominator;
  UInt_t fNBinsToScale;

 public:
  ClassDef(AliHBTFunction1D,2)
};

/******************************************************************/
/******************************************************************/
/******************************************************************/

//____________________
///////////////////////////////////////////////////////
//                                                   //
// AliHBTFunction2D                                  //
//                                                   //
// Base Calss for 2-dimensinal Functions             //
//                                                   //
// Piotr.Skowronski@cern.ch                          //
// http://alisoft.cern.ch/people/skowron/analyzer    //
//                                                   //
///////////////////////////////////////////////////////
 
class AliHBTFunction2D: public AliHBTFunction
{
 private:
  //this must be declared before constructors because they are used as a default arguments
  static const Int_t fgkDefaultNBinsX;//default number of Bins in X axis in histograms
  static const Float_t fgkDefaultMinX;//Default min value of X axis in histograms
  static const Float_t fgkDefaultMaxX;//Default max value of X axis inhistograms
  static const Int_t fgkDefaultNBinsY;//default number of Bins in histograms
  static const Float_t fgkDefaultMinY;//Default min value of histograms
  static const Float_t fgkDefaultMaxY;//Default max value of histograms

  static const UInt_t fgkDefaultNBinsToScaleX;//Default number of X bins used for scaling to tale
  static const UInt_t fgkDefaultNBinsToScaleY;//Default number of bins used for scaling to tale

 public:
  AliHBTFunction2D();

  AliHBTFunction2D(const Char_t *name, const Char_t *title);

  AliHBTFunction2D(Int_t nXbins, Double_t maxXval, Double_t minXval, 
                      Int_t nYbins, Double_t maxYval, Double_t minYval);

  AliHBTFunction2D(const Char_t *name, const Char_t *title,
                      Int_t nXbins, Double_t maxXval, Double_t minXval, 
                      Int_t nYbins, Double_t maxYval, Double_t minYval);
	  
  virtual ~AliHBTFunction2D();
  
  TH1* GetNumerator() const {return fNumerator;}
  TH1* GetDenominator() const {return fDenominator;}
  
  void SetNumberOfBinsToScale(UInt_t xn = fgkDefaultNBinsToScaleX, 
                              UInt_t yn = fgkDefaultNBinsToScaleY);
  
  Double_t Scale();
 protected:
  virtual void BuildHistos(Int_t nxbins, Float_t xmax, Float_t xmin,
                           Int_t nybins, Float_t ymax, Float_t ymin);
  virtual void BuildHistos();
  
  TH2D* fNumerator;
  TH2D* fDenominator;
  
  //definition of area used for scaling -> Scale is calculated this 
  //way that after division tale is on 1
  UInt_t fNBinsToScaleX;//number of bins on X axis
  UInt_t fNBinsToScaleY;//number of bins on Y axis

 public:
  ClassDef(AliHBTFunction2D,2)
};
/******************************************************************/
/******************************************************************/
/******************************************************************/

//____________________
///////////////////////////////////////////////////////
//                                                   //
// AliHBTFunction3D                               //
//                                                   //
// Base Calss for 3-dimensinal Functions that need   //
// one pair to fill function                         //
//                                                   //
// Piotr.Skowronski@cern.ch                          //
// http://alisoft.cern.ch/people/skowron/analyzer    //
//                                                   //
///////////////////////////////////////////////////////

class AliHBTFunction3D: public AliHBTFunction
{
 private:
  //this must be declared before constructors because they are used as a default arguments
  static const Int_t fgkDefaultNBinsX;//default number of Bins in X axis in histograms
  static const Float_t fgkDefaultMinX;//Default min value of X axis in histograms
  static const Float_t fgkDefaultMaxX;//Default max value of X axis inhistograms
  static const Int_t fgkDefaultNBinsY;//default number of Bins in Y axis in histograms
  static const Float_t fgkDefaultMinY;//Default min value of Y axis in histograms
  static const Float_t fgkDefaultMaxY;//Default max value of Y axis inhistograms
  static const Int_t fgkDefaultNBinsZ;//default number of Bins in Z axis in histograms
  static const Float_t fgkDefaultMinZ;//Default min value of Z axis in histograms
  static const Float_t fgkDefaultMaxZ;//Default max value of Z axis inhistograms

  static const UInt_t fgkDefaultNBinsToScaleX;//Default number of X bins used for scaling to tale
  static const UInt_t fgkDefaultNBinsToScaleY;//Default number of Y bins used for scaling to tale
  static const UInt_t fgkDefaultNBinsToScaleZ;//Default number of Z bins used for scaling to tale
  
 public:
  AliHBTFunction3D();

  AliHBTFunction3D(const Char_t *name, const Char_t *title);

  AliHBTFunction3D(Int_t nXbins, Double_t maxXval, Double_t minXval, 
                   Int_t nYbins, Double_t maxYval, Double_t minYval, 
                   Int_t nZbins, Double_t maxZval, Double_t minZval);

  AliHBTFunction3D(const Char_t *name, const Char_t *title,
                   Int_t nXbins, Double_t maxXval, Double_t minXval, 
                   Int_t nYbins, Double_t maxYval, Double_t minYval, 
                   Int_t nZbins, Double_t maxZval, Double_t minZval);
   
  virtual ~AliHBTFunction3D();//destructor

  TH1* GetNumerator() const {return fNumerator;}
  TH1* GetDenominator() const {return fDenominator;}


  void SetNumberOfBinsToScale(UInt_t xn = fgkDefaultNBinsToScaleX, 
                              UInt_t yn = fgkDefaultNBinsToScaleY,
                              UInt_t zn = fgkDefaultNBinsToScaleZ);

  Double_t Scale();

 protected:
  virtual void BuildHistos(Int_t nxbins, Float_t xmax, Float_t xmin,
                           Int_t nybins, Float_t ymax, Float_t ymin,
	       Int_t nzbins, Float_t zmax, Float_t zmin);
  virtual void BuildHistos();
  
  TH3D* fNumerator;
  TH3D* fDenominator;
  
  //definition of area used for scaling -> Scale is calculated this 
  //way that after division tale is on 1
  UInt_t fNBinsToScaleX;//number of bins on X axis
  UInt_t fNBinsToScaleY;//number of bins on Y axis
  UInt_t fNBinsToScaleZ;//number of bins on Z axis
  
 public:
  ClassDef(AliHBTFunction3D,2)
};
/******************************************************************/
/******************************************************************/
/******************************************************************/

//____________________
///////////////////////////////////////////////////////
//                                                   //
// AliHBTOnePairFctn1D                               //
//                                                   //
// Base Calss for 1-dimensinal Functions that need   //
// one pair to fill function                         //
//                                                   //
// Piotr.Skowronski@cern.ch                          //
// http://alisoft.cern.ch/people/skowron/analyzer    //
//                                                   //
///////////////////////////////////////////////////////

class AliHBTOnePairFctn1D: public AliHBTOnePairFctn, public AliHBTFunction1D
{
 public:
  AliHBTOnePairFctn1D(){}//default conmstructor
  AliHBTOnePairFctn1D(Int_t nbins, Float_t maxXval, Float_t minXval);
  AliHBTOnePairFctn1D(const Char_t *name, const Char_t *title);
  AliHBTOnePairFctn1D(const Char_t *name, const Char_t *title,
                      Int_t nbins, Float_t maxXval, Float_t minXval);
  virtual ~AliHBTOnePairFctn1D(){}

  void ProcessSameEventParticles(AliHBTPair* pair);
  void ProcessDiffEventParticles(AliHBTPair* pair);
  void Write(){AliHBTFunction::Write();}
  void Init(){AliHBTFunction::Init();}
 protected:
  //retruns velue to be histogrammed
  virtual Double_t GetValue(AliHBTPair* pair) = 0; 
  ClassDef(AliHBTOnePairFctn1D,2)
};
/******************************************************************/
/******************************************************************/
/******************************************************************/

//____________________
///////////////////////////////////////////////////////
//                                                   //
// AliHBTOnePairFctn2D                               //
//                                                   //
// Base Calss for 2-dimensinal Functions that need   //
// one pair to fill function                         //
//                                                   //
// Piotr.Skowronski@cern.ch                          //
// http://alisoft.cern.ch/people/skowron/analyzer    //
//                                                   //
///////////////////////////////////////////////////////

class AliHBTOnePairFctn2D: public AliHBTOnePairFctn, public AliHBTFunction2D
{
 public:
  AliHBTOnePairFctn2D(){}

  AliHBTOnePairFctn2D(const Char_t *name, const Char_t *title);

  AliHBTOnePairFctn2D(Int_t nXbins, Double_t maxXval, Double_t minXval, 
                      Int_t nYbins, Double_t maxYval, Double_t minYval);

  AliHBTOnePairFctn2D(const Char_t *name, const Char_t *title,
                      Int_t nXbins, Double_t maxXval, Double_t minXval, 
                      Int_t nYbins, Double_t maxYval, Double_t minYval);
	  
  virtual ~AliHBTOnePairFctn2D(){}
  
  void ProcessSameEventParticles(AliHBTPair* pair);
  void ProcessDiffEventParticles(AliHBTPair* pair);
  void Write(){AliHBTFunction::Write();}
  void Init(){AliHBTFunction::Init();}
 protected:
  virtual void GetValues(AliHBTPair* pair, Double_t& x, Double_t& y) = 0;
  ClassDef(AliHBTOnePairFctn2D,2)
};
/******************************************************************/
/******************************************************************/
/******************************************************************/

//____________________
///////////////////////////////////////////////////////
//                                                   //
// AliHBTOnePairFctn3D                               //
//                                                   //
// Base Calss for 3-dimensinal Functions that need   //
// one pair to fill function                         //
//                                                   //
// Piotr.Skowronski@cern.ch                          //
// http://alisoft.cern.ch/people/skowron/analyzer    //
//                                                   //
///////////////////////////////////////////////////////

class AliHBTOnePairFctn3D: public AliHBTOnePairFctn, public AliHBTFunction3D
{
 public:
  AliHBTOnePairFctn3D(){}

  AliHBTOnePairFctn3D(const Char_t *name, const Char_t *title);

  AliHBTOnePairFctn3D(Int_t nXbins, Double_t maxXval, Double_t minXval, 
                      Int_t nYbins, Double_t maxYval, Double_t minYval, 
                      Int_t nZbins, Double_t maxZval, Double_t minZval);

  AliHBTOnePairFctn3D(const Char_t *name, const Char_t *title,
                      Int_t nXbins, Double_t maxXval, Double_t minXval, 
                      Int_t nYbins, Double_t maxYval, Double_t minYval, 
                      Int_t nZbins, Double_t maxZval, Double_t minZval);
   
  virtual ~AliHBTOnePairFctn3D(){}//destructor

  void ProcessSameEventParticles(AliHBTPair* pair);
  void ProcessDiffEventParticles(AliHBTPair* pair);
  void Write(){AliHBTFunction::Write();}
  void Init(){AliHBTFunction::Init();}
 protected:
  virtual void GetValues(AliHBTPair* pair, Double_t& x, Double_t& y, Double_t& z) = 0;
 ClassDef(AliHBTOnePairFctn3D,2)
};
/******************************************************************/
/******************************************************************/
/******************************************************************/

//____________________
///////////////////////////////////////////////////////
//                                                   //
// AliHBTTwoPairFctn1D                               //
//                                                   //
// Base Calss for 1-dimensinal Functions that need   //
// two pair (simulated and reconstructed)            //
// to fill function                                  //
//                                                   //
// Piotr.Skowronski@cern.ch                          //
// http://alisoft.cern.ch/people/skowron/analyzer    //
//                                                   //
///////////////////////////////////////////////////////

class AliHBTTwoPairFctn1D: public AliHBTTwoPairFctn, public AliHBTFunction1D
{
 public:
  AliHBTTwoPairFctn1D(){}//default conmstructor
  AliHBTTwoPairFctn1D(Int_t nbins, Float_t maxXval, Float_t minXval);
  AliHBTTwoPairFctn1D(const Char_t *name, const Char_t *title);
  AliHBTTwoPairFctn1D(const Char_t *name, const Char_t *title,
                      Int_t nbins, Float_t maxXval, Float_t minXval);
  virtual ~AliHBTTwoPairFctn1D(){}

  void ProcessSameEventParticles(AliHBTPair* trackpair, AliHBTPair* partpair);
  void ProcessDiffEventParticles(AliHBTPair* trackpair, AliHBTPair* partpair);
  void Write(){AliHBTFunction::Write();}
  void Init(){AliHBTFunction::Init();}
 protected:
  virtual Double_t GetValue(AliHBTPair* trackpair, AliHBTPair* partpair) = 0;
 public: 
  ClassDef(AliHBTTwoPairFctn1D,2)
};
/******************************************************************/
/******************************************************************/
/******************************************************************/

//____________________
///////////////////////////////////////////////////////
//                                                   //
// AliHBTTwoPairFctn2D                               //
//                                                   //
// Base Calss for 2-dimensinal Functions that need   //
// two pair (simulated and reconstructed)            //
// to fill function                                  //
//                                                   //
// Piotr.Skowronski@cern.ch                          //
// http://alisoft.cern.ch/people/skowron/analyzer    //
//                                                   //
///////////////////////////////////////////////////////

class AliHBTTwoPairFctn2D: public AliHBTTwoPairFctn, public AliHBTFunction2D
{
 public:
  AliHBTTwoPairFctn2D(){}

  AliHBTTwoPairFctn2D(const Char_t *name, const Char_t *title);

  AliHBTTwoPairFctn2D(Int_t nXbins, Double_t maxXval, Double_t minXval, 
                      Int_t nYbins, Double_t maxYval, Double_t minYval);

  AliHBTTwoPairFctn2D(const Char_t *name, const Char_t *title,
                      Int_t nXbins, Double_t maxXval, Double_t minXval, 
                      Int_t nYbins, Double_t maxYval, Double_t minYval);
	  
  virtual ~AliHBTTwoPairFctn2D(){}
  
  void ProcessSameEventParticles(AliHBTPair* trackpair, AliHBTPair* partpair);
  void ProcessDiffEventParticles(AliHBTPair* trackpair, AliHBTPair* partpair);
  void Write(){AliHBTFunction::Write();}
  void Init(){AliHBTFunction::Init();}

 protected:
  virtual void GetValues(AliHBTPair* trackpair, AliHBTPair* partpair, Double_t& x, Double_t& y) = 0;

 public:
  ClassDef(AliHBTTwoPairFctn2D,2)
};
/******************************************************************/
/******************************************************************/
/******************************************************************/

//____________________
///////////////////////////////////////////////////////
//                                                   //
// AliHBTTwoPairFctn3D                               //
//                                                   //
// Base Calss for 3-dimensinal Functions that need   //
// two pair (simulated and reconstructed)            //
// to fill function                                  //
//                                                   //
// Piotr.Skowronski@cern.ch                          //
// http://alisoft.cern.ch/people/skowron/analyzer    //
//                                                   //
///////////////////////////////////////////////////////

class AliHBTTwoPairFctn3D: public AliHBTTwoPairFctn, public AliHBTFunction3D
{
 public:
  AliHBTTwoPairFctn3D(){}
  
  AliHBTTwoPairFctn3D(const Char_t *name, const Char_t *title);

  AliHBTTwoPairFctn3D(Int_t nXbins, Double_t maxXval, Double_t minXval, 
                      Int_t nYbins, Double_t maxYval, Double_t minYval, 
                      Int_t nZbins, Double_t maxZval, Double_t minZval);

  AliHBTTwoPairFctn3D(const Char_t *name, const Char_t *title,
                      Int_t nXbins, Double_t maxXval, Double_t minXval, 
                      Int_t nYbins, Double_t maxYval, Double_t minYval, 
                      Int_t nZbins, Double_t maxZval, Double_t minZval);
   
  virtual ~AliHBTTwoPairFctn3D(){}//destructor

  void ProcessSameEventParticles(AliHBTPair* trackpair, AliHBTPair* partpair);
  void ProcessDiffEventParticles(AliHBTPair* trackpair, AliHBTPair* partpair);
  void Write(){AliHBTFunction::Write();}
  void Init(){AliHBTFunction::Init();}

 protected:
  virtual void GetValues(AliHBTPair* trackpair, AliHBTPair* partpair, Double_t& x, Double_t& y, Double_t& z) = 0;

 public:
  ClassDef(AliHBTTwoPairFctn3D,2)
};

#endif
