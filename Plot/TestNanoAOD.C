// Script to validate nano AOD
//

TFile* f = NULL;
TTree* aodTree = NULL;
AliAODEvent* aod = NULL;
TClonesArray* pairs = NULL;
TClonesArray* daughters = NULL;

void TestNanoAOD(const char* fileName = "AliAOD.Dielectron_strict.root"){

  f = new TFile(fileName);
  aodTree = (TTree*)(f->Get("aodTree"));
  aod = new AliAODEvent;
  aod->ReadFromTree(aodTree);
  
  pairs = new TClonesArray("AliDielectronPair",100);
  aodTree->SetBranchAddress("dielectrons",&pairs);

  daughters = new TClonesArray("AliKFParticle",100);

  for(Int_t i = 0 ; i < aodTree->GetEntries(); i++){
    aodTree->GetEntry(i);
    TIter nextPair(pairs);
    AliDielectronPair* jpsi = NULL;
    daughters->Clear("C");
    Int_t nEle = daughters->GetEntries();
    while(jpsi = static_cast<AliDielectronPair*>(nextPair())){
      AliKFParticle& ele = jpsi->GetKFFirstDaughter();
      //new ((*daughters)[nEle++]) AliKFParticle(ele);
      cout << ele.GetPt() << "  " << ele.GetEta() << "  " << ele.GetPhi() << endl;
      ele = jpsi->GetKFSecondDaughter();
      //new ((*daughters)[nEle++]) AliKFParticle(ele);
      cout << ele.GetPt() << "  " << ele.GetEta() << "  " << ele.GetPhi() << endl;
    }
  }
}
