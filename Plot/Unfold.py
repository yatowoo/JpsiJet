#!/usr/bin/env python3
# Test for RooUnfold

import ROOT
from ROOT import TFile, TCanvas, TLegend
from ROOT import RooUnfold, RooUnfoldBayes, RooUnfoldSvd, RooUnfoldResponse
import ana_util
from ana_util import ResponseNorm, SelectColor, SelectMarker

# Drawing
padFF = TCanvas('cFF','Unfolding', 1600, 600)
padFF.Divide(2)
padFF.Draw()

# Data outputs
raw = TFile('../JpsiJetAna.root')
# MC outputs
mc = TFile('../output/QM19/JpsiJetMC.root')

def DrawFF(hist, name):
  hist.SetName(name)
  hist.Scale(1./hist.Integral('width'))
  histColor = next(ana_util.COLOR)
  hist.SetLineColor(histColor)
  hist.SetMarkerColor(histColor)
  hist.SetMarkerStyle(next(ana_util.MARKER))
  hist.Draw('same PE')
  return hist

def UnfoldFF(rawFF, detResponse, tag):
  padFF.Clear()
  padFF.Divide(2)
  # Detector response
  ResponseNorm(detResponse)
  padFF.cd(1)
  detResponse.Draw('COLZ')
  response = RooUnfoldResponse(rawFF, rawFF, detResponse)
  bayes = RooUnfoldBayes(response, rawFF)
  # Unfolding - Bayesian
  ana_util.COLOR = SelectColor(0)
  ana_util.MARKER = SelectMarker(0)
  padFF.cd(2)
  # Legend
  lgd = TLegend(0.12, 0.6, 0.3, 0.85)
  lgd.SetName('lgd' + tag)
  lgd.SetBorderSize(0)
  lgd.SetFillColor(0)
  # Z measured
  rawFF.SetBins(10, 0, 1.0)
  DrawFF(rawFF, 'hRaw_' + tag)
  lgd.AddEntry(rawFF, 'Raw')
  for nIter in range(4,5):
    bayes.SetIterations(nIter)
    hist = DrawFF(bayes.Hreco(0), 'hBayes' + repr(nIter) + '_' + tag)
    lgd.AddEntry(hist, 'Bayes (N=%d)' % nIter)
  lgd.Draw('same')
  padFF.SaveAs('UnfoldFF_' + tag + '.root')
  padFF.SaveAs('UnfoldFF_' + tag + '.pdf')

UnfoldFF(raw.hZPromptAfter, mc.hResponseFF_Prompt, 'Prompt')
UnfoldFF(raw.hZBdecayAfter, mc.hResponseFF_Bdecay, 'Bdecay')




