#!/usr/bin/env python3

import sys, os, json, argparse
from tqdm import tqdm

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

for ev in tqdm(f.aodTree, desc='Processing events : ', unit='evts'):
  for jet in ev.jets:
    HistMgr['JetPt'].Fill(jet.Pt())
    HistMgr['JetEta'].Fill(jet.Eta())
  for jpsi in ev.dielectrons:
    HistMgr['M'].Fill(jpsi.M())
    HistMgr['DielePt'].Fill(jpsi.Pt())
    HistMgr['DieleY'].Fill(jpsi.Y())

fout.cd()
for obj in HistMgr.values():
  obj.Write()

fout.Close()
f.Close()

