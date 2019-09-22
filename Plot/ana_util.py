#!/usr/bin/env python3

# Utility lib for general purpose in post-processing

import ROOT
  # Colors
from ROOT import kBlack, kRed, kBlue, kGreen, kOrange, kViolet, kCyan, kPink
import sys, os, time, math, json, logging

# Normalize by column
  # X=measured, Y=true
def ResponseNorm(h2):
  NX = h2.GetNbinsX()
  NY = h2.GetNbinsY()
  for ix in range(1,NX+1):
    SumY = h2.Integral(ix, ix, 1, NY)
    if(SumY == 0):
      continue
    for iy in range(1, NY+1):
      val = h2.GetBinContent(ix, iy) / SumY
      h2.SetBinContent(ix, iy, val)
  return h2
# Input: TH1, Int_t
# Deprecated: use TH1/2::Scale(1./TH::Integral('width'))
def HistNorm(hist, NEv = 0):
  if(NEv == 0):
    NEv = hist.GetEntries()
  for i in range(1, hist.GetNbinsX() + 1):
    factor = NEv * hist.GetBinWidth(i)
    err = hist.GetBinError(i)
    hist.SetBinContent(i, hist.GetBinContent(i) / factor)
    hist.SetBinError(i,  err / factor)
  return hist

def HistCount(hist, xlow, xup, err = None):
  xBinLow = hist.FindBin(xlow)
  xBinUp  = hist.FindBin(xup)
  # x_up on the edge of bin
  if(hist.GetBinCenter(xBinUp) > xup):
    xBinUp -= 1
  if(not err):
    return hist.Integral(xBinLow, xBinUp)
  else:
    return hist.IntegralAndError(xBinLow, xBinUp, err)

# Select color and marker style in pre-defined group
  # Use Bool isNEW to reset the index
COLOR_SET = [kBlack, kRed, kBlue, kGreen+3, kOrange, kViolet, kCyan, kOrange-6, kPink]
COLOR_INDEX = -1
def SelectColor(COLOR_INDEX = 0):
  while(COLOR_INDEX < 100):
    yield COLOR_SET[COLOR_INDEX % len(COLOR_SET)]
    COLOR_INDEX += 1
# Marker Style
kRound,  kBlock, kDelta, kNabla, kPenta, kDiamond, kCross, kClover, kClover4, kStar, kIronCross, kXMark = 20, 21, 22, 23, 29, 33, 34, 39, 41, 43, 45, 47
kRoundHollow, kBlockHollow, kDeltaHollow, kNablaHollow, kPentaHollow, kDiamondHollow, kCrossHollow, kCloverHollow, kClover4Hollow, kStarHollow, kIronCrossHollow, kXMarkHollow = 24, 25, 26, 32, 30, 27, 28, 37, 40, 42, 44, 46
MARKER_SET = [kRound, kBlockHollow, kCross, kDelta, kDiamondHollow, kPenta, kBlock, kCrossHollow, kDiamond, kXMark]
def SelectMarker(MARKER_INDEX = 0):
  while(MARKER_INDEX < 100):
    yield MARKER_SET[MARKER_INDEX % len(MARKER_SET)]
    MARKER_INDEX += 1
COLOR = SelectColor()
MARKER = SelectMarker()

def PrintCover(pad, file, title = '', isBack = False):
  pTxt = ROOT.TPaveText(0.25,0.4,0.75,0.6, "brNDC")
  if(title == ''):
    if(isBack):
      pTxt.AddText('Thanks for your attention!')
    else:
      pTxt.AddText(pad.GetTitle())
  else:
    pTxt.AddText(title)
  pad.cd()
  pad.Draw()
  pTxt.Draw()
  if(isBack):
    pad.Print(file + ')', 'Title:End')
  else:
    pad.Print(file + '(', 'Title:Cover')
  pTxt.Delete()

if __name__ == '__main__':
  print("Utility lib for post-processing with ROOT")