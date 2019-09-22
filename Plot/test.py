#!/usr/bin/env python3

from InvMass import InvMass
import ROOT
f = ROOT.TFile('../output/QM19/PairVars_LHC16_G2.root')
fMC = ROOT.TFile('MCsignal.root')
Jpsi = InvMass(f.PairVars.Projection(1), signalMC=fMC.hJpsiMC)

c = ROOT.TCanvas('cTest','Plot Test', 800, 600)
c.Draw()

Jpsi.hM.Draw()
Jpsi.SignalExtraction()
Jpsi.SelectRegion()
c.SaveAs('cTest.pdf')
f.Close()
fMC.Close()
