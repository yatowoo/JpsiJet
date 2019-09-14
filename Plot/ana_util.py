#!/usr/bin/env python3

# Utility lib for general purpose in post-processing

import ROOT
import sys, os, time, math, json, logging

# Input: TH1, Int_t
def HistNorm(hist, NEv = 0):
  if(NEv == 0):
    NEv = hist.GetEntries()
  for i in range(1, hist.GetNbinsX() + 1):
    factor = NEv * hist.GetBinWidth(i)
    err = hist.GetBinError(i)
    hist.SetBinContent(i, hist.GetBinContent(i) / factor)
    hist.SetBinError(i,  err / factor)
  return hist

if __name__ == '__main__':
  print("Utility lib for post-processing with ROOT")