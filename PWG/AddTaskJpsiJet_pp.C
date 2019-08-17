#include "AliAnalysisTaskJpsiJet.h"

AliAnalysisTaskJpsiJet* AddTaskJpsiJet_pp(){
  // Analysis Manager
  AliAnalysisManager* mgr = AliAnalysisManager::GetAnalysisManager();

  AliAnalysisTaskJpsiJet *task = new AliAnalysisTaskJpsiJet("JpsiJet_PP13TeV");

  if(task) mgr->AddTask(task);

  // Output container
  TString containerName = mgr->GetCommonFileName();
	containerName += ":JpsiJetAnalysis";

  AliAnalysisDataContainer* cHistos = mgr->CreateContainer("QA_histos", TList::Class(), AliAnalysisManager::kOutputContainer, containerName.Data());

  mgr->ConnectInput(task,0,mgr->GetCommonInputContainer());
  mgr->ConnectOutput(task,1, cHistos);

  return task;
}