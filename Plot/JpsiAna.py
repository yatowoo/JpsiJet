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
parser.add_argument('--pt',help='pT analysis', default=False, action='store_true')
parser.add_argument('--lxy',help='Pseudo-Lxy analysis', default=False, action='store_true')
args = parser.parse_args()

import ana_util
import ana_phys
from ana_phys import InvMass
import ROOT

fData = ROOT.TFile(args.file)
fMC = ROOT.TFile(args.mcSig)

c = ROOT.TCanvas('cTest','Plot Test', 800, 600)

pairList = fData.Get(args.trig)
pairList.SetOwner(True)

JPSI_PT_BINS = {}
JPSI_PT_BINS['PairL'] = [5., 7., 9., 13., 21., 50.]
JPSI_PT_BINS['PairH'] = [10., 12., 14., 18., 26., 50.]
if(args.pt):
  c.SetWindowSize(1600, 1800)
  c.Divide(2,3)
  c.Draw()
  # InvMass - Pt, TH2D
  hMPt = pairList.FindObject('InvMass_Pt')
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
  # Final output
  c.SaveAs('Jpsi_MPt_%s.pdf' % args.trig)

def ProjectTH2(hname, h2, ylow, yup):
  yBinLow = h2.GetYaxis().FindBin(ylow)
  yBinUp = h2.GetYaxis().FindBin(yup)
  if(yup > h2.GetYaxis().GetBinCenter(yBinUp)):
    yBinUp -= 1
  return h2.ProjectionX(hname, yBinLow, yBinUp)

# Pave for cuts
pTxt = ROOT.TPaveText(0.15, 0.45, 0.35, 0.6, "brNDC")
pTxt.SetFillColor(0)
pTxt.AddText('|y_{e^{+}e^{-}}| < 0.9')
pTxt.AddText('%.1f < p_{T,e^{+}e^{-}} < %.1f GeV/c' % (JPSI_PT_BINS[args.trig][0], JPSI_PT_BINS[args.trig][-1]))

if(args.lxy):
  c.Clear()
  c.SetWindowSize(1600, 600)
  c.Divide(2)
  # Inv. Mass
  c.cd(1)
  hMLxy = pairList.FindObject('InvMass_ProperTime')
  hM = hMLxy.ProjectionY('hM')
  hM.SetTitle('')
  jpsi = ana_phys.ProcessInvMass(hM, fMC.hJpsiMC)
  jpsi.hM.Draw('same PE')
  pTxt.Draw('same')
  # Pseudo-Lxy
  c.cd(2)
  ROOT.gPad.SetLogy()
  fMCLxy = ROOT.TFile('mc16.root')
  hLxyData = ProjectTH2('hLxySignal', hMLxy, ana_phys.JPSI_MASS_LOWER, ana_phys.JPSI_MASS_UPPER)
  hLxyData.SetTitle('')
  hLxyData.GetXaxis().SetRangeUser(-0.2, 0.2)
  sideband = jpsi.result['Region']['SidebandL']
  hLxyBkg = ProjectTH2('hLxySB', hMLxy, sideband[0], sideband[1])
  sideband = jpsi.result['Region']['SidebandR']
  hLxyBkg.Add(ProjectTH2('hLxytmp', hMLxy, sideband[0], sideband[1]))
  hLxyBkg.Scale(jpsi.result['SBfactor'][0])
  # Fitting
  Lxy = ana_phys.PseudoLxy(
    hLxyData,
    fMCLxy.hJpsiLxyPrompt,
    fMCLxy.hJpsiLxyBdecay,
    hLxyBkg)
  # For background error
  Lxy.result['Bkg'] = (0., jpsi.result['SBfactor'][1] / jpsi.result['SBfactor'][0])
  # Fitting and drawing results
  Lxy.hData.Draw('PE')
  Lxy.Fitting()
  pTxt.Draw('same')
  # Output
  c.SaveAs('Jpsi_MLxy_%s.pdf' % args.trig)

fData.Close()
fMC.Close()