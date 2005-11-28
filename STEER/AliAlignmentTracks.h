#ifndef ALIALIGNMENTTRACKS_H
#define ALIALIGNMENTTRACKS_H

//*************************************************************************
// AliAlignmentTracks: main steering class which deals with the alignment *
// procedures based on reconstructed tracks.                              *
// More comments will come with the development of the interfaces and     *
// functionalities of the class.                                          *
//*************************************************************************

#include <TObject.h>

#include "AliAlignObj.h"

class TChain;
class AliTrackPointArray;
class AliAlignObj;
class AliTrackFitter;
class AliTrackResiduals;

class AliAlignmentTracks : public TObject {

 public:

  AliAlignmentTracks();
  AliAlignmentTracks(TChain *esdchain);
  AliAlignmentTracks(const char *esdfilename, const char *esdtreename = "esdTree");
  AliAlignmentTracks(const AliAlignmentTracks & alignment);
  AliAlignmentTracks& operator= (const AliAlignmentTracks& alignment);
  virtual ~AliAlignmentTracks();

  void AddESD(TChain *esdchain);
  void AddESD(const char *esdfilename, const char *esdtreename = "esdTree");

  void SetPointsFilename(const char *pointsfilename = "AliTrackPoints.root") { fPointsFilename = pointsfilename; }

  void ProcessESD(TSelector *selector);
  void ProcessESD();

  void BuildIndex();
/*   void BuildIndexLayer(AliAlignObj::ELayerID layer); */
/*   void BuildIndexVolume(UShort_t volid); */

  Bool_t ReadAlignObjs(const char *alignobjfilename = "AlignObjs.root");

  void SetTrackFitter(AliTrackFitter *fitter) { fTrackFitter = fitter; }
  void SetMinimizer(AliTrackResiduals *minimizer) { fMinimizer = minimizer; }

  void Align(Int_t iterations = 100);
  void AlignLayer(AliAlignObj::ELayerID layer,
		  AliAlignObj::ELayerID layerRangeMin = AliAlignObj::kFirstLayer,
		  AliAlignObj::ELayerID layerRangeMax = AliAlignObj::kLastLayer,
		  Int_t iterations = 1);
  void AlignVolume(UShort_t volid,
		   AliAlignObj::ELayerID layerRangeMin = AliAlignObj::kFirstLayer,
		   AliAlignObj::ELayerID layerRangeMax = AliAlignObj::kLastLayer);

 protected:

  void InitIndex();
  void ResetIndex();
  void DeleteIndex();

  void InitAlignObjs();
  void ResetAlignObjs();
  void DeleteAlignObjs();

  Int_t LoadPoints(UShort_t volid, AliTrackPointArray** &points);
  void  UnloadPoints(Int_t n, AliTrackPointArray **points);

  AliTrackFitter *CreateFitter();
  AliTrackResiduals *CreateMinimizer();

  TChain           *fESDChain;       //! Chain with ESDs
  TString           fPointsFilename; //  Name of the file containing the track point arrays
  TFile            *fPointsFile;     //  File containing the track point arrays
  TTree            *fPointsTree;     //  Tree with the track point arrays
  Int_t           **fLastIndex;      //! Last filled index in volume arrays
  TArrayI        ***fArrayIndex;     //! Volume arrays which contains the tree index
  Bool_t            fIsIndexBuilt;   //  Is points tree index built
  AliAlignObj    ***fAlignObjs;      //  Array with alignment objects
  AliTrackFitter   *fTrackFitter;    //  Pointer to the track fitter
  AliTrackResiduals*fMinimizer;      //  Pointer to track residuals minimizer

  ClassDef(AliAlignmentTracks,1)

};

#endif
