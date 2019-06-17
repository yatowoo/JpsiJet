/*
 *
 * ROOT Macro to extract J/psi-Jet correlation function (\eta-\phi)
 *
 * Input: AliAOD.Dielectron.root
 * */

Bool_t Accept(AliDielectronPair* jpsi, Bool_t reqPrompt = kFALSE){
 
  Double_t invMass = jpsi->M();
  // pT cut - with EG2 threshold
  if(jpsi->Pt() < 5.0) return kFALSE;
  if(invMass < 2.92 || invMass > 3.16)
    return kFALSE;
  if(reqPrompt){
    return kFALSE;
  }
  return kTRUE; 
}

Bool_t Accept(AliEmcalJet* jet){
  
  if(jet->Pt() < 5.0) return kFALSE;

  return kTRUE;
}

Double_t DeltaPhi(Double_t phi1, Double_t phi2){
  // Range from -3/2\pi to 1/2\pi
  Double_t dPhi = phi1 - phi2;
  if(dPhi > 1.5 * TMath::Pi())
    dPhi = dPhi - TMath::TwoPi();
  else if(dPhi < - 0.5 * TMath::Pi())
    dPhi = TMath::TwoPi() + dPhi;
  return dPhi; 
}

void JpsiJet_Correlation(
    const char* fileName = "/data2/ytwu/LOG/ALICE/JpsiJet_QAFilter_16l_190608/AliAOD.Dielectron_strict.root")
{
  TFile* nanoAOD = new TFile(fileName);
  TTree* aodTree = (TTree*)(nanoAOD->Get("aodTree"));
  AliAODEvent* aod = new AliAODEvent;
  aod->ReadFromTree(aodTree);
  TClonesArray* pairs = new TClonesArray("AliDielectronPair",10);
  aodTree->SetBranchAddress("dielectrons",&pairs);
  TClonesArray* jets = new TClonesArray("AliEmcalJet",100);
  aodTree->SetBranchAddress("jets04",&jets);

  TH2D* hMap = new TH2D("hMap","J/#psi-Jet correlation function (#Delta#eta-#Delta#phi);#Delta#eta;#Delta#phi (rad);#entries",
      40, -2, 2,
      100, -4, 6);
  TH2D* hJpsiMap = new TH2D("hJpsi","J/#psi #eta-#phi distribution;#Delta#eta;#Delta#phi (rad);#entries",
      20,-1,1,
      100, -2, 8);
  TH2D* hJetMap = new TH2D("hJet","Jet #eta-#phi distribution;#Delta#eta;#Delta#phi (rad);#entries",
      50,-1,1,
      100, -2, 8);
  Int_t nJpsi = 0;
  for(Int_t i = 0 ; i < aodTree->GetEntries(); i++){
    aodTree->GetEntry(i);
    if(i % (aodTree->GetEntries() / 100) == 0)
      cout << "[-] Event: " << i << endl;
    TIter nextPair(pairs);
    AliDielectronPair* jpsi = NULL;
    while(jpsi = static_cast<AliDielectronPair*>(nextPair())){
      if(!Accept(jpsi)) continue;
      Double_t jpsiPhi = TVector2::Phi_0_2pi(jpsi->Phi());
      cout << "[+] J/psi found (Total: "  << ++nJpsi << "): " 
        << jpsi->M() << ", " << jpsi->Pt() << ", " << jpsi->Eta() << ", " << jpsiPhi << endl;
      hJpsiMap->Fill(jpsi->Eta(),jpsiPhi);
      TIter nextJet(jets);
      AliEmcalJet* jet = NULL;
      while(jet = static_cast<AliEmcalJet*>(nextJet())){
        if(!Accept(jet)) continue;
        hJetMap->Fill(jet->Eta(),jet->Phi());
        hMap->Fill(
          jpsi->Eta() - jet->Eta(),
          DeltaPhi(jpsiPhi,jet->Phi()));
      }// Loop - jets
    }// Loop - pairs
  }// Loop - events
  //nanoAOD->Close();
}
