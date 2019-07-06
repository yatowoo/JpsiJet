/**
 * Main macro for running jobs on ALICE software
 * Arguments: doMult, doEmcalCorrection, doJetQA, doJpsiQA, doJpsiFilter, doPIDQA, doPhysAna
 *   -- mode: local, test, full, merge, final
 *   -- datasets: entry in Datasets/DQ_pp_AOD.C
 *   -- data_dir: 20XX/LHCXX[period]
 *   -- work_dir: output dir on AliEn
 *   -- task_name
 * Environment: AliRoot_v5.09.43 @ ROOT5
 * Template from https://alice-doc.github.io/alice-analysis-tutorial/analysis/local.html
*/

class AliAnalysisTaskEmcal;

#include "Datasets/DQ_pp_AOD.C"

AliAnalysisAlien* SetupGridHandler(
    TString mode = "local",
    TString datasets = "16l_pass1",
    TString data_dir = "2016/LHC16l",
    TString work_dir = "test",
    TString task_name = "JpsiJet"
){
  gROOT->LoadMacro("Datasets/DQ_pp_AOD.C");
  DQ_pp_AOD();
  TString runlist = DATASETS[datasets];
  if(!runlist.Length()){
    cout << "[X] ERROR - Wrong datasets : " << datasets << endl;
    exit(1);
  }


  AliAnalysisAlien *alienHandler = new AliAnalysisAlien();

  alienHandler->AddIncludePath("-I. -I$ROOTSYS/include -I$ALICE_ROOT -I$ALICE_ROOT/include -I$ALICE_PHYSICS/include");

  alienHandler->SetAdditionalLibs("AddTaskJPSIFilter.C AddTask_cjahnke_JPsi.C ConfigJpsi_cj_pp.C AddTaskJetQA.C YatoJpsiFilterTask.h YatoJpsiFilterTask.cxx");
  alienHandler->SetAnalysisSource("YatoJpsiFilterTask.cxx");

  alienHandler->SetAliPhysicsVersion("vAN-20190614_ROOT6-1");

  alienHandler->SetAPIVersion("V1.1x");

  // MC production
  if(data_dir.Length() > 11){
    alienHandler->SetGridDataDir("/alice/sim/"+data_dir);
    alienHandler->SetDataPattern("*/AOD/*AOD.root");
    alienHandler->SetRunPrefix("");
  }
  else{
    alienHandler->SetGridDataDir("/alice/data/"+data_dir);
    alienHandler->SetDataPattern("*/pass1/AOD/*AOD.root");
    alienHandler->SetRunPrefix("000");
  }

  alienHandler->AddRunNumber(runlist);

  alienHandler->SetNrunsPerMaster(1);

  alienHandler->SetSplitMaxInputFileNumber(20);

  alienHandler->SetTTL(43200); // 12 hours


  alienHandler->SetGridWorkingDir(work_dir);
  alienHandler->SetGridOutputDir("OutputAOD");

  alienHandler->SetAnalysisMacro(task_name + ".C");
  alienHandler->SetExecutable(task_name + ".sh");
  alienHandler->SetJDLName(task_name + ".jdl");
  alienHandler->SetOutputToRunNo(kTRUE);
  alienHandler->SetKeepLogs(kTRUE);

  alienHandler->SetMergeAOD(kTRUE);
  alienHandler->SetMaxMergeStages(1);
  if(mode == "final")
    alienHandler->SetMergeViaJDL(kFALSE);
  else
    alienHandler->SetMergeViaJDL(kTRUE);
  if(mode == "test") {
    alienHandler->SetNtestFiles(1);
    alienHandler->SetRunMode("test");
  } else if(mode == "full"){
    alienHandler->SetRunMode("full");
  } else if(mode == "merge" || mode == "final") {
    alienHandler->SetRunMode("terminate");
  } else{
    cout << "[X] Error - Unknown mode : " << mode << endl;
    exit(1);
  }
  return alienHandler;
}

void runAnalysis(
    Bool_t doMult = kFALSE,
    Bool_t doEmcalCorrection = kFALSE,
    Bool_t doJetQA = kTRUE,
    Bool_t doJpsiQA = kTRUE,
    Bool_t doJpsiFilter = kFALSE,
    Bool_t doPIDQA = kFALSE,
    Bool_t doPhysAna = kFALSE,
    TString mode = "local",
    TString datasets = "16l_pass1",
    TString data_dir = "2016/LHC16l",
    TString work_dir = "test",
    TString task_name = "JpsiJet"
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
  if(data_dir.Length() == 11) // Exp. data
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
    gInterpreter->ExecuteMacro(Form("AddTaskJPSIFilter.C(%d)",int(doPhysAna)));
    aodOutputH->SetOutputFileName("AliAOD.root");
    mgr->RegisterExtraFile("AliAOD.Dielectron.root");
  }
  // Task - J/psi QA
  if(doJpsiQA)
    gInterpreter->ExecuteMacro(Form("AddTaskJpsiQA.C(%d)", doJpsiFilter));

  // Start analysis
  if(!mgr->InitAnalysis()) return;
  mgr->PrintStatus();
  mgr->SetUseProgressBar(1, 25);
  if(mode == "local"){
    // Input data file
    TChain *chain = new TChain("aodTree");
    chain->Add("AliAOD_input.root");
    // Start Analysis
    mgr->StartAnalysis("local", chain);
  }else{
    AliAnalysisAlien* alienH = SetupGridHandler(mode, datasets, data_dir, work_dir, task_name);
    mgr->SetGridHandler(alienH);
    mgr->StartAnalysis("grid");
  }
}
