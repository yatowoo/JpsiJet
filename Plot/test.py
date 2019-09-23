#!/usr/bin/env python3

import argparse
# Command-line Arguments
parser = argparse.ArgumentParser(description='Test script for post-processing')
parser.add_argument('--mc',help='MC flag', default=False, action='store_true')
parser.add_argument('--root',help='ROOT flag, output ROOT file instead PDF', default=False, action='store_true')
args = parser.parse_args()

import ana_util
import ana_phys
from ana_phys import InvMass
import ROOT
f = ROOT.TFile('../output/QM19/PairVars_LHC16_G2.root')
if(args.mc):
  fMC = ROOT.TFile('MCsignal.root')
  Jpsi = InvMass(f.PairVars.Projection(1), signalMC=fMC.hJpsiMC)
else:
  Jpsi = InvMass(f.PairVars.Projection(1))

c = ROOT.TCanvas('cTest','Plot Test', 800, 600)
c.Draw()

Jpsi.hM.Draw()
Jpsi.SignalExtraction(1.5, 4.5)
Jpsi.SelectSignalRegion()
Jpsi.SelectSideband()
Jpsi.DrawResult()

outputFormat = '.root' if args.root else '.pdf'
if(args.mc):
  c.SaveAs('cTest_MC' + outputFormat)
  fMC.Close()
else:
  c.SaveAs('cTest' + outputFormat)
f.Close()
