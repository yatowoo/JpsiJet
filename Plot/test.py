#!/usr/bin/env python3

from pprint import pprint
import argparse
# Command-line Arguments
parser = argparse.ArgumentParser(description='Test script for post-processing')
parser.add_argument('--mc',help='MC flag', default=False, action='store_true')
parser.add_argument('--root',help='ROOT flag, output ROOT file instead PDF', default=False, action='store_true')
parser.add_argument('--cut',help='Cut flag, apply analysis cuts', default=False, action='store_true')
parser.add_argument('--lxy',help='Fit Pseudo-Lxy with MC shape and sideband', default=False, action='store_true')
args = parser.parse_args()

import ana_util
import ana_phys
from ana_phys import InvMass
import ROOT
fData = ROOT.TFile('../output/QM19/PairVars_LHC16_G1.root')
# Jet pT cut
if(args.cut):
  fData.PairVars.GetAxis(5).SetRangeUser(15., 50.)
if(args.mc):
  fMC = ROOT.TFile('MCsignal.root')
  Jpsi = InvMass(fData.PairVars.Projection(1), signalMC=fMC.hJpsiMC)
else:
  Jpsi = InvMass(fData.PairVars.Projection(1))

c = ROOT.TCanvas('cTest','Plot Test', 1600, 600)
c.Divide(2)
c.Draw()

c.cd(1)
Jpsi.hM.Draw()
Jpsi.SignalExtraction(1.5, 4.5)
Jpsi.SelectSignalRegion()
Jpsi.SelectSideband()
Jpsi.DrawResult()
pprint(Jpsi.result)

# Lxy
if(args.lxy):
  c.cd(2)
  ROOT.gPad.SetLogy()
  fMCLxy = ROOT.TFile('mc16.root')
  fData.PairVars.GetAxis(1).SetRangeUser(ana_phys.JPSI_MASS_LOWER, ana_phys.JPSI_MASS_UPPER)
  LxyData = fData.PairVars.Projection(2).Clone('hSignal')
  #f.PairVars.GetAxis(5).SetRangeUser(15., 50.)
  # Sideband
  sideband = Jpsi.result['Region']['SidebandL']
  fData.PairVars.GetAxis(1).SetRangeUser(sideband[0], sideband[1])
  LxyBkg = fData.PairVars.Projection(2).Clone('hSideband')
  sideband = Jpsi.result['Region']['SidebandR']
  fData.PairVars.GetAxis(1).SetRangeUser(sideband[0], sideband[1])
  LxyBkg.Add(fData.PairVars.Projection(2))
  LxyBkg.Scale(Jpsi.result['SBfactor'][0])
  Lxy = ana_phys.PseudoLxy(
    LxyData,
    fMCLxy.hJpsiLxyPrompt,
    fMCLxy.hJpsiLxyBdecay,
    LxyBkg
  )
  Lxy.hData.Draw('PE')
  Lxy.Fitting()

# Output
outputFormat = '.root' if args.root else '.pdf'
if(args.mc):
  c.SaveAs('cTest_MC' + outputFormat)
  fMC.Close()
else:
  c.SaveAs('cTest' + outputFormat)
fData.Close()
fMCLxy.Close()
