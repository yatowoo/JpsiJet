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

  fHistEventStat = new TH1D("hEventStats","Number of events by status",int(kEventStatusN),-0.5,float(kEventStatusN)-0.5);
  fHistosQA->Add(fHistEventStat);

  fHistTrigger = new TH1D("hEventTrigger","Number of event by offline triggers",32,-0.5,31.5);
  fHistosQA->Add(fHistTrigger);

  fHistTriggerClass = new TH1D("hEventTriggerClass","Number of event by fired trigger class",10,-0.5,9.5);
  fHistosQA->Add(fHistTriggerClass);

  PostData(1, fHistosQA);
}

void AliAnalysisTaskJpsiJet::UserExec(Option_t*){
  AliAODEvent *aod = dynamic_cast<AliAODEvent*>(InputEvent());

  fHistEventStat->Fill(kAllInAOD);

  // Offline Trigger
  AliAODHeader* header = dynamic_cast<AliAODHeader*>(aod->GetHeader());
  UInt_t offlineTrigger = header->GetOfflineTrigger();
  for(Short_t i = 0; i < 32; i++){
    if(offlineTrigger & BIT(i)) fHistTrigger->Fill(i);
  }
  // Trigger Classes
  TString triggerClass = aod->GetFiredTriggerClasses();
  TObjArray* tcArray = triggerClass.Tokenize(" ");
  for(Short_t i = 0; i < tcArray->GetEntries(); i++){
    TString strClass = tcArray->At(i)->GetName();
    TObjArray* tmp = strClass.Tokenize("-");
    strClass = tmp->At(0)->GetName();
    fHistTriggerClass->Fill(strClass.Data(), 1.);
    tmp->SetOwner(kTRUE);
    delete tmp;
  }
  tcArray->SetOwner(kTRUE);
  delete tcArray;

  PostData(1, fHistosQA);
}
