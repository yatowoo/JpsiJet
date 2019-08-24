#ifndef ALIANALYSISTASK_JPSIJET_H
#define ALIANALYSISTASK_JPSIJET_H

/* Copyright(c) 1998-2019, ALICE Experiment at CERN, All rights reserved. *
 * See cxx source for full Copyright notice                               */

/**
 *  Analysis Task for J/psi production in jets
 * 
 * Pre-requisities
 *  - Physics selection
 *  - PID response
 *  - CDB connect
 *  - EMCal correction
 * 
 * Dependencies
 *  - PWGDQ/dielectron
 *  - PWGJE/EMCalJet
 *  - AOD Extension + Branch Replicator
 * 
 * Supported
 *  - Event: AOD
 *  - Period: Run2 pp 13TeV
 * 
 * Outputs
 *  - AliAOD.Dielectron.root: filtered AOD event with dielectron and jets
 *  - JpsiJetResult.root: histograms for QA and physic objects
 * 
 * Author: [Yìtāo WÚ](yitao.wu@cern.ch), USTC
 * Create Date: 2019.08.17
 * More details: [Analysis Repo](https://github.com/yatowoo/JpsiJet) 
**/

#include "THistManager.h"

#include "AliAODEvent.h"
#include "AliAnalysisCuts.h"
#include "AliEmcalJetTask.h"

#include "AliAnalysisTaskSE.h"

class AliAnalysisCuts;

class AliAnalysisTaskJpsiJet : public AliAnalysisTaskSE
{
public:
  AliAnalysisTaskJpsiJet();
  AliAnalysisTaskJpsiJet(const char *taskName);
  virtual ~AliAnalysisTaskJpsiJet();
  // Interface for anslysis manager (based on TaskSE)
public:
  virtual void LocalInit();
  virtual void UserCreateOutputObjects();
  virtual void UserExec(Option_t * /*option*/);
  virtual void UserExecMix(Option_t * /*option*/) { ; }
  virtual Bool_t UserNotify() { return kTRUE; }
  virtual void NotifyRun() { ; }
  virtual void Terminate(Option_t *option="");

  // Histograms
  void InitHistogramsForEventQA(const char* histClass);
  void InitHistogramsForJetQA(const char* histClass);
  void FillHistogramsForEventQA(const char* histClass);
  void FillHistogramsForJetQA(const char* histClass);

  // Jet finder task
  // -- These sub-tasks are managed by this task, not by AliAnalysisManager.
public:
  void AddTaskEmcalJet(
    const TString nTracks = "usedefault",
    const TString nClusters = "",
    const AliJetContainer::EJetAlgo_t jetAlgo = AliJetContainer::antikt_algorithm,
    const Double_t radius = 0.4,
    const AliJetContainer::EJetType_t jetType = AliJetContainer::kFullJet,
    const Double_t minTrPt = 0.15,
    const Double_t minClPt = 0.30,
    const Double_t ghostArea = 0.005,
    const AliJetContainer::ERecoScheme_t reco = AliJetContainer::pt_scheme,
    const TString tag = "Jet",
    const Double_t minJetPt = 0.0,
    const Bool_t lockTask = kTRUE,
    const Bool_t bFillGhosts = kFALSE);
  void InitJetFinders();

  // Event selection
public:
  void     SetTrigger(UInt_t trigger){fSelectedTrigger = trigger;}
  UInt_t GetTrigger(){return fSelectedTrigger;}
  void     SetTriggerClasses(TString trigClasses){fSelectedTriggerClasses = trigClasses;}
  TString  GetTriggerClasses(){return fSelectedTriggerClasses;}
  TString  GetTriggerTag(){return fFiredTriggerTag;}
  void     SetRejectPileup(Bool_t rejectPileup){fRejectPileup = rejectPileup;}
  Bool_t   GetRejectPileup(){return fRejectPileup;}
  void     SetEventFilter(AliAnalysisCuts *eventCuts){fEventFilter = eventCuts;}
  void     PrintEventFilter(){;}

/**
 *  Data members 
 **/
public:
enum EventStatus{ // for histogram event stats
  kAllInAOD,
  kPhysSelected,
  kFiltered,
  kV0ANDtrigger,
  kTRDtrigger,
  kAfterPileUp,
  kWithSinglePair,
  kWithMultiPair,
  kWithPairInJet,
  kEventStatusN
};

private:
  AliAODEvent       *fAOD; // Input AOD event
  TObjArray         *fJetTasks; // Jet finder tasks
  TObjArray         *fJetContainers; // Jet container
  UInt_t             fSelectedTrigger; // Event offline trigger
  TString            fSelectedTriggerClasses; // Event fired trigger classes (separated by ';')
  TString            fFiredTriggerTag; // MB, EG1, EG2, DG1, DG2 
  Bool_t             fRejectPileup;
  Bool_t             fIsPileup;
  AliAnalysisCuts   *fEventFilter;

// Histograms
  THistManager      *fHistos;

private:
  ClassDef(AliAnalysisTaskJpsiJet, 0);
};

#endif /* ALIANALYSISTASK_JPSIJET_H */