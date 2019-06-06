/* PP 13TeV Jet QA
* Refer:
  - LEGO train - JE_EMC_pp
  - runEMCalJetSampleTask.C
* Configuration:
*   - JetFinder: AddTaskEmcalJet, Wagon-PPJetFinder_charged_AKT_02/04
      - nTrack      = "usedefault" // name_of_track_branch
      - nClusters   = "" // name_of_cluster_branch
      - jetAlgo     = AliJetContainer::antikt_algorithm // EJetAlgo_t
      - radius      = 0.2 / 0.4
      - jetType     = AliJetContainer::kChargedJet // EJetType_t
      - minTrPt     = 0.15 // Track pt cut (GeV/c)
      - minClPt     = 0.30 // Cluster energy cut (GeV)
      - ghostArea   = 0.01
      - reco        = AliJetContainer::pt_scheme // ERecoScheme_t
      - tag         = "Jet"
      - minJetPt    = 1.
      - lockTask    = kFALSE
      - bFillGhosts = kFALSE
*   - Rho: AddTaskRhoSparse, Wagon-HMRhoTask_02/04
      ? Parameter in wagon can not match current macro
      - runEMCalJetSampleTask.C, AliAnalysisTaskRho::AddTaskRhoNew
*   - DeltaPt: AddTaskDeltaPt, Wagon-HMDeltaPt_02/04
      -
* Main task:
*   - SpectraQA: AliAnalysisTaskEmcalJetSpectraQA
*   - PWGJEQA: AliAnalysisTaskPWGJEQA
      Wagon: 	JE_QA_pp_Leticia (Last run: 20190507)
*/

AliAnalysisTaskDeltaPt* AddDeltaPt(Double_t jetRadius, TString jetName, TString rhoName){
  gROOT->Load("$ALICE_PHYSICS/PWGJE/EMCALJetTasks/macros/AddTaskDeltaPt.C");
  AliAnalysisTaskDeltaPt* jetDeltaPt = AddTaskDeltaPt("tracks", "", jetName.Data(),"","", "", "tracks","", rhoName.Data(), jetRadius, 0.01, 0.15, 0.3, "TPC", "AliAnalysisTaskDeltaPt");

  jetDeltaPt->SelectCollisionCandidates(AliVEvent::kINT7);
  jetDeltaPt->SetForceBeamType(AliAnalysisTaskEmcal::kpp);
  jetDeltaPt->SetUseNewCentralityEstimation(kTRUE);
  jetDeltaPt->SetNCentBins(1);
  jetDeltaPt->SetUseAliAnaUtils(kTRUE);
  jetDeltaPt->SetUseSPDTrackletVsClusterBG(kTRUE);
  jetDeltaPt->SetZvertexDiffValue(0.5);
  jetDeltaPt->SetNeedEmcalGeom(kFALSE);
  jetDeltaPt->SetRCperEvent(100);
  jetDeltaPt->SetJetMinRC2LJ(0.2);
  jetDeltaPt->SetConeRadius(0.2);

  return jetDeltaPt;
}

AliAnalysisTaskRho* AddRho(Double_t jetRadius, TString jetName, TString rhoName){
  AliAnalysisTaskRho* jetRho = AliAnalysisTaskRho::AddTaskRhoNew("tracks","", rhoName, jetRadius, AliEmcalJet::kTPCfid, AliJetContainer::kChargedJet, kTRUE, AliJetContainer::pt_scheme, rhoName);

  jetRho->SetExcludeLeadJets(2);
  jetRho->SetOutRhoName(rhoName);
  jetRho->SelectCollisionCandidates(AliVEvent::kINT7);
  jetRho->SetForceBeamType(AliAnalysisTaskEmcal::kpp);
  jetRho->SetUseNewCentralityEstimation(kTRUE);

  jetRho->SetUseAliAnaUtils(kTRUE);
  jetRho->SetUseSPDTrackletVsClusterBG(kTRUE);
  jetRho->SetZvertexDiffValue(0.5);
  jetRho->SetNeedEmcalGeom(kFALSE);

  return jetRho;
}

AliEmcalJetTask* AddJetFinder(Double_t jetRadius){
  AliEmcalJetTask* jetFinder = AliEmcalJetTask::AddTaskEmcalJet("usedefault", "", AliJetContainer::antikt_algorithm, jetRadius, AliJetContainer::kChargedJet, 0.15, 0.3, 0.01, AliJetContainer::pt_scheme, "Jet", 1., kFALSE, kFALSE);

  jetFinder->SetForceBeamType(AliAnalysisTaskEmcal::kpp);
  jetFinder->SelectCollisionCandidates(AliVEvent::kINT7);
  jetFinder->SetUseAliAnaUtils(kTRUE);
  jetFinder->SetZvertexDiffValue(0.5);
  jetFinder->SetNeedEmcalGeom(kFALSE);

  TString jetName = Form("Jet_AKTChargedR%03d_tracks_pT0150_pt_scheme",int(jetRadius*100));
  TString rhoName = Form("Rho%02d",int(jetRadius*10));
  AddRho(jetRadius, jetName, rhoName);
  AddDeltaPt(jetRadius, jetName, rhoName);

  return jetFinder;
}

AliJetContainer* AddContainer(AliAnalysisTaskEmcalJet* jetTask, Double_t jetRadius){
  AliJetContainer* jetContCh = jetTask->AddJetContainer(AliJetContainer::kChargedJet, AliJetContainer::antikt_algorithm, AliJetContainer::pt_scheme, 0.2, AliJetContainer::kTPCfid,  "tracks", "", "Jet");
  jetContCh->SetRhoName(Form("Rho%02d",int(jetRadius*10)));
  jetContCh->SetPercAreaCut(0.6);

  return jetContCh;
}

void AddTaskJetQA(UInt_t kPhysSel = AliVEvent::kINT7){

  // Task - Jet finder 
  AliEmcalJetTask * jetFinder02 = AddJetFinder(0.2);
  AliEmcalJetTask * jetFinder04 = AddJetFinder(0.4);
  
  // Task - PWGJE QA (Event, Track, Calo, Jet)
  AliAnalysisTaskPWGJEQA* jetQA = reinterpret_cast<AliAnalysisTaskPWGJEQA*>(gInterpreter->ExecuteMacro("$ALICE_PHYSICS/PWGJE/EMCALJetTasks/macros/AddTaskPWGJEQA.C(\"usedefault\",\"usedefault\",\"usedefault\",\"\")"));
  
  AddContainer(jetQA, 0.2);
  AddContainer(jetQA, 0.4);

  jetQA->SelectCollisionCandidates(kPhysSel);
  jetQA->SetUseNewCentralityEstimation(kTRUE);
  jetQA->SetNCentBins(1);
  jetQA->SetForceBeamType(AliAnalysisTaskEmcal::kpp);
  jetQA->SetUseAliEventCuts(1);


  // Task - Jet Spectra QA
  AliAnalysisTaskEmcalJetSpectraQA* jetSpectraQA = AliAnalysisTaskEmcalJetSpectraQA::AddTaskEmcalJetSpectraQA("usedefault", "", 0.15, 0.30, "");

  AddContainer(jetSpectraQA, 0.2);
  AddContainer(jetSpectraQA, 0.4);

  jetSpectraQA->SetHistoType(AliAnalysisTaskEmcalJetSpectraQA::kTH2);
  jetSpectraQA->SetPtBin(1.,200.);
  jetSpectraQA->SelectCollisionCandidates(kPhysSel);
  jetSpectraQA->SetForceBeamType(AliAnalysisTaskEmcalLight::kpp);
  jetSpectraQA->SetCentralityEstimation(1);
  jetSpectraQA->SetCentRange(0, 100);
  jetSpectraQA->SetCentralityEstimator("V0M");
  jetSpectraQA->SetMinNVertCont(1);
  jetSpectraQA->SetZvertexDiffValue(0.5);
  jetSpectraQA->SetNeedEmcalGeom(kFALSE);

}
