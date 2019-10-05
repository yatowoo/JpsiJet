#!/usr/bin/env python3

# Utility lib for general purpose in post-processing

import ROOT
  # Colors
from ROOT import kBlack, kRed, kBlue, kGreen, kOrange, kViolet, kCyan, kPink
import sys, os, time, math, json, logging
from array import array

# Global style
ROOT.gStyle.SetOptStat(0)
ROOT.gStyle.SetPalette()

# J/psi pT bins : 0 - 50, binw = 0.2, 0.5, 1, 2, 5
BINNING_JPSI_PT = [0.2*x for x in range(0,25,1)]
BINNING_JPSI_PT += [ 0.1*x for x in range(50,100,5)]
BINNING_JPSI_PT += list(range(10, 15, 1))
BINNING_JPSI_PT += list(range(15, 25, 2))
BINNING_JPSI_PT += list(range(25, 55, 5))
BINNING_JPSI_PT = array('d', BINNING_JPSI_PT) # Convert to double*

# Jet pT bins - Edge=0, 0.3, 1, 3, 10, 20, 50, 100
BINNING_JET_PT = [0.05*x for x in range(0,6,1)]
BINNING_JET_PT += [ 0.1*x for x in range(3,10,1)]
BINNING_JET_PT += [ 0.2*x for x in range(5,15,1)]
BINNING_JET_PT += [ x for x in range(3,10,1)]
BINNING_JET_PT += list(range(10,20,2))
BINNING_JET_PT += list(range(20,50,5))
BINNING_JET_PT += list(range(50,110,10))
BINNING_JET_PT = array('d', BINNING_JET_PT)

def ResetLegend(lgd, xlow, ylow, xup, yup):
  lgd.Clear()
  lgd.SetX1NDC(xlow)
  lgd.SetX2NDC(xup)
  lgd.SetY1NDC(ylow)
  lgd.SetY2NDC(yup)

def H2ProjectionX(hname, h2, ylow, yup):
  yBinLow = h2.GetYaxis().FindBin(ylow)
  yBinUp = h2.GetYaxis().FindBin(yup)
  if(yup > h2.GetYaxis().GetBinCenter(yBinUp)):
    yBinUp -= 1
  return h2.ProjectionX(hname, yBinLow, yBinUp)

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

DATA_MARKER = [kRound,  kBlock, kDelta, kNabla, kPenta, kDiamond, kCross, kClover, kClover4, kStar, kIronCross, kXMark]

MC_MARKER = [kRoundHollow, kBlockHollow, kDeltaHollow, kNablaHollow, kPentaHollow, kDiamondHollow, kCrossHollow, kCloverHollow, kClover4Hollow, kStarHollow, kIronCrossHollow, kXMarkHollow]

def SelectMarker(MARKER_INDEX = 0):
  while(MARKER_INDEX < 100):
    yield MARKER_SET[MARKER_INDEX % len(MARKER_SET)]
    MARKER_INDEX += 1
COLOR = SelectColor()
MARKER = SelectMarker()

def SetColorAndStyle(obj, c = None, s = None):
  if(c is None):
    c = next(COLOR)
  obj.SetLineColor(c)
  obj.SetMarkerColor(c)
  if(s is None):
    s = next(MARKER)
  obj.SetMarkerStyle(s)

def NewRatioPads(c, nameUpper, nameLower):
  c.Clear()
  c.SetWindowSize(800,800)
  c.Draw()
  padMain = ROOT.TPad(nameUpper,nameUpper, 0, 0.3, 1, 1.0)
  padMain.SetBottomMargin(0)
  padMain.SetLogy()
  padMain.Draw()
  c.cd()
  padRatio = ROOT.TPad(nameLower,nameLower, 0, 0.05, 1, 0.3)
  padRatio.SetTopMargin(0)
  padRatio.SetBottomMargin(0.2)
  padRatio.SetGrid()
  padRatio.Draw()
  return padMain, padRatio

def SetRatioPlot(rP, rmin=0.5, rmax=1.5):
  rP.SetTitle('')
  rP.SetMinimum(rmin)
  rP.SetMaximum(rmax)
  rP.GetYaxis().SetTitleOffset(0.4)
  rP.GetYaxis().SetTitleSize(0.09)
  rP.GetYaxis().SetLabelSize(0.08)
  rP.GetXaxis().SetLabelSize(0.09)
  rP.GetXaxis().SetTitleSize(0.09)
  rP.GetXaxis().SetTitleOffset(1.0)

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