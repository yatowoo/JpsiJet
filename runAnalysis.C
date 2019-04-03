/**
 * Main macro for running jobs on ALICE software
 * 
 * Environment: AliRoot_v5.09.43 @ ROOT6
 * Template from https://alice-doc.github.io/alice-analysis-tutorial/analysis/local.html
*/

void runAnalysis(){
  // Include
  gInterpreter->ProcessLine(".include $ROOTSYS/include");
  gInterpreter->ProcessLine(".include $ALICE_ROOT/include");
  gInterpreter->ProcessLine(".include $ALICE_PHYSICS/include");

  // Analysis Manager
  AliAnalysisManager *mgr = new AliAnalysisManager("JpsiJetTask");
  AliAODInputHandler *aodH = new AliAODInputHandler();
  mgr->SetInputEventHandler(aodH);

  // Task - Jet finder 
    // Charged Jet
  AliEmcalJetTask *pChJet02Task = AliEmcalJetTask::AddTaskEmcalJet("usedefault", "", AliJetContainer::antikt_algorithm, 0.2, AliJetContainer::kChargedJet, 0.15, 0, 0.01, AliJetContainer::pt_scheme, "Jet", 1., kFALSE, kFALSE);
  pChJet02Task->SelectCollisionCandidates(AliVEvent::kAny);
  pChJet02Task->SetNeedEmcalGeom(kFALSE);
    // Full Jet
  AliEmcalJetTask *pFuJet02Task = AliEmcalJetTask::AddTaskEmcalJet("usedefault", "usedefault", AliJetContainer::antikt_algorithm, 0.2, AliJetContainer::kFullJet, 0.15, 0.30, 0.01, AliJetContainer::pt_scheme, "Jet", 1., kFALSE, kFALSE);
  pFuJet02Task->SelectCollisionCandidates(AliVEvent::kAny);
  // Task - PWGJE QA (Event, Track, Calo, Jet)
  AliAnalysisTaskPWGJEQA* jetQA = reinterpret_cast<AliAnalysisTaskPWGJEQA*>(gInterpreter->ExecuteMacro("$ALICE_PHYSICS/PWGJE/EMCALJetTasks/macros/AddTaskPWGJEQA.C(\"usedefault\",\"usedefault\",\"usedefault\",\"\")"));
  AliJetContainer* jetChCont02 = jetQA->AddJetContainer(AliJetContainer::kChargedJet, AliJetContainer::antikt_algorithm, AliJetContainer::pt_scheme, 0.2, AliEmcalJet::kTPCfid, "Jet");
  AliJetContainer* jetFuCont02 = jetQA->AddJetContainer(AliJetContainer::kFullJet, AliJetContainer::antikt_algorithm, AliJetContainer::pt_scheme, 0.2, AliEmcalJet::kEMCALfid, "Jet");

  // Task - PID QA
  gInterpreter->ExecuteMacro("$ALICE_ROOT/ANALYSIS/macros/AddTaskPIDResponse.C");
  gInterpreter->ExecuteMacro("$ALICE_ROOT/ANALYSIS/macros/AddTaskPIDqa.C");

  // Input data file
  TChain *chain = new TChain("aodTree");
  chain->Add("AliAOD_input.root");

  // Start Analysis
  if(!mgr->InitAnalysis()) return;
  mgr->PrintStatus();
  mgr->SetUseProgressBar(1, 25);
  mgr->StartAnalysis("local", chain);
}
