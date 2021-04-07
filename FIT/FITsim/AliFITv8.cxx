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

/////////////////////////////////////////////////////////////////////
//                                                                 //
// FIT detector full geometry  version 8
// similar to O2
//
// Begin Html
/*
<img src="gif/AliFITv6Class.gif">
*/
// End Html
// Alla.Maevskaya@cern.ch
////T0+ optical propreties from Maciej and Noa
// V0+ part by Lizardo Valencia Palomo    lvalenci@cern.ch          //
//                                                                  //
//                                                                  //
//////////////////////////////////////////////////////////////////////

#include <Riostream.h>
#include <stdlib.h>

#include "TGeoArb8.h"
#include "TGeoBBox.h"
#include "TGeoCompositeShape.h"
#include "TGeoManager.h"
#include "TGeoMatrix.h"
#include "TGeoNode.h"
#include "TGeoTube.h"
#include "TGeoVolume.h"

#include <TGeoCone.h>
#include <TGeoMedium.h>

#include <TGeoGlobalMagField.h>
#include <TGraph.h>
#include <TLorentzVector.h>
#include <TMath.h>
#include <TParticle.h>
#include <TString.h>
#include <TVirtualMC.h>

#include "AliLog.h"
#include "AliMagF.h"
#include "AliRun.h"

#include "AliFITHits.h"
#include "AliFITv8.h"

#include "AliCDBEntry.h"
#include "AliCDBLocal.h"
#include "AliCDBManager.h"
#include "AliCDBStorage.h"
#include "AliMC.h"
#include "AliTrackReference.h"

ClassImp(AliFITv8)

    using std::cout;
using std::endl;

//--------------------------------------------------------------------

AliFITv8::AliFITv8()
    : AliFIT(), fIdSens1(0), fIdSens2(0), fPMTeff(0x0),
      // V0+
      nSectors(16), nRings(5), fCellId(0), fSenseless(-1),
      fV0PlusR0(3.97),        // Computed for z = 325 cm from IP
      fV0PlusR1(7.6),         // From V0A
      fV0PlusR2(13.8),        // From V0A
      fV0PlusR3(22.7),        // From V0A
      fV0PlusR4(41.3),        // From V0A
      fV0PlusR5(72.94),       // Computed for z = 325 cm from IP
      fV0PlusR6(72.6),        // Needed to compute fV0PlusnMeters
      fV0PlusSciWd(5.),       // From V0A
      fV0PlusFraWd(0.2),      // From V0A
      fV0PlusZposition(+318), // Must be changed to specifications from Corrado
                              // (meeting 10/11/176)
      fV0PlusnMeters(fV0PlusR6 * 0.01), // From V0A
      fV0PlusLightYield(93.75),         // From V0A
      fV0PlusLightAttenuation(0.05),    // From V0A
      fV0PlusFibToPhot(0.3),            // From V0A
      mLeftTransformation(nullptr), mRightTransformation(nullptr) {

  // Standart constructor for T0 Detector version 0

  for (Int_t i = 0; i < nSectors; i++) {
    for (Int_t j = 0; j < nRings; j++)
      fIdV0Plus[i][j] = 0;
  }
 
}
//--------------------------------------------------------------------

AliFITv8::AliFITv8(const char *name, const char *title)
    : AliFIT(name, title), fIdSens1(0), fIdSens2(0), fPMTeff(0x0),
      // V0+
      nSectors(16), nRings(5), fCellId(0), fSenseless(-1),
      fV0PlusR0(3.97),        // Computed for z = 325 cm from IP
      fV0PlusR1(7.6),         // From V0A
      fV0PlusR2(13.8),        // From V0A
      fV0PlusR3(22.7),        // From V0A
      fV0PlusR4(41.3),        // From V0A
      fV0PlusR5(72.94),       // Computed for z = 325 cm from IP
      fV0PlusR6(72.6),        // Needed to compute fV0PlusnMeters
      fV0PlusSciWd(5.),       // From V0A
      fV0PlusFraWd(0.2),      // From V0A
      fV0PlusZposition(+318), // Must be changed to specifications from Corrado
                              // (meeting 10/11/176)
      fV0PlusnMeters(fV0PlusR6 * 0.01), // From V0A
      fV0PlusLightYield(93.75),         // From V0A
      fV0PlusLightAttenuation(0.05),    // From V0A
      fV0PlusFibToPhot(0.3),            // From V0A
      mLeftTransformation(nullptr), mRightTransformation(nullptr)

{
  //
  // Standart constructor for FIT Detector version 8
  //
  std::cout << "\n*******************DeBug:On*************************\n"
            << endl;
  std::cout << "\n Info:: Constructor 1 is called..Rihan.. " << endl;
  std::cout << " #Sector = " << sNumberOfCellSectors
            << " #Ring = " << sNumberOfCellRings;
  std::cout << "\t dZ Scint = " << sDzScintillator
            << "\t Scint.Sep = " << sDrSeparationScint;
  std::cout << "\t dZPlastic = " << sZPlastic << std::endl;

  std::cout << "\n Avg Ring. Radii = " << sCellRingRadii[0] << ", "
            << sCellRingRadii[1] << ", " << sCellRingRadii[2];
  std::cout << ", " << sCellRingRadii[3] << ", " << sCellRingRadii[4]
            << std::endl;

  std::cout << "\n****************************************************\n"
            << endl;

  fIshunt = 2;
  //  SetPMTeff();
}
//_____________________________________________________________________________

/// ------------ Assign Global Variables -----------------

const std::string AliFITv8::sDetectorName = "FV0";
const std::string AliFITv8::sScintillatorName = "SCINT";
const std::string AliFITv8::sPlasticName = "PLAST";
const std::string AliFITv8::sSectorName = "SECTOR";
const std::string AliFITv8::sCellName = "CELL";
const std::string AliFITv8::sScintillatorSectorName =
    sScintillatorName + sSectorName;
const std::string AliFITv8::sScintillatorCellName =
    sScintillatorName + sCellName;
const std::string AliFITv8::sPlasticSectorName = sPlasticName + sSectorName;
const std::string AliFITv8::sPlasticCellName = sPlasticName + sCellName;
const std::string AliFITv8::sFiberName = "FIBER";
const std::string AliFITv8::sScrewName = "SCREW";
const std::string AliFITv8::sScrewHolesCSName = "FV0SCREWHOLES";
const std::string AliFITv8::sRodName = "ROD";
const std::string AliFITv8::sRodHolesCSName = "FV0RODHOLES";
const std::string AliFITv8::sContainerName = "CONTAINER";

const int AliFITv8::sNumberOfCellSectors =
    4; // < Number of cell sectors for one half of the detector
const int AliFITv8::sNumberOfCellRings = 5; // < Number of cell rings

const float AliFITv8::sDzContainer = 30.0; // <-- Depth of the metal container
const float AliFITv8::sDrContainerHole =
    4.05; // <-- Radius of the beam hole in the metal container
const float AliFITv8::sXShiftContainerHole =
    -0.15; // <-- x-shift of the beam hole in the metal container
const float AliFITv8::sDrMaxContainerBack =
    83.1; // <-- Outer radius of the container backplate
const float AliFITv8::sDzContainerBack =
    1.00; // <-- Thickness of the container backplate
const float AliFITv8::sDrMinContainerFront =
    45.7; // <-- Inner radius of the container frontplate
const float AliFITv8::sDrMaxContainerFront =
    83.1; // <-- Outer radius of the container frontplate
const float AliFITv8::sDzContainerFront =
    1.00; // <-- Thickness of the container frontplate
const float AliFITv8::sDxContainerStand =
    40.0; // <-- Width of the container stand
const float AliFITv8::sDyContainerStand =
    3.00; // <-- Height of the container stand at its center in x
const float AliFITv8::sDrMinContainerCone =
    24.3; // <-- Inner radius at bottom of container frontplate cone
const float AliFITv8::sDzContainerCone =
    16.2; // <-- Depth of the container frontplate cone
const float AliFITv8::sThicknessContainerCone =
    0.60; // <-- Thickness of the container frontplate cone
const float AliFITv8::sXYThicknessContainerCone =
    0.975; // <-- Radial thickness in the xy-plane of container cone
const float AliFITv8::sDrMinContainerOuterShield =
    82.5; // <-- Inner radius of outer container shield
const float AliFITv8::sDrMaxContainerOuterShield =
    82.65; // <-- Outer radius of outer container shield
const float AliFITv8::sDrMinContainerInnerShield =
    4.00; // <-- Inner radius of the inner container shield
const float AliFITv8::sDrMaxContainerInnerShield =
    4.05; // <-- Outer radius of inner container shield
const float AliFITv8::sDxContainerCover =
    0.15; // <-- Thickness of the container cover
const float AliFITv8::sDxContainerStandBottom =
    38.5; // <-- Width of the bottom of the container stand
const float AliFITv8::sDyContainerStandBottom =
    2.00; // <-- Thickness of the bottom of the container stand

const float AliFITv8::sEpsilon = 0.01; //   Used to make one spatial dimension
                                       //   infinitesimally larger than other
const float AliFITv8::sDzScintillator = 4.00; //   Thickness of the scintillator
const float AliFITv8::sDzPlastic = 1.00; //   Thickness of the fiber plastics
const float AliFITv8::sXGlobal =
    0.00; //   Global x-position of the geometrical center of scintillators
const float AliFITv8::sYGlobal =
    0.00; //   Global y-position of the geometrical center of scintillators
const float AliFITv8::sDxHalvesSeparation =
    0.00; //   Separation between the left and right side of the detector
const float AliFITv8::sDyHalvesSeparation =
    0.00; //   y-position of the right detector part relative to the left part
const float AliFITv8::sDzHalvesSeparation =
    0.00; //   z-position of the right detector part relative to the left part

const float AliFITv8::sXScintillator =
    sDxContainerCover; // x-position of the right half of the scintillator.
const float AliFITv8::sZGlobal =
    320 -
    sDzScintillator / 2; // Global z-pos of geometrical center of scintillators

const float AliFITv8::dxHoleCut =
    0.20; // <-- width of extension of hole 1,2,7 in "a"-type cell
const float AliFITv8::sXShiftInnerRadiusScintillator =
    -0.15; // <-- Shift of the inner radius origin of the scintillators.
const float AliFITv8::sDxHoleExtensionScintillator =
    0.200; // <-- Extension of the scintillator holes for the metal rods
const float AliFITv8::sDrHoleSmallScintillator =
    0.265; // <-- Radius of the small scintillator screw hole
const float AliFITv8::sDrHoleLargeScintillator =
    0.415; // <-- Radius of the large scintillator screw hole
const float AliFITv8::sDrSeparationScint =
    0.03 +
    0.04; // <-- Separation b/w scint cells = paint width + half of separation
const float AliFITv8::xHole =
    sDrSeparationScint +
    dxHoleCut; // <-- x-placement of holes 1, 2 and 7 in the "a" cell

const float AliFITv8::sZScintillator = 0;
const float AliFITv8::sXShiftScrews = sXScintillator;
const float AliFITv8::sZPlastic =
    sZScintillator + sDzScintillator / 2 +
    sDzPlastic / 2; // <-- z-position of the plastic cells.
const float AliFITv8::sZFiber =
    (sZPlastic + sZContainerFront) / 2; // <-- z-position of the fiber volumes.

const float AliFITv8::sZContainerBack =
    sZScintillator - sDzScintillator / 2 -
    sDzContainerBack / 2; // <-- z-pos of container backplate.
const float AliFITv8::sZContainerFront =
    sZContainerBack - sDzContainerBack / 2 + sDzContainer -
    sDzContainerFront / 2; // <-- z-pos of container frontplate.
const float AliFITv8::sZContainerMid =
    (sZContainerBack + sZContainerFront) /
    2; // <-- z-pos of the center of container.
const float AliFITv8::sZCone =
    sZContainerFront + sDzContainerFront / 2 -
    sDzContainerCone / 2; // <-- z-pos of container frontplate cone.

///// Screw properties:
const float AliFITv8::sZShiftScrew =
    0; // <-- z shift of the screws. 0 = aligned with the scintillator.
const int AliFITv8::sNumberOfScrewTypes =
    6; // <-- Number of the different screw types.
const float AliFITv8::sDrMinScrewTypes[6] = {
    0.25, 0.25, 0.40,
    0.40, 0.40, 0.4}; // <-- Radii of the thinner part of the screw types.
const float AliFITv8::sDrMaxScrewTypes[6] = {
    0.0,  0.50, 0.60,
    0.60, 0.60, 0.0}; // <-- Radii of the thicker part of the screw types.
const float AliFITv8::sDzMaxScrewTypes[6] = {
    6.02,  13.09, 13.1,
    23.10, 28.30, 5.0}; // <-- Length of the thinner part of the screw types.
const float AliFITv8::sDzMinScrewTypes[6] = {
    0.0,   6.78,  6.58,
    15.98, 21.48, 0.0}; // <-- Length of the thicker part of the screw types.

///// Rod properties:
const float AliFITv8::sZShiftRod =
    -0.05; // <-- z shift of the rods. 0 = aligned with tht scintillators.
const int AliFITv8::sNumberOfRodTypes =
    4; // <-- Number of the different screw types.
const float AliFITv8::sDxMinRodTypes[4] = {
    0.366, 0.344, 0.344,
    0.344}; // <-- Width of the thinner part of the rod types.
const float AliFITv8::sDxMaxRodTypes[4] = {
    0.536, 0.566, 0.566,
    0.716}; // <-- Width of the thicker part of the rod types.
const float AliFITv8::sDyMinRodTypes[4] = {
    0.5, 0.8, 0.8, 0.8}; // <-- Height of the thinner part of the rod types.
const float AliFITv8::sDyMaxRodTypes[4] = {
    0.9, 1.2, 1.2, 1.2}; // <-- Height of the thicker part of the rod types.
const float AliFITv8::sDzMaxRodTypes[4] = {
    12.5, 12.5, 22.5, 27.7}; // <-- Length of the thinner part of the rod types.
const float AliFITv8::sDzMinRodTypes[4] = {
    7.45, 7.45, 17.45,
    22.65}; // <-- Length of the thicker part of the rod types.

const float AliFITv8::sCellRingRadii[10] = {4.01,  7.3,  12.9,
                                            21.25, 38.7, 72.115};
// BEGIN: Support structure constants
// offset found to potentially remove overlaps

const Float_t AliFITv8::sEps = 0.05;
// offset found to potentially remove overlaps
const Float_t AliFITv8::sXoffset = 0.3027999999999995;
const Float_t AliFITv8::sYoffset = -0.6570999999999998;

// frame 1 has a longer side horizontal
const Float_t AliFITv8::sFrame1X = 21.500;
const Float_t AliFITv8::sFrame1Y = 13.705;
const Float_t AliFITv8::sFrame1PosX = 7.9278 - sXoffset;
const Float_t AliFITv8::sFrame1PosY = 9.2454 - sYoffset;
const Float_t AliFITv8::sRect1X = 15;
const Float_t AliFITv8::sRect1Y = 1.33;
const Float_t AliFITv8::sRect2X = 2.9;
const Float_t AliFITv8::sRect2Y = 12.2;
const Float_t AliFITv8::sRect3X = 1.57;
const Float_t AliFITv8::sRect3Y = .175;
const Float_t AliFITv8::sRect4X = 5.65;
const Float_t AliFITv8::sRect4Y = 1.075;
// frame 2 has a longer side vertical
const Float_t AliFITv8::sFrame2X = 13.930;
const Float_t AliFITv8::sFrame2Y = 21.475;
const Float_t AliFITv8::sFrame2PosX = 10.1428 - sXoffset;
const Float_t AliFITv8::sFrame2PosY = -8.3446 - sYoffset;
const Float_t AliFITv8::sRect5X = 1.33;
const Float_t AliFITv8::sRect5Y = 12.1;

const Float_t AliFITv8::sRect6X = .83;
const Float_t AliFITv8::sRect6Y = 3.0;
const Float_t AliFITv8::sRect7X = 13.1;
const Float_t AliFITv8::sRect7Y = 3.0;
const Float_t AliFITv8::sRect8X = 1.425;
const Float_t AliFITv8::sRect8Y = 5.5;

// both frame boxes are the same height
const Float_t AliFITv8::sFrameZ = 5.700;
const Float_t AliFITv8::sMountZ = 1.5;

// PMT socket dimensions
const Float_t AliFITv8::sPmtSide = 5.950;
const Float_t AliFITv8::sPmtZ = 3.750;

// quartz radiator socket dimensions
const Float_t AliFITv8::sQuartzRadiatorSide = 5.350;
const Float_t AliFITv8::sQuartzRadiatorZ = 1.950;
//  const Float_t AliFITv8::sQuartzRadiatorSide= 5.40;
//  const Float_t AliFITv8::sQuartzRadiatorZ= 2.0;
// for the rounded socket corners
const Float_t AliFITv8::sCornerRadius = .300;

// bottom plates on the frame
const Float_t AliFITv8::sPlateSide = 6.000;
const Float_t AliFITv8::sBasicPlateZ = 0.200;
const Float_t AliFITv8::sCablePlateZ = 0.500;
const Float_t AliFITv8::sFiberHeadX = 0.675 * 2;
const Float_t AliFITv8::sFiberHeadY = 0.275 * 2;

// plate transformations
const Float_t AliFITv8::sOpticalFiberPlateZ = 0.35;
const Float_t AliFITv8::sPlateSpacing = 6.100;
const Float_t AliFITv8::sPlateDisplacementDeltaY = 1.33;
const Float_t AliFITv8::sPlateDisplacementX = sPlateSpacing + 0.3028;
const Float_t AliFITv8::sPlateDisplacementY =
    12.8789 - sPlateDisplacementDeltaY;
const Float_t AliFITv8::sPlateGroupZ = -sFrameZ / 2 - sOpticalFiberPlateZ;

// quartz & PMT socket transformations
const Float_t AliFITv8::sQuartzHeight = -sFrameZ / 2 + sQuartzRadiatorZ / 2;
const Float_t AliFITv8::sPmtHeight = sFrameZ / 2 - sPmtZ / 2;
const Float_t AliFITv8::sPmtCornerTubePos = -.15;
const Float_t AliFITv8::sPmtCornerPos = 2.825;
const Float_t AliFITv8::sEdgeCornerPos[2] = {-6.515, -.515};
const Float_t AliFITv8::sQuartzFrameOffsetX = -1.525;
const Float_t AliFITv8::sPos1X[3] = {sQuartzFrameOffsetX - sPlateSpacing,
                                     sQuartzFrameOffsetX,
                                     sQuartzFrameOffsetX + sPlateSpacing};
const Float_t AliFITv8::sPos1Y[4] = {3.6275, -2.4725, 2.2975, -3.8025};
const Float_t AliFITv8::sPos2X[4] = {3.69, -2.410, 2.360, -3.740};
const Float_t AliFITv8::sPos2Y[3] = {7.6875, 1.5875, -4.5125};
// END: Support structure constants
const Float_t AliFITv8::fPosModuleAx[24] = {
    -12.2, -6.1, 0,        6.1,        12.2,       -12.2,   -6.1,  0,
    6.1,   12.2, -13.3743, -7.2742999, 7.27429999, 13.3743, -12.2, -6.1,
    0,     6.1,  12.2,     -12.2,      -6.1,       0,       6.1,   12.2};

const Float_t AliFITv8::fPosModuleAy[24] = {
    12.2,  12.2, 13.53, 12.2,  12.2,  6.1,    6.1,   7.43,
    6.1,   6.1,  0,     0,     0,     0,      -6.1,  -6.1,
    -7.43, -6.1, -6.1,  -12.2, -12.2, -13.53, -12.2, -12.2};
const  float  AliFITv8::fstartC[3] = {20., 20, 5.5};
const  float  AliFITv8::fstartA[3] = {20., 20., 5};
const  float  AliFITv8::fInStart[3] = {2.9491, 2.9491, 2.5};

//-------------------------------------------------------------------------
AliFITv8::~AliFITv8() {
  // desctructor
  if (mRightTransformation)
    delete mRightTransformation;
  if (mLeftTransformation)
    delete mLeftTransformation;
}
//-------------------------------------------------------------------------

TGeoRotation *AliFITv8::createAndRegisterRot(const std::string &name,
                                             const double phi,
                                             const double theta,
                                             const double psi) const {
  TGeoRotation *rot = new TGeoRotation(name.c_str(), phi, theta, psi);
  rot->RegisterYourself();
  return rot;
}

TGeoTranslation *AliFITv8::createAndRegisterTrans(const std::string &name,
                                                  const double dx,
                                                  const double dy,
                                                  const double dz) const {
  TGeoTranslation *trans = new TGeoTranslation(name.c_str(), dx, dy, dz);
  trans->RegisterYourself();
  return trans;
}

void AliFITv8::initializeSectorTransformations() {

  // NB: This version of the code makes transformations for all 8 sectors;
  // places left and right half at once
  std::cout << " The reference to \"a\" and \"b\" can be understood with the "
               "CAD drawings of the detector."
            << std::endl;

  for (int iSector = 0; iSector < 8; ++iSector) {
    // iSector = 0 => first sector clockwise from the y-axis
    // iSector = 1 => the next sector in clockwise direction and so on
    TGeoRotation *trans = createAndRegisterRot(
        sDetectorName + sSectorName + std::to_string(iSector) + "TRANS", 0, 0,
        0);

    if (iSector == 2 || iSector == 3) { // "a" and "b" mirrors.
      trans->ReflectY(true);
    }
    if (iSector == 4 || iSector == 5) {
      trans->ReflectY(true);
      trans->ReflectX(true);
    }
    if (iSector == 6 || iSector == 7) {
      trans->ReflectX(true);
    }

    mSectorTrans.push_back(trans);
  }
}

void AliFITv8::initializeTransformations() {
  // Notes:
  // As of now: sDxHalvesSeparation = 0; sDyHalvesSeparation = 0;
  // sDzHalvesSeparation = 0;

  TGeoTranslation leftTranslation(-sDxHalvesSeparation / 2, 0, 0);
  TGeoRotation leftRotation;
  leftRotation.ReflectX(true);

  TGeoHMatrix leftTotalTransformation =
      leftTranslation * leftRotation; // this is matrix multiplication!!!

  mLeftTransformation = new TGeoHMatrix(leftTotalTransformation);
  mRightTransformation = new TGeoTranslation(
      sDxHalvesSeparation / 2, sDyHalvesSeparation, sDzHalvesSeparation);
}

void AliFITv8::SetVZEROGeo(TGeoVolume *alice) {

  const int kV0PlusColorSci = 616;
  // TGeoMedium *medV0PlusSci = gGeoManager->GetMedium("FIT_V0PlusSci");
  // TGeoMedium *medium = gGeoManager->GetMedium("FIT_V0PlusSci");

  Double_t Pi = TMath::Pi();
  Double_t Sin22p5 = TMath::Sin(Pi / 8.);
  Double_t Cos22p5 = TMath::Cos(Pi / 8.);
  Double_t v0PlusPts[16];

  // Defining the master volume for V0Plus
  TGeoVolume *v0Plus = new TGeoVolumeAssembly("V0LE");

  std::cout << "\n\n====>  DebugON:: Running AliFITv8 version...  "
            << std::endl;

  //---- Initalize Global variables (parameters/values etc.) ---->
  std::cout << "====>  DebugON:: Initalize Global/Local variables "
               "(parameters/values/transformations etc)..."
            << std::endl;
  initializeCellRingRadii();
  initializeTransformations(); // std::cout<<" DebugON::
                               // initializeTransformations(); "<<std::endl;
  initializeSectorTransformations(); // std::cout<<" DebugON::
                                     // initializeSectorTransformations();
                                     // "<<std::endl;

  std::cout << "\n====>  DebugON:: Adding Scintillator Sectors..." << std::endl;
  // Set the parameters:
  const Bool_t isSensitive = kTRUE;
  const float dZscint = sDzScintillator;
  const std::string celltype = sScintillatorName;
  const TGeoMedium *medV0PlusSci = gGeoManager->GetMedium("FIT_V0PlusSci");
  initializeCells(celltype, dZscint, medV0PlusSci, isSensitive);

  assembleScintSectors(v0Plus); // Add Scintilltor sector volumes (sensitive) to
                                // V0 Master Volume:

  const Bool_t isPlasticSensitive = kFALSE;
  const float dZplastic = sDzPlastic;
  const std::string plasticName = sPlasticName;
  const TGeoMedium *mediumPlastic = gGeoManager->GetMedium("FIT_V0Plastic");
  if (!mediumPlastic) {
    std::cout << "\n::::FATAL ERROR::::\nMedium for Plastic NOT "
                 "defined!!\n...Please check defination...\n"
              << std::endl;
    exit(1);
  }

  std::cout << "\n====>  DebugON:: Adding PlasticSectors..." << std::endl;

  initializeCells(plasticName, dZplastic, mediumPlastic, isPlasticSensitive);
  assemblePlasticSectors(v0Plus);

  initializeScrewAndRodRadii();
  initializeScrewAndRodPositionsAndDimensions();

  //---------- Add 30 Titatium screws --------->
  initializeScrewHoles();
  initializeScrewTypeMedium(); // this must be called before next function:
                               // "initializeScrews()"
  initializeScrews();
  std::cout << "====> DebugOn:: Adding Titanium Screws...  " << std::endl;
  assembleScrews(v0Plus);

  //------------ Add the Aluminium Rods -------->
  initializeRodHoles();
  initializeRodTypeMedium();
  initializeRods();
  std::cout << "====> DebugOn:: Adding Aluminium Rods... " << std::endl;
  assembleRods(v0Plus);

  //------------ Add the Opt Fiber ------------->
  initializeFiberVolumeRadii();
  initializeFiberMedium();
  initializeFibers();
  std::cout << "====> DebugOn:: Adding Optical Fibers... " << std::endl;
  assembleFibers(v0Plus);

  //------------ Add the Opt Fiber ------------->
  initializeMetalContainer();
  std::cout << "====> DebugOn:: Adding Metal Container... " << std::endl;
  assembleMetalContainer(v0Plus);

  //---------- Add V0+ volume to ALICE --------->
  alice->AddNode(v0Plus, 1,
                 new TGeoTranslation(
                     0, 0, fV0PlusZposition)); // Add Volume to ALICE volume:

  std::cout << "\n " << std::endl;

} ///// AliFITv8::SetVZEROGeo() /// new

void AliFITv8::initializeCellRingRadii() {
  mRAvgRing.assign(
      sCellRingRadii,
      sCellRingRadii + sNumberOfCellRings +
          1); // Index of mRAvgRing is NOT linked directly to any ring number
  for (int i = 0; i < mRAvgRing.size() - 1;
       ++i) { // Set real scintillator radii (reduced by paint thickness and
              // separation gap)
    mRMinScintillator.push_back(
        mRAvgRing[i] +
        sDrSeparationScint); // Now indices of mRMinScint and mRMaxScint
                             // correspond to the same ring
    mRMaxScintillator.push_back(
        mRAvgRing[i + 1] -
        sDrSeparationScint); // Now indices of mRMinScint and mRMaxScint
                             // correspond to the same ring
  }
}

void AliFITv8::initializeCells(const std::string &cellType,
                               const float zThickness, const TGeoMedium *medium,
                               const bool isSensitive) {

  // The reference to "a" and "b" cells can be understood with drawings of the
  // detector (see: O2::Geometry.cxx)

  // Sector separation gap shape
  const std::string secSepShapeName = "FV0_" + cellType + "SectorSeparation";
  new TGeoBBox(secSepShapeName.c_str(), mRMaxScintillator.back() + sEpsilon,
               sDrSeparationScint, zThickness / 2);

  // Sector separation gap rotations
  const std::string secSepRot45Name = "FV0_" + cellType + "SecSepRot45";
  const std::string secSepRot90Name = "FV0_" + cellType + "SecSepRot90";

  createAndRegisterRot(secSepRot45Name, 45, 0, 0);
  createAndRegisterRot(secSepRot90Name, 90, 0, 0);

  // Hole shapes
  const std::string holeSmallName = "FV0_" + cellType + "HoleSmall";
  const std::string holeLargeName = "FV0_" + cellType + "HoleLarge";
  const std::string holeSmallCutName = "FV0_" + cellType + "HoleSmallCut";
  const std::string holeLargeCutName = "FV0_" + cellType + "HoleLargeCut";

  new TGeoTube(holeSmallName.c_str(), 0.0000, sDrHoleSmallScintillator,
               zThickness / 2);
  new TGeoTube(holeLargeName.c_str(), 0.0000, sDrHoleLargeScintillator,
               zThickness / 2);
  new TGeoBBox(holeSmallCutName.c_str(), dxHoleCut, sDrHoleSmallScintillator,
               zThickness / 2);
  new TGeoBBox(holeLargeCutName.c_str(), dxHoleCut, sDrHoleLargeScintillator,
               zThickness / 2);

  // std::cout<<" ..Now going to loop::sNumberOfCellRings"<<std::endl;

  for (int ir = 0; ir < sNumberOfCellRings; ++ir) { // <== 5 rings
    // Radii without separation
    const float rMin = mRAvgRing[ir];
    const float rMax = mRAvgRing[ir + 1];
    const float rMid = rMin + (rMax - rMin) / 2;

    const std::string aCellName =
        createVolumeName(cellType + sCellName + "a", ir);
    const std::string aCellShapeName = aCellName + "Shape"; // Base shape

    if (ir == 0) { // <-- The cells in the innermost ring have a slightly
                   // shifted inner radius origin.
      // The innermost "a"-type cell
      const std::string a1CellShapeFullName = aCellShapeName + "Full";
      const std::string a1CellShapeHoleCutName = aCellShapeName + "HoleCut";
      const std::string a1CellShapeHoleCutTransName =
          a1CellShapeHoleCutName + "Trans";

      new TGeoTubeSeg(a1CellShapeFullName.c_str(), 0, mRMaxScintillator[ir],
                      zThickness / 2 - sEpsilon, 45, 90);
      new TGeoTube(a1CellShapeHoleCutName.c_str(), 0, mRMinScintillator[ir],
                   zThickness);

      createAndRegisterTrans(a1CellShapeHoleCutTransName,
                             sXShiftInnerRadiusScintillator, 0, 0);

      const std::string a1BoolFormula = a1CellShapeFullName + "-" +
                                        a1CellShapeHoleCutName + ":" +
                                        a1CellShapeHoleCutTransName;
      new TGeoCompositeShape(aCellShapeName.c_str(), a1BoolFormula.c_str());

    } else { // <-- The rest of the "a"-type cells
      new TGeoTubeSeg(aCellShapeName.c_str(), mRMinScintillator[ir],
                      mRMaxScintillator[ir], zThickness / 2, 45, 90);
    }
    // Translations for screw holes (inner = rmin, half-length = rmid, outer =
    // rmax) 1 = outer left 2 = inner left 3 = outer right 4 = inner right 5 =
    // outer middle 6 = inner middle 7 = half-length left 8 = half-length right
    // holes 1, 2 and 7 are slightly shifted along the rim of the cell

    const std::string aHole1TransName = aCellName + "Hole1Trans";
    const std::string aHole2TransName = aCellName + "Hole2Trans";
    const std::string aHole3TransName = aCellName + "Hole3Trans";
    const std::string aHole4TransName = aCellName + "Hole4Trans";
    const std::string aHole5TransName = aCellName + "Hole5Trans";
    const std::string aHole6TransName = aCellName + "Hole6Trans";
    const std::string aHole7TransName = aCellName + "Hole7Trans";
    const std::string aHole8TransName = aCellName + "Hole8Trans";
    const std::string aHole1CutTransName = aCellName + "Hole1CutTrans";
    const std::string aHole2CutTransName = aCellName + "Hole2CutTrans";
    const std::string aHole7CutTransName = aCellName + "Hole7CutTrans";

    createAndRegisterTrans(aHole1TransName, xHole,
                           cos(asin(xHole / rMax)) * rMax, 0);
    createAndRegisterTrans(aHole2TransName, xHole,
                           cos(asin(xHole / rMin)) * rMin, 0);
    createAndRegisterTrans(aHole3TransName, sin(45 * M_PI / 180) * rMax,
                           cos(45 * M_PI / 180) * rMax, 0);
    createAndRegisterTrans(aHole4TransName, sin(45 * M_PI / 180) * rMin,
                           cos(45 * M_PI / 180) * rMin, 0);
    createAndRegisterTrans(aHole5TransName, sin(22.5 * M_PI / 180) * rMax,
                           cos(22.5 * M_PI / 180) * rMax, 0);
    createAndRegisterTrans(aHole6TransName, sin(22.5 * M_PI / 180) * rMin,
                           cos(22.5 * M_PI / 180) * rMin, 0);
    createAndRegisterTrans(aHole7TransName, xHole,
                           cos(asin(xHole / rMid)) * rMid, 0);
    createAndRegisterTrans(aHole8TransName, sin(45 * M_PI / 180) * rMid,
                           cos(45 * M_PI / 180) * rMid, 0);
    createAndRegisterTrans(aHole1CutTransName, 0,
                           cos(asin(xHole / rMax)) * rMax, 0);
    createAndRegisterTrans(aHole2CutTransName, 0,
                           cos(asin(xHole / rMin)) * rMin, 0);
    createAndRegisterTrans(aHole7CutTransName, 0,
                           cos(asin(xHole / rMid)) * rMid, 0);

    // Composite shape
    std::string aBoolFormula = aCellShapeName;

    // sector separation
    aBoolFormula += "-" + secSepShapeName + ":" + secSepRot45Name;
    aBoolFormula += "-" + secSepShapeName + ":" + secSepRot90Name;

    // outer holes
    aBoolFormula += "-" + ((ir < 2) ? holeSmallName : holeLargeName) + ":" +
                    aHole1TransName;
    aBoolFormula += "-" + ((ir < 2) ? holeSmallCutName : holeLargeCutName) +
                    ":" + aHole1CutTransName;
    aBoolFormula += "-" + ((ir < 2) ? holeSmallName : holeLargeName) + ":" +
                    aHole3TransName;

    // inner holes
    if (ir > 0) {
      const std::string screwHoleName =
          (ir < 3) ? holeSmallName : holeLargeName;
      const std::string screwHoleCutName =
          (ir < 3) ? holeSmallCutName : holeLargeCutName;

      aBoolFormula += "-" + screwHoleName + ":" + aHole2TransName;
      aBoolFormula += "-" + screwHoleCutName + ":" + aHole2CutTransName;
      aBoolFormula += "-" + screwHoleName + ":" + aHole4TransName;
    }

    // outer middle hole
    if (ir > 1) {
      aBoolFormula += "-" + holeLargeName + ":" + aHole5TransName;
    }

    // inner middle hole
    if (ir > 2) {
      aBoolFormula += "-" + holeLargeName + ":" + aHole6TransName;
    }

    // half-length holes
    if (ir == 4) {
      aBoolFormula += "-" + holeLargeName + ":" + aHole7TransName;
      aBoolFormula += "-" + holeLargeCutName + ":" + aHole7CutTransName;
      aBoolFormula += "-" + holeLargeName + ":" + aHole8TransName;
    }

    /* old method:::::
    const std::string aCellCSName = aCellName + "CS";
    const TGeoVolume*         aCell   = new TGeoVolume(aCellName.c_str(),
    aCellCs, medium); // Cell volume
    */ //////

    // std::cout<<"AliFITv8::Info:: created Cell for ring = "<<ir<<"with name =
    // "<<aCellName.c_str()<<" CompositShape name =
    // "<<aCellCSName.c_str()<<std::endl;

    for (int iSector = 0; iSector < mSectorTrans.size(); ++iSector) {
      const std::string aCellCSName =
          aCellName + "CS" + Form("Sector%d", iSector);
      const TGeoCompositeShape *aCellCs =
          new TGeoCompositeShape(aCellCSName.c_str(), aBoolFormula.c_str());
      const std::string newaCellName = aCellName + Form("Sector%d", iSector);
      // std::cout<<"\n  Adding cell named: " << newaCellName.c_str() << " in
      // Ring: " <<ir<< ", Sector: "<<iSector+1<<std::endl;
      const TGeoVolume *aCell =
          new TGeoVolume(newaCellName.c_str(), aCellCs, medium); // Cell volume
    }

    for (int iSector = 4; iSector < 8; ++iSector) {
      const std::string aCellCSName =
          aCellName + "CS" + Form("Sector%d", iSector);
      const TGeoCompositeShape *aCellCs =
          new TGeoCompositeShape(aCellCSName.c_str(), aBoolFormula.c_str());
      const std::string newaCellName = aCellName + Form("Sector%d", iSector);
      // std::cout<<"\n  Adding cell named: " << newaCellName.c_str() << " in
      // Ring: " <<ir<< ", Sector: "<<iSector+1<<std::endl;
      const TGeoVolume *aCell =
          new TGeoVolume(newaCellName.c_str(), aCellCs, medium); // Cell volume
    }

    //------------- Now add b-type Cells --------------
    const std::string bCellName =
        createVolumeName(cellType + sCellName + "b", ir);
    const std::string bCellShapeName = bCellName + "Shape"; // Base shape

    if (ir == 0) { // <-- The cells in the innermost ring are slightly different
                   // than the rest
      // The innermost "b"-type cell
      const std::string b1CellShapeFullName = bCellShapeName + "Full";
      const std::string b1CellShapeHoleCutName = bCellShapeName + "Cut";
      const std::string b1CellShapeHoleCutTransName =
          b1CellShapeHoleCutName + "Trans";

      new TGeoTubeSeg(b1CellShapeFullName.c_str(), 0, mRMaxScintillator[ir],
                      zThickness / 2 - sEpsilon, 0, 45);
      new TGeoTube(b1CellShapeHoleCutName.c_str(), 0, mRMinScintillator[ir],
                   zThickness);

      createAndRegisterTrans(b1CellShapeHoleCutTransName,
                             sXShiftInnerRadiusScintillator, 0, 0);

      const std::string b1BoolFormula = b1CellShapeFullName + "-" +
                                        b1CellShapeHoleCutName + ":" +
                                        b1CellShapeHoleCutTransName;
      new TGeoCompositeShape(bCellShapeName.c_str(), b1BoolFormula.c_str());
    } else { // <--- The rest of the "b"-type cells
      new TGeoTubeSeg(bCellShapeName.c_str(), mRMinScintillator[ir],
                      mRMaxScintillator[ir], zThickness / 2, 0, 45);
    }

    // Translations for holes
    //
    // 1 = outer left
    // 2 = inner left
    // 3 = outer right
    // 4 = inner right
    // 5 = outer middle
    // 6 = inner middle
    // 7 = half-lenght left
    // 8 = half-length right

    const std::string bHole1TransName = bCellName + "Hole1Trans";
    const std::string bHole2TransName = bCellName + "Hole2Trans";
    const std::string bHole3TransName = bCellName + "Hole3Trans";
    const std::string bHole4TransName = bCellName + "Hole4Trans";
    const std::string bHole5TransName = bCellName + "Hole5Trans";
    const std::string bHole6TransName = bCellName + "Hole6Trans";
    const std::string bHole7TransName = bCellName + "Hole7Trans";
    const std::string bHole8TransName = bCellName + "Hole8Trans";

    createAndRegisterTrans(bHole1TransName, sin(45 * M_PI / 180) * rMax,
                           cos(45 * M_PI / 180) * rMax, 0);
    createAndRegisterTrans(bHole2TransName, sin(45 * M_PI / 180) * rMin,
                           cos(45 * M_PI / 180) * rMin, 0);
    createAndRegisterTrans(bHole3TransName, rMax, 0, 0);
    createAndRegisterTrans(bHole4TransName, rMin, 0, 0);
    createAndRegisterTrans(bHole5TransName, cos(22.5 * M_PI / 180) * rMax,
                           sin(22.5 * M_PI / 180) * rMax, 0);
    createAndRegisterTrans(bHole6TransName, cos(22.5 * M_PI / 180) * rMin,
                           sin(22.5 * M_PI / 180) * rMin, 0);
    createAndRegisterTrans(bHole7TransName, sin(45 * M_PI / 180) * rMid,
                           cos(45 * M_PI / 180) * rMid, 0);
    createAndRegisterTrans(bHole8TransName, rMid, 0, 0);

    // Composite shape
    std::string bBoolFormula = bCellShapeName;

    // sector separation
    bBoolFormula += "-" + secSepShapeName;
    bBoolFormula += "-" + secSepShapeName + ":" + secSepRot45Name;

    // outer holes
    bBoolFormula += "-" + ((ir < 2) ? holeSmallName : holeLargeName) + ":" +
                    bHole1TransName;
    bBoolFormula += "-" + ((ir < 2) ? holeSmallName : holeLargeName) + ":" +
                    bHole3TransName;

    // inner holes
    if (ir > 0) {
      const std::string holeName = (ir < 3) ? holeSmallName : holeLargeName;

      bBoolFormula += "-" + holeName + ":" + bHole2TransName;
      bBoolFormula += "-" + holeName + ":" + bHole4TransName;
    }

    // outer middle hole
    if (ir > 1) {
      bBoolFormula += "-" + holeLargeName + ":" + bHole5TransName;
    }

    // inner middle hole
    if (ir > 2) {
      bBoolFormula += "-" + holeLargeName + ":" + bHole6TransName;
    }

    // half-lenght holes
    if (ir == 4) {
      bBoolFormula += "-" + holeLargeName + ":" + bHole7TransName;
      bBoolFormula += "-" + holeLargeName + ":" + bHole8TransName;
    }

    // const std::string bCellCSName = bCellName + "CS";
    // const TGeoCompositeShape* bCellCs = new
    // TGeoCompositeShape(bCellCSName.c_str(), bBoolFormula.c_str()); const
    // TGeoVolume*         bCell   = new TGeoVolume(bCellName.c_str(), bCellCs,
    // medium); // Cell volume

    for (int iSector = 0; iSector < mSectorTrans.size(); ++iSector) {
      const std::string bCellCSName =
          bCellName + "CS" + Form("Sector%d", iSector);
      const TGeoCompositeShape *bCellCs =
          new TGeoCompositeShape(bCellCSName.c_str(), bBoolFormula.c_str());
      const std::string newbCellName = bCellName + Form("Sector%d", iSector);
      // std::cout<<"\n  Adding b-cell named: " << newbCellName.c_str() << " in
      // Ring: " <<ir<< ", Sector: "<<iSector+1<<std::endl;
      const TGeoVolume *bCell =
          new TGeoVolume(newbCellName.c_str(), bCellCs, medium); // Cell volume
    }

    for (int iSector = 4; iSector < 8; ++iSector) {
      const std::string bCellCSName =
          bCellName + "CS" + Form("Sector%d", iSector);
      const TGeoCompositeShape *bCellCs =
          new TGeoCompositeShape(bCellCSName.c_str(), bBoolFormula.c_str());
      const std::string newbCellName = bCellName + Form("Sector%d", iSector);
      // std::cout<<"\n  Adding b-cell named: " << newbCellName.c_str() << " in
      // Ring: " <<ir<< ", Sector: "<<iSector+1<<std::endl;
      const TGeoVolume *bCell =
          new TGeoVolume(newbCellName.c_str(), bCellCs, medium); // Cell volume
    }

  } // sNumberOfCellRings

} //::inializeCells

/// Debug help:
/// https://www.geeksforgeeks.org/core-dump-segmentation-fault-c-cpp/

void AliFITv8::assemblePlasticSectors(TGeoVolume *vFV0) const {
  TGeoVolumeAssembly *sectors = buildSectorAssembly(sPlasticName, kFALSE);

  // Move the plastic cells to back of the scintillator cells:
  TGeoHMatrix *leftTrans = new TGeoHMatrix(*mLeftTransformation);
  leftTrans->SetDz(leftTrans->GetTranslation()[2] + sZPlastic);

  TGeoMatrix *rightTrans = new TGeoHMatrix(*mRightTransformation);
  rightTrans->SetDz(rightTrans->GetTranslation()[2] + sZPlastic);

  vFV0->AddNode(
      sectors, 0,
      rightTrans); // I have added All sectors at once. No need for leftTrans
  // vFV0->AddNode(sectors, 1, leftTrans);
}

void AliFITv8::assembleScintSectors(TGeoVolume *vFV0) const {
  TGeoVolumeAssembly *sectors = buildSectorAssembly(sScintillatorName, kTRUE);

  vFV0->AddNode(sectors, 0,
                mRightTransformation); // I have added All sectors at once. No
                                       // need for mLeftTransformation
  // vFV0->AddNode(sectors, 1, mLeftTransformation);
}

TGeoVolumeAssembly *AliFITv8::buildSectorAssembly(const std::string &cellName,
                                                  Bool_t isSensitive) const {
  TGeoVolumeAssembly *assembly =
      new TGeoVolumeAssembly(createVolumeName(cellName).c_str());

  for (int iSector = 0; iSector < 8; ++iSector) {

    std::cout << " DebugOn::buildSectorAssembly() Constructing Sector "
              << iSector << "..." << std::endl;
    TGeoVolumeAssembly *sector = buildSector(cellName, iSector, isSensitive);

    const TGeoRotation *rot =
        (TGeoRotation *)mSectorTrans[iSector]->GetRotationMatrix();
    assembly->AddNode(sector, iSector, mSectorTrans[iSector]);

    /*if(iSector<4){
      const TGeoRotation *rot = (TGeoRotation *)
    mSectorTrans[iSector]->GetRotationMatrix(); assembly->AddNode(sector,
    iSector, mSectorTrans[iSector]); std::cout<<", Adding "<<cellName.c_str()<<"
    cells in iSector "<<iSector+1<<" phi = "<< rot->GetPhiRotation()<<std::endl;
    }
    else{
      //const TGeoRotation *rot = (TGeoRotation *)
    mSectorTrans[8-iSector]->GetRotationMatrix(); assembly->AddNode(sector,
    iSector, mSectorTrans[iSector]); std::cout<<", Adding "<<cellName.c_str()<<"
    cells in iSector "<<iSector+1<<std::endl;
    } */
  }

  return assembly;
}

TGeoVolumeAssembly *AliFITv8::buildSector(const std::string &cellType,
                                          const int iSector,
                                          Bool_t isSensitive) const {

  const char sCellTypes[8] = {'a', 'b', 'b', 'a', 'a', 'b', 'b', 'a'};

  Color_t kBorderColor = 1;
  Color_t kMediaColor = 1;

  std::string baseCellname = sScintillatorName; // sensitive

  if (isSensitive) {
    kBorderColor = kYellow + 1; // Yellow for sensitive;
    kMediaColor = kYellow - 9;
  } else {
    baseCellname = sPlasticName; // non-sensitive;
    kBorderColor = kMagenta + 2; // Magenta for non-sensitive;
    kMediaColor = kMagenta - 9;
  }

  TGeoVolumeAssembly *sector = new TGeoVolumeAssembly(
      createVolumeName(cellType + sSectorName, iSector).c_str());

  for (int i = 0; i < sNumberOfCellRings; ++i) {

    const std::string cellName =
        createVolumeName(cellType + sCellName + sCellTypes[iSector], i);
    const std::string NewcellName = cellName + Form("Sector%d", iSector);

    TGeoTranslation leftTranslation(-sDxHalvesSeparation / 2, 0, 0);
    TGeoRotation leftRotationUp; ////No need to rotate here. It is done in
                                 /// AliFITv8::buildSectorAssembly()
    TGeoRotation leftRotationDown;

    TGeoHMatrix leftTotTransCellUp = leftTranslation * leftRotationUp;
    TGeoHMatrix leftTotTransCellDown = leftTranslation * leftRotationDown;

    TGeoHMatrix *leftTransCellUp = new TGeoHMatrix(leftTotTransCellUp);
    TGeoHMatrix *leftTransCellDown = new TGeoHMatrix(leftTotTransCellDown);

    TGeoVolume *cell = gGeoManager->GetVolume(NewcellName.c_str());

    if (!cell) {
      std::cout << " \n ********** ERROR!!!! ********* \n in "
                   "AliFITv8::buildSector(): \n Couldn't find cell named: "
                << std::endl;
      std::cout << NewcellName.c_str() << std::endl;
      std::cout << "\n Please Check cell Volume Definitions !!! \n (email "
                   "mhaque@cern.ch) \n "
                << std::endl;
      exit(1);
    } else {
      cell->SetFillColor(kMediaColor);
      cell->SetLineColor(kBorderColor);

      if (iSector < 4) {
        sector->AddNode(cell, 5 * iSector + i,
                        new TGeoTranslation(sXScintillator, 0, 0));
      } else if (iSector >= 4 && iSector < 6) {
        sector->AddNode(cell, 5 * iSector + i, leftTransCellUp);
      } else if (iSector >= 6) {
        sector->AddNode(cell, 5 * iSector + i, leftTransCellDown);
      }
    } //// if cell found

    if (isSensitive) {
      std::cout << " SCINTILATOR cell " << NewcellName.c_str()
                << " is added in Ring #" << i + 1 << std::endl;
    }
  } //// Ring# loop

  return sector;
}

/////=================== Sectors Done ====================== ////

const std::string AliFITv8::createVolumeName(const std::string &volumeType,
                                             const int number) const {
  return sDetectorName + volumeType +
         ((number >= 0) ? std::to_string(number) : "");
}

///--------------- Function for Adding Screws -----------------

void AliFITv8::initializeScrewAndRodRadii() {
  mRScrewAndRod.push_back(mRAvgRing[1]);
  mRScrewAndRod.push_back(mRAvgRing[2]);
  mRScrewAndRod.push_back(mRAvgRing[3]);
  mRScrewAndRod.push_back(mRAvgRing[4]);
  mRScrewAndRod.push_back((mRAvgRing[4] + mRAvgRing[5]) /
                          2); // why this is done?? : Rihan
  mRScrewAndRod.push_back(mRAvgRing[5]);
}

void AliFITv8::addScrewProperties(const int screwTypeID, const int iRing,
                                  const float phi) {
  // std::cout<<"Info:: called addScrewProperties(...,...,) "<<std::endl;
  float r = mRScrewAndRod[iRing];
  mScrewTypeIDs.push_back(screwTypeID);
  mScrewPos.push_back(
      std::vector<float>{cosf(phi * M_PI / 180) * r, sinf(phi * M_PI / 180) * r,
                         sZScintillator - sDzScintillator / 2 + sZShiftScrew +
                             sDzMaxScrewTypes[screwTypeID] / 2});
  mDrMinScrews.push_back(sDrMinScrewTypes[screwTypeID]);
  mDrMaxScrews.push_back(sDrMaxScrewTypes[screwTypeID]);
  mDzMaxScrews.push_back(sDzMaxScrewTypes[screwTypeID]);
  mDzMinScrews.push_back(sDzMinScrewTypes[screwTypeID]);
}

TGeoShape *AliFITv8::createScrewShape(const std::string &shapeName,
                                      const int screwTypeID,
                                      const float xEpsilon,
                                      const float yEpsilon,
                                      const float zEpsilon) const {
  // std::cout<<"Info:: called createScrewShape(...,...,) "<<std::endl;
  const float xyEpsilon =
      (fabs(xEpsilon) > fabs(yEpsilon)) ? xEpsilon : yEpsilon;
  const float dzMax = sDzMaxScrewTypes[screwTypeID] / 2 + zEpsilon;
  const float dzMin = sDzMinScrewTypes[screwTypeID] / 2 + zEpsilon;

  const std::string thinPartName = shapeName + "Thin";
  const std::string thickPartName = shapeName + "Thick";
  const std::string thickPartTransName = thickPartName + "Trans";

  new TGeoTube(thinPartName.c_str(), 0,
               sDrMinScrewTypes[screwTypeID] + xyEpsilon, dzMax);
  new TGeoTube(thickPartName.c_str(), 0,
               sDrMaxScrewTypes[screwTypeID] + xyEpsilon, dzMin);

  createAndRegisterTrans(thickPartTransName, 0, 0,
                         -dzMax - sZShiftScrew + sDzScintillator + sDzPlastic +
                             dzMin);

  std::string boolFormula = thinPartName;
  boolFormula += "+" + thickPartName + ":" + thickPartTransName;

  TGeoCompositeShape *screwShape =
      new TGeoCompositeShape(shapeName.c_str(), boolFormula.c_str());
  return screwShape;
}

void AliFITv8::initializeScrewTypeMedium() {
  // There are no further checks if the medium is actually found
  TGeoMedium *medium = gGeoManager->GetMedium("FIT_Titanium");
  for (int i = 0; i < sNumberOfScrewTypes; ++i) {
    mMediumScrewTypes.push_back(medium);
  }
}

void AliFITv8::initializeScrews() {
  for (int i = 0; i < sNumberOfScrewTypes; ++i) {
    const std::string screwName = createVolumeName(sScrewName, i);
    const TGeoShape *screwShape =
        createScrewShape(screwName + "Shape", i, 0, 0, 0);

    // If modifying materials, make sure the appropriate initialization is done
    // in initializeXxxMedium() methods
    new TGeoVolume(screwName.c_str(), screwShape, mMediumScrewTypes[i]);
  }
}

void AliFITv8::initializeScrewHoles() {
  // std::cout<<"Info:: called initializeScrewHoles() "<<std::endl;
  std::string boolFormula = "";
  for (int i = 0; i < mScrewPos.size(); ++i) {
    std::string holeShapeName =
        sDetectorName + sScrewName + "HOLE" + std::to_string(i);
    std::string holeTransName =
        sDetectorName + sScrewName + "HOLETRANS" + std::to_string(i);
    // std::cout<<" adding Screw hole named: "<<holeShapeName.data()<<std::endl;
    createScrewShape(holeShapeName, mScrewTypeIDs[i], sEpsilon, sEpsilon);
    createAndRegisterTrans(holeTransName, mScrewPos[i][0] + sXShiftScrews,
                           mScrewPos[i][1], mScrewPos[i][2]);
    boolFormula += ((i != 0) ? "+" : "") + holeShapeName + ":" + holeTransName;
  }
  new TGeoCompositeShape(sScrewHolesCSName.c_str(), boolFormula.c_str());
}

void AliFITv8::assembleScrews(TGeoVolume *vFV0) const {
  // std::cout<<"Info:: called assembleScrews() "<<std::endl;
  TGeoVolumeAssembly *screws =
      new TGeoVolumeAssembly(createVolumeName("SCREWS").c_str());
  Color_t kMediaColor = kCyan;

  // If modifying something here, make sure screw initialization is OK
  for (int i = 0; i < mScrewPos.size(); ++i) {
    TString screwname = createVolumeName(sScrewName, mScrewTypeIDs[i]).c_str();
    // std::cout<<"\n Info:: Adding Screw no. "<<i<<" of type:
    // "<<mScrewTypeIDs[i]<<" name = "<<screwname.Data()<<std::endl;
    TGeoVolume *screw = gGeoManager->GetVolume(screwname);
    screw->SetLineColor(kMediaColor);
    screw->SetFillColor(kMediaColor);

    if (!screw) {
      // LOG(WARNING) << "FV0 Geometry::assembleScrews(): Screw no. " << i << "
      // not found";
      std::cout << " **** Error ****** \n FV0 Geometry::assembleScrews(): "
                   "Volume of screw no. "
                << i << " not found!\n"
                << std::endl;
    } else {
      // std::cout<<"Info:: Adding Screw no. "<<i<<" at pos = "<<
      // mScrewPos[i][1]<<","<<mScrewPos[i][2]<<std::endl;
      screws->AddNode(screw, i,
                      new TGeoTranslation(mScrewPos[i][0] + sXShiftScrews,
                                          mScrewPos[i][1], mScrewPos[i][2]));
    }
  }

  // vFV0->AddNode(screws, 1, mLeftTransformation);  // This causes overlaps;
  // all 8 sectors are placed at once
  vFV0->AddNode(screws, 2, mRightTransformation);
}

///////// Functions for Adding Rods: ///////////////

void AliFITv8::addRodProperties(const int rodTypeID, const int iRing) {
  mRodTypeIDs.push_back(rodTypeID);
  mRodPos.push_back(
      std::vector<float>{sDxMinRodTypes[rodTypeID] / 2, mRScrewAndRod[iRing],
                         sZScintillator - sDzScintillator / 2 + sZShiftRod +
                             sDzMaxRodTypes[rodTypeID] / 2});
  mDxMinRods.push_back(sDxMinRodTypes[rodTypeID]);
  mDzMaxRods.push_back(sDxMaxRodTypes[rodTypeID]);
  mDyMinRods.push_back(sDyMinRodTypes[rodTypeID]);
  mDyMaxRods.push_back(sDyMaxRodTypes[rodTypeID]);
  mDzMaxRods.push_back(sDzMaxRodTypes[rodTypeID]);
  mDzMinRods.push_back(sDzMinRodTypes[rodTypeID]);

  mRodTypeIDs.push_back(rodTypeID);
  mRodPos.push_back(
      std::vector<float>{sDxMinRodTypes[rodTypeID] / 2, -mRScrewAndRod[iRing],
                         sZScintillator - sDzScintillator / 2 + sZShiftRod +
                             sDzMaxRodTypes[rodTypeID] / 2});
  mDxMinRods.push_back(sDxMinRodTypes[rodTypeID]);
  mDzMaxRods.push_back(sDxMaxRodTypes[rodTypeID]);
  mDyMinRods.push_back(sDyMinRodTypes[rodTypeID]);
  mDyMaxRods.push_back(sDyMaxRodTypes[rodTypeID]);
  mDzMaxRods.push_back(sDzMaxRodTypes[rodTypeID]);
  mDzMinRods.push_back(sDzMinRodTypes[rodTypeID]);
}

void AliFITv8::initializeScrewAndRodPositionsAndDimensions() {
  for (int iRing = 0; iRing < mRScrewAndRod.size(); ++iRing) {
    switch (iRing) {
    case 0:
      addRodProperties(0, iRing);
      for (float phi = 45; phi >= -45; phi -= 45) {
        addScrewProperties(0, iRing, phi);
      }
      break;
    case 1:
      addRodProperties(0, iRing);
      for (float phi = 45; phi >= -45; phi -= 45) {
        addScrewProperties(1, iRing, phi);
      }
      break;
    case 2:
      addRodProperties(1, iRing);
      for (float phi = 67.5; phi >= -67.5; phi -= 22.5) {
        addScrewProperties(2, iRing, phi);
      }
      break;
    case 3:
      addRodProperties(2, iRing);
      for (float phi = 67.5; phi >= -67.5; phi -= 22.5) {
        addScrewProperties(3, iRing, phi);
      }
      break;
    case 4:
      addRodProperties(3, iRing);
      for (float phi = 45; phi >= -45; phi -= 45) {
        addScrewProperties(4, iRing, phi);
      }
      break;
    case 5:
      addRodProperties(3, iRing);
      for (float phi = 67.5; phi >= -67.5; phi -= 22.5) {
        addScrewProperties(5, iRing, phi);
      }
      break;
    default:
      break;
    }
  }
}

TGeoShape *AliFITv8::createRodShape(const std::string &shapeName,
                                    const int rodTypeID, const float xEpsilon,
                                    const float yEpsilon,
                                    const float zEpsilon) const {
  // std::cout<<" Debug:: Creating Rod Shape: "<<std::endl;
  const float dxMin = sDxMinRodTypes[rodTypeID] / 2 + xEpsilon;
  const float dxMax = sDxMaxRodTypes[rodTypeID] / 2 + xEpsilon;
  const float dyMin = sDyMinRodTypes[rodTypeID] / 2 + yEpsilon;
  const float dyMax = sDyMaxRodTypes[rodTypeID] / 2 + yEpsilon;
  const float dzMax = sDzMaxRodTypes[rodTypeID] / 2 + zEpsilon;
  const float dzMin = sDzMinRodTypes[rodTypeID] / 2 + zEpsilon;

  const std::string thinPartName = shapeName + "Thin";
  const std::string thickPartName = shapeName + "Thick";
  const std::string thickPartTransName = thickPartName + "Trans";

  new TGeoBBox(thinPartName.c_str(), dxMin, dyMin, dzMax);
  new TGeoBBox(thickPartName.c_str(), dxMax, dyMax, dzMin);
  createAndRegisterTrans(thickPartTransName, dxMax - dxMin, 0,
                         -dzMax - sZShiftRod + sDzScintillator + sDzPlastic +
                             dzMin);

  std::string boolFormula = thinPartName;
  boolFormula += "+" + thickPartName + ":" + thickPartTransName;

  TGeoCompositeShape *rodShape =
      new TGeoCompositeShape(shapeName.c_str(), boolFormula.c_str());
  return rodShape;
}

void AliFITv8::initializeRodHoles() {
  std::string boolFormula = "";

  for (int i = 0; i < mRodPos.size(); ++i) {
    std::string holeShapeName =
        sDetectorName + sRodName + "HOLE" + std::to_string(i);
    std::string holeTransName =
        sDetectorName + sRodName + "HOLETRANS" + std::to_string(i);
    // std::cout<<" Debug:: adding rod hole named:
    // "<<holeShapeName.data()<<std::endl;
    createRodShape(holeShapeName, mRodTypeIDs[i], sEpsilon, sEpsilon);
    createAndRegisterTrans(holeTransName, mRodPos[i][0] + sXShiftScrews,
                           mRodPos[i][1], mRodPos[i][2]);

    boolFormula += ((i != 0) ? "+" : "") + holeShapeName + ":" + holeTransName;
  }

  new TGeoCompositeShape(sRodHolesCSName.c_str(), boolFormula.c_str());
}

void AliFITv8::initializeRodTypeMedium() {

  // There are no further checks if the medium is actually found
  TGeoMedium *medium = gGeoManager->GetMedium("FIT_Aluminium$");
  if (!medium) {
    std::cout << "\n Debug:: *********** Error ********** \n FIT_Aluminium not "
                 "found...!!!"
              << std::endl;
  }
  for (int i = 0; i < sNumberOfRodTypes; ++i) {
    mMediumRodTypes.push_back(medium);
  }
}
void AliFITv8::initializeRods() {
  // std::cout<<" Debug::initializeRods() called..."<<std::endl;
  for (int i = 0; i < sNumberOfRodTypes; ++i) {
    const std::string rodName = createVolumeName(sRodName, i);
    const TGeoShape *rodShape =
        createRodShape(rodName + "Shape", i, -sEpsilon, -sEpsilon);
    // std::cout<<" Debug::initializeRods() creating rod named:
    // "<<createVolumeName(sRodName, i).data()<<std::endl;

    // If modifying materials, make sure the appropriate initialization is done
    // in initializeXxxMedium() methods
    new TGeoVolume(rodName.c_str(), rodShape, mMediumRodTypes[i]);
  }
}

void AliFITv8::assembleRods(TGeoVolume *vFV0) const {
  TGeoVolumeAssembly *rods =
      new TGeoVolumeAssembly(createVolumeName("RODS").c_str());
  // If modifying something here, make sure rod initialization is OK
  Color_t kMediaColor = kTeal;

  for (int i = 0; i < mRodPos.size(); ++i) {
    TGeoVolume *rod = gGeoManager->GetVolume(
        createVolumeName(sRodName, mRodTypeIDs[i]).c_str());
    rod->SetFillColor(kMediaColor);
    rod->SetLineColor(kMediaColor);

    if (!rod) {
      // LOG(INFO) << "FV0 Geometry::assembleRods(): Rod no. " << i << " not
      // found";
      std::cout << "\n *** ERROR ***\nFV0 Geometry::assembleRods(): Rod no. "
                << i << " not found !!" << std::endl;
    } else {
      rods->AddNode(rod, i,
                    new TGeoTranslation(mRodPos[i][0] + sXShiftScrews,
                                        mRodPos[i][1], mRodPos[i][2]));
    }
  }
  // vFV0->AddNode(rods, 1, mLeftTransformation);  // This causes overlaps; all
  // 8 sectors are placed at once
  vFV0->AddNode(rods, 2, mRightTransformation);
}

/////// Add Fibre Functions ///////////

void AliFITv8::initializeFiberVolumeRadii() {
  // std::cout<<" ====>  Debug:: called  AliFITv8::initializeFiberVolumeRadii()
  // "<<std::endl;
  mRMinFiber.push_back(0);
  mRMinFiber.push_back(sDrMinContainerCone + sEpsilon);
  mRMinFiber.push_back(sDrMinContainerFront + sEpsilon);

  mRMaxFiber.push_back(sDrMinContainerCone);
  mRMaxFiber.push_back(sDrMinContainerFront);
  mRMaxFiber.push_back(mRMaxScintillator.back());
}
void AliFITv8::initializeFiberMedium() {
  // std::cout<<" ====>  Debug:: called  AliFITv8::initializeFiberMedium()
  // "<<std::endl;
  mMediumFiber.push_back(gGeoManager->GetMedium("FIT_FiberInner"));
  mMediumFiber.push_back(gGeoManager->GetMedium("FIT_FiberMiddle"));
  mMediumFiber.push_back(gGeoManager->GetMedium("FIT_FiberOuter"));
}

void AliFITv8::initializeFibers() {
  // std::cout<<" ====>  Debug:: called  AliFITv8::initializeFibers()
  // "<<std::endl;
  // depth of the fiber volumes
  const float dzFibers = sDzContainer - sDzContainerBack - sDzContainerFront -
                         sDzScintillator - sDzPlastic - 2 * sEpsilon;

  const std::string fiberName = "FV0_Fibers"; // No volume with this name

  const std::string fiberSepCutName = fiberName + "SepCut";
  const std::string fiberConeCutName = fiberName + "ConeCut";
  const std::string fiberHoleCutName = fiberName + "HoleCut";

  const std::string fiberTransName = fiberName + "Trans";
  const std::string fiberConeCutTransName = fiberConeCutName + "Trans";
  const std::string fiberHoleCutTransName = fiberHoleCutName + "Trans";

  new TGeoBBox(fiberSepCutName.c_str(), sDrSeparationScint,
               mRMaxFiber.back() + sEpsilon, dzFibers / 2 + sEpsilon);
  new TGeoConeSeg(fiberConeCutName.c_str(), sDzContainerCone / 2 + sEpsilon, 0,
                  sDrMinContainerCone + sXYThicknessContainerCone + sEpsilon, 0,
                  sDrMinContainerFront + sEpsilon, -90, 90);
  new TGeoTube(fiberHoleCutName.c_str(), 0, mRMinScintillator.front(),
               dzFibers / 2 + sEpsilon);

  createAndRegisterTrans(fiberTransName, sXScintillator, 0, sZFiber);
  createAndRegisterTrans(fiberConeCutTransName, sXScintillator, 0, sZCone);
  createAndRegisterTrans(fiberHoleCutTransName,
                         sXScintillator + sXShiftInnerRadiusScintillator, 0,
                         sZFiber);

  for (int i = 0; i < mRMinFiber.size(); ++i) {
    const std::string fiberShapeName = fiberName + std::to_string(i + 1);
    new TGeoTubeSeg(fiberShapeName.c_str(), mRMinFiber[i],
                    mRMaxFiber[i] - sEpsilon, dzFibers / 2, -90, 90);

    // Composite shape
    std::string boolFormula = "";
    boolFormula += fiberShapeName + ":" + fiberTransName;
    boolFormula += "-" + fiberSepCutName + ":" + fiberTransName;
    boolFormula += "-" + fiberConeCutName + ":" + fiberConeCutTransName;

    if (i == 0) {
      // Cut out the hole in the innermost fiber volume
      boolFormula += "-" + fiberHoleCutName + ":" + fiberHoleCutTransName;
    }

    // Remove holes for screws and rods
    boolFormula += "-" + sScrewHolesCSName;
    boolFormula += "-" + sRodHolesCSName;

    const TGeoCompositeShape *fiberCS = new TGeoCompositeShape(
        (fiberShapeName + "CS").c_str(), boolFormula.c_str());

    // If modifying materials, make sure the appropriate initialization is done
    // in initializeXxxMedium() methods
    new TGeoVolume(createVolumeName(sFiberName, i).c_str(), fiberCS,
                   mMediumFiber[i]);
  }
}

void AliFITv8::assembleFibers(TGeoVolume *vFV0) const {
  TGeoVolumeAssembly *fibers =
      new TGeoVolumeAssembly(createVolumeName("FIBERS").c_str());
  Color_t kBorderColor = kBlue; // Magenta for sensitive;
  Color_t kMediaColor = kAzure - 3;
  for (int i = 0; i < mRMinFiber.size(); ++i) {
    TGeoVolume *fiber =
        gGeoManager->GetVolume(createVolumeName(sFiberName, i).c_str());
    if (!fiber) {
      // LOG(WARNING) << "FV0 Geometry::assembleFibers(): Fiber volume no. " <<
      // i << " not found.";
      std::cout << "\n ***** Error *******\n FV0  Geometry::assembleFibers(): "
                   "Fiber volume no. "
                << i << " not found." << std::endl;
    }
    fiber->SetFillColor(kMediaColor);
    fiber->SetLineColor(kMediaColor);
    fibers->AddNode(fiber, i);
  }

  vFV0->AddNode(fibers, 1, mLeftTransformation);
  vFV0->AddNode(fibers, 2, mRightTransformation);
}

/////////////////// Finally the Metal Container functions //////////////

void AliFITv8::initializeMetalContainer() {
  // The metal container is constructed starting from the backplate. The
  // backplate is positioned first, relative to the scintillator cells. The rest
  // of the container parts are positioned relative to the backplate. Caution:
  // some position variables are in global coords, and some are relative to some
  // other part of the container

  // Backplate
  const std::string backPlateName = "FV0_BackPlate"; // the full backplate
  const std::string backPlateStandName =
      backPlateName + "Stand"; // the stand part of the backplate
  const std::string backPlateHoleName =
      backPlateName + "Hole"; // the hole in the middle of the backplate
  const std::string backPlateHoleCutName =
      backPlateHoleName + "Cut"; // extension of the hole
  const std::string backPlateStandTransName =
      backPlateStandName + "Trans"; // shift of the backplate stand
  const std::string backPlateHoleTransName =
      backPlateHoleName + "Trans"; // shift of the backplate inner radius

  new TGeoTubeSeg(backPlateName.c_str(), 0, sDrMaxContainerBack,
                  sDzContainerBack / 2, -90, 90);
  new TGeoBBox(backPlateStandName.c_str(), sDxContainerStand / 2,
               (sDrMaxContainerBack + sDyContainerStand) / 2,
               sDzContainerBack / 2);
  new TGeoTubeSeg(backPlateHoleName.c_str(), 0, sDrContainerHole,
                  sDzContainerBack / 2, -90, 90);
  new TGeoBBox(backPlateHoleCutName.c_str(), -sXShiftContainerHole,
               sDrContainerHole, sDzContainerBack);

  createAndRegisterTrans(backPlateStandTransName, sDxContainerStand / 2,
                         -(sDrMaxContainerBack + sDyContainerStand) / 2, 0);
  createAndRegisterTrans(backPlateHoleTransName, sXShiftContainerHole, 0, 0);
  // Backplate composite shape
  std::string backPlateBoolFormula = "";
  backPlateBoolFormula += backPlateName;
  backPlateBoolFormula +=
      "+" + backPlateStandName + ":" + backPlateStandTransName;
  backPlateBoolFormula +=
      "-" + backPlateHoleName + ":" + backPlateHoleTransName;
  backPlateBoolFormula += "-" + backPlateHoleCutName;

  const std::string backPlateCSName = backPlateName + "CS";
  const std::string backPlateCSTransName = backPlateCSName + "Trans";

  new TGeoCompositeShape(backPlateCSName.c_str(), backPlateBoolFormula.c_str());
  createAndRegisterTrans(backPlateCSTransName, 0, 0, sZContainerBack);
  //--------------------------------------------------------------------

  // Frontplate
  // the z-position of the frontplate
  const float zPosFrontPlate = sZContainerFront;
  // the height of the total stand overlapping with the rest of the plate
  const float dyFrontPlateStand =
      sDyContainerStand + (sDrMaxContainerFront - sDrMinContainerFront) / 2;
  // the y-position of the total stand
  const float yPosFrontPlateStand =
      -sDrMaxContainerFront - sDyContainerStand + dyFrontPlateStand / 2;

  const std::string frontPlateName = "FV0_FrontPlate";
  const std::string frontPlateStandName = frontPlateName + "Stand";
  const std::string frontPlateTransName = frontPlateName + "Trans";
  const std::string frontPlateStandTransName = frontPlateStandName + "Trans";

  new TGeoTubeSeg(frontPlateName.c_str(), sDrMinContainerFront,
                  sDrMaxContainerFront, sDzContainerFront / 2, -90, 90);
  new TGeoBBox(frontPlateStandName.c_str(), sDxContainerStand / 2,
               dyFrontPlateStand / 2, sDzContainerBack / 2);

  createAndRegisterTrans(frontPlateTransName, 0, 0, zPosFrontPlate);
  createAndRegisterTrans(frontPlateStandTransName, sDxContainerStand / 2,
                         yPosFrontPlateStand, 0);
  //--------------------------------------------------------------------

  // Frontplate cone composite shape
  std::string frontPlateBoolFormula = "";
  frontPlateBoolFormula += frontPlateName;
  frontPlateBoolFormula +=
      "+" + frontPlateStandName + ":" + frontPlateStandTransName;

  const std::string frontPlateCSName = frontPlateName + "CS";
  new TGeoCompositeShape(frontPlateCSName.c_str(),
                         frontPlateBoolFormula.c_str());
  //--------------------------------------------------------------------

  // Frontplate cone
  // radial thickness of frontplate cone in the xy-plane
  const float thicknessFrontPlateCone = sXYThicknessContainerCone;
  // z-position of the frontplate cone relative to the frontplate
  const float zPosCone = sDzContainerFront / 2 - sDzContainerCone / 2;

  const std::string frontPlateConeName =
      "FV0_FrontPlateCone"; // no volume with this name
  const std::string frontPlateConeShieldName =
      frontPlateConeName + "Shield"; // the "sides" of the cone
  const std::string frontPlateConeShieldTransName =
      frontPlateConeShieldName + "Trans";

  new TGeoConeSeg(frontPlateConeShieldName.c_str(), sDzContainerCone / 2,
                  sDrMinContainerCone,
                  sDrMinContainerCone + thicknessFrontPlateCone,
                  sDrMinContainerFront - thicknessFrontPlateCone,
                  sDrMinContainerFront, -90, 90);
  createAndRegisterTrans(frontPlateConeShieldTransName, 0, 0, zPosCone);
  //--------------------------------------------------------------------

  // Frontplate cone "bottom"
  // z-position of the cone bottom relative to the frontplate
  const float zPosConePlate =
      sDzContainerFront / 2 - sDzContainerCone + thicknessFrontPlateCone / 2;
  // the bottom of the cone
  const std::string frontPlateConePlateName = frontPlateConeName + "Plate";

  new TGeoTubeSeg(frontPlateConePlateName.c_str(), 0,
                  sDrMinContainerCone + thicknessFrontPlateCone,
                  thicknessFrontPlateCone / 2, -90, 90);

  // Frontplate cone bottom composite shape
  std::string frontPlateConePlateCSBoolFormula;
  frontPlateConePlateCSBoolFormula += frontPlateConePlateName;
  frontPlateConePlateCSBoolFormula +=
      "-" + backPlateHoleName + ":" + backPlateHoleTransName;

  const std::string frontPlateConePlateCSName = frontPlateConePlateName + "CS";
  const std::string frontPlateConePlateCSTransName =
      frontPlateConePlateCSName + "Trans";
  new TGeoCompositeShape(frontPlateConePlateCSName.c_str(),
                         frontPlateConePlateCSBoolFormula.c_str());
  createAndRegisterTrans(frontPlateConePlateCSTransName, 0, 0, zPosConePlate);
  //--------------------------------------------------------------------

  // Frontplate cone composite shape
  std::string frontPlateConeCSBoolFormula = "";
  frontPlateConeCSBoolFormula +=
      frontPlateConeShieldName + ":" + frontPlateConeShieldTransName;
  frontPlateConeCSBoolFormula +=
      "+" + frontPlateConePlateCSName + ":" + frontPlateConePlateCSTransName;

  const std::string frontPlateConeCSName = frontPlateConeName + "CS";
  new TGeoCompositeShape(frontPlateConeCSName.c_str(),
                         frontPlateConeCSBoolFormula.c_str());

  // Shields
  const float dzShieldGap =
      0.7; // z-distance between the shields and the front- and backplate outer
           // edges (in z-direction)
  const float dzShield = sDzContainer - 2 * dzShieldGap; // depth of the shields

  // Outer shield
  const float zPosOuterShield = (sZContainerBack + sZContainerFront) /
                                2; // z-position of the outer shield

  const std::string outerShieldName = "FV0_OuterShield";
  const std::string outerShieldTransName = outerShieldName + "Trans";

  new TGeoTubeSeg(outerShieldName.c_str(), sDrMinContainerOuterShield,
                  sDrMaxContainerOuterShield, dzShield / 2, -90, 90);
  createAndRegisterTrans(outerShieldTransName, 0, 0, zPosOuterShield);
  //--------------------------------------------------------------------

  // Inner shield
  // depth of the inner shield
  const float dzInnerShield = sDzContainer - sDzContainerCone - dzShieldGap;
  // z-position of the inner shield relative to the backplate
  const float zPosInnerShield =
      sZContainerBack - sDzContainerBack / 2 + dzShieldGap + dzInnerShield / 2;

  const std::string innerShieldName = "FV0_InnerShield";
  const std::string innerShieldCutName = innerShieldName + "Cut";

  new TGeoTubeSeg(innerShieldName.c_str(), sDrMinContainerInnerShield,
                  sDrMaxContainerInnerShield, dzInnerShield / 2, -90, 90);
  new TGeoBBox(innerShieldCutName.c_str(), fabs(sXShiftContainerHole),
               sDrMaxContainerInnerShield, dzInnerShield / 2);

  // Inner shield composite shape
  std::string innerShieldCSBoolFormula;
  innerShieldCSBoolFormula = innerShieldName;
  innerShieldCSBoolFormula += "-" + innerShieldCutName;

  const std::string innerShieldCSName = innerShieldName + "CS";
  const std::string innerShieldCSTransName = innerShieldCSName + "Trans";
  new TGeoCompositeShape(innerShieldCSName.c_str(),
                         innerShieldCSBoolFormula.c_str());
  createAndRegisterTrans(innerShieldCSTransName, sXShiftContainerHole, 0,
                         zPosInnerShield);
  //--------------------------------------------------------------------

  // Cover
  // Depth of the covers
  const float dzCover = sDzContainer;
  // Set the cone cut relative to the frontplate so that the exact position of
  // the aluminium cone part can be used.
  const float zPosCoverConeCut = zPosFrontPlate + zPosCone;

  const std::string coverName = "FV0_Cover";
  const std::string coverConeCutName = coverName + "ConeCut";
  const std::string coverHoleCutName = coverName + "HoleCut";

  new TGeoBBox(coverName.c_str(), sDxContainerCover / 2,
               sDrMaxContainerOuterShield, dzCover / 2);
  new TGeoCone(coverConeCutName.c_str(), sDzContainerCone / 2, 0,
               sDrMinContainerCone + thicknessFrontPlateCone, 0,
               sDrMinContainerFront);
  new TGeoTubeSeg(coverHoleCutName.c_str(), 0, sDrMinContainerInnerShield,
                  dzCover / 2, 0, 360);

  const std::string coverTransName = coverName + "Trans";
  const std::string coverConeCutTransName = coverConeCutName + "Trans";
  const std::string coverHoleCutTransName = coverHoleCutName + "Trans";

  createAndRegisterTrans(coverTransName, sDxContainerCover / 2, 0,
                         zPosOuterShield);
  createAndRegisterTrans(coverConeCutTransName, 0, 0, zPosCoverConeCut);
  createAndRegisterTrans(coverHoleCutTransName.c_str(), sXShiftContainerHole, 0,
                         zPosOuterShield);
  //--------------------------------------------------------------------

  // Cover composite shape
  std::string coverCSBoolFormula = "";
  coverCSBoolFormula += coverName + ":" + coverTransName;
  coverCSBoolFormula += "-" + coverConeCutName + ":" + coverConeCutTransName;
  coverCSBoolFormula += "-" + coverHoleCutName + ":" + coverHoleCutTransName;

  const std::string coverCSName = coverName + "CS";
  new TGeoCompositeShape(coverCSName.c_str(), coverCSBoolFormula.c_str());

  // Stand bottom
  const float dzStandBottom =
      sDzContainer - sDzContainerBack - sDzContainerFront;
  const float dyStandBottomGap =
      0.5; // This bottom part is not vertically aligned with the "front and
           // backplate stands"
  const float dxStandBottomHole = 9.4;
  const float dzStandBottomHole = 20.4;
  const float dxStandBottomHoleSpacing = 3.1;

  const std::string standName = "FV0_StandBottom";
  const std::string standHoleName = standName + "Hole";

  new TGeoBBox(standName.c_str(), sDxContainerStandBottom / 2,
               sDyContainerStandBottom / 2, dzStandBottom / 2);
  new TGeoBBox(standHoleName.c_str(), dxStandBottomHole / 2,
               sDyContainerStandBottom / 2 + sEpsilon, dzStandBottomHole / 2);

  const std::string standHoleTrans1Name = standHoleName + "Trans1";
  const std::string standHoleTrans2Name = standHoleName + "Trans2";
  const std::string standHoleTrans3Name = standHoleName + "Trans3";

  createAndRegisterTrans(standHoleTrans1Name,
                         -dxStandBottomHoleSpacing - dxStandBottomHole, 0, 0);
  createAndRegisterTrans(standHoleTrans2Name, 0, 0, 0);
  createAndRegisterTrans(standHoleTrans3Name,
                         dxStandBottomHoleSpacing + dxStandBottomHole, 0, 0);
  //--------------------------------------------------------------------

  // Stand bottom composite shape
  const std::string standCSName = standName + "CS";

  std::string standBoolFormula = "";
  standBoolFormula += standName;
  standBoolFormula += "-" + standHoleName + ":" + standHoleTrans1Name;
  standBoolFormula += "-" + standHoleName + ":" + standHoleTrans2Name;
  standBoolFormula += "-" + standHoleName + ":" + standHoleTrans3Name;

  new TGeoCompositeShape(standCSName.c_str(), standBoolFormula.c_str());

  const std::string standCSTransName = standCSName + "Trans";

  createAndRegisterTrans(standCSTransName.c_str(),
                         sDxContainerStand - sDxContainerStandBottom / 2,
                         -(sDrMaxContainerBack + sDyContainerStand) +
                             sDyContainerStandBottom / 2 + dyStandBottomGap,
                         sZContainerMid);
  //--------------------------------------------------------------------

  // Composite shape
  std::string boolFormula = "";
  boolFormula += backPlateCSName + ":" + backPlateCSTransName;
  boolFormula += "+" + frontPlateCSName + ":" + frontPlateTransName;
  boolFormula += "+" + frontPlateConeCSName + ":" + frontPlateTransName;
  boolFormula += "+" + outerShieldName + ":" + outerShieldTransName;
  boolFormula += "+" + innerShieldCSName + ":" + innerShieldCSTransName;
  boolFormula += "+" + coverCSName;
  boolFormula += "+" + standCSName + ":" + standCSTransName;
  boolFormula += "-" + sScrewHolesCSName; // Remove holes for screws
  boolFormula += "-" + sRodHolesCSName;   // Remove holes for rods

  const std::string aluContCSName = "FV0_AluContCS";
  const TGeoCompositeShape *aluContCS =
      new TGeoCompositeShape(aluContCSName.c_str(), boolFormula.c_str());

  // Volume
  const std::string aluContName = createVolumeName(sContainerName);
  const TGeoMedium *medium = gGeoManager->GetMedium("FIT_Aluminium$");
  if (!medium) {
    std::cout << "\n ************** Debug: Error **************\n Material "
                 "Aluminium not found for Container formation "
              << std::endl;
  }
  new TGeoVolume(aluContName.c_str(), aluContCS, medium);
}

void AliFITv8::assembleMetalContainer(TGeoVolume *vFV0) const {
  Color_t containerColor = kGray + 2;
  TGeoVolume *container =
      gGeoManager->GetVolume(createVolumeName(sContainerName).c_str());
  container->SetLineColor(containerColor);
  container->SetFillColor(containerColor);
  if (!container) {
    // LOG(WARNING) << "FV0: Could not find container volume";
    std::cout
        << "\n****** ERROR ******\n AliFITv8: Could not find container volume";
  } else {
    vFV0->AddNode(container, 1, mLeftTransformation);
    vFV0->AddNode(container, 2, mRightTransformation);
  }
}

///----------- Rihan Upto this implemented -----------------

void AliFITv8::CreateGeometry() {
  //
  // Create the geometry of FIT Detector
  //
  // begin Html
  //

  /// T0 implementation done here
  Int_t *idtmed = fIdtmed->GetArray();
  Float_t zdetC = 82; // center of mother volume
  Float_t zdetA = 335.5;

  Int_t idrotm[999];
  Double_t x, y, z;
  AliMatrix(idrotm[901], 90., 0., 90., 90., 180., 0.);

  TGeoVolumeAssembly *stlinC = new TGeoVolumeAssembly("0STR"); // C side mother
  TGeoVolumeAssembly *stlinA = new TGeoVolumeAssembly("0STL"); // A side mother
  // FIT interior
  float pins[3];
  for (int i=0; i<3; i++){
    pins[i] = fInStart[i];
  }
  TVirtualMC::GetMC()->Gsvolu("0INS", "BOX", idtmed[kAir], pins, 3);
  TGeoVolume *ins = gGeoManager->GetVolume("0INS");
  SetOneMCP(ins);
  // C side Concave Geometry
  Double_t crad = 82.; // define concave c-side radius here
  //  Double_t dP = 3.31735114408; // Work in Progress side length
  Double_t dP = fInStart[0]; // uniform angle between detector faces==
  Double_t btta = 2 * TMath::ATan(dP / crad);

  // get noncompensated translation data
  Double_t grdin[6] = {-3, -2, -1, 1, 2, 3};
  Double_t gridpoints[6];
  for (Int_t i = 0; i < 6; i++) {
    gridpoints[i] = crad * TMath::Sin((1 - 1 / (2 * TMath::Abs(grdin[i]))) *
                                      grdin[i] * btta);
  }

  std::vector<Double_t> xi, yi;

  for (Int_t j = 5; j >= 0; j--) {
    for (Int_t i = 0; i < 6; i++) {
      if (!(((j == 5 || j == 0) && (i == 5 || i == 0)) ||
            ((j == 3 || j == 2) && (i == 3 || i == 2)))) {
        xi.push_back(gridpoints[i]);
        yi.push_back(gridpoints[j]);
      }
    }
  }
  Double_t zi[28];
  for (Int_t i = 0; i < 28; i++) {
    zi[i] = TMath::Sqrt(TMath::Power(crad, 2) - TMath::Power(xi[i], 2) -
                        TMath::Power(yi[i], 2));
  }

  // get rotation data
  Double_t ac[28], bc[28], gc[28];
  for (Int_t i = 0; i < 28; i++) {
    ac[i] = TMath::ATan(yi[i] / xi[i]) - TMath::Pi() / 2 + 2 * TMath::Pi();
    if (xi[i] < 0) {
      bc[i] = TMath::ACos(zi[i] / crad);
    } else {
      bc[i] = -1 * TMath::ACos(zi[i] / crad);
    }
  }
  Double_t xc2[28], yc2[28], zc2[28];

  // compensation based on node position within individual detector geometries
  // determine compensated radius
  Double_t rcomp = crad + fstartC[2] / 2.0; //
  for (Int_t i = 0; i < 28; i++) {
    // Get compensated translation data
    xc2[i] =
        rcomp * TMath::Cos(ac[i] + TMath::Pi() / 2) * TMath::Sin(-1 * bc[i]);
    yc2[i] =
        rcomp * TMath::Sin(ac[i] + TMath::Pi() / 2) * TMath::Sin(-1 * bc[i]);
    zc2[i] = rcomp * TMath::Cos(bc[i]);
    printf(" side C %f  xc2[i] %f yc2[i] %f zc2[i] %f \n",  xc2[i], yc2[i], zc2[i] ) ;

    // Convert angles to degrees
    ac[i] *= 180 / TMath::Pi();
    bc[i] *= 180 / TMath::Pi();
    gc[i] = -1 * ac[i];
  }

  TGeoTranslation *tr[52];
  TString nameTr;

  // C Side Transformations
  TGeoRotation *rot[28];
  TString nameRot;
  TGeoCombiTrans *com[28];
  TGeoCombiTrans *comCable[28];
  TString nameCom;

  // C Side Transformations
  for (Int_t itr = 24; itr < 52; itr++) {
    nameTr = Form("0TR%i", itr + 1);
    nameRot = Form("0Rot%i", itr + 1);
    int ic = itr - 24;
    // nameCom = Form("0Com%i",itr+1);
    rot[ic] = new TGeoRotation(nameRot.Data(), ac[ic], bc[ic], gc[ic]);
    rot[ic]->RegisterYourself();

    //    tr[itr] = new TGeoTranslation(nameTr.Data(), xc2[ic], yc2[ic],
    //    (zc2[ic] - 80.));
    // tr[itr]->RegisterYourself();
    com[ic] = new TGeoCombiTrans(xc2[ic], yc2[ic], (zc2[ic] - 80), rot[ic]);
    //    com[ic] = new TGeoCombiTrans(tr[itr], rot[ic]);
    //    fPosModuleCx[ic] = xc2[ic];
    // fPosModuleCy[ic] = yc2[ic];
    // fPosModuleCz[ic] = zc2[ic] - 80;

    TGeoHMatrix hm = *com[ic];
    TGeoHMatrix *ph = new TGeoHMatrix(hm);
    stlinC->AddNode(ins, itr, ph);
    // cables
    TGeoVolume *cables = SetCablesSize(itr);
    comCable[ic] = new TGeoCombiTrans(
        xc2[ic], yc2[ic], zc2[ic] - 80 + fInStart[2] + 0.2, rot[ic]);
    TGeoHMatrix hmCable = *comCable[ic];
    TGeoHMatrix *phCable = new TGeoHMatrix(hmCable);
    stlinC->AddNode(cables, itr, comCable[ic]);
  }

  // A side Translations
  for (Int_t itr = 0; itr < 24; itr++) {
    nameTr = Form("0TR%i", itr + 1);
    z = -fstartA[2] + fInStart[2];
    tr[itr] = new TGeoTranslation(nameTr.Data(), fPosModuleAx[itr],
                                  fPosModuleAy[itr], z);
    tr[itr]->RegisterYourself();
    stlinA->AddNode(ins, itr, tr[itr]);
    printf("A  x %f y %f z %f \n",  fPosModuleAx[itr],  fPosModuleAy[itr], z);
    }

  SetCablesA(stlinA);
  // Add FT0-A support Structure to the geometry
  printf(" module size %f %f %f \n", fInStart[0], fInStart[1], fInStart[2]);
  stlinA->AddNode(constructFrameGeometry(), 1,
                  new TGeoTranslation(0, 0, -fstartA[2] + fInStart[2]));

  TGeoVolume *alice = gGeoManager->GetVolume("ALIC");
  alice->AddNode(stlinA, 1, new TGeoTranslation(0, 0, zdetA));
  TGeoRotation *rotC = new TGeoRotation("rotC", 90., 0., 90., 90., 180., 0.);
  alice->AddNode(stlinC, 1, new TGeoCombiTrans(0., 0., -zdetC, rotC));
  stlinC->Print();
  stlinA->Print();

  SetVZEROGeo(alice); // rihan: the V0+ geometry is set by this function.


}
//--------------------------------------------------------------------

void AliFITv8::SetOneMCP(TGeoVolume *ins) {
  Int_t *idtmed = fIdtmed->GetArray();
  Double_t x, y, z;
  Float_t ptop[3] = {1.324, 1.324, 1.}; // Cherenkov radiator
  Float_t ptopref[3] = {1.3241, 1.3241,
                        1.}; // Cherenkov radiator wrapped with reflector
  Double_t prfv[3] = {0.0002, 1.323,
                      1.}; // Vertical refracting layer bettwen radiators and
                           // between radiator and not optical Air
  Double_t prfh[3] = {
      1.323, 0.0002,
      1.}; // Horizontal refracting layer bettwen radiators and ...
  Float_t pmcp[3] = {2.949, 2.949, 0.66}; // MCP
  Float_t pmcpinner[3] = {2.749, 2.749, 0.1};
  Float_t pmcpbase[3] = {2.949, 2.949, 0.675};
  Float_t pmcpside[3] = {0.15, 2.949, 0.65};
  Float_t pmcptopglass[3] = {2.949, 2.949, 0.1}; // MCP top glass optical
  Float_t preg[3] = {1.324, 1.324, 0.005};       // Photcathode
  ///  Double_t pal[3] = {2.648, 2.648, 0.25}; // 5mm Al on top of each radiator

  // Entry window (glass)
  TVirtualMC::GetMC()->Gsvolu("0TOP", "BOX", idtmed[kOpGlass], ptop,
                              3); // Glass radiator
  TGeoVolume *top = gGeoManager->GetVolume("0TOP");
  top->Print();
  TVirtualMC::GetMC()->Gsvolu("0TRE", "BOX", idtmed[kAir], ptopref,
                              3); // Air: wrapped  radiator
  TGeoVolume *topref = gGeoManager->GetVolume("0TRE");
  TVirtualMC::GetMC()->Gsvolu("0RFV", "BOX", idtmed[kOptAl], prfv,
                              3); // Optical Air vertical
  TGeoVolume *rfv = gGeoManager->GetVolume("0RFV");
  TVirtualMC::GetMC()->Gsvolu("0RFH", "BOX", idtmed[kOptAl], prfh,
                              3); // Optical Air horizontal
  TGeoVolume *rfh = gGeoManager->GetVolume("0RFH");

  TVirtualMC::GetMC()->Gsvolu("0REG", "BOX", idtmed[kOpGlassCathode], preg, 3);
  TGeoVolume *cat = gGeoManager->GetVolume("0REG");

  // wrapped radiator +  reflecting layers

  Int_t ntops = 0, nrfvs = 0, nrfhs = 0;
  x = y = z = 0;
  topref->AddNode(top, 1, new TGeoTranslation(0, 0, 0));
  float xinv = -ptop[0] - prfv[0];
  topref->AddNode(rfv, 1, new TGeoTranslation(xinv, 0, 0));
  xinv = ptop[0] + prfv[0];
  topref->AddNode(rfv, 2, new TGeoTranslation(xinv, 0, 0));
  float yinv = -ptop[1] - prfh[1];
  topref->AddNode(rfh, 1, new TGeoTranslation(0, yinv, 0));
  yinv = ptop[1] + prfh[1];
  topref->AddNode(rfh, 2, new TGeoTranslation(0, yinv, 0));

  // container for radiator, cathode
  for (Int_t ix = 0; ix < 2; ix++) {
    float xin = -fInStart[0] + 0.3 + (ix + 0.5) * 2 * ptopref[0];
    for (Int_t iy = 0; iy < 2; iy++) {
      z = -fInStart[2] + ptopref[2];
      float yin = -fInStart[1] + 0.3 + (iy + 0.5) * 2 * ptopref[1];
      ntops++;
      ins->AddNode(topref, ntops, new TGeoTranslation(xin, yin, z));
      topref->Print();
      z += ptopref[2] + 2. * pmcptopglass[2] + preg[2];
      ins->AddNode(cat, ntops, new TGeoTranslation(xin, yin, z));
      cat->Print();
    }
  }
  // MCP
  TVirtualMC::GetMC()->Gsvolu("0MTO", "BOX", idtmed[kOpGlass], pmcptopglass,
                              3); // Op  Glass
  TGeoVolume *mcptop = gGeoManager->GetVolume("0MTO");
  mcptop->Print();
  z = -fInStart[2] + 2 * ptopref[2] + pmcptopglass[2];
  ins->AddNode(mcptop, 1, new TGeoTranslation(0, 0, z));

  TVirtualMC::GetMC()->Gsvolu("0MCP", "BOX", idtmed[kAir], pmcp, 3); // glass
  TGeoVolume *mcp = gGeoManager->GetVolume("0MCP");
  z = -fInStart[2] + 2 * ptopref[2] + 2 * pmcptopglass[2] + 2 * preg[2] +
      pmcp[2];
  ins->AddNode(mcp, 1, new TGeoTranslation(0, 0, z));

  TVirtualMC::GetMC()->Gsvolu("0MSI", "BOX", idtmed[kMCPwalls], pmcpside,
                              3); // glass
  TGeoVolume *mcpside = gGeoManager->GetVolume("0MSI");
  mcpside->Print();
  x = -pmcp[0] + pmcpside[0];
  y = -pmcp[1] + pmcpside[1];
  mcp->AddNode(mcpside, 1, new TGeoTranslation(x, y, 0));
  x = pmcp[0] - pmcpside[0];
  y = pmcp[1] - pmcpside[1];
  mcp->AddNode(mcpside, 2, new TGeoTranslation(x, y, 0));
  x = -pmcp[1] + pmcpside[1];
  y = -pmcp[0] + pmcpside[0];
  mcp->AddNode(mcpside, 3,
               new TGeoCombiTrans(x, y, 0, new TGeoRotation("R2", 90, 0, 0)));
  x = pmcp[1] - pmcpside[1];
  y = pmcp[0] - pmcpside[0];
  mcp->AddNode(mcpside, 4,
               new TGeoCombiTrans(x, y, 0, new TGeoRotation("R2", 90, 0, 0)));

  TVirtualMC::GetMC()->Gsvolu("0MBA", "BOX", idtmed[kCeramic], pmcpbase,
                              3); // glass
  TGeoVolume *mcpbase = gGeoManager->GetVolume("0MBA");
  mcpbase->Print();
  z = -fInStart[2] + 2 * ptopref[2] + pmcptopglass[2] + 2 * pmcp[2] +
      pmcpbase[2];
  ins->AddNode(mcpbase, 1, new TGeoTranslation(0, 0, z));
  ins->Print();
}

//------------------------------------------------------------------------
void AliFITv8::SetCablesA(TGeoVolume *stl) {

  Int_t *idtmed = fIdtmed->GetArray();
  Float_t pcableplane[3] = {20, 20, 0.25}; //

  TVirtualMC::GetMC()->Gsvolu("0CAA", "BOX", idtmed[kAir], pcableplane,
                              3); // container for cables
  TGeoVolume *cableplane = gGeoManager->GetVolume("0CAA");
  int na = 0;

  double xcell[24], ycell[24];

  for (int imcp = 0; imcp < 24; imcp++) {
    xcell[na] = fPosModuleAx[imcp];
    ycell[na] = fPosModuleAy[imcp];
    TGeoVolume *vol = SetCablesSize(imcp);
    cableplane->AddNode(vol, na, new TGeoTranslation(xcell[na], ycell[na], 0));
    na++;
  }

  // 12 cables extending beyond the frame
  Float_t pcablesextend[3] = {2, 15, 0.245};
  Float_t pcablesextendsmall[3] = {3, 2, 0.245};
  // Float_t *ppcablesextend[4] = {pcablesextend, pcablesextend,
  // pcablesextendsmall,
  //                         pcablesextendsmall};
  // left side
  double xcell_side[4] = {-fstartA[0] + pcablesextend[0],
                          fstartA[0] - pcablesextend[0], 0, 0};
  double ycell_side[4] = {0, 0, -fstartA[1] + pcablesextendsmall[1],
                          +fstartA[1] - pcablesextendsmall[1]};

  for (int icab = 0; icab < 2; icab++) {
    const std::string volName = Form("CAB%2.i", 52 + icab);
    TVirtualMC::GetMC()->Gsvolu(volName.c_str(), "BOX", idtmed[kCable],
                                pcablesextend, 3); // cables
    TGeoVolume *vol = gGeoManager->GetVolume(volName.c_str());
    cableplane->AddNode(
        vol, 1, new TGeoTranslation(xcell_side[icab], ycell_side[icab], 0));
  }
  for (int icab = 2; icab < 4; icab++) {
    const std::string volName = Form("CAB%2.i", 52 + icab);
    TVirtualMC::GetMC()->Gsvolu(volName.c_str(), "BOX", idtmed[kCable],
                                pcablesextendsmall, 3); // cables
    TGeoVolume *vol = gGeoManager->GetVolume(volName.c_str());
    cableplane->AddNode(
        vol, 1, new TGeoTranslation(xcell_side[icab], ycell_side[icab], 0));
  }
  float zcableplane = -fstartA[2] + 2 * fInStart[2] + pcableplane[2];
  stl->AddNode(cableplane, 1, new TGeoTranslation(0, 0, zcableplane));
}

//------------------------------------------
TGeoVolume *AliFITv8::SetCablesSize(int mod) {
  Int_t *idtmed = fIdtmed->GetArray();
  int na = 0;
  int ncells = 52; // N modules
  int mcpcables[52] = {2, 1, 2, 1, 2, 2, 1, 1, 1, 2, 2, 1, 1, 2, 2, 1, 1, 1,
                       2, 2, 1, 2, 1, 2, 2, 1, 1, 2, 3, 2, 1, 1, 2, 3, 2, 1,
                       1, 2, 2, 1, 1, 2, 3, 2, 1, 1, 2, 3, 2, 1, 1, 2};
  // cable D=0.257cm, Weight: 13 lbs/1000ft = 0.197g/cm; 1 piece 0.65cm
  // 1st 8 pieces - tube  8*0.65cm = 5.2cm; V = 0.0531cm2 -> box
  // {0.27*0.27*1}cm; W = 0.66g 2nd 24 pieces 24*0.65cm; V = 0.76 -> {0.44,
  // 0.447 1}; W = 3.07g 3d  48  pieces  48*0.65cm;  V = 1.53cm^3; ->box {0.66,
  // 0.66, 1.}; W= 6.14g
  double xcell[ncells], ycell[ncells], zcell[ncells];
  float xsize[3] = {1.8, 1.8, 2.6}; //
  float ysize[3] = {0.6, 1.7, 2.};
  float zsize[3] = {0.1, 0.1, 0.1};
  //  for (int imcp = 0; imcp < Geometry::NCellsC; imcp++) {
  int ic = mcpcables[mod];
  float calblesize[3];
  calblesize[0] = xsize[ic - 1];
  calblesize[1] = ysize[ic - 1];
  calblesize[2] = zsize[ic - 1];
  const std::string volName = Form("CAB%2.i", mod);
  TVirtualMC::GetMC()->Gsvolu(volName.c_str(), "BOX", idtmed[kCable],
                              calblesize, 3); // cables
  TGeoVolume *vol = gGeoManager->GetVolume(volName.c_str());
  //  vol->Print();
  //  vol->Weight();
  return vol;
}

// Class wrapper for construction of FT0-A support structure
// The frame is constructed by defining two aluminum boxes that are placed in an
// L-shape, with material sequentially removed to re-create the CAD drawings,
// including sockets defined by the parameters of the sensitive elements that
// they are placed into Two L-shaped elements form the full support structure,
// with one reflected about the axes of symmetry First written by Joe Crowley
// and revised by Jason Pruitt from Cal Poly in 2019-2021
TGeoVolume *AliFITv8::constructFrameGeometry() {
  // define the media
  TGeoMedium *Vacuum = gGeoManager->GetMedium(3);
  TGeoMedium *Al = gGeoManager->GetMedium(15);
  Al->Print();
  // make a volume assembly for the frame
  TGeoVolumeAssembly *FT0_Frame = new TGeoVolumeAssembly("FT0_Frame");

  // define translations for the quartz radiator and PMT sockets
  defineTransformations();

  // frame1 and frame2 are rectangles that approximate the outline of one L
  // shape of the frame
  TGeoBBox *frame1 =
      new TGeoBBox("frame1", sFrame1X / 2, sFrame1Y / 2, sFrameZ / 2);
  TGeoBBox *frame2 =
      new TGeoBBox("frame2", sFrame2X / 2, sFrame2Y / 2, sFrameZ / 2);
  // the following elements are subtracted from frame1 and frame2 to better
  // approximate the CAD shape
  TGeoBBox *rect1 =
      new TGeoBBox("rect1", sRect1X / 2, sRect1Y / 2, sFrameZ / 2);
  TGeoBBox *rect2 = new TGeoBBox("rect2", sRect2X / 2, sRect2Y / 2 + sEps,
                                 sFrameZ / 2 - sMountZ / 2);
  TGeoBBox *rect3 =
      new TGeoBBox("rect3", sRect3X / 2, sRect3Y / 2, sFrameZ / 2);
  TGeoBBox *rect4 =
      new TGeoBBox("rect4", sRect4X / 2, sRect4Y / 2, sFrameZ / 2);
  TGeoBBox *rect5 = new TGeoBBox("rect5", sRect5X / 2 + sEps,
                                 sRect5Y / 2 + sEps, sFrameZ / 2 + sEps);
  TGeoBBox *rect6 = new TGeoBBox("rect6", sRect6X / 2 + sEps,
                                 sRect6Y / 2 + sEps, sFrameZ / 2 + sEps);
  TGeoBBox *rect7 =
      new TGeoBBox("rect7", sRect7X / 2 + sEps, sRect7Y / 2 + sEps,
                   sFrameZ / 2 - sMountZ / 2 + sEps);
  TGeoBBox *rect8 = new TGeoBBox("rect8", sRect8X / 2 + sEps,
                                 sRect8Y / 2 + sEps, sFrameZ / 2 + sEps);

  // Define a value to overcut the coincidence between the closure of the tube
  // and the edge of the rectangle to eliminate artifacts
  Double_t flopsErr = .00001;
  // PMT and quartz radiator shapes provide the dimensions of the sockets to be
  // subtracted from the frame that will make room for a sensitive element to
  // fit
  TGeoBBox *quartzRadiator =
      new TGeoBBox("quartzRadiator", sQuartzRadiatorSide / 2,
                   sQuartzRadiatorSide / 2, sQuartzRadiatorZ / 2);
  TGeoBBox *pmtBox = new TGeoBBox("pmtBox", sPmtSide / 2 + sEps,
                                  sPmtSide / 2 + sEps, sPmtZ / 2 + sEps);
  // these two shapes create a subtraction so that the corners of the holes that
  // seat the sens elements are rounded
  TGeoBBox *pmtCornerRect =
      new TGeoBBox("pmtCornerRect", sCornerRadius / 2 - flopsErr,
                   sCornerRadius / 2 - flopsErr, sPmtZ / 2);
  TGeoTube *pmtCornerTube =
      new TGeoTube("pmtCornerTube", 0, sCornerRadius, sPmtZ / 2 + sEps);
  TGeoVolume *PMTCorner =
      new TGeoVolume("PMTCorner",
                     new TGeoCompositeShape(
                         "PMTCorner", pmtCornerCompositeShapeBoolean().c_str()),
                     Al);
  // TGeoVolume* PMT = new TGeoVolume("PMT", new TGeoCompositeShape("PMT",
  // pmtCompositeShapeBoolean().c_str()), Vacuum);
  TGeoVolume *PMT = gGeoManager->MakeBox("PMT", Vacuum, sPmtSide / 2 + sEps,
                                         sPmtSide / 2 + sEps, sPmtZ / 2 + sEps);

  // add the plates on the bottom of the frame
  TGeoBBox *basicPlate = new TGeoBBox("basicPlate", sPlateSide / 2,
                                      sPlateSide / 2, sBasicPlateZ / 2);
  TGeoBBox *cablePlate = new TGeoBBox("cablePlate", sPlateSide / 2,
                                      sPlateSide / 2, sCablePlateZ / 2);
  TGeoBBox *opticalFiberHead = new TGeoBBox("opticalFiberHead", sFiberHeadX / 2,
                                            sFiberHeadY / 2, sCablePlateZ / 2);
  TGeoCompositeShape *opticalFiberPlate1 = new TGeoCompositeShape(
      "opticalFiberPlate1", opticalFiberPlateCompositeShapeBoolean1().c_str());
  TGeoCompositeShape *opticalFiberPlate2 = new TGeoCompositeShape(
      "opticalFiberPlate2", opticalFiberPlateCompositeShapeBoolean2().c_str());
  TGeoCompositeShape *plateBox = new TGeoCompositeShape(
      "plateBox", plateBoxCompositeShapeBoolean().c_str());
  // holds 2 basic plates and 2 cable plates
  TGeoVolume *plateGroup = new TGeoVolume(
      "plateGroup",
      new TGeoCompositeShape("plateGroup",
                             plateGroupCompositeShapeBoolean().c_str()),
      Al); // holds 3 plate boxes
  // remove the material to form the sockets for the quartz radiators and PMTs
  TGeoCompositeShape *frameRemovedPMTandRadiators1 = new TGeoCompositeShape(
      "frameRemovedPMTandRadiators1", frame1CompositeShapeBoolean().c_str());
  TGeoCompositeShape *frameRemovedPMTandRadiators2 = new TGeoCompositeShape(
      "frameRemovedPMTandRadiators2", frame2CompositeShapeBoolean().c_str());

  // make the right side frame - L shape
  TGeoVolume *frame = new TGeoVolume(
      "frame",
      new TGeoCompositeShape("frame", frameCompositeShapeBoolean().c_str()),
      Al);

  // reflection for the left side of the frame
  TGeoRotation *reflect = new TGeoRotation("reflect");
  reflect->ReflectX(true);
  reflect->ReflectY(true);
  reflect->RegisterYourself();

  // add a shift to eliminate overlaps between sens elements and frame sockets
  // this shift will apply to both sides of the frame
  TGeoTranslation *xshift = new TGeoTranslation("xshift", .1028, 0, 0);

  // add the right and left sides to top volume
  FT0_Frame->AddNode(frame, 1, xshift);  // right side
  FT0_Frame->AddNode(frame, 2, reflect); // left side

  return FT0_Frame;
}
// the following are continually concatenated strings that ROOT Geometry will
// read in order to piece together the objects and translations that are
// defined above (what ROOT Geometry calls Booleans)
// frame1 is a horizontal aluminum box piece of the L-shape
std::string AliFITv8::frame1CompositeShapeBoolean() {
  // create a string for the boolean operations for the composite frame shape
  std::string frame1CompositeShapeBoolean = "";
  frame1CompositeShapeBoolean += "((frame1";

  // remove the radiator shapes for the sockets
  // frame1 is the horizontal piece of the right-hand L-shape (looking from
  // back) with its own internal numbering for the sockets.  To more easily map
  // between the sensitive elements and their socket locations, we've included
  // the correspondence between them.  Within the horizontal piece, the sockets
  // are numbered column by column from left to right
  // ---------
  // |       |                 <-----Rectangle 1 removed here
  // |   1   |----------------- ^
  // |       |        |       | |
  // ---------    3   |   5   | |
  // |       |        |       | | Rectangle 2 removed here
  // |   2   |----------------- |
  // |       |        |       | |
  // --------|    4   |   6   | |
  //         |        |       | v
  //    ^    ------------------  <------Rectangle 3 removed here
  //    |
  //    |
  //    Rectangle 4 removed here
  //
  // internal numbering for each is mapped to the sensitive element numbering
  // for ease of comparison and identification
  // Since one L is reflected about the axes of symmetry, the correspondence
  // with sensitive element numbering for the left-side L-shape is also included
  // here.
  frame1CompositeShapeBoolean +=
      " - quartzRadiator:quartzRadiatorTr1"; // Sens Elmt 2,21
  frame1CompositeShapeBoolean +=
      " - quartzRadiator:quartzRadiatorTr2"; // Sens Elmt 7,16
  frame1CompositeShapeBoolean +=
      " - quartzRadiator:quartzRadiatorTr3"; // Sens Elmt 3,20
  frame1CompositeShapeBoolean +=
      " - quartzRadiator:quartzRadiatorTr4"; // Sens Elmt 8,15
  frame1CompositeShapeBoolean +=
      " - quartzRadiator:quartzRadiatorTr5"; // Sens Elmt 4,19
  frame1CompositeShapeBoolean +=
      " - quartzRadiator:quartzRadiatorTr6)"; // Sens Elmt 9,14

  // remove the PMT shapes for the sockets
  frame1CompositeShapeBoolean += " - PMT:PMTTr1";
  frame1CompositeShapeBoolean += " - PMT:PMTTr2";
  frame1CompositeShapeBoolean += " - PMT:PMTTr3";
  frame1CompositeShapeBoolean += " - PMT:PMTTr4";
  frame1CompositeShapeBoolean += " - PMT:PMTTr5";
  frame1CompositeShapeBoolean += " - PMT:PMTTr6)";

  return frame1CompositeShapeBoolean;
}
// frame2 is the vertical aluminum box piece of the L-shape
std::string AliFITv8::frame2CompositeShapeBoolean() {
  std::string frame2CompositeShapeBoolean = "";
  frame2CompositeShapeBoolean += "((frame2";

  // remove the radiator shapes for the sockets
  // frame2 is the vertical piece of the right-hand L-shape (looking from back)
  // with its own internal numbering for the sockets.  To more easily map
  // between the sensitive elements and their socket locations, we've included
  // the correspondence between them.  Within the vertical piece, the sockets
  // are numbered row by row from right to left
  //                  -----------------
  //                  |       |       |
  //  Rectangle-->    |   8   |   7   |
  //     8            |       |       |
  //  removed      --------------------
  //    here       |       |       | ^
  //               |  10   |   9   | |
  //               |       |       | | Rectangle 5 removed here
  //               ----------------- |
  //               |       |       | |
  //               |  12   |  11   | v
  //               |       |       |  <-----Rectangle 6 removed here
  //               -----------------
  //              <---------------->
  //            Rectangle 7 removed here
  //
  // internal numbering for each is mapped to the sensitive element numbering
  // for ease of comparison and identification
  // Since one L is reflected about the axes of symmetry, the correspondence
  // with sensitive element numbering for the left-side L-shape is also included
  // here.
  frame2CompositeShapeBoolean +=
      " - quartzRadiator:quartzRadiatorTr7"; // Sens Elmt 13,10
  frame2CompositeShapeBoolean +=
      " - quartzRadiator:quartzRadiatorTr8"; // Sens Elmt 12,11
  frame2CompositeShapeBoolean +=
      " - quartzRadiator:quartzRadiatorTr9"; // Sens Elmt 18,14
  frame2CompositeShapeBoolean +=
      " - quartzRadiator:quartzRadiatorTr10"; // Sens Elmt 17,15
  frame2CompositeShapeBoolean +=
      " - quartzRadiator:quartzRadiatorTr11"; // Sens Elmt 23,0
  frame2CompositeShapeBoolean +=
      " - quartzRadiator:quartzRadiatorTr12)"; // Sens Elmt 22,1

  // remove the PMT shapes for the sockets
  frame2CompositeShapeBoolean += " - PMT:PMTTr7";
  frame2CompositeShapeBoolean += " - PMT:PMTTr8";
  frame2CompositeShapeBoolean += " - PMT:PMTTr9";
  frame2CompositeShapeBoolean += " - PMT:PMTTr10";
  frame2CompositeShapeBoolean += " - PMT:PMTTr11";
  frame2CompositeShapeBoolean += " - PMT:PMTTr12)";

  return frame2CompositeShapeBoolean;
}
// Support structure L-shape element definition
std::string AliFITv8::frameCompositeShapeBoolean() {
  // create a string for the boolean operations for the composite plateGroup
  // shape
  std::string frameCompositeShapeBoolean = "";

  // add the two pieces called frame 1 and 2 into a single L-shaped element
  frameCompositeShapeBoolean += "frameRemovedPMTandRadiators1:frameTr1";
  frameCompositeShapeBoolean += " + frameRemovedPMTandRadiators2:frameTr2";

  // add the plateGroups to the L-shaped elements
  frameCompositeShapeBoolean += " + plateGroup:plateGroupTr1";
  frameCompositeShapeBoolean += " + plateGroup:plateGroupTr2";

  // subtract the extra Al from the L-shaped elements
  frameCompositeShapeBoolean += " - rect1:rectTr1";
  frameCompositeShapeBoolean += " - rect2:rectTr2";
  frameCompositeShapeBoolean += " - rect3:rectTr3";
  frameCompositeShapeBoolean += " - rect4:rectTr4";
  frameCompositeShapeBoolean += " - rect5:rectTr5";
  frameCompositeShapeBoolean += " - rect6:rectTr6";
  frameCompositeShapeBoolean += " - rect7:rectTr7";
  frameCompositeShapeBoolean += " - rect8:rectTr8";

  return frameCompositeShapeBoolean;
}

// Plate group elements
std::string AliFITv8::plateGroupCompositeShapeBoolean() {
  // create a string for the boolean operations for the composite plateGroup
  // shape
  std::string plateGroupCompositeShapeBoolean = "";

  // add the plateBoxes to the plateGroup
  plateGroupCompositeShapeBoolean += "plateBox:plateTr1";
  plateGroupCompositeShapeBoolean += " + plateBox:plateTr2";
  plateGroupCompositeShapeBoolean += " + plateBox:plateTr3";

  return plateGroupCompositeShapeBoolean;
}

// Optical fiber plate for the first aluminum box in the L-shaped element
std::string AliFITv8::opticalFiberPlateCompositeShapeBoolean1() {
  // create a string for the boolean operations for the composite
  // opticalFiberPlate1 shape
  std::string opticalFiberPlateCompositeShapeBoolean1 = "";
  opticalFiberPlateCompositeShapeBoolean1 += "cablePlate";
  opticalFiberPlateCompositeShapeBoolean1 +=
      " - opticalFiberHead:opticalFiberHeadTr1";
  opticalFiberPlateCompositeShapeBoolean1 +=
      " - opticalFiberHead:opticalFiberHeadTr2";
  opticalFiberPlateCompositeShapeBoolean1 +=
      " - opticalFiberHead:opticalFiberHeadTr3";
  opticalFiberPlateCompositeShapeBoolean1 +=
      " - opticalFiberHead:opticalFiberHeadTr4";

  return opticalFiberPlateCompositeShapeBoolean1;
}
// Optical fiber plate for the second aluminum box in the L-shaped element
std::string AliFITv8::opticalFiberPlateCompositeShapeBoolean2() {
  // create a string for the boolean operations for the composite
  // opticalFiberPlate2 shape
  std::string opticalFiberPlateCompositeShapeBoolean2 = "";

  // remove the opticalFiberHead shapes from the cablePlate
  opticalFiberPlateCompositeShapeBoolean2 += "cablePlate";
  opticalFiberPlateCompositeShapeBoolean2 +=
      " - opticalFiberHead:opticalFiberHeadTr5";
  opticalFiberPlateCompositeShapeBoolean2 +=
      " - opticalFiberHead:opticalFiberHeadTr6";
  opticalFiberPlateCompositeShapeBoolean2 +=
      " - opticalFiberHead:opticalFiberHeadTr7";
  opticalFiberPlateCompositeShapeBoolean2 +=
      " - opticalFiberHead:opticalFiberHeadTr8";

  return opticalFiberPlateCompositeShapeBoolean2;
}

// Create rounded PMT socket corners
std::string AliFITv8::pmtCornerCompositeShapeBoolean() {
  // create a string for the boolean operations for the composite pmtCorner
  // shape
  std::string pmtCornerCompositeShapeBoolean = "";
  pmtCornerCompositeShapeBoolean += "pmtCornerRect:pmtCornerRectTr";
  pmtCornerCompositeShapeBoolean += " - pmtCornerTube:pmtCornerTubeTr";

  return pmtCornerCompositeShapeBoolean;
}

// Create PMT socket shape
std::string AliFITv8::pmtCompositeShapeBoolean() {
  // create a string for the boolean operations for the composite PMT shape
  std::string pmtCompositeShapeBoolean = "";
  pmtCompositeShapeBoolean += "pmtBox";
  pmtCompositeShapeBoolean += " - PMTCorner:PMTCornerTr1";
  pmtCompositeShapeBoolean += " - PMTCorner:PMTCornerTr2";
  pmtCompositeShapeBoolean += " - PMTCorner:PMTCornerTr3";
  pmtCompositeShapeBoolean += " - PMTCorner:PMTCornerTr4";

  return pmtCompositeShapeBoolean;
}
// Plate composite structure
std::string AliFITv8::plateBoxCompositeShapeBoolean() {
  // create a string for the boolean operations for the composite plateBox shape
  std::string plateBoxCompositeShapeBoolean = "";
  plateBoxCompositeShapeBoolean += "basicPlate";
  plateBoxCompositeShapeBoolean += " + basicPlate:basicPlateTr";
  plateBoxCompositeShapeBoolean += " + opticalFiberPlate1:opticalFiberPlateTr1";
  plateBoxCompositeShapeBoolean += " + opticalFiberPlate2:opticalFiberPlateTr2";

  return plateBoxCompositeShapeBoolean;
}

// Wrapper function to define all support structure transformations at once
void AliFITv8::defineTransformations() {
  defineQuartzRadiatorTransformations();
  definePmtTransformations();
  definePlateTransformations();
  defineFrameTransformations();
}
void AliFITv8::defineQuartzRadiatorTransformations() {
  // translations for quartz radiator shapes to be removed from the frame2 pice
  // of the L-shaped element
  TGeoTranslation *quartzRadiatorTr1 = new TGeoTranslation(
      "quartzRadiatorTr1", sPos1X[0], sPos1Y[0], sQuartzHeight);
  quartzRadiatorTr1->RegisterYourself();
  TGeoTranslation *quartzRadiatorTr2 = new TGeoTranslation(
      "quartzRadiatorTr2", sPos1X[0], sPos1Y[1], sQuartzHeight);
  quartzRadiatorTr2->RegisterYourself();
  TGeoTranslation *quartzRadiatorTr3 = new TGeoTranslation(
      "quartzRadiatorTr3", sPos1X[1], sPos1Y[2], sQuartzHeight);
  quartzRadiatorTr3->RegisterYourself();
  TGeoTranslation *quartzRadiatorTr4 = new TGeoTranslation(
      "quartzRadiatorTr4", sPos1X[1], sPos1Y[3], sQuartzHeight);
  quartzRadiatorTr4->RegisterYourself();
  TGeoTranslation *quartzRadiatorTr5 = new TGeoTranslation(
      "quartzRadiatorTr5", sPos1X[2], sPos1Y[2], sQuartzHeight);
  quartzRadiatorTr5->RegisterYourself();
  TGeoTranslation *quartzRadiatorTr6 = new TGeoTranslation(
      "quartzRadiatorTr6", sPos1X[2], sPos1Y[3], sQuartzHeight);
  quartzRadiatorTr6->RegisterYourself();
  // translations for quartz radiator shapes to be removed from the frame1 piece
  // of the L-shaped element
  TGeoTranslation *quartzRadiatorTr7 = new TGeoTranslation(
      "quartzRadiatorTr7", sPos2X[0], sPos2Y[0], sQuartzHeight);
  quartzRadiatorTr7->RegisterYourself();
  TGeoTranslation *quartzRadiatorTr8 = new TGeoTranslation(
      "quartzRadiatorTr8", sPos2X[1], sPos2Y[0], sQuartzHeight);
  quartzRadiatorTr8->RegisterYourself();
  TGeoTranslation *quartzRadiatorTr9 = new TGeoTranslation(
      "quartzRadiatorTr9", sPos2X[2], sPos2Y[1], sQuartzHeight);
  quartzRadiatorTr9->RegisterYourself();
  TGeoTranslation *quartzRadiatorTr10 = new TGeoTranslation(
      "quartzRadiatorTr10", sPos2X[3], sPos2Y[1], sQuartzHeight);
  quartzRadiatorTr10->RegisterYourself();
  TGeoTranslation *quartzRadiatorTr11 = new TGeoTranslation(
      "quartzRadiatorTr11", sPos2X[2], sPos2Y[2], sQuartzHeight);
  quartzRadiatorTr11->RegisterYourself();
  TGeoTranslation *quartzRadiatorTr12 = new TGeoTranslation(
      "quartzRadiatorTr12", sPos2X[3], sPos2Y[2], sQuartzHeight);
  quartzRadiatorTr12->RegisterYourself();
}
// Transformations for PMT sockets, including rounded corners
void AliFITv8::definePmtTransformations() {
  // translations for PMT shapes to be removed from the frame2 piece in the
  // L-shaped element
  TGeoTranslation *PMTTr1 =
      new TGeoTranslation("PMTTr1", sPos1X[0], sPos1Y[0], sPmtHeight);
  PMTTr1->RegisterYourself();
  TGeoTranslation *PMTTr2 =
      new TGeoTranslation("PMTTr2", sPos1X[0], sPos1Y[1], sPmtHeight);
  PMTTr2->RegisterYourself();
  TGeoTranslation *PMTTr3 =
      new TGeoTranslation("PMTTr3", sPos1X[1], sPos1Y[2], sPmtHeight);
  PMTTr3->RegisterYourself();
  TGeoTranslation *PMTTr4 =
      new TGeoTranslation("PMTTr4", sPos1X[1], sPos1Y[3], sPmtHeight);
  PMTTr4->RegisterYourself();
  TGeoTranslation *PMTTr5 =
      new TGeoTranslation("PMTTr5", sPos1X[2], sPos1Y[2], sPmtHeight);
  PMTTr5->RegisterYourself();
  TGeoTranslation *PMTTr6 =
      new TGeoTranslation("PMTTr6", sPos1X[2], sPos1Y[3], sPmtHeight);
  PMTTr6->RegisterYourself();
  // translations for PMT shapes to be removed from the frame1 piece in the
  // L-shaped element
  TGeoTranslation *PMTTr7 =
      new TGeoTranslation("PMTTr7", sPos2X[0], sPos2Y[0], sPmtHeight);
  PMTTr7->RegisterYourself();
  TGeoTranslation *PMTTr8 =
      new TGeoTranslation("PMTTr8", sPos2X[1], sPos2Y[0], sPmtHeight);
  PMTTr8->RegisterYourself();
  TGeoTranslation *PMTTr9 =
      new TGeoTranslation("PMTTr9", sPos2X[2], sPos2Y[1], sPmtHeight);
  PMTTr9->RegisterYourself();
  TGeoTranslation *PMTTr10 =
      new TGeoTranslation("PMTTr10", sPos2X[3], sPos2Y[1], sPmtHeight);
  PMTTr10->RegisterYourself();
  TGeoTranslation *PMTTr11 =
      new TGeoTranslation("PMTTr11", sPos2X[2], sPos2Y[2], sPmtHeight);
  PMTTr11->RegisterYourself();
  TGeoTranslation *PMTTr12 =
      new TGeoTranslation("PMTTr12", sPos2X[3], sPos2Y[2], sPmtHeight);
  PMTTr12->RegisterYourself();
  // define pmtCorner transformations
  TGeoTranslation *pmtCornerTubeTr = new TGeoTranslation(
      "pmtCornerTubeTr", sPmtCornerTubePos, sPmtCornerTubePos, 0);
  pmtCornerTubeTr->RegisterYourself();
  TGeoTranslation *pmtCornerRectTr =
      new TGeoTranslation("pmtCornerRectTr", 0, 0, 0);
  pmtCornerRectTr->RegisterYourself();
  TGeoTranslation *PMTCornerTr1 =
      new TGeoTranslation("PMTCornerTr1", sPmtCornerPos, sPmtCornerPos, 0);
  PMTCornerTr1->RegisterYourself();
  TGeoRotation *reflect2 = new TGeoRotation();
  reflect2->ReflectX(true);
  reflect2->RegisterYourself();
  TGeoCombiTrans *PMTCornerTr2 = new TGeoCombiTrans(
      "PMTCornerTr2", -sPmtCornerPos, sPmtCornerPos, 0, reflect2);
  PMTCornerTr2->RegisterYourself();
  TGeoRotation *reflect3 = new TGeoRotation();
  reflect3->ReflectX(true);
  reflect3->ReflectY(true);
  reflect3->RegisterYourself();
  TGeoCombiTrans *PMTCornerTr3 = new TGeoCombiTrans(
      "PMTCornerTr3", -sPmtCornerPos, -sPmtCornerPos, 0, reflect3);
  PMTCornerTr3->RegisterYourself();
  TGeoRotation *reflect4 = new TGeoRotation();
  reflect4->ReflectY(true);
  reflect4->RegisterYourself();
  TGeoCombiTrans *PMTCornerTr4 = new TGeoCombiTrans(
      "PMTCornerTr4", sPmtCornerPos, -sPmtCornerPos, 0, reflect4);
  PMTCornerTr4->RegisterYourself();
  TGeoRotation *reflect5 = new TGeoRotation();
  reflect5->ReflectX(true);
  reflect5->ReflectY(true);
  reflect5->RegisterYourself();
  TGeoCombiTrans *edgeCornerTr = new TGeoCombiTrans(
      "edgeCornerTr", sEdgeCornerPos[0], sEdgeCornerPos[1], 0, reflect5);
  edgeCornerTr->RegisterYourself();
} // Transformations for plate elements
void AliFITv8::definePlateTransformations() {
  // TODO: redefine fiber head transformations
  // TODO: move hard-coded numbers to be variables in the constants lists
  // define transformations for the fiber heads in opticalFiberPlate1
  TGeoTranslation *opticalFiberHeadTr1 =
      new TGeoTranslation("opticalFiberHeadTr1", 1.7384, 1.36, 0);
  opticalFiberHeadTr1->RegisterYourself();
  TGeoTranslation *opticalFiberHeadTr2 =
      new TGeoTranslation("opticalFiberHeadTr2", 1.7384, -1.36, 0);
  opticalFiberHeadTr2->RegisterYourself();
  TGeoCombiTrans *opticalFiberHeadTr3 =
      new TGeoCombiTrans("opticalFiberHeadTr3", -0.9252, -.9375, 0,
                         new TGeoRotation("rot3", 15, 0, 0));
  opticalFiberHeadTr3->RegisterYourself();
  TGeoCombiTrans *opticalFiberHeadTr4 =
      new TGeoCombiTrans("opticalFiberHeadTr4", -0.9252, .9375, 0,
                         new TGeoRotation("rot4", -15, 0, 0));
  opticalFiberHeadTr4->RegisterYourself();

  // make the transformations for the fiber heads in opticalFiberPlate2
  TGeoCombiTrans *opticalFiberHeadTr5 =
      new TGeoCombiTrans("opticalFiberHeadTr5", 1.6714, 1.525, 0,
                         new TGeoRotation("rot5", 30, 0, 0));
  opticalFiberHeadTr5->RegisterYourself();
  TGeoCombiTrans *opticalFiberHeadTr6 =
      new TGeoCombiTrans("opticalFiberHeadTr6", 1.6714, -1.525, 0,
                         new TGeoRotation("rot6", -30, 0, 0));
  opticalFiberHeadTr6->RegisterYourself();
  TGeoCombiTrans *opticalFiberHeadTr7 =
      new TGeoCombiTrans("opticalFiberHeadTr7", -0.9786, -1.125, 0,
                         new TGeoRotation("rot7", 30, 0, 0));
  opticalFiberHeadTr7->RegisterYourself();
  TGeoCombiTrans *opticalFiberHeadTr8 =
      new TGeoCombiTrans("opticalFiberHeadTr8", -0.9786, 1.125, 0,
                         new TGeoRotation("rot8", -30, 0, 0));
  opticalFiberHeadTr8->RegisterYourself();
  // define transformations to form a plateBox (2 basicPlates and 2 cablePlates)
  TGeoCombiTrans *basicPlateTr =
      new TGeoCombiTrans("basicPlateTr", 0, -sPlateSpacing, 0,
                         new TGeoRotation("basicPlateRot", 90, 0, 0));
  basicPlateTr->RegisterYourself();
  TGeoCombiTrans *opticalFiberPlateTr1 =
      new TGeoCombiTrans("opticalFiberPlateTr1", 0, 0, sOpticalFiberPlateZ,
                         new TGeoRotation("opticalFiberPlateRot1", 90, 0, 0));
  opticalFiberPlateTr1->RegisterYourself();
  TGeoCombiTrans *opticalFiberPlateTr2 = new TGeoCombiTrans(
      "opticalFiberPlateTr2", 0, -sPlateSpacing, sOpticalFiberPlateZ,
      new TGeoRotation("opticalFiberPlateRot2", 90, 0, 0));
  opticalFiberPlateTr2->RegisterYourself();

  // define transformations to form a plateGroup
  TGeoTranslation *plateTr1 = new TGeoTranslation("plateTr1", -sPlateSpacing,
                                                  sPlateDisplacementDeltaY, 0);
  plateTr1->RegisterYourself();
  TGeoTranslation *plateTr2 = new TGeoTranslation("plateTr2", 0, 0, 0);
  plateTr2->RegisterYourself();
  TGeoTranslation *plateTr3 =
      new TGeoTranslation("plateTr3", sPlateSpacing, 0, 0);
  plateTr3->RegisterYourself();
  // TODO: fix plateGroupTr2
  // TODO: Move hard-coded numbers to variables defined in the constants list
  // define transformations for the plateGroups (6 basicPlates and 6
  // cablePlates)
  TGeoTranslation *plateGroupTr1 = new TGeoTranslation(
      "plateGroupTr1", sPlateDisplacementX, sPlateDisplacementY, sPlateGroupZ);
  plateGroupTr1->RegisterYourself();
  TGeoCombiTrans *plateGroupTr2 = new TGeoCombiTrans(
      "plateGroupTr2", 10.4358 + 1.5 * sPlateDisplacementDeltaY, -7.0747,
      sPlateGroupZ, new TGeoRotation("plateGroup2Rotation", -90, 0, 0));
  plateGroupTr2->RegisterYourself();
}
// Transformations for the L-shaped elements
void AliFITv8::defineFrameTransformations() {

  // TODO: Confirm shifts that eliminate internal overlaps do not then cause
  //       overlaps with FV0 or other elements
  // TODO: Move these hard-coded numbers to be variables in the list of
  // constants
  Float_t zshift = .2741;
  Float_t rectShift = .274101;
  Float_t frameXshift = -.1009;

  // position of the two rectangles used to approximate the L-shaped frame
  // element
  TGeoTranslation *frameTr1 = new TGeoTranslation(
      "frameTr1", sFrame1PosX + frameXshift, sFrame1PosY, 0 + zshift);
  frameTr1->RegisterYourself();
  TGeoTranslation *frameTr2 = new TGeoTranslation(
      "frameTr2", sFrame2PosX + frameXshift, sFrame2PosY, 0 + zshift);
  frameTr2->RegisterYourself();

  // remove the two smaller rectangles from the L-shaped frame element
  TGeoTranslation *rectTr1 = new TGeoTranslation(
      "rectTr1", sFrame1PosX + sXoffset + frameXshift + 3.25,
      sFrame1PosY + sYoffset + 6.1875, 0 + zshift);
  rectTr1->RegisterYourself();

  TGeoTranslation *rectTr2 = new TGeoTranslation(
      "rectTr2", sFrame1PosX + sXoffset + frameXshift + 9.3,
      sFrame1PosY + sYoffset - 0.5775, sMountZ / 2 + zshift);
  rectTr2->RegisterYourself();

  TGeoTranslation *rectTr3 = new TGeoTranslation(
      "rectTr3", sFrame1PosX + sXoffset + frameXshift + 10.75 - sRect3X / 2,
      sFrame1PosY + sYoffset - 6.8525 + sRect3Y / 2, 0 + zshift);
  rectTr3->RegisterYourself();
  TGeoTranslation *rectTr4 = new TGeoTranslation(
      "rectTr4", sFrame1PosX + sXoffset + frameXshift - 7.925,
      sFrame1PosY + sYoffset - 6.44, 0 + zshift + 10);
  rectTr4->RegisterYourself();

  TGeoTranslation *rectTr5 = new TGeoTranslation(
      "rectTr5", sFrame2PosX + sXoffset + frameXshift + 6.965 + sRect5X / 2,
      sFrame2PosY + sYoffset + 4.3625 - sRect5Y / 2, 0 + zshift + rectShift);
  rectTr5->RegisterYourself();

  TGeoTranslation *rectTr6 = new TGeoTranslation(
      "rectTr6", sFrame2PosX + sXoffset + frameXshift + 6.965 - sRect6X / 2,
      sFrame2PosY + sYoffset - 10.7375 + sRect6Y / 2, 0 + zshift);
  rectTr6->RegisterYourself();

  TGeoTranslation *rectTr7 = new TGeoTranslation(
      "rectTr7",
      sFrame2PosX + sXoffset + frameXshift + 6.965 - sRect6X - sRect7X / 2,
      sFrame2PosY + sYoffset - 10.7375 + sRect7Y / 2, sMountZ / 2 + zshift);
  rectTr7->RegisterYourself();

  TGeoTranslation *rectTr8 = new TGeoTranslation(
      "rectTr8", sFrame2PosX + sXoffset + frameXshift - 5.89 - sRect8X / 2,
      sFrame2PosY + sYoffset + 5.1125 + sRect8Y / 2, 0 + zshift);
  rectTr8->RegisterYourself();
}

void AliFITv8::AddAlignableVolumes() const {
  // Create entries for alignable volumes associating the symbolic volume
  // name with the corresponding volume path. Needs to be synchronized with
  // eventual changes in the geometry.

  TString volPath;
  TString symName, sn;
  TString vpAalign = "/ALIC_1/0STL_1";
  TString vpCalign = "/ALIC_1/0STR_1";
  for (Int_t imod = 0; imod < 2; imod++) {
    if (imod == 0) {
      volPath = vpCalign;
      symName = "/ALIC_1/0STL";
    }
    if (imod == 1) {
      volPath = vpAalign;
      symName = "/ALIC_1/0STR";
    }

    AliDebug(2, "--------------------------------------------");
    AliDebug(2, Form("volPath=%s\n", volPath.Data()));
    AliDebug(2, Form("symName=%s\n", symName.Data()));
    AliDebug(2, "--------------------------------------------");
    if (!gGeoManager->SetAlignableEntry(symName.Data(), volPath.Data())) {
      AliFatal(Form("Alignable entry %s not created. Volume path %s not valid",
                    symName.Data(), volPath.Data()));
    }
  }
}
//------------------------------------------------------------------------

void AliFITv8::CreateMaterials() {
  Int_t isxfld =
      ((AliMagF *)TGeoGlobalMagField::Instance()->GetField())->Integ();
  Float_t sxmgmx =
      ((AliMagF *)TGeoGlobalMagField::Instance()->GetField())->Max();

  // Float_t a,z,d,radl,absl,buf[1];
  // Int_t nbuf;
  Float_t aAir[4] = {12.0107, 14.0067, 15.9994, 39.948};
  Float_t zAir[4] = {6., 7., 8., 18.};
  Float_t wAir[4] = {0.000124, 0.755267, 0.231781, 0.012827};
  Float_t dAir = 1.20479E-3;
  Float_t dAir1 = 1.20479E-11;
  // Radiator  glass SiO2
  Float_t aglass[2] = {28.0855, 15.9994};
  Float_t zglass[2] = {14., 8.};
  Float_t wglass[2] = {1., 2.};
  Float_t dglass = 2.2;
  // MCP glass SiO2
  Float_t dglass_mcp = 1.3;
  /* Ceramic   97.2% Al2O3 , 2.8% SiO2 : average material for
   -  stack of 2 MCPs thickness 2mm with density 1.6 g/cm3
   -  back wall of MCP thickness 2 mm with density 2.4 g/cm3
   -  MCP electrods thickness 1 mm with density 4.2 g/cm3
   -  Backplane PCBs thickness 4.5 mm with density 1.85 g/cm3
   -  electromagnetic shielding 1 mm  with density 2.8 g/cm3
   -  Al back cover 5mm  2.7 g/cm3
  */
  Float_t aCeramic[2] = {26.981539, 15.9994};
  Float_t zCeramic[2] = {13., 8.};
  Float_t wCeramic[2] = {2., 3.};
  Float_t denscer = 2.37;

  // MCP walls Ceramic+Nickel (50//50)
  const Int_t nCeramicNice = 3;
  Float_t aCeramicNicel[3] = {26.981539, 15.9994, 58.6934};
  Float_t zCeramicNicel[3] = {13., 8., 28};
  Float_t wCeramicNicel[3] = {0.2, 0.3, 0.5};
  Float_t denscerCeramicNickel = 5.6;

  // Mixed Cables material simulated as plastic with density taken from
  // description of Low Loss Microwave Coax24 AWG 0
  //  plastic + cooper (6%)
  Float_t aT0Plast[4] = {1.00784, 12.0107, 15.999, 63.54};
  Float_t zT0Plast[4] = {1, 6, 8, 29};
  Float_t wT0Plast[4] = {0.08, 0.53, 0.22, 0.17}; ////!!!!!
  Float_t denCable = 3.66;

  //*** Definition Of avaible FIT materials ***
  AliMaterial(11, "Aliminium$", 26.98, 13.0, 2.7, 8.9, 999);
  AliMixture(1, "Vacuum$", aAir, zAir, dAir1, 4, wAir);
  AliMixture(2, "Air$", aAir, zAir, dAir, 4, wAir);
  AliMixture(4, "MCP glass   $", aglass, zglass, dglass_mcp, -2, wglass);
  AliMixture(24, "Radiator Optical glass$", aglass, zglass, dglass, -2, wglass);
  AliMixture(3, "Ceramic$", aCeramic, zCeramic, denscer, -2, wCeramic);
  AliMixture(23, "CablePlasticCooper$", aT0Plast, zT0Plast, denCable, 4,
             wT0Plast);
  AliMixture(25, "MCPwalls $", aCeramicNicel, zCeramicNicel,
             denscerCeramicNickel, 3, wCeramicNicel);

  AliMedium(1, "Air$", 2, 0, isxfld, sxmgmx, 10., .1, 1., .003, .003);
  AliMedium(3, "Vacuum$", 1, 0, isxfld, sxmgmx, 10., .01, .1, .003, .003);
  AliMedium(4, "Ceramic$", 3, 0, isxfld, sxmgmx, 10., .01, .1, .003, .003);
  AliMedium(6, "Glass$", 4, 0, isxfld, sxmgmx, 10., .01, .1, .003, .003);
  //  Medium(7, "OpAir$", 2, 0, isxfld, sxmgmx, 10., .1, 1., .003, .003);
  //  Medium(18, "OpBlack$", 2, 0, isxfld, sxmgmx, 10., .1, 1., .003, .003);
  AliMedium(15, "Aluminium$", 11, 0, isxfld, sxmgmx, 10., .01, 1., .003, .003);
  AliMedium(17, "OptAluminium$", 11, 0, isxfld, sxmgmx, 10., .01, 1., .003,
            .003);
  AliMedium(16, "OpticalGlass$", 24, 1, isxfld, sxmgmx, 10., .01, .1, .003,
            .01);
  AliMedium(19, "OpticalGlassCathode$", 24, 1, isxfld, sxmgmx, 10., .01, .1,
            .003, .003);
  //  Medium(22, "SensAir$", 2, 1, isxfld, sxmgmx, 10., .1, 1., .003, .003);
  AliMedium(23, "Cables$", 23, 1, isxfld, sxmgmx, 10., .1, 1., .003, .003);
  AliMedium(25, "MCPWalls", 25, 1, isxfld, sxmgmx, 10., .1, 1., .003, .003);

  // V0+

  // Parameters for simulation scope
  Int_t FieldType = ((AliMagF *)TGeoGlobalMagField::Instance()->GetField())
                        ->Integ(); // Field type
  Double_t MaxField = ((AliMagF *)TGeoGlobalMagField::Instance()->GetField())
                          ->Max(); // Field max.
  Double_t MaxBending = 10;        // Max Angle
  Double_t MaxStepSize = 0.01;     // Max step size
  Double_t MaxEnergyLoss = 1;      // Max Delta E
  Double_t Precision = 0.003;      // Precision
  Double_t MinStepSize = 0.003;    // Minimum step size

  Int_t Id = 8;
  Double_t A, Z, RadLength, AbsLength;
  Float_t Density, as[4], zs[4], ws[4];

  ///// Parameters for V0Plusscintilator: BC404
  // as[0] = 1.00794;	as[1] = 12.011;
  // zs[0] = 1.;		zs[1] = 6.;
  // ws[0] = 5.21;	ws[1] = 4.74;
  // Density      = 1.032;
  // AliMixture(Id, "V0PlusSci", as, zs, Density, -2, ws);
  // AliMedium(Id,  "V0PlusSci", Id, 1, FieldType, MaxField, MaxBending,
  // MaxStepSize, MaxEnergyLoss, Precision, MinStepSize);

  ////// V0 Scintillator material for Run-3
  //... Rihan: EJ-204 scintillator, based on polyvinyltoluene
  const Int_t nScint = 2;
  Float_t aScint[nScint] = {1.00784, 12.0107};
  Float_t zScint[nScint] = {1, 6};
  // TODO: Verify which of the following 2 lines is correct
  Float_t wScint[nScint] = {0.07085,
                            0.92915}; // based on EJ-204 datasheet: n_atoms/cm3
  // Float_t wScint[nScint] = { 0.08528, 0.91472 }; // based on chemical
  // composition of base: polyvinyltoluene
  const Float_t dScint = 1.023;
  AliMixture(Id, "V0PlusSci", aScint, zScint, dScint, -2, wScint);
  AliMedium(Id, "V0PlusSci", Id, 1, FieldType, MaxField, MaxBending,
            MaxStepSize, MaxEnergyLoss, Precision, MinStepSize);

  //*******************************************************************

  // Log::Rihan,10Nov2019 => Added V0 plastic material. See
  // O2/AliFITv8s/FIT/FV0/simulation/src/AliFITv8.cxx for details!! PMMA plastic
  // mixture: (C5O2H8)n, same for plastic fiber support and for the fiber core
  // Fiber cladding is different, but it comprises only 3% of the fiber volume,
  // so it is not included

  Int_t unsens = 0; // unsensitive medium
  Int_t sens = 1;   // sensitive  medium

  Int_t PasticId = 9; /// Rihan:I Randomly choose 9 (actually this is 'not
                      /// occupied' in the list above!).

  const Int_t nPlast = 3;
  float aPlast[nPlast] = {1.00784, 12.0107, 15.999};
  float zPlast[nPlast] = {1.00000, 6.0000, 8.000};
  float wPlast[nPlast] = {0.08054, 0.59985, 0.31961};
  float densityPlast = 1.18;

  // std::cout<<"\n ******** Debug:: Creating Plastic medium ********* \n
  // "<<std::endl;
  AliMixture(PasticId, "V0Plastic", aPlast, zPlast, densityPlast, 3, wPlast);
  AliMedium(PasticId, "V0Plastic", PasticId, unsens, FieldType, MaxField,
            MaxBending, MaxStepSize, MaxEnergyLoss, Precision, MinStepSize);

  ///// Log:Rihan,10Nov2019 => Added Optical Fiber.  See
  /// O2/AliFITv8s/FIT/FV0/simulation/src/AliFITv8.cxx for details!!
  // Densities of fiber-equivalent material, for 3 radially-distributed density
  // regions
  float dFiberInner = 0.087;
  float dFiberMiddle = 0.129;
  float dFiberOuter = 0.049;

  Int_t materiaId = 31;
  AliMixture(materiaId, "FiberInner", aPlast, zPlast, dFiberInner, 3, wPlast);
  AliMedium(materiaId, "FiberInner", materiaId, unsens, FieldType, MaxField,
            MaxBending, MaxStepSize, MaxEnergyLoss, Precision, MinStepSize);

  materiaId++;
  AliMixture(materiaId, "FiberMiddle", aPlast, zPlast, dFiberMiddle, 3, wPlast);
  AliMedium(materiaId, "FiberMiddle", materiaId, unsens, FieldType, MaxField,
            MaxBending, MaxStepSize, MaxEnergyLoss, Precision, MinStepSize);

  materiaId++;
  AliMixture(materiaId, "FiberOuter", aPlast, zPlast, dFiberOuter, 3, wPlast);
  AliMedium(materiaId, "FiberOuter", materiaId, unsens, FieldType, MaxField,
            MaxBending, MaxStepSize, MaxEnergyLoss, Precision, MinStepSize);

  // Titanium grade 5 (https://en.wikipedia.org/wiki/Titanium_alloy) without
  // iron and oxygen
  Float_t aTitanium[3] = {47.87, 26.98, 50.94};
  Float_t zTitanium[3] = {22, 13, 23};
  Float_t wTitanium[3] = {0.9, 0.06, 0.04};
  const Float_t dTitanium = 4.42;
  // std::cout<<"\n ******** Debug:: Creating Titanium medium ********* \n
  // "<<std::endl;

  materiaId++;
  AliMixture(materiaId, "Titanium", aTitanium, zTitanium, dTitanium, 3,
             wTitanium);
  AliMedium(materiaId, "Titanium", materiaId, unsens, FieldType, MaxField,
            MaxBending, MaxStepSize, MaxEnergyLoss, Precision, MinStepSize);

  // Rihan:: take Aluminium properties from O2:::
  /*
  // Aluminum
  Float_t aAlu = 26.981;
  Float_t zAlu = 13;
  Float_t dAlu = 2.7;

  materiaId++;
  AliMaterial(materiaId,"Aluminium", aAlu, zAlu, dAlu, 8.9, 999);
  materiaId++;
  AliMedium(materiaId, "Aluminium", materiaId, unsens, FieldType, MaxField,
  MaxBending, MaxStepSize, MaxEnergyLoss, Precision, MinStepSize);
  */

  std::cout << "\n AliFITv8:: ===> Debug:: Created All Medium Successfully "
               "------------ \n "
            << std::endl;

  AliDebugClass(1, ": ++++++++++++++Medium set++++++++++");
}
//-------------------------------------------------------------------

void AliFITv8::DefineOpticalProperties() {
  // Path of the optical properties input file
  TString optPropPath = "$(ALICE_ROOT)/FIT/sim/quartzOptProperties.txt";
  optPropPath = gSystem->ExpandPathName(
      optPropPath.Data()); // Expand $(ALICE_ROOT) into real system path

  // Optical properties definition.
  Int_t *idtmed = fIdtmed->GetArray();

  // Prepare pointers for arrays read from the input file
  Float_t *aPckov = NULL;
  Double_t *dPckov = NULL;
  Float_t *aAbsSiO2 = NULL;
  Float_t *rindexSiO2 = NULL;
  Float_t *qeff = NULL;
  Int_t kNbins = 0;
  ReadOptProperties(optPropPath.Data(), &aPckov, &dPckov, &aAbsSiO2,
                    &rindexSiO2, &qeff, kNbins);
  fPMTeff = new TGraph(kNbins, aPckov, qeff); // set QE

  // Prepare pointers for arrays with constant and hardcoded values (independent
  // of wavelength)
  Float_t *efficAll = NULL;
  Float_t *rindexAir = NULL;
  Float_t *absorAir = NULL;
  Float_t *rindexCathodeNext = NULL;
  Float_t *absorbCathodeNext = NULL;
  Double_t *efficMet = NULL;
  Double_t *aReflMet = NULL;
  FillOtherOptProperties(&efficAll, &rindexAir, &absorAir, &rindexCathodeNext,
                         &absorbCathodeNext, &efficMet, &aReflMet, kNbins);

  TVirtualMC::GetMC()->SetCerenkov(idtmed[kOpGlass], kNbins, aPckov, aAbsSiO2,
                                   efficAll, rindexSiO2);
  // TVirtualMC::GetMC()->SetCerenkov (idtmed[kOpGlassCathode], kNbins, aPckov,
  // aAbsSiO2, effCathode, rindexSiO2);
  TVirtualMC::GetMC()->SetCerenkov(idtmed[kOpGlassCathode], kNbins, aPckov,
                                   aAbsSiO2, efficAll, rindexSiO2);
  // TVirtualMC::GetMC()->SetCerenkov (idtmed[kOpAir],       kNbins, aPckov,
  // absorAir ,efficAll, rindexAir); TVirtualMC::GetMC()->SetCerenkov
  // (idtmed[kOpAirNext],   kNbins, aPckov, absorbCathodeNext, efficAll,
  // rindexCathodeNext);

  // Define a border for radiator optical properties
  // TODO: Maciek: The following 3 lines just generate warnings and do nothing
  // else - could be deleted
  //               - for now I only comment them out
  // TVirtualMC::GetMC()->DefineOpSurface("surfRd", kUnified
  // /*kGlisur*/,kDielectric_metal,kPolished, 0.);
  // TVirtualMC::GetMC()->SetMaterialProperty("surfRd", "EFFICIENCY", kNbins,
  // dPckov, efficMet); TVirtualMC::GetMC()->SetMaterialProperty("surfRd",
  // "REFLECTIVITY", kNbins, dPckov, aReflMet);

  DeleteOptPropertiesArr(&aPckov, &dPckov, &aAbsSiO2, &rindexSiO2, &efficAll,
                         &rindexAir, &absorAir, &rindexCathodeNext,
                         &absorbCathodeNext, &efficMet, &aReflMet);
}
//-------------------------------------------------------------------

void AliFITv8::Init() {
  AliFIT::Init();

  fIdSens1 =
      TVirtualMC::GetMC()->VolId("0REG"); // <--- 0REG is sensitive volume Id?

  ////// Rihan:New sensitive volume ==>
  for (Int_t Sec = 0; Sec < 8; Sec++) {
    for (Int_t Ring = 0; Ring < sNumberOfCellRings; Ring++) {
      if (Sec == 0 || Sec == 3 || Sec == 4 || Sec == 7) {
        fIdV0Plus[Sec][Ring] = TVirtualMC::GetMC()->VolId(
            Form("FV0SCINTCELLa%dSector%d", Ring,
                 Sec)); //// *** Rihan: new Scint cells added!!
        std::cout << "Info::Init()"
                  << " Sector#" << Sec + 1 << " Ring #" << Ring + 1
                  << "\t MC id# " << fIdV0Plus[Sec][Ring]
                  << " cell = " << Form("FV0SCINTCELLa%dSector%d", Ring, Sec)
                  << std::endl;
      } else {
        fIdV0Plus[Sec][Ring] = TVirtualMC::GetMC()->VolId(
            Form("FV0SCINTCELLb%dSector%d", Ring, Sec));
        std::cout << "Info::Init()"
                  << " Sector#" << Sec + 1 << " Ring #" << Ring + 1
                  << "\t MC id# " << fIdV0Plus[Sec][Ring]
                  << " cell = " << Form("FV0SCINTCELLb%dSector%d", Ring, Sec)
                  << std::endl;
      }

      //// Rihan: Initialize map of Scintilator cells to Ring number
      if (Ring == 0)
        mcIdtoRing1map.push_back(fIdV0Plus[Sec][Ring]);
      if (Ring == 1)
        mcIdtoRing2map.push_back(fIdV0Plus[Sec][Ring]);
      if (Ring == 2)
        mcIdtoRing3map.push_back(fIdV0Plus[Sec][Ring]);
      if (Ring == 3)
        mcIdtoRing4map.push_back(fIdV0Plus[Sec][Ring]);
      if (Ring == 4)
        mcIdtoRing5map.push_back(fIdV0Plus[Sec][Ring]);
    }
  }

  AliDebug(1, Form("%s: *** FIT version 1 initialized ***\n", ClassName()));
}
//-------------------------------------------------------------------

void AliFITv8::StepManager() {
  // Called for every step in the FIT AliFITv8
  Int_t id, copy, copy1;
  Float_t hits[13];
  static Int_t vol[3];
  TLorentzVector pos;
  TLorentzVector mom;

  // TClonesArray &lhits = *fHits;

  if (!TVirtualMC::GetMC()->IsTrackAlive())
    return; // particle has disappeared

  id = TVirtualMC::GetMC()->CurrentVolID(copy);

  // printf("T0 :::volumes %i %s \n", id, TVirtualMC::GetMC()->CurrentVolName()
  // );
  // T0+

  if (id == fIdSens1) {

    if (TVirtualMC::GetMC()->IsTrackEntering()) {

      TVirtualMC::GetMC()->CurrentVolOffID(1, copy1);
      vol[1] = copy1;
      vol[0] = copy;

      TVirtualMC::GetMC()->TrackPosition(pos);
      TVirtualMC::GetMC()->TrackMomentum(mom);

      Float_t Pt = TMath::Sqrt(mom.Px() * mom.Px() + mom.Py() * mom.Py());

      TParticle *Particle = gAlice->GetMCApp()->Particle(
          gAlice->GetMCApp()->GetCurrentTrackNumber());
      hits[0] = pos[0];
      hits[1] = pos[1];
      hits[2] = pos[2];

      if (pos[2] < 0) {
        vol[2] = 0;
      } else {
        vol[2] = 1;
      }
      Float_t etot = TVirtualMC::GetMC()->Etot();
      hits[3] = etot;

      Int_t iPart = TVirtualMC::GetMC()->TrackPid();
      Int_t partID = TVirtualMC::GetMC()->IdFromPDG(iPart);
      hits[4] = partID;

      Float_t ttime = TVirtualMC::GetMC()->TrackTime();
      hits[5] = ttime * 1e12;
      hits[6] = TVirtualMC::GetMC()->TrackCharge();
      hits[7] = mom.Px();
      hits[8] = mom.Py();
      hits[9] = mom.Pz();
      hits[10] = fSenseless; // Energy loss is sensless for T0+
      hits[11] = fSenseless; // Track length is sensless for T0+
      hits[12] = fSenseless; // Photon production for V0+

      if (TVirtualMC::GetMC()->TrackPid() ==
          50000050) { // If particles is photon then ...
        if (RegisterPhotoE(hits[3])) {
          fIshunt = 2;
          AddHit(gAlice->GetMCApp()->GetCurrentTrackNumber(), vol, hits);
          // Create a track reference at the exit of photocatode
        }
      }

      // charge particle HITS
      if (TVirtualMC::GetMC()->TrackCharge()) {
        fIshunt = 0;
        AddHit(gAlice->GetMCApp()->GetCurrentTrackNumber(), vol, hits);
      }

      // charge particle TrackReference
      if (TVirtualMC::GetMC()->TrackCharge()) {
        AddTrackReference(gAlice->GetMCApp()->GetCurrentTrackNumber(),
                          AliTrackReference::kFIT);
      }
    } // trck entering
  }   // sensitive

  // V0+
  if (!gMC->TrackCharge() || !gMC->IsTrackAlive())
    return; // Only interested in charged and alive tracks

  // Check if there is a hit in any of the V0+ sensitive volumes defined in Init
  Bool_t IsId = kFALSE;

  // Rihan::This is old sector loop
  // for (Int_t i = 0; i < nSectors; i++) {    // rihan:TBE => no. of sectors
  //   for (Int_t j = 0; j < nRings; j++) {    // rihan:TBE => no. of Rings
  //     if (id == fIdV0Plus[i][j]) {
  //       IsId = kTRUE;
  //       break;
  //     }
  //   }
  // }

  // Rihan::This is new sector loop
  for (Int_t i = 0; i < 8; i++) {        // rihan:TBE => no. of sectors
    for (Int_t j = 0; j < nRings; j++) { // rihan:TBE => no. of Rings
      if (id == fIdV0Plus[i][j]) {
        IsId = kTRUE;
        break;
      }
    }
  }

  if (IsId == kTRUE) { /// if the volume is sensitive

    // Defining V0+ ring numbers using the sensitive volumes

    Int_t RingNumber =
        -1; // rihan:TBE => This would change!! as no. of  scint Cell reduced.

    ////Rihan: Map for new Scintilator cells =>
    RingNumber = getRingNumberFromMCcellId(id);

    /// this was the old map:

    if (RingNumber < 1) {
      std::cout << "\n\n  MC cell id = " << id
                << " This volume is not a V0+ Ring \n"
                << std::endl;
    } else if (RingNumber) {

      if (TVirtualMC::GetMC()->IsTrackEntering()) {
        TVirtualMC::GetMC()->TrackPosition(pos);
        TVirtualMC::GetMC()->TrackMomentum(mom);

        Float_t Pt = TMath::Sqrt(mom.Px() * mom.Px() + mom.Py() * mom.Py());
        TParticle *Particle = gAlice->GetMCApp()->Particle(
            gAlice->GetMCApp()->GetCurrentTrackNumber());
        hits[0] = pos[0];
        hits[1] = pos[1];
        hits[2] = pos[2];

        Float_t etot = TVirtualMC::GetMC()->Etot();
        hits[3] = etot;

        Int_t iPart = TVirtualMC::GetMC()->TrackPid();
        Int_t partID = TVirtualMC::GetMC()->IdFromPDG(iPart);
        hits[4] = partID;

        Float_t ttime = TVirtualMC::GetMC()->TrackTime();
        hits[5] = ttime * 1e12;
        hits[6] = TVirtualMC::GetMC()->TrackCharge();
        hits[7] = mom.Px();
        hits[8] = mom.Py();
        hits[9] = mom.Pz();

        //    std::cout << "===> AliFITv8 Debug:: MCTrack has Hit in cell no.= "
        //         << vol[0] << " in V0+ Ring = " << RingNumber << std::endl;

      } // Track entering

      if (gMC->IsTrackExiting() || gMC->IsTrackStop() ||
          gMC->IsTrackDisappeared()) {

        Float_t Eloss, Tlength;
        Float_t EnergyDep = TVirtualMC::GetMC()->Edep();
        Float_t Step =
            TVirtualMC::GetMC()->TrackStep(); // Energy loss in the current step
        Int_t nPhotonsInStep = 0;
        Int_t nPhotons = 0;
        nPhotonsInStep = Int_t(EnergyDep / (fV0PlusLightYield * 1e-9));
        nPhotons =
            nPhotonsInStep - Int_t((Float_t(nPhotonsInStep) *
                                    fV0PlusLightAttenuation * fV0PlusnMeters));
        nPhotons = nPhotons - Int_t(Float_t(nPhotons) * fV0PlusFibToPhot);
        Eloss += EnergyDep;
        Tlength += Step;
        hits[10] = Eloss;
        hits[11] = Tlength;
        hits[12] = nPhotons;

        vol[0] = GetCellId(vol);
        vol[1] = RingNumber;
        vol[2] = 2;

        fIshunt = 0;

        AddHit(gAlice->GetMCApp()->GetCurrentTrackNumber(), vol, hits);
        AddTrackReference(gAlice->GetMCApp()->GetCurrentTrackNumber(),
                          AliTrackReference::kFIT);
        Tlength = 0.0;
        Eloss = 0.0;

        //    std::cout << "===> AliFITv8 Debug:: MCTrack
        //    Exited/Stopped/Dissapeared "
        //              "in cell no.= "
        //          << vol[0] << " in V0+ Ring = " << RingNumber << std::endl;

      } // Track exiting, stopped or disappeared track
    }   // Ring number
  }
}
//------------------------------------------------------------------------

Bool_t AliFITv8::RegisterPhotoE(Double_t energy) {
  Float_t eff = fPMTeff->Eval(energy);
  Double_t p = gRandom->Rndm();

  if (p > eff) {
    return kFALSE;
  }
  return kTRUE;
}
//-------------------------------------------------------------------------

int AliFITv8::getRingNumberFromMCcellId(int gMCcellId) {
  int ringNum = -999;

  // try first Ring #1:
  for (int i = 0; i < 8; i++) {
    if (gMCcellId == mcIdtoRing1map[i]) {
      ringNum = 1;
      break;
    }
  }
  // now Try Ring#2
  if (ringNum < 0) {
    for (int i = 0; i < 8; i++) {
      if (gMCcellId == mcIdtoRing2map[i]) {
        ringNum = 2;
        break;
      }
    }
  }
  // now Try Ring#3
  if (ringNum < 0) {
    for (int i = 0; i < 8; i++) {
      if (gMCcellId == mcIdtoRing3map[i]) {
        ringNum = 3;
        break;
      }
    }
  }
  // now Try Ring#4
  if (ringNum < 0) {
    for (int i = 0; i < 8; i++) {
      if (gMCcellId == mcIdtoRing4map[i]) {
        ringNum = 4;
        break;
      }
    }
  }
  // now Try Ring#5
  if (ringNum < 0) {
    for (int i = 0; i < 8; i++) {
      if (gMCcellId == mcIdtoRing5map[i]) {
        ringNum = 5;
        break;
      }
    }
  }

  return ringNum;
}

Int_t AliFITv8::GetCellId(Int_t *vol) {
  fCellId = 0;
  // Ring:1
  if (gMC->CurrentVolID(vol[2]) == gMC->VolId("FV0SCINTCELLa0Sector0"))
    fCellId = 1;
  if (gMC->CurrentVolID(vol[2]) == gMC->VolId("FV0SCINTCELLb0Sector1"))
    fCellId = 2;
  if (gMC->CurrentVolID(vol[2]) == gMC->VolId("FV0SCINTCELLb0Sector2"))
    fCellId = 3;
  if (gMC->CurrentVolID(vol[2]) == gMC->VolId("FV0SCINTCELLa0Sector3"))
    fCellId = 4;
  if (gMC->CurrentVolID(vol[2]) == gMC->VolId("FV0SCINTCELLa0Sector4"))
    fCellId = 5;
  if (gMC->CurrentVolID(vol[2]) == gMC->VolId("FV0SCINTCELLb0Sector5"))
    fCellId = 6;
  if (gMC->CurrentVolID(vol[2]) == gMC->VolId("FV0SCINTCELLb0Sector6"))
    fCellId = 7;
  if (gMC->CurrentVolID(vol[2]) == gMC->VolId("FV0SCINTCELLa0Sector7"))
    fCellId = 8;
  // Ring:2
  if (gMC->CurrentVolID(vol[2]) == gMC->VolId("FV0SCINTCELLa1Sector0"))
    fCellId = 9;
  if (gMC->CurrentVolID(vol[2]) == gMC->VolId("FV0SCINTCELLb1Sector1"))
    fCellId = 10;
  if (gMC->CurrentVolID(vol[2]) == gMC->VolId("FV0SCINTCELLb1Sector2"))
    fCellId = 11;
  if (gMC->CurrentVolID(vol[2]) == gMC->VolId("FV0SCINTCELLa1Sector3"))
    fCellId = 12;
  if (gMC->CurrentVolID(vol[2]) == gMC->VolId("FV0SCINTCELLa1Sector4"))
    fCellId = 13;
  if (gMC->CurrentVolID(vol[2]) == gMC->VolId("FV0SCINTCELLb1Sector5"))
    fCellId = 14;
  if (gMC->CurrentVolID(vol[2]) == gMC->VolId("FV0SCINTCELLb1Sector6"))
    fCellId = 15;
  if (gMC->CurrentVolID(vol[2]) == gMC->VolId("FV0SCINTCELLa1Sector7"))
    fCellId = 16;
  // Ring:3
  if (gMC->CurrentVolID(vol[2]) == gMC->VolId("FV0SCINTCELLa2Sector0"))
    fCellId = 17;
  if (gMC->CurrentVolID(vol[2]) == gMC->VolId("FV0SCINTCELLb2Sector1"))
    fCellId = 18;
  if (gMC->CurrentVolID(vol[2]) == gMC->VolId("FV0SCINTCELLb2Sector2"))
    fCellId = 19;
  if (gMC->CurrentVolID(vol[2]) == gMC->VolId("FV0SCINTCELLa2Sector3"))
    fCellId = 20;
  if (gMC->CurrentVolID(vol[2]) == gMC->VolId("FV0SCINTCELLa2Sector4"))
    fCellId = 21;
  if (gMC->CurrentVolID(vol[2]) == gMC->VolId("FV0SCINTCELLb2Sector5"))
    fCellId = 22;
  if (gMC->CurrentVolID(vol[2]) == gMC->VolId("FV0SCINTCELLb2Sector6"))
    fCellId = 23;
  if (gMC->CurrentVolID(vol[2]) == gMC->VolId("FV0SCINTCELLa2Sector7"))
    fCellId = 24;
  // Ring:4
  if (gMC->CurrentVolID(vol[2]) == gMC->VolId("FV0SCINTCELLa3Sector0"))
    fCellId = 25;
  if (gMC->CurrentVolID(vol[2]) == gMC->VolId("FV0SCINTCELLb3Sector1"))
    fCellId = 26;
  if (gMC->CurrentVolID(vol[2]) == gMC->VolId("FV0SCINTCELLb3Sector2"))
    fCellId = 27;
  if (gMC->CurrentVolID(vol[2]) == gMC->VolId("FV0SCINTCELLa3Sector3"))
    fCellId = 28;
  if (gMC->CurrentVolID(vol[2]) == gMC->VolId("FV0SCINTCELLa3Sector4"))
    fCellId = 29;
  if (gMC->CurrentVolID(vol[2]) == gMC->VolId("FV0SCINTCELLb3Sector5"))
    fCellId = 30;
  if (gMC->CurrentVolID(vol[2]) == gMC->VolId("FV0SCINTCELLb3Sector6"))
    fCellId = 31;
  if (gMC->CurrentVolID(vol[2]) == gMC->VolId("FV0SCINTCELLa3Sector7"))
    fCellId = 32;
  // Ring:5
  if (gMC->CurrentVolID(vol[2]) == gMC->VolId("FV0SCINTCELLa4Sector0"))
    fCellId = 33;
  if (gMC->CurrentVolID(vol[2]) == gMC->VolId("FV0SCINTCELLb4Sector1"))
    fCellId = 34;
  if (gMC->CurrentVolID(vol[2]) == gMC->VolId("FV0SCINTCELLb4Sector2"))
    fCellId = 35;
  if (gMC->CurrentVolID(vol[2]) == gMC->VolId("FV0SCINTCELLa4Sector3"))
    fCellId = 36;
  if (gMC->CurrentVolID(vol[2]) == gMC->VolId("FV0SCINTCELLa4Sector4"))
    fCellId = 37;
  if (gMC->CurrentVolID(vol[2]) == gMC->VolId("FV0SCINTCELLb4Sector5"))
    fCellId = 38;
  if (gMC->CurrentVolID(vol[2]) == gMC->VolId("FV0SCINTCELLb4Sector6"))
    fCellId = 39;
  if (gMC->CurrentVolID(vol[2]) == gMC->VolId("FV0SCINTCELLa4Sector7"))
    fCellId = 40;

  return fCellId;
}

Int_t AliFITv8::GetCellIdOld(Int_t *vol) {
  fCellId = 0;

  if (gMC->CurrentVolID(vol[2]) == gMC->VolId("V0Plus1Sec1"))
    fCellId = 1;
  if (gMC->CurrentVolID(vol[2]) == gMC->VolId("V0Plus1Sec2"))
    fCellId = 2;
  if (gMC->CurrentVolID(vol[2]) == gMC->VolId("V0Plus1Sec3"))
    fCellId = 3;
  if (gMC->CurrentVolID(vol[2]) == gMC->VolId("V0Plus1Sec4"))
    fCellId = 4;
  if (gMC->CurrentVolID(vol[2]) == gMC->VolId("V0Plus1Sec5"))
    fCellId = 5;
  if (gMC->CurrentVolID(vol[2]) == gMC->VolId("V0Plus1Sec6"))
    fCellId = 6;
  if (gMC->CurrentVolID(vol[2]) == gMC->VolId("V0Plus1Sec7"))
    fCellId = 7;
  if (gMC->CurrentVolID(vol[2]) == gMC->VolId("V0Plus1Sec8"))
    fCellId = 8;
  if (gMC->CurrentVolID(vol[2]) == gMC->VolId("V0Plus1Sec9"))
    fCellId = 9;
  if (gMC->CurrentVolID(vol[2]) == gMC->VolId("V0Plus1Sec10"))
    fCellId = 10;
  if (gMC->CurrentVolID(vol[2]) == gMC->VolId("V0Plus1Sec11"))
    fCellId = 11;
  if (gMC->CurrentVolID(vol[2]) == gMC->VolId("V0Plus1Sec12"))
    fCellId = 12;
  if (gMC->CurrentVolID(vol[2]) == gMC->VolId("V0Plus1Sec13"))
    fCellId = 13;
  if (gMC->CurrentVolID(vol[2]) == gMC->VolId("V0Plus1Sec14"))
    fCellId = 14;
  if (gMC->CurrentVolID(vol[2]) == gMC->VolId("V0Plus1Sec15"))
    fCellId = 15;
  if (gMC->CurrentVolID(vol[2]) == gMC->VolId("V0Plus1Sec16"))
    fCellId = 16;
  if (gMC->CurrentVolID(vol[2]) == gMC->VolId("V0Plus2Sec1"))
    fCellId = 17;
  if (gMC->CurrentVolID(vol[2]) == gMC->VolId("V0Plus2Sec2"))
    fCellId = 18;
  if (gMC->CurrentVolID(vol[2]) == gMC->VolId("V0Plus2Sec3"))
    fCellId = 19;
  if (gMC->CurrentVolID(vol[2]) == gMC->VolId("V0Plus2Sec4"))
    fCellId = 20;
  if (gMC->CurrentVolID(vol[2]) == gMC->VolId("V0Plus2Sec5"))
    fCellId = 21;
  if (gMC->CurrentVolID(vol[2]) == gMC->VolId("V0Plus2Sec6"))
    fCellId = 22;
  if (gMC->CurrentVolID(vol[2]) == gMC->VolId("V0Plus2Sec7"))
    fCellId = 23;
  if (gMC->CurrentVolID(vol[2]) == gMC->VolId("V0Plus2Sec8"))
    fCellId = 24;
  if (gMC->CurrentVolID(vol[2]) == gMC->VolId("V0Plus2Sec9"))
    fCellId = 25;
  if (gMC->CurrentVolID(vol[2]) == gMC->VolId("V0Plus2Sec10"))
    fCellId = 26;
  if (gMC->CurrentVolID(vol[2]) == gMC->VolId("V0Plus2Sec11"))
    fCellId = 27;
  if (gMC->CurrentVolID(vol[2]) == gMC->VolId("V0Plus2Sec12"))
    fCellId = 28;
  if (gMC->CurrentVolID(vol[2]) == gMC->VolId("V0Plus2Sec13"))
    fCellId = 29;
  if (gMC->CurrentVolID(vol[2]) == gMC->VolId("V0Plus2Sec14"))
    fCellId = 30;
  if (gMC->CurrentVolID(vol[2]) == gMC->VolId("V0Plus2Sec15"))
    fCellId = 31;
  if (gMC->CurrentVolID(vol[2]) == gMC->VolId("V0Plus2Sec16"))
    fCellId = 32;
  if (gMC->CurrentVolID(vol[2]) == gMC->VolId("V0Plus3Sec1"))
    fCellId = 33;
  if (gMC->CurrentVolID(vol[2]) == gMC->VolId("V0Plus3Sec2"))
    fCellId = 34;
  if (gMC->CurrentVolID(vol[2]) == gMC->VolId("V0Plus3Sec3"))
    fCellId = 35;
  if (gMC->CurrentVolID(vol[2]) == gMC->VolId("V0Plus3Sec4"))
    fCellId = 36;
  if (gMC->CurrentVolID(vol[2]) == gMC->VolId("V0Plus3Sec5"))
    fCellId = 37;
  if (gMC->CurrentVolID(vol[2]) == gMC->VolId("V0Plus3Sec6"))
    fCellId = 38;
  if (gMC->CurrentVolID(vol[2]) == gMC->VolId("V0Plus3Sec7"))
    fCellId = 39;
  if (gMC->CurrentVolID(vol[2]) == gMC->VolId("V0Plus3Sec8"))
    fCellId = 40;
  if (gMC->CurrentVolID(vol[2]) == gMC->VolId("V0Plus3Sec9"))
    fCellId = 41;
  if (gMC->CurrentVolID(vol[2]) == gMC->VolId("V0Plus3Sec10"))
    fCellId = 42;
  if (gMC->CurrentVolID(vol[2]) == gMC->VolId("V0Plus3Sec11"))
    fCellId = 43;
  if (gMC->CurrentVolID(vol[2]) == gMC->VolId("V0Plus3Sec12"))
    fCellId = 44;
  if (gMC->CurrentVolID(vol[2]) == gMC->VolId("V0Plus3Sec13"))
    fCellId = 45;
  if (gMC->CurrentVolID(vol[2]) == gMC->VolId("V0Plus3Sec14"))
    fCellId = 46;
  if (gMC->CurrentVolID(vol[2]) == gMC->VolId("V0Plus3Sec15"))
    fCellId = 47;
  if (gMC->CurrentVolID(vol[2]) == gMC->VolId("V0Plus3Sec16"))
    fCellId = 48;
  if (gMC->CurrentVolID(vol[2]) == gMC->VolId("V0Plus4Sec1"))
    fCellId = 49;
  if (gMC->CurrentVolID(vol[2]) == gMC->VolId("V0Plus4Sec2"))
    fCellId = 50;
  if (gMC->CurrentVolID(vol[2]) == gMC->VolId("V0Plus4Sec3"))
    fCellId = 51;
  if (gMC->CurrentVolID(vol[2]) == gMC->VolId("V0Plus4Sec4"))
    fCellId = 52;
  if (gMC->CurrentVolID(vol[2]) == gMC->VolId("V0Plus4Sec5"))
    fCellId = 53;
  if (gMC->CurrentVolID(vol[2]) == gMC->VolId("V0Plus4Sec6"))
    fCellId = 54;
  if (gMC->CurrentVolID(vol[2]) == gMC->VolId("V0Plus4Sec7"))
    fCellId = 55;
  if (gMC->CurrentVolID(vol[2]) == gMC->VolId("V0Plus4Sec8"))
    fCellId = 56;
  if (gMC->CurrentVolID(vol[2]) == gMC->VolId("V0Plus4Sec9"))
    fCellId = 57;
  if (gMC->CurrentVolID(vol[2]) == gMC->VolId("V0Plus4Sec10"))
    fCellId = 58;
  if (gMC->CurrentVolID(vol[2]) == gMC->VolId("V0Plus4Sec11"))
    fCellId = 59;
  if (gMC->CurrentVolID(vol[2]) == gMC->VolId("V0Plus4Sec12"))
    fCellId = 60;
  if (gMC->CurrentVolID(vol[2]) == gMC->VolId("V0Plus4Sec13"))
    fCellId = 61;
  if (gMC->CurrentVolID(vol[2]) == gMC->VolId("V0Plus4Sec14"))
    fCellId = 62;
  if (gMC->CurrentVolID(vol[2]) == gMC->VolId("V0Plus4Sec15"))
    fCellId = 63;
  if (gMC->CurrentVolID(vol[2]) == gMC->VolId("V0Plus4Sec16"))
    fCellId = 64;
  if (gMC->CurrentVolID(vol[2]) == gMC->VolId("V0Plus5Sec1"))
    fCellId = 65;
  if (gMC->CurrentVolID(vol[2]) == gMC->VolId("V0Plus5Sec2"))
    fCellId = 66;
  if (gMC->CurrentVolID(vol[2]) == gMC->VolId("V0Plus5Sec3"))
    fCellId = 67;
  if (gMC->CurrentVolID(vol[2]) == gMC->VolId("V0Plus5Sec4"))
    fCellId = 68;
  if (gMC->CurrentVolID(vol[2]) == gMC->VolId("V0Plus5Sec5"))
    fCellId = 69;
  if (gMC->CurrentVolID(vol[2]) == gMC->VolId("V0Plus5Sec6"))
    fCellId = 70;
  if (gMC->CurrentVolID(vol[2]) == gMC->VolId("V0Plus5Sec7"))
    fCellId = 71;
  if (gMC->CurrentVolID(vol[2]) == gMC->VolId("V0Plus5Sec8"))
    fCellId = 72;
  if (gMC->CurrentVolID(vol[2]) == gMC->VolId("V0Plus5Sec9"))
    fCellId = 73;
  if (gMC->CurrentVolID(vol[2]) == gMC->VolId("V0Plus5Sec10"))
    fCellId = 74;
  if (gMC->CurrentVolID(vol[2]) == gMC->VolId("V0Plus5Sec11"))
    fCellId = 75;
  if (gMC->CurrentVolID(vol[2]) == gMC->VolId("V0Plus5Sec12"))
    fCellId = 76;
  if (gMC->CurrentVolID(vol[2]) == gMC->VolId("V0Plus5Sec13"))
    fCellId = 77;
  if (gMC->CurrentVolID(vol[2]) == gMC->VolId("V0Plus5Sec14"))
    fCellId = 78;
  if (gMC->CurrentVolID(vol[2]) == gMC->VolId("V0Plus5Sec15"))
    fCellId = 79;
  if (gMC->CurrentVolID(vol[2]) == gMC->VolId("V0Plus5Sec16"))
    fCellId = 80;

  return fCellId;
}
//-----------------------------------------------------------------

Int_t AliFITv8::ReadOptProperties(const std::string filePath, Float_t **e,
                                  Double_t **de, Float_t **abs, Float_t **n,
                                  Float_t **qe, Int_t &kNbins) const {
  std::ifstream infile;
  infile.open(filePath.c_str());

  // Check if file is opened correctly
  if (infile.fail() == true) {
    AliFatal(Form("Error opening ascii file: %s", filePath.c_str()));
    return -1;
  }

  std::string comment; // dummy, used just to read 4 first lines and move the
                       // cursor to the 5th, otherwise unused
  if (!getline(infile, comment)) { // first comment line
    AliFatal(Form("Error opening ascii file (it is probably a folder!): %s",
                  filePath.c_str()));
    return -2;
  }
  getline(infile, comment); // 2nd comment line

  // Get number of elements required for the array
  infile >> kNbins;
  if (kNbins < 0 || kNbins > 1e4) {
    AliFatal(
        Form("Input arraySize out of range 0..1e4: %i. Check input file: %s",
             kNbins, filePath.c_str()));
    return -4;
  }

  // Allocate memory required for arrays
  *e = new Float_t[kNbins];
  *de = new Double_t[kNbins];
  *abs = new Float_t[kNbins];
  *n = new Float_t[kNbins];
  *qe = new Float_t[kNbins];
  getline(infile, comment); // finish 3rd line after the kNbins are read
  getline(infile, comment); // 4th comment line - ignore

  // read the main body of the file (table of values: energy, absorption length,
  // refractive index and quantum efficiency)
  Int_t iLine = 0;
  std::string sLine;
  getline(infile, sLine);
  while (!infile.eof()) {
    if (iLine >= kNbins) {
      AliFatal(Form("Line number: %i reaches range of declared arraySize: %i. "
                    "Check input file: %s",
                    iLine, kNbins, filePath.c_str()));
      return -5;
    }
    std::stringstream ssLine(sLine);
    ssLine >> (*de)[iLine];
    (*de)[iLine] *= 1e-9; // Convert eV -> GeV immediately
    (*e)[iLine] =
        static_cast<Float_t>((*de)[iLine]); // same value, different precision
    ssLine >> (*abs)[iLine];
    ssLine >> (*n)[iLine];
    ssLine >> (*qe)[iLine];
    if (!(ssLine.good() || ssLine.eof())) { // check if there were problems with
                                            // numbers conversion
      AliFatal(
          Form("Error while reading line %i: %s", iLine, ssLine.str().c_str()));
      return -6;
    }
    getline(infile, sLine);
    iLine++;
  }
  if (iLine != kNbins) {
    AliFatal(Form("Total number of lines %i is different than declared %i. "
                  "Check input file: %s",
                  iLine, kNbins, filePath.c_str()));
    return -7;
  }

  AliInfo(Form(
      "Optical properties taken from the file: %s. Number of lines read: %i",
      filePath.c_str(), iLine));
  return 0;
}
//--------------------------------------------------------------------

void AliFITv8::FillOtherOptProperties(Float_t **efficAll, Float_t **rindexAir,
                                      Float_t **absorAir,
                                      Float_t **rindexCathodeNext,
                                      Float_t **absorbCathodeNext,
                                      Double_t **efficMet, Double_t **aReflMet,
                                      const Int_t kNbins) const {
  // Allocate memory for these arrays according to the required size
  *efficAll = new Float_t[kNbins];
  *rindexAir = new Float_t[kNbins];
  *absorAir = new Float_t[kNbins];
  *rindexCathodeNext = new Float_t[kNbins];
  *absorbCathodeNext = new Float_t[kNbins];
  *efficMet = new Double_t[kNbins];
  *aReflMet = new Double_t[kNbins];

  // Set constant values to the arrays
  for (Int_t i = 0; i < kNbins; i++) {
    (*efficAll)[i] = 1.;
    (*rindexAir)[i] = 1.;
    (*absorAir)[i] = 0.3;
    (*rindexCathodeNext)[i] = 0;
    (*absorbCathodeNext)[i] = 0;
    (*efficMet)[i] = 0.;
    (*aReflMet)[i] = 1.;
  }
}
//--------------------------------------------------------------------

void AliFITv8::DeleteOptPropertiesArr(Float_t **e, Double_t **de, Float_t **abs,
                                      Float_t **n, Float_t **efficAll,
                                      Float_t **rindexAir, Float_t **absorAir,
                                      Float_t **rindexCathodeNext,
                                      Float_t **absorbCathodeNext,
                                      Double_t **efficMet,
                                      Double_t **aReflMet) const {
  delete[](*e);
  delete[](*de);
  delete[](*abs);
  delete[](*n);
  delete[](*efficAll);
  delete[](*rindexAir);
  delete[](*absorAir);
  delete[](*rindexCathodeNext);
  delete[](*absorbCathodeNext);
  delete[](*efficMet);
  delete[](*aReflMet);
}
