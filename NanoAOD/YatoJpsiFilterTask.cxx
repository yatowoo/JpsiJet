#include "YatoJpsiFilterTask.h"

#include "TChain.h"
#include "THashList.h"

#include <AliLog.h>
#include "AliAODCaloCluster.h"

ClassImp(YatoJpsiFilterTask)

YatoJpsiFilterTask::YatoJpsiFilterTask() : 
  AliAnalysisTaskSE(),
  fIsToMerge(kFALSE),
	fDielectron(0),
	fSelectPhysics(kTRUE),
	fTriggerMask(AliVEvent::kMB),
	fExcludeTriggerMask(0),
	fTriggerOnV0AND(kFALSE),
	fRejectPileup(kFALSE),
	fEventStat(0x0),
	fTriggerLogic(kAny),
	fTriggerAnalysis(0x0),
	fStoreLikeSign(kFALSE),
	fStoreRotatedPairs(kFALSE),
	fStoreEventsWithSingleTracks(kFALSE),
	fCreateNanoAOD(kFALSE),
	fStoreHeader(kFALSE),
	fStoreEventplanes(kFALSE),
	fEventFilter(0x0),
	fQnList(0x0)
{}

YatoJpsiFilterTask::YatoJpsiFilterTask(const char* name) : 
  AliAnalysisTaskSE(name),
  fIsToMerge(kFALSE),
	fDielectron(0),
	fSelectPhysics(kTRUE),
	fTriggerMask(AliVEvent::kMB),
	fExcludeTriggerMask(0),
	fTriggerOnV0AND(kFALSE),
	fRejectPileup(kFALSE),
	fEventStat(0x0),
	fTriggerLogic(kAny),
	fTriggerAnalysis(0x0),
	fStoreLikeSign(kFALSE),
	fStoreRotatedPairs(kFALSE),
	fStoreEventsWithSingleTracks(kFALSE),
	fCreateNanoAOD(kFALSE),
	fStoreHeader(kFALSE),
	fStoreEventplanes(kFALSE),
	fEventFilter(0x0),
	fQnList(0x0)
{
  DefineInput(0,TChain::Class());
  DefineOutput(1, THashList::Class());
  DefineOutput(2, TH1D::Class());
};

YatoJpsiFilterTask::~YatoJpsiFilterTask()
{
  // Destructor
  if(fDielectron)
    delete fDielectron;
  if(fEventStat)
    delete fEventStat;
  if(fTriggerAnalysis)
    delete fTriggerAnalysis;
  if(fEventFilter)
    delete fEventFilter;
  if(fQnList)
    delete fQnList;
}

Bool_t YatoJpsiFilterTask::Notify()
{
	AddMetadataToUserInfo();
	return kTRUE;
}

Bool_t YatoJpsiFilterTask::AddMetadataToUserInfo()
{
	static Bool_t copyFirst = kFALSE;
	if (!copyFirst) {
		AliAnalysisManager *mgr = AliAnalysisManager::GetAnalysisManager();
		if (!mgr) {
			AliError("YatoJpsiFilterTask::AddMetadataToUserInfo() : No analysis manager !");
			return kFALSE;
		}
		TTree *aodTree = mgr->GetTree()->GetTree();
		if (!aodTree) return kFALSE;
		TNamed *alirootVersion = (TNamed*)aodTree->GetUserInfo()->FindObject("alirootVersion");
		if (!alirootVersion) return kFALSE;
		AliAODHandler *aodHandler = dynamic_cast<AliAODHandler*>(mgr->GetOutputEventHandler());
		if (!aodHandler) return kFALSE;
		AliAODExtension *extDielectron = aodHandler->GetFilteredAOD("AliAOD.Dielectron.root");
		TTree *nanoaodTree = extDielectron->GetTree();
		if (!nanoaodTree) return kFALSE;
		nanoaodTree->GetUserInfo()->Add(new TNamed(*alirootVersion));
		copyFirst = kTRUE;
	}
	return kTRUE;
}

void YatoJpsiFilterTask::UserCreateOutputObjects()
{
	if (!fDielectron) {
		AliFatal("Dielectron framework class required. Please create and instance with proper cuts and set it via 'SetDielectron' before executing this task!!!");
		return;
	}
	if(fStoreRotatedPairs) fDielectron->SetStoreRotatedPairs(kTRUE);
	fDielectron->SetDontClearArrays();
	fDielectron->Init();

	Int_t nbins=kNbinsEvent+2;
	if (!fEventStat){
		fEventStat=new TH1D("hEventStat","Event statistics",nbins,0,nbins);
		fEventStat->GetXaxis()->SetBinLabel(1,"Before Phys. Sel.");
		fEventStat->GetXaxis()->SetBinLabel(2,"After Phys. Sel.");

		fEventStat->GetXaxis()->SetBinLabel(3,"Bin3 not used");
		fEventStat->GetXaxis()->SetBinLabel(4,"Bin4 not used");
		fEventStat->GetXaxis()->SetBinLabel(5,"Bin5 not used");

		if(fTriggerOnV0AND) fEventStat->GetXaxis()->SetBinLabel(3,"V0and triggers");
		if (fEventFilter) fEventStat->GetXaxis()->SetBinLabel(4,"After Event Filter");
		if (fRejectPileup) fEventStat->GetXaxis()->SetBinLabel(5,"After Pileup rejection");

		fEventStat->GetXaxis()->SetBinLabel((kNbinsEvent+1),Form("#splitline{1 candidate}{%s}",fDielectron->GetName()));
		fEventStat->GetXaxis()->SetBinLabel((kNbinsEvent+2),Form("#splitline{With >1 candidate}{%s}",fDielectron->GetName()));
	}

	PostData(1, const_cast<THashList*>(fDielectron->GetHistogramList()));
	PostData(2,fEventStat);
}


void YatoJpsiFilterTask::Init(){
  // Initialization
  if (fDebug > 1) AliInfo("Init() \n");
  // require AOD handler
  AliAODHandler *aodH = (AliAODHandler*)((AliAnalysisManager::GetAnalysisManager())->GetOutputEventHandler());
  if (!aodH) AliFatal("No AOD handler. Halting.");
  aodH->AddFilteredAOD("AliAOD.Dielectron.root", "DielectronEvents", fIsToMerge);
}

// Update: copy title for primary vertex
void YatoJpsiFilterTask::UserExec(Option_t*){
	//
	// Main loop. Called for every event
	//

	if (!fDielectron)
		return;

	AliAnalysisManager *man = AliAnalysisManager::GetAnalysisManager();
	Bool_t isESD = man->GetInputEventHandler()->IsA() == AliESDInputHandler::Class();
	Bool_t isAOD = man->GetInputEventHandler()->IsA() == AliAODInputHandler::Class();

	AliInputEventHandler *inputHandler = (AliInputEventHandler *)(man->GetInputEventHandler());
	if (!inputHandler)
		return;

	if (inputHandler->GetPIDResponse())
	{
		AliDielectronVarManager::SetPIDResponse(inputHandler->GetPIDResponse());
	}
	else
	{
		AliFatal("This task needs the PID response attached to the input event handler!");
	}

	// Was event selected ?
	ULong64_t isSelected = AliVEvent::kAny;
	Bool_t isRejected = kFALSE;
	if (fSelectPhysics && inputHandler)
	{
		if ((isESD && inputHandler->GetEventSelection()) || isAOD)
		{
			isSelected = inputHandler->IsEventSelected();
			if (fExcludeTriggerMask && (isSelected & fExcludeTriggerMask))
				isRejected = kTRUE;
			if (fTriggerLogic == kAny)
				isSelected &= fTriggerMask;
			else if (fTriggerLogic == kExact)
				isSelected = ((isSelected & fTriggerMask) == fTriggerMask);
		}
	}

	//before physics selection
	fEventStat->Fill(kAllEvents);
	if (isSelected == 0 || isRejected)
	{
		PostData(2, fEventStat);
		return;
	}
	//after physics selection
	fEventStat->Fill(kSelectedEvents);

	//V0and
	if (fTriggerOnV0AND)
	{
		if (isESD)
		{
			if (!fTriggerAnalysis->IsOfflineTriggerFired(static_cast<AliESDEvent *>(InputEvent()), AliTriggerAnalysis::kV0AND))
				return;
		}
		if (isAOD)
		{
			if (!((static_cast<AliAODEvent *>(InputEvent()))->GetVZEROData()->GetV0ADecision() == AliVVZERO::kV0BB &&
						(static_cast<AliAODEvent *>(InputEvent()))->GetVZEROData()->GetV0CDecision() == AliVVZERO::kV0BB))
				return;
		}
	}

	fEventStat->Fill(kV0andEvents);

	//Fill Event histograms before the event filter
	AliDielectronHistos *h = fDielectron->GetHistoManager();

	Double_t values[AliDielectronVarManager::kNMaxValues] = {0};
	Double_t valuesMC[AliDielectronVarManager::kNMaxValues] = {0};
	if (h)
		AliDielectronVarManager::SetFillMap(h->GetUsedVars());
	else
		AliDielectronVarManager::SetFillMap(0x0);
	AliDielectronVarManager::SetEvent(InputEvent());
	AliDielectronVarManager::Fill(InputEvent(), values);
	AliDielectronVarManager::Fill(InputEvent(), valuesMC);

	Bool_t hasMC = AliDielectronMC::Instance()->HasMC();
	if (hasMC)
	{
		if (AliDielectronMC::Instance()->ConnectMCEvent())
			AliDielectronVarManager::Fill(AliDielectronMC::Instance()->GetMCEvent(), valuesMC);
	}

	if (h)
	{
		if (h->GetHistogramList()->FindObject("Event_noCuts"))
			h->FillClass("Event_noCuts", AliDielectronVarManager::kNMaxValues, values);
		if (hasMC && h->GetHistogramList()->FindObject("MCEvent_noCuts"))
			h->FillClass("Event_noCuts", AliDielectronVarManager::kNMaxValues, valuesMC);
	}

	//event filter
	if (fEventFilter)
	{
		if (!fEventFilter->IsSelected(InputEvent()))
			return;
	}
	fEventStat->Fill(kFilteredEvents);

	//pileup
	if (fRejectPileup)
	{
		if (InputEvent()->IsPileupFromSPD(3, 0.8, 3., 2., 5.))
			return;
	}
	fEventStat->Fill(kPileupEvents);

	//bz for AliKF
	Double_t bz = InputEvent()->GetMagneticField();
	AliKFParticle::SetField(bz);

	AliDielectronPID::SetCorrVal((Double_t)InputEvent()->GetRunNumber());

	fDielectron->Process(InputEvent());

	Bool_t hasCand = kFALSE;
	if (fStoreLikeSign)
		hasCand = (fDielectron->HasCandidates() || fDielectron->HasCandidatesLikeSign());
	else
		hasCand = (fDielectron->HasCandidates());

	if (fStoreRotatedPairs)
		hasCand = (hasCand || fDielectron->HasCandidatesTR());

	if (fStoreEventsWithSingleTracks)
		hasCand = (hasCand || fDielectron->GetTrackArray(0) || fDielectron->GetTrackArray(1));

	AliAODHandler *aodH = (AliAODHandler *)((AliAnalysisManager::GetAnalysisManager())->GetOutputEventHandler());
	AliAODExtension *extDielectron = aodH->GetFilteredAOD("AliAOD.Dielectron.root");
	if (hasCand)
	{
		AliAODEvent *aod = aodH->GetAOD();
		AliDebug(2,Form("Select event with %d tracks.",aod->GetNumberOfESDTracks()));

		extDielectron->SelectEvent();
		Int_t ncandidates = fDielectron->GetPairArray(1)->GetEntriesFast();
		if (ncandidates == 1)
			fEventStat->Fill((kNbinsEvent));
		else if (ncandidates > 1)
			fEventStat->Fill((kNbinsEvent + 1));

    extDielectron->FinishEvent();
	}

  
  // Clear pair arrays -- from fDielectron->ClearArrays();
  for (Int_t i=0;i<11;++i){
    TObjArray* pairArr = (TObjArray *)((*(fDielectron->GetPairArraysPointer()))->UncheckedAt(i));
    if (pairArr) pairArr->Delete();
  }

	PostData(1, const_cast<THashList *>(fDielectron->GetHistogramList()));
	PostData(2, fEventStat);
	return;
} 
