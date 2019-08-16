#ALICE Analysis Task for Jpsi in Jet

AliPhysics @ ROOT6

Event data model: AOD

## Dependency

PWGDQ/dielectron - AliDielectron
PWGJE/EMCalJet - AliEmcalJet
AliROOT - AODExtension / BranchReplicator

## Methods

Initialization of tasks and outputs.

Execution:
1. Check PID response and AOD
	1.1 Fill Event_RAW histograms, with Track_RAW
2. Event selection
	2.1 Fill Event histograms, with Track_ALL
3. Track selection
	3.1 Fill Track histograms
4. Dielectron pairing
	4.1 Check $J/\psi$ candidates
	4.2 Fill Dielectron histograms
5. Run jet finder
	5.1 Fill Jet histograms, Jet
6. Fill AOD event
7. Replace legs with $J/\psi$ as track
8. Re-run jet finder
	8.1 Fill Jet_WithJpsi histograms
9. Find $J/\psi$ in jets
	9.1 Extract FF histograms

## Interface

Getter/Setter for AliDielectron, AliEmcalJet and AODExtension

Event selection: trigger and cuts

Cuts definition, for events, tracks, $e^{+}e^{-}$ pairs and jets.

FLAG: Runwise QA

## MC

$J/\psi$ from $c\bar{c}$, $b\bar{b}$, b-hadron.

Decay channel: $J/\psi\rightarrow e^{+}e^{-}$

## Histograms

	Runwise?
	RAW - before EventCut
	Event - Before Track cut
	QA - After Track cut, by triggers
	Physics - After J/psi and Jet reconstruction

## Output

AliAOD.Dielectron.root

JpsiJetResult.root