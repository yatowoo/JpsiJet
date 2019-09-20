#!/usr/bin/env python3

# Merge output with JpsiJetAna

import ana_util
from ana_util import *

f={}
f['15'] = ROOT.TFile('../output/QM19/JpsiJetAna_16High.root')
f['20'] = ROOT.TFile('../output/QM19/JpsiJetAna_16High_jet20.root')

output = ROOT.TFile('MergeFF.root','RECREATE')

padFF = ROOT.TCanvas('cFF','Jpsi in jets FF', 1600, 800)
padFF.Divide(2)
padFF.Draw()
# Legend
lgdPrompt = ROOT.TLegend(0.15, 0.4, 0.40, 0.55)
lgdPrompt.SetName('lgdPrompt')
lgdPrompt.SetBorderSize(0)
lgdPrompt.SetFillColor(0)

lgdBdecay = lgdPrompt.Clone('lgdBdecay')
lgdBdecay.SetName('lgdBdecay')
# Cuts
pTxt = ROOT.TPaveText(0.15, 0.58, 0.40, 0.88, "brNDC");
pTxt.SetName("yTxtCuts")
pTxt.SetBorderSize(0)
pTxt.SetTextAlign(12)
pTxt.SetTextFont(42)
pTxt.SetTextSize(0.03)
pTxt.SetFillColor(0)
pTxt.AddText('ALICE pp #sqrt{s} = 13 TeV, 2016')
figName = pTxt.AddText('Prompt (uncorrected)')
figName.SetTextFont(6)
figName.SetTextSize(30)
pTxt.AddText("|y_{e^{+}e^{-}}| < %.1f" % 0.9)
pTxt.AddText("%.1f < p_{T,e^{+}e^{-}} < %.1f GeV/c" % (10.0, 50.0))
pTxt.AddText("|#eta_{jet}| < %.1f" % 0.5)

pTxtPrompt = pTxt.Clone('yTxtCutsPrompt')
pTxtPrompt.SetName('yTxtCutsPrompt')
figName.SetTitle('Non-Prompt (uncorrected)')


MARKER_SIZE = 2
for tag in f:
  tagShape = next(MARKER)
  tagColor = next(COLOR)
  hPrompt = f[tag].hZPromptAfter
  hPrompt.Scale(1./hPrompt.Integral('width'))
  hPrompt.SetLineColor(tagColor)
  hPrompt.SetMarkerColor(tagColor)
  hPrompt.SetMarkerStyle(tagShape)
  hPrompt.SetMarkerSize(MARKER_SIZE)
  lgdPrompt.AddEntry(hPrompt, tag + ' < p_{T,jet} < 50 GeV/c')
  padFF.cd(1)
  hPrompt.SetYTitle('1/N dN/dz')
  hPrompt.GetYaxis().SetRangeUser(-0.5, 5.0)
  hPrompt.GetXaxis().SetTitleOffset(1.25)
  hPrompt.Draw('same PE1')
  # Bdecay
  hBedcay = f[tag].hZBdecayAfter
  hBedcay.Scale(1./hBedcay.Integral('width'))
  hBedcay.SetLineColor(tagColor)
  hBedcay.SetMarkerColor(tagColor)
  hBedcay.SetMarkerStyle(tagShape)
  hBedcay.SetMarkerSize(MARKER_SIZE)
  lgdBdecay.AddEntry(hBedcay, tag + ' < p_{T,jet} < 50 GeV/c')
  padFF.cd(2)
  hBedcay.SetYTitle('1/N dN/dz')
  hBedcay.GetYaxis().SetRangeUser(-0.5, 5.0)
  hBedcay.GetXaxis().SetTitleOffset(1.25)
  hBedcay.Draw('same PE1')

padFF.cd(1)
pTxtPrompt.Draw('same')
pTxtPrompt.Write('pTxtPrompt')
lgdPrompt.Draw('same')

padFF.cd(2)
lgdBdecay.Draw('same')
lgdBdecay.Write('lgdBdecay')
figName.SetTitle('Non-prompt (uncorrected)')
pTxt.Draw('same')
padFF.SaveAs('MergeFF.pdf')
padFF.Write('cFF')

output.Close()
