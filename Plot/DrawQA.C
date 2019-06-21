// DrawQA with AnalysisResult.root
//
// Dielectron/Jpsi, Jet, EMCal RF
  
// Global variable
TFile* anaResult = NULL;

TF1* fcn = NULL;

Int_t SelectColor(Bool_t newSets = kFALSE){
  static const Int_t COLOR_NUMBER = 9;
  static const Int_t COLOR_SET[COLOR_NUMBER] = {kBlack, kRed, kBlue, kGreen+3, kOrange, kViolet, kCyan, kOrange-6, kPink};
  static Int_t CURRENT_INDEX = -1;
  if(!newSets)
    CURRENT_INDEX = (CURRENT_INDEX+1) % COLOR_NUMBER;
  else
    CURRENT_INDEX = 0;
  return COLOR_SET[CURRENT_INDEX];
}

TH1* GetEMCalE(TList* dieQA,const char* trigName, const char* cutDef = "RAW"){
  // Get EMCal_E with +/- tracks
  TH1* htmp = (TH1*)(dieQA->FindObject(cutDef)->FindObject("Track_ev1+")->FindObject("EMCal_E"));
  TH1* hE = (TH1*)(htmp->Clone(Form("hEMCalE_%s",trigName)));
  hE->SetTitle(trigName);
  htmp = (TH1*)(dieQA->FindObject(cutDef)->FindObject("Track_ev1-")->FindObject("EMCal_E"));
  hE->Add(htmp);
  // Get event number
  htmp = (TH1*)(dieQA->FindObject(cutDef)->FindObject("Event")->FindObject("VtxZ"));
  Int_t nEvent = htmp->GetEntries();
  fcn->SetParameter(0,nEvent * hE->GetBinWidth(1));
  hE->Divide(fcn);
  hE->SetYTitle("1/N_{ev} dN/dE_{cls}");
  hE->SetMarkerColor(SelectColor());
  hE->SetMarkerStyle(20);

  hE->SetDirectory(NULL);

  return hE;
}

void EMCalRF(){
  if(!fcn) fcn = new TF1("fNorm","[0]", 0, 100);  
  TH1* hMB = GetEMCalE((TList*)(anaResult->Get("PWGDQ_dielectron_MultiDie_EMCal_0/cjahnke_QA_0")),"MB");
  TH1* hEG1 = GetEMCalE((TList*)(anaResult->Get("PWGDQ_dielectron_MultiDie_EMCal_3/cjahnke_QA_3")),"EG1");
  TH1* hEG2 = GetEMCalE((TList*)(anaResult->Get("PWGDQ_dielectron_MultiDie_EMCal_4/cjahnke_QA_4")),"EG2");
  TH1* hDG1 = GetEMCalE((TList*)(anaResult->Get("PWGDQ_dielectron_MultiDie_EMCal_30/cjahnke_QA_30")),"DG1");
  TH1* hDG2 = GetEMCalE((TList*)(anaResult->Get("PWGDQ_dielectron_MultiDie_EMCal_40/cjahnke_QA_40")),"DG2");
  hMB->Draw("P");
  hEG1->Draw("same P");
  hEG2->Draw("same P");
  hDG1->Draw("same P");
  hDG2->Draw("same P");
  hMB->GetYaxis()->SetRangeUser(1e-6,30);
  gPad->SetLogy(kTRUE);
}

void DrawQA(
    const char* anaResultFileName = "~/LOG/ALICE/JpsiJet_QAFilter_16l_190608/OutputAOD/AnalysisResults.root"){
  anaResult = new TFile(anaResultFileName);
  
  anaResult->ls();

  EMCalRF();

  anaResult->Close();
}
