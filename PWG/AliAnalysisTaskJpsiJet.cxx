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
#include "AliDielectronVarCuts.h"
#include "AliDielectronTrackCuts.h"
#include "AliDielectronPairLegCuts.h"

#include "AliAnalysisTaskJpsiJet.h"

class AliDielectronVarCuts;
class AliDielectronTrackCuts;
class AliDielectronPairLegCuts;
class AliAnalysisTaskJpsiJet;

using namespace std;

ClassImp(AliAnalysisTaskJpsiJet)

AliAnalysisTaskJpsiJet::AliAnalysisTaskJpsiJet():
  AliAnalysisTaskSE(),
  fAOD(NULL),
  fDielectron(NULL),
  fPairs(NULL),
  fDaughters(NULL),
  fTracksWithPair(NULL),
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
  fDielectron(NULL),
  fPairs(NULL),
  fDaughters(NULL),
  fTracksWithPair(NULL),
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
  // Dielectron task
  InitDielectron();
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

  // Init dielectron
  fDielectron->SetDontClearArrays();
  fDielectron->Init();
  InitHistogramsForDielectron();
  fHistosQA->Add(const_cast<THashList*>(fDielectron->GetHistogramList()));

  // Init jet finder tasks
  AliEmcalJetTask* jetFinder = NULL;
  TIter next(fJetTasks);
  while((jetFinder=(AliEmcalJetTask*)next()))
    jetFinder->CreateOutputObjects();
  InitHistogramsForJetQA("Jet");

  // Init pair in jet analysis
  InitHistogramsForTaggedJet();

  PostData(1, fHistosQA);
}

void AliAnalysisTaskJpsiJet::UserExec(Option_t*){
  fAOD = dynamic_cast<AliAODEvent*>(InputEvent());
  fFiredTriggerTag = "";

  // PID response
  AliAnalysisManager *mgr = AliAnalysisManager::GetAnalysisManager();
  AliInputEventHandler *handler = (AliInputEventHandler*)(mgr->GetInputEventHandler());
  if(handler->GetPIDResponse()){
    AliDielectronVarManager::SetPIDResponse(handler->GetPIDResponse());
  }
  else{
    AliFatal("This task needs the PID response attached to the input event handler!");
    return;
  }

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
  if(!isSelected) return;
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
  if(!isFired) return;

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

  // Run dielectron task
  AliKFParticle::SetField(fAOD->GetMagneticField());
  AliDielectronPID::SetCorrVal(fAOD->GetRunNumber());
  AliDielectronHistos *dieHistos = fDielectron->GetHistoManager();
  Double_t values[AliDielectronVarManager::kNMaxValues] = {0};
  AliDielectronVarManager::SetFillMap(dieHistos->GetUsedVars());
  AliDielectronVarManager::SetEvent(fAOD);
  AliDielectronVarManager::Fill(fAOD, values);
  fDielectron->Process(fAOD);

  // Build tracks with dielectron pair
  Int_t nCandidates = fDielectron->GetPairArray(1)->GetEntriesFast();
  if(nCandidates == 1)
    fHistos->FillTH1("EventStats", kWithSinglePair);
  else if(nCandidates > 1)
    fHistos->FillTH1("EventStats", kWithMultiPair);
  else
    return;
  // Pairs and daughters
  fPairs->Clear("C");
  fDaughters->Clear("C");
  Int_t nD = 0;
  const TObjArray *candidates = fDielectron->GetPairArray(1);
  for (Int_t i = 0; i < nCandidates; i++)
  {
    AliDielectronPair *pair = (AliDielectronPair *)(candidates->UncheckedAt(i));
    new ((*fPairs)[i]) AliDielectronPair(*pair);
    const AliKFParticle &d1 = pair->GetKFFirstDaughter();
    new ((*fDaughters)[nD++]) TLorentzVector(d1.GetPx(), d1.GetPy(), d1.GetPz(), d1.GetE());
    const AliKFParticle &d2 = pair->GetKFSecondDaughter();
    new ((*fDaughters)[nD++]) TLorentzVector(d2.GetPx(), d2.GetPy(), d2.GetPz(), d2.GetE());
  }
  // Fill tracks
  fTracksWithPair->Clear("C");
  TIter nextTrk(fAOD->GetTracks());
  AliAODTrack* trk = NULL;
  AliAODTrack* trkTemplate = NULL;
  Int_t nDaughters = 0;
  Int_t nTracks = 0;
  while (( trk = static_cast<AliAODTrack*>(nextTrk()) )) {
    if(FindDaughters(trk)){
      nDaughters ++;
      if(!trkTemplate || trkTemplate->Pt() < trk->Pt())
        trkTemplate = trk;
      continue;
    }
    new ((*fTracksWithPair)[nTracks++]) AliAODTrack(*trk);
  }
  AliDebug(1, Form("Find dielectron pairs : %d, daughters : %d", nCandidates, nDaughters));
  // Insert pair as AOD track
  AddTrackFromPair(trkTemplate);
  AliDebug(1, Form("AOD tracks : %d, tracks with pair : %d", fAOD->GetNumberOfTracks(), fTracksWithPair->GetEntriesFast()));
  // Register in AOD event
  fAOD->AddObject(fTracksWithPair);

  // Run jet finder tasks
  AliEmcalJetTask* jetFinder = NULL;
  TIter next(fJetTasks);
  while((jetFinder=(AliEmcalJetTask*)next())){
    jetFinder->Reset();
    jetFinder->SetActive(kTRUE);
    jetFinder->Exec("");
  }
  FillHistogramsForJetQA("Jet");

  FillHistogramsForTaggedJet();
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

  fHistos->CreateTH1(Form("%s/FiredTag", histClass),"Number of event by fired trigger tag;Trig. Tag;N_{events}",32,-0.5,31.5);
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

  // Trigger Tag
  fHistos->FillTH1(Form("%s/FiredTag", histClass), fFiredTriggerTag.Data());
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
  else if (trackName.Contains("tracks")) {
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
  AddTaskEmcalJet("tracksWithPair", "", AliJetContainer::antikt_algorithm, 0.4, AliJetContainer::kChargedJet, 0.15, 0.3, 0.01, AliJetContainer::pt_scheme, "JpsiJet", 1., kFALSE, kFALSE);
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

    AliDebug(1, Form("%d jets found in %s", jets->GetNJets(), jets->GetName()));
    for(auto jet : jets->all()){
      // Jet cuts
      UInt_t rejectionReason = 0;
      if (!jets->AcceptJet(jet, rejectionReason)) {
        AliDebug(2,Form("Jet was rejected for reason : %d, details : %s", rejectionReason, (jet->toString()).Data()));
        continue;
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

void AliAnalysisTaskJpsiJet::InitDielectron(){
  fDielectron = new AliDielectron("Diele","Dielectron with EMCal triggered");
  fPairs = new TClonesArray("AliDielectronPair",10);
  fPairs->SetName("dielectrons");
  fDaughters = new TClonesArray("TLorentzVector",20);
  fDaughters->SetName("daughters");
  fTracksWithPair = new TClonesArray("AliAODTrack",500);
  fTracksWithPair->SetName("tracksWithPair");

/**
 *  Track cuts
**/
  // Good track
  AliDielectronTrackCuts *trackCuts = new AliDielectronTrackCuts("trackCuts", "trackCuts");
  trackCuts->SetClusterRequirementITS(AliDielectronTrackCuts::kSPD, AliDielectronTrackCuts::kAny);
  trackCuts->SetRequireTPCRefit(kTRUE);
  trackCuts->SetRequireITSRefit(kTRUE);
  fDielectron->GetTrackFilter().AddCuts(trackCuts);
  // Track cuts for electron PID
  AliDielectronVarCuts *ePID = new AliDielectronVarCuts("ePidCut", "Track cuts for electron PID");
  ePID->AddCut(AliDielectronVarManager::kKinkIndex0, 0.);
  // Track quality cuts
  ePID->AddCut(AliDielectronVarManager::kNclsTPC, 85., 160.);
  ePID->AddCut(AliDielectronVarManager::kEta, -0.9, 0.9);
  ePID->AddCut(AliDielectronVarManager::kImpactParXY, -1., 1.);
  ePID->AddCut(AliDielectronVarManager::kImpactParZ, -3., 3.);
  ePID->AddCut(AliDielectronVarManager::kITSLayerFirstCls, 0., 4.);
  ePID->AddCut(AliDielectronVarManager::kTPCchi2Cl, 0., 4.);
  ePID->AddCut(AliDielectronVarManager::kPt, 1.0, 1e30);
  // Electron PID with TPC
  ePID->AddCut(AliDielectronVarManager::kTPCnSigmaEle, -2.0, 3.0);
  // Exclude hadrons
  ePID->AddCut(AliDielectronVarManager::kTPCnSigmaPio, -100.0, 1.0, kTRUE);
  ePID->AddCut(AliDielectronVarManager::kTPCnSigmaKao, -100.0, 3.0, kTRUE);
  ePID->AddCut(AliDielectronVarManager::kTPCnSigmaPro, -100.0, 3.0, kTRUE);
  fDielectron->GetTrackFilter().AddCuts(ePID);

/**
 *  Pair cuts
**/
  // EMCal cuts
  Double_t EMCal_E_Threshold = 5.0;
  //Invariant mass and rapidity selection
  AliDielectronVarCuts *pairCut = new AliDielectronVarCuts("jpsiCuts", "1<M<5 + |Y|<.9");
  pairCut->AddCut(AliDielectronVarManager::kM, 1.0, 5.0);
  pairCut->AddCut(AliDielectronVarManager::kY, -0.9, 0.9);
  pairCut->AddCut(AliDielectronVarManager::kPt, 1, 1e30);
  pairCut->AddCut(AliDielectronVarManager::kPt, EMCal_E_Threshold, 1e30);
  fDielectron->GetPairFilter().AddCuts(pairCut);
  // Leg cuts
  AliDielectronVarCuts *emcCut = new AliDielectronVarCuts("CutEMCAL", "Jpsi leg cuts for EMCal");
  emcCut->AddCut(AliDielectronVarManager::kEMCALEoverP, 0.8, 1.3);
  emcCut->AddCut(AliDielectronVarManager::kEMCALE, EMCal_E_Threshold, 1e30);
  //emcCut->AddCut(AliDielectronVarManager::kPhi, 4.377, 5.7071, kTRUE); // Exclude DCal
  //emcCut->AddCut(AliDielectronVarManager::kPhi, 1.396, 3.2637, kTRUE); // Exclude EMCal
  AliDielectronPairLegCuts *legVar = new AliDielectronPairLegCuts();
  legVar->GetLeg1Filter().AddCuts(emcCut);
  legVar->GetLeg2Filter().AddCuts(emcCut);
  legVar->SetCutType(AliDielectronPairLegCuts::kAnyLeg);
  fDielectron->GetPairFilter().AddCuts(legVar);
}

void AliAnalysisTaskJpsiJet::InitHistogramsForDielectron()
{
  //Setup histogram Manager
  AliDielectronHistos *histos = new AliDielectronHistos("Dielectron", "Histograms for dielectron");
  fDielectron->SetHistogramManager(histos);

  //Initialise histogram classes
  histos->SetReservedWords("Track;Pair");
  //Track classes
  for (Int_t i = 0; i < 2; ++i)
  {
    histos->AddClass(Form("Track_%s", AliDielectron::TrackClassName(i)));
  }
  //Pair classes
  for (Int_t i = 0; i < 3; ++i)
    histos->AddClass(Form("Pair_%s", AliDielectron::PairClassName(i)));
  //Legs from pair
  for (Int_t i = 0; i < 3; ++i)
    histos->AddClass(Form("Track_Legs_%s", AliDielectron::PairClassName(i)));

/**
 *  Event
**/
  TString histClass = "Event";
  histos->AddClass(histClass);
  // Event Primary vertex and diamond (IP) stats.
  histos->UserHistogram(histClass, "VtxZ", "Vertex Z;Z[cm];#events", 1000, -50., 50., AliDielectronVarManager::kZvPrim);
  histos->UserHistogram(histClass, "VtxX", "Vertex X;X[cm];#events", 1000, -1.0, 1.0, AliDielectronVarManager::kXvPrim);
  histos->UserHistogram(histClass, "VtxY", "Vertex Y;Y[cm];#events", 2000, -1.0, 1.0, AliDielectronVarManager::kYvPrim);
  histos->UserHistogram(histClass, "NVContrib", "Number of Vertex contributors", 1000, 0, 1000, AliDielectronVarManager::kNVtxContrib);
  // Event track and SPD (tracklets) stats.
  histos->UserHistogram(histClass, "kNTrk", "Number of tracks;kNTrk;Entries", 4000, 0., 4000., AliDielectronVarManager::kNTrk);
  histos->UserHistogram(histClass, "kNaccTrcklts", "Number of accepted SPD tracklets in |eta|<1.6;kNaccTrcklts;Entries", 1000, 0., 1000., AliDielectronVarManager::kNaccTrcklts);
  histos->UserHistogram(histClass, "kNaccTrcklts10Corr", "kNaccTrcklts10Corr;kNaccTrcklts10Corr;Entries", 500, 0., 500., AliDielectronVarManager::kNaccTrcklts10Corr);
  histos->UserHistogram(histClass, "VtxZ_kNaccTrcklts10Corr", "VtxZ vs. kNaccTrcklts10Corr;VtxZ;kNaccTrcklts10Corr", 800, -40., 40., 500, 0., 500., AliDielectronVarManager::kZvPrim, AliDielectronVarManager::kNaccTrcklts10Corr);
  //new multiplicity estimator: V0
  histos->UserHistogram(histClass, "kMultV0", "kMultV0;kMultV0;Entries", 1000, 0., 1000., AliDielectronVarManager::kMultV0);
  histos->UserHistogram(histClass, "kMultV0A", "kMultV0;kMultV0;Entries", 1000, 0., 1000., AliDielectronVarManager::kMultV0A);
  histos->UserHistogram(histClass, "kMultV0C", "kMultV0;kMultV0;Entries", 1000, 0., 1000., AliDielectronVarManager::kMultV0C);
  // 2D
  histos->UserHistogram(histClass, "kMultV0A_kMultV0C", "kMultV0A vs. kMultV0C;kMultV0A;kMultV0C", 1000, 0., 1000., 1000, 0., 1000., AliDielectronVarManager::kMultV0A, AliDielectronVarManager::kMultV0C);
  histos->UserHistogram(histClass, "kMultV0_kMultV0A", "kMultV0 vs. kMultV0A;kMultV0;kMultV0A", 1000, 0., 1000., 1000, 0., 1000., AliDielectronVarManager::kMultV0, AliDielectronVarManager::kMultV0A);
  histos->UserHistogram(histClass, "kMultV0_kMultV0C", "kMultV0 vs. kMultV0C;kMultV0;kMultV0C", 1000, 0., 1000., 1000, 0., 1000., AliDielectronVarManager::kMultV0, AliDielectronVarManager::kMultV0C);
  // vs Vertex Z
  histos->UserHistogram(histClass, "VtxZ_kMultV0A", "VtxZ vs. kMultV0A;VtxZ;kMultV0A", 300, -15., 15., 1000, 0., 1000., AliDielectronVarManager::kZvPrim, AliDielectronVarManager::kMultV0A);
  histos->UserHistogram(histClass, "VtxZ_kMultV0C", "VtxZ vs. kMultV0C;VtxZ;kMultV0C", 300, -15., 15., 1000, 0., 1000., AliDielectronVarManager::kZvPrim, AliDielectronVarManager::kMultV0C);
  histos->UserHistogram(histClass, "VtxZ_kMultV0", "VtxZ vs. kMultV0;VtxZ;kMultV0", 300, -15., 15., 1000, 0., 1000., AliDielectronVarManager::kZvPrim, AliDielectronVarManager::kMultV0);

  // Dielectron info.
  histos->UserHistogram(histClass, "Nelectrons", "Number of tracks/electron selected by AliDielectron after cuts;N_{e};#events", 50, 0, 50, AliDielectronVarManager::kTracks);
  histos->UserHistogram(histClass, "Npairs", "Number of Ev1PM pair candidates after all cuts;J/#psi candidates;#events", 20, 0, 20, AliDielectronVarManager::kPairs);
  /*
	  Histogram for Track
	*/
  // Track kinetics parameter
  histos->UserHistogram("Track", "Pt", "Pt;Pt [GeV/c];#tracks", 2000, 0, 100, AliDielectronVarManager::kPt, kTRUE);
  histos->UserHistogram("Track", "Eta_Phi", "Eta Phi Map; Eta; Phi;#tracks",
                        100, -1, 1, 144, 0, TMath::TwoPi(), AliDielectronVarManager::kEta, AliDielectronVarManager::kPhi, kTRUE);
  histos->UserHistogram("Track", "dXY", "dXY;dXY [cm];#tracks", 1000, -50, 50, AliDielectronVarManager::kImpactParXY, kTRUE);
  histos->UserHistogram("Track", "dZ", "dZ;dZ [cm];#tracks", 1000, -50., 50., AliDielectronVarManager::kImpactParZ, kTRUE);
  // Tracking quality
  histos->UserHistogram("Track", "ITS_FirstCls", "ITS First Cluster;Layer No. of ITS 1st cluster;#Entries", 6, 0., 6., AliDielectronVarManager::kITSLayerFirstCls, kTRUE);
  histos->UserHistogram("Track", "TPCnCls", "Number of Clusters TPC;TPC number clusteres;#tracks", 160, 0, 160, AliDielectronVarManager::kNclsTPC, kTRUE);
  histos->UserHistogram("Track", "TPCchi2Cl", "Chi-2/Clusters TPC;Chi2/ncls number clusteres;#tracks", 100, 0, 10, AliDielectronVarManager::kTPCchi2Cl, kTRUE);
  // PID - TPC
  histos->UserHistogram("Track", "dEdx_P", "dEdx vs PinTPC;P [GeV];TPC signal (a.u.);#tracks",
                        800, 0., 40., 800, 20., 200., AliDielectronVarManager::kPIn, AliDielectronVarManager::kTPCsignal, kTRUE);
  histos->UserHistogram("Track", "dEdx_Pt", "dEdx vs Pt;Pt [GeV];TPC signal (a.u.);#tracks",
                        800, 0., 40., 800, 20., 200., AliDielectronVarManager::kPt, AliDielectronVarManager::kTPCsignal, kTRUE);
  histos->UserHistogram("Track", "dEdx_Phi", "dEdx vs #phi;#phi [rad];TPC signal (a.u.);#tracks",
                        200, 0., 2 * TMath::Pi(), 800, 20., 200., AliDielectronVarManager::kPhi, AliDielectronVarManager::kTPCsignal, kTRUE);
  histos->UserHistogram("Track", "dEdx_Eta", "dEdx vs #eta;#eta;TPC signal (a.u.);#tracks",
                        200, -1., 1., 800, 20., 200., AliDielectronVarManager::kEta, AliDielectronVarManager::kTPCsignal, kTRUE);
  histos->UserHistogram("Track", "TPCnSigmaEle_P", "n#sigma_{e}(TPC) vs P_{in} TPC;P_{in} [GeV];n#sigma_{e}(TPC);#tracks",
                        800, 0., 40., 800, -12., 12., AliDielectronVarManager::kPIn, AliDielectronVarManager::kTPCnSigmaEle, kTRUE);
  histos->UserHistogram("Track", "TPCnSigmaEle_Pt", "n#sigma_{e}(TPC) vs Pt;Pt [GeV];n#sigma_{e}(TPC);#tracks",
                        800, 0., 40., 800, -12., 12., AliDielectronVarManager::kPt, AliDielectronVarManager::kTPCnSigmaEle, kTRUE);
  histos->UserHistogram("Track", "TPCnSigmaEle_Phi", "n#sigma_{e}(TPC) vs #phi;#phi [rad];n#sigma_{e}(TPC);#tracks",
                        200, 0., 2 * TMath::Pi(), 800, -12., 12., AliDielectronVarManager::kPhi, AliDielectronVarManager::kTPCnSigmaEle, kTRUE);
  histos->UserHistogram("Track", "TPCnSigmaEle_Eta", "n#sigma_{e}(TPC) vs #eta;#eta;n#sigma_{e}(TPC);#tracks",
                        200, -1., 1., 800, -12., 12., AliDielectronVarManager::kEta, AliDielectronVarManager::kTPCnSigmaEle, kTRUE);
  histos->UserHistogram("Track", "dEdx_nSigmaEMCal", "dEdx vs n#sigma_{e}(EMCAL);n#sigma_{e}(EMCAL);TPC signal (a.u.);#tracks",
                        200, -5., 5., 800, 20., 200., AliDielectronVarManager::kEMCALnSigmaEle, AliDielectronVarManager::kTPCsignal, kTRUE);
  histos->UserHistogram("Track", "dEdx_TPCnSigmaEle", "dEdx vs n#sigma_{e}(TPC);n#sigma_{e}(TPC);TPC signal (a.u.);#tracks",
                        100, -10., 10., 800, 20., 200., AliDielectronVarManager::kTPCnSigmaEle, AliDielectronVarManager::kTPCsignal, kTRUE);
  // Track - EMCal
  histos->UserHistogram("Track", "EMCalE", "EmcalE;Cluster Energy [GeV];#Clusters",
                        200, 0., 40., AliDielectronVarManager::kEMCALE, kTRUE);
  histos->UserHistogram("Track", "EMCalE_Pt", "Cluster energy vs. pT; EMCal_E;pT;#tracks",
                        200, 0., 40., 800, 0., 40, AliDielectronVarManager::kEMCALE, AliDielectronVarManager::kPt, kTRUE);
  //Ecluster versus Phi to separate EMCal and DCal
  histos->UserHistogram("Track", "EMCalE_Phi", "Cluster energy vs. #phi; EMCal_E;Phi;#tracks",
                        200, 0., 40., 200, 0., TMath::TwoPi(), AliDielectronVarManager::kEMCALE, AliDielectronVarManager::kPhi, kTRUE);
  histos->UserHistogram("Track", "EMCalE_Eta", "Cluster energy vs. #eta; EMCal_E;Eta;#tracks",
                        200, 0., 40., 200, -1.0, 1.0, AliDielectronVarManager::kEMCALE, AliDielectronVarManager::kEta, kTRUE);
  // PID - EMCal
  // E/p ratio
  histos->UserHistogram("Track", "EoverP", "EMCal E/p ratio;E/p;#Clusters",
                        200, 0., 2., AliDielectronVarManager::kEMCALEoverP, kTRUE);
  histos->UserHistogram("Track", "EoverP_pt", "E/p ratio vs Pt;Pt (GeV/c);E/p;#tracks",
                        200, 0., 40., 200, 0., 2., AliDielectronVarManager::kPt, AliDielectronVarManager::kEMCALEoverP, kTRUE);
  histos->UserHistogram("Track", "EoverP_Phi", "E/p ratio vs #phi;Phi;E/p;#tracks",
                        200, 0., TMath::TwoPi(), 200, 0., 2., AliDielectronVarManager::kPhi, AliDielectronVarManager::kEMCALEoverP, kTRUE);
  histos->UserHistogram("Track", "EoverP_Eta", "E/p ratio vs #eta;Eta;E/p;#tracks",
                        200, -1.0, 1.0, 200, 0., 2., AliDielectronVarManager::kEta, AliDielectronVarManager::kEMCALEoverP, kTRUE);
  // EMCal nSigma electron
  histos->UserHistogram("Track", "EMCALnSigmaE_pt", "n#sigma_{e} vs Pt;Pt (GeV/c);n#sigma_{e};#tracks",
                        200, 0., 40., 200, -12, 12, AliDielectronVarManager::kPt, AliDielectronVarManager::kEMCALnSigmaEle, kTRUE);
  histos->UserHistogram("Track", "EMCALnSigmaE_Phi", "n#sigma_{e} vs #phi;Phi;n#sigma_{e};#tracks",
                        200, 0., TMath::TwoPi(), 200, -12, 12, AliDielectronVarManager::kPhi, AliDielectronVarManager::kEMCALnSigmaEle, kTRUE);
  histos->UserHistogram("Track", "EMCALnSigmaE_Eta", "n#sigma_{e} vs #eta;Eta;n#sigma_{e};#tracks",
                        200, -1.0, 1.0, 200, 0., 2., AliDielectronVarManager::kEta, AliDielectronVarManager::kEMCALnSigmaEle, kTRUE);
  histos->UserHistogram("Track", "EMCALnSigmaEle_EoverP", "n#sigma_{e}(EMCal) vs E/p;E/p;n#sigma_{e}(EMCal);#tracks",
                        200, 0., 2., 200, -12., 12., AliDielectronVarManager::kEMCALEoverP, AliDielectronVarManager::kEMCALnSigmaEle, kTRUE);
  // PID - TPC + EMCal
  histos->UserHistogram("Track", "dEdx_EoverP", "dEdx vs E/p;E/P;TPC signal (a.u.);#tracks",
                        200, 0., 2., 800, 20., 200., AliDielectronVarManager::kEMCALEoverP, AliDielectronVarManager::kTPCsignal, kTRUE);
  histos->UserHistogram("Track", "TPCnSigmaEle_EoverP", "n#sigma_{e}(TPC) vs E/p;E/p;n#sigma_{e}(TPC);#tracks",
                        200, 0., 2., 200, -12., 12., AliDielectronVarManager::kEMCALEoverP, AliDielectronVarManager::kTPCnSigmaEle, kTRUE);
  histos->UserHistogram("Track", "dEdx_EMCALnSigmaE", "dEdx vs n#sigma_{e}(EMCAL);n#sigma_{e}(EMCAL);TPC signal (a.u.);#tracks",
                        200, -12., 12., 800, 20., 200., AliDielectronVarManager::kEMCALEoverP, AliDielectronVarManager::kTPCsignal, kTRUE);
  histos->UserHistogram("Track", "nSigmaTPC_EMCal", "n#sigma_{e}(TPC vs EMCAL);n#sigma_{e}(EMCAL);n#sigma_{e}(TPC);#tracks",
                        200, -5., 5., 200, -12., 12., AliDielectronVarManager::kEMCALnSigmaEle, AliDielectronVarManager::kTPCnSigmaEle, kTRUE);

/**
 *  Histograms for Pair
**/
  histos->UserHistogram("Pair", "InvMass", "Inv.Mass;Inv. Mass (GeV/c^{2});#pairs/(40 MeV/c^{2})",
                        100, 1.0, 5.0, AliDielectronVarManager::kM);
  histos->UserHistogram("Pair", "pT", "Pt;Pt (GeV/c);#pairs",
                        2000, 0., 100.0, AliDielectronVarManager::kPt);
  histos->UserHistogram("Pair", "Eta_Phi", "#eta-#phi map of dielectron pairs;#eta_{ee};#phi_{ee};#pairs",
                        200, -1, 1, 200, 0., 10, AliDielectronVarManager::kEta, AliDielectronVarManager::kPhi);
  histos->UserHistogram("Pair", "Rapidity", "Rapidity;Rapidity;#pairs",
                        200, -1., 1., AliDielectronVarManager::kY);
  histos->UserHistogram("Pair", "OpeningAngle", "Opening angle / rad;#pairs",
                        50, 0., 3.15, AliDielectronVarManager::kOpeningAngle);

  histos->UserHistogram("Pair", "PseudoProperTime", "Pseudoproper decay length; pseudoproper-decay-length[cm];#pairs / 40#mum",
                        150, -0.3, 0.3, AliDielectronVarManager::kPseudoProperTime);
  histos->UserHistogram("Pair", "InvMass_Pt", "Inv. Mass vs Pt;Pt (GeV/c); Inv. Mass (GeV/c^{2})",
                        200, 0., 40., 100, 1.0, 5.0, AliDielectronVarManager::kPt, AliDielectronVarManager::kM);
  histos->UserHistogram("Pair", "OpeningAngle_Pt", "Opening angle vs p_{T} ;p_{T} (GeV/c); angle",
                        200, 0., 40., 200, 0, TMath::Pi(), AliDielectronVarManager::kPt, AliDielectronVarManager::kOpeningAngle);
  //InvMass versus Proper time
  histos->UserHistogram("Pair", "InvMass_ProperTime", "InvMass vs. ProperTime;pseudoproper-decay-length[cm]; Inv. Mass [GeV]",
                        120, -0.3, 0.3, 100, 1.0, 5.0, AliDielectronVarManager::kPseudoProperTime, AliDielectronVarManager::kM);
}

Bool_t AliAnalysisTaskJpsiJet::FindDaughters(AliVTrack* trk){
  static const Double_t ERR_LIMIT = 1e-6;
  TIter nextEle(fDaughters);
  TLorentzVector* vec = NULL;
  while((vec = static_cast<TLorentzVector*>(nextEle()))){
    if(TMath::Abs(vec->Pt() - trk->Pt()) < ERR_LIMIT &&
       TMath::Abs(vec->Eta() - trk->Eta()) < ERR_LIMIT &&
       TMath::Abs(TVector2::Phi_0_2pi(vec->Phi()) - trk->Phi()) < ERR_LIMIT )
      return kTRUE;
  }
  return kFALSE;
}

Double_t AliAnalysisTaskJpsiJet::GetPseudoProperDecayTime(AliDielectronPair* pair){
  auto priv = fAOD->GetPrimaryVertex();
  Double_t errPseudoProperTime2 = 0.;
  AliKFParticle kfPair = pair->GetKFParticle();
  Double_t lxy = kfPair.GetPseudoProperDecayTime(*priv, TDatabasePDG::Instance()->GetParticle(443)->Mass(), &errPseudoProperTime2 );
  return lxy;
}

void AliAnalysisTaskJpsiJet::AddTrackFromPair(AliAODTrack* trkTemplate){
  AliAODTrack* trk = new ((*fTracksWithPair)[fTracksWithPair->GetEntriesFast()]) AliAODTrack(*trkTemplate);
  AliDielectronPair *pair = static_cast<AliDielectronPair*>(fPairs->UncheckedAt(0));

  trk->SetProdVertex(fAOD->GetPrimaryVertex());
  //trk->SetStatus(AliVTrack::kEmbedded);

  trk->SetPt(pair->Pt());
  trk->SetPhi(TVector2::Phi_0_2pi(pair->Phi()));
  trk->SetTheta(TMath::ACos(pair->Pz() / pair->P()));

  // Remove EMCal
  trk->ResetStatus(AliVTrack::kEMCALmatch);
  trk->SetEMCALcluster(AliVTrack::kEMCALNoMatch);

  // Reset reference
  trk->ResetBit(kIsReferenced);
  trk->SetUniqueID(0);

  // DEBUG - Pseudo-proper decay length
  trk->SetTrackPhiEtaPtOnEMCal(pair->M(), GetPseudoProperDecayTime(pair), 0.);
}

void AliAnalysisTaskJpsiJet::InitHistogramsForTaggedJet(){
  TString histGroup = "PairInJet";
  // THnSparse - pT, M, Lxy, z, \DeltaR
  TString histName = histGroup + "/PairVars";
  Int_t nBins[5] =   {2000, 100, 150,  12,  10};
  Double_t xmin[5] = {0.,   1., -0.3,  0.,  0.};
  Double_t xmax[5] = {100., 5.,  0.3, 1.2, 1.0};
  THnSparse *hs = fHistos->CreateTHnSparse(histName.Data(), "Dielectron pair in jet variables (p_{T}-M_{e^{+}e^{-}}-L_{xy}-z-#DeltaR);p^{pair}_{T} (GeV/c);M_{e^{+}e^{-}} (GeV/c^{2});L_{xy} (cm);#DeltaR;N_{pairs}", 5, nBins, xmin, xmax);
}

Bool_t AliAnalysisTaskJpsiJet::FillHistogramsForTaggedJet(){
  // Tagged jet container
  AliJetContainer *jets = (AliJetContainer*)(fJetContainers->At(fJetContainers->GetEntriesFast()-1));
  if(!jets->GetNJets()) return kFALSE;

  // Get dielectron pair and track
  AliDielectronPair *pair = (AliDielectronPair*)(fPairs->At(0));
  Int_t pairTrackID = fTracksWithPair->GetEntriesFast() - 1;
  AliAODTrack *pairTrack = (AliAODTrack*)(fTracksWithPair->UncheckedAt(pairTrackID));
  
  // Find pair in jet
  AliEmcalJet *taggedJet = NULL;
  for(auto jet : jets->all()){
    // Map track ID in jet constituents.
    // - AliEmcalJetTask::fgkConstIndexShift = 100000
    // - More details in AliEmcalJetTask.cxx
    if(jet->ContainsTrack(pairTrackID) >= 0 || jet->ContainsTrack(pairTrackID + 100000) >= 0){
      taggedJet = jet;
      break;
    }
  }
  if(!taggedJet) return kFALSE;
  AliDebug(1, Form("Found pair (%.2f, %.2f, %.2f) in jet (%s)", pair->Pt(), pair->Eta(), TVector2::Phi_0_2pi(pair->Phi()), (taggedJet->toString()).Data()));

  TString histGroup = "PairInJet";
  // THnSparse - pT, M, Lxy, z, \DeltaR
  TString histName = histGroup + "/PairVars";
  Double_t x[5] = {0.};
  x[0] = pair->Pt();
  x[1] = pair->M();
  x[2] = GetPseudoProperDecayTime(pair);
  x[3] = (taggedJet->GetNumberOfTracks() == 1 ? 1.0 : pair->Pt() / taggedJet->Pt());
  x[4] = 0.0;

  fHistos->FillTHnSparse(histName.Data(), x, 1.0);

  return kTRUE;
}