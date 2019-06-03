#ifndef __CINT__
#include "AliDielectron.h"
class AliDielectron;
#include "AliDielectronVarManager.h"
class AliDielectronVarManager;
#include "AliDielectronVarCuts.h"
class AliDielectronVarCuts;
#include "AliDielectronTrackCuts.h"
class AliDielectronTrackCuts;
#include "AliDielectronEventCuts.h"
class AliDielectronEventCuts;
#include "YatoJpsiFilterTask.h"
class YatoJpsiFilterTask;
#endif

YatoJpsiFilterTask* AddTaskJPSIFilter(Bool_t storeLS = kTRUE, Bool_t hasMC_aod = kFALSE, Bool_t storeTR = kTRUE){
  //get the current analysis manager
  AliAnalysisManager *mgr = AliAnalysisManager::GetAnalysisManager();
  if (!mgr) {
    Error("AddTaskJPSIFilter", "No analysis manager found.");
    return 0;
  }
  
  //check for output aod handler
  if (!mgr->GetOutputEventHandler()||mgr->GetOutputEventHandler()->IsA()!=AliAODHandler::Class()) {
    Warning("AddTaskJPSIFilter","No AOD output handler available. Not adding the task!");
    return 0;
  }

  //Do we have an MC handler?
  Bool_t hasMC=(AliAnalysisManager::GetAnalysisManager()->GetMCtruthEventHandler()!=0x0)||hasMC_aod;
  
  //Do we run on AOD?
  Bool_t isAOD=mgr->GetInputEventHandler()->IsA()==AliAODInputHandler::Class();

  
  if(isAOD) {
    //add options to AliAODHandler to duplicate input event
    AliAODHandler *aodHandler = (AliAODHandler*)mgr->GetOutputEventHandler();
    aodHandler->SetCreateNonStandardAOD();
    aodHandler->SetNeedsHeaderReplication();
    //aodHandler->SetNeedsTOFHeaderReplication();
    //aodHandler->SetNeedsVZEROReplication();
    //aodHandler->SetNeedsTracksBranchReplication();
    //aodHandler->SetNeedsVerticesBranchReplication();
    //aodHandler->SetNeedsTrackletsBranchReplication();
    //aodHandler->SetNeedsPMDClustersBranchReplication();
    //aodHandler->SetNeedsFMDClustersBranchReplication();
    //aodHandler->SetNeedsCaloClustersBranchReplication();
    //aodHandler->SetNeedsCaloTriggerBranchReplication();
    //aodHandler->SetNeedsHMPIDBranchReplication();
    if(hasMC) aodHandler->SetNeedsMCParticlesBranchReplication();
  }
  
  //Create task and add it to the analysis manager
  YatoJpsiFilterTask *task=new YatoJpsiFilterTask("jpsi2ee_EMCalFilter");
  task->SetTriggerMask(AliVEvent::kEMCEGA); 
  if (!hasMC) task->UsePhysicsSelection();
  if(storeLS) task->SetStoreLikeSignCandidates(storeLS);
  task->SetStoreRotatedPairs(storeTR);  
  task->SetToMerge(kTRUE);
	//Add event filter
	AliDielectronEventCuts *eventCuts = new AliDielectronEventCuts("eventCuts", "Vertex Track && |vtxZ|<10 && ncontrib>0");
	if (isAOD)
		eventCuts->SetVertexType(AliDielectronEventCuts::kVtxAny);
	eventCuts->SetRequireVertex();
	eventCuts->SetMinVtxContributors(1);
	eventCuts->SetVertexZ(-10., 10.);
	task->SetEventFilter(eventCuts);
  // Add AliDielectron
    // kEMCEGA + EMCal_loose
  AliDielectron* jpsi = reinterpret_cast<AliDielectron *>(gInterpreter->ExecuteMacro(Form("ConfigJpsi_cj_pp.C(1,%d,2)", isAOD)));
  jpsi->SetHasMC(hasMC);
  task->SetDielectron(jpsi);

  mgr->AddTask(task);

  //----------------------
  //create data containers
  //----------------------
  
  
  TString containerName = mgr->GetCommonFileName();
  containerName += ":PWGDQ_dielectronFilter";
  
  //create output container
  
  AliAnalysisDataContainer *cOutputHist1 =
    mgr->CreateContainer("jpsi_FilterQA",
                         THashList::Class(),
                         AliAnalysisManager::kOutputContainer,
                         containerName.Data());
  
  AliAnalysisDataContainer *cOutputHist2 =
    mgr->CreateContainer("jpsi_FilterEventStat",
                         TH1D::Class(),
                         AliAnalysisManager::kOutputContainer,
                         containerName.Data());
  
  
  mgr->ConnectInput(task,  0, mgr->GetCommonInputContainer());
  mgr->ConnectOutput(task, 1, cOutputHist1);
  mgr->ConnectOutput(task, 2, cOutputHist2);
  
  return task;
}
