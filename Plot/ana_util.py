#!/usr/bin/env python3

# Utility lib for general purpose in post-processing

import ROOT
import sys, os, time, math, json, logging

# Input: TH1, Int_t
def HistNorm(hist, NEv = 0):
  if(NEv == 0):
    NEv = hist.GetEntries()
  factor = NEv * hist.GetBinWidth(1)
  logging.info('Histogram normalization factor : ' + repr(factor))
  fcn = ROOT.TF1("fNorm", "[0]", -1e6, 1e6)
  fcn.SetParameter(0, factor)
  hist.Divide(fcn)
  for i in range(1, hist.GetNbinsX() + 1):
    hist.SetBinError(i, hist.GetBinError(i) / math.sqrt(factor))
  fcn.Delete()
  return hist

if __name__ == '__main__':
  print("Utility lib for post-processing with ROOT")