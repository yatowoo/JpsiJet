/*
 *
 * ROOT Macro for post-processing of QA result
 *
 * */

// Drawing style
const Int_t LINE_WIDTH = 1;
const Int_t MARKER_SIZE = 2;

void DrawQA(
  TString output_dir = "/data2/ytwu/LOG/ALICE/JpsiJet_QAFilter_test_190608"
){

  TString ret = gSystem->GetFromPipe(Form("ls %s/OutputAOD/ | grep 000",output_dir.Data()));


  TObjArray* runlist = ret.Tokenize("\n");
  Int_t N_RUNS = runlist->GetEntriesFast();
  // Init histogram for run-wise QA
  TH1* hEventTotal = new TH1F("hEvTot","Event number (Total);Run Number;#events", N_RUNS, 0, N_RUNS);
  TH1* hEventEGA = new TH1F("hEvEGA","Event number (kEMCEGA);Run Number;#events", N_RUNS, 0, N_RUNS);
  TH1* hEventMB = new TH1F("hEvMB","Event number (kINT7);Run Number;#events", N_RUNS, 0, N_RUNS);
  
  // Loop for result by run
  for(Int_t i = 0; i < runlist->GetEntriesFast(); i++){
    TString runDir = runlist->At(i)->GetName();
    TString runNumber = runDir.Strip(TString::kLeading,'0');
    cout << "[+] Run " << runNumber << " Found. Start processing : " << endl;
    TFile* anaResult = new TFile(Form("%s/OutputAOD/%s/AnalysisResults.root",output_dir.Data(),runDir.Data()));
    // Dielectron filter QA - kEMCEGA + EMCal_loose
    TH1* filterEventStat = (TH1*)(anaResult->Get("PWGDQ_dielectronFilter/hEventStat"));
    auto eventQA = (THashList*)(anaResult->Get("PWGDQ_dielectronFilter/jpsi_FilterQA")->FindObject("Event"));
    auto nTracks = (TH1*)(eventQA->FindObject("kNTrk"));
    auto nSPDtracklets = (TH1*)(eventQA->FindObject("kNaccTrcklts10Corr"));
    auto nEletrons = (TH1*)(eventQA->FindObject("Ntracks"));
    auto vtxZ = (TH1*)(eventQA->FindObject("VtxZ"));
    auto vzero= (TH1*)(eventQA->FindObject("kMultV0"));
    cout << "Before PS:\t\t" << filterEventStat->GetBinContent(1) << endl;
    hEventTotal->Fill(runNumber.Data(),filterEventStat->GetBinContent(1));
    cout << "After PS:\t\t" << filterEventStat->GetBinContent(2) << endl;
    cout << "After Event Filter:\t" << filterEventStat->GetBinContent(4) << endl;
    cout << "After Pileup Rejection:\t" << filterEventStat->GetBinContent(5) << endl;
    hEventEGA->Fill(runNumber.Data(),filterEventStat->GetBinContent(5));
    cout << "<N_tracks>:\t\t" << nTracks->GetMean() << endl;
    cout << "<N_SPDtracklets>:\t" << nSPDtracklets->GetMean() << endl;
    cout << "<N_electrons>:\t\t" << nEletrons->GetMean() << endl; 
    cout << "<Z_vtx>:\t\t" << vtxZ->GetMean() << endl; 
    cout << "RMS_Zvtx:\t\t" << vtxZ->GetRMS() << endl; 
    cout << "<Multiplicity_V0>:\t" << vzero->GetMean() << endl; 
    // Dielectron QA - INT7/EG1/EG2/DG1/DG2 + EMCal_strict/RAW
    auto eventQA_MB = (THashList*)(anaResult->Get("PWGDQ_dielectron_MultiDie_EMCal_0/cjahnke_QA_0")->FindObject("RAW")->FindObject("Event"));
    auto hEventStat_MB = (TH1*)(anaResult->Get("PWGDQ_dielectron_MultiDie_EMCal_0/hEventStat"));
    hEventMB->Fill(runNumber.Data(),hEventStat_MB->GetBinContent(7));

    // PWGJEQA - kEMCEGA, trackPt>0.15, caloE>0.30
    auto jetQA = ((AliEmcalList*)(anaResult->Get("AliAnalysisTaskPWGJEQA_tracks_caloClusters_emcalCells_histos")));
    auto trigClass = (TH1*)(jetQA->FindObject("fHistTriggerClasses"));
      // Pt, eta, phi, type, dPt/Pt
    auto jetTrackQA = (THnSparseT<TArrayD>*)(jetQA->FindObject("tracks"));
      // E, eta, phi, type
    auto jetCaloQA = (THnSparseT<TArrayD>*)(jetQA->FindObject("caloClusters")->FindObject("clusterObservables"));
      // Ntracks, Pt_leading, Nclusters, E_leading
    auto jetEventQA = (THnSparseT<TArrayD>*)(jetQA->FindObject("eventQA"));
      // fHistCellEnergy, fProfCellAbsIdEnergy, fHistCellTime, fProfCellAbsIdTime, fHistCellEvsTime
    auto jetCellQA = (THashList*)(jetQA->FindObject("emcalCells"));
      // eta, phi, Pt, Pt_leading
    auto jets02QA = (THnSparseT<TArrayD>*)(jetQA->FindObject("Jet_AKTChargedR020_tracks_pT0150_pt_scheme")->FindObject("fHistJetObservables"));
    auto jets04QA = (THnSparseT<TArrayD>*)(jetQA->FindObject("Jet_AKTChargedR040_tracks_pT0150_pt_scheme")->FindObject("fHistJetObservables"));
    
    anaResult->Close();
  }// Loop runlist

  // Drawing style
  gStyle->SetOptLogy(kTRUE);
  gStyle->SetOptStat(kFALSE);
  gStyle->SetOptTitle(kFALSE);

  hEventTotal->SetStats(kFALSE);
  hEventTotal->SetLineColor(kBlack);
  hEventTotal->SetMarkerColor(kBlack);
  hEventTotal->SetMarkerStyle(20);
  hEventTotal->SetMarkerSize(MARKER_SIZE);
  hEventTotal->Draw("E0");
  hEventTotal->GetYaxis()->SetRangeUser(1,5*hEventTotal->GetMaximum());

  hEventEGA->SetStats(kFALSE);
  hEventEGA->SetLineColor(kBlue);
  hEventEGA->SetMarkerColor(kBlue);
  hEventEGA->SetMarkerStyle(26);
  hEventEGA->SetMarkerSize(MARKER_SIZE);
  hEventEGA->Draw("same E0");

  hEventMB->SetStats(kFALSE);
  hEventMB->SetLineColor(kRed);
  hEventMB->SetMarkerColor(kRed);
  hEventMB->SetMarkerStyle(34);
  hEventMB->SetMarkerSize(MARKER_SIZE);
  hEventMB->Draw("same E0");
}
