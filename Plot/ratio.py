#!/usr/bin/env python3

# Test ratio plot for systematics


import ROOT
from ROOT import TCanvas, TColor, TGaxis, TH1F, TPad
from ana_util import *

fStd = ROOT.TFile('std.root')
fSys = ROOT.TFile('syspid.root')

c = ROOT.TCanvas('cMC','Systematics', 800, 800)

pad1 = TPad("pad1", "pad1", 0, 0.3, 1, 1.0)
pad1.SetBottomMargin(0)  # joins upper and lower plot
pad1.SetGrid()
pad1.SetLogy()
pad1.Draw()

pad1.cd()
lgd = ROOT.TLegend(0.5, 0.12, 0.85, 0.45)
fStd.hJpsiEffPrompt.Draw('PE')
lgd.AddEntry(fStd.hJpsiEffPrompt, 'Prompt (Std. exclude hadrons)')
fStd.hJpsiEffBdecay.Draw('same')
lgd.AddEntry(fStd.hJpsiEffBdecay, 'Non-prompt (Std. exclude hadrons)')

hPsys = fSys.hJpsiEffPrompt.Clone('hPsys')
hPsys.SetMarkerStyle(kRoundHollow)
hPsys.Draw('same')
lgd.AddEntry(hPsys, 'Prompt (compare)')

hBsys = fSys.hJpsiEffBdecay.Clone('hBsys')
hBsys.SetMarkerStyle(kRoundHollow)
hBsys.Draw('same')
lgd.AddEntry(hBsys, 'Non-prompt (compare)')

lgd.Draw('same')

# Ratio plot
c.cd()
pad2 = TPad("pad2", "pad2", 0, 0.05, 1, 0.3)
pad2.SetTopMargin(0)  # joins upper and lower plot
pad2.SetBottomMargin(0.2)
pad2.SetGrid()
pad2.Draw()

rP = hPsys.Clone('rP')
rP.SetTitle('')
rP.SetMinimum(0.8)
rP.SetMaximum(3.0)
rP.GetYaxis().SetTitle('Ratio (Compare/Std)')
rP.GetYaxis().SetTitleOffset(0.4)
rP.GetYaxis().SetTitleSize(0.09)
rP.GetYaxis().SetLabelSize(0.08)
rP.GetXaxis().SetLabelSize(0.11)
rP.GetXaxis().SetTitleSize(0.06)
rP.GetXaxis().SetTitleOffset(1.5)

rP.Divide(fStd.hJpsiEffPrompt)

rB = hBsys.Clone('rB')
rB.Divide(fStd.hJpsiEffBdecay)

pad2.cd()
rP.Draw()
rB.Draw('same')

c.SaveAs('ratio.root')