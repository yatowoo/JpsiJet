#include "AliAnalysisTaskJpsiJetFilter.h"

#include "TChain.h"
#include "THashList.h"

#include <AliLog.h>
#include "AliAODCaloCluster.h"
#include "AliAODBranchReplicator.h"

ClassImp(AliAnalysisTaskJpsiJetFilter)

AliAnalysisTaskJpsiJetFilter::AliAnalysisTaskJpsiJetFilter() : 
  AliAnalysisTaskSE(),
  fIsToMerge(kFALSE),
  fIsToReplace(kFALSE),
  fOutputFileName("AliAOD.Dielectron.root"),
  fExtAOD(0x0),
  fSPD(0x0),
  fEMCALTrigger(0x0),
  fPHOSTrigger(0x0),
  fEMCalCells(0x0),
  fPHOSCells(0x0),
  fAODZDC(0x0),
  fAODAD(0x0),
  fAODTZERO(0x0),
  fPairs(0x0),
  fDaughters(0x0),
  fJets02(0x0),
  fJets04(0x0),
  fDielectron(0),
  fSelectPhysics(kTRUE),
  fTriggerMask(AliVEvent::kMB),
  fExcludeTriggerMask(0),
  fTriggerOnV0AND(kFALSE),
  fRejectPileup(kFALSE),
  fEventStat(0x0),
  fTriggerLogic(kAny),
  fTriggerAnalysis(0x0),
  fStoreLikeSign(kFALSE),
  fStoreRotatedPairs(kFALSE),
  fStoreEventsWithSingleTracks(kFALSE),
  fCreateNanoAOD(kFALSE),
  fStoreHeader(kFALSE),
  fStoreEventplanes(kFALSE),
  fEventFilter(0x0),
  fQnList(0x0)
{}

AliAnalysisTaskJpsiJetFilter::AliAnalysisTaskJpsiJetFilter(const char* name) : 
  AliAnalysisTaskSE(name),
  fIsToMerge(kFALSE),
  fIsToReplace(kFALSE),
  fOutputFileName("AliAOD.Dielectron.root"),
  fExtAOD(0x0),
  fSPD(0x0),
  fEMCALTrigger(0x0),
  fPHOSTrigger(0x0),
  fEMCalCells(0x0),
  fPHOSCells(0x0),
  fAODZDC(0x0),
  fAODAD(0x0),
  fAODTZERO(0x0),
  fPairs(0x0),
  fDaughters(0x0),
  fJets02(0x0),
  fJets04(0x0),
  fDielectron(0),
  fSelectPhysics(kTRUE),
  fTriggerMask(AliVEvent::kMB),
  fExcludeTriggerMask(0),
  fTriggerOnV0AND(kFALSE),
  fRejectPileup(kFALSE),
  fEventStat(0x0),
  fTriggerLogic(kAny),
  fTriggerAnalysis(0x0),
  fStoreLikeSign(kFALSE),
  fStoreRotatedPairs(kFALSE),
  fStoreEventsWithSingleTracks(kFALSE),
  fCreateNanoAOD(kFALSE),
  fStoreHeader(kFALSE),
  fStoreEventplanes(kFALSE),
  fEventFilter(0x0),
  fQnList(0x0)
{
  DefineInput(0,TChain::Class());
  DefineOutput(1, THashList::Class());
  DefineOutput(2, TH1D::Class());
};

AliAnalysisTaskJpsiJetFilter::~AliAnalysisTaskJpsiJetFilter()
{
  // Destructor
  if(fDielectron)
    delete fDielectron;
  if(fEventStat)
    delete fEventStat;
  if(fTriggerAnalysis)
    delete fTriggerAnalysis;
  if(fEventFilter)
    delete fEventFilter;
  if(fQnList)
    delete fQnList;
  if(fExtAOD) delete fExtAOD;
  if(fSPD) delete fSPD;
  if(fEMCALTrigger) delete fEMCALTrigger;
  if(fPHOSTrigger) delete fPHOSTrigger;
  if(fEMCalCells) delete fEMCalCells;
  if(fPHOSCells) delete fPHOSCells;
  if(fAODZDC) delete fAODZDC;
  if(fAODAD) delete fAODAD;
  if(fAODTZERO) delete fAODTZERO;
  if(fPairs){
    fPairs->Clear("C");
    delete fPairs;
  }
  if(fDaughters){
    fDaughters->Clear("C");
    delete fDaughters;
  }
  if(fJets02){
    fJets02->Clear("C");
    delete fJets02;
  }
  if(fJets04){
    fJets04->Clear("C");
    delete fJets04;
  }
}

Bool_t AliAnalysisTaskJpsiJetFilter::Notify()
{
  AddMetadataToUserInfo();
  return kTRUE;
}

Bool_t AliAnalysisTaskJpsiJetFilter::AddMetadataToUserInfo()
{
  static Bool_t copyFirst = kFALSE;
  if (!copyFirst) {
    AliAnalysisManager *mgr = AliAnalysisManager::GetAnalysisManager();
    if (!mgr) {
      AliError("AliAnalysisTaskJpsiJetFilter::AddMetadataToUserInfo() : No analysis manager !");
      return kFALSE;
    }
    TTree *aodTree = mgr->GetTree()->GetTree();
    if (!aodTree) return kFALSE;
    TNamed *alirootVersion = (TNamed*)aodTree->GetUserInfo()->FindObject("alirootVersion");
    if (!alirootVersion) return kFALSE;
    AliAODHandler *aodHandler = dynamic_cast<AliAODHandler*>(mgr->GetOutputEventHandler());
    if (!aodHandler) return kFALSE;
    AliAODExtension *extDielectron = aodHandler->GetFilteredAOD("AliAOD.Dielectron.root");
    TTree *nanoaodTree = extDielectron->GetTree();
    if (!nanoaodTree) return kFALSE;
    nanoaodTree->GetUserInfo()->Add(new TNamed(*alirootVersion));
    copyFirst = kTRUE;
  }
  return kTRUE;
}

void AliAnalysisTaskJpsiJetFilter::UserCreateOutputObjects()
{
  if (!fDielectron) {
    AliFatal("Dielectron framework class required. Please create and instance with proper cuts and set it via 'SetDielectron' before executing this task!!!");
    return;
  }
  if(fStoreRotatedPairs) fDielectron->SetStoreRotatedPairs(kTRUE);
  fDielectron->SetDontClearArrays();
  fDielectron->Init();

  Int_t nbins=kNbinsEvent+2;
  if (!fEventStat){
    fEventStat=new TH1D("hEventStat","Event statistics",nbins,0,nbins);
    fEventStat->GetXaxis()->SetBinLabel(1,"Before Phys. Sel.");
    fEventStat->GetXaxis()->SetBinLabel(2,"After Phys. Sel.");

    fEventStat->GetXaxis()->SetBinLabel(3,"Bin3 not used");
    fEventStat->GetXaxis()->SetBinLabel(4,"Bin4 not used");
    fEventStat->GetXaxis()->SetBinLabel(5,"Bin5 not used");

    if(fTriggerOnV0AND) fEventStat->GetXaxis()->SetBinLabel(3,"V0and triggers");
    if (fEventFilter) fEventStat->GetXaxis()->SetBinLabel(4,"After Event Filter");
    if (fRejectPileup) fEventStat->GetXaxis()->SetBinLabel(5,"After Pileup rejection");

    fEventStat->GetXaxis()->SetBinLabel((kNbinsEvent+1),Form("#splitline{1 candidate}{%s}",fDielectron->GetName()));
    fEventStat->GetXaxis()->SetBinLabel((kNbinsEvent+2),Form("#splitline{With >1 candidate}{%s}",fDielectron->GetName()));
  }

  // Create branch to nano AOD - track/vertex/cluster
  TClonesArray *nanoAODTracks = new TClonesArray("AliAODTrack",500);
  nanoAODTracks->SetName("tracks");
  fExtAOD->AddBranch("TClonesArray", &nanoAODTracks);
  TClonesArray *nanoAODVertices = new TClonesArray("AliAODVertex",500);
  nanoAODVertices->SetName("vertices");
  fExtAOD->AddBranch("TClonesArray", &nanoAODVertices);
  TClonesArray *nanoAODCaloCluster = new TClonesArray("AliAODCaloCluster",500);
  nanoAODCaloCluster->SetName("caloClusters");
  fExtAOD->AddBranch("TClonesArray", &nanoAODCaloCluster);
  // Create branch for other objects (not TClonesArray) 
  fSPD = new AliAODTracklets;
  fSPD->SetName("tracklets");
  fExtAOD->AddBranch("AliAODTracklets",&fSPD);

  fEMCALTrigger = new AliAODCaloTrigger;
  fEMCALTrigger->SetName("emcalTrigger");
  fExtAOD->AddBranch("AliAODCaloTrigger",&fEMCALTrigger);
  fPHOSTrigger = new AliAODCaloTrigger;
  fPHOSTrigger->SetName("phosTrigger");
  fExtAOD->AddBranch("AliAODCaloTrigger",&fPHOSTrigger);

  fEMCalCells = new AliAODCaloCells;
  fEMCalCells->SetName("emcalCells");
  fExtAOD->AddBranch("AliAODCaloCells",&fEMCalCells);
  fPHOSCells = new AliAODCaloCells;
  fPHOSCells->SetName("phosCells");
  
  fExtAOD->AddBranch("AliAODCaloCells",&fPHOSCells);
  fAODZDC = new AliAODZDC;
  fExtAOD->AddBranch("AliAODZDC",&fAODZDC);
  fAODAD = new AliAODAD;
  fExtAOD->AddBranch("AliAODAD",&fAODAD);
  fAODTZERO = new AliAODTZERO;
  fExtAOD->AddBranch("AliAODTZERO",&fAODTZERO);
  
  // Create branch for user-defined objects
  fPairs = new TClonesArray("AliDielectronPair",10);
  fPairs->SetName("dielectrons");
  fExtAOD->AddBranch("TClonesArray", &fPairs);
  fJets02 = new TClonesArray("AliEmcalJet", 100);
  fJets02->SetName("jets02");
  fExtAOD->AddBranch("TClonesArray", &fJets02);
  fJets04 = new TClonesArray("AliEmcalJet", 100);
  fJets04->SetName("jets04");
  fExtAOD->AddBranch("TClonesArray", &fJets04);

  fExtAOD->GetAOD()->GetStdContent();

  PostData(1, const_cast<THashList*>(fDielectron->GetHistogramList()));
  PostData(2,fEventStat);

  // Init array for pair daughter
  fDaughters = new TClonesArray("TLorentzVector",20);
  fDaughters->SetName("daughters");
}


void AliAnalysisTaskJpsiJetFilter::Init(){
  // Initialization
  if (fDebug > 1) AliInfo("Init() \n");
  // require AOD handler
  AliAODHandler *aodH = (AliAODHandler*)((AliAnalysisManager::GetAnalysisManager())->GetOutputEventHandler());
  if (!aodH) AliFatal("No AOD handler. Halting.");
  fExtAOD = aodH->AddFilteredAOD(fOutputFileName, "DielectronEvents", fIsToMerge);
  if(!fExtAOD) AliFatal("Fail to add filtered AOD");
}

// Update: copy title for primary vertex
void AliAnalysisTaskJpsiJetFilter::UserExec(Option_t*){
  //
  // Main loop. Called for every event
  //

  if (!fDielectron)
    return;

  AliAnalysisManager *man = AliAnalysisManager::GetAnalysisManager();
  Bool_t isESD = man->GetInputEventHandler()->IsA() == AliESDInputHandler::Class();
  Bool_t isAOD = man->GetInputEventHandler()->IsA() == AliAODInputHandler::Class();

  AliInputEventHandler *inputHandler = (AliInputEventHandler *)(man->GetInputEventHandler());
  if (!inputHandler)
    return;

  if (inputHandler->GetPIDResponse())
  {
    AliDielectronVarManager::SetPIDResponse(inputHandler->GetPIDResponse());
  }
  else
  {
    AliFatal("This task needs the PID response attached to the input event handler!");
  }

  // Was event selected ?
  ULong64_t isSelected = AliVEvent::kAny;
  Bool_t isRejected = kFALSE;
  if (fSelectPhysics && inputHandler)
  {
    if ((isESD && inputHandler->GetEventSelection()) || isAOD)
    {
      isSelected = inputHandler->IsEventSelected();
      if (fExcludeTriggerMask && (isSelected & fExcludeTriggerMask))
        isRejected = kTRUE;
      if (fTriggerLogic == kAny)
        isSelected &= fTriggerMask;
      else if (fTriggerLogic == kExact)
        isSelected = ((isSelected & fTriggerMask) == fTriggerMask);
    }
  }

  //before physics selection
  fEventStat->Fill(kAllEvents);
  if (isSelected == 0 || isRejected)
  {
    PostData(2, fEventStat);
    return;
  }
  //after physics selection
  fEventStat->Fill(kSelectedEvents);

  //V0and
  if (fTriggerOnV0AND)
  {
    if (isESD)
    {
      if (!fTriggerAnalysis->IsOfflineTriggerFired(static_cast<AliESDEvent *>(InputEvent()), AliTriggerAnalysis::kV0AND))
        return;
    }
    if (isAOD)
    {
      if (!((static_cast<AliAODEvent *>(InputEvent()))->GetVZEROData()->GetV0ADecision() == AliVVZERO::kV0BB &&
            (static_cast<AliAODEvent *>(InputEvent()))->GetVZEROData()->GetV0CDecision() == AliVVZERO::kV0BB))
        return;
    }
  }

  fEventStat->Fill(kV0andEvents);

  //Fill Event histograms before the event filter
  AliDielectronHistos *h = fDielectron->GetHistoManager();

  Double_t values[AliDielectronVarManager::kNMaxValues] = {0};
  Double_t valuesMC[AliDielectronVarManager::kNMaxValues] = {0};
  if (h)
    AliDielectronVarManager::SetFillMap(h->GetUsedVars());
  else
    AliDielectronVarManager::SetFillMap(0x0);
  AliDielectronVarManager::SetEvent(InputEvent());
  AliDielectronVarManager::Fill(InputEvent(), values);
  AliDielectronVarManager::Fill(InputEvent(), valuesMC);

  Bool_t hasMC = fDielectron->GetHasMC();
  if (hasMC)
  {
    if (AliDielectronMC::Instance()->ConnectMCEvent())
      AliDielectronVarManager::Fill(AliDielectronMC::Instance()->GetMCEvent(), valuesMC);
  }

  if (h)
  {
    if (h->GetHistogramList()->FindObject("Event_noCuts"))
      h->FillClass("Event_noCuts", AliDielectronVarManager::kNMaxValues, values);
    if (hasMC && h->GetHistogramList()->FindObject("MCEvent_noCuts"))
      h->FillClass("Event_noCuts", AliDielectronVarManager::kNMaxValues, valuesMC);
  }

  //event filter
  if (fEventFilter)
  {
    if (!fEventFilter->IsSelected(InputEvent()))
      return;
  }
  fEventStat->Fill(kFilteredEvents);

  //pileup
  if (fRejectPileup)
  {
    if (InputEvent()->IsPileupFromSPD(3, 0.8, 3., 2., 5.))
      return;
  }
  fEventStat->Fill(kPileupEvents);

  //bz for AliKF
  Double_t bz = InputEvent()->GetMagneticField();
  AliKFParticle::SetField(bz);

  AliDielectronPID::SetCorrVal((Double_t)InputEvent()->GetRunNumber());

  fDielectron->Process(InputEvent());

  Bool_t hasCand = kFALSE;
  if (fStoreLikeSign)
    hasCand = (fDielectron->HasCandidates() || fDielectron->HasCandidatesLikeSign());
  else
    hasCand = (fDielectron->HasCandidates());

  if (fStoreRotatedPairs)
    hasCand = (hasCand || fDielectron->HasCandidatesTR());

  if (fStoreEventsWithSingleTracks)
    hasCand = (hasCand || fDielectron->GetTrackArray(0) || fDielectron->GetTrackArray(1));

  // Fill nano AOD
  if ( hasCand)
  {
    AliAODEvent *aodEv = (static_cast<AliAODEvent *>(InputEvent()));
    // Fill ZDC, AD, TZERO data
    AliAODZDC* zdc = aodEv->GetZDCData();
    fAODZDC->Clear("C");
    *fAODZDC = *zdc;
    AliAODAD* ad = aodEv->GetADData();
    fAODAD->Clear("C");
    *fAODAD = *ad;
    AliAODTZERO* tzero = aodEv->GetTZEROData();
    fAODTZERO->Clear("C");
    *fAODTZERO = *tzero;
    // Fil tracklets, EMCal/PHOS calo trigger
    AliAODTracklets* spd = aodEv->GetMultiplicity();
    fSPD->Clear("C");
    *fSPD = *spd;
    AliAODCaloTrigger* caloTrig = aodEv->GetCaloTrigger("EMCAL");
    fEMCALTrigger->Clear("C");
    *fEMCALTrigger = *caloTrig;
    caloTrig = aodEv->GetCaloTrigger("PHOS");
    fPHOSTrigger->Clear("C");
    *fPHOSTrigger = *caloTrig;
    // Fill EMCal/PHOS cells
    AliAODCaloCells* cells = aodEv->GetEMCALCells();
    fEMCalCells->Clear("C");
    *fEMCalCells = *cells;
    cells = aodEv->GetPHOSCells();
    fPHOSCells->Clear("C");
    *fPHOSCells = *cells;
    // Clear arrays
    AliAODEvent *nanoEv = fExtAOD->GetAOD();
    nanoEv->GetTracks()->Clear("C");
    nanoEv->GetVertices()->Clear("C");
    nanoEv->GetCaloClusters()->Clear("C");
    // Fill primary and SPD vertex
    AliAODVertex *vtxPriv = aodEv->GetPrimaryVertex();
    if(!vtxPriv) AliFatal("No primary vertex");
    AliAODVertex *vtxSPD = aodEv->GetPrimaryVertexSPD();
    AliAODVertex *vtxTPC = aodEv->GetPrimaryVertexTPC();
    AliAODVertex *tmp = vtxPriv->CloneWithoutRefs();
    tmp->SetTitle("VertexerTracksMVWithConstraint");
    nanoEv->AddVertex(tmp);
    AliAODVertex *tmpSPD = vtxSPD->CloneWithoutRefs(); 
    tmpSPD->SetTitle(vtxSPD->GetTitle());
    nanoEv->AddVertex(tmpSPD); 
    AliAODVertex *tmpTPC = vtxTPC->CloneWithoutRefs(); 
    tmpTPC->SetTitle(vtxTPC->GetTitle());
    nanoEv->AddVertex(tmpTPC); 
    
    // Fill pairs
    Int_t ncandidates = fDielectron->GetPairArray(1)->GetEntriesFast();
    if (ncandidates == 1)
      fEventStat->Fill((kNbinsEvent));
    else if (ncandidates > 1)
      fEventStat->Fill((kNbinsEvent + 1));
    fPairs->Clear("C");
    fDaughters->Clear("C");
    Int_t nD = 0;
    const TObjArray* candidates = fDielectron->GetPairArray(1);
    for(Int_t i = 0; i < ncandidates; i++){
      AliDielectronPair* pair = (AliDielectronPair*)(candidates->UncheckedAt(i));
      new((*fPairs)[i]) AliDielectronPair(*pair);
      const AliKFParticle& d1 = pair->GetKFFirstDaughter();
      new ((*fDaughters)[nD++]) TLorentzVector(d1.GetPx(),d1.GetPy(),d1.GetPz(),d1.GetE());
      const AliKFParticle& d2 = pair->GetKFSecondDaughter();
      new ((*fDaughters)[nD++]) TLorentzVector(d2.GetPx(),d2.GetPy(),d2.GetPz(),d2.GetE());
    }
    fPairs->Expand(ncandidates);

    // Fill tracks
    Int_t nTracks = aodEv->GetNumberOfTracks();
    Int_t nTrackMatched = 0;
    AliAODTrack* trkTemplate = NULL; // To insert pair as track
    for (int iTrk = 0; iTrk < nTracks; iTrk++)
    {
      AliAODTrack* oldTrack = (AliAODTrack*)(aodEv->GetTrack(iTrk));
      // To remove tracks used as pair daughters
      if(fIsToReplace && FindDaughters(oldTrack)){
        nTrackMatched++;
        if(!trkTemplate || trkTemplate->Pt() < oldTrack->Pt()) trkTemplate = oldTrack;
        continue;
      }
      // Add track for nano AOD
      Int_t trkID = nanoEv->AddTrack(oldTrack);
      AliAODTrack* trk = (AliAODTrack*)(nanoEv->GetTrack(trkID));
      trk->ResetBit(kIsReferenced);
      trk->SetUniqueID(0);
      // Vertex of origin
      Int_t vtxID = nanoEv->AddVertex(oldTrack->GetProdVertex());
      AliAODVertex* vtx = nanoEv->GetVertex(vtxID);
      vtx->ResetBit(kIsReferenced);
      vtx->SetUniqueID(0);
      vtx->RemoveDaughters();
      trk->SetProdVertex(vtx);
      // Calo cluster
      Int_t oldCaloID = oldTrack->GetEMCALcluster();
      if(oldCaloID >= 0){
        AliAODCaloCluster* oldCalo = aodEv->GetCaloCluster(oldCaloID);
        if (oldCalo)
        {
          Int_t caloID = nanoEv->AddCaloCluster(oldCalo);
          trk->SetEMCALcluster(caloID);
          AliAODCaloCluster *calo = nanoEv->GetCaloCluster(caloID);
          for (int u = 0; u < calo->GetNTracksMatched(); u++)
            calo->RemoveTrackMatched(calo->GetTrackMatched(u));
          calo->AddTrackMatched(trk);
        }
      }
    }
    if(fIsToReplace){
      TIter nextPair(fPairs);
      AliDielectronPair* pair = NULL;
      while(pair = static_cast<AliDielectronPair*>(nextPair())){
        Int_t trkID = nanoEv->AddTrack(trkTemplate);
        AliAODTrack* trk = (AliAODTrack*)(nanoEv->GetTrack(trkID));
        SetTrackFromPair(pair, trk);
        trk->SetProdVertex(nanoEv->GetPrimaryVertex());
      }
      AliDebug(2, Form("Remove Ndaughters : %d, Add Npairs : %d", nTrackMatched, ncandidates));
    }
    nanoEv->GetTracks()->Expand(nanoEv->GetNumberOfTracks());
    nanoEv->GetVertices()->Expand(nanoEv->GetNumberOfVertices());
    nanoEv->GetCaloClusters()->Expand(nanoEv->GetNumberOfCaloClusters());


    // Fill jets
    FillJets(aodEv, fJets04, "Jet_AKTChargedR040_tracks_pT0150_pt_scheme");

    // Write output
    fExtAOD->SelectEvent();
    // DEBUG - not use FinishEvent() to avoid auto flush
    fExtAOD->GetTree()->Fill();
    
    delete tmp;
    delete tmpSPD;
    delete tmpTPC;
  }

  
  // Clear pair arrays -- from fDielectron->ClearArrays();
  for (Int_t i=0;i<11;++i){
    TObjArray* pairArr = (TObjArray *)((*(fDielectron->GetPairArraysPointer()))->UncheckedAt(i));
    if (pairArr) pairArr->Delete();
  }

  PostData(1, const_cast<THashList *>(fDielectron->GetHistogramList()));
  PostData(2, fEventStat);
  return;
} 

void AliAnalysisTaskJpsiJetFilter::FillJets(AliAODEvent* aodEv, TClonesArray* jetArray, TString jetName){
  jetArray->Clear("C");
  TClonesArray* jets = dynamic_cast<TClonesArray*>(aodEv->FindListObject(jetName));
  if(jets){
    for(Int_t i = 0; i < jets->GetEntriesFast(); i++){
      AliEmcalJet* jet = dynamic_cast<AliEmcalJet*>(jets->UncheckedAt(i));
      new((*jetArray)[i]) AliEmcalJet(*jet);
    }
    jetArray->Expand(jets->GetEntriesFast());
  }else{
    AliInfo(Form("Could not found jet container : %s", jetName.Data()));
  }

}

Bool_t AliAnalysisTaskJpsiJetFilter::FindDaughters(AliVTrack* trk){
  static const Double_t ERR_LIMIT = 1e-6;
  TIter nextEle(fDaughters);
  TLorentzVector* vec = NULL;
  while(vec = static_cast<TLorentzVector*>(nextEle())){
    if(TMath::Abs(vec->Pt() - trk->Pt()) < ERR_LIMIT &&
       TMath::Abs(vec->Eta() - trk->Eta()) < ERR_LIMIT &&
       TMath::Abs(TVector2::Phi_0_2pi(vec->Phi()) - trk->Phi()) < ERR_LIMIT )
      return kTRUE;
  }
  return kFALSE;
}

void AliAnalysisTaskJpsiJetFilter::SetTrackFromPair(AliDielectronPair* pair, AliAODTrack* trk){
  
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
  auto aod = (AliAODEvent*)(InputEvent());
  auto priv = aod->GetPrimaryVertex();
  Double_t errPseudoProperTime2 = 0.;
  AliKFParticle kfPair = pair->GetKFParticle();
  Double_t lxy = kfPair.GetPseudoProperDecayTime(*priv, TDatabasePDG::Instance()->GetParticle(443)->Mass(), &errPseudoProperTime2 );
  trk->SetTrackPhiEtaPtOnEMCal(pair->M(), lxy, 0.);

}
