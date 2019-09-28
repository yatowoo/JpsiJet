#!/usr/bin/env python3

# Post-processing for J/psi

from pprint import pprint
import argparse
# Command-line Arguments
parser = argparse.ArgumentParser(description='Test script for post-processing')
parser.add_argument('-f', '--file',help='Input file', default='JpsiData.root')
parser.add_argument('--mcSig',help='MC signal file', default='MCsignal.root')
parser.add_argument('--mcLxy',help='MC Pseudo-Lxy file', default='mc2016.root')
parser.add_argument('-o','--output',help='Output ROOT file instead PDF', default='AnaJpsi.root')
parser.add_argument('--trig',help='Trigger for analysis', default='PairL')
args = parser.parse_args()

import ana_util
import ana_phys
from ana_phys import InvMass
import ROOT

fData = ROOT.TFile(args.file)
fMC = ROOT.TFile(args.mcSig)

c = ROOT.TCanvas('cTest','Plot Test', 1600, 1800)
c.Divide(2,3)
c.Draw()

JPSI_PT_BINS = {}
JPSI_PT_BINS['PairL'] = [5., 7., 9., 13., 21., 50.]
JPSI_PT_BINS['PairH'] = [10., 12., 14., 18., 26., 50.]

hMPt = fData.Get(args.trig).FindObject('InvMass_Pt')
N_BINS = len(JPSI_PT_BINS[args.trig]) -1
hM = list(range(N_BINS))
jpsi = list(range(N_BINS))
# Integrated
c.cd(1)
hMAll = hMPt.ProjectionY('hM')
hMAll.SetTitle('Integrated, %.1f < pT < %.1f GeV/c' % (JPSI_PT_BINS[args.trig][0], JPSI_PT_BINS[args.trig][-1]))
jpsiAll = ana_phys.ProcessInvMass(hMAll, fMC.hJpsiMC)
jpsiAll.hM.Draw('same PE')
ROOT.gPad.SaveAs('Jpsi_MPt_%s_0.pdf' % args.trig)
# pT bin
for iBin in range(N_BINS):
  c.cd(iBin+2)
  pT = JPSI_PT_BINS[args.trig][iBin]
  pTMax = JPSI_PT_BINS[args.trig][iBin+1]
  hM[iBin] = hMPt.ProjectionY('hM_'+repr(iBin),
    hMPt.GetXaxis().FindBin(pT),
    hMPt.GetXaxis().FindBin(pTMax) - 1)
  hM[iBin].SetTitle('%.1f < pT < %.1f GeV/c' % (pT, pTMax))
  jpsi[iBin] = ana_phys.ProcessInvMass(hM[iBin], fMC.hJpsiMC)
  jpsi[iBin].hM.Draw('same PE')
  ROOT.gPad.SaveAs('Jpsi_MPt_%s_%d.pdf' % (args.trig, iBin+1))

c.SaveAs('Jpsi_MPt_%s.pdf' % args.trig)
fData.Close()
fMC.Close()