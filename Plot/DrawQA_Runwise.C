/*
 *
 * ROOT Macro for post-processing of QA result
 *
 * */

// Drawing style
const Int_t LINE_WIDTH = 1;
const Float_t MARKER_SIZE = 1.5;
Bool_t gIsMC = kFALSE;

enum RunwiseVar{
  kNEventTotal,
  kNEventMB,
  kNEventEGA,
  kNEventEG1,
  kNEventEG2,
  kNEventDG1,
  kNEventDG2,
  kNEventFilter,
  kNEventListN,
  // Filter QA - Event
  kVtxZ = kNEventListN,
  kNTracks,
  kNElectrons,
  kNSPDtracklets,
  kV0,
  // Filter QA - Track (EMCal_loose)
  kElectron_Pt,
  kElectron_Eta,
  kElectron_Phi,
  kEMCalE,
  // PWGJEQA - EMCEGA
  kNTracks_Jet,
  kNCaloClusters,
  // End
  kVarN
};
// Name, Title, Label
const char* HISTO_SETUP[kVarN][3] = {
  {"hEvTot", "FullAOD", "N_{events}"},
  {"hEvMB", "MB/INT7", "N_{events}"},
  {"hEvEGA", "EGA", "N_{events}"},
  {"hEvEG1", "EG1", "N_{events}"},
  {"hEvEG2", "EG2", "N_{events}"},
  {"hEvDG1", "DG1", "N_{events}"},
  {"hEvDG2", "DG2", "N_{events}"},
  {"hEvNano", "Nano", "N_{events}"},
  {"hVtxZ","Primary Vertex Z","<Z_{vtx}> [cm]"},
  {"hNTrk","Number of tracks (no cuts)","<N_{tracks}>"},
  {"hNEle","Number of electrons (with cuts: EMCal_loose)","<N_{electrons}>"},
  {"hSPD","Multiplicity by SPD tracklets (|#eta|<1.0)","<N_{SPDtracklets}>"},
  {"hV0","Multiplicity by V0 amplitude","<V0 amp.>"},
  {"hElePt","Electrons/Positrons p_{T}","<p^{e}_{T}>"},
  {"hEleEta","Electrons/Positrons #eta","<#eta_{e}>"},
  {"hElePhi","Electrons/Positrons #phi","<#phi_{e}>"},
  {"hEleE","EMCal cluster energy","<E_{cluster}>"},
  {"hNTrkJet","Number of tracks (p_{T}>0.15 GeV/c for jet finder)","<N_{tracks}>"},
  {"hNCaloCls","Number of calo/EMCal clusters (E_{cluster} > 0.3 GeV)","<N_{cluster}>"}
};

TH1* histos[kVarN] = {NULL};
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

Int_t SelectMarkerStyle(Bool_t newSets = kFALSE){
  static const Int_t MARKER_NUMBER = 10;
  static const Int_t MARKER_SET[MARKER_NUMBER] = {20, 25, 34, 22, 27, 29, 21, 28, 33, 31};
  static Int_t CURRENT_INDEX = -1;
  if(!newSets)
    CURRENT_INDEX = (CURRENT_INDEX+1) % MARKER_NUMBER;
  else
    CURRENT_INDEX = 0;
  return MARKER_SET[CURRENT_INDEX];
}

const char* ObjTitle(TString str){
  if(gIsMC) str = str + "_MC";
  return str.Data();
}

void InitHistograms(Int_t N_RUNS){
  for(Int_t i = 0; i < kVarN; i++){
    TString hTitle = HISTO_SETUP[i][0];
    if(gIsMC) hTitle = Form("%s_MC",hTitle.Data());
    histos[i] = new TH1F(
        hTitle.Data(),
        Form("%s;Run No.;%s",HISTO_SETUP[i][1],HISTO_SETUP[i][2]),
        N_RUNS,0,N_RUNS);
  }
}

Bool_t FillHistogramValue(const char* hLabel, TH1* hQA, TH1* hSource){
  if(!hSource) return kFALSE;
  Int_t binNo = hQA->Fill(hLabel,hSource->GetMean());
  hQA->SetBinError(binNo, hSource->GetMeanError());
  return kTRUE;
}

void FillHistograms(TFile* anaResult, TString runNumber){
  // Dielectron filter QA - kEMCEGA + EMCal_loose
   // Number of events with triggers
  
  TH1* filterEventStat = (TH1*)(anaResult->Get("PWGDQ_dielectronFilter/hEventStat"));
  histos[kNEventTotal]->Fill(runNumber.Data(),filterEventStat->GetBinContent(1));
  histos[kNEventEGA]->Fill(runNumber.Data(),filterEventStat->GetBinContent(5));
   // Number of dielectron filtered events
  auto filterQA = (THashList*)(anaResult->Get("PWGDQ_dielectronFilter/jpsi_FilterQA"));
  filterQA->SetOwner(kTRUE);
  auto eventQA = (THashList*)(filterQA->FindObject("Event"));
  eventQA->SetOwner(kTRUE);
  auto nPair = (TH1*)(eventQA->FindObject("Npairs"));
  
  Int_t nEvNano = nPair->GetEntries() - nPair->GetBinContent(1);
  
  histos[kNEventFilter]->Fill(runNumber.Data(),nEvNano);
  
  cout << "N events (Before PS):\t" << filterEventStat->GetBinContent(1) << endl;
  cout << "N events (EMCEGA):\t" << filterEventStat->GetBinContent(5) << endl;
  cout << "N events (NanoAOD):\t" << nEvNano << endl;
  
   // Event Variables
  FillHistogramValue(runNumber.Data(), histos[kNTracks], (TH1*)(eventQA->FindObject("kNTrk")));
  FillHistogramValue(runNumber.Data(), histos[kNSPDtracklets], (TH1*)(eventQA->FindObject("kNaccTrcklts10Corr")));
  FillHistogramValue(runNumber.Data(), histos[kNElectrons], (TH1*)(eventQA->FindObject("Nelectrons")));
  FillHistogramValue(runNumber.Data(), histos[kVtxZ], (TH1*)(eventQA->FindObject("VtxZ")));
  FillHistogramValue(runNumber.Data(), histos[kV0], (TH1*)(eventQA->FindObject("kMultV0")));

    // Filtered Electrons
  auto trackQA = (THashList*)(filterQA->FindObject("Track_ev1+"));
  trackQA->SetOwner(kTRUE);
  auto trackQA_tmp = (THashList*)(filterQA->FindObject("Track_ev1-"));
  trackQA_tmp->SetOwner(kTRUE);
  auto elePt = (TH1*)(trackQA->FindObject("Pt"));
  elePt->Add((TH1*)(trackQA_tmp->FindObject("Pt")));
  FillHistogramValue(runNumber.Data(), histos[kElectron_Pt], elePt);
 
  auto eleEtaPhi = (TH2*)(trackQA->FindObject("Eta_Phi"));
  eleEtaPhi->Add((TH2*)(trackQA_tmp->FindObject("Eta_Phi")));
  FillHistogramValue(runNumber.Data(), histos[kElectron_Eta], eleEtaPhi->ProjectionX());
  FillHistogramValue(runNumber.Data(), histos[kElectron_Phi], eleEtaPhi->ProjectionY());
  
  auto eleE = (TH1*)(trackQA->FindObject("EMCalE"));
  eleE->Add((TH1*)(trackQA_tmp->FindObject("EMCalE")));
  FillHistogramValue(runNumber.Data(), histos[kEMCalE], eleE);
  
  
  delete filterEventStat;
  delete filterQA;
  
  // PWGJEQA - kEMCEGA, trackPt>0.15, caloE>0.30
  auto jetQA = ((AliEmcalList*)(anaResult->Get("AliAnalysisTaskPWGJEQA_tracks_caloClusters_emcalCells_histos")));
  jetQA->SetOwner(kTRUE);
  auto trigClass = (TH1*)(jetQA->FindObject("fHistTriggerClasses"));
  // Pt, eta, phi, type, dPt/Pt
  auto jetTrackQA = (THnSparseT<TArrayD>*)(jetQA->FindObject("tracks"));
  // E, eta, phi, type
  //auto jetCaloQA = (THnSparseT<TArrayD>*)(jetQA->FindObject("caloClusters")->FindObject("clusterObservables"));
  // Ntracks, Pt_leading, Nclusters, E_leading
  auto jetEventQA = (THnSparseT<TArrayD>*)(jetQA->FindObject("eventQA"));
  FillHistogramValue(runNumber.Data(), histos[kNTracks_Jet], jetEventQA->Projection(0));
  FillHistogramValue(runNumber.Data(), histos[kNCaloClusters], jetEventQA->Projection(2));
  // fHistCellEnergy, fProfCellAbsIdEnergy, fHistCellTime, fProfCellAbsIdTime, fHistCellEvsTime
  //auto jetCellQA = (THashList*)(jetQA->FindObject("emcalCells"));
  // eta, phi, Pt, Pt_leading
  //auto jets02QA = (THnSparseT<TArrayD>*)(jetQA->FindObject("Jet_AKTChargedR020_tracks_pT0150_pt_scheme")->FindObject("fHistJetObservables"));
  //auto jets04QA = (THnSparseT<TArrayD>*)(jetQA->FindObject("Jet_AKTChargedR040_tracks_pT0150_pt_scheme")->FindObject("fHistJetObservables"));

  delete jetQA;
  
  if(gIsMC) return;
  /*
    // Dielectron QA - INT7/EG1/EG2/DG1/DG2 + EMCal_strict/RAW
  auto eventQA_MB = (THashList*)(anaResult->Get("PWGDQ_dielectron_MultiDie_EMCal_0/cjahnke_QA_0")->FindObject("RAW")->FindObject("Event"));
  auto hEventStat_MB = (TH1*)(anaResult->Get("PWGDQ_dielectron_MultiDie_EMCal_0/hEventStat"));
  histos[kNEventMB]->Fill(runNumber.Data(),hEventStat_MB->GetBinContent(7));
  auto hEventStat = (TH1*)(anaResult->Get("PWGDQ_dielectron_MultiDie_EMCal_3/hEventStat"));
  histos[kNEventEG1]->Fill(runNumber.Data(),hEventStat->GetBinContent(7));
  hEventStat = (TH1*)(anaResult->Get("PWGDQ_dielectron_MultiDie_EMCal_4/hEventStat"));
  histos[kNEventEG2]->Fill(runNumber.Data(),hEventStat->GetBinContent(7));
  hEventStat = (TH1*)(anaResult->Get("PWGDQ_dielectron_MultiDie_EMCal_30/hEventStat"));
  histos[kNEventDG1]->Fill(runNumber.Data(),hEventStat->GetBinContent(7));
  hEventStat = (TH1*)(anaResult->Get("PWGDQ_dielectron_MultiDie_EMCal_40/hEventStat"));
  histos[kNEventDG2]->Fill(runNumber.Data(),hEventStat->GetBinContent(7));
  */
}

void DrawHistograms(TString outputFileName = "OutputQA.root"){
  if(gIsMC) outputFileName = "OutputQA_MC.root";
  TFile* outputQA = new TFile(outputFileName,"RECREATE");
  
  TCanvas* cNEvent = new TCanvas(ObjTitle("cEv"),"Event Number QA");
  cNEvent->SetLogy(kTRUE);
  if(!histos[0]) return;
  // Drawing style
  gStyle->SetOptLogy(kTRUE);
  gStyle->SetOptStat(kFALSE);
  gStyle->SetOptTitle(kFALSE);

  histos[0]->SetStats(kFALSE);
  Int_t color = SelectColor();
  histos[0]->SetLineColor(color);
  histos[0]->SetMarkerColor(color);
  histos[0]->SetMarkerStyle(SelectMarkerStyle());
  histos[0]->SetMarkerSize(MARKER_SIZE);
  histos[0]->Draw("E0");
  histos[0]->GetYaxis()->SetRangeUser(1,5*histos[0]->GetMaximum());

  for(Int_t i = 1; i < kNEventListN; i++){
    histos[i]->SetStats(kFALSE);
    Int_t color = SelectColor();
    histos[i]->SetLineColor(color);
    histos[i]->SetMarkerColor(color);
    histos[i]->SetMarkerStyle(SelectMarkerStyle());
    histos[i]->SetMarkerSize(MARKER_SIZE);
    histos[i]->Draw("same E0");
  }// Loop - histos
  cNEvent->Write();

  TCanvas* cRunwise = new TCanvas(ObjTitle("cQA"),"Runwise QA",800,600);
  cRunwise->Divide(4,4);
  cRunwise->SetLogy(kFALSE); 
  gStyle->SetOptTitle(kTRUE);
  for(Int_t i = kNEventListN; i < kVarN; i++){
    cRunwise->cd(i-kNEventListN+1);
    gPad->SetLogy(kFALSE);
    Int_t color = SelectColor(kTRUE);
    if(gIsMC) color = kRed;
    histos[i]->SetLineColor(color);
    histos[i]->SetMarkerColor(color);
    histos[i]->SetStats(kFALSE);
    histos[i]->SetMarkerStyle(SelectMarkerStyle(kTRUE));
    histos[i]->SetMarkerSize(MARKER_SIZE);
    histos[i]->Draw("E0");
    // Set value range
    Double_t mean = histos[i]->GetBinContent(1);
    Double_t err = histos[i]->GetBinError(1);
    Double_t ymin = 0.;
    Double_t ymax = 2.5 * mean;
    if(i == kVtxZ){
      ymin = -1.;
      ymax = 1.;
    }
    histos[i]->GetYaxis()->SetRangeUser(ymin, ymax);
  }
  cRunwise->Write();

  for(Int_t i = 0; i < kVarN; i++)
    histos[i]->Write();

}

void DrawQA_Runwise(
  Bool_t isMC = kTRUE,
  TString output_dir = "output_mc"
){

  gIsMC = isMC;

  TString ret = gSystem->GetFromPipe(Form("ls %s | grep 2",output_dir.Data()));


  TObjArray* runlist = ret.Tokenize("\n");
  Int_t N_RUNS = runlist->GetEntriesFast();
  // Init histogram for run-wise QA
  InitHistograms(N_RUNS);
  
  // Loop for result by run
  for(Int_t i = 0; i < runlist->GetEntriesFast(); i++){
    TString runDir = runlist->At(i)->GetName();
    TString runNumber = runDir.Strip(TString::kLeading,'0');
    cout << "[+] Run " << runNumber << " Found. Start processing : " << endl;
    TFile* anaResult = new TFile(Form("%s/%s/AnalysisResults.root",output_dir.Data(),runDir.Data()));

    FillHistograms(anaResult, runNumber);
    
    anaResult->Close();
    delete anaResult;
  }// Loop runlist

  // Plot and output
  DrawHistograms();
}
