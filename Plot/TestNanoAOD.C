// Script to validate nano AOD
//

Bool_t yatoDebug = kFALSE;

TFile* f = NULL;
TTree* aodT = NULL;
AliAODEvent* aod = NULL;
TClonesArray* pairs = NULL;
TClonesArray* daughters = NULL;
TClonesArray* jets = NULL;

Int_t trkID_EleMaxPt = 0;

AliAODTrack* GetTrackFromPair(AliDielectronPair* pair, AliAODTrack* tmp){

  AliAODTrack* trk = (AliAODTrack*)(tmp->Clone("AliAODTrack"));

  trk->SetPt(pair->Pt());
  trk->SetPhi(TVector2::Phi_0_2pi(pair->Phi()));
  trk->SetTheta(TMath::ACos(pair->Pz() / pair->P()));

  trk->SetStatus(AliVTrack::kEmbedded);

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

Bool_t TestPairDaughter(){
  
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
  cout << "[-] TestPairDaughter - Pairs=" << pairs->GetEntries()
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
  trkID_EleMaxPt = aod->GetTracks()->IndexOf(tmp);
  if(nEle != nTrackMatched){
    cout << "[X] ERROR TestPairDaughter -  nTrackMatched=" << nTrackMatched << endl;
    //exit(1);
  }
  nextPair.Reset();
  while(jpsi = static_cast<AliDielectronPair*>(nextPair())){
    if(yatoDebug) tmp->Print();
    trk = GetTrackFromPair(jpsi, tmp);
    if(yatoDebug) trk->Print();
    delete trk;trk=NULL;
  }


  return kTRUE;
}
  
TH1* h = NULL; 
TH1* h2 = NULL;

Bool_t TestJpsiInJet(){

  if(!h){
    h = new TH1D("hZall","pT ratio of leading track",20,0,2);
    h2 = new TH1D("hZpair","pT ratio of dielectron pair",20,0,2);
  }
  Int_t pairTrackID = aod->GetNumberOfTracks() -1;
  //Int_t pairTrackID = trkID_EleMaxPt;
  AliAODTrack* pTrk = (AliAODTrack*)(aod->GetTrack(pairTrackID));

  TIter nextJet(jets);

  while( jet = static_cast<AliEmcalJet*>(nextJet())){

    if(jet->Pt() > 5. ){

      if(jet->ContainsTrack(pairTrackID) >= 0){
        if(yatoDebug){
          pTrk->Print();
          jet->Print();
        }
        h2->Fill(pTrk->Pt() / jet->Pt());
      }
      
      Double_t maxPt = jet->MaxChargedPt();
      Double_t pt = jet->Pt();
      Double_t z = maxPt/pt;
      h->Fill(z);
    }
  }

  return kTRUE;
}

void TestNanoAOD( Bool_t usrDebug = kFALSE, const char* fileName = "AliAOD.ANA.root"){

  yatoDebug= usrDebug;

  f = new TFile(fileName);
  aodT = (TTree*)(f->Get("aodTree"));
  aod = new AliAODEvent;
  aod->ReadFromTree(aodTree);
  
  pairs = new TClonesArray("AliDielectronPair",100);
  aodTree->SetBranchAddress("dielectrons",&pairs);

  jets = new TClonesArray("AliEmcalJet",100);
  aodTree->SetBranchAddress("jets04",&jets);

  daughters = new TClonesArray("TLorentzVector",100);

  for(Int_t i = 0 ; i < aodT->GetEntries(); i++){
    aodT->GetEntry(i);
    if(yatoDebug) cout << "[-] Event " << i << " ------> " << endl;
    //TestPairDaughter();
    TestJpsiInJet();
  }
}
