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

/* $Id$ */
//

///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//  Time Projection Chamber version 0 -- "coarse" TPC                        //
//                                                                           //
//Begin_Html
/*
<img src="picts/AliTPCv0Class.gif">
*/
//End_Html
//                                                                           //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include <Riostream.h>

#include <TMath.h>
#include <TVirtualMC.h>
#include <TString.h>
#include <TSystem.h>

#include "AliConst.h"
#include "AliRun.h"
#include "AliTPCDigitsArray.h"
#include "AliTPCParamSR.h"
#include "AliTPCv0.h"
#include "TGeoManager.h"
#include "TGeoVolume.h"
#include "TGeoPcon.h"
#include "TGeoCone.h"
#include "TGeoTube.h"
#include "TGeoPgon.h"
#include "TGeoTrd1.h"
#include "TGeoCompositeShape.h"
#include "TGeoPara.h"
#include "TGeoPhysicalNode.h"
#include "TGeoHalfSpace.h"
#include "TTreeStream.h"
#include "TGeoArb8.h"
using std::ifstream;
using std::ios_base;
ClassImp(AliTPCv0)
 
//_____________________________________________________________________________
AliTPCv0::AliTPCv0(const char *name, const char *title) 
         :AliTPC(name, title)
{
  //
  // Standard creator for TPC version 0
  //


  if (fTPCParam)
     fTPCParam->Write(fTPCParam->GetTitle());
}

//_____________________________________________________________________________

void AliTPCv0::CreateGeometry()
{

  //
  // Creation of the TPC version 0, i.e. no sensitive volumes,
  // only the material distribution
  //
  //Begin_Html
  /*
    <img src="picts/AliTPC.gif">
  */
  //End_Html
  //Begin_Html
  /*
    <img src="picts/AliTPCv0Tree.gif">
  */
  //End_Html

 //----------------------------------------------------------
  // This geometry is written using TGeo class
  // Firstly the shapes are defined, and only then the volumes
  // What is recognized by the MC are volumes
  //----------------------------------------------------------
  //
  //  tpc - this will be the mother volume
  //

  //
  // here I define a volume TPC
  // retrive the medium name with "TPC_" as a leading string
  //
  TGeoPcon *tpc = new TGeoPcon(0.,360.,30); //30 sections
  //
  tpc->DefineSection(0,-289.6,77.,278.);
  tpc->DefineSection(1,-262.1,77.,278.);
  //
  tpc->DefineSection(2,-262.1,83.1,278.);
  tpc->DefineSection(3,-260.,83.1,278.);
  //
  tpc->DefineSection(4,-260.,70.,278.);    
  tpc->DefineSection(5,-259.6,70.,278.);
  //
  tpc->DefineSection(6,-259.6,68.1,278.);
  tpc->DefineSection(7,-253.6,68.1,278.);
  //
  tpc->DefineSection(8,-253.6,67.88,278.);//hs
  tpc->DefineSection(9,-74.0,60.68,278.);// hs
  //
  tpc->DefineSection(10,-74.0,60.1,278.);
  tpc->DefineSection(11,-73.3,60.1,278.);
  //
  tpc->DefineSection(12,-73.3,56.9,278.); 
  tpc->DefineSection(13,-68.5,56.9,278.);
  //
  tpc->DefineSection(14,-68.5,60.,278.);
  tpc->DefineSection(15,-64.7,60.,278.);
  //
  tpc->DefineSection(16,-64.7,56.9,278.); 
  tpc->DefineSection(17,73.3,56.9,278.);
  //
  tpc->DefineSection(18,73.3,60.1,278.);
  tpc->DefineSection(19,74.0,60.1,278.);
  //
  tpc->DefineSection(20,74.0,60.68,278.);// hs
  tpc->DefineSection(21,253.6,65.38,278.);// hs
  //
  tpc->DefineSection(22,253.6,65.6,278.);
  tpc->DefineSection(23,259.6,65.6,278.);
  //
  tpc->DefineSection(24,259.6,70.0,278.);
  tpc->DefineSection(25,260.,70.0,278.);
  //
  tpc->DefineSection(26,260.,83.1,278.);
  tpc->DefineSection(27,262.1,83.1,278.);
  //
  tpc->DefineSection(28,262.1,77.,278);
  tpc->DefineSection(29,289.6,77.,278.);
  //
  TGeoMedium *m1 = gGeoManager->GetMedium("TPC_Air");
  TGeoVolume *v1 = new TGeoVolume("TPC_M",tpc,m1);
  //
  // drift volume - sensitive volume, extended beyond the
  // endcaps, because of the alignment
  //
  TGeoPcon *dvol = new TGeoPcon(0.,360.,6);
  dvol->DefineSection(0,-260.,74.5,264.4);
  dvol->DefineSection(1,-253.6,74.5,264.4);
  //
  dvol->DefineSection(2,-253.6,76.6774,258.);
  dvol->DefineSection(3,253.6,76.6774,258.); 
  //
  dvol->DefineSection(4,253.6,74.5,264.4);
  dvol->DefineSection(5,260.,74.5,264.4);
  //
  TGeoMedium *m5 = gGeoManager->GetMedium("TPC_DriftGasS1");
  TGeoVolume *v9 = new TGeoVolume("TPC_Drift",dvol,m5);
  //
  v1->AddNode(v9,1);
  //
  // outer insulator
  //
  TGeoPcon *tpco = new TGeoPcon(0.,360.,6); //insulator
  //
  tpco->DefineSection(0,-256.6,264.8,278.);
  tpco->DefineSection(1,-253.6,264.8,278.);
  //
  tpco->DefineSection(2,-253.6,258.,278.);
  tpco->DefineSection(3,250.6,258.,278.);
  //
  tpco->DefineSection(4,250.6,258.,275.5);
  tpco->DefineSection(5,253.6,258.,275.5);
  //
  TGeoMedium *m2 = gGeoManager->GetMedium("TPC_CO2");
  TGeoVolume *v2 = new TGeoVolume("TPC_OI",tpco,m2);
  //
  // outer containment vessel
  //
  TGeoPcon *tocv = new TGeoPcon(0.,360.,6);  // containment vessel
  //
  tocv->DefineSection(0,-256.6,264.8,278.);
  tocv->DefineSection(1,-253.6,264.8,278.);
  //
  tocv->DefineSection(2,-253.6,274.8124,278.);
  tocv->DefineSection(3,247.6,274.8124,278.);  
  //
  tocv->DefineSection(4,247.6,270.4,278.);
  tocv->DefineSection(5,250.6,270.4,278.);
  //
  TGeoMedium *m3 = gGeoManager->GetMedium("TPC_Al");
  TGeoVolume *v3 = new TGeoVolume("TPC_OCV",tocv,m3); 
  //
  TGeoTube *to1 = new TGeoTube(274.8174,277.995,252.1); //epoxy
  TGeoTube *to2 = new TGeoTube(274.8274,277.985,252.1); //tedlar
  TGeoTube *to3 = new TGeoTube(274.8312,277.9812,252.1);//prepreg2
  TGeoTube *to4 = new TGeoTube(274.9062,277.9062,252.1);//nomex
  //
  TGeoMedium *sm1 = gGeoManager->GetMedium("TPC_Epoxy");
  TGeoMedium *sm2 = gGeoManager->GetMedium("TPC_Tedlar");
  TGeoMedium *sm3 = gGeoManager->GetMedium("TPC_Prepreg2");
  TGeoMedium *sm4 = gGeoManager->GetMedium("TPC_Nomex");
  //
  //
  TGeoVolume *tov1 = new TGeoVolume("TPC_OCV1",to1,sm1);
  TGeoVolume *tov2 = new TGeoVolume("TPC_OCV2",to2,sm2);
  TGeoVolume *tov3 = new TGeoVolume("TPC_OCV3",to3,sm3);
  TGeoVolume *tov4 = new TGeoVolume("TPC_OCV4",to4,sm4);
  TGeoMedium *mhs = gGeoManager->GetMedium("TPC_Steel");
  TGeoMedium *m12 =  gGeoManager->GetMedium("TPC_Water");
  //-------------------------------------------------------
  //  Tpc Outer Field Cage
  //  daughters - composite (sandwich)
  //-------------------------------------------------------

  TGeoPcon *tofc = new TGeoPcon(0.,360.,6);
  //
  tofc->DefineSection(0,-253.6,258.,269.6);
  tofc->DefineSection(1,-250.6,258.,269.6);
  //
  tofc->DefineSection(2,-250.6,258.,260.0676); 
  tofc->DefineSection(3,250.6,258.,260.0676);
  //
  tofc->DefineSection(4,250.6,258.,275.5);
  tofc->DefineSection(5,253.6,258.,275.5);
  //
  TGeoVolume *v4 = new TGeoVolume("TPC_TOFC",tofc,m3); 
  //sandwich
  TGeoTube *tf1 = new TGeoTube(258.0,260.0676,252.1); //tedlar
  TGeoTube *tf2 = new TGeoTube(258.0038,260.0638,252.1); //prepreg3
  TGeoTube *tf3 = new TGeoTube(258.0338,260.0338,252.1);//nomex
  //
  TGeoMedium *sm5 = gGeoManager->GetMedium("TPC_Prepreg3");
  //
  TGeoVolume *tf1v = new TGeoVolume("TPC_OFC1",tf1,sm2);
  TGeoVolume *tf2v = new TGeoVolume("TPC_OFC2",tf2,sm5);
  TGeoVolume *tf3v = new TGeoVolume("TPC_OFC3",tf3,sm4);
  //
  // outer part - positioning
  //
  tov1->AddNode(tov2,1); tov2->AddNode(tov3,1); tov3->AddNode(tov4,1);
  //
  tf1v->AddNode(tf2v,1); tf2v->AddNode(tf3v,1);
  //
  v3->AddNode(tov1,1,new TGeoTranslation(0.,0.,-1.5)); v4->AddNode(tf1v,1);
  //
  v2->AddNode(v3,1); v2->AddNode(v4,1); 
  //
  v1->AddNode(v2,1);
  //-----------------------------------------------------------
  //--------------------------------------------------------------------
  // Tpc Inner INsulator (CO2) 
  // the cones, the central drum and the inner f.c. sandwich with a piece
  // of the flane will be placed in the TPC
  //--------------------------------------------------------------------
  TGeoPcon *tpci = new TGeoPcon(0.,360.,4);
  //
  tpci->DefineSection(0,-253.6,68.4,76.6774);
  tpci->DefineSection(1,-74.0,61.2,76.6774);
  //
  tpci->DefineSection(2,74.0,61.2,76.6774);  
  //
  tpci->DefineSection(3,253.6,65.9,76.6774);
  //
  TGeoVolume *v5 = new TGeoVolume("TPC_INI",tpci,m2);
  //
  // now the inner field cage - only part of flanges (2 copies)
  //
  TGeoTube *tif1 = new TGeoTube(69.9,76.6774,1.5); 
  TGeoVolume *v6 = new TGeoVolume("TPC_IFC1",tif1,m3);
  //
 //---------------------------------------------------------
  // Tpc Inner Containment vessel - Muon side
  //---------------------------------------------------------
  TGeoPcon *tcms = new TGeoPcon(0.,360.,10);
  //
  tcms->DefineSection(0,-259.1,68.1,74.2);
  tcms->DefineSection(1,-253.6,68.1,74.2);
  //
  tcms->DefineSection(2,-253.6,68.1,68.4);
  tcms->DefineSection(3,-74.0,60.9,61.2);
  //
  tcms->DefineSection(4,-74.0,60.1,61.2);
  tcms->DefineSection(5,-73.3,60.1,61.2);
  //
  tcms->DefineSection(6,-73.3,56.9,61.2);
  tcms->DefineSection(7,-73.0,56.9,61.2);
  //
  tcms->DefineSection(8,-73.0,56.9,58.8);
  tcms->DefineSection(9,-71.3,56.9,58.8);
  //
  TGeoVolume *v7 = new TGeoVolume("TPC_ICVM",tcms,m3);
  //------------------------------------------------
  //  Heat screen muon side
  //------------------------------------------------
 
  TGeoCone *thsm = new TGeoCone(89.8,67.88,68.1,60.68,60.9);
  TGeoCone *thsmw = new TGeoCone(89.8,67.94,68.04,60.74,60.84);
  TGeoVolume *hvsm = new TGeoVolume("TPC_HSM",thsm,mhs); //steel
  TGeoVolume *hvsmw = new TGeoVolume("TPC_HSMW",thsmw,m12); //water 
  // assembly heat screen muon
  hvsm->AddNode(hvsmw,1);
  //-----------------------------------------------
  // inner containment vessel - shaft side
  //-----------------------------------------------
  TGeoPcon *tcss = new TGeoPcon(0.,360.,10);
  //
  tcss->DefineSection(0,71.3,56.9,58.8);
  tcss->DefineSection(1,73.0,56.9,58.8);
  //
  tcss->DefineSection(2,73.0,56.9,61.2);
  tcss->DefineSection(3,73.3,56.9,61.2);
  //  
  tcss->DefineSection(4,73.3,60.1,61.2);
  tcss->DefineSection(5,74.0,60.1,61.2);
  //
  tcss->DefineSection(6,74.0,60.9,61.2);
  tcss->DefineSection(7,253.6,65.6,65.9);
  //
  tcss->DefineSection(8,253.6,65.6,74.2);
  tcss->DefineSection(9,258.1,65.6,74.2);
  //
  TGeoVolume *v8 = new TGeoVolume("TPC_ICVS",tcss,m3);
  //-------------------------------------------------
  //  Heat screen shaft side
  //--------------------------------------------------
  TGeoCone *thss = new TGeoCone(89.8,60.68,60.9,65.38,65.6);       
  TGeoCone *thssw = new TGeoCone(89.8,60.74,60.84,65.44,65.54);     
  TGeoVolume *hvss = new TGeoVolume("TPC_HSS",thss,mhs); //steel
  TGeoVolume *hvssw = new TGeoVolume("TPC_HSSW",thssw,m12); //water 
  //assembly heat screen shaft
  hvss->AddNode(hvssw,1);
  //-----------------------------------------------
  //  Inner field cage
  //  define 4 parts and make an assembly
  //-----------------------------------------------
  // part1 - Al - 2 copies
  TGeoTube *t1 = new TGeoTube(76.6774,78.845,0.75);
  TGeoVolume *tv1 = new TGeoVolume("TPC_IFC2",t1,m3);
  // sandwich - outermost parts - 2 copies
  TGeoTube *t2 = new TGeoTube(76.6774,78.845,74.175); // tedlar 38 microns
  TGeoTube *t3 = new TGeoTube(76.6812,78.8412,74.175); // prepreg2 500 microns
  TGeoTube *t4 = new TGeoTube(76.7312,78.7912,74.175); // prepreg3 300 microns
  TGeoTube *t5 = new TGeoTube(76.7612,78.7612,74.175); // nomex 2 cm
  //
  TGeoVolume *tv2 = new TGeoVolume("TPC_IFC3",t2,sm2);
  TGeoVolume *tv3 = new TGeoVolume("TPC_IFC4",t3,sm3);
  TGeoVolume *tv4 = new TGeoVolume("TPC_IFC5",t4,sm5);
  TGeoVolume *tv5 = new TGeoVolume("TPC_IFC6",t5,sm4);
  //
  // middle parts - 2 copies
  TGeoTube *t6 = new TGeoTube(76.6774,78.795,5.); // tedlar 38 microns
  TGeoTube *t7 = new TGeoTube(76.6812,78.7912,5.); // prepreg2 250 microns
  TGeoTube *t8 = new TGeoTube(76.7062,78.7662,5.); // prepreg3 300 microns
  TGeoTube *t9 = new TGeoTube(76.7362,78.7362,5.); // nomex 2 cm
  //
  TGeoVolume *tv6 = new TGeoVolume("TPC_IFC7",t6,sm2);
  TGeoVolume *tv7 = new TGeoVolume("TPC_IFC8",t7,sm3);
  TGeoVolume *tv8 = new TGeoVolume("TPC_IFC9",t8,sm5);
  TGeoVolume *tv9 = new TGeoVolume("TPC_IFC10",t9,sm4);
  // central part - 1 copy
  TGeoTube *t10 = new TGeoTube(76.6774,78.745,93.75); // tedlar 38 microns 
  TGeoTube *t11 = new TGeoTube(76.6812,78.7412,93.75); // prepreg3 300 microns
  TGeoTube *t12 = new TGeoTube(76.7112,78.7112,93.75); // nomex 2 cm
  //
  TGeoVolume *tv10 = new TGeoVolume("TPC_IFC11",t10,sm2);
  TGeoVolume *tv11 = new TGeoVolume("TPC_IFC12",t11,sm5);
  TGeoVolume *tv12 = new TGeoVolume("TPC_IFC13",t12,sm4);
  //
  // inner part - positioning
  //
  // creating a sandwich
  tv2->AddNode(tv3,1); tv3->AddNode(tv4,1); tv4->AddNode(tv5,1);
  //
  tv6->AddNode(tv7,1); tv7->AddNode(tv8,1); tv8->AddNode(tv9,1);
  //
  tv10->AddNode(tv11,1); tv11->AddNode(tv12,1);
  //
  TGeoVolumeAssembly *tv100 = new TGeoVolumeAssembly("TPC_IFC");
  //
  tv100->AddNode(tv10,1);
  tv100->AddNode(tv6,1,new TGeoTranslation(0.,0.,-98.75));
  tv100->AddNode(tv6,2,new TGeoTranslation(0.,0.,98.75));
  tv100->AddNode(tv2,1,new TGeoTranslation(0.,0.,-177.925));
  tv100->AddNode(tv2,2,new TGeoTranslation(0.,0.,177.925));
  tv100->AddNode(tv1,1,new TGeoTranslation(0.,0.,-252.85));
  tv100->AddNode(tv1,2,new TGeoTranslation(0.,0.,252.85));
  //
  v5->AddNode(v6,1, new TGeoTranslation(0.,0.,-252.1));
  v5->AddNode(v6,2, new TGeoTranslation(0.,0.,252.1));
  v1->AddNode(v5,1); v1->AddNode(v7,1); v1->AddNode(v8,1); 
  v1->AddNode(hvsm,1,new TGeoTranslation(0.,0.,-163.8)); 
  v1->AddNode(hvss,1,new TGeoTranslation(0.,0.,163.8)); 
  v9->AddNode(tv100,1);
  //
  // central drum 
  //
  // flange + sandwich
  //
  TGeoPcon *cfl = new TGeoPcon(0.,360.,6);
  cfl->DefineSection(0,-71.1,59.7,61.2);
  cfl->DefineSection(1,-68.6,59.7,61.2);
  //
  cfl->DefineSection(2,-68.6,60.6324,61.2);
  cfl->DefineSection(3,68.6,60.6324,61.2); 
  //
  cfl->DefineSection(4,68.6,59.7,61.2);
  cfl->DefineSection(5,71.1,59.7,61.2);  
  //
  TGeoVolume *cflv = new TGeoVolume("TPC_CDR",cfl,m3);
  // sandwich
  TGeoTube *cd1 = new TGeoTube(60.6424,61.19,71.1);
  TGeoTube *cd2 = new TGeoTube(60.6462,61.1862,71.1);
  TGeoTube *cd3 = new TGeoTube(60.6662,61.1662,71.1);  
  //
  TGeoMedium *sm6 = gGeoManager->GetMedium("TPC_Prepreg1");
  TGeoVolume *cd1v = new TGeoVolume("TPC_CDR1",cd1,sm2); //tedlar
  TGeoVolume *cd2v = new TGeoVolume("TPC_CDR2",cd2,sm6);// prepreg1
  TGeoVolume *cd3v = new TGeoVolume("TPC_CDR3",cd3,sm4); //nomex
  //
  // seals for central drum 2 copies
  //
  TGeoTube *cs = new TGeoTube(56.9,61.2,0.1);
  TGeoMedium *sm7 = gGeoManager->GetMedium("TPC_Mylar");
  TGeoVolume *csv = new TGeoVolume("TPC_CDRS",cs,sm7);
  v1->AddNode(csv,1,new TGeoTranslation(0.,0.,-71.2));
  v1->AddNode(csv,2,new TGeoTranslation(0.,0.,71.2));
  //
  // seal collars 
  TGeoPcon *se = new TGeoPcon(0.,360.,6);
  se->DefineSection(0,-72.8,59.7,61.2);
  se->DefineSection(1,-72.3,59.7,61.2);
  //
  se->DefineSection(2,-72.3,58.85,61.2);
  se->DefineSection(3,-71.6,58.85,61.2); 
  //
  se->DefineSection(4,-71.6,59.7,61.2);
  se->DefineSection(5,-71.3,59.7,61.2);  
  //
  TGeoVolume *sev = new TGeoVolume("TPC_CDCE",se,m3);
  //
  TGeoTube *si = new TGeoTube(56.9,58.8,1.); 
  TGeoVolume *siv = new TGeoVolume("TPC_CDCI",si,m3);
  //
  // define reflection matrix 
  //
  TGeoRotation *ref = new TGeoRotation("ref",90.,0.,90.,90.,180.,0.);
  //
  cd1v->AddNode(cd2v,1); cd2v->AddNode(cd3v,1); cflv->AddNode(cd1v,1);
  //
  v1->AddNode(siv,1,new TGeoTranslation(0.,0.,-69.9));
  v1->AddNode(siv,2,new TGeoTranslation(0.,0.,69.9));
  v1->AddNode(sev,1); v1->AddNode(sev,2,ref); v1->AddNode(cflv,1);
  //
  // central membrane - 2 rings and a mylar membrane - assembly
  //
  TGeoTube *ih = new TGeoTube(81.05,84.05,0.3);
  TGeoTube *oh = new TGeoTube(250.,256.,.5);
  TGeoTube *mem = new TGeoTube(84.05,250,0.01);
  TGeoVolume *ihv = new TGeoVolume("TPC_IHVH",ih,m3);
  TGeoVolume *ohv = new TGeoVolume("TPC_OHVH",oh,m3);
  TGeoVolume *memv = new TGeoVolume("TPC_HV",mem,sm7);
  //
  TGeoVolumeAssembly *cm = new TGeoVolumeAssembly("TPC_HVMEM");
  cm->AddNode(ihv,1);
  cm->AddNode(ohv,1);
  cm->AddNode(memv,1);
  v9->AddNode(cm,1);
  //
  // end caps - they are make as an assembly of single segments
  // containing both readout chambers
  //
  Double_t openingAngle = 10.*TMath::DegToRad();
  Double_t thick=1.5; // rib
  Double_t shift = thick/TMath::Sin(openingAngle);
  //
  Double_t lowEdge = 86.3; // hole in the wheel
  Double_t upEdge = 240.4; // hole in the wheel
  //
  new TGeoTubeSeg("sec",74.5,264.4,3.,0.,20.);
  //
  TGeoPgon *hole = new TGeoPgon("hole",0.,20.,1,4);
  //
  hole->DefineSection(0,-3.5,lowEdge-shift,upEdge-shift);
  hole->DefineSection(1,-1.5,lowEdge-shift,upEdge-shift);
  //
  hole->DefineSection(2,-1.5,lowEdge-shift,upEdge+3.-shift);
  hole->DefineSection(3,3.5,lowEdge-shift,upEdge+3.-shift);
  //
  Double_t ys = shift*TMath::Sin(openingAngle); 
  Double_t xs = shift*TMath::Cos(openingAngle);
  TGeoTranslation *tr = new TGeoTranslation("tr",xs,ys,0.);  
  tr->RegisterYourself();
  TGeoCompositeShape *chamber = new TGeoCompositeShape("sec-hole:tr");
  TGeoVolume *sv = new TGeoVolume("TPC_WSEG",chamber,m3);
  TGeoPgon *bar = new TGeoPgon("bar",0.,20.,1,2);
  bar->DefineSection(0,-3.,131.5-shift,136.5-shift);
  bar->DefineSection(1,1.5,131.5-shift,136.5-shift);
  TGeoVolume *barv = new TGeoVolume("TPC_WBAR",bar,m3);
  TGeoVolumeAssembly *ch = new TGeoVolumeAssembly("TPC_WCH");//empty segment
  //
  ch->AddNode(sv,1); ch->AddNode(barv,1,tr);
  //
  // readout chambers
  //
  // IROC first
  //
   TGeoTrd1 *ibody = new TGeoTrd1(13.8742,21.3328,4.29,21.15);
   TGeoVolume *ibdv = new TGeoVolume("TPC_IROCB",ibody,m3);
  // empty space
   TGeoTrd1 *emp = new TGeoTrd1(12.3742,19.8328,3.99,19.65);
   TGeoVolume *empv = new TGeoVolume("TPC_IROCE",emp,m1);
   ibdv->AddNode(empv,1,new TGeoTranslation(0.,-0.3,0.));
   //bars
   Double_t tga = (19.8328-12.3742)/39.3;
   Double_t xmin,xmax;
   xmin = 9.55*tga+12.3742;
   xmax = 9.95*tga+12.3742;
   TGeoTrd1 *ib1 = new TGeoTrd1(xmin,xmax,3.29,0.2);
   TGeoVolume *ib1v = new TGeoVolume("TPC_IRB1",ib1,m3);
   empv->AddNode(ib1v,1,new TGeoTranslation("tt1",0.,0.7,-9.9));
   xmin=19.4*tga+12.3742;
   xmax=19.9*tga+12.3742;
   TGeoTrd1 *ib2 = new TGeoTrd1(xmin,xmax,3.29,0.25);
   TGeoVolume *ib2v = new TGeoVolume("TPC_TRB2",ib2,m3);
   empv->AddNode(ib2v,1,new TGeoTranslation(0.,0.7,0.));
   xmin=29.35*tga+12.3742;
   xmax=29.75*tga+12.3742;
   TGeoTrd1 *ib3 = new TGeoTrd1(xmin,xmax,3.29,0.2); 
   TGeoVolume *ib3v = new TGeoVolume("TPC_IRB3",ib3,m3);    
   empv->AddNode(ib3v,1,new TGeoTranslation(0.,0.7,9.9));
   //
   // holes for connectors
   //
   TGeoBBox *conn = new TGeoBBox(0.4,0.3,4.675); // identical for iroc and oroc
   TGeoVolume *connv = new TGeoVolume("TPC_RCCON",conn,m1);
   TString fileName(gSystem->Getenv("ALICE_ROOT"));
   fileName += "/TPC/conn_iroc.dat";
   ifstream in;
   in.open(fileName.Data(), ios_base::in); // asci file
   for(Int_t i =0;i<86;i++){
      Double_t y = 3.99;
      Double_t x,z,ang;
      in>>x>>z>>ang;
      z-=26.5;
      TGeoRotation *rrr = new TGeoRotation();
      rrr->RotateY(ang);
      TGeoCombiTrans *trans = new TGeoCombiTrans("trans",x,y,z,rrr);
      ibdv->AddNode(connv,i+1,trans);
   }
   in.close();
   // "cap"
   new TGeoTrd1("icap",14.5974,23.3521,1.19,24.825);
   // "hole"
   new TGeoTrd1("ihole",13.8742,21.3328,1.2,21.15);
   TGeoTranslation *tr1 = new TGeoTranslation("tr1",0.,0.,1.725);  
   tr1->RegisterYourself();
   TGeoCompositeShape *ic = new TGeoCompositeShape("icap-ihole:tr1");
   TGeoVolume *icv = new TGeoVolume("TPC_IRCAP",ic,m3);
   //
   // pad plane and wire fixations
   //
   TGeoTrd1 *pp = new TGeoTrd1(14.5974,23.3521,0.3,24.825); //pad+iso
   TGeoMedium *m4 = gGeoManager->GetMedium("TPC_G10");
   TGeoVolume *ppv = new TGeoVolume("TPC_IRPP",pp,m4);
   TGeoPara *f1 = new TGeoPara(.6,.5,24.825,0.,-10.,0.);
   TGeoVolume *f1v = new TGeoVolume("TPC_IRF1",f1,m4);
   TGeoPara *f2 = new TGeoPara(.6,.5,24.825,0.,10.,0.);
   TGeoVolume *f2v = new TGeoVolume("TPC_IRF2",f2,m4);
   //
   TGeoVolumeAssembly *iroc = new TGeoVolumeAssembly("TPC_IROC");
   //
   iroc->AddNode(ibdv,1);
   iroc->AddNode(icv,1,new TGeoTranslation(0.,3.1,-1.725));
   iroc->AddNode(ppv,1,new TGeoTranslation(0.,4.59,-1.725));
   tga =(23.3521-14.5974)/49.65; 
   Double_t xx = 24.825*tga+14.5974-0.6;
   iroc->AddNode(f1v,1,new TGeoTranslation(-xx,5.39,-1.725));
   iroc->AddNode(f2v,1,new TGeoTranslation(xx,5.39,-1.725));
   //
   // OROC
   //
   TGeoTrd1 *obody = new TGeoTrd1(22.2938,40.5084,4.19,51.65);
   TGeoVolume *obdv = new TGeoVolume("TPC_OROCB",obody,m3);
   TGeoTrd1 *oemp = new TGeoTrd1(20.2938,38.5084,3.89,49.65);
   TGeoVolume *oempv = new TGeoVolume("TPC_OROCE",oemp,m1);
   obdv->AddNode(oempv,1,new TGeoTranslation(0.,-0.3,0.));
   //horizontal bars
   tga=(38.5084-20.2938)/99.3;
   xmin=tga*10.2+20.2938;
   xmax=tga*10.6+20.2938;
   TGeoTrd1 *ob1 = new TGeoTrd1(xmin,xmax,2.915,0.2);
   TGeoVolume *ob1v = new TGeoVolume("TPC_ORB1",ob1,m3);
   //
   xmin=22.55*tga+20.2938;
   xmax=24.15*tga+20.2938;
   TGeoTrd1 *ob2 = new TGeoTrd1(xmin,xmax,2.915,0.8);
   TGeoVolume *ob2v = new TGeoVolume("TPC_ORB2",ob2,m3);
   //
   xmin=36.1*tga+20.2938;
   xmax=36.5*tga+20.2938;
   TGeoTrd1 *ob3 = new TGeoTrd1(xmin,xmax,2.915,0.2);
   TGeoVolume *ob3v = new TGeoVolume("TPC_ORB3",ob3,m3);
   //
   xmin=49.0*tga+20.2938;
   xmax=50.6*tga+20.2938;   
   TGeoTrd1 *ob4 = new TGeoTrd1(xmin,xmax,2.915,0.8);
   TGeoVolume *ob4v = new TGeoVolume("TPC_ORB4",ob4,m3);
   //
   xmin=63.6*tga+20.2938;
   xmax=64.0*tga+20.2938;
   TGeoTrd1 *ob5 = new TGeoTrd1(xmin,xmax,2.915,0.2);
   TGeoVolume *ob5v = new TGeoVolume("TPC_ORB5",ob5,m3);
   //
   xmin=75.5*tga+20.2938;
   xmax=77.15*tga+20.2938;
   TGeoTrd1 *ob6 = new TGeoTrd1(xmin,xmax,2.915,0.8);
   TGeoVolume *ob6v = new TGeoVolume("TPC_ORB6",ob6,m3);
   //
   xmin=88.7*tga+20.2938;
   xmax=89.1*tga+20.2938;
   TGeoTrd1 *ob7 = new TGeoTrd1(xmin,xmax,2.915,0.2);
   TGeoVolume *ob7v = new TGeoVolume("TPC_ORB7",ob7,m3);
   //
   oempv->AddNode(ob1v,1,new TGeoTranslation(0.,0.975,-39.25));
   oempv->AddNode(ob2v,1,new TGeoTranslation(0.,0.975,-26.3));
   oempv->AddNode(ob3v,1,new TGeoTranslation(0.,0.975,-13.35));
   oempv->AddNode(ob4v,1,new TGeoTranslation(0.,0.975,0.15));
   oempv->AddNode(ob5v,1,new TGeoTranslation(0.,0.975,14.15));
   oempv->AddNode(ob6v,1,new TGeoTranslation(0.,0.975,26.7));
   oempv->AddNode(ob7v,1,new TGeoTranslation(0.,0.975,39.25));
   // vertical bars
   TGeoBBox *ob8 = new TGeoBBox(0.8,2.915,5.1); 
   TGeoBBox *ob9 = new TGeoBBox(0.8,2.915,5.975);
   TGeoBBox *ob10 = new TGeoBBox(0.8,2.915,5.775);
   TGeoBBox *ob11 = new TGeoBBox(0.8,2.915,6.25);
   TGeoBBox *ob12 = new TGeoBBox(0.8,2.915,6.5);
   //
   TGeoVolume *ob8v = new TGeoVolume("TPC_ORB8",ob8,m3);
   TGeoVolume *ob9v = new TGeoVolume("TPC_ORB9",ob9,m3);
   TGeoVolume *ob10v = new TGeoVolume("TPC_ORB10",ob10,m3);
   TGeoVolume *ob11v = new TGeoVolume("TPC_ORB11",ob11,m3);
   TGeoVolume *ob12v = new TGeoVolume("TPC_ORB12",ob12,m3);
   //
   oempv->AddNode(ob8v,1,new TGeoTranslation(0.,0.975,-44.55));
   oempv->AddNode(ob8v,2,new TGeoTranslation(0.,0.975,44.55));
   oempv->AddNode(ob9v,1,new TGeoTranslation(0.,0.975,-33.075));
   oempv->AddNode(ob9v,2,new TGeoTranslation(0.,0.975,-19.525));
   oempv->AddNode(ob10v,1,new TGeoTranslation(0.,0.975,20.125));
   oempv->AddNode(ob10v,2,new TGeoTranslation(0.,0.975,33.275));
   oempv->AddNode(ob11v,1,new TGeoTranslation(0.,0.975,-6.9));
   oempv->AddNode(ob12v,1,new TGeoTranslation(0.,0.975,7.45));
   //
   // holes for connectors
   //
   fileName = gSystem->Getenv("ALICE_ROOT");
   fileName += "/TPC/conn_oroc.dat";
   in.open(fileName.Data(), ios_base::in); // asci file
   for(Int_t i =0;i<78;i++){
      Double_t y =3.89;
      Double_t x,z,ang;
      Double_t x1,z1,x2,z2;
      in>>x>>z>>ang;        
      Double_t xr = 4.7*TMath::Sin(ang*TMath::DegToRad());
      Double_t zr = 4.7*TMath::Cos(ang*TMath::DegToRad());
      //
      x1=xr+x; x2=-xr+x; z1=zr+z; z2 = -zr+z;      
      //
      TGeoRotation *rr = new TGeoRotation();
      rr->RotateY(ang); 
      z1-=54.95;
      z2-=54.95;
      TGeoCombiTrans *trans1 = new TGeoCombiTrans("trans1",x1,y,z1,rr);
      TGeoCombiTrans *trans2 = new TGeoCombiTrans("trans2",x2,y,z2,rr);
      obdv->AddNode(connv,i+1,trans1);
      obdv->AddNode(connv,i+79,trans2);
   }
   in.close();
   // cap
   new TGeoTrd1("ocap",23.3874,43.5239,1.09,57.1);
   new TGeoTrd1("ohole",22.2938,40.5084,1.09,51.65);
   TGeoTranslation *tr5 = new TGeoTranslation("tr5",0.,0.,-2.15);
   tr5->RegisterYourself();
   TGeoCompositeShape *oc = new TGeoCompositeShape("ocap-ohole:tr5");
   TGeoVolume *ocv = new TGeoVolume("TPC_ORCAP",oc,m3);
   //
   // pad plane and wire fixations
   //
   TGeoTrd1 *opp = new TGeoTrd1(23.3874,43.5239,0.3,57.1);
   TGeoVolume *oppv = new TGeoVolume("TPC_ORPP",opp,m4);
   //
   tga=(43.5239-23.3874)/114.2;
   TGeoPara *f3 = new TGeoPara(.7,.6,57.1,0.,-10.,0.);
   TGeoPara *f4 = new TGeoPara(.7,.6,57.1,0.,10.,0.);  
   xx = 57.1*tga+23.3874-0.7;
   TGeoVolume *f3v = new TGeoVolume("TPC_ORF1",f3,m4);
   TGeoVolume *f4v = new TGeoVolume("TPC_ORF2",f4,m4);
   //
   TGeoVolumeAssembly *oroc = new TGeoVolumeAssembly("TPC_OROC");
   //
   oroc->AddNode(obdv,1);
   oroc->AddNode(ocv,1,new TGeoTranslation(0.,3.1,2.15));
   oroc->AddNode(oppv,1,new TGeoTranslation(0.,4.49,2.15));
   oroc->AddNode(f3v,1,new TGeoTranslation(-xx,5.39,2.15));
   oroc->AddNode(f4v,1,new TGeoTranslation(xx,5.39,2.15));
   // 
   // now iroc and oroc are placed into a sector...
   //
   TGeoVolumeAssembly *secta = new TGeoVolumeAssembly("TPC_SECT"); // a-side
   TGeoVolumeAssembly *sectc = new TGeoVolumeAssembly("TPC_SECT"); // c-side
   //

   TGeoRotation rot1("rot1",90.,90.,0.);
   TGeoRotation rot2("rot2");
   rot2.RotateY(10.);
   TGeoRotation *rot = new TGeoRotation("rot");
   *rot=rot1*rot2;
   //
   Double_t x0,y0;
   x0=110.2*TMath::Cos(openingAngle);
   y0=110.2*TMath::Sin(openingAngle);
   TGeoCombiTrans *combi1a = new TGeoCombiTrans("combi1",x0,y0,1.09+0.195,rot); //a-side 
   TGeoCombiTrans *combi1c = new TGeoCombiTrans("combi1",x0,y0,1.09+0.222,rot); //c-side
   x0=188.45*TMath::Cos(openingAngle);
   y0=188.45*TMath::Sin(openingAngle);
   TGeoCombiTrans *combi2a = new TGeoCombiTrans("combi2",x0,y0,0.99+0.195,rot); //a-side
   TGeoCombiTrans *combi2c = new TGeoCombiTrans("combi2",x0,y0,0.99+0.222,rot); //c-side
   //
   // A-side
   //
   secta->AddNode(ch,1);
   secta->AddNode(iroc,1,combi1a);
   secta->AddNode(oroc,1,combi2a);
   //
   // C-side
   //
   sectc->AddNode(ch,1);
   sectc->AddNode(iroc,1,combi1c);
   sectc->AddNode(oroc,1,combi2c);
   //
   // now I try to make  wheels...
   //
   TGeoVolumeAssembly *wheela = new TGeoVolumeAssembly("TPC_ENDCAP");
   TGeoVolumeAssembly *wheelc = new TGeoVolumeAssembly("TPC_ENDCAP");
   //
   for(Int_t i =0;i<18;i++){
     Double_t phi = (20.*i);
     TGeoRotation *r = new TGeoRotation();
     r->RotateZ(phi);
     wheela->AddNode(secta,i+1,r);
     wheelc->AddNode(sectc,i+1,r);    
   }
   // wheels in the drift volume!
   TGeoCombiTrans *combi3 = new TGeoCombiTrans("combi3",0.,0.,256.6,ref);
   v9->AddNode(wheela,1,combi3);
   v9->AddNode(wheelc,2,new TGeoTranslation(0.,0.,-256.6));
   //_____________________________________________________________
   // service support wheel
   //_____________________________________________________________
  TGeoPgon *sw = new TGeoPgon(0.,20.,1,2);
  sw->DefineSection(0,-4.,80.5,251.75);
  sw->DefineSection(1,4.,80.5,251.75); 
  TGeoVolume *swv = new TGeoVolume("TPC_SWSEG",sw,m3); //Al
  //
  thick=1.;
  shift = thick/TMath::Sin(openingAngle);
  TGeoPgon *sh = new TGeoPgon(0.,20.,1,2);
  sh->DefineSection(0,-4.,81.5-shift,250.75-shift);
  sh->DefineSection(1,4.,81.5-shift,250.75-shift);
  TGeoVolume *shv = new TGeoVolume("TPC_SWS1",sh,m1); //Air
  //
  TGeoMedium *m9 =  gGeoManager->GetMedium("TPC_Si"); 
  TGeoPgon *el = new TGeoPgon(0.,20.,1,2);
  el->DefineSection(0,-1.872,81.5-shift,250.75-shift);
  el->DefineSection(1,1.872,81.5-shift,250.75-shift);
  TGeoVolume *elv = new TGeoVolume("TPC_ELEC",el,m9); //Si 
  //
  shv->AddNode(elv,1);
  //
  //
  ys = shift*TMath::Sin(openingAngle);
  xs = shift*TMath::Cos(openingAngle);
  swv->AddNode(shv,1,new TGeoTranslation(xs,ys,0.));
  // cover
  TGeoPgon *co = new TGeoPgon(0.,20.,1,2);
  co->DefineSection(0,-0.5,77.,255.25);
  co->DefineSection(1,0.5,77.,255.25);
  TGeoVolume *cov = new TGeoVolume("TPC_SWC1",co,m3);//Al
  // hole in a cover
  TGeoPgon *coh = new TGeoPgon(0.,20.,1,2);
  shift=4./TMath::Sin(openingAngle);
  coh->DefineSection(0,-0.5,85.-shift,247.25-shift);
  coh->DefineSection(1,0.5,85.-shift,247.25-shift);  
  //
  TGeoVolume *cohv = new TGeoVolume("TPC_SWC2",coh,m1);
  //
  ys = shift*TMath::Sin(openingAngle);
  xs = shift*TMath::Cos(openingAngle);  
  cov->AddNode(cohv,1,new TGeoTranslation(xs,ys,0.));
  //
  // Sector as an Assembly
  //
  TGeoVolumeAssembly *swhs = new TGeoVolumeAssembly("TPC_SSWSEC");
  swhs->AddNode(swv,1);
  swhs->AddNode(cov,1,new TGeoTranslation(0.,0.,-4.5));
  swhs->AddNode(cov,2,new TGeoTranslation(0.,0.,4.5));
  //
  // SSW as an Assembly of sectors
  //
 TGeoVolumeAssembly *swheel = new TGeoVolumeAssembly("TPC_SSWHEEL");
   for(Int_t i =0;i<18;i++){
     Double_t phi = (20.*i);
     TGeoRotation *r = new TGeoRotation();
     r->RotateZ(phi);
     swheel->AddNode(swhs,i+1,r);   
   }
   v1->AddNode(swheel,1,new TGeoTranslation(0.,0.,-284.6));
   v1->AddNode(swheel,2,new TGeoTranslation(0.,0.,284.6));
   //
  //----------------------------------------------------------
  // TPc Support Rods - MAKROLON
  //----------------------------------------------------------
  TGeoMedium *m6=gGeoManager->GetMedium("TPC_Makrolon");
  TGeoMedium *m7=gGeoManager->GetMedium("TPC_Cu");
  TGeoMedium *m10 =  gGeoManager->GetMedium("TPC_Alumina");
  TGeoMedium *m11 =  gGeoManager->GetMedium("TPC_Peek");
  TGeoMedium *m13 = gGeoManager->GetMedium("TPC_Brass");
  TGeoMedium *m14 = gGeoManager->GetMedium("TPC_Alumina1");
  // 
  // tpc rod is an assembly of 10 long parts and 2 short parts
  // connected with alu rings and plagged on both sides.
  //
  //
// tpc rod long
//
  TGeoPcon *rod = new TGeoPcon("rod",0.,360.,6);
 rod->DefineSection(0,-10.43,1.92,2.08);
 rod->DefineSection(1,-9.75,1.92,2.08);

 rod->DefineSection(2,-9.75,1.8,2.2);
 rod->DefineSection(3,9.75,1.8,2.2);

 rod->DefineSection(4,9.75,1.92,2.08);
 rod->DefineSection(5,10.43,1.92,2.08);
 //
 TGeoVolume *mrodl = new TGeoVolume("TPC_mrodl",rod,m6);
 //
 // tpc rod short 
 //
 TGeoPcon *rod1 = new TGeoPcon("rod1",0.,360.,6);
 rod1->DefineSection(0,-8.93,1.92,2.08);
 rod1->DefineSection(1,-8.25,1.92,2.08);

 rod1->DefineSection(2,-8.25,1.8,2.2);
 rod1->DefineSection(3,8.25,1.8,2.2);

 rod1->DefineSection(4,8.25,1.92,2.08);
 rod1->DefineSection(5,8.93,1.92,2.08);
 //
 TGeoVolume *mrods = new TGeoVolume("TPC_mrods",rod1,m6);
 //
 // below is for the resistor rod
 //
 // hole for the brass connectors
 //

 new TGeoTube("hhole",0.,0.3,0.3);
 //
 //transformations for holes - initialy they
 // are placed at x=0 and negative y
 //
  TGeoRotation *rhole = new TGeoRotation();
  rhole->RotateX(90.);
  TGeoCombiTrans *transf[13];
  Char_t name[30];
  for(Int_t i=0;i<13;i++){
    //sprintf(name,"transf%d",i);
    snprintf(name,30,"transf%d",i);
    transf[i]= new TGeoCombiTrans(name,0.,-2.,-9.+i*1.5,rhole);
    transf[i]->RegisterYourself();
  }
  // union expression for holes
  TString operl("hhole:transf0");
  for (Int_t i=1;i<13;i++){
    //sprintf(name,"+hhole:transf%d",i);
    snprintf(name,30,"+hhole:transf%d",i);
    operl.Append(name);   
  }
  //
 TString opers("hhole:transf1");
  for (Int_t i=2;i<12;i++){
    //sprintf(name,"+hhole:transf%d",i);
    snprintf(name,30,"+hhole:transf%d",i); 
    opers.Append(name);   
  }
  //union of holes
  new TGeoCompositeShape("hlv",operl.Data());
  new TGeoCompositeShape("hsv",opers.Data());
  //
  TGeoCompositeShape *rodl = new TGeoCompositeShape("rodl","rod-hlv");
  TGeoCompositeShape *rods = new TGeoCompositeShape("rods","rod1-hsv");
 //rods - volumes - makrolon rods with holes
  TGeoVolume *rodlv = new TGeoVolume("TPC_rodl",rodl,m6);
  TGeoVolume *rodsv = new TGeoVolume("TPC_rods",rods,m6);
  //brass connectors
 //connectors
  TGeoTube *bcon = new TGeoTube(0.,0.3,0.3);//connectors
  TGeoVolume *bconv = new TGeoVolume("TPC_bcon",bcon,m13);
 //
 // hooks holding strips
 //
 new TGeoBBox("hk1",0.625,0.015,0.75);
 new TGeoBBox("hk2",0.625,0.015,0.15);
 TGeoTranslation *tr21 = new TGeoTranslation("tr21",0.,-0.03,-0.6);
 TGeoTranslation *tr12 = new TGeoTranslation("tr12",0.,-0.03,0.6);
 tr21->RegisterYourself();
 tr12->RegisterYourself();
 
 TGeoCompositeShape *hook = new TGeoCompositeShape("hook","hk1+hk2:tr21+hk2:tr12");
 TGeoVolume *hookv = new TGeoVolume("TPC_hook",hook,m13);
 //
 // assembly of the short rod with connectors and hooks
 //
 //
 // short rod
 //
  TGeoVolumeAssembly *spart = new TGeoVolumeAssembly("TPC_spart");
  //
  spart->AddNode( rodsv,1);
  for(Int_t i=1;i<12;i++){
  spart->AddNode(bconv,i,transf[i]);
  }
  for(Int_t i =0;i<11;i++){
    spart->AddNode(hookv,i+1,new TGeoTranslation(0.,-2.315,-7.5+i*1.5));
  } 
 //
 // long rod
 //
  TGeoVolumeAssembly *lpart = new TGeoVolumeAssembly("TPC_lpart");
  //
  lpart->AddNode( rodlv,1);
  for(Int_t i=0;i<13;i++){
  lpart->AddNode(bconv,i+12,transf[i]);
  }
  for(Int_t i =0;i<13;i++){
    lpart->AddNode(hookv,i+12,new TGeoTranslation(0.,-2.315,-9.+i*1.5));
  }   
  //
  // alu ring
  //
  new TGeoTube("ring1",2.1075,2.235,0.53);
  new TGeoTube("ring2",1.7925,1.89,0.43);
  new TGeoTube("ring3",1.89,2.1075,0.05);
  TGeoCompositeShape *ring = new TGeoCompositeShape("ring","ring1+ring2+ring3");
  TGeoVolume *ringv = new TGeoVolume("TPC_ring",ring,m3);
  //
  // rod assembly
  //
  TGeoVolumeAssembly *tpcrrod = new TGeoVolumeAssembly("TPC_rrod");//rrod
  TGeoVolumeAssembly *tpcmrod = new TGeoVolumeAssembly("TPC_mrod");//makrolon rod  
  //long pieces
  for(Int_t i=0;i<11;i++){
    tpcrrod->AddNode(ringv,i+1,new TGeoTranslation(0.,0.,-105.+i*21));
    tpcmrod->AddNode(ringv,i+12,new TGeoTranslation(0.,0.,-105.+i*21));
  }
  for(Int_t i=0;i<10;i++){
    tpcrrod->AddNode(lpart,i+1,new TGeoTranslation(0.,0.,-94.5+i*21));//resistor rod
    tpcmrod->AddNode(mrodl,i+1,new TGeoTranslation(0.,0.,-94.5+i*21));//makrolon rod	    
  }
  //
  // right plug - identical for all rods
  //
  TGeoPcon *tpcrp = new TGeoPcon(0.,360.,6);
  //
  tpcrp->DefineSection(0,123.05,1.89,2.1075);
  tpcrp->DefineSection(1,123.59,1.89,2.1075);
  //
  tpcrp->DefineSection(2,123.59,1.8,2.2);
  tpcrp->DefineSection(3,127.,1.8,2.2);
  //
  tpcrp->DefineSection(4,127.,0.,2.2);
  tpcrp->DefineSection(5,127.5,0.,2.2);
  //
  TGeoVolume *tpcrpv = new TGeoVolume("TPC_RP",tpcrp,m6);
  //
  // adding short pieces and right plug
  //
  tpcrrod->AddNode(spart,1,new TGeoTranslation(0.,0.,-114.));
  tpcrrod->AddNode(spart,2,new TGeoTranslation(0.,0.,114.)); 
  tpcrrod->AddNode(ringv,23,new TGeoTranslation(0.,0.,-123.));
  tpcrrod->AddNode(ringv,24,new TGeoTranslation(0.,0.,123.));
  tpcrrod->AddNode(tpcrpv,1);
  //
  tpcmrod->AddNode(mrods,1,new TGeoTranslation(0.,0.,-114.));
  tpcmrod->AddNode(mrods,2,new TGeoTranslation(0.,0.,114.)); 
  tpcmrod->AddNode(ringv,25,new TGeoTranslation(0.,0.,-123.));
  tpcmrod->AddNode(ringv,26,new TGeoTranslation(0.,0.,123.));
  tpcmrod->AddNode(tpcrpv,2);
  //
  // from the ringv position to the CM is 3.0 cm!
  //----------------------------------------
  //
  //
  //HV rods - makrolon + 0.58cm (diameter) Cu ->check the length
  TGeoTube *hvr = new TGeoTube(0.,1.465,123.);
  TGeoTube *hvc = new TGeoTube(0.,0.29,123.);
  //
  TGeoVolume *hvrv = new TGeoVolume("TPC_HV_Rod",hvr,m6);
  TGeoVolume *hvcv = new TGeoVolume("TPC_HV_Cable",hvc,m7);
  hvrv->AddNode(hvcv,1);
  //
  //resistor rod
  //
  TGeoTube *cr = new TGeoTube(0.,0.45,123.);   
  TGeoTube *cw = new TGeoTube(0.,0.15,123.);  
  TGeoVolume *crv = new TGeoVolume("TPC_CR",cr,m10);
  TGeoVolume *cwv = new TGeoVolume("TPC_W",cw,m12);   
  //
  // ceramic rod with water
  //
  crv->AddNode(cwv,1);
  //
  //peek rod
  //
  TGeoTube *pr =new TGeoTube(0.2,0.35,123.);  
  TGeoVolume *prv = new TGeoVolume("TPC_PR",pr,m11); 
  //
  // copper plates with connectors
  //
  new TGeoTube("tub",0.,1.7,0.025);
  //
  // half space - points on the plane and a normal vector
  //
  Double_t n[3],p[3];
  Double_t slope = TMath::Tan(22.*TMath::DegToRad());
  Double_t intp = 1.245;
  //
  Double_t b = slope*slope+1.;
  p[0]=intp*slope/b;
  p[1]=-intp/b;
  p[2]=0.;
  //
  n[0]=-p[0];
  n[1]=-p[1];
  n[2]=0.;
  Double_t norm;
  norm=TMath::Sqrt(n[0]*n[0]+n[1]*n[1]);
  n[0] /= norm;
  n[1] /=norm;
  //
  new TGeoHalfSpace("sp1",p,n);
  //
  slope = -slope; 
  //
  p[0]=intp*slope/b;
  p[1]=-intp/b;
  //
  n[0]=-p[0];
  n[1]=-p[1];
  norm=TMath::Sqrt(n[0]*n[0]+n[1]*n[1]);
  n[0] /= norm;
  n[1] /=norm;
  //
  new TGeoHalfSpace("sp2",p,n);
  // holes for rods
 //holes
 new TGeoTube("h1",0.,0.5,0.025);
 new TGeoTube("h2",0.,0.35,0.025);
 //translations:
 TGeoTranslation *ttr11 = new TGeoTranslation("ttr11",-0.866,0.5,0.);
 TGeoTranslation *ttr22 = new TGeoTranslation("ttr22",0.866,0.5,0.);
 ttr11->RegisterYourself();
 ttr22->RegisterYourself();
 // elastic connector
 new TGeoBBox("elcon",0.72,0.005,0.3);
 TGeoRotation *crr1 = new TGeoRotation();
 crr1->RotateZ(-22.);
TGeoCombiTrans *ctr1 = new TGeoCombiTrans("ctr1",-0.36011, -1.09951,-0.325,crr1);
ctr1->RegisterYourself();
 TGeoCompositeShape *cs1 = new TGeoCompositeShape("cs1",
"(((((tub-h1:ttr11)-h1:ttr22)-sp1)-sp2)-h2)+elcon:ctr1");
 //
 TGeoVolume *csvv = new TGeoVolume("TPC_RR_CU",cs1,m7);
 //
 // resistor rod assembly 2 ceramic rods, peak rod, Cu plates
 // and resistors
 //
 TGeoVolumeAssembly *rrod = new TGeoVolumeAssembly("TPC_RRIN");
 // rods
 rrod->AddNode(crv,1,ttr11);
 rrod->AddNode(crv,2,ttr22); 
 rrod->AddNode(prv,1);
 //Cu plates
 for(Int_t i=0;i<165;i++){
   rrod->AddNode(csvv,i+1,new TGeoTranslation(0.,0.,-122.675+i*1.5));
 }
 //resistors
 TGeoTube *res = new TGeoTube(0.,0.15,0.5);
 TGeoVolume *resv = new TGeoVolume("TPC_RES",res,m14);
 TGeoVolumeAssembly *ress = new TGeoVolumeAssembly("TPC_RES_CH");
 ress->AddNode(resv,1,new TGeoTranslation(0.2,0.,0.));
 ress->AddNode(resv,2,new TGeoTranslation(-0.2,0.,0.));
 //
 TGeoRotation *crr2 = new TGeoRotation();
 crr2->RotateY(30.);
 TGeoRotation *crr3 = new TGeoRotation();
 crr3->RotateY(-30.); 
 //
 for(Int_t i=0;i<164;i+=2){
   rrod->AddNode(ress,i+1, new TGeoCombiTrans(0.,1.2,-121.925+i*1.5,crr2));
   rrod->AddNode(ress,i+2, new TGeoCombiTrans(0.,1.2,-121.925+(i+1)*1.5,crr3));
 }

 tpcrrod->AddNode(rrod,1,new TGeoCombiTrans(0.,0.,0.5,crr1));
 //
 // rod left head with holders - inner
 //
 // first element - support for inner holder  TPC_IHS
 Double_t shift1[3] = {0.0,-0.175,0.0};

new TGeoBBox("tpcihs1", 4.7, 0.66, 2.35);               
new TGeoBBox("tpcihs2", 4.7, 0.485, 1.0, shift1);
new TGeoBBox("tpcihs3", 1.5, 0.485, 2.35, shift1);
new TGeoTube("tpcihs4", 0.0, 2.38, 0.1);
//
Double_t pointstrap[16];    
pointstrap[0]= 0.0;
pointstrap[1]= 0.0;
pointstrap[2]= 0.0;
pointstrap[3]= 1.08;
pointstrap[4]= 2.3;
pointstrap[5]= 1.08;
pointstrap[6]= 3.38;
pointstrap[7]= 0.0;
pointstrap[8]= 0.0;
pointstrap[9]= 0.0;
pointstrap[10]= 0.0;
pointstrap[11]= 1.08;
pointstrap[12]= 2.3;
pointstrap[13]= 1.08;
pointstrap[14]= 3.38;
pointstrap[15]= 0.0;
//
TGeoArb8 *tpcihs5 = new TGeoArb8("tpcihs5", 0.6, pointstrap);
//
//  half space - cutting "legs"
// 
 p[0]=0.0;
 p[1]=0.105;
 p[2]=0.0;
 //       
n[0] = 0.0;
n[1] = 1.0;
n[2] = 0.0;

new TGeoHalfSpace("cutil1", p, n);

//
// transformations
//
TGeoTranslation *trans2 = new TGeoTranslation("trans2", 0.0, 2.84, 2.25);
trans2->RegisterYourself();
TGeoTranslation*trans3= new TGeoTranslation("trans3", 0.0, 2.84, -2.25);
trans3->RegisterYourself();
//support - composite volume
//
TGeoCompositeShape *tpcihs6 = new TGeoCompositeShape("tpcihs6", "tpcihs1-(tpcihs2+tpcihs3)-(tpcihs4:trans2)-(tpcihs4:trans3)-cutil1");
//
// volumes - all makrolon
//  
 TGeoVolume *tpcihss = new TGeoVolume("TPC_IHSS", tpcihs6, m6); //support
 TGeoVolume *tpcihst = new TGeoVolume("TPC_IHSTR",tpcihs5 , m6); //trapesoid
 //now assembly
TGeoRotation *rot111 = new TGeoRotation(); 
rot111->RotateY(180.0);
//
TGeoVolumeAssembly *tpcihs = new TGeoVolumeAssembly("TPC_IHS");    // assembly of the support
tpcihs->AddNode(tpcihss, 1);
tpcihs->AddNode(tpcihst, 1, new TGeoTranslation(-4.7, 0.66, 0.0));
tpcihs->AddNode(tpcihst, 2, new TGeoCombiTrans(4.7, 0.66, 0.0, rot111));
//
// two rod holders (TPC_IRH) assembled with the support
//
new TGeoBBox("tpcirh1", 4.7, 1.33, 0.5);
 shift1[0]=-3.65;
 shift1[1]=0.53;
 shift1[2]=0.;
new TGeoBBox("tpcirh2", 1.05, 0.8, 0.5, shift1);
 shift1[0]=3.65;
 shift1[1]=0.53;
 shift1[2]=0.;
new TGeoBBox("tpcirh3", 1.05, 0.8, 0.5, shift1);
 shift1[0]=0.0;
 shift1[1]=1.08;
 shift1[2]=0.;
new TGeoBBox("tpcirh4", 1.9, 0.25, 0.5, shift1);
new TGeoTube("tpcirh5", 0, 1.9, 5);
//
TGeoTranslation *trans4 = new TGeoTranslation("trans4", 0, 0.83, 0.0); 
trans4->RegisterYourself();
//
TGeoCompositeShape *tpcirh6 = new TGeoCompositeShape("tpcirh6", "tpcirh1-tpcirh2-tpcirh3-(tpcirh5:trans4)-tpcirh4");
//
// now volume
//
TGeoVolume *tpcirh = new TGeoVolume("TPC_IRH", tpcirh6, m6);   
//
// and all together...
//
TGeoVolume *tpciclamp = new TGeoVolumeAssembly("TPC_ICLP");
tpciclamp->AddNode(tpcihs, 1);
tpciclamp->AddNode(tpcirh, 1, new TGeoTranslation(0, 1.99, 1.1));
tpciclamp->AddNode(tpcirh, 2, new TGeoTranslation(0, 1.99, -1.1));
//
// and now left inner "head"
//
TGeoPcon *inplug = new TGeoPcon("inplug", 0.0, 360.0, 13); 

inplug->DefineSection(0, 0.3, 0.0, 2.2);
inplug->DefineSection(1, 0.6, 0.0, 2.2);

inplug->DefineSection(2, 0.6, 0.0, 1.75);
inplug->DefineSection(3, 0.7, 0.0, 1.75);

inplug->DefineSection(4, 0.7, 1.55, 1.75);
inplug->DefineSection(5, 1.6, 1.55, 1.75);

inplug->DefineSection(6, 1.6, 1.55, 2.2);

inplug->DefineSection(7, 1.875, 1.55, 2.2);
inplug->DefineSection(8, 2.47, 1.75, 2.2);

inplug->DefineSection(9, 2.47, 1.75, 2.08);
inplug->DefineSection(10, 2.57, 1.8, 2.08);

inplug->DefineSection(11, 2.57, 1.92, 2.08);
inplug->DefineSection(12, 2.95, 1.92, 2.08);
//
 shift1[0]=0.0;
 shift1[1]=-2.09;
 shift1[2]=1.075;
 //
new TGeoBBox("pcuti", 1.5, 0.11, 1.075, shift1);
//
TGeoCompositeShape *inplleft = new TGeoCompositeShape("inplleft", "inplug-pcuti");
TGeoVolume *tpcinlplug = new TGeoVolume("TPC_INPLL", inplleft, m6);   
//
//  holder + plugs
//
 TGeoVolume *tpcihpl = new TGeoVolumeAssembly("TPC_IHPL"); //holder+2 plugs (reflected)
 tpcihpl->AddNode(tpcinlplug, 1);
 tpcihpl->AddNode(tpcinlplug, 2,ref);
 tpcihpl->AddNode(tpciclamp,1,new TGeoTranslation(0.0, -2.765, 0.0)); 
 //
 // outer holders and clamps
 //

 // outer membrane holder (between rods)  
pointstrap[0]= 0.0;
pointstrap[1]= 0.0;
pointstrap[2]= 0.0;
pointstrap[3]= 2.8;
pointstrap[4]= 3.1;
pointstrap[5]= 2.8-3.1*TMath::Tan(15.*TMath::DegToRad());
pointstrap[6]= 3.1;
pointstrap[7]= 0.0;
pointstrap[8]= 0.0;
pointstrap[9]= 0.0;
pointstrap[10]= 0.0;
pointstrap[11]= 2.8;
pointstrap[12]= 3.1;
pointstrap[13]= 2.8-3.1*TMath::Tan(15.*TMath::DegToRad());
pointstrap[14]= 3.1;
pointstrap[15]= 0.0;
//
TGeoArb8 *tpcomh1 = new TGeoArb8("tpcomh1", 1.05, pointstrap);   
TGeoBBox *tpcomh2 = new TGeoBBox("tpcomh2", 0.8, 1.4, 6);
//
TGeoVolume *tpcomh1v = new TGeoVolume("TPC_OMH1", tpcomh1, m7);    
TGeoVolume *tpcomh2v = new TGeoVolume("TPC_OMH2", tpcomh2, m7);
//
TGeoVolume *tpcomh3v = new TGeoVolumeAssembly("TPC_OMH3");    // assembly1
tpcomh3v->AddNode(tpcomh1v, 1, new TGeoTranslation(0.8, -1.4, 4.95));
 tpcomh3v->AddNode(tpcomh1v, 2, new TGeoTranslation(0.8, -1.4, -4.95));
tpcomh3v->AddNode(tpcomh2v, 1);
//
shift1[0] = 0.9;
shift1[1] = -1.85;
shift1[2] = 0.0;
//
new TGeoBBox("tpcomh3", 1.65, 1.15, 3.4);
TGeoBBox *tpcomh4 = new TGeoBBox("tpcomh4", 0.75, 0.7, 3.4, shift1);
//
// halfspace 1
//
p[0] = 0.0;
p[1] = -1.05;   
p[2] = -3.4;
//
n[0] = 0.0;
n[1] = -1.0*TMath::Tan(30.*TMath::DegToRad());
n[2] = 1.0;
//
new TGeoHalfSpace("cutomh1", p, n); 
//
// halfspace 2
//
p[0] = 0.0;
p[1] = -1.05;   
p[2] = 3.4;
//
n[0] = 0.0;
n[1] = -1.0*TMath::Tan(30.*TMath::DegToRad());
n[2] = -1.0;
//
new TGeoHalfSpace("cutomh2", p, n);
//
// halfspace 3
//
p[0] = -1.65;
p[1] = 0.0;     
p[2] = -0.9;
//
n[0] = 1.0*TMath::Tan(75.*TMath::DegToRad());
n[1] = 0.0;
n[2] = 1.0;
//
new TGeoHalfSpace("cutomh3", p, n);   
//
// halfspace 4
//
p[0] = -1.65;
p[1] = 0.0;      
p[2] = 0.9;
//
n[0] = 1.0*TMath::Tan(75*TMath::DegToRad());
n[1] = 0.0;
n[2] = -1.0;
//
new TGeoHalfSpace("cutomh4", p, n); 
//
// halsfspace 5
//
p[0] = 1.65;
p[1] = -1.05;      
p[2] = 0.0;  
//
n[0] = -1.0;
n[1] = -1.0*TMath::Tan(20.*TMath::DegToRad());
n[2] = 0.0;
//
new TGeoHalfSpace("cutomh5", p, n); 
//  
TGeoCompositeShape *tpcomh5 = new TGeoCompositeShape("tpcomh5", "tpcomh3-cutomh1-cutomh2-cutomh3-cutomh4-cutomh5"); 
//
 TGeoVolume *tpcomh5v = new TGeoVolume("TPC_OMH5",tpcomh5,m6);
 TGeoVolume *tpcomh4v = new TGeoVolume("TPC_OMH6",tpcomh4,m6);
 //
 TGeoVolumeAssembly *tpcomh7v = new TGeoVolumeAssembly("TPC_OMH7");
 tpcomh7v->AddNode(tpcomh5v,1);
 tpcomh7v->AddNode(tpcomh4v,1);
 //
 // full membrane holder - tpcomh3v + tpcomh7v
 //
 TGeoVolumeAssembly *tpcomh = new TGeoVolumeAssembly("TPC_OMH");
 tpcomh->AddNode(tpcomh3v,1,new TGeoTranslation(1.5,0.,0.));
 tpcomh->AddNode(tpcomh3v,2,new TGeoCombiTrans(-1.5,0.,0.,rot111));
 tpcomh->AddNode(tpcomh7v,1,new TGeoTranslation(0.65+1.5, 2.55, 0.0));
 tpcomh->AddNode(tpcomh7v,2,new TGeoCombiTrans(-0.65-1.5, 2.55, 0.0,rot111)); 
 //
 //  outer rod holder support
 //
new TGeoBBox("tpcohs1", 3.8, 0.675, 2.35); 
//
shift1[0] = 0.0;
shift1[1] = 0.175;
shift1[2] = 0.0;
//
new TGeoBBox("tpcohs2", 1.5, 0.5, 2.35, shift1);
new TGeoBBox("tpcohs3", 3.8, 0.5, 0.85, shift1);
//
shift1[0] = 0.0;
shift1[1] = -1.175;
shift1[2] = 0.0;
//
TGeoBBox *tpcohs4 = new TGeoBBox("tpsohs4", 3.1, 0.5, 0.7, shift1);
//
TGeoVolume *tpcohs4v = new TGeoVolume("TPC_OHS4", tpcohs4, m6);
//
p[0] = 0.0;
p[1] = -0.186;   
p[2] = 0.0;
//
n[0] = 0.0;
n[1] = -1.0;
n[2] = 0.0;
//
new TGeoHalfSpace("cutohs1", p, n);
//
TGeoCompositeShape *tpcohs5 = new TGeoCompositeShape("tpcohs5", "tpcohs1-tpcohs2-tpcohs3-cutohs1"); 
TGeoVolume *tpcohs5v = new TGeoVolume("TPC_OHS5", tpcohs5, m6); 
//
TGeoVolumeAssembly *tpcohs = new TGeoVolumeAssembly("TPC_OHS");    
tpcohs->AddNode(tpcohs5v, 1);
tpcohs->AddNode(tpcohs4v, 1); 
//
// outer rod holder itself
//
shift1[0] = 0.0;
shift1[1] = 1.325;
shift1[2] = 0.0;
 new TGeoBBox("tpcorh1", 3.1, 1.825, 0.55); //from this box we cut pieces...
 //
shift1[0] = -3.1;	
shift1[1] = -0.5;	
shift1[2] = 0.0;
//
new TGeoBBox("tpcorh2", 0.5, 2.75, 1.1, shift1);
//
shift1[0] = 3.1;	
shift1[1] = -0.5;	
shift1[2] = 0.0;
//
new TGeoBBox("tpcorh3", 0.5, 2.75, 1.1, shift1);
//
shift1[0] = 0.0;
shift1[1] = -0.5;	
shift1[2] = -0.95;
//
new TGeoBBox("tpcorh4", 3.9, 2.75, 0.5, shift1);
//
shift1[0] = 0.0;
shift1[1] = -0.5;
shift1[2] = 0.0;
//
new TGeoBBox("tpcorh5", 1.95, 0.5, 1.1, shift1);
//
shift1[0] = 0.0;
shift1[1] = -0.5;
shift1[2] = 0.55;
//
new TGeoBBox("tpcorh6", 2.4, 0.5, 0.6, shift1);
//
new TGeoTube("tpcorh7", 0, 1.95, 0.85);
new TGeoTube("tpcorh8", 0, 2.4, 0.6);
//
TGeoTranslation *trans33 = new TGeoTranslation("trans33", 0.0, 0.0, 0.55); 
trans33->RegisterYourself();
//
TGeoCompositeShape *tpcorh9 = new TGeoCompositeShape("tpcorh9", "tpcorh1-tpcorh2-tpcorh3-tpcorh4-tpcorh5-tpcorh6-(tpcorh8:trans33)-tpcorh7");
//
 TGeoVolume *tpcorh9v = new TGeoVolume("TPC_ORH",tpcorh9,m6); //outer rod holder 
 //
 // now 2 holders together
 //
 TGeoVolumeAssembly *tpcorh = new TGeoVolumeAssembly("TPC_ORH2");
 //
 tpcorh->AddNode(tpcorh9v,1,new TGeoTranslation(0.0, 0.0, 1.25));
 tpcorh->AddNode(tpcorh9v,2,new TGeoCombiTrans(0.0, 0.0, -1.25,rot111));
//
// outer rod plug left
//
TGeoPcon *outplug = new TGeoPcon("outplug", 0.0, 360.0, 13); 

outplug->DefineSection(0, 0.5, 0.0, 2.2);
outplug->DefineSection(1, 0.7, 0.0, 2.2);

outplug->DefineSection(2, 0.7, 1.55, 2.2);
outplug->DefineSection(3, 0.8, 1.55, 2.2);

outplug->DefineSection(4, 0.8, 1.55, 1.75);
outplug->DefineSection(5, 1.2, 1.55, 1.75);

outplug->DefineSection(6, 1.2, 1.55, 2.2);
outplug->DefineSection(7, 1.875, 1.55, 2.2);

outplug->DefineSection(8, 2.47, 1.75, 2.2);

outplug->DefineSection(9, 2.47, 1.75, 2.08);
outplug->DefineSection(10, 2.57, 1.8, 2.08);

outplug->DefineSection(11, 2.57, 1.92, 2.08);
outplug->DefineSection(12, 2.95, 1.92, 2.08);
//
shift1[0] = 0.0;
shift1[1] = 2.09;
shift1[2] = 1.01;

new TGeoBBox("cutout", 2.5, 0.11, 1.01, shift1);
//

TGeoCompositeShape *outplleft = new TGeoCompositeShape("outplleft", "outplug-cutout");    
TGeoVolume *outplleftv = new TGeoVolume("TPC_OPLL", outplleft, m6); 
//
//  support + holder + plug
//

 
 TGeoVolumeAssembly *tpcohpl = new TGeoVolumeAssembly("TPC_OHPL"); 
 //
 tpcohpl->AddNode(outplleftv,1); //plug
 tpcohpl->AddNode(outplleftv,2,ref); //plug reflected
 tpcohpl->AddNode(tpcorh,1); //rod holder
 tpcohpl->AddNode(tpcohs,1,new TGeoTranslation(0.0, 3.925, 0)); // support
 //

 //
 // main membrane holder
 //	      
pointstrap[0]= 0.0;
pointstrap[1]= 0.0;
pointstrap[2]= 0.0;
pointstrap[3]= 2.8;
pointstrap[4]= 3.1;
pointstrap[5]= 1.96;
pointstrap[6]= 3.1;
pointstrap[7]= 0.0;
pointstrap[8]= 0.0;
pointstrap[9]= 0.0;
pointstrap[10]= 0.0;
pointstrap[11]= 2.8;
pointstrap[12]= 3.1;
pointstrap[13]= 1.96;
pointstrap[14]= 3.1;
pointstrap[15]= 0.0; 
//
TGeoArb8 *tpcmmh1 = new TGeoArb8("tpcmmh1", 1.75, pointstrap);
TGeoBBox *tpcmmh2 = new TGeoBBox("tpcmmh2", 0.8, 1.4, 12.5); 
//
TGeoVolume *tpcmmh1v = new TGeoVolume("TPC_MMH1", tpcmmh1, m6);   
TGeoVolume *tpcmmh2v = new TGeoVolume("TPC_MMH2", tpcmmh2, m6);
//
 TGeoVolumeAssembly *tpcmmhs = new TGeoVolumeAssembly("TPC_MMHS");
 tpcmmhs->AddNode(tpcmmh1v,1,new TGeoTranslation(0.8, -1.4, 10.75));
 tpcmmhs->AddNode(tpcmmh1v,2,new TGeoTranslation(0.8, -1.4, -10.75));
 tpcmmhs->AddNode(tpcmmh2v,1);
 //
 // main membrahe holder clamp
 //
shift1[0] = -0.75;
shift1[1] = -1.15;
shift1[2] = 0.0;
//
new TGeoBBox("tpcmmhc1", 1.65, 1.85, 8.9); 
new TGeoBBox("tpcmmhc2", 0.9, 0.7, 8.9, shift1);
//
// half spaces  - cuts
//
p[0] = -1.65;
p[1] = 0.0;   
p[2] = -0.9;
//
n[0] = 8.0;
n[1] = 0.0;
n[2] = 8.0*TMath::Tan(13.*TMath::DegToRad());
//
new TGeoHalfSpace("cutmmh1", p, n);
//
p[0] = -1.65;
p[1] = 0.0;   
p[2] = 0.9;
//
n[0] = 8.0;
n[1] = 0.0;
n[2] = -8.0*TMath::Tan(13.*TMath::DegToRad());
//
new TGeoHalfSpace("cutmmh2", p, n);
//
p[0] = 0.0;
p[1] = 1.85;   
p[2] = -2.8;
//
n[0] = 0.0;
n[1] = -6.1;
n[2] = 6.1*TMath::Tan(20.*TMath::DegToRad());
//
new TGeoHalfSpace("cutmmh3", p, n);
//
p[0] = 0.0;
p[1] = 1.85;      
p[2] = 2.8;
//
n[0] = 0.0;
n[1] = -6.1;
n[2] = -6.1*TMath::Tan(20*TMath::DegToRad());
//
new TGeoHalfSpace("cutmmh4", p, n);
//
p[0] = 0.75;
p[1] = 0.0; 
p[2] = -8.9;
//
n[0] = 2.4*TMath::Tan(30*TMath::DegToRad());
n[1] = 0.0;
n[2] = 2.4;
//
 new TGeoHalfSpace("cutmmh5", p, n);
//
p[0] = 0.75;
p[1] = 0.0;
p[2] = 8.9;
//
n[0] = 2.4*TMath::Tan(30*TMath::DegToRad());
n[1] = 0.0;
n[2] = -2.4;
//
new TGeoHalfSpace("cutmmh6", p, n);

TGeoCompositeShape *tpcmmhc = new TGeoCompositeShape("TPC_MMHC", "tpcmmhc1-tpcmmhc2-cutmmh1-cutmmh2-cutmmh3-cutmmh4-cutmmh5-cutmmh6"); 

 TGeoVolume *tpcmmhcv = new TGeoVolume("TPC_MMHC",tpcmmhc,m6);
 //
TGeoVolume *tpcmmh = new TGeoVolumeAssembly("TPC_MMH");
//
 tpcmmh->AddNode(tpcmmhcv,1,new TGeoTranslation(0.65+1.5, 1.85, 0.0));
 tpcmmh->AddNode(tpcmmhcv,2,new TGeoCombiTrans(-0.65-1.5, 1.85, 0.0,rot111));
 tpcmmh->AddNode(tpcmmhs,1,new TGeoTranslation(1.5, 0.0, 0.0));
 tpcmmh->AddNode(tpcmmhs,2,new TGeoCombiTrans(-1.5, 0.0, 0.0,rot111));
 //
 
 //

//--------------------------------------------
 //
 // guard ring resistor chain
 //

 TGeoTube *gres1 = new TGeoTube(0.,0.375,125.);// inside ifc
 //
 TGeoVolume *vgres1 = new TGeoVolume("TPC_GRES1",gres1,m14);

 //
 Double_t xrc,yrc;
 //
 xrc=79.3*TMath::Cos(350.*TMath::DegToRad());
 yrc=79.3*TMath::Sin(350.*TMath::DegToRad());
 //
 v9->AddNode(vgres1,1,new TGeoTranslation(xrc,yrc,126.9));
 v9->AddNode(vgres1,2,new TGeoTranslation(xrc,yrc,-126.9));
 //
 xrc=79.3*TMath::Cos(190.*TMath::DegToRad());
 yrc=79.3*TMath::Sin(190.*TMath::DegToRad()); 
 //
 v9->AddNode(vgres1,3,new TGeoTranslation(xrc,yrc,126.9));
 v9->AddNode(vgres1,4,new TGeoTranslation(xrc,yrc,-126.9));
 //------------------------------------------------------------------
 TGeoRotation refl("refl",90.,0.,90.,90.,180.,0.);
 TGeoRotation rotrod("rotrod");
 //
 TGeoRotation *rotpos[2]; 
 //
 TGeoRotation *rotrod1[2]; 

  
 //v9 - drift gas

    TGeoRotation rot102("rot102");
    rot102.RotateY(-90.);

  for(Int_t i=0;i<18;i++){
    Double_t angle,x,y;
    Double_t z,r; 
    angle=TMath::DegToRad()*20.*(Double_t)i;
    //inner rods
    r=81.5;
    x=r * TMath::Cos(angle);
    y=r * TMath::Sin(angle);
    z = 126.;
    TGeoRotation *rot12 = new TGeoRotation();
    rot12->RotateZ(-90.0+i*20.);
    v9->AddNode(tpcihpl,i+1,new TGeoCombiTrans(x, y, 0., rot12));
    //
    if(i==11){//resistor rod inner
       rotrod.RotateZ(-90.+angle);
       rotrod1[0]= new TGeoRotation();
       rotpos[0]= new TGeoRotation();
       //
       rotrod1[0]->RotateZ(-90.+angle);
       *rotpos[0] = refl*rotrod; //rotation+reflection
	v9->AddNode(tpcrrod,1,new TGeoCombiTrans(x,y, z, rotrod1[0])); //A
	v9->AddNode(tpcrrod,2,new TGeoCombiTrans(x,y,-z, rotpos[0])); //C      
    } 
    else { 
      v9->AddNode(tpcmrod,i+1,new TGeoTranslation(x,y,z));//shaft
      v9->AddNode(tpcmrod,i+19,new TGeoCombiTrans(x,y,-z,ref));//muon
    }
    // outer rods
    r=254.25;
    x=r * TMath::Cos(angle);
    y=r * TMath::Sin(angle);
    z=126.;
    //
    // outer rod holder + outer left plug
    //
    TGeoRotation *rot33 = new TGeoRotation();
    rot33->RotateZ(-90+i*20.);
    //
    v9->AddNode(tpcohpl,i+1,new TGeoCombiTrans(x, y, 0., rot33));
    //
    Double_t xxx = 256.297*TMath::Cos((i*20.+10.)*TMath::DegToRad());
    Double_t yyy = 256.297*TMath::Sin((i*20.+10.)*TMath::DegToRad());
    //
    TGeoRotation rot101("rot101");
    rot101.RotateZ(90.+i*20.+10.);
    TGeoRotation *rot103 = new TGeoRotation("rot103");
    *rot103 = rot101*rot102;
    //
    TGeoCombiTrans *trh100 = new TGeoCombiTrans(xxx,yyy,0.,rot103);
    //
    if(i==2) {
      //main membrane holder
      v9->AddNode(tpcmmh,1,trh100);
    }
    else{
      // "normal" membrane holder
      v9->AddNode(tpcomh,i+1,trh100);      
    }

    //
    if(i==3){//resistor rod outer
      rotrod.RotateZ(90.+angle);
      rotrod1[1]= new TGeoRotation();
      rotpos[1]= new TGeoRotation();
      rotrod1[1]->RotateZ(90.+angle);
      *rotpos[1] = refl*rotrod;//rotation+reflection
      v9->AddNode(tpcrrod,3,new TGeoCombiTrans(x,y, z, rotrod1[1])); //A 
      v9->AddNode(tpcrrod,4,new TGeoCombiTrans(x,y, -z, rotpos[1])); //C
    }
    else {
      v9->AddNode(tpcmrod,i+37,new TGeoTranslation(x,y,z));//shaft
      v9->AddNode(tpcmrod,i+55,new TGeoCombiTrans(x,y,-z,ref));//muon      
    }
    if(i==15){
      v9->AddNode(hvrv,1,new TGeoTranslation(x,y,z+0.7)); //hv->A-side only      


    }
  } //end of rods positioning

  TGeoVolume *alice = gGeoManager->GetVolume("ALIC");
  alice->AddNode(v1,1);
  

} // end of function



//_____________________________________________________________________________
void AliTPCv0::CreateMaterials()
{
  //
  // Define materials for the TPC
  //
  AliTPC::CreateMaterials();
}


//_____________________________________________________________________________
void AliTPCv0::Init()
{
  //
  // Initialise Time Projection Chamber version 0
  //
 
  printf("%s: *** TPC version 0 initialized***\n",ClassName()); 

  //  printf("TPC version 0 initialized\n");
}

//_____________________________________________________________________________
void AliTPCv0::StepManager()
{
  //
  // Procedure called at each step in the TPC
  //
}
