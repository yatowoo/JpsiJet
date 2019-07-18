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
  TH1* htmp = (TH1*)(dieQA->FindObject(cutDef)->FindObject("Track_ev1+")->FindObject("EMCalE"));
  TH1* hE = (TH1*)(htmp->Clone(Form("hEMCalE_%s",trigName)));
  hE->SetTitle(trigName);
  htmp = (TH1*)(dieQA->FindObject(cutDef)->FindObject("Track_ev1-")->FindObject("EMCalE"));
  hE->Add(htmp);
  // Get event number
  htmp = (TH1*)(dieQA->FindObject("RAW")->FindObject("Event")->FindObject("VtxZ"));
  Int_t nEvent = htmp->GetEntries();
  Double_t normFactor = nEvent * hE->GetBinWidth(1);
  fcn->SetParameter(0,normFactor);
  hE->Divide(fcn);
  for(Int_t i = 0; i <= hE->GetNbinsX(); i++)
    hE->SetBinError(i, hE->GetBinError(i) / TMath::Sqrt(normFactor));

  hE->SetYTitle("1/N_{ev} dN/dE_{cls}");
  hE->SetMarkerColor(SelectColor());
  hE->SetMarkerStyle(20);

  hE->SetDirectory(NULL);

  return hE;
}

void EMCalRF(){
  if(!fcn) fcn = new TF1("fNorm","[0]", 0, 100);  
  TCanvas* cEMCal = new TCanvas("cE","QA for EMCal", 1600, 600);
  gStyle->SetOptStat(kFALSE);
  cEMCal->Divide(2);
  TH1* hMB = GetEMCalE((TList*)(anaResult->Get("PWGDQ_dielectron_MultiDie_EMCal_0/cjahnke_QA_0")),"MB");
  TH1* hEG1 = GetEMCalE((TList*)(anaResult->Get("PWGDQ_dielectron_MultiDie_EMCal_3/cjahnke_QA_3")),"EG1");
  TH1* hEG2 = GetEMCalE((TList*)(anaResult->Get("PWGDQ_dielectron_MultiDie_EMCal_4/cjahnke_QA_4")),"EG2");
  TH1* hDG1 = GetEMCalE((TList*)(anaResult->Get("PWGDQ_dielectron_MultiDie_EMCal_30/cjahnke_QA_30")),"DG1");
  TH1* hDG2 = GetEMCalE((TList*)(anaResult->Get("PWGDQ_dielectron_MultiDie_EMCal_40/cjahnke_QA_40")),"DG2");
  cEMCal->cd(1);
  hMB->Draw("E");
  hEG1->Draw("same E");
  hEG2->Draw("same E");
  hDG1->Draw("same E");
  hDG2->Draw("same E");
  hMB->GetYaxis()->SetRangeUser(1e-6,30);
  gPad->SetLogy(kTRUE);
  gPad->BuildLegend();
  hMB->SetTitle("EMCal cluster energy distribution");
  gPad->Update();
  // Rejection Factor
  auto RF1 = (TH1*)(hEG1->Clone("hRF1"));
  RF1->Divide(hMB);
  RF1->SetTitle("EG1/MB");
  auto RF2 = (TH1*)(hEG2->Clone("hRF2"));
  RF2->Divide(hMB);
  RF2->SetTitle("EG2/MB");
  auto RF10 = (TH1*)(hDG1->Clone("hRF10"));
  RF10->Divide(hMB);
  RF10->SetTitle("DG1/MB");
  auto RF20 = (TH1*)(hDG2->Clone("hRF20"));
  RF20->Divide(hMB);
  RF20->SetTitle("DG2/MB");
  auto RF12 = (TH1*)(hEG1->Clone("hRF12"));
  RF12->Divide(hEG2);
  RF12->SetTitle("EG1/EG2");
  RF12->SetMarkerStyle(24);
  auto RF120 = (TH1*)(hDG1->Clone("hRF120"));
  RF120->Divide(hEG2);
  RF120->SetTitle("DG1/DG2");
  RF120->SetMarkerStyle(24);
  cEMCal->cd(2);
  RF1->Draw("E");
  RF2->Draw("same E");
  RF10->Draw("same E");
  RF20->Draw("same E");
  RF12->Draw("same E");
  RF120->Draw("same E");
  RF1->GetYaxis()->SetRangeUser(1e-2,1000);
  gPad->SetLogy(kTRUE);
  gPad->BuildLegend();
  RF1->SetTitle("Rejection factor of EMCal trigger");
}

void DrawQA(
    const char* anaResultFileName = "AnalysisResults.root"){
  anaResult = new TFile(anaResultFileName);
  
  anaResult->ls();

  EMCalRF();

  //anaResult->Close();
}
