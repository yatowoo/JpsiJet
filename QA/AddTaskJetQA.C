void AddTaskJetQA(UInt_t kPhysSel = AliVEvent::kINT7 | AliVEvent::kEMCEGA){

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
