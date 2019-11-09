#!/usr/bin/env python3

# Run Analysis script

import ROOT

# Include
ROOT.gInterpreter.ProcessLine(".include $ROOTSYS/include");
ROOT.gInterpreter.ProcessLine(".include $ALICE_ROOT/include");
ROOT.gInterpreter.ProcessLine(".include $ALICE_PHYSICS/include");
  
# Analysis Manager
mgr = ROOT.AliAnalysisManager("JpsiJetAnalysis");
# Input handler
aodH = ROOT.AliAODInputHandler();
mgr.SetInputEventHandler(aodH);
# Output handler
aodOutputH = ROOT.AliAODHandler();
aodOutputH.SetOutputFileName("AliAOD.root");
mgr.SetOutputEventHandler(aodOutputH);

ROOT.gInterpreter.ExecuteMacro("$ALICE_PHYSICS/OADB/macros/AddTaskPhysicsSelection.C");
ROOT.gInterpreter.ExecuteMacro("$ALICE_ROOT/ANALYSIS/macros/AddTaskPIDResponse.C");
ROOT.gInterpreter.ExecuteMacro("$ALICE_ROOT/ANALYSIS/macros/AddTaskPIDqa.C");
# Start analysis
if( not mgr.InitAnalysis()):
  exit()
 
mgr.PrintStatus();
mgr.SetUseProgressBar(1, 25);
    
# Input data file
chain = ROOT.TChain("aodTree");
chain.Add("AliAOD_input.root");
 
mgr.StartAnalysis("local", chain);

# End
