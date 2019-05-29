/**
 * Main macro for running jobs on ALICE software
 * 
 * Environment: AliRoot_v5.09.43 @ ROOT6
 * Template from https://alice-doc.github.io/alice-analysis-tutorial/analysis/local.html
*/

void runAnalysis(
    Bool_t doMult = kFALSE,
    Bool_t doEmcalCorrection = kFALSE,
    Bool_t doJetQA = kTRUE,
    Bool_t doJpsiQA = kTRUE,
    Bool_t doJpsiFilter = kFALSE,
    Bool_t doPIDQA = kFALSE
){
  // Include
  gInterpreter->ProcessLine(".include $ROOTSYS/include");
  gInterpreter->ProcessLine(".include $ALICE_ROOT/include");
  gInterpreter->ProcessLine(".include $ALICE_PHYSICS/include");
  
  // Analysis Manager
  AliAnalysisManager *mgr = new AliAnalysisManager("JpsiJetTask");
    // Input handler
  AliAODInputHandler *aodH = new AliAODInputHandler();
  mgr->SetInputEventHandler(aodH);
    // Output handler
  AliAODHandler* aodOutputH = new AliAODHandler();
  aodOutputH->SetOutputFileName("AliAOD.root");
  mgr->SetOutputEventHandler(aodOutputH);

  // Task - Physics Selection
  gInterpreter->ExecuteMacro("$ALICE_PHYSICS/OADB/macros/AddTaskPhysicsSelection.C");

  // Task - Centrality / Multiplicity
  if(doMult)
    gInterpreter->ExecuteMacro("$ALICE_PHYSICS/OADB/COMMON/MULTIPLICITY/macros/AddTaskMultSelection.C");

  if(doEmcalCorrection){
    // Task - Connect OCDB (ONLY for LEGO train)
    gInterpreter->ExecuteMacro("$ALICE_PHYSICS/PWGPP/PilotTrain/AddTaskCDBconnect.C");

    // Task - EMCal Correction
    AliEmcalCorrectionTask * correctionTask = AliEmcalCorrectionTask::AddTaskEmcalCorrectionTask();
    correctionTask->SelectCollisionCandidates(AliVEvent::kINT7 | AliVEvent::kEMCEGA);
    correctionTask->SetForceBeamType(static_cast<AliEmcalCorrectionTask::BeamType>(AliAnalysisTaskEmcal::kpp));
    correctionTask->SetUserConfigurationFilename("$ALICE_PHYSICS/PWGHF/hfe/macros/configs/pp/userConfigurationEMCele_pp_pPb.yaml");
    correctionTask->Initialize();
  }
  
  // QA directory - Git repo
  gROOT->SetMacroPath(".:./QA/:./NanoAOD/");

  // Task - Jet QA
  if(doJetQA)
    gInterpreter->ExecuteMacro("AddTaskJetQA.C");

  // Task - PID QA
  if(doJpsiQA || doJpsiFilter || doPIDQA)
    gInterpreter->ExecuteMacro("$ALICE_ROOT/ANALYSIS/macros/AddTaskPIDResponse.C");
  if(doPIDQA)
    gInterpreter->ExecuteMacro("$ALICE_ROOT/ANALYSIS/macros/AddTaskPIDqa.C");

  // Task - J/psi Filter
  if(doJpsiFilter){
    gInterpreter->LoadMacro("YatoJpsiFilterTask.cxx++g");
    gInterpreter->ExecuteMacro("AddTaskJPSIFilter.C");
    aodOutputH->SetOutputFileName("AliAOD.Dielectron.root");
    mgr->RegisterExtraFile("AliAOD.Dielectron.root");
  }
  // Task - J/psi QA
  if(doJpsiQA)
    gInterpreter->ExecuteMacro("AddTaskJpsiQA.C");

  // Input data file
  TChain *chain = new TChain("aodTree");
  chain->Add("AliAOD_input.root");

  // Start Analysis
  if(!mgr->InitAnalysis()) return;
  mgr->PrintStatus();
  mgr->SetUseProgressBar(1, 25);
  mgr->StartAnalysis("local", chain);
}
