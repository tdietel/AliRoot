/**************************************************************************
 * This file is property of and copyright by the Experimental Nuclear     *
 * Physics Group, Dep. of Physics                                         *
 * University of Oslo, Norway, 2007                                       *
 *                                                                        *
 * Author: federico ronchetti         for the ALICE HLT Project.*
 *                                                                        *
 * Permission to use, copy, modify and distribute this software and its   *
 * documentation strictly for non-commercial purposes is hereby granted   *
 * without fee, provided that the above copyright notice appears in all   *
 * copies and that both the copyright notice and this permission notice   *
 * appear in the supporting documentation. The authors make no claims     *
 * about the suitability of this software for any purpose. It is          *
 * provided "as is" without express or implied warranty.                  *
 **************************************************************************/

#include "AliHLTEMCALGeometry.h"
#include "AliHLTEMCALConstants.h"
#include "AliCDBEntry.h"
#include "AliCDBManager.h"
#include "AliCDBPath.h"

ClassImp(AliHLTEMCALGeometry);
TGeoManager *gGeoManager = 0;

AliHLTEMCALGeometry::AliHLTEMCALGeometry(Int_t runnumber) : AliHLTCaloGeometry("EMCAL"),
                                                            fGeo(0), fReco(0)
{
  GetGeometryFromCDB(runnumber);
}

Int_t AliHLTEMCALGeometry::InitialiseGeometry()
{

  return GetGeometryFromCDB();
}

AliHLTEMCALGeometry::~AliHLTEMCALGeometry()
{
}

void AliHLTEMCALGeometry::GetGlobalCoordinates(AliHLTCaloRecPointDataStruct &recPoint, AliHLTCaloGlobalCoordinate &globalCoord, Int_t iParticle)
{

  Float_t fDepth = 0;
  // remove misalignments for 2010
  //Float_t *fRot = fReco->GetMisalRotShiftArray();
  //Float_t *fTrans = fReco->GetMisalTransShiftArray();

  Float_t fRot[15];
  Float_t fTrans[15];

  Float_t glob[] = {0., 0., 0.};

  // Zeroing out misaligments
  for (Int_t i = 0; i < 14; i++)
  {
    fRot[i] = 0;
    fTrans[i] = 0;
  }

  //assume only photon for the moment

  if (recPoint.fX >= -0.5 && recPoint.fZ >= -0.5) // -0.5 is the extreme border of 0,0 cell

  {
    if (iParticle == 1) // electron case
      fDepth = fReco->GetDepth(recPoint.fAmp, AliEMCALRecoUtilsBase::kElectron, recPoint.fModule);
    else if (iParticle == 2) // hadron case
      fDepth = fReco->GetDepth(recPoint.fAmp, AliEMCALRecoUtilsBase::kHadron, recPoint.fModule);
    else // anything else is photon
      fDepth = fReco->GetDepth(recPoint.fAmp, AliEMCALRecoUtilsBase::kPhoton, recPoint.fModule);
  }

  else
  {
    HLTDebug("W-AliHLTEMCALGeometry: got cluster with negative flags in coordinates.");
    return; // we don't want to give out strange coordinates to event display
  }

  fGeo->RecalculateTowerPosition(recPoint.fX, recPoint.fZ, recPoint.fModule, fDepth, fTrans, fRot, glob);

  globalCoord.fX = glob[0];
  globalCoord.fY = glob[1];
  globalCoord.fZ = glob[2];
}

void AliHLTEMCALGeometry::GetCellAbsId(UInt_t module, UInt_t x, UInt_t z, Int_t &AbsId)
{

  if (!fGeo)
  {
    Logging(kHLTLogError, "HLT", "EMCAL", "AliHLTEMCALGeometry::GetCellAbsId: no geometry initialised");
    return;
  }
  AbsId = fGeo->GetAbsCellIdFromCellIndexes(module, (Int_t)x, (Int_t)z);
}

int AliHLTEMCALGeometry::GetGeometryFromCDB(Int_t runnumber)
{
  // local path to OCDB
  // AliCDBManager::Instance()->SetDefaultStorage("local://$ALICE_ROOT/OCDB");

  AliCDBPath path("GRP", "Geometry", "Data");
  if (path.GetPath())
  {
    //      HLTInfo("configure from entry %s", path.GetPath());
    AliCDBEntry *pEntry = AliCDBManager::Instance()->Get(path /*,GetRunNo()*/);
    if (pEntry)
    {
      if (!fGeo)
      {
        delete fGeo;
        fGeo = 0;
      }
      if (!gGeoManager)
      {
        gGeoManager = (TGeoManager *)pEntry->GetObject();
      }

      if (gGeoManager)
      {
        HLTDebug("Getting geometry from CDB");
        if (runnumber < 0)
          fGeo = AliEMCALGeometry::GetInstance("EMCAL_COMPLETEV1");
        else
          fGeo = AliEMCALGeometry::GetInstanceFromRunNumber(runnumber);
        //fGeo = new AliEMCALGeoUtils("EMCAL_COMPLETE","EMCAL");
        fReco = new AliEMCALRecoUtilsBase;

        // Old misalignments for 2010
        // We don't use them now
        /*
	      fReco->SetMisalTransShift(0,1.08); 
	      fReco->SetMisalTransShift(1,8.35); 
	      fReco->SetMisalTransShift(2,1.12); //sector 0
	      fReco->SetMisalRotShift(3,-8.05); 
	      fReco->SetMisalRotShift(4,8.05); 
	      fReco->SetMisalTransShift(3,-0.42); 
	      fReco->SetMisalTransShift(5,1.55);//sector 1
	      */
      }
    }
    else
    {
      //HLTError("can not fetch object \"%s\" from OCDB", path);
      Logging(kHLTLogError, "HLT", "EMCAL", "can not fetch object from OCDB");
    }
  }
  return 0;
}

void AliHLTEMCALGeometry::GetLocalCoordinatesFromAbsId(Int_t absId, Int_t &module, Int_t &x, Int_t &z)
{
  Int_t mod; // not super module
  Int_t tmpx;
  Int_t tmpz;

  fGeo->GetCellIndex(absId, module, mod, tmpx, tmpz);
  fGeo->GetCellPhiEtaIndexInSModule(module, mod, tmpx, tmpz, x, z);

  HLTDebug("ID: %d, smodule: %d, mod: %d, x: %d, z: %d", absId, module, mod, x, z);
}
