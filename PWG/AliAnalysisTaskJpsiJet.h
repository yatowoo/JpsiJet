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

#include "AliAnalysisTaskSE.h"

class AliAnalysisTaskJpsiJet : public AliAnalysisTaskSE
{
public:
  AliAnalysisTaskJpsiJet();
  AliAnalysisTaskJpsiJet(const char *taskName);
  AliAnalysisTaskJpsiJet(const AliAnalysisTaskJpsiJet &obj);
  AliAnalysisTaskJpsiJet &operator=(const AliAnalysisTaskJpsiJet &others);
  virtual ~AliAnalysisTaskJpsiJet() { ; }
  // Interface for anslysis manager (based on TaskSE)
public:
  virtual void UserCreateOutputObjects() { ; }
  virtual void UserExec(Option_t * /*option*/) { ; }
  virtual void UserExecMix(Option_t * /*option*/) { ; }
  virtual Bool_t UserNotify() { return kTRUE; }
  virtual void NotifyRun() { ; }

private:
  ClassDef(AliAnalysisTaskJpsiJet, 0);
};

#endif /* ALIANALYSISTASK_JPSIJET_H */