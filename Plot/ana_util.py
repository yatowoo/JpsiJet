#!/usr/bin/env python3

# Utility lib for general purpose in post-processing

import ROOT
  # Colors
from ROOT import kBlack, kRed, kBlue, kGreen, kViolet, kCyan, kOrange, kPink, kYellow, kMagenta, kGray, kWhite
from ROOT import kFullCircle, kFullSquare, kOpenCircle, kOpenSquare, kOpenDiamond, kOpenCross, kFullCross, kFullDiamond, kFullStar, kOpenStar, kOpenCircle, kOpenSquare, kOpenTriangleUp, kOpenTriangleDown, kOpenStar, kOpenDiamond, kOpenCross, kOpenThreeTriangles, kOpenFourTrianglesX, kOpenDoubleDiamond, kOpenFourTrianglesPlus, kOpenCrossX, kFullTriangleUp, kOpenTriangleUp, kFullCrossX, kOpenCrossX, kFullTriangleDown, kFullThreeTriangles, kOpenThreeTriangles, kFullFourTrianglesX, kFullDoubleDiamond, kFullFourTrianglesPlus
import sys, os, time, math, json, logging
from array import array

# Global style
ROOT.gStyle.SetOptStat(0)
ROOT.gStyle.SetPalette()

def ALICEStyle(graypalette = False):
  print("[-] INFO - Setting ALICE figure style")
  ROOT.gStyle.Reset("Plain")
  ROOT.gStyle.SetOptTitle(0)
  ROOT.gStyle.SetOptStat(0)
  if(graypalette):
    ROOT.gStyle.SetPalette(8,0)
  else:
    ROOT.gStyle.SetPalette(1)
  ROOT.gStyle.SetCanvasColor(10)
  ROOT.gStyle.SetCanvasBorderMode(0)
  ROOT.gStyle.SetFrameLineWidth(1)
  ROOT.gStyle.SetFrameFillColor(kWhite)
  ROOT.gStyle.SetPadColor(10)
  ROOT.gStyle.SetPadTickX(1)
  ROOT.gStyle.SetPadTickY(1)
  ROOT.gStyle.SetPadTopMargin(0.02)
  ROOT.gStyle.SetPadBottomMargin(0.12)
  ROOT.gStyle.SetPadLeftMargin(0.14)
  ROOT.gStyle.SetPadRightMargin(0.02)
  ROOT.gStyle.SetHistLineWidth(1)
  ROOT.gStyle.SetHistLineColor(kRed)
  ROOT.gStyle.SetFuncWidth(2)
  ROOT.gStyle.SetFuncColor(kGreen+3)
  ROOT.gStyle.SetLineWidth(2)
  ROOT.gStyle.SetLabelSize(0.045,"xyz")
  ROOT.gStyle.SetLabelOffset(0.01,"y")
  ROOT.gStyle.SetLabelOffset(0.01,"x")
  ROOT.gStyle.SetLabelColor(kBlack,"xyz")
  ROOT.gStyle.SetTitleSize(0.05,"xyz")
  ROOT.gStyle.SetTitleOffset(1.2,"y")
  ROOT.gStyle.SetTitleOffset(1.1,"x")
  ROOT.gStyle.SetTitleFillColor(kWhite)
  ROOT.gStyle.SetTextSizePixels(26)
  ROOT.gStyle.SetTextFont(42)
  ROOT.gStyle.SetLegendBorderSize(0)
  ROOT.gStyle.SetLegendFillColor(kWhite)
  ROOT.gStyle.SetLegendFont(42)

def InitALICELabel(x1 = 0.02, y1 = -0.18, x2 = 0.35, y2 = -0.02, size=0.04, type="perf"):
  PAD_EDGE_LEFT = ROOT.gPad.GetLeftMargin()
  PAD_EDGE_RIGHT = 1 - ROOT.gPad.GetRightMargin()
  PAD_EDGE_BOTTOM   = ROOT.gPad.GetBottomMargin()
  PAD_EDGE_TOP   = 1 - ROOT.gPad.GetTopMargin()
  pTxtALICE = ROOT.TPaveText(PAD_EDGE_LEFT + x1, PAD_EDGE_TOP + y1, PAD_EDGE_LEFT + x2, PAD_EDGE_TOP + y2,"brNDC")
  pTxtALICE.SetBorderSize(0)
  pTxtALICE.SetFillColor(0)
  pTxtALICE.SetTextSize(size)
  pTxtALICE.SetTextFont(42) # Helvetica
  pTxtALICE.SetTextAlign(13) # Top Left
  if(type == "perf"):
    text = "ALICE Performance"
  elif(type == "simul"):
    text = "ALICE Simulation"
  elif(type == "prel"):
    text = "ALICE Preliminary"
  txt = pTxtALICE.AddText(text)
  txt.SetTextFont(42) # Helvetica Bold
  return pTxtALICE

def PrintFigure(name):
  ROOT.gPad.SaveAs(name + ".pdf")
  ROOT.gPad.SaveAs(name + ".eps")
  ROOT.gPad.SaveAs(name + ".png")
  ROOT.gPad.SaveAs(name + ".root")

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
COLOR_SET_ALICE = [kBlack, kRed+1 , kBlue+1, kGreen+3, kMagenta+1, kOrange-1,kCyan+2,kYellow+2]
COLOR_SET_ALICE_FILL = [kGray+1,  kRed-10, kBlue-9, kGreen-8, kMagenta-9, kOrange-9,kCyan-8,kYellow-7] # For systematic bands
COLOR_INDEX = -1
def SelectColor(COLOR_INDEX = 0):
  while(COLOR_INDEX < 100):
    yield COLOR_SET[COLOR_INDEX % len(COLOR_SET)]
    COLOR_INDEX += 1
# Marker Style
kRound,  kBlock, kDelta, kNabla, kPenta, kDiamond, kCross, kClover, kClover4, kStar, kIronCross, kXMark = 20, 21, 22, 23, 29, 33, 34, 39, 41, 43, 45, 47
kRoundHollow, kBlockHollow, kDeltaHollow, kNablaHollow, kPentaHollow, kDiamondHollow, kCrossHollow, kCloverHollow, kClover4Hollow, kStarHollow, kIronCrossHollow, kXMarkHollow = 24, 25, 26, 32, 30, 27, 28, 37, 40, 42, 44, 46
MARKER_SET = [kFullCircle, kOpenSquare, kCross, kFullTriangleUp, kOpenDiamond, kFullStar, kFullSquare, kOpenCross, kFullDiamond, kFullCrossX]

MARKER_SET_ALICE = [kFullCircle, kFullSquare, kOpenCircle, kOpenSquare, kOpenDiamond, kOpenCross, kFullCross, kFullDiamond, kFullStar, kOpenStar]

DATA_MARKER = [kFullCircle, kFullSquare, kFullTriangleUp, kFullTriangleDown, kFullStar, kFullDiamond, kFullCross, kFullThreeTriangles, kFullFourTrianglesX, kFullDoubleDiamond, kFullFourTrianglesPlus, kFullCrossX]

MC_MARKER = [kOpenCircle, kOpenSquare, kOpenTriangleUp, kOpenTriangleDown, kOpenStar, kOpenDiamond, kOpenCross, kOpenThreeTriangles, kOpenFourTrianglesX, kOpenDoubleDiamond, kOpenFourTrianglesPlus, kOpenCrossX]

def SelectMarker(MARKER_INDEX = 0):
  while(MARKER_INDEX < 100):
    yield MARKER_SET[MARKER_INDEX % len(MARKER_SET)]
    MARKER_INDEX += 1
COLOR = SelectColor()
MARKER = SelectMarker()

def SetColorAndStyle(obj, c = None, s = None, size = 1.0):
  if(c is None):
    c = next(COLOR)
  obj.SetLineColor(c)
  obj.SetMarkerColor(c)
  if(s is None):
    s = next(MARKER)
  obj.SetMarkerStyle(s)
  obj.SetMarkerSize(size)

def NewRatioPads(c, nameUpper, nameLower):
  c.Clear()
  c.Draw()
  padMain = ROOT.TPad(nameUpper,nameUpper, 0, 0.3, 1, 1.0)
  padMain.SetBottomMargin(0.02)
  padMain.Draw()
  c.cd()
  padRatio = ROOT.TPad(nameLower,nameLower, 0, 0.0, 1, 0.28)
  padRatio.SetTopMargin(0.03)
  padRatio.SetBottomMargin(0.25)
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
  pad.Clear()
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

def PrintOut(canvas, title, printFile, more=False):
  canvas.Write('c' + title)
  canvas.Print(printFile, 'Title:' + title)
  if(more):
    canvas.SaveAs(printFile + '_' + title + '.pdf')

def Rebin2D(h2raw, BINNING_X, BINNING_Y, name='h2new', title='New 2D histograms with user-defined binning', normalize=True):
  NX_RAW = h2raw.GetNbinsX()
  x_raw = h2raw.GetXaxis()
  NY_RAW = h2raw.GetNbinsY()
  y_raw = h2raw.GetYaxis()
  NX = len(BINNING_X) - 1
  BINNING_X_input = array('d', BINNING_X)
  NY = len(BINNING_Y) - 1
  BINNING_Y_input = array('d', BINNING_Y)
  # Histogram - NEW
  h2 = ROOT.TH2D(name, title, NX, BINNING_X_input, NY, BINNING_Y_input)
  x = h2.GetXaxis()
  y = h2.GetXaxis()
  # 2D bin content
    # TH1 bin index from 1 to N
  for i in range(1, NX_RAW +1, 1):
    for j in range(1, NY_RAW +1, 1):
      val = h2raw.GetBinContent(i,j)
      h2.Fill(x_raw.GetBinCenter(i), y_raw.GetBinCenter(j), val)
    # Normalized with bin area and all events
  for i in range(1, NX+1, 1):
    for j in range(1, NY+1, 1):
      val = h2.GetBinContent(i,j)
      val /= x.GetBinWidth(i) * y.GetBinWidth(j) * h2raw.GetSum()
      h2.SetBinContent(i, j, val)
  return h2

if __name__ == '__main__':
  print("Utility lib for post-processing with ROOT")