// Script to validate nano AOD
//

TFile* f = NULL;
TTree* aodT = NULL;
AliAODEvent* aod = NULL;
TClonesArray* pairs = NULL;
TClonesArray* daughters = NULL;

AliAODTrack* GetTrackFromPair(AliDielectronPair* pair, AliAODTrack* tmp){

  Double_t p[3] = {0.};
  Double_t v[3] = {0.};
  Double_t cov[21] = {0.};
  pair->PxPyPz(p);
  pair->XvYvZv(v);
  
  tmp->GetCovarianceXYZPxPyPz(cov);
  
  AliAODTrack* trk = new AliAODTrack(
      tmp->GetID(),
      tmp->GetLabel(),
      p, kTRUE,
      v, kFALSE,
      cov,
      (Short_t)(tmp->GetSign()),
      tmp->GetITSClusterMap(),
      tmp->GetProdVertex(), // PrimaryVertex?
      tmp->GetUsedForVtxFit(),
      tmp->GetUsedForPrimVtxFit(),
      tmp->GetType(),
      tmp->GetFilterMap(),
      tmp->Chi2perNDF()
      );

  return trk;
}

Bool_t FindDaughters(AliVTrack* trk){
  static const Double_t ERR_LIMIT = 1e-6;
  TIter nextEle(daughters);
  TLorentzVector* vec = NULL;
  while(vec = static_cast<TLorentzVector*>(nextEle())){
    if(TMath::Abs(vec->Pt() - trk->Pt()) < ERR_LIMIT &&
       TMath::Abs(vec->Eta() - trk->Eta()) < ERR_LIMIT &&
       TMath::Abs(TVector2::Phi_0_2pi(vec->Phi()) - trk->Phi()) < ERR_LIMIT )
      return kTRUE;
  }
  return kFALSE;
}

void TestNanoAOD(const char* fileName = "AliAOD.ANA.root"){

  f = new TFile(fileName);
  aodT = (TTree*)(f->Get("aodTree"));
  aod = new AliAODEvent;
  aod->ReadFromTree(aodTree);
  
  pairs = new TClonesArray("AliDielectronPair",100);
  aodTree->SetBranchAddress("dielectrons",&pairs);

  daughters = new TClonesArray("TLorentzVector",100);

  for(Int_t i = 0 ; i < aodT->GetEntries(); i++){
    aodT->GetEntry(i);
    // DEBUG - Shared electron?
    if(pairs->GetEntries() == 1) continue;
    TIter nextPair(pairs);
    AliDielectronPair* jpsi = NULL;
    daughters->Clear("C");
    Int_t nEle = 0;
    while(jpsi = static_cast<AliDielectronPair*>(nextPair())){
      AliKFParticle& ele = jpsi->GetKFFirstDaughter();
      new ((*daughters)[nEle++]) TLorentzVector(ele.GetPx(),ele.GetPy(),ele.GetPz(),ele.GetE());
      ele = jpsi->GetKFSecondDaughter();
      new ((*daughters)[nEle++]) TLorentzVector(ele.GetPx(),ele.GetPy(),ele.GetPz(),ele.GetE());
    }
    if(nEle == 0) continue;
    cout << "[-] Event " << i << " - Pairs=" << pairs->GetEntries()
     << " Daughters=" << daughters->GetEntries() << endl; 
    TIter nextTrack(aod->GetTracks());
    AliAODTrack* trk = NULL;
    AliAODTrack* tmp = NULL;
    Int_t nTrackMatched = 0;
    while(trk = static_cast<AliAODTrack*>(nextTrack())){
      if(FindDaughters(trk)){
        nTrackMatched++;
        if(!tmp || tmp->Pt() < trk->Pt()) tmp = trk;
      }
    }
    if(nEle != nTrackMatched){
      cout << "[X] ERROR - event " << i << " : nTrackMatched=" << nTrackMatched << endl;
      daughters->Print();
      //exit(1);
    }
    nextPair.Reset();
    while(jpsi = static_cast<AliDielectronPair*>(nextPair())){
      trk = GetTrackFromPair(jpsi, tmp);
      trk->Print();
      delete trk;trk=NULL;
    }
  }
}
