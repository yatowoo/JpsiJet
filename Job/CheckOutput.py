#!/usr/bin/env python3

import sys, os, json, argparse,math
from tqdm import tqdm
from math import pi

parser = argparse.ArgumentParser(description='Check AOD files')
parser.add_argument('filelist',help='Path list for reading', default="Periods.txt")
parser.add_argument('no',help='Number of processing', type=int, default=0)
args = parser.parse_args()

import ROOT

with open(args.filelist) as f:
  path = f.readlines()[args.no].strip()

f = ROOT.TFile(path + "/AliAOD.Dielectron.root")
fout = ROOT.TFile("JpsiJetFilterAna.root","RECREATE")

print(path.split('/')[-2] + '\t' + repr(f.aodTree.GetEntries()))

HistMgr = {}
HistMgr['M'] = ROOT.TH1D("hInvMass",
  "Invariant Mass Spectrum;#it{M}_{e^{+}e^{-}} (GeV/#it{c}^{2});Counts per 40 MeV/#it{c}^{2};",
  100, 1.0, 5.0)
HistMgr['DielePt'] = ROOT.TH1D("hDielePt",
  "Inclusive dielectrons - raw #it{p}_{T};#it{p}_{T,e^{+}e^{-}} (GeV/#it{c});Counts per 1 GeV/#it{c};",
  100, 0, 100.0)
HistMgr['DieleY'] = ROOT.TH1D("hDieleY",
  "Inclusive dielectrons - Rapidity;#it{Y};Counts;",
  200, -1.0, 1.0)
HistMgr['JetPt'] = ROOT.TH1D("hJetPt",
  "Inclusive jets - raw #it{p}_{T};#it{p}_{T,ch jet} (GeV/#it{c});Counts per 0.05 GeV/#it{c};",
  2000, 0, 100.0)
HistMgr['JetEta'] = ROOT.TH1D("hJetEta",
  "Inclusive jets - PseudoRapidity;#it{eta};Counts;",
  200, -1.0, 1.0)
HistMgr['JpsiJet'] = ROOT.TH2D("hJpsiJet",
  "J/#psi-jet azimuthal correlation (#Delta#eta-#Delta#varphi);#Delta#varphi;#Delta#eta;Counts;",
  50, -1.6, 4.8,
  50, -1.5, 1.5)

# Input : AliEmcalJet
def CutJet(jet):
  if(jet.Eta() < -0.5 or jet.Eta() > 0.5):
    return False
  return True
# Input : AliDielectronPair
def CutJpsi(jpsi):
  if(jpsi.M() < 2.92 or jpsi.M() > 3.16):
    return False
  return True

def DeltaPhi(phi1, phi2):
  # Range from -3/2\pi to 1/2\pi
  dPhi = phi1 - phi2;
  if(dPhi > 1.5*pi):
    dPhi = dPhi - 2*pi
  if(dPhi < - 0.5*pi):
    dPhi += 2*pi;
  return dPhi; 

for ev in tqdm(f.aodTree, desc='Processing events : ', unit='evts'):
  for jet in ev.jets:
    if(not CutJet(jet)):
      continue
    HistMgr['JetPt'].Fill(jet.Pt())
    HistMgr['JetEta'].Fill(jet.Eta())
  for jpsi in ev.dielectrons:
    HistMgr['M'].Fill(jpsi.M())
    HistMgr['DielePt'].Fill(jpsi.Pt())
    HistMgr['DieleY'].Fill(jpsi.Y())
    if(CutJpsi(jpsi)):
      for jet in ev.jets:
        if(CutJet(jet)):
          HistMgr['JpsiJet'].Fill(
            DeltaPhi(ROOT.TVector2.Phi_0_2pi(jpsi.Phi()),jet.Phi()),
            jpsi.Eta()-jet.Eta())

fout.cd()
for obj in HistMgr.values():
  obj.Write()

fout.Close()
f.Close()

