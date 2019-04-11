/**
 * J/psi filter for nano AOD production
 * 
 * Impelement <merge> interface for AliAODExtension 
 * Based on AliAnalysisTaskDielectronFilter
 * 
 * NOTICE: Test OK AliPhysics::vAN-20180414
 * 	FAILED on vAN-20181208 because of the private variables.
 *
 * By: Yìtāo WÚ <yitao@cern.ch>
*/

#ifndef YATO_JPSI_FILTER_TASK_H
#define YATO_JPSI_FILTER_TASK_H

#include "TH1D.h"

#include "AliLog.h"
#include "AliAODHandler.h"
#include "AliAODExtension.h"
#include "AliESDInputHandler.h"
#include "AliAODInputHandler.h"
#include "AliAnalysisManager.h"
#include "AliTriggerAnalysis.h"

#include "AliDielectron.h"
class AliDielectron;
#include "AliDielectronVarManager.h"
class AliDielectronVarManager;

#include "AliAnalysisTaskSE.h"

class AliESDInputHandler;
class AliAODInputHandler;
class AliAnalysisManager;
class AliTriggerAnalysis;

class YatoJpsiFilterTask : public AliAnalysisTaskSE {

public:
  YatoJpsiFilterTask();
  YatoJpsiFilterTask(const char* name);
  virtual ~YatoJpsiFilterTask(){}

  virtual void Init();
  virtual void UserExec(Option_t* option);

  Bool_t IsToMerge() { return fIsToMerge;}
  void SetToMerge(Bool_t isToMerge = kTRUE){ fIsToMerge = isToMerge;}

private:
	Bool_t fIsToMerge; // Option for AliAODExtension

/*Copy from AliAnalysisTaskDielectronFilter*/
public:
	enum ETriggerLogig {kAny, kExact};
  virtual void UserCreateOutputObjects();
	virtual void LocalInit() {Init();}
	void NotifyRun(){AliDielectronPID::SetCorrVal((Double_t)fCurrentRunNumber);}

	void UsePhysicsSelection(Bool_t phy=kTRUE) {fSelectPhysics=phy;}
	void SetTriggerMask(UInt_t mask) {fTriggerMask=mask;}
	UInt_t GetTriggerMask() const { return fTriggerMask; }
	void SetExcludeTriggerMask(ULong64_t mask) {fExcludeTriggerMask=mask;}
	UInt_t GetExcludeTriggerMask() const { return fExcludeTriggerMask; }
	void SetTriggerLogic(ETriggerLogig log) {fTriggerLogic=log;}
	ETriggerLogig GetTriggerLogic() const {return fTriggerLogic;}


	void SetDielectron(AliDielectron * const die) { fDielectron = die; }

	void SetStoreLikeSignCandidates(Bool_t storeLS) { fStoreLikeSign = storeLS; }
	void SetStoreRotatedPairs(Bool_t storeTR) { fStoreRotatedPairs = storeTR; }
	void SetStoreEventsWithSingleTracks(Bool_t storeSingleTrk) { fStoreEventsWithSingleTracks = storeSingleTrk; }
	void SetCreateNanoAODs(Bool_t storeTrackRef) { fCreateNanoAOD = storeTrackRef; }
	void SetStoreHeader(Bool_t storeHeader) { fStoreHeader = storeHeader; }
	void SetStoreEventplanes(Bool_t storeEventplanes) {fStoreEventplanes = storeEventplanes;}

	void SetEventFilter(AliAnalysisCuts * const filter) {fEventFilter=filter;}

private:
	enum {kAllEvents=0, kSelectedEvents, kV0andEvents, kFilteredEvents, kPileupEvents, kNbinsEvent};

	void SetHeaderData(AliAODHeader* hin, AliAODHeader* hout, Double_t values[AliDielectronVarManager::kNMaxValues]);


	AliDielectron *fDielectron;             // J/psi framework object

	Bool_t fSelectPhysics;                  // Whether to use physics selection
	UInt_t fTriggerMask;               // Event trigger mask
	UInt_t fExcludeTriggerMask;        // Triggers to exclude from the analysis
	Bool_t fTriggerOnV0AND;            // if to trigger on V0and
	Bool_t fRejectPileup;              // pileup rejection wanted

	TH1D *fEventStat;                  //! Histogram with event statistics

	ETriggerLogig fTriggerLogic;       // trigger logic: any or all bits need to be matching

	AliTriggerAnalysis *fTriggerAnalysis; //! trigger analysis class

	Bool_t fStoreLikeSign;        // flag to store like-sign candidates
	Bool_t fStoreRotatedPairs;    // flag to store rotation
	Bool_t fStoreEventsWithSingleTracks;    // flag to store events with a least one reconstructed track
	Bool_t fCreateNanoAOD;        // flag to create nanoAODs
	Bool_t fStoreHeader;          // flag to store header for all events
	Bool_t fStoreEventplanes;     // flag to store eventplane information in a seperated tree
	Bool_t AddMetadataToUserInfo(); // Function to add ProdInfo to Nano AOD Tree
	Bool_t Notify(); // Function to add ProdInfo to Nano AOD Tree

	AliAnalysisCuts *fEventFilter;     // event filter
	TList *fQnList; //! List for the storage of the output of the QnFramework needed for event-plane analysis since 2016

	ClassDef(YatoJpsiFilterTask, 1);

};
#endif // #ifndef YATO_JPSI_FILTER_TASK_H
