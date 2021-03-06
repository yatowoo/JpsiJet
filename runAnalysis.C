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

Bool_t isMC = kFALSE;

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

  alienHandler->SetAdditionalLibs("AddTaskJpsiJetFilter_pp.C AliAnalysisTaskJpsiJetFilter.cxx AliAnalysisTaskJpsiJetFilter.h AddTaskJpsiJet_pp.C AliAnalysisTaskJpsiJet.cxx AliAnalysisTaskJpsiJet.h");
  alienHandler->SetAnalysisSource("AliAnalysisTaskJpsiJetFilter.cxx AliAnalysisTaskJpsiJet.cxx");

  alienHandler->SetAliPhysicsVersion("vAN-20190921_ROOT6-1");

  alienHandler->SetAPIVersion("V1.1x");

  // MC production
  if(isMC){
    alienHandler->SetGridDataDir("/alice/sim/"+data_dir);
    alienHandler->SetDataPattern("*/AOD/*AOD.root");
    alienHandler->SetRunPrefix("");
  }
  else{
    alienHandler->SetGridDataDir("/alice/data/"+data_dir);
    if(datasets.Contains("18m")){
      alienHandler->SetDataPattern("*/pass1_withTRDtracking/AOD*/*AOD.root");}
    else{
      alienHandler->SetDataPattern("*/pass1/AOD*/*AOD.root");
    }
    alienHandler->SetRunPrefix("000");
  }

  alienHandler->AddRunNumber(runlist);

  alienHandler->SetNrunsPerMaster(200);

  alienHandler->SetSplitMaxInputFileNumber(20);

  alienHandler->SetTTL(43200); // 12 hours


  alienHandler->SetGridWorkingDir(work_dir);
  alienHandler->SetGridOutputDir("OutputAOD");

  alienHandler->SetAnalysisMacro(task_name + ".C");
  alienHandler->SetExecutable(task_name + ".sh");
  alienHandler->SetJDLName(task_name + ".jdl");
  alienHandler->SetOutputToRunNo(kTRUE);
  alienHandler->SetKeepLogs(kTRUE);
  
  alienHandler->SetMaxMergeFiles(50); // DEBUG: memory leak during merge stage
  alienHandler->SetMergeAOD(kTRUE);
  alienHandler->SetMaxMergeStages(2);
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
    Bool_t doDevPWG = kTRUE,
    Bool_t doMult = kFALSE,
    Bool_t doEmcalCorrection = kFALSE,
    Bool_t doJetQA = kFALSE,
    Bool_t doJpsiQA = kFALSE,
    Bool_t doJpsiFilter = kTRUE,
    Bool_t doPIDQA = kFALSE,
    Bool_t doPhysAna = kFALSE,
    TString mode = "local",
    TString datasets = "16k_pass1",
    TString data_dir = "2016/LHC16k",
    TString work_dir = "test",
    TString task_name = "JpsiJet"
){
  // Include
  gInterpreter->ProcessLine(".include $ROOTSYS/include");
  gInterpreter->ProcessLine(".include $ALICE_ROOT/include");
  gInterpreter->ProcessLine(".include $ALICE_PHYSICS/include");
  
  // Analysis Manager
  AliAnalysisManager *mgr = new AliAnalysisManager("JpsiJetTask");
    // DATA or MC input
  if(data_dir.Length() > 11)
    isMC = kTRUE;
    // Input handler
  AliAODInputHandler *aodH = new AliAODInputHandler();
  mgr->SetInputEventHandler(aodH);
    // Output handler
  AliAODHandler* aodOutputH = new AliAODHandler();
  aodOutputH->SetOutputFileName("AliAOD.root");
  mgr->SetOutputEventHandler(aodOutputH);

  // Task - Physics Selection
  if(!isMC) // Exp. data
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
  gROOT->SetMacroPath(".:./QA/:./NanoAOD/:./PWG");

  // Task - Jet QA
  if(doJetQA)
    gInterpreter->ExecuteMacro("AddTaskJetQA.C");

  // Task - PID
  if(doDevPWG || doJpsiQA || doJpsiFilter || doPIDQA)
    if(!isMC)
      gInterpreter->ExecuteMacro("$ALICE_ROOT/ANALYSIS/macros/AddTaskPIDResponse.C");
    else{
      gInterpreter->ExecuteMacro("$ALICE_ROOT/ANALYSIS/macros/AddTaskPIDResponse.C(1,0,1,\"pass1\")");
    }
  if(doPIDQA)
    gInterpreter->ExecuteMacro("$ALICE_ROOT/ANALYSIS/macros/AddTaskPIDqa.C");

  // Task - J/psi QA
  if(doJpsiQA)
    gInterpreter->ExecuteMacro(Form("AddTaskJpsiQA.C(%d,\"%s\")", doJpsiFilter,datasets.Data()));

  // Task - JpsiJet
  if(doDevPWG){
    gInterpreter->AddIncludePath("./PWG");
    gInterpreter->LoadMacro("AliAnalysisTaskJpsiJet.cxx++g");
    if(isMC)
      gInterpreter->ExecuteMacro(Form("AddTaskJpsiJet_pp.C(kMC,kTRUE,\"%s\")", datasets.Data()));
    else{
      gInterpreter->ExecuteMacro("AddTaskJpsiJet_pp.C(kALL,kTRUE)");
      gInterpreter->Execute("AddTaskJpsiJet_pp","kINT7,kFALSE");
      gInterpreter->Execute("AddTaskJpsiJet_pp","kEG1,kFALSE");
      gInterpreter->Execute("AddTaskJpsiJet_pp","kEG2,kFALSE");
      gInterpreter->Execute("AddTaskJpsiJet_pp","kDG1,kFALSE");
      gInterpreter->Execute("AddTaskJpsiJet_pp","kDG2,kFALSE"); 
    }
  }
  // Task - J/psi Filter
  if(doJpsiFilter){
    gInterpreter->AddIncludePath("./PWG");
    gInterpreter->LoadMacro("AliAnalysisTaskJpsiJetFilter.cxx++g");
    gInterpreter->ExecuteMacro("AddTaskJpsiJetFilter_pp.C");
    aodOutputH->SetOutputFileName("AliAOD.root");
    mgr->RegisterExtraFile("AliAOD.Dielectron.root");
  }
  // Start analysis
  if(!mgr->InitAnalysis()) return;
  mgr->PrintStatus();
  mgr->SetUseProgressBar(1, 25);
  if(mode == "local"){
    // Input data file
    TChain *chain = new TChain("aodTree");
    if(isMC)
      chain->Add("AliAOD_MCinput.root");
    else
      chain->Add("AliAOD_input.root");
    // Start Analysis
    mgr->StartAnalysis("local", chain);
  }else{
    AliAnalysisAlien* alienH = SetupGridHandler(mode, datasets, data_dir, work_dir, task_name);
    alienH->SetDropToShell(kFALSE);
    mgr->SetGridHandler(alienH);
    mgr->StartAnalysis("grid");
  }
}
