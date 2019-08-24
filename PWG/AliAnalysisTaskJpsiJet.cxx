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
#include "THnSparse.h"

#include "AliAODEvent.h"
#include "AliAnalysisManager.h"

#include "AliAnalysisTaskJpsiJet.h"

class AliAnalysisTaskJpsiJet;

using namespace std;

ClassImp(AliAnalysisTaskJpsiJet)

AliAnalysisTaskJpsiJet::AliAnalysisTaskJpsiJet():
  AliAnalysisTaskSE(),
  fAOD(NULL),
  fJetTasks(NULL),
  fJetContainers(NULL),
  fSelectedTrigger(0),
  fSelectedTriggerClasses(""),
  fFiredTriggerTag(""),
  fRejectPileup(kFALSE),
  fIsPileup(kFALSE),
  fEventFilter(NULL),
  fHistos(NULL)
{
  // Constructor
}

AliAnalysisTaskJpsiJet::AliAnalysisTaskJpsiJet(const char* taskName):
  AliAnalysisTaskSE(taskName),
  fAOD(NULL),
  fJetTasks(NULL),
  fJetContainers(NULL),
  fSelectedTrigger(0),
  fSelectedTriggerClasses(""),
  fFiredTriggerTag(""),
  fRejectPileup(kFALSE),
  fIsPileup(kFALSE),
  fEventFilter(NULL),
  fHistos(NULL)
{
  // IO
  DefineInput(0, TChain::Class());
  DefineOutput(1, TList::Class());
  // Jet task
  InitJetFinders();
}

AliAnalysisTaskJpsiJet::~AliAnalysisTaskJpsiJet(){
  // Destructor
  if(fEventFilter) delete fEventFilter;
  // Histogram list from AliAnalysisTaskSE
  if(fHistosQA) delete fHistosQA;
  if(fHistos) delete fHistos;
}

void AliAnalysisTaskJpsiJet::UserCreateOutputObjects(){
  fHistos = new THistManager("JpsiJetQA");
  fHistosQA = fHistos->GetListOfHistograms();

  TH1* fHistEventStat = fHistos->CreateTH1("EventStats","Event statistics;Status;N_{event}",int(kEventStatusN),-0.5,float(kEventStatusN)-0.5);
  fHistEventStat->GetXaxis()->SetBinLabel(kAllInAOD + 1, "Before PS");
  fHistEventStat->GetXaxis()->SetBinLabel(kPhysSelected + 1, "After PS");
  fHistEventStat->GetXaxis()->SetBinLabel(kV0ANDtrigger + 1, "V0AND Trig. ");
  fHistEventStat->GetXaxis()->SetBinLabel(kTRDtrigger + 1, "TRD Trig.");
  fHistEventStat->GetXaxis()->SetBinLabel(kFiltered + 1, "After cuts");
  fHistEventStat->GetXaxis()->SetBinLabel(kAfterPileUp + 1, "Pileup Rejected");
  fHistEventStat->GetXaxis()->SetBinLabel(kWithSinglePair + 1, "N_{pair}==1");
  fHistEventStat->GetXaxis()->SetBinLabel(kWithMultiPair + 1, "N_{pair}>1");
  fHistEventStat->GetXaxis()->SetBinLabel(kWithPairInJet + 1, "e^{+}e^{-} in jet");

  InitHistogramsForEventQA("Event_ALL");
  InitHistogramsForEventQA("Event_beforeCuts");
  InitHistogramsForEventQA("Event_afterCuts");

  // Init jet finder tasks
  AliEmcalJetTask* jetFinder = NULL;
  TIter next(fJetTasks);
  while((jetFinder=(AliEmcalJetTask*)next()))
    jetFinder->CreateOutputObjects();
  InitHistogramsForJetQA("Jet");

  PostData(1, fHistosQA);
}

void AliAnalysisTaskJpsiJet::UserExec(Option_t*){
  fAOD = dynamic_cast<AliAODEvent*>(InputEvent());

  fHistos->FillTH1("EventStats", kAllInAOD);

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

  fHistos->FillTH1("EventStats", kPhysSelected);

  FillHistogramsForEventQA("Event_beforeCuts");

  // Event cuts
  if(fEventFilter && !fEventFilter->IsSelected(fAOD)) return;
  fHistos->FillTH1("EventStats", kFiltered);
  // Pileup
  fIsPileup = fAOD->IsPileupFromSPD(3, 0.8, 3., 2., 5.);
  if(fRejectPileup && fIsPileup) return;
  fHistos->FillTH1("EventStats", kAfterPileUp);

  FillHistogramsForEventQA("Event_afterCuts");

  // Run jet finder tasks
  AliEmcalJetTask* jetFinder = NULL;
  TIter next(fJetTasks);
  while((jetFinder=(AliEmcalJetTask*)next())){
    jetFinder->Reset();
    jetFinder->SetActive(kTRUE);
    jetFinder->Exec("");
  }
  FillHistogramsForJetQA("Jet");
}

// Create event QA histograms in output list
void AliAnalysisTaskJpsiJet::InitHistogramsForEventQA(const char* histClass){

  TH1* hTrigger = fHistos->CreateTH1(Form("%s/Trigger", histClass),"Number of event by offline triggers;Nbits in AliVEvent;N_{events}",32,-0.5,31.5);
  const char* labelOfflineTrigger[32] = {
    "MB/INT1", "INT7", "MUON", "HM", "EMC1", "INT5", "MUS", "MUSH7",
    "MUL7", "MUU7", "EMC7/8", "MUS7", "PHI1", "PHI7/8", "EMCEJE", "EMCEGA",
    "HMV0/CENT", "SemiCENT", "DG/5", "ZED", "SPI/7", "INT8", "MUSL8", "MUSH8",
    "MULL8", "MUUL8", "MUUL0", "UserDef", "TRD", "MUCalo", "FastOnly", ""};
  for(int i = 0; i < 32; i++)
    hTrigger->GetXaxis()->SetBinLabel(i+1, labelOfflineTrigger[i]);

  TH1* hTriggerClass = fHistos->CreateTH1(Form("%s/TriggerClass", histClass),"Number of event by fired trigger class;Trig. Descriptor;N_{events}",10,-0.5,9.5);

}

void AliAnalysisTaskJpsiJet::FillHistogramsForEventQA(const char* histClass){
  // Offline Trigger
  AliAODHeader* header = dynamic_cast<AliAODHeader*>(fAOD->GetHeader());
  UInt_t offlineTrigger = header->GetOfflineTrigger();
  for(Short_t i = 0; i < 32; i++){
    if(offlineTrigger & BIT(i)) fHistos->FillTH1(Form("%s/Trigger", histClass), i);
  }

  // Trigger Classes
  TString triggerClass = fAOD->GetFiredTriggerClasses();
  TObjArray* tcArray = triggerClass.Tokenize(" ");
  for(Short_t i = 0; i < tcArray->GetEntries(); i++){
    TString strClass = tcArray->At(i)->GetName();
    TObjArray* tmp = strClass.Tokenize("-");
    strClass = tmp->At(0)->GetName();
    fHistos->FillTH1(Form("%s/TriggerClass", histClass), strClass.Data());
    tmp->SetOwner(kTRUE);
    delete tmp;
  }
  tcArray->SetOwner(kTRUE);
  delete tcArray;
}

// Copy from AliEmcalJetTask::AddTaskEmcalJet
void AliAnalysisTaskJpsiJet::AddTaskEmcalJet(
  const TString nTracks, const TString nClusters,
  const AliJetContainer::EJetAlgo_t jetAlgo, const Double_t radius, const AliJetContainer::EJetType_t jetType,
  const Double_t minTrPt, const Double_t minClPt,
  const Double_t ghostArea, const AliJetContainer::ERecoScheme_t reco,
  const TString tag, const Double_t minJetPt,
  const Bool_t lockTask, const Bool_t bFillGhosts
){
  // Setup containers
  TString trackName(nTracks);
  TString clusName(nClusters);
  if (trackName == "usedefault") {
    trackName = "tracks";
  }
  if (clusName == "usedefault") {
      clusName = "caloClusters";
  }

  AliParticleContainer* partCont = 0;
  if (trackName.Contains("mcparticles")) {  // must be contains in order to allow for non-standard particle containers
    AliMCParticleContainer* mcpartCont = new AliMCParticleContainer(trackName);
    partCont = mcpartCont;
  }
  else if (trackName == "tracks" || trackName == "Tracks") {
    AliTrackContainer* trackCont = new AliTrackContainer(trackName);
    partCont = trackCont;
  }
  else if (!trackName.IsNull()) {
    partCont = new AliParticleContainer(trackName);
  }
  if (partCont) partCont->SetParticlePtCut(minTrPt);

  AliClusterContainer* clusCont = 0;
  if (!clusName.IsNull()) {
    clusCont = new AliClusterContainer(clusName);
    clusCont->SetClusECut(0.);
    clusCont->SetClusPtCut(0.);
    clusCont->SetClusHadCorrEnergyCut(minClPt);
    clusCont->SetDefaultClusterEnergy(AliVCluster::kHadCorr);
  }

  switch (jetType) {
  case AliJetContainer::kChargedJet:
    if (partCont) partCont->SetCharge(AliParticleContainer::kCharged);
    break;
  case AliJetContainer::kNeutralJet:
    if (partCont) partCont->SetCharge(AliParticleContainer::kNeutral);
    break;
  default:
    break;
  }
  
  TString name = AliJetContainer::GenerateJetName(jetType, jetAlgo, reco, radius, partCont, clusCont, tag);
  ;
  if(fJetTasks->FindObject(name.Data())){
    AliWarning(Form("Jet task existed: %s", name.Data()));
    return;
  }
  AliInfo(Form("Jet task name : %s", name.Data()));

  AliEmcalJetTask* jetTask = new AliEmcalJetTask(name);
  fJetTasks->Add(jetTask);

  jetTask->SetJetType(jetType);
  jetTask->SetJetAlgo(jetAlgo);
  jetTask->SetRecombScheme(reco);
  jetTask->SetRadius(radius);
  if (partCont) jetTask->AdoptParticleContainer(partCont);
  if (clusCont) jetTask->AdoptClusterContainer(clusCont);
  jetTask->SetJetsName(tag);
  jetTask->SetMinJetPt(minJetPt);
  jetTask->SetGhostArea(ghostArea);

  if (bFillGhosts) jetTask->SetFillGhost();
  if (lockTask) jetTask->SetLocked();

  jetTask->SetForceBeamType(AliAnalysisTaskEmcal::kpp);
  jetTask->SelectCollisionCandidates(AliVEvent::kAny);
  jetTask->SetUseAliAnaUtils(kTRUE);
  jetTask->SetZvertexDiffValue(0.5);
  jetTask->SetNeedEmcalGeom(kFALSE);

  // Connect input
  AliAnalysisManager *mgr = AliAnalysisManager::GetAnalysisManager();
  jetTask->ConnectInput(0, mgr->GetCommonInputContainer());

  // Add jet container
  AliJetContainer *cont = new AliJetContainer(jetType, jetAlgo, reco, radius, partCont, clusCont, tag);
  cont->SetPercAreaCut(0.6);
  if(jetType == AliJetContainer::kChargedJet)
    cont->SetJetAcceptanceType(AliJetContainer::kTPCfid);
  // else for neutral jet, EMCal/DCal/PHOS should be considered
  fJetContainers->Add(cont);
}

void AliAnalysisTaskJpsiJet::InitJetFinders(){
  if(!fJetTasks) fJetTasks = new TObjArray;
  fJetTasks->SetOwner(kTRUE);
  if(!fJetContainers) fJetContainers = new TObjArray;
  fJetContainers->SetOwner(kTRUE);

  AddTaskEmcalJet("usedefault", "", AliJetContainer::antikt_algorithm, 0.2, AliJetContainer::kChargedJet, 0.15, 0.3, 0.01, AliJetContainer::pt_scheme, "Jet", 1., kFALSE, kFALSE);
  AddTaskEmcalJet("usedefault", "", AliJetContainer::antikt_algorithm, 0.4, AliJetContainer::kChargedJet, 0.15, 0.3, 0.01, AliJetContainer::pt_scheme, "Jet", 1., kFALSE, kFALSE);
}

void AliAnalysisTaskJpsiJet::InitHistogramsForJetQA(const char* histClass){

  AliJetContainer* jets = NULL;
  TIter next(fJetContainers);
  while((jets = static_cast<AliJetContainer*>(next()))){
    TString histGroup = Form("%s/%s", histClass, jets->GetName());
    // THnSparse - pT, eta， phi
    Int_t nBins[3]   = {2000, 200, 100};
    Double_t xmin[3] = {0.,   -1., -2.};
    Double_t xmax[3] = {100.,  1.,  8.};
    THnSparse* hs = fHistos->CreateTHnSparse(Form("%s/jetPtEtaPhi", histGroup.Data()), "THnSparse for jet kinetic variables (p_{T}-#eta-#phi)", 3, nBins, xmin, xmax);
  }
}

void AliAnalysisTaskJpsiJet::FillHistogramsForJetQA(const char* histClass){
  if(!fJetContainers) InitJetFinders();
  
  AliJetContainer* jets = NULL;
  TIter next(fJetContainers);
  while((jets = static_cast<AliJetContainer*>(next()))){
    jets->NextEvent(fAOD);
    jets->SetArray(fAOD);
    
    TString histGroup = Form("%s/%s", histClass, jets->GetName());

    for(auto jet : jets->all()){
      // Jet cuts
      UInt_t rejectionReason = 0;
      if (!jets->AcceptJet(jet, rejectionReason)) {
        AliDebug(2,Form("Jet was rejected for reason : %d", rejectionReason));
        return;
      }
      Double_t x[3] = {0.};
      x[0] = jet->Pt();
      x[1] = jet->Eta();
      x[2] = jet->Phi();
      fHistos->FillTHnSparse(Form("%s/jetPtEtaPhi", histGroup.Data()),x,1.0);
    }
  }
}

void AliAnalysisTaskJpsiJet::LocalInit(){
  AliEmcalJetTask* jetFinder = NULL;
  TIter next(fJetTasks);
  while((jetFinder=(AliEmcalJetTask*)next()))
    jetFinder->LocalInit();
}

void AliAnalysisTaskJpsiJet::Terminate(Option_t*){
  AliEmcalJetTask* jetFinder = NULL;
  TIter next(fJetTasks);
  while((jetFinder=(AliEmcalJetTask*)next()))
    jetFinder->Terminate();
}