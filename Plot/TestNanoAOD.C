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

Bool_t Accept(AliAODTrack* jpsi, Bool_t reqSideBand = kFALSE, Bool_t reqPrompt = kFALSE){
 
  Double_t invMass = jpsi->GetTrackPhiOnEMCal();
  Double_t lxy = jpsi->GetTrackEtaOnEMCal();
  // pT cut - with EG2 threshold
  if(jpsi->Pt() < 5.0) return kFALSE;
  Double_t Mmin = 2.92;
  Double_t Mmax = 3.16;
  if(reqSideBand){
    Mmin = 3.24;
    Mmax = 3.56;
  }
  if(invMass < Mmin || invMass > Mmax)
    return kFALSE;
  if(reqPrompt && TMath::Abs(lxy) > 0.01)
    return kFALSE;
  if(!reqPrompt && lxy < 0.01)
    return kFALSE;
  
  if(yatoDebug){
    cout << "[-] Pair info - M=" << invMass << ", Lxy=" << lxy << endl;
  }
  return kTRUE; 
}

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
    const AliKFParticle& ele = jpsi->GetKFFirstDaughter();
    new ((*daughters)[nEle++]) TLorentzVector(ele.GetPx(),ele.GetPy(),ele.GetPz(),ele.GetE());
    const AliKFParticle& ele2 = jpsi->GetKFSecondDaughter();
    new ((*daughters)[nEle++]) TLorentzVector(ele2.GetPx(),ele2.GetPy(),ele2.GetPz(),ele2.GetE());
  }
  if(nEle == 0) return kFALSE;
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
  
Bool_t Accept(AliEmcalJet* jet, Float_t jetR = 0.4){
  
  if(jet->Pt() < 5.0) return kFALSE;
  // Geometry cut - TPCfid
  if(TMath::Abs(jet->Eta()) > 0.9 - jetR)
    return kFALSE;

  return kTRUE;
}

TH1* h = NULL; 
TH1* hPrompt = NULL;
TH1* hNonPrompt = NULL;
TH1* hPromptSB = NULL;
TH1* hNonPromptSB = NULL;
  
TF1* fcn = NULL;

void HistoNorm(TH1* h, Long_t nEvent = 0){
  if(nEvent == 0) nEvent = h->GetEntries();
  Double_t normFactor = nEvent * h->GetBinWidth(1);
  if(yatoDebug) cout << "[+] Histogram Normalization Factor : " << normFactor << endl;
  if(!fcn) fcn = new TF1("fNorm","[0]",-1e6,1e6);
  fcn->SetParameter(0, normFactor);
  h->Divide(fcn);
  for(Int_t i = 0; i <= h->GetNbinsX(); i++){
    h->SetBinError(i, h->GetBinError(i) / TMath::Sqrt(normFactor)) ;
  }

}

Bool_t TestJpsiInJet(){

  if(!h){
    h = new TH1D("hZall","pT ratio of leading track;z;1/N dN/dz",12,0,1.2);
    hPrompt = new TH1D("hZpair","pT ratio of dielectron pair (Prompt);z;1/N dN/dz",12,0,1.2);
    hNonPrompt = new TH1D("hZpairB","pT ratio of dielectron pair (Non-prompt);z;1/N dN/dz",12,0,1.2);
    hPromptSB = new TH1D("hZSB","pT ratio of dielectron pair (Prompt SB);z;1/N dN/dz",12,0,1.2);
    hNonPromptSB = new TH1D("hZSBB","pT ratio of dielectron pair (Non-prompt SB);z;1/N dN/dz",12,0,1.2);
  }
  Int_t pairTrackID = aod->GetNumberOfTracks() -1;
  //Int_t pairTrackID = trkID_EleMaxPt;
  AliAODTrack* pTrk = (AliAODTrack*)(aod->GetTrack(pairTrackID));

  TIter nextJet(jets);
  AliEmcalJet* jet = NULL;

  while( jet = static_cast<AliEmcalJet*>(nextJet())){

    if(Accept(jet)){

      if(jet->ContainsTrack(pairTrackID) >= 0){
        if(yatoDebug){
          pTrk->Print();
          jet->Print();
        }
        Double_t z = pTrk->Pt() / jet->Pt();
        // Signal region
        if(Accept(pTrk, kFALSE, kTRUE))
          hPrompt->Fill(z);
        if(Accept(pTrk, kFALSE, kFALSE))
          hNonPrompt->Fill(z);
        // Side band
        if(Accept(pTrk, kTRUE, kTRUE))
          hPromptSB->Fill(z);
        if(Accept(pTrk, kTRUE, kFALSE))
          hNonPromptSB->Fill(z);
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
  aod->ReadFromTree(aodT);
  
  pairs = new TClonesArray("AliDielectronPair",100);
  aodT->SetBranchAddress("dielectrons",&pairs);

  jets = new TClonesArray("AliEmcalJet",100);
  aodT->SetBranchAddress("jets04",&jets);

  daughters = new TClonesArray("TLorentzVector",100);

  for(Int_t i = 0 ; i < aodT->GetEntries(); i++){
    aodT->GetEntry(i);
    if(yatoDebug) cout << "[-] Event " << i << " ------> " << endl;
    //TestPairDaughter();
    TestJpsiInJet();
  }
}
