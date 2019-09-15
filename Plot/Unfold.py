#!/usr/bin/env python3
# Test for RooUnfold

import ROOT
from ROOT import TFile, TCanvas
from ROOT import RooUnfold, RooUnfoldBayes, RooUnfoldSvd, RooUnfoldResponse
from ana_util import HistNorm, SelectColor, SelectMarker

# Drawing
COLOR = SelectColor()
MARKER = SelectMarker()
padFF = TCanvas('cFF','Unfolding', 1600, 600)
padFF.Divide(2)
padFF.Draw()

# Data outputs
raw = TFile('../JpsiJetAna.root')
# MC outputs
mc = TFile('../output/QM19/JpsiJetMC.root')

def UnfoldFF(rawFF, detResponse, tag):
  # Z measured
  rawFF.SetBins(10, 0, 1.0)
  rawFF.Scale(1./rawFF.Integral('width'))
  rawFF.Draw()
  # Detector response
  detResponse.Scale(1./detResponse.Integral())
  response = RooUnfoldResponse(rawFF, rawFF, detResponse)
  bayes = RooUnfoldBayes(response, rawFF)
  # Unfolding - Bayesian
  COLOR = SelectColor()
  MARKER = SelectMarker()
  for nIter in range(4,5):
    bayes.SetIterations(nIter)
    hist = bayes.Hreco(0)
    hist.SetName('hBayes' + repr(nIter) + '_' + tag)
    hist.Scale(1./hist.Integral('width'))
    histColor = next(COLOR)
    hist.SetLineColor(histColor)
    hist.SetMarkerColor(histColor)
    hist.SetMarkerStyle(next(MARKER))
    hist.Draw('same PE')

padFF.cd(1)
UnfoldFF(raw.hZPromptAfter, mc.hResponseFF_Prompt, 'Prompt')
padFF.cd(2)
UnfoldFF(raw.hZBdecayAfter, mc.hResponseFF_Bdecay, 'Bdecay')

padFF.SaveAs('UnfoldFF.root')



