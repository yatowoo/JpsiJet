#include "AliDielectronEventCuts.h"

#include "AliAnalysisTaskJpsiJet.h"

AliAnalysisTaskJpsiJet* AddTaskJpsiJet_pp(){
  // Analysis Manager
  AliAnalysisManager* mgr = AliAnalysisManager::GetAnalysisManager();

  // Analysis Task
  AliAnalysisTaskJpsiJet *task = new AliAnalysisTaskJpsiJet("JpsiJet_PP13TeV");

  task->SetTrigger(AliVEvent::kINT7 | AliVEvent::kEMCEGA);
  task->SetTriggerClasses("MB;INT7;EG1;EG2;DG1;DG2");
  task->SetTriggerQA(kTRUE);

  //Event filter
  AliDielectronEventCuts *eventCuts = new AliDielectronEventCuts("eventCuts", "Vertex Track && |vtxZ|<10 && ncontrib>1");
  eventCuts->SetVertexType(AliDielectronEventCuts::kVtxAny);
  eventCuts->SetRequireVertex();
  eventCuts->SetMinVtxContributors(1);
  eventCuts->SetVertexZ(-10., 10.); // Unit: cm
  task->SetEventFilter(eventCuts);

  task->SetRejectPileup(kTRUE);

  if(task) mgr->AddTask(task);

  // Output container
  TString containerName = mgr->GetCommonFileName();
	containerName += ":JpsiJetAnalysis";

  AliAnalysisDataContainer* cHistos = mgr->CreateContainer("QA_histos", TList::Class(), AliAnalysisManager::kOutputContainer, containerName.Data());

  mgr->ConnectInput(task,0,mgr->GetCommonInputContainer());
  mgr->ConnectOutput(task,1, cHistos);

  return task;
}
