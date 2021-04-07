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
// $MpId$

#include "AliMpDEVisu.h"

#include "AliMpSlatMotifMap.h"
#include "AliMpSt345Reader.h"
#include "AliMpSectorReader.h"
#include "AliMpSlat.h"
#include "AliMpPCB.h"
#include "AliMpSectorReader.h"
#include "AliMpSector.h"
#include "AliMpRow.h"
#include "AliMpVRowSegment.h"
#include "AliMpVPainter.h"
#include "AliMpMotifPosition.h"
#include "AliMpMotifMap.h"
#include "AliMpDEIterator.h"
#include "AliMpDEManager.h"
#include "AliMpDetElement.h"
#include "AliMpStationType.h"
#include "AliMpSegmentation.h"
#include "AliMpPad.h"
#include "AliMpDDLStore.h"
#include "AliMpManuStore.h"
#include "AliMpVPadIterator.h"
#include "AliMpCDB.h"
#include "AliMpDataStreams.h"

#include "AliLog.h"

#include <TMath.h>
#include <TString.h>
#include <TCanvas.h>
#include <TGButton.h>
#include <TRootEmbeddedCanvas.h>
#include <TGLabel.h>
#include <TGComboBox.h>
#include <TGNumberEntry.h>
#include <TGTextView.h>
#include <TGTextEntry.h>
#include <TVirtualX.h>

#include <cstdio>

// Category: graphics

//-----------------------------------------------------------------------------
// Class AliMpDEVisu
// -----------------------
// GUI for drawing segmentation
// motif manu and associated channels
// date: 2007/01/26
// Author: Ch. Finck
//-----------------------------------------------------------------------------

/// \cond CLASSIMP
ClassImp(AliMpDEVisu)
/// \endcond

//_________________________________________________________________
AliMpDEVisu::AliMpDEVisu(UInt_t w, UInt_t h) 
: TGFrame(gClient->GetRoot(), w, h),
  fkMainWindow(gClient->GetRoot()),
  fMain(new TGMainFrame(gClient->GetRoot(), w, h)),
  fEcanvas(0x0),
  fChamberCombo(0x0),
  fDECombo(0x0),
  fNumberEntry(0x0),
  fPlaneButton(0x0),
  fZoomButton(0x0),
  fNameDECombo(0x0),
  fLogMessage(0x0),
  fLogFile(0x0),
  fTrashList(0x0),
  fDEComboIdx(),
  fNameDEComboIdx(),
  fDEOccurrence(1026),
  fCurrentPlane(AliMp::kBendingPlane),
  fCurrentDetElem(100),
  fCurrentDEName(),
  fkSegmentation(),
  fDDLStore(0x0),
  fManuStore(0x0),
  fZoomMode(false)
{
/// Standard constructor

  // Load mapping
  if ( ! AliMpCDB::LoadDDLStore() ) {
    AliFatal("Could not access mapping from OCDB !");
  }

  if ( ! AliMpCDB::LoadManuStore() ) {
    AliFatal("Could not access run-dependent mapping from OCDB !");
  }

  fDDLStore = AliMpDDLStore::Instance();
  fManuStore = AliMpManuStore::Instance();

  fTrashList.SetOwner(kFALSE);
  
  // Create canvas widget
  
  Int_t width  = Int_t(w*0.99);
  Int_t height = Int_t(h*0.99);
  
  fEcanvas = new TRootEmbeddedCanvas("Ecanvas",fMain, width, height);
  fEcanvas->GetCanvas()->Connect("ProcessedEvent(Int_t, Int_t, Int_t, TObject*)",
                                 "AliMpDEVisu",
                                 this,
                                 "HandleMovement(Int_t, Int_t, Int_t, TObject*)");

  // Create a horizontal frame widget with buttons
  TGHorizontalFrame *hframe = new TGHorizontalFrame(fMain,width,height/6);
  
  TGTextButton *draw = new TGTextButton(hframe,"&Draw");
  draw->Connect("Clicked()","AliMpDEVisu",this,"DrawDE()");
  hframe->AddFrame(draw, new TGLayoutHints(kLHintsLeft | kLHintsCenterY,15,5,3,4));
  
  TGTextButton *exit = new TGTextButton(hframe,"&Exit","gApplication->Terminate(0)");
  hframe->AddFrame(exit, new TGLayoutHints(kLHintsLeft| kLHintsCenterY,5,5,3,4));
  
  
  Int_t i = 0;
  Char_t text[20];
  
  // chamber label
  TGLabel* chamberLabel = new TGLabel(hframe, "Chamber :");
  hframe->AddFrame(chamberLabel, new TGLayoutHints(kLHintsCenterX | kLHintsLeft | kLHintsCenterY, 10, 0, 20, 0));
  fChamberCombo = new TGComboBox(hframe, kChamberCombo);
  
  fDEComboIdx.Set(26);
  for(i = 0; i < 10; i++)
  {
    snprintf(text,20,"%d",i+1);
    fChamberCombo->AddEntry(text,i);
  }
  fChamberCombo->Resize(40,20);
  fChamberCombo->Select(0);
  fChamberCombo->Associate(this);
  hframe->AddFrame(fChamberCombo, new TGLayoutHints(kLHintsLeft, 10, 0, 9, 0));
  
  // DE label
  TGLabel*  detElemLabel = new TGLabel(hframe, "DE :");
  hframe->AddFrame(detElemLabel, new TGLayoutHints(kLHintsCenterX | kLHintsLeft | kLHintsCenterY, 10, 0, 20, 0));
  fDECombo = new TGComboBox(hframe, kDECombo);
  UpdateComboDE();
  
  fDECombo->Resize(80,20);
  fDECombo->Select(0);
  fDECombo->Associate(this);
  hframe->AddFrame(fDECombo, new TGLayoutHints(kLHintsLeft, 10, 0, 9, 0));
  fDEOccurrence.Reset(-1);

  TGTextButton *next = new TGTextButton(hframe,"&Next");
  next->Connect("Clicked()","AliMpDEVisu",this,"NextDE()");
  hframe->AddFrame(next, new TGLayoutHints(kLHintsLeft | kLHintsCenterY,5,5,3,4));
  
  // DE name
  TGLabel*  detElemName = new TGLabel(hframe, "Name :");
  hframe->AddFrame(detElemName, new TGLayoutHints(kLHintsCenterX | kLHintsLeft | kLHintsCenterY, 10, 0, 20, 0));
  
  AliMpDetElement* detElem = AliMpDEManager::GetDetElement(fCurrentDetElem);
  fCurrentDEName = detElem->GetDEName();
  fNameDECombo = new  TGComboBox(hframe, kDEName);

  UpdateNameView(kTRUE);
  fNameDECombo->Resize(160, 20);
  fNameDECombo->Select(0);
  fNameDECombo->Associate(this);
  hframe->AddFrame(fNameDECombo, new TGLayoutHints(kLHintsLeft, 10, 0, 9, 0));

  // plane type
  fPlaneButton = new TGCheckButton(hframe, "NB Plane", kPlaneType);
  fPlaneButton->SetState(kButtonUp);
  fPlaneButton->Associate(this);
  hframe->AddFrame(fPlaneButton, new TGLayoutHints(kLHintsLeft, 10, 0, 9, 0));
  
  
  // button motif
  TGTextButton* drawManu = new TGTextButton(hframe,"&Search");
  drawManu->Connect("Clicked()","AliMpDEVisu",this,"DrawManuMotif(Bool_t)");
  drawManu->SetToolTipText("Search for a given manu number");
  hframe->AddFrame(drawManu, new TGLayoutHints(kLHintsLeft | kLHintsCenterY,15,5,3,4));
  
  // entry manu
  fNumberEntry  = new TGNumberEntry(hframe, 0, 4, -1, 
                                    TGNumberFormat::kNESInteger, TGNumberFormat::kNEANonNegative,
                                    TGNumberFormat::kNELLimitMinMax, 1, 1500);
  fNumberEntry->Resize(60,20);
  hframe->AddFrame(fNumberEntry, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 10, 0, 9, 0));
  
  // reset button
  TGTextButton *resetManu = new TGTextButton(hframe,"&Reset");
  resetManu->Connect("Clicked()","AliMpDEVisu",this,"ResetManu()");
  hframe->AddFrame(resetManu, new TGLayoutHints(kLHintsLeft | kLHintsCenterY,10,5,3,4));
  
  // delete window button
  TGTextButton* deletePopup = new TGTextButton(hframe,"&Delete_Popup");
  deletePopup->Connect("Clicked()","AliMpDEVisu",this,"DeletePopUp()");
  deletePopup->SetToolTipText("Delete motif popup window");
  hframe->AddFrame(deletePopup, new TGLayoutHints(kLHintsLeft | kLHintsCenterY,15,5,3,4));
  
  
  // log mesg
  TGHorizontalFrame *logFrame = new TGHorizontalFrame(fMain, width/2, height/6);
  
  TGLabel*  logName = new TGLabel(logFrame, "Log MSG:");
  logFrame->AddFrame(logName, new TGLayoutHints(kLHintsCenterX | kLHintsLeft | kLHintsCenterY, 10, 0, 2, 0));
  
  fLogMessage = new TGTextView(logFrame, width/2, 60);
  fLogMessage->ShowBottom();
  
  logFrame->AddFrame(fLogMessage, new TGLayoutHints(kLHintsLeft, 10, 0, 2, 0));
  
  // clear log mesg
  TGTextButton* clearLog = new TGTextButton(logFrame,"&Clear");
  clearLog->Connect("Clicked()","AliMpDEVisu",this,"ClearLogMessage()");
  clearLog->SetToolTipText("Clear log message");
  logFrame->AddFrame(clearLog, new TGLayoutHints(kLHintsLeft | kLHintsCenterY,15,5,3,4));
  
  // save log mesg
  TGTextButton* saveLog = new TGTextButton(logFrame,"&Save");
  saveLog->Connect("Clicked()","AliMpDEVisu",this,"SaveLogMessage()");
  saveLog->SetToolTipText("Save log message into file");
  logFrame->AddFrame(saveLog, new TGLayoutHints(kLHintsLeft | kLHintsCenterY,5,5,3,4));
  
  // log file name
  fLogFile = new TGTextEntry(logFrame,"AliMpDEVisu.log");
  fLogFile->SetToolTipText("Default log file name");
  logFrame->AddFrame(fLogFile, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 10, 0, 9, 0));
  
  // zoom mode
  fZoomButton = new TGCheckButton(logFrame, new TGHotString("&Zoom Mode"), kZoomMode);
  fZoomButton->Associate(this);
  fZoomButton->SetState(kButtonUp);

  logFrame->AddFrame(fZoomButton, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 20, 0, 9, 0));
  
  // frame
  fMain->AddFrame(hframe, new TGLayoutHints(kLHintsLeft | kLHintsCenterY,2,2,10,10));
  fMain->AddFrame(fEcanvas, new TGLayoutHints(kLHintsCenterX,	10,10,10,10));
  
  fMain->AddFrame(logFrame, new TGLayoutHints(kLHintsLeft,2,2,2,10));
  
  // Set a name to the main frame
  fMain->SetWindowName("MUON Detection Element Visualization");
  
  // Map all subwindows of main frame
  fMain->MapSubwindows();
  
  // Initialize the layout algorithm
  fMain->Resize(fMain->GetDefaultSize());
  
  // Map main frame
  fMain->MapWindow();
  
  // instance segmentation
  fkSegmentation = AliMpSegmentation::Instance()->GetMpSegmentation(fCurrentDetElem, detElem->GetCathodType(fCurrentPlane));
  fLogMessage->AddLine("Segmentation loaded");
  fLogMessage->ShowBottom();
}

//____________________________________________________________
AliMpDEVisu::~AliMpDEVisu() 
{
  /// Clean up used widgets: frames, buttons, layouthints
  
  fChamberCombo->Delete();
  fDECombo->Delete();
  fNumberEntry->Delete();
  fPlaneButton->Delete();
  fNameDECombo->Delete();
  fLogMessage->Delete();
  fLogFile->Delete();
  fMain->Cleanup();
  delete fMain;
}

//__________________________________________________________
void AliMpDEVisu::HandleMovement(Int_t eventType, Int_t eventX, Int_t eventY, TObject* /*select*/)
{
  /// handle cursor mouvement
  
  static Int_t eventXold, eventYold;
  static Int_t eventX0, eventY0;
  static Int_t linedrawn;
  

  // enable again  KeyAutoRepeat after HandleKey
  if (eventType == kKeyPress) {
    gVirtualX->SetKeyAutoRepeat(kTRUE);
  }

  fEcanvas->GetCanvas()->cd();

  // zoom mode
  if (fZoomMode) {
    
    if (eventType == kButton1Down) {
      gVirtualX->SetLineWidth(3);
      eventX0   = eventX; eventY0   = eventY;
      eventXold = eventX; eventYold = eventY;
      linedrawn = 0;
    }
    
    if (eventType == kButton1Motion) {
      if (linedrawn) gVirtualX->DrawBox(eventX0, eventY0, eventXold, eventYold, TVirtualX::kHollow);
      eventXold = eventX;
      eventYold = eventY;
      linedrawn = 1;
      gVirtualX->DrawBox(eventX0, eventY0, eventXold, eventYold,  TVirtualX::kHollow);
    }
    
    if (eventType == kButton1Up)
    {
      if ( (eventX0 - eventX) && (eventY0 - eventY) ) 
      {
        PopUpZoom(eventX0,eventY0,eventX,eventY);
        linedrawn=1;
      }
    }
    
    
  } else {
    
    // click mode
    
    if (eventType == kButton1Down) {
      
      // calculated in pixel 
      Double_t x = 0.;
      Double_t y = 0.;
      
      EventToReal(eventX,eventY,x,y);
      
      // get manu
      AliMpPad pad = fkSegmentation->PadByPosition(x,y, false);
      
      if (!pad.IsValid()) {
       
        fLogMessage->AddLine(Form("PopupManuMotif: No manu for DE: %d at position (%5.2f, %5.2f)",
			      fCurrentDetElem, x, y));
        fLogMessage->ShowBottom();
        return;
      }
      
      Int_t manu = pad.GetManuId();
      
      fNumberEntry->SetNumber(manu);
      
     
      fLogMessage->AddLine(Form("PopupManuMotif: DE: %d, manu: %d at position: %5.2f, %5.2f", 
				fCurrentDetElem, manu, x, y));
      fLogMessage->ShowBottom();
      
      DrawManuMotif(true);
      
    }
  }
}

//__________________________________________________________
void AliMpDEVisu::DrawDE(Bool_t info) 
{
  /// Draws function graphics in randomly choosen interval
  
  if (info)
    InfoDE();
  
  fEcanvas->GetCanvas()->cd();
  fEcanvas->GetCanvas()->SetEditable(kTRUE);
  
  fNameDECombo->Select(fDEOccurrence[fCurrentDetElem]);
  TGLBEntry* entry = fNameDECombo->GetSelectedEntry();
  entry->SetBackgroundColor(0xDDDDDD);

  if (AliMpDEManager::GetStationType(fCurrentDetElem) == AliMp::kStation345 ) {
    
    DrawSlat("PMCI");
    
  } else {
    
    DrawQuadrant("RSMCI");
    
  }
  DeletePopUp();
  fEcanvas->GetCanvas()->SetEditable(kFALSE);
  
}

//__________________________________________________________
void AliMpDEVisu::DrawManuMotif(Bool_t popup) 
{
  ///  Draw manu motif in yellow and popup channel motif window
  
  fEcanvas->GetCanvas()->SetEditable(kTRUE);
  
  if (!fNumberEntry->GetIntNumber()) return;
  
  Char_t command[255];
  
  if (AliMpDEManager::GetStationType(fCurrentDetElem) == AliMp::kStation345 ) {
    
    snprintf(command, 255, "%s%d", "PMCI:", (Int_t)fNumberEntry->GetIntNumber());
    
    DrawSlat(command, popup);
    
  } else {   
    
    snprintf(command, 255, "%s%d", "RSMCI:", (Int_t)fNumberEntry->GetIntNumber());
    
    DrawQuadrant(command, popup);
    
  }
  
  fEcanvas->GetCanvas()->SetEditable(kFALSE);
}

//__________________________________________________________
void AliMpDEVisu::DrawSlat(Option_t* option, Bool_t popup) 
{
  /// draw slat segmentation
  
  TCanvas *canvas = fEcanvas->GetCanvas();
  canvas->Clear();
  
  AliMpDetElement* detElem = AliMpDEManager::GetDetElement(fCurrentDetElem);
  TString nameType =  detElem->GetSegType();

  AliMpDataStreams dataStreams;

  AliMpSlatMotifMap mm;
  
  AliMpSt345Reader reader(&mm);
  AliMpSlat* slatCurrent = reader.ReadSlat(dataStreams,nameType.Data(), fCurrentPlane);
  AliMpSlat* slatOther   = reader.ReadSlat(dataStreams,nameType.Data(), AliMp::OtherPlaneType(fCurrentPlane));
  
  AliMpVPainter* painter = AliMpVPainter::CreatePainter(slatCurrent);
  painter->Draw(option);
  
  canvas->Update();
  
  Int_t numberOfManu = 
      slatCurrent->GetNofElectronicCards() + 
      slatOther->GetNofElectronicCards();
    
  fLogMessage->AddLine(Form("DrawSlat: number of manus: %d", numberOfManu));
  fLogMessage->ShowBottom();   
  
  if (popup)
    PopUpManuMotif(slatCurrent);
  
}

//__________________________________________________________
void AliMpDEVisu::DrawQuadrant(Option_t* option, Bool_t popup)
{
  /// draw quadrant segmentation
  
  TCanvas *canvas = fEcanvas->GetCanvas();
  canvas->Clear();
  
  AliMpDetElement* detElem = AliMpDEManager::GetDetElement(fCurrentDetElem);
  AliMq::Station12Type  station = detElem->GetStation12Type();

  AliMpDataStreams dataStreams;
  
  AliMpSectorReader readerCurrent(station, fCurrentPlane);
  AliMpSector* sectorCurrent = readerCurrent.BuildSector(dataStreams);
    
  AliMpSectorReader readerOther(station, AliMp::OtherPlaneType(fCurrentPlane));
  AliMpSector* sectorOther = readerOther.BuildSector(dataStreams);
  
  AliMpVPainter *painter = AliMpVPainter::CreatePainter(sectorCurrent);
  painter->Draw(option);
  
  canvas->Update();
  
  Int_t numberOfManu =  
      sectorCurrent->GetMotifMap()->GetNofMotifPositions() + 
      sectorOther->GetMotifMap()->GetNofMotifPositions();
  
  fLogMessage->AddLine(Form("DrawQuadrant: number of manus: %d", numberOfManu));
  fLogMessage->ShowBottom();   

  if (popup) 
    PopUpManuMotif(sectorCurrent);
 
}

//______________________________________________________________________________
void 
AliMpDEVisu::EventToReal(Int_t eventX, Int_t eventY, 
                         Double_t& x, Double_t& y) const
{
  /// estimate graphic pad sizes

  static Double_t ulx(gPad->XtoPixel(0.01)); 
  static Double_t uly(gPad->YtoPixel(0.99));
  static Double_t brx(gPad->XtoPixel(0.99));
  static Double_t bry(gPad->YtoPixel(0.01));

  static Double_t padDimX = brx - ulx;
  static Double_t padDimY = bry - uly;
  
  // get DE dimension (half size)
  Double_t deDimX(fkSegmentation->GetDimensionX()*2.0);
  Double_t deDimY(fkSegmentation->GetDimensionY()*2.0);
  
  if ( AliMpDEManager::GetStationType(fCurrentDetElem) == AliMp::kStation345 )
  {
    // origin at center
    x = (eventX - ulx - padDimX/2.)/(padDimX)*deDimX; 
    y = (bry - eventY - padDimY/2.)/(padDimY)*deDimY; 
    
  } 
  else 
  {
    // origin at bottom left
    x = (eventX - ulx)/(padDimX)*deDimX; 
    y = (bry - eventY)/(padDimY)*deDimY; 
  }
  
}

//__________________________________________________________
void AliMpDEVisu::ResetManu() 
{
  /// reset manu search entry 
  fLogMessage->AddLine("Reset Motif Search Entry:");
  fLogMessage->ShowBottom();
  fNumberEntry->SetNumber(0);
  
}

//__________________________________________________________
void AliMpDEVisu::DeletePopUp() 
{
/// delete motif popup windows 
  
  if (fTrashList.GetEntries() > 0) 
  {
    fLogMessage->AddLine("Delete Motif PopUp Windows:");
    fLogMessage->ShowBottom();
    
    fTrashList.Delete();
  } 
}

//__________________________________________________________
void AliMpDEVisu::SaveLogMessage() 
{
  /// save log message into log file
  
  TString logFile = fLogFile->GetDisplayText();
  fLogMessage->GetText()->Save(logFile.Data());

  fLogMessage->AddLine(Form("SaveLogMessage: saving log message into logfile: %s", logFile.Data()));
  fLogMessage->ShowBottom();  
}

//__________________________________________________________
void AliMpDEVisu::ClearLogMessage() 
{
  /// clear log message 
  fLogMessage->GetText()->Clear();
  fLogMessage->AddLine("ClearLogMessage: clear log messages:");
  fLogMessage->ShowBottom();  
}

//__________________________________________________________
void AliMpDEVisu::InfoDE() 
{
  /// info concerning the whole DE
  
  AliMpDetElement* detElem = fDDLStore->GetDetElement(fCurrentDetElem);
  Int_t ddlId        = detElem->GetDdlId();
  Int_t numberOfBus  = detElem->GetNofBusPatches();
  
  Int_t vec[24];
  for (Int_t i = 0; i <  detElem->GetNofBusPatches(); ++i)
      vec[i] = detElem->GetBusPatchId(i);

  Int_t firstBus     = TMath::MinElement(detElem->GetNofBusPatches(), vec);
  Int_t lastBus      = TMath::MaxElement(detElem->GetNofBusPatches(), vec);
  
  Int_t numberOfSerialManu = fManuStore->NofManus(detElem->GetId()); // number of manu with an identified serial number
  
  fLogMessage->AddLine(Form("DrawDE: detection element: %d, name: %s", 
		       fCurrentDetElem, fCurrentDEName.Data()));
  fLogMessage->ShowBottom();   
  
  fLogMessage->AddLine(Form("DrawDE: DDL: %d, number of buspatches %d from %d to %d", 
			    ddlId, numberOfBus, firstBus, lastBus));
  fLogMessage->ShowBottom();
  
  if (numberOfSerialManu != 0) { // not available yet for all DE 
    fLogMessage->AddLine(Form("DrawDE: number of manus with serial number: %d", numberOfSerialManu));
    fLogMessage->ShowBottom();
  }
  
}

//__________________________________________________________
Bool_t AliMpDEVisu::ProcessMessage(Long_t msg, Long_t parm1, Long_t /*parm2*/)
{
  /// process message from widgets actions/entries

  AliMpDetElement* detElem = AliMpDEManager::GetDetElement(fCurrentDetElem);

  switch(GET_MSG(msg)) 
  {
    case kC_COMMAND: 
      switch (GET_SUBMSG(msg)) 
      {
        case kCM_COMBOBOX: 
          
          switch (parm1) 
          {
            case kChamberCombo: 
              UpdateComboDE();
              UpdateNameView(kTRUE);
              break;
              
            case kDECombo:
              UpdateNameView();
              break; 
	     
	  case kDEName:
	      UpdateComboCH();
	      break;
          }
          break;
          
        case kCM_CHECKBUTTON:
          switch (parm1) 
          {
            case kPlaneType:
              if (fPlaneButton->GetState() == kButtonDown) {
                fCurrentPlane = AliMp::kNonBendingPlane;
                if (fNumberEntry->GetIntNumber() && fNumberEntry->GetIntNumber() <= 1024)
                  fNumberEntry->SetNumber(fNumberEntry->GetIntNumber() + 1024);
              } else {
                fCurrentPlane = AliMp::kBendingPlane;
                if (fNumberEntry->GetIntNumber() && fNumberEntry->GetIntNumber() > 1024)
                  fNumberEntry->SetNumber(fNumberEntry->GetIntNumber() - 1024);
              }
              DrawDE();
              fkSegmentation = AliMpSegmentation::Instance()
                ->GetMpSegmentation(fCurrentDetElem, detElem->GetCathodType(fCurrentPlane));
              break;

            case kZoomMode:
		if (fZoomButton->GetState() == kButtonDown)
		    fZoomMode = true;
		else
		    fZoomMode = false;
              break;
          }
          break;
          
      }
      break;
  }
  return true;
}

//__________________________________________________________
void AliMpDEVisu::NextDE()
{
  /// select next DE
  
  Int_t next = fDECombo->GetSelected() + 1;
  
  if (next < fDECombo->GetNumberOfEntries())
    fDECombo->Select(next);
  else 
    fDECombo->Select(0);
  
  UpdateNameView();
  DrawDE();
}

//__________________________________________________________
void AliMpDEVisu::UpdateComboCH()
{
  /// update Chamber/DE in respect to DE Name

    fCurrentDEName = fNameDEComboIdx[fNameDECombo->GetSelected()];

    AliMpDetElement* detElem = AliMpDEManager::GetDetElement(fCurrentDEName);

    Int_t idx =  AliMpDEManager::GetChamberId(detElem->GetId());
    fChamberCombo->Select(idx);

    UpdateComboDE();

    idx = detElem->GetId() % 100;
    fDECombo->Select(idx);

    fCurrentDetElem = fDEComboIdx[fDECombo->GetSelected()];

    fkSegmentation = AliMpSegmentation::Instance()
	->GetMpSegmentation(fCurrentDetElem, detElem->GetCathodType(fCurrentPlane));
}

//__________________________________________________________
void AliMpDEVisu::UpdateComboDE()
{
  /// update DE in respect to selected chamber
  
  fDECombo->RemoveAll();
  
  AliMpDEIterator it;
  Int_t i = 0;
  fDEComboIdx.Reset();
  
  for ( it.First(fChamberCombo->GetSelected()); ! it.IsDone(); it.Next() ) {
    fDECombo->AddEntry(Form("%d",it.CurrentDE()->GetId()), i);
    fDEComboIdx[i++] = it.CurrentDE()->GetId();
  }
  fDECombo->Select(0);
}

//__________________________________________________________
void AliMpDEVisu::UpdateNameView(Bool_t firstTime)
{
  /// update DE name in respect to selected DE id.
    
  fCurrentDetElem = fDEComboIdx[fDECombo->GetSelected()];

  AliMpDetElement* detElem = AliMpDEManager::GetDetElement(fCurrentDetElem);

  fCurrentDEName = detElem->GetDEName();

  if (firstTime) {
    AliMpDEIterator it;
 
    fNameDECombo->RemoveAll();
  
    for ( it.First(fChamberCombo->GetSelected()); !it.IsDone(); it.Next() ) {

      Int_t detElemId = it.CurrentDE()->GetId();
      TString deName  = it.CurrentDE()->GetDEName();

      Int_t idx = fNameDECombo->GetNumberOfEntries();
      fNameDECombo->AddEntry(deName.Data(), idx);
      fNameDEComboIdx[idx]   = deName;
      fDEOccurrence[detElemId] = idx;
    }
  }

  fNameDECombo->Select(fDEOccurrence[fCurrentDetElem]);
  fkSegmentation = AliMpSegmentation::Instance()
    ->GetMpSegmentation(fCurrentDetElem, detElem->GetCathodType(fCurrentPlane));
}

//______________________________________________________________________________
void
AliMpDEVisu::CreatePopupWindow(Int_t w, Int_t h, const char* windowName,
                               AliMpVPainter* painter,
                               const char* option)
{
  /// Create transient frame
  
  TCanvas* c = new TCanvas(windowName,windowName,-200,100,w,h);

  Int_t n = fTrashList.GetEntries();
  
  c->Connect("Closed()","AliMpDEVisu",this,Form("ClosePopupWindow(=%d)",n));
             
  fTrashList.AddLast(c);
  
  c->Draw();
  
  painter->Draw(option);
  
  c->Update();
}

//__________________________________________________________
void AliMpDEVisu::PopUpManuMotif(AliMpSlat* slat)
{
  /// pop up manu window motif painter for slat
  
  // motif painter
  AliMpMotifPosition* motifPosFound = 0x0;
  AliMpMotifPosition* motifPos = 0x0;
  Int_t manuId = 0;

  for ( Int_t i = 0; i < slat->GetSize(); ++i ) {
    
    AliMpPCB* pcb = slat->GetPCB(i);
    
    for ( Int_t j = 0; j < slat->GetPCB(i)->GetSize(); ++j ) {
      
      motifPos = pcb->GetMotifPosition(j);
      
      manuId = motifPos->GetID();
      if (manuId == (Int_t)fNumberEntry->GetIntNumber()) {
        motifPosFound = motifPos;
        break;
      }
    }
    if (motifPosFound)
      break;
  }  

  if(motifPosFound) 
  {
    InfoManuMotif(motifPosFound);

    Double_t dimensionX(motifPosFound->GetDimensionX()); 
    Double_t dimensionY(motifPosFound->GetDimensionY());

    Int_t h = 500;
    Int_t w = Int_t(h*dimensionX/dimensionY);
    AliMpVPainter* painter = AliMpVPainter::CreatePainter(motifPosFound);
      
    CreatePopupWindow(w,h,Form("Manu %ld",fNumberEntry->GetIntNumber()),
		      painter,"ZT");
  }
}

//__________________________________________________________
void AliMpDEVisu::PopUpManuMotif(AliMpSector* sector)
{
  /// pop up manu window motif painter for sector
    
  // motif painter
  AliMpMotifPosition* motifPosFound = 0x0;
  AliMpMotifPosition* motifPos = 0x0;
  Int_t manuId = 0;

  for (Int_t iRow = 0; iRow < sector->GetNofRows(); ++iRow) {
    
    AliMpRow* row = sector->GetRow(iRow);
    
    for (Int_t iRowSeg = 0; iRowSeg < sector->GetRow(iRow)->GetNofRowSegments(); ++iRowSeg){
      
      for (Int_t iRowSeg2 = 0; iRowSeg2 < row->GetNofRowSegments(); ++iRowSeg2) {
        AliMpVRowSegment *rowSegment = row->GetRowSegment(iRowSeg2);
        
        for (Int_t iMotif = 0; iMotif < rowSegment->GetNofMotifs(); ++iMotif){
          
          Int_t motifPositionId = rowSegment->GetMotifPositionId(iMotif);
          motifPos = rowSegment->GetRow()
            ->GetMotifMap()->FindMotifPosition(motifPositionId);
          
          manuId = motifPos->GetID();
          if (manuId == (Int_t)fNumberEntry->GetIntNumber()) {
            motifPosFound = motifPos;
            break;
          }
        }
        if (motifPosFound)
          break;
      }
    }
    if (motifPosFound)
      break;
  }
  
  if(motifPosFound) 
  {    

    InfoManuMotif(motifPosFound);

    Double_t dimensionX(motifPosFound->GetDimensionX()); 
    Double_t dimensionY(motifPosFound->GetDimensionY());

    // Create transient frame
    Int_t h = 400;
    Int_t w = Int_t(h*dimensionX/dimensionY);

    AliMpVPainter* painter = AliMpVPainter::CreatePainter(motifPosFound);
    
    CreatePopupWindow(w,h,Form("Manu %ld",fNumberEntry->GetIntNumber()),
                      painter,"ZT");
    
  }
}

//__________________________________________________________
void AliMpDEVisu::InfoManuMotif(AliMpMotifPosition* motifPos)
{
/// info for popup manu motif

   // log message
    Int_t manuId = fNumberEntry->GetIntNumber();

    fLogMessage->AddLine(Form("PopupManuMotif: motif dimension: %5.2f, %5.2f", 
			      motifPos->GetDimensionX()*2., 
                              motifPos->GetDimensionY()*2.));
      
    fLogMessage->AddLine( Form("PopupManuMotif: pad dimension: %4.2f, %4.2f", 
			       motifPos->GetMotif()->GetPadDimensionX(0)*2.,  
			       motifPos->GetMotif()->GetPadDimensionY(0)*2. ));

    fLogMessage->AddLine( Form("PopupManuMotif: manu: %d, serial number: %d, buspatch: %d",
                             manuId, 
                             fManuStore->GetManuSerial(fCurrentDetElem, manuId),
                             fDDLStore->GetBusPatchId(fCurrentDetElem, manuId)) );

    fLogMessage->ShowBottom();

}

//______________________________________________________________________________
void AliMpDEVisu::PopUpZoom(Int_t ix0, Int_t iy0, Int_t ix1, Int_t iy1)
{
  /// popup zoom window

  Double_t x0,y0;
  EventToReal(ix0,iy0,x0,y0);
  Double_t x1,y1;
  EventToReal(ix1,iy1,x1,y1);

  fLogMessage->AddLine(Form("PopUpZoom: zoom positions (x0, y0: %6.2f, %6.2f), (x1, y1: %6.2f, %6.2f)",
                             x0, y0, x1, y1));
  fLogMessage->ShowBottom();
  
  // Create transient frame
  Int_t h = 500;//TMath::Abs(x1-x0);
  Int_t w = 500;//TMath::Abs(y1-y0);

  AliMpArea area((x0+x1)/2.0,(y0+y1)/2.0,
                 TMath::Abs(x1-x0)/2.0,TMath::Abs(y1-y0)/2.0);
//  area.Print();
  
  if ( area.IsValid() )
  {
    AliMpVPadIterator* iterator = fkSegmentation->CreateIterator(area);
    if (iterator)
    {
      AliMpVPainter* painter = AliMpVPainter::CreatePainter(iterator);
      CreatePopupWindow(w,h,"Zoom",painter,"");
    }
    else
    {
      AliError("could not get iterator for that area");
    }
        
    delete iterator;
  }
}

//__________________________________________________________
void AliMpDEVisu::ClosePopupWindow(Int_t id)
{
  /// close signal
  fEcanvas->GetCanvas()->cd();
  fTrashList.RemoveAt(id);
}
