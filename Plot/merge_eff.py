#!/usr/bin/env python3

# Check efficiency

import ROOT
from ROOT import TCanvas, TColor, TGaxis, TH1F, TPad
import ana_util
from ana_util import *

TAG_LIST = ['ALL', '16', '17', '18']
Eff = {}

c = ROOT.TCanvas('cMC','Efficiency', 800, 800)

# Eff plot
pad1, pad2 = ana_util.NewRatioPads(c,'Eff','Ratio')
pad1.SetGrid()

lgd = ROOT.TLegend(0.5, 0.12, 0.85, 0.45)
lgd.SetNColumns(2)
lgd.SetBorderSize(0)
lgd.SetFillColor(0)
for tag in TAG_LIST:
  Eff[tag] = {}
  Eff[tag]['MC'] = ROOT.TFile('MC'+tag+'_QM.root')
  pad1.cd()
  Eff[tag]['Prompt'] = Eff[tag]['MC'].hJpsiEffPrompt.Clone('hPrompt'+tag)
  Eff[tag]['Prompt'].Draw('same PE')
  color = next(COLOR)
  ana_util.SetColorAndStyle(Eff[tag]['Prompt'], color, kRound)
  lgd.AddEntry(Eff[tag]['Prompt'], 'Prompt (%s)' % tag)
  Eff[tag]['Bdecay'] = Eff[tag]['MC'].hJpsiEffBdecay.Clone('hBdecay'+tag)
  Eff[tag]['Bdecay'].Draw('same PE')
  ana_util.SetColorAndStyle(Eff[tag]['Bdecay'], color, kRoundHollow)
  lgd.AddEntry(Eff[tag]['Bdecay'], 'Non-prompt (%s)' % tag)
  # Ratio
  if(tag == 'ALL'):
    continue
  pad2.cd()
  Eff[tag]['RatioPrompt'] = Eff[tag]['Prompt'].Clone('rP'+tag)
  Eff[tag]['RatioPrompt'].Divide(Eff['ALL']['Prompt'])
  ana_util.SetRatioPlot(Eff[tag]['RatioPrompt'])
  Eff[tag]['RatioPrompt'].Draw('SAME PE')
  Eff[tag]['RatioBdecay'] = Eff[tag]['Bdecay'].Clone('rP'+tag)
  Eff[tag]['RatioBdecay'].Divide(Eff['ALL']['Bdecay'])
  Eff[tag]['RatioBdecay'].Draw('SAME PE')

pad1.cd()
lgd.Draw('same')
c.SaveAs('Eff.pdf')
c.SaveAs('Eff.root')
