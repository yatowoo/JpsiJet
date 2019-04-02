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