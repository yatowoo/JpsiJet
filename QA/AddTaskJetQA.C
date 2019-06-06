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
      - ghostArea   = 0.005
      - reco        = AliJetContainer::pt_scheme // ERecoScheme_t
      - tag         = "Jet"
      - minJetPt    = 1.
      - lockTask    = kFALSE
      - bFillGhosts = kFALSE
*   - Rho: AddTaskRhoSparse, Wagon-HMRhoTask_02/04
      - runEMCalJetSampleTask.C, AliAnalysisTaskRho::AddTaskRhoNew
*   - DeltaPt: AddTaskDeltaPt, Wagon-HMDeltaPt_02/04
      -
* Main task:
*   - SpectraQA: AliAnalysisTaskEmcalJetSpectraQA
*   - PWGJEQA: AliAnalysisTaskPWGJEQA
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
  AliAnalysisTaskRho* jetRho = AliAnalysisTaskRho::AddTaskRhoNew("tracks","", rhoName, jetRadius, AliEmcalJet::kTPCfid, AliJetContainer::kChargedJet, AliJetContainer::pt_scheme, "");

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
  AliEmcalJetTask* jetFinder = AliEmcalJetTask::AddTaskEmcalJet("usedefault", "", AliJetContainer::antikt_algorithm, jetRadius, AliJetContainer::kChargedJet, 0.15, 0.3, 0.005, AliJetContainer::pt_scheme, "Jet", 1., kFALSE, kFALSE);

  jetFinder->SetForceBeamType(AliAnalysisTaskEmcal::kpp);
  jetFinder->SelectCollisionCandidates(AliVEvent::kINT7);
  jetFinder->SetUseAliAnaUtils(kTRUE);
  jetFinder->SetZvertexDiffValue(0.5);
  jetFinder->SetNeedEmcalGeom(kFALSE);

  TString jetName = Form("Jet_AKTChargedR%03d_tracks_pT0150_pt_scheme",int(jetRadius*100));
  TString rhoName = Form("Rho%03d",int(jetRadius*100));
  AddRho(jetRadius, jetName, rhoName);
  AddDeltaPt(jetRadius, jetName, rhoName);

  return jetFinder;
}

void AddTaskJetQA(UInt_t kPhysSel = AliVEvent::kINT7){

  // Task - Jet finder 
    // Charged Jet
  AliEmcalJetTask *pChJet02Task = AliEmcalJetTask::AddTaskEmcalJet("usedefault", "", AliJetContainer::antikt_algorithm, 0.2, AliJetContainer::kChargedJet, 0.15, 0, 0.01, AliJetContainer::pt_scheme, "Jet", 1., kFALSE, kFALSE);
  pChJet02Task->SelectCollisionCandidates(kPhysSel);
  pChJet02Task->SetNeedEmcalGeom(kFALSE);
    // Full Jet
  AliEmcalJetTask *pFuJet02Task = AliEmcalJetTask::AddTaskEmcalJet("usedefault", "usedefault", AliJetContainer::antikt_algorithm, 0.2, AliJetContainer::kFullJet, 0.15, 0.30, 0.01, AliJetContainer::pt_scheme, "Jet", 1., kFALSE, kFALSE);
  pFuJet02Task->SelectCollisionCandidates(kPhysSel);
  
  // Task - PWGJE QA (Event, Track, Calo, Jet)
  AliAnalysisTaskPWGJEQA* jetQA = reinterpret_cast<AliAnalysisTaskPWGJEQA*>(gInterpreter->ExecuteMacro("$ALICE_PHYSICS/PWGJE/EMCALJetTasks/macros/AddTaskPWGJEQA.C(\"usedefault\",\"usedefault\",\"usedefault\",\"\")"));
  jetQA->SelectCollisionCandidates(kPhysSel);
  AliJetContainer* jetChCont02 = jetQA->AddJetContainer(AliJetContainer::kChargedJet, AliJetContainer::antikt_algorithm, AliJetContainer::pt_scheme, 0.2, AliEmcalJet::kTPCfid, "Jet");
  AliJetContainer* jetFuCont02 = jetQA->AddJetContainer(AliJetContainer::kFullJet, AliJetContainer::antikt_algorithm, AliJetContainer::pt_scheme, 0.2, AliEmcalJet::kEMCALfid, "Jet");

  // Task - Jet Spectra QA
  AliAnalysisTaskEmcalJetSpectraQA* jetSpectraQA = AliAnalysisTaskEmcalJetSpectraQA::AddTaskEmcalJetSpectraQA("usedefault", "", 0.15, 0.30, "");

  jetContCh02 = jetSpectraQA->AddJetContainer(AliJetContainer::kChargedJet, AliJetContainer::antikt_algorithm, AliJetContainer::pt_scheme, 0.2, AliJetContainer::kTPCfid,  "tracks", "", "Jet");
  //jetContCh02->SetRhoName("Rho02");
  jetContCh02->SetPercAreaCut(0.6);

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
