#ifndef __CINT__
#ifndef __MultiDie__
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
#endif
#endif // __MultiDie__

void SetupTrackCutsDieleData(AliDielectron *diele, Int_t cutDefinition, Bool_t isAOD, Bool_t isMC);
void SetupPairCutsDieleData(AliDielectron *diele, Int_t cutDefinition, Bool_t isAOD, Int_t trigger_index, Bool_t isMC);
void InitHistogramsDieleData(AliDielectron *diele, Int_t cutDefinition, Int_t trigger_index, Bool_t isAOD);
void InitCFDieleData(AliDielectron *diele, Int_t cutDefinition, Bool_t isAOD, Bool_t isMC);
void AddMCSignals(AliDielectron *diele);

AliESDtrackCuts *SetupESDtrackCutsDieleData(Int_t cutDefinition);

//
// Cut Definitions
//
TString namesDieleData = "EMCal_strict;RAW;EMCal;EMCal_loose";
enum CutType
{
	kEMCal_strict,
	kRAW,
	kEMCal,
	kEMCal_loose,
  kCutN = 4
};
TObjArray *arrNamesDieleData = namesDieleData.Tokenize(";");
const Int_t nDie = arrNamesDieleData->GetEntries();

AliDielectron *ConfigJpsi_cj_pp(Int_t cutDefinition, Bool_t isAOD = kFALSE, Int_t trigger_index = 0, Bool_t isMC=kFALSE)
{
	//
	// Setup the instance of AliDielectron
	//

	// create the actual framework object
	TString name = Form("%02d", cutDefinition);
	if (cutDefinition < arrNamesDieleData->GetEntriesFast())
	{
		name = arrNamesDieleData->At(cutDefinition)->GetName();
	}
	AliDielectron *diele = new AliDielectron(Form("%s", name.Data()), Form("Track cuts: %s", name.Data()));

	//profile

	TFile *file = TFile::Open("$ALICE_PHYSICS/PWGDQ/dielectron/files/estimators.root");

	TProfile *corr;
	corr = (TProfile *)file->Get("SPDmult10_LHC16l");

	TObjArray obj_corr;
	obj_corr.Add(corr);

	AliDielectronVarManager::InitEstimatorObjArrayAvg(&obj_corr);

	//==========================

	//MC signals
	//from Marcel December 13th
	if (isMC)
	{
		AddMCSignals(diele);
		printf(" Add %d MC signals \n", diele->GetMCSignals()->GetEntriesFast());
	}

	// Cuts Setup
  //diele->SetCutQA(kTRUE);
  if(cutDefinition != kRAW){
	  SetupTrackCutsDieleData(diele, cutDefinition, isAOD, isMC);
	  SetupPairCutsDieleData(diele, cutDefinition, isAOD, trigger_index, isMC);
  }else{
    diele->SetNoPairing(kTRUE);
  }
	// Histogram Setup
	InitHistogramsDieleData(diele, cutDefinition, trigger_index, isAOD);
	if (isMC)
	{
		InitCFDieleData(diele, cutDefinition, isAOD, isMC);
	}

	// Track Rotator
	/*
	AliDielectronTrackRotator *rot=new AliDielectronTrackRotator;
	rot->SetIterations(20);
	rot->SetConeAnglePhi(TMath::Pi()/2);
	rot->SetStartAnglePhi(TMath::Pi()/2);
	diele->SetTrackRotator(rot);
	*/

	// Event Mixing
	/*
  AliDielectronMixingHandler *mix=new AliDielectronMixingHandler;
  mix->AddVariable(AliDielectronVarManager::kZvPrim, 100,-10.,10.);
  mix->SetMixType(AliDielectronMixingHandler::kAll);
  mix->SetDepth(20);
  diele->SetMixingHandler(mix);
 */

	return diele;
}

//______________________________________________________________________________________
void SetupTrackCutsDieleData(AliDielectron *diele, Int_t cutDefinition, Bool_t isAOD, Bool_t isMC)
{
	//
	// Setup the track cuts
	//
	//ESD quality cuts DielectronTrackCuts
	if (!isAOD)
	{
		diele->GetTrackFilter().AddCuts(SetupESDtrackCutsDieleData(cutDefinition));
	}
	else
	{
		AliDielectronTrackCuts *trackCuts = new AliDielectronTrackCuts("trackCuts", "trackCuts");
		trackCuts->SetClusterRequirementITS(AliDielectronTrackCuts::kSPD, AliDielectronTrackCuts::kAny);
		trackCuts->SetRequireTPCRefit(kTRUE);
		trackCuts->SetRequireITSRefit(kTRUE);
		diele->GetTrackFilter().AddCuts(trackCuts);
	}

	// Track cuts for electron PID
	AliDielectronVarCuts *ePID = new AliDielectronVarCuts("ePidCut", "Track cuts for electron PID");
	ePID->AddCut(AliDielectronVarManager::kKinkIndex0, 0.);
	//AOD additional cuts
	//
	if (isAOD)
	{
		// Track quality cuts
		ePID->AddCut(AliDielectronVarManager::kNclsTPC, 85., 160.);
		ePID->AddCut(AliDielectronVarManager::kEta, -0.9, 0.9);
		ePID->AddCut(AliDielectronVarManager::kImpactParXY, -1., 1.);
		ePID->AddCut(AliDielectronVarManager::kImpactParZ, -3., 3.);
		ePID->AddCut(AliDielectronVarManager::kITSLayerFirstCls, 0., 4.);
		ePID->AddCut(AliDielectronVarManager::kTPCchi2Cl, 0., 4.);
		// PID cuts
		if( cutDefinition == kEMCal){
			ePID->AddCut(AliDielectronVarManager::kPt, 1.0, 1e30);
			ePID->AddCut(AliDielectronVarManager::kTPCnSigmaEle, -2.25, 3.0);
		}
		else if( cutDefinition == kEMCal_loose){
			ePID->AddCut(AliDielectronVarManager::kPt, 0.7, 1e30);
			ePID->AddCut(AliDielectronVarManager::kTPCnSigmaEle, -3.0, 3.0);
		}
		else if( cutDefinition == kEMCal_strict){
			ePID->AddCut(AliDielectronVarManager::kPt, 1.0, 1e30);
			ePID->AddCut(AliDielectronVarManager::kTPCnSigmaEle, -2.0, 3.0);
			// Exclude hadrons
			ePID->AddCut(AliDielectronVarManager::kTPCnSigmaPio, -100.0, 1.0, kTRUE);
			ePID->AddCut(AliDielectronVarManager::kTPCnSigmaKao, -100.0, 3.0, kTRUE);
			ePID->AddCut(AliDielectronVarManager::kTPCnSigmaPro, -100.0, 3.0, kTRUE);
		}
	}

	diele->GetTrackFilter().AddCuts(ePID);
}

//______________________________________________________________________________________
void SetupPairCutsDieleData(AliDielectron *diele, Int_t cutDefinition, Bool_t isAOD, Int_t trigger_index, Bool_t isMC)
{
	/*
		Jpsi cuts
	*/
	//Invariant mass and rapidity selection
	AliDielectronVarCuts *pairCut = new AliDielectronVarCuts("jpsiCuts", "1<M<5 + |Y|<.9");
	pairCut->AddCut(AliDielectronVarManager::kM, 1.0, 5.0);
	pairCut->AddCut(AliDielectronVarManager::kY, -0.9, 0.9);
	pairCut->AddCut(AliDielectronVarManager::kPt, 1, 1e30);

	//EMC7 - L0 trigger
	if (trigger_index == 1)
		pairCut->AddCut(AliDielectronVarManager::kPt, 1., 1e30);
	// EMCEGA - L1 trigger
	else if (trigger_index == 2)
		pairCut->AddCut(AliDielectronVarManager::kPt, 1., 1e30);
	// EMCEGA - EG1
	else if (trigger_index == 3)
		pairCut->AddCut(AliDielectronVarManager::kPt, 7, 1e30);
	// EMCEGA - EG2
	else if (trigger_index == 4 || trigger_index == 40)
		pairCut->AddCut(AliDielectronVarManager::kPt, 5, 1e30);
	// EMCEG2 - EG1
	else if (trigger_index == 6 || trigger_index == 60)
		pairCut->AddCut(AliDielectronVarManager::kPt, 11, 1e30);

	diele->GetPairFilter().AddCuts(pairCut);
	
	/*
		Leg cuts
	*/
	AliDielectronVarCuts *emcCut = new AliDielectronVarCuts("CutEMCAL", "Jpsi leg cuts for EMCal");
	// E/p for electron
	if( cutDefinition == kEMCal_loose)
		emcCut->AddCut(AliDielectronVarManager::kEMCALEoverP, 0.75, 1.35);
	else
		emcCut->AddCut(AliDielectronVarManager::kEMCALEoverP, 0.8, 1.3);

	// EMCal energy threshold
	// EMC7 - L0 trigger (2.5 GeV - 2016)
	// EMCEGA - L1 trigger, use L0 threshold for nano AOD filter
	if (trigger_index == 1 || trigger_index == 2)
		emcCut->AddCut(AliDielectronVarManager::kEMCALE, 3, 1e30);
	// EMCEGA - EG1/DG1, LHC16 except 16k
	else if (trigger_index == 3 || trigger_index == 30)
		emcCut->AddCut(AliDielectronVarManager::kEMCALE, 7, 1e30);
	// EMCEGA - EG2/DG2
	else if (trigger_index == 4 || trigger_index == 40)
		emcCut->AddCut(AliDielectronVarManager::kEMCALE, 5, 50.);
	// EMCEGA - EG1/DG1 (16k)
	else if (trigger_index == 6 || trigger_index == 60)
		emcCut->AddCut(AliDielectronVarManager::kEMCALE, 11, 50.);
	else
		emcCut->AddCut(AliDielectronVarManager::kEMCALE, 1, 50.);

	// Exclude DCal for EG1/EG2
	if ( trigger_index == 3 || trigger_index == 4 || trigger_index == 6)
		emcCut->AddCut(AliDielectronVarManager::kPhi, 4.377, 5.7071, kTRUE); 
	// Exclude EMCal for DG1/DG2
	else if (trigger_index == 30 || trigger_index == 40 || trigger_index == 60)
		emcCut->AddCut(AliDielectronVarManager::kPhi, 1.396, 3.2637, kTRUE);

	AliDielectronPairLegCuts *varpair = new AliDielectronPairLegCuts();
	varpair->GetLeg1Filter().AddCuts(emcCut);
	varpair->GetLeg2Filter().AddCuts(emcCut);
	varpair->SetCutType(AliDielectronPairLegCuts::kAnyLeg);
	diele->GetPairFilter().AddCuts(varpair);
}

//______________________________________________________________________________________
AliESDtrackCuts *SetupESDtrackCutsDieleData(Int_t cutDefinition)
{
	//
	// Setup default AliESDtrackCuts
	//
	AliESDtrackCuts *esdTrackCuts = new AliESDtrackCuts;

	// basic track quality cuts  (basicQ)
	esdTrackCuts->SetMaxDCAToVertexZ(3.0);
	esdTrackCuts->SetMaxDCAToVertexXY(1.0);

	esdTrackCuts->SetEtaRange(-0.9, 0.9);

	esdTrackCuts->SetAcceptKinkDaughters(kFALSE);
	esdTrackCuts->SetRequireITSRefit(kTRUE);
	esdTrackCuts->SetRequireTPCRefit(kTRUE);

	esdTrackCuts->SetPtRange(1, 1e30);

	esdTrackCuts->SetMinNClustersTPC(85);
	esdTrackCuts->SetMaxChi2PerClusterTPC(4);

	// default SPD any
	esdTrackCuts->SetClusterRequirementITS(AliESDtrackCuts::kSPD, AliESDtrackCuts::kAny);

	return esdTrackCuts;
}


void InitHistogramsForEvent(AliDielectronHistos* histos, const char* histClass){
	histos->AddClass(histClass);
	// Event Primary vertex and diamond (IP) stats.
	histos->UserHistogram(histClass, "VtxZ", "Vertex Z;Z[cm];#events", 1000, -50., 50., AliDielectronVarManager::kZvPrim);
	histos->UserHistogram(histClass, "VtxX", "Vertex X;X[cm];#events", 1000, -1.0, 1.0, AliDielectronVarManager::kXvPrim);
	histos->UserHistogram(histClass, "VtxY", "Vertex Y;Y[cm];#events", 2000, -1.0, 1.0, AliDielectronVarManager::kYvPrim);
  histos->UserHistogram(histClass, "NVContrib","Number of Vertex contributors", 1000, 0, 1000, AliDielectronVarManager::kNVtxContrib );
	// Event track and SPD (tracklets) stats.
	histos->UserHistogram(histClass, "kNTrk", "Number of tracks;kNTrk;Entries", 4000, 0., 4000., AliDielectronVarManager::kNTrk);
	histos->UserHistogram(histClass, "kNaccTrcklts", "Number of accepted SPD tracklets in |eta|<1.6;kNaccTrcklts;Entries", 1000, 0., 1000., AliDielectronVarManager::kNaccTrcklts);
	histos->UserHistogram(histClass, "kNaccTrcklts10Corr", "kNaccTrcklts10Corr;kNaccTrcklts10Corr;Entries", 500, 0., 500., AliDielectronVarManager::kNaccTrcklts10Corr);
	histos->UserHistogram(histClass, "VtxZ_kNaccTrcklts10Corr", "VtxZ vs. kNaccTrcklts10Corr;VtxZ;kNaccTrcklts10Corr", 800, -40., 40., 500, 0., 500., AliDielectronVarManager::kZvPrim, AliDielectronVarManager::kNaccTrcklts10Corr);
	//new multiplicity estimator: V0
	histos->UserHistogram(histClass, "kMultV0", "kMultV0;kMultV0;Entries", 1000, 0., 1000., AliDielectronVarManager::kMultV0);
	histos->UserHistogram(histClass, "kMultV0A", "kMultV0;kMultV0;Entries", 1000, 0., 1000., AliDielectronVarManager::kMultV0A);
	histos->UserHistogram(histClass, "kMultV0C", "kMultV0;kMultV0;Entries", 1000, 0., 1000., AliDielectronVarManager::kMultV0C);
	// 2D
	histos->UserHistogram(histClass, "kMultV0A_kMultV0C", "kMultV0A vs. kMultV0C;kMultV0A;kMultV0C", 1000, 0., 1000., 1000, 0., 1000., AliDielectronVarManager::kMultV0A, AliDielectronVarManager::kMultV0C);
	histos->UserHistogram(histClass, "kMultV0_kMultV0A", "kMultV0 vs. kMultV0A;kMultV0;kMultV0A", 1000, 0., 1000., 1000, 0., 1000., AliDielectronVarManager::kMultV0, AliDielectronVarManager::kMultV0A);
	histos->UserHistogram(histClass, "kMultV0_kMultV0C", "kMultV0 vs. kMultV0C;kMultV0;kMultV0C", 1000, 0., 1000., 1000, 0., 1000., AliDielectronVarManager::kMultV0, AliDielectronVarManager::kMultV0C);
	// vs Vertex Z
	histos->UserHistogram(histClass, "VtxZ_kMultV0A", "VtxZ vs. kMultV0A;VtxZ;kMultV0A", 300, -15., 15., 1000, 0., 1000., AliDielectronVarManager::kZvPrim, AliDielectronVarManager::kMultV0A);
	histos->UserHistogram(histClass, "VtxZ_kMultV0C", "VtxZ vs. kMultV0C;VtxZ;kMultV0C", 300, -15., 15., 1000, 0., 1000., AliDielectronVarManager::kZvPrim, AliDielectronVarManager::kMultV0C);
	histos->UserHistogram(histClass, "VtxZ_kMultV0", "VtxZ vs. kMultV0;VtxZ;kMultV0", 300, -15., 15., 1000, 0., 1000., AliDielectronVarManager::kZvPrim, AliDielectronVarManager::kMultV0);

  // Dielectron info.
  histos->UserHistogram(histClass, "Nelectrons", "Number of tracks/electron selected by AliDielectron after cuts;N_{e};#events", 50, 0, 50, AliDielectronVarManager::kTracks);
	histos->UserHistogram(histClass, "Npairs", "Number of Ev1PM pair candidates after all cuts;J/#psi candidates;#events", 20, 0, 20, AliDielectronVarManager::kPairs);
}
//______________________________________________________________________________________
void InitHistogramsDieleData(AliDielectron *diele, Int_t cutDefinition, Int_t trigger_index, Bool_t isAOD)
{
	//
	// Initialise the histograms
	//

	//Setup histogram Manager
	AliDielectronHistos *histos = new AliDielectronHistos(diele->GetName(), diele->GetTitle());

	//Initialise histogram classes
	histos->SetReservedWords("Track;Pair");
	//Track classes
	for (Int_t i = 0; i < 2; ++i)
	{
		histos->AddClass(Form("Track_%s", AliDielectron::TrackClassName(i)));
	}
  if(cutDefinition != kRAW){
	  //Pair classes
	  for (Int_t i = 0; i < 3; ++i)
      histos->AddClass(Form("Pair_%s", AliDielectron::PairClassName(i)));
	  //Legs from pair
	  for (Int_t i = 0; i < 3; ++i)
  		histos->AddClass(Form("Track_Legs_%s", AliDielectron::PairClassName(i)));
  }
	//track rotation
	//histos->AddClass(Form("Pair_%s",PairClassName(AliDielectron::kEv1PMRot)));
	//histos->AddClass(Form("Track_Legs_%s",PairClassName(AliDielectron::kEv1PMRot)));

  if(cutDefinition == kRAW || trigger_index == 2){
	/*
		Histogram for Event (before event filter)
	*/
	  InitHistogramsForEvent(histos, "Event_noCuts");
	/*
		Histogram for Event
	*/
	  InitHistogramsForEvent(histos, "Event");
    // Event vs Track - by AliDielectronEvtVsTrkHist
    //histos->AddClass("EvtVsTrk");
  }
	
	/*
	  Histogram for Track
	*/
	// Track kinetics parameter
	histos->UserHistogram("Track", "Pt", "Pt;Pt [GeV/c];#tracks", 2000, 0, 100, AliDielectronVarManager::kPt, kTRUE);
	histos->UserHistogram("Track", "Eta_Phi", "Eta Phi Map; Eta; Phi;#tracks",
												100, -1, 1, 144, 0, TMath::TwoPi(), AliDielectronVarManager::kEta, AliDielectronVarManager::kPhi, kTRUE);
	histos->UserHistogram("Track", "dXY", "dXY;dXY [cm];#tracks", 1000, -50, 50, AliDielectronVarManager::kImpactParXY, kTRUE);
	histos->UserHistogram("Track", "dZ", "dZ;dZ [cm];#tracks", 1000, -50., 50., AliDielectronVarManager::kImpactParZ, kTRUE);
	// Tracking quality
	histos->UserHistogram("Track", "ITS_FirstCls", "ITS First Cluster;Layer No. of ITS 1st cluster;#Entries", 6, 0., 6., AliDielectronVarManager::kITSLayerFirstCls, kTRUE);
	histos->UserHistogram("Track", "TPCnCls", "Number of Clusters TPC;TPC number clusteres;#tracks", 160, 0, 160, AliDielectronVarManager::kNclsTPC, kTRUE);
	histos->UserHistogram("Track", "TPCchi2Cl", "Chi-2/Clusters TPC;Chi2/ncls number clusteres;#tracks", 100, 0, 10, AliDielectronVarManager::kTPCchi2Cl, kTRUE);
	// PID - TPC
	histos->UserHistogram("Track", "dEdx_P", "dEdx vs PinTPC;P [GeV];TPC signal (a.u.);#tracks",
												800, 0., 40., 800, 20., 200., AliDielectronVarManager::kPIn, AliDielectronVarManager::kTPCsignal, kTRUE);
	histos->UserHistogram("Track", "dEdx_Pt", "dEdx vs Pt;Pt [GeV];TPC signal (a.u.);#tracks",
												800, 0., 40., 800, 20., 200., AliDielectronVarManager::kPt, AliDielectronVarManager::kTPCsignal, kTRUE);
	histos->UserHistogram("Track", "dEdx_Phi", "dEdx vs #phi;#phi [rad];TPC signal (a.u.);#tracks",
												200, 0., 2 * TMath::Pi(), 800, 20., 200., AliDielectronVarManager::kPhi, AliDielectronVarManager::kTPCsignal, kTRUE);
	histos->UserHistogram("Track", "dEdx_Eta", "dEdx vs #eta;#eta;TPC signal (a.u.);#tracks",
												200, -1., 1., 800, 20., 200., AliDielectronVarManager::kEta, AliDielectronVarManager::kTPCsignal, kTRUE);
	histos->UserHistogram("Track", "TPCnSigmaEle_P", "n#sigma_{e}(TPC) vs P_{in} TPC;P_{in} [GeV];n#sigma_{e}(TPC);#tracks",
												800, 0., 40., 800, -12., 12., AliDielectronVarManager::kPIn, AliDielectronVarManager::kTPCnSigmaEle, kTRUE);
	histos->UserHistogram("Track", "TPCnSigmaEle_Pt", "n#sigma_{e}(TPC) vs Pt;Pt [GeV];n#sigma_{e}(TPC);#tracks",
												800, 0., 40., 800, -12., 12., AliDielectronVarManager::kPt, AliDielectronVarManager::kTPCnSigmaEle, kTRUE);
	histos->UserHistogram("Track", "TPCnSigmaEle_Phi", "n#sigma_{e}(TPC) vs #phi;#phi [rad];n#sigma_{e}(TPC);#tracks",
												200, 0., 2 * TMath::Pi(), 800, -12., 12., AliDielectronVarManager::kPhi, AliDielectronVarManager::kTPCnSigmaEle, kTRUE);
	histos->UserHistogram("Track", "TPCnSigmaEle_Eta", "n#sigma_{e}(TPC) vs #eta;#eta;n#sigma_{e}(TPC);#tracks",
												200, -1., 1., 800, -12., 12., AliDielectronVarManager::kEta, AliDielectronVarManager::kTPCnSigmaEle, kTRUE);
	histos->UserHistogram("Track", "dEdx_nSigmaEMCal", "dEdx vs n#sigma_{e}(EMCAL);n#sigma_{e}(EMCAL);TPC signal (a.u.);#tracks",
												200, -5., 5., 800, 20., 200., AliDielectronVarManager::kEMCALnSigmaEle, AliDielectronVarManager::kTPCsignal, kTRUE);
	histos->UserHistogram("Track", "dEdx_TPCnSigmaEle", "dEdx vs n#sigma_{e}(TPC);n#sigma_{e}(TPC);TPC signal (a.u.);#tracks",
												100, -10., 10., 800, 20., 200., AliDielectronVarManager::kTPCnSigmaEle, AliDielectronVarManager::kTPCsignal, kTRUE);
	// Track - EMCal
	histos->UserHistogram("Track", "EMCalE", "EmcalE;Cluster Energy [GeV];#Clusters",
												200, 0., 40., AliDielectronVarManager::kEMCALE, kTRUE);
	histos->UserHistogram("Track", "EMCalE_Pt", "Cluster energy vs. pT; EMCal_E;pT;#tracks",
												200, 0., 40., 800, 0., 40, AliDielectronVarManager::kEMCALE, AliDielectronVarManager::kPt, kTRUE);
	//Ecluster versus Phi to separate EMCal and DCal
	histos->UserHistogram("Track", "EMCalE_Phi", "Cluster energy vs. #phi; EMCal_E;Phi;#tracks",
												200, 0., 40., 200, 0., TMath::TwoPi(), AliDielectronVarManager::kEMCALE, AliDielectronVarManager::kPhi, kTRUE);
	histos->UserHistogram("Track", "EMCalE_Eta", "Cluster energy vs. #eta; EMCal_E;Eta;#tracks",
												200, 0., 40., 200, -1.0, 1.0, AliDielectronVarManager::kEMCALE, AliDielectronVarManager::kEta, kTRUE);
	// PID - EMCal
		// E/p ratio
	histos->UserHistogram("Track", "EoverP", "EMCal E/p ratio;E/p;#Clusters",
												200, 0., 2., AliDielectronVarManager::kEMCALEoverP, kTRUE);
	histos->UserHistogram("Track", "EoverP_pt", "E/p ratio vs Pt;Pt (GeV/c);E/p;#tracks",
												200, 0., 40., 200, 0., 2., AliDielectronVarManager::kPt, AliDielectronVarManager::kEMCALEoverP, kTRUE);
	histos->UserHistogram("Track", "EoverP_Phi", "E/p ratio vs #phi;Phi;E/p;#tracks",
												200, 0., TMath::TwoPi(), 200, 0., 2., AliDielectronVarManager::kPhi, AliDielectronVarManager::kEMCALEoverP, kTRUE);
	histos->UserHistogram("Track", "EoverP_Eta", "E/p ratio vs #eta;Eta;E/p;#tracks",
												200, -1.0, 1.0, 200, 0., 2., AliDielectronVarManager::kEta, AliDielectronVarManager::kEMCALEoverP, kTRUE);
	// EMCal nSigma electron
	histos->UserHistogram("Track", "EMCALnSigmaE_pt", "n#sigma_{e} vs Pt;Pt (GeV/c);n#sigma_{e};#tracks",
												200, 0., 40., 200, -12, 12, AliDielectronVarManager::kPt, AliDielectronVarManager::kEMCALnSigmaEle, kTRUE);
	histos->UserHistogram("Track", "EMCALnSigmaE_Phi", "n#sigma_{e} vs #phi;Phi;n#sigma_{e};#tracks",
												200, 0., TMath::TwoPi(), 200, -12, 12, AliDielectronVarManager::kPhi, AliDielectronVarManager::kEMCALnSigmaEle, kTRUE);
	histos->UserHistogram("Track", "EMCALnSigmaE_Eta", "n#sigma_{e} vs #eta;Eta;n#sigma_{e};#tracks",
												200, -1.0, 1.0, 200, 0., 2., AliDielectronVarManager::kEta, AliDielectronVarManager::kEMCALnSigmaEle, kTRUE);
	histos->UserHistogram("Track", "EMCALnSigmaEle_EoverP", "n#sigma_{e}(EMCal) vs E/p;E/p;n#sigma_{e}(EMCal);#tracks",
												200, 0., 2., 200, -12., 12., AliDielectronVarManager::kEMCALEoverP, AliDielectronVarManager::kEMCALnSigmaEle, kTRUE);
	// PID - TPC + EMCal
	histos->UserHistogram("Track", "dEdx_EoverP", "dEdx vs E/p;E/P;TPC signal (a.u.);#tracks",
												200, 0., 2., 800, 20., 200., AliDielectronVarManager::kEMCALEoverP, AliDielectronVarManager::kTPCsignal, kTRUE);
	histos->UserHistogram("Track", "TPCnSigmaEle_EoverP", "n#sigma_{e}(TPC) vs E/p;E/p;n#sigma_{e}(TPC);#tracks",
												200, 0., 2., 200, -12., 12., AliDielectronVarManager::kEMCALEoverP, AliDielectronVarManager::kTPCnSigmaEle, kTRUE);
	histos->UserHistogram("Track", "dEdx_EMCALnSigmaE", "dEdx vs n#sigma_{e}(EMCAL);n#sigma_{e}(EMCAL);TPC signal (a.u.);#tracks",
												200, -12., 12., 800, 20., 200., AliDielectronVarManager::kEMCALEoverP, AliDielectronVarManager::kTPCsignal, kTRUE);
	histos->UserHistogram("Track", "nSigmaTPC_EMCal", "n#sigma_{e}(TPC vs EMCAL);n#sigma_{e}(EMCAL);n#sigma_{e}(TPC);#tracks",
												200, -5., 5., 200, -12., 12., AliDielectronVarManager::kEMCALnSigmaEle, AliDielectronVarManager::kTPCnSigmaEle, kTRUE);

	/*
	// Histograms for Pair
	*/
	histos->UserHistogram("Pair", "InvMass", "Inv.Mass;Inv. Mass (GeV/c^{2});#pairs/(40 MeV/c^{2})",
												100, 1.0, 5.0, AliDielectronVarManager::kM);
	histos->UserHistogram("Pair", "pT", "Pt;Pt (GeV/c);#pairs",
												2000, 0., 100.0, AliDielectronVarManager::kPt);
	histos->UserHistogram("Pair", "Eta_Phi", "#eta-#phi map of dielectron pairs;#eta_{ee};#phi_{ee};#pairs",
                        200, -1, 1, 200, 0., 10, AliDielectronVarManager::kEta, AliDielectronVarManager::kPhi);
	histos->UserHistogram("Pair", "Rapidity", "Rapidity;Rapidity;#pairs",
												200, -1., 1., AliDielectronVarManager::kY);
	histos->UserHistogram("Pair", "OpeningAngle", "Opening angle / rad;#pairs",
												50, 0., 3.15, AliDielectronVarManager::kOpeningAngle);

	histos->UserHistogram("Pair", "PseudoProperTime", "Pseudoproper decay length; pseudoproper-decay-length[cm];#pairs / 40#mum",
												150, -0.3, 0.3, AliDielectronVarManager::kPseudoProperTime);
	histos->UserHistogram("Pair", "InvMass_Pt", "Inv. Mass vs Pt;Pt (GeV/c); Inv. Mass (GeV/c^{2})",
												200, 0., 40., 100, 1.0, 5.0, AliDielectronVarManager::kPt, AliDielectronVarManager::kM);
	histos->UserHistogram("Pair", "OpeningAngle_Pt", "Opening angle vs p_{T} ;p_{T} (GeV/c); angle",
												200, 0., 40., 200, 0, TMath::Pi(), AliDielectronVarManager::kPt, AliDielectronVarManager::kOpeningAngle);
	//InvMass versus Proper time
	histos->UserHistogram("Pair", "InvMass_ProperTime", "InvMass vs. ProperTime;pseudoproper-decay-length[cm]; Inv. Mass [GeV]",
												120, -0.3, 0.3, 100, 1.0, 5.0, AliDielectronVarManager::kPseudoProperTime, AliDielectronVarManager::kM);

	diele->SetHistogramManager(histos);
}

void InitCFDieleData(AliDielectron *diele, Int_t cutDefinition, Bool_t isAOD, Bool_t isMC)
{
	//
	// Setupd the CF Manager if needed
	//
	if (cutDefinition == 0 || cutDefinition == 1)
	{

		AliDielectronCF *cf = new AliDielectronCF(diele->GetName(), diele->GetTitle());
		//for centrality selection
		// cf->AddVariable(AliDielectronVarManager::kCentralitySPDTracklets, "0,20,40,60,100, 200, 300");
		//cf->AddVariable(AliDielectronVarManager::kNaccTrcklts10Corr, "0,30,60,100,1000");
		//pair variables

		cf->AddVariable(AliDielectronVarManager::kPt, "5.0,7.0,9.0,11.0,15.0,20.0,25.0,30.0,35.0,40.0");
		cf->AddVariable(AliDielectronVarManager::kM, 125, 0., 125 * .04); //40Mev Steps

		cf->AddVariable(AliDielectronVarManager::kPairType, 3, 0, 3);

		// cf->AddVariable(AliDielectronVarManager::kOpeningAngle,16,0,3.2);
		//cf->AddVariable(AliDielectronVarManager::kEta,"-1.0,-0.9,-0.8,0,0.8,0.9,1.0");
		// cf->AddVariable(AliDielectronVarManager::kY,40,-1.,1.);
		// cf->AddVariable(AliDielectronVarManager::kPhi,"0,1.0,2.0,2.5,3.0,3.5,4.0,4.5,5.0,6.0,6.5");
		// cf->AddVariable(AliDielectronVarManager::kPseudoProperTime,"-3.0,-2.0,-1.0,-0.9,-0.8,-0.6,-0.4,-0.2, 0,0.2, 0.4, 0.6,0.8,0.9,1.0, 2.0, 3.0");
		//cf->AddVariable(AliDielectronVarManager::kPseudoProperTimeErr,200,0.,0.1);
		//cf->AddVariable(AliDielectronVarManager::kPseudoProperTimeResolution,400,-0.1,0.1);
		//cf->AddVariable(AliDielectronVarManager::kPseudoProperTimePull,400,-0.1,0.1);
		//cf->AddVariable(AliDielectronVarManager::kChi2NDF,100, 0., 20.);

		//leg variables

		cf->AddVariable(AliDielectronVarManager::kPt, "1.0,3.0,5.0,6.0,7.0,8.0,9.0,10.0,11.0,12.0,15.0,20.0, 25.,30.,35.0,40.0", kTRUE);

		//cf->AddVariable(AliDielectronVarManager::kNclsTPC,"70, 80, 85, 90, 100,160",kTRUE);
		//cf->AddVariable(AliDielectronVarManager::kTPCchi2Cl,5, 0., 10.,kTRUE);
		//cf->AddVariable(AliDielectronVarManager::kTPCsignalN,160,-0.5,159.5,kTRUE);

		cf->AddVariable(AliDielectronVarManager::kEta, "-1.0,-0.9,-0.8,0,0.8,0.9,1.0", kTRUE);
		// cf->AddVariable(AliDielectronVarManager::kPhi,"0,1.0,2.0,2.5,3.0,3.5,4.0,4.5,5.0,6.0,6.5",kTRUE);

		cf->AddVariable(AliDielectronVarManager::kEMCALE, "4.5, 5.0, 5.5, 6.5, 7.0, 7.5,  10.0, 10.5, 11.0, 11.5, 12.0, 15.0,17.5, 20.0, 30.0", kTRUE);
		//cf->AddVariable(AliDielectronVarManager::kEMCALnSigmaEle,"-4.5,-4.,-3.75,-3.5,-3.25,-3.0,-2.75,-2.5,-2.0,-1.0,1.0,2.0,2.5,3.0,3.25,3.5,3.75,4.0,5.0",kTRUE);
		// cf->AddVariable(AliDielectronVarManager::kEMCALNCells,25,0,25,kTRUE);
		cf->AddVariable(AliDielectronVarManager::kEMCALEoverP, "0,0.4,0.6,0.7,0.8,0.9,1.0,1.1, 1.2,1.3,1.4,2", kTRUE);
		// cf->AddVariable(AliDielectronVarManager::kTPCsignal,"40.,50.,55.,60.,65.,68.,70.,72.,75.,80.,90.,100.,110.,200.",kTRUE);

		cf->AddVariable(AliDielectronVarManager::kTPCnSigmaEle, " -3.0,-2.5,-2.25, -2.0,-1.5, -1.0, 0, 1.0,1.5, 2.0,2.25, 2.5, 3.0, 4.0", kTRUE);
		/*
  if(!isMC){
	 cf->AddVariable(AliDielectronVarManager::kTPCnSigmaPio,"-10, -3.0, -2.0,-1.5, -1.0, 0, 1.0,1.5, 2.0, 3.0, 4.0, 10",kTRUE);
	 cf->AddVariable(AliDielectronVarManager::kTPCnSigmaPro,"-10, -3.0, -2.0,-1.5, -1.0, 0, 1.0,1.5, 2.0, 3.0, 4.0, 10",kTRUE);
	 cf->AddVariable(AliDielectronVarManager::kTPCnSigmaKao,"-10, -3.0, -2.0,-1.5, -1.0, 0, 1.0,1.5, 2.0, 3.0, 4.0, 10",kTRUE);
  }
	*/

		//cf->AddVariable(AliDielectronVarManager::kTOFnSigmaEle,",-3.5,-3.0,-2.75,-2.5,-2.25,-2.0,-1.75,-1.5,-1.0,-0.5,0.0,1.0,2.0,2.25,2.5,2.75,3.0,3.25,3.5,4.0",kTRUE);
		//cf->AddVariable(AliDielectronVarManager::kTOFnSigmaPio,"1.,2.,2.5,3.0,3.5,4.0,4.5,100",kTRUE);

		//cf->AddVariable(AliDielectronVarManager::kITSLayerFirstCls,6,0.,6.,kTRUE);
		//cf->AddVariable(AliDielectronVarManager::kZvPrim,20,-20.,20.);
		//cf->AddVariable(AliDielectronVarManager::kImpactParXY,"-2.0, -1.0,-0.5,0.5, 1.0, 2.0",kTRUE);
		//cf->AddVariable(AliDielectronVarManager::kImpactParZ,"-4.0,-3.0, -2.0, 2.0,3.0, 4.0",kTRUE);

		// if (!isAOD){
		// Bool_t hasMC=(AliAnalysisManager::GetAnalysisManager()->GetMCtruthEventHandler()!=0x0);
		/*
    if (isMC){
			// printf("Is MC in the containers!!!\n");
      cf->AddVariable(AliDielectronVarManager::kPdgCode,10000,-5000.5,4999.5,kTRUE);
      cf->AddVariable(AliDielectronVarManager::kPdgCodeMother,10000,-5000.5,4999.5,kTRUE);
      cf->AddVariable(AliDielectronVarManager::kPdgCodeGrandMother,10000,-5000.5,4999.5,kTRUE);
		// }
	}
	 */

		//only in this case write MC truth info
		if ((cutDefinition == 1) && isMC)
		{
			cf->SetStepForMCtruth();
		}

		diele->SetCFManagerPair(cf);
	}
}
void AddMCSignals(AliDielectron *diele)
{

	AliDielectronSignalMC *inclusiveJpsi = new AliDielectronSignalMC("inclusiveJpsi", "Inclusive J/psi");
	inclusiveJpsi->SetLegPDGs(11, -11);
	inclusiveJpsi->SetMotherPDGs(443, 443);
	inclusiveJpsi->SetMothersRelation(AliDielectronSignalMC::kSame);
	inclusiveJpsi->SetFillPureMCStep(kTRUE);
	inclusiveJpsi->SetCheckBothChargesLegs(kTRUE, kTRUE);
	inclusiveJpsi->SetCheckBothChargesMothers(kTRUE, kTRUE);
	diele->AddSignalMC(inclusiveJpsi);

	AliDielectronSignalMC *promptJpsi = new AliDielectronSignalMC("promptJpsi", "Prompt J/psi"); // prompt J/psi (not from beauty decays)
	promptJpsi->SetLegPDGs(11, -11);
	promptJpsi->SetMotherPDGs(443, 443);
	promptJpsi->SetGrandMotherPDGs(503, 503, kTRUE, kTRUE); // not from beauty hadrons
	promptJpsi->SetMothersRelation(AliDielectronSignalMC::kSame);
	promptJpsi->SetFillPureMCStep(kTRUE);
	promptJpsi->SetLegSources(AliDielectronSignalMC::kFinalState, AliDielectronSignalMC::kFinalState);
	promptJpsi->SetCheckBothChargesLegs(kTRUE, kTRUE);
	promptJpsi->SetCheckBothChargesMothers(kTRUE, kTRUE);
	promptJpsi->SetCheckBothChargesGrandMothers(kTRUE, kTRUE);
	diele->AddSignalMC(promptJpsi);

	AliDielectronSignalMC *beautyJpsi = new AliDielectronSignalMC("beautyJpsi", "Beauty J/psi");
	beautyJpsi->SetLegPDGs(11, -11);
	beautyJpsi->SetMotherPDGs(443, 443);
	beautyJpsi->SetMothersRelation(AliDielectronSignalMC::kSame);
	beautyJpsi->SetGrandMotherPDGs(500, 500);
	beautyJpsi->SetFillPureMCStep(kTRUE);
	beautyJpsi->SetCheckBothChargesLegs(kTRUE, kTRUE);
	beautyJpsi->SetCheckBothChargesMothers(kTRUE, kTRUE);
	beautyJpsi->SetCheckBothChargesGrandMothers(kTRUE, kTRUE);
	diele->AddSignalMC(beautyJpsi);

	AliDielectronSignalMC *directJpsi = new AliDielectronSignalMC("directJpsi", "Direct J/psi"); // embedded J/psi
	directJpsi->SetLegPDGs(11, -11);
	directJpsi->SetMotherPDGs(443, 443);
	directJpsi->SetMothersRelation(AliDielectronSignalMC::kSame);
	directJpsi->SetFillPureMCStep(kTRUE);
	directJpsi->SetLegSources(AliDielectronSignalMC::kFinalState, AliDielectronSignalMC::kFinalState);
	directJpsi->SetMotherSources(AliDielectronSignalMC::kDirect, AliDielectronSignalMC::kDirect);
	directJpsi->SetCheckBothChargesLegs(kTRUE, kTRUE);
	directJpsi->SetCheckBothChargesMothers(kTRUE, kTRUE);
	diele->AddSignalMC(directJpsi);
}
