/*
 *
 * ROOT Macro for post-processing of QA result
 *
 * */

void DrawQA(
  TString output_dir = "/data2/ytwu/LOG/ALICE/JpsiJet_QAFilter_test_190608"
){

  TString ret = gSystem->GetFromPipe(Form("ls %s/OutputAOD/ | grep 000",output_dir.Data()));

  TObjArray* runlist = ret.Tokenize("\n");
  for(Int_t i = 0; i < runlist->GetEntriesFast(); i++){
    TString runNumber = runlist->At(i)->GetName();
    cout << "[+] Run " << runNumber << " Found. Start processing : " << endl;
    TFile* anaResult = new TFile(Form("%s/OutputAOD/%s/AnalysisResults.root",output_dir.Data(),runNumber.Data()));
    TH1* filterEventStat = (TH1*)(anaResult->Get("PWGDQ_dielectronFilter/hEventStat"));
    auto eventQA = (THashList*)(anaResult->Get("PWGDQ_dielectronFilter/jpsi_FilterQA")->FindObject("Event"));
    auto nTracks = (TH1*)(eventQA->FindObject("kNTrk"));
    auto nSPDtracklets = (TH1*)(eventQA->FindObject("kNaccTrcklts10Corr"));
    auto nEletrons = (TH1*)(eventQA->FindObject("Ntracks"));
    auto vtxZ = (TH1*)(eventQA->FindObject("VtxZ"));
    auto vzero= (TH1*)(eventQA->FindObject("kMultV0"));
    cout << "Before PS:\t\t" << filterEventStat->GetBinContent(1) << endl;
    cout << "After PS:\t\t" << filterEventStat->GetBinContent(2) << endl;
    cout << "After Event Filter:\t" << filterEventStat->GetBinContent(4) << endl;
    cout << "After Pileup Rejection:\t" << filterEventStat->GetBinContent(5) << endl;
    cout << "<N_tracks>:\t\t" << nTracks->GetMean() << endl;
    cout << "<N_SPDtracklets>:\t" << nSPDtracklets->GetMean() << endl;
    cout << "<N_electrons>:\t\t" << nEletrons->GetMean() << endl; 
    cout << "<Z_vtx>:\t\t" << vtxZ->GetMean() << endl; 
    cout << "RMS_Zvtx:\t\t" << vtxZ->GetRMS() << endl; 
    cout << "<Multiplicity_V0>:\t" << vzero->GetMean() << endl; 
    
    anaResult->Close();
  }// Loop runlist
}
