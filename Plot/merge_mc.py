#!/usr/bin/env python3

import ROOT
import ana_util
from ana_util import *

periods = 'ghijklop'

fout = ROOT.TFile('../output/QM19/merge_mc.root','RECREATE')

mcEff = ROOT.TCanvas('cMC','MC canvas')
mcEff.Draw()
mcEff.SetLogy()
mcEff.SetTicks()
mcEff.SetGrid()

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

for pname in periods:
  fileName = '../output/QM19/JpsiJetMC_16' + pname + '.root'
  tag = '16' + pname
  f = ROOT.TFile(fileName)
  if(f.IsOpen()):
    print('[-] INFO - Merging MC ' + tag)
  else:
    continue
  tagColor = next(ana_util.COLOR)
  DrawJpsiEff(f.hJpsiEffPrompt.Clone('hPrompt_' + tag), 'Prompt - ' + tag, tagColor, kRound)
  DrawJpsiEff(f.hJpsiEffBdecay.Clone('hBdecay_' + tag), 'Non-prompt - ' + tag, tagColor, kCrossHollow)
  f.Close()

fout.cd()
fout.Write()
mcEff.Write('cMC')
fout.Close()