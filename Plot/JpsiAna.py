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

c = ROOT.TCanvas('cTest','Plot Test', 800, 600)
c.Draw()

hMPt = fData.Get(args.trig).FindObject('InvMass_Pt')
N_BINS = len(ana_phys.JPSI_PT_BINS) -1
hM = list(range(N_BINS))
jpsi = list(range(N_BINS))
for iBin in range(N_BINS):
  pT = ana_phys.JPSI_PT_BINS[iBin]
  pTMax = ana_phys.JPSI_PT_BINS[iBin+1]
  hM[iBin] = hMPt.ProjectionY('hM_'+repr(iBin),
    hMPt.GetXaxis().FindBin(pT),
    hMPt.GetXaxis().FindBin(pTMax) - 1)
  hM[iBin].SetTitle('%.1f < pT < %.1f GeV/c' % (pT, pTMax))
  jpsi[iBin] = ana_phys.ProcessInvMass(hM[iBin], fMC.hJpsiMC)
  jpsi[iBin].hM.Draw('same PE')
  c.SaveAs('Jpsi_InvMassPt_%d.pdf' % iBin)

fData.Close()
fMC.Close()