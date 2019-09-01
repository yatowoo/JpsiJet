#include "AliDielectronEventCuts.h"

#include "AliAnalysisTaskJpsiJet.h"

// Trigger Index : 0/ALL, 1/INT7, 2/EG1, 3/EG2, 4/DG1, 5/DG2
enum TriggerIndex{
  kALL, kINT7, kEG1, kEG2, kDG1, kDG2, kNTrigIndex
};
const char* TRIGGER_CLASS[kNTrigIndex] = {
  "INT7;EG1;EG2;DG1;DG2",
  "INT7", "EG1", "EG2", "DG1", "DG2"};

AliAnalysisTaskJpsiJet* AddTaskJpsiJet_pp(int trigIndex = int(kALL)){
  // Analysis Manager
  AliAnalysisManager* mgr = AliAnalysisManager::GetAnalysisManager();

  // Analysis Task
  TString trigClass = TRIGGER_CLASS[trigIndex];
  TString taskName = Form("JpsiJet_PP13TeV_%s", trigClass.Data());
  if(trigIndex == kALL) taskName = "JpsiJet_PP13TeV_ALL";

  AliAnalysisTaskJpsiJet *task = new AliAnalysisTaskJpsiJet(taskName.Data());

  task->SetTrigger(AliVEvent::kINT7 | AliVEvent::kEMCEGA);
  task->SetTriggerClasses(trigClass.Data());
  if(trigClass.Contains(";"))
    task->SetTriggerQA(kTRUE); // Multi-triggers in single task

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

  if(trigIndex == kALL) trigClass = "ALL";
  AliAnalysisDataContainer* cHistos = mgr->CreateContainer(Form("QAhistos_%s", trigClass.Data()), TList::Class(), AliAnalysisManager::kOutputContainer, containerName.Data());

  mgr->ConnectInput(task,0,mgr->GetCommonInputContainer());
  mgr->ConnectOutput(task,1, cHistos);

  return task;
}
