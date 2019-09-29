#!/usr/bin/env python3

import ROOT
import ana_util
from ana_util import *

periods = 'ghijklop'

fout = ROOT.TFile('../output/QM19/merge_mc.root','RECREATE')

mcEff = ROOT.TCanvas('cMC','MC canvas', 1600, 600)
mcEff.Divide(2)
mcEff.Draw()
mcEff.cd(1)
ROOT.gPad.SetLogy()
ROOT.gPad.SetTicks()
ROOT.gPad.SetGrid()
mcEff.cd(2)
ROOT.gPad.SetLogy()
ROOT.gPad.SetTicks()
ROOT.gPad.SetGrid()

def DrawJpsiEff(h, title, tagColor, tagStyle):
  h.SetTitle(title)
  h.SetLineColor(tagColor)
  h.SetMarkerColor(tagColor)
  h.SetMarkerStyle(tagStyle)
  h.SetMarkerSize(1.)
  h.Draw('same PE')
  fout.cd()
  h.SetDirectory(fout)
  h.Write()

Hists = {}
Hists['JpsiEff'] = {}
for pname in periods:
  fileName = '../output/QM19/JpsiJetMC_16' + pname + '.root'
  tag = '16' + pname
  f = ROOT.TFile(fileName)
  if(f.IsOpen()):
    print('[-] INFO - Merging MC ' + tag)
  else:
    continue
  tagColor = next(ana_util.COLOR)
  Hists['JpsiEff']['hPrompt_' + tag] = f.hJpsiEffPrompt.Clone('hPrompt_' + tag)
  h = Hists['JpsiEff']['hPrompt_' + tag]
  h.GetYaxis().SetRangeUser(2e-4, 3e-1)
  h.SetYTitle('A #times #varepsilon')
  mcEff.cd(1)
  DrawJpsiEff(h, 'Prompt - ' + tag, tagColor, kRound)
  Hists['JpsiEff']['hBdecay_' + tag] = f.hJpsiEffBdecay.Clone('hBdecay_' + tag)
  mcEff.cd(2)
  DrawJpsiEff(Hists['JpsiEff']['hBdecay_' + tag], 'Non-prompt - ' + tag, tagColor, kCrossHollow)
  f.Close()

fout.cd()
mcEff.cd(1)
ROOT.gPad.BuildLegend()
mcEff.cd(2)
ROOT.gPad.BuildLegend()
mcEff.Write('cMC')
fout.Close()