/**************************************************************************
 * Copyright(c) 1998-2019, ALICE Experiment at CERN, All rights reserved. *
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

#include "TChain.h"

#include "AliAODEvent.h"

#include "AliAnalysisTaskJpsiJet.h"

class AliAnalysisTaskJpsiJet;

using namespace std;

ClassImp(AliAnalysisTaskJpsiJet)

AliAnalysisTaskJpsiJet::AliAnalysisTaskJpsiJet():
  AliAnalysisTaskSE(),
  fAOD(NULL),
  fSelectedTrigger(0),
  fSelectedTriggerClasses(""),
  fFiredTriggerTag(""),
  fRejectPileup(kFALSE),
  fIsPileup(kFALSE),
  fEventFilter(NULL),
  fHistEventStat(NULL),
  fHistTrigger(NULL),
  fHistTriggerClass(NULL)
{
  // Constructor
}

AliAnalysisTaskJpsiJet::AliAnalysisTaskJpsiJet(const char* taskName):
  AliAnalysisTaskSE(taskName),
  fAOD(NULL),
  fSelectedTrigger(0),
  fSelectedTriggerClasses(""),
  fFiredTriggerTag(""),
  fRejectPileup(kFALSE),
  fIsPileup(kFALSE),
  fEventFilter(NULL),
  fHistEventStat(NULL),
  fHistTrigger(NULL),
  fHistTriggerClass(NULL)
{
  // IO
  DefineInput(0, TChain::Class());
  DefineOutput(1, TList::Class());
}

AliAnalysisTaskJpsiJet::~AliAnalysisTaskJpsiJet(){
  // Destructor
  if(fEventFilter) delete fEventFilter;
}

void AliAnalysisTaskJpsiJet::UserCreateOutputObjects(){
  fHistosQA = new TList();
  fHistosQA->SetOwner(kTRUE);

  fHistEventStat = new TH1D("EventStats","Event statistics;Status;N_{event}",int(kEventStatusN),-0.5,float(kEventStatusN)-0.5);
  fHistEventStat->GetXaxis()->SetBinLabel(kAllInAOD + 1, "Before PS");
  fHistEventStat->GetXaxis()->SetBinLabel(kPhysSelected + 1, "After PS");
  fHistEventStat->GetXaxis()->SetBinLabel(kV0ANDtrigger + 1, "V0AND Trig. ");
  fHistEventStat->GetXaxis()->SetBinLabel(kTRDtrigger + 1, "TRD Trig.");
  fHistEventStat->GetXaxis()->SetBinLabel(kFiltered + 1, "After cuts");
  fHistEventStat->GetXaxis()->SetBinLabel(kAfterPileUp + 1, "Pileup Rejected");
  fHistEventStat->GetXaxis()->SetBinLabel(kWithSinglePair + 1, "N_{pair}==1");
  fHistEventStat->GetXaxis()->SetBinLabel(kWithMultiPair + 1, "N_{pair}>1");
  fHistEventStat->GetXaxis()->SetBinLabel(kWithPairInJet + 1, "e^{+}e^{-} in jet");
  fHistosQA->Add(fHistEventStat);

  InitHistogramsForEventQA("Event_ALL");
  InitHistogramsForEventQA("Event_beforeCuts");

  PostData(1, fHistosQA);
}

void AliAnalysisTaskJpsiJet::UserExec(Option_t*){
  fAOD = dynamic_cast<AliAODEvent*>(InputEvent());

  fHistEventStat->Fill(kAllInAOD);

  FillHistogramsForEventQA("Event_ALL");
/*
 *  Event Physics Selection  
**/
  UInt_t isSelected = AliVEvent::kAny;
  // Select trigger
  AliAODHeader* header = dynamic_cast<AliAODHeader*>(fAOD->GetHeader());
  UInt_t offlineTrigger = header->GetOfflineTrigger();
  isSelected &= (fSelectedTrigger & offlineTrigger);
  // Select trigger classes
  TString triggerClass = fAOD->GetFiredTriggerClasses();
  Bool_t isFired = kFALSE;
  if(fSelectedTriggerClasses != ""){
    TObjArray* selectedTrigClasses = fSelectedTriggerClasses.Tokenize(";");
    selectedTrigClasses->SetOwner(kTRUE);
    for(int i = 0; i < selectedTrigClasses->GetEntries(); i++){
      TString tag = selectedTrigClasses->At(i)->GetName();
      if(triggerClass.Contains(tag)){
        isFired = kTRUE;
        fFiredTriggerTag += tag + "_";
      }
    }
    if(fFiredTriggerTag != "")
      fFiredTriggerTag.Remove(fFiredTriggerTag.Length()-1);
  }
  if(!isSelected || !isFired) return;

  fHistEventStat->Fill(kPhysSelected);

  FillHistogramsForEventQA("Event_beforeCuts");

}

// Create event QA histograms in output list
void AliAnalysisTaskJpsiJet::InitHistogramsForEventQA(const char* histClass){
  // Check existence
  if(!fHistosQA || fHistosQA->FindObject(histClass)){
    AliWarning(Form("Histograms for QA : %s existed.", histClass));
    return;
  }

  // Init histograms
  TList* eventQA = new TList();
  eventQA->SetName(histClass);
  eventQA->SetOwner(kTRUE);
  fHistosQA->Add(eventQA);

  TH1* hTrigger = new TH1D("Trigger","Number of event by offline triggers;Nbits in AliVEvent;N_{events}",32,-0.5,31.5);
  const char* labelOfflineTrigger[32] = {
    "MB/INT1", "INT7", "MUON", "HM", "EMC1", "INT5", "MUS", "MUSH7",
    "MUL7", "MUU7", "EMC7/8", "MUS7", "PHI1", "PHI7/8", "EMCEJE", "EMCEGA",
    "HMV0/CENT", "SemiCENT", "DG/5", "ZED", "SPI/7", "INT8", "MUSL8", "MUSH8",
    "MULL8", "MUUL8", "MUUL0", "UserDef", "TRD", "MUCalo", "FastOnly", ""};
  for(int i = 0; i < 32; i++)
    hTrigger->GetXaxis()->SetBinLabel(i+1, labelOfflineTrigger[i]);
  eventQA->Add(hTrigger);

  TH1* hTriggerClass = new TH1D("TriggerClass","Number of event by fired trigger class;Trig. Descriptor;N_{events}",10,-0.5,9.5);
  eventQA->Add(hTriggerClass);

}

// Locate histograms by names
TH1* AliAnalysisTaskJpsiJet::GetHist(const char* histClass, const char* histName){
  // Check existence
  if(!fHistosQA) return NULL;
  if(!fHistosQA->FindObject(histClass)) return NULL;
  return (TH1*)(fHistosQA->FindObject(histClass)->FindObject(histName));
}

void AliAnalysisTaskJpsiJet::FillHist(const char* histClass, const char* histName, Double_t value, Double_t weight){
  TH1* hist = GetHist(histClass, histName);
  if(!hist){
    AliWarning(Form("QA histograms : Can not find histogram %s/%s", histClass, histName));
    return;
  }
  hist->Fill(value, weight);
}

// Fill string/label directly
void AliAnalysisTaskJpsiJet::FillHist(const char* histClass, const char* histName, const char* value){
  TH1* hist = GetHist(histClass, histName);
  if(!hist){
    AliWarning(Form("QA histograms : Can not find histogram %s/%s", histClass, histName));
    return;
  }
  hist->Fill(value, 1.);
}

void AliAnalysisTaskJpsiJet::FillHistogramsForEventQA(const char* histClass){
  // Offline Trigger
  AliAODHeader* header = dynamic_cast<AliAODHeader*>(fAOD->GetHeader());
  UInt_t offlineTrigger = header->GetOfflineTrigger();
  for(Short_t i = 0; i < 32; i++){
    if(offlineTrigger & BIT(i)) FillHist(histClass, "Trigger", i);
  }

  // Trigger Classes
  TString triggerClass = fAOD->GetFiredTriggerClasses();
  TObjArray* tcArray = triggerClass.Tokenize(" ");
  for(Short_t i = 0; i < tcArray->GetEntries(); i++){
    TString strClass = tcArray->At(i)->GetName();
    TObjArray* tmp = strClass.Tokenize("-");
    strClass = tmp->At(0)->GetName();
    FillHist(histClass, "TriggerClass", strClass.Data());
    tmp->SetOwner(kTRUE);
    delete tmp;
  }
  tcArray->SetOwner(kTRUE);
  delete tcArray;
}