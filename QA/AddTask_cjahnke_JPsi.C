#ifndef __CINT__ && __MultiDie__ 
#define __MultiDie__
#include "AliDielectron.h"
class AliDielectron;
#include "AliESDtrackCuts.h"
class AliESDtrackCuts;
#include "AliDielectronVarManager.h"
class AliDielectronVarManager;
#include "AliDielectronVarCuts.h"
class AliDielectronVarCuts;
#include "AliDielectronPairLegCuts.h"
class AliDielectronPairLegCuts;
#include "AliDielectronTrackCuts.h"
class AliDielectronTrackCuts;
#include "AliDielectronCF.h"
class AliDielectronCF;
#include "AliAnalysisTaskMultiDielectron.h"
class AliAnalysisTaskMultiDielectron;
#endif // __MultiDie__

AliAnalysisTask *AddTask_cjahnke_JPsi(Int_t trigger_index = 0, Bool_t isMC = kFALSE)
{
	//get the current analysis manager
	AliAnalysisManager *mgr = AliAnalysisManager::GetAnalysisManager();
	if (!mgr)
	{
		::Error("AddTask_cjahnke_JPsi", "No analysis manager found.");
		return NULL;
	}
	if (!mgr->GetInputEventHandler())
	{
		::Error("AddTask_cjahnke_JPsi", "This task requires an input event handler");
		return NULL;
	}

	//Do we have an MC handler?
	Bool_t hasMC = (isMC && (mgr->GetMCtruthEventHandler() != NULL));

	Bool_t isAOD = (mgr->GetInputEventHandler()->IsA() == AliAODInputHandler::Class());

	//create task and add it to the manager
	AliAnalysisTaskMultiDielectron *task = new AliAnalysisTaskMultiDielectron(Form("MultiDie_%d", trigger_index));
  if(task){
  	mgr->AddTask(task);
  }
  else{
    cout << "[X] Fail to add task : MultiDie_" << trigger_index << endl;
    exit(1); 
  }

	///======
	// Load configuration file
	// ======

	//add dielectron analysis with different cuts to the task
  // ONLY use RAW and EMCal_strict cut definition
	for (Int_t i = 0; i < 2; ++i)
	{ //nDie defined in config file
    AliDielectron *jpsi = ConfigJpsi_cj_pp(i, isAOD, trigger_index, hasMC);
		if (isAOD)
			jpsi->SetHasMC(hasMC);
		if (jpsi)
			task->AddDielectron(jpsi);
	}

	//Add event filter
	AliDielectronEventCuts *eventCuts = new AliDielectronEventCuts("eventCuts", "Vertex Track && |vtxZ|<10 && ncontrib>0");
	if (isAOD)
		eventCuts->SetVertexType(AliDielectronEventCuts::kVtxAny);
	eventCuts->SetRequireVertex();
	eventCuts->SetMinVtxContributors(1);
	eventCuts->SetVertexZ(-10., 10.);
	task->SetEventFilter(eventCuts);
	task->SetRejectPileup();

	// Select event by trigger
	if (!hasMC)
	{
		if(trigger_index == 0)
			task->SetTriggerMask(AliVEvent::kINT7);
		else if (trigger_index == 1)
			task->SetTriggerMask(AliVEvent::kEMC7);
		else if (trigger_index == 2)
			task->SetTriggerMask(AliVEvent::kEMCEGA);
		else if (trigger_index == 3)
		{
			task->SetTriggerMask(AliVEvent::kEMCEGA);
			task->SetFiredTriggerName("EG1");
		}
		else if (trigger_index == 4)
		{
			task->SetTriggerMask(AliVEvent::kEMCEGA);
			task->SetFiredTriggerName("EG2");
		}
		//for 16k which has a different threshold:
		else if (trigger_index == 6)
		{
			task->SetTriggerMask(AliVEvent::kEMCEGA);
			task->SetFiredTriggerName("EG1");
		}
		//DCal triggers
		else if (trigger_index == 30)
		{
			task->SetTriggerMask(AliVEvent::kEMCEGA);
			task->SetFiredTriggerName("DG1");
		}
		else if (trigger_index == 40)
		{
			task->SetTriggerMask(AliVEvent::kEMCEGA);
			task->SetFiredTriggerName("DG2");
		}
		//for 16k which has a different threshold:
		else if (trigger_index == 60)
		{
			task->SetTriggerMask(AliVEvent::kEMCEGA);
			task->SetFiredTriggerName("DG1");
		}
		else if (trigger_index == 5)
		{
			task->SetTriggerMask(AliVEvent::kAny);
			task->SetFiredTriggerName("HMV0");
		}
		else if (trigger_index == 50)
			task->SetTriggerMask(AliVEvent::kHighMultV0);

		//use this line for data, otherwise the physics selection is ignored by my task...
		task->UsePhysicsSelection();
	}

	//----------------------
	//create data containers
	//----------------------

	TString containerName = mgr->GetCommonFileName();
	containerName += Form(":PWGDQ_dielectron_MultiDie_EMCal_%d",trigger_index);

	//create output container

	AliAnalysisDataContainer *cOutputHist1 =
			mgr->CreateContainer(Form("cjahnke_QA_%d", trigger_index), TList::Class(), AliAnalysisManager::kOutputContainer,
													 containerName.Data());

	AliAnalysisDataContainer *cOutputHist2 =
			mgr->CreateContainer(Form("cjahnke_CF_%d", trigger_index), TList::Class(), AliAnalysisManager::kOutputContainer,
													 containerName.Data());

	AliAnalysisDataContainer *cOutputHist3 =
			mgr->CreateContainer(Form("cjahnke_EventStat_%d", trigger_index), TH1D::Class(), AliAnalysisManager::kOutputContainer,
													 containerName.Data());

	mgr->ConnectInput(task, 0, mgr->GetCommonInputContainer());
	mgr->ConnectOutput(task, 1, cOutputHist1);
	mgr->ConnectOutput(task, 2, cOutputHist2);
	mgr->ConnectOutput(task, 3, cOutputHist3);

	return task;
}
