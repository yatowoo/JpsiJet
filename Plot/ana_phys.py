#!/usr/bin/env python3

# Physics object analyzing
# - Invariant mass spectrum processing

import math
from array import array
import ROOT
from ROOT import TH1, TH1D, TF1, TGraph
import ana_util
from ana_util import *

# Default parameters for constants and style
# Variables
N_FITTING             = 3     # Max times for re-fitting
# Drawing style
DATA_COLOR            = kBlue
DATA_STYLE            = kRound
TOTAL_COLOR           = kRed
TOTAL_LINE            = 1     # Normal
TOTAL_LINE_WIDTH      = 3
SIGNAL_COLOR          = kBlack
SIGNAL_LINE           = 2     # Dash-line
SIGNAL_LINE_WIDTH     = 2
BACKGROUND_COLOR      = kGreen
BACKGROUND_LINE       = 2
BACKGROUND_LINE_WIDTH = 2
DEFAULT_NPX           = 1000
# Constants
JPSI_MASS_PDG         = 3.096 # GeV/c^2
JPSI_MASS_LOWER       = 2.92
JPSI_MASS_UPPER       = 3.16
JPSI_SIDEBAND_OFFSET_L= 1.0
JPSI_SIDEBAND_OFFSET_R= 0.2
N_PARAMS_POL2         = 3
N_PARAMS_MC           = 1
N_PARAMS_CRYSTALBALL  = 5

class InvMass:
  hM     = None # Invariant mass spectrum, TH1D
  hMPt   = None # Invariant mass vs pT, TH2D (optional)
  hsMC   = None # Show MC fitting, THStack
  hSigMC = None # MC shape for signal fitting, TH1D
  hBkgMC = None # Background shape for MC fitting, TH1D
  hTotMC = None # Total shape for MC fitting, TH1D
  fSig   = None # Function for signal, TF1
  fBkg   = None # Function for background, TF1
  fTot   = None # Function for total spectrum, TF1
  gFitL  = 1.8  # Fitting region, lower edge
  gFitH  = 4.2  # Fitting region, higher edge
  gHistL = 1.5  # Drawing region, lower edge
  gHistH = 4.5  # Drawing region, higher edge
  lgd    = None # Drawing legend for fitting marks, TLegend
  gDrawingList = None # Store objects for drawing to avoid garbage collecting, TObjArray
  result = {}
  def DrawStack(self):
    self.hsMC = ROOT.THStack("hsMC","Histograms of MC fitting results")
    # Background
    self.hBkgMC = self.hM.Clone('hBkgMC')
    self.hBkgMC.SetLineColor(BACKGROUND_COLOR)
    self.hBkgMC.SetLineStyle(TOTAL_LINE)
    self.hBkgMC.SetFillColor(BACKGROUND_COLOR)
    self.hBkgMC.SetFillStyle(3006)
    for iBin in range(self.hBkgMC.GetNbinsX()+2):
      self.hBkgMC.SetBinContent(iBin, self.fBkg.Eval(self.hBkgMC.GetBinCenter(iBin)))
    self.hsMC.Add(self.hBkgMC)
    # Signal as Total by stacked
    self.hTotMC = self.hM.Clone('hTotMC')
    self.hTotMC.SetLineWidth(TOTAL_LINE_WIDTH)
    self.hTotMC.SetLineStyle(TOTAL_LINE)
    self.hTotMC.SetLineColor(TOTAL_COLOR)
    self.hTotMC.SetFillColor(TOTAL_COLOR)
    self.hTotMC.SetFillStyle(3004)
    self.hsMC.Add(self.hTotMC)
    for iBin in range(self.hBkgMC.GetNbinsX()+2):
      self.hTotMC.SetBinContent(iBin, self.fSig.Eval(self.hTotMC.GetBinCenter(iBin)))
    self.hsMC.Draw('same')
  def DrawLine(self, xval, fcn = None, color = TOTAL_COLOR, style = 3):
    ymin = self.hM.GetMinimum()
    if(not fcn):
      ymax = self.hM.GetMaximum()
    else:
      ymax = fcn.Eval(xval)
    yLine = ROOT.TLine(xval, ymin, xval, ymax)
    yLine.SetLineColor(color)
    yLine.SetLineStyle(style)
    yLine.Draw('same')
    return self.gDrawingList.Add(yLine)
  def DrawRegion(self, fcn, xlow, xup, tag, color = TOTAL_COLOR, style = 3004):
    region = fcn.Clone('fRegion_' + tag)
    region.SetRange(xlow, xup)
    region.SetFillColor(color)
    region.SetFillStyle(style)
    region.Draw('same FC')
    self.gDrawingList.Add(region)
    return region
  def DrawResult(self):
    # Fitting result - TPaveText
    pTxtFit = ROOT.TPaveText(0.62, 0.39, 0.87, 0.88, "brNDC")
    pTxtFit.SetName("pTxtFit")
    pTxtFit.SetBorderSize(0)
    pTxtFit.SetTextAlign(12)
    pTxtFit.SetTextFont(42)
    pTxtFit.SetTextSize(0.03)
    pTxtFit.SetFillColor(0)
      # Entries - Signal region
    entry = pTxtFit.AddText("M_{J/#psi} #in [%.2f, %.2f] (GeV/c^{2})" % self.result['Region']['Signal'])
    entry.SetTextSize(0.03)
    entry.SetTextFont(62) # Helvetica (Bold)
      # Entries - Integral
    pTxtFit.AddText("Data:     %.0f #pm %.0f" % self.result['Data'])
    pTxtFit.AddText("Total:    %.0f #pm %.0f" % self.result['Total'])
    pTxtFit.AddText("Signal:  %.0f #pm %.0f" % self.result['Signal'])
    pTxtFit.AddText("Bkg:      %.0f #pm %.0f" % self.result['Bkg'])
    pTxtFit.AddText("S/B         = %.2f #pm %.2f" % self.result['SBratio'])
    pTxtFit.AddText("S/#sqrt{S+B}   = %.2f #pm %.2f" % self.result['SNratio'])
    pTxtFit.AddText("#chi^{2} / NDF  = %.1f / %d" % self.result['Chi2'])
    pTxtFit.AddText("f_{sideband}    = %.2f #pm %.2f" % self.result['SBfactor'])
    pTxtFit.Draw("same")
    return self.gDrawingList.Add(pTxtFit)
  def SelectSideband(self, method='Normal'):
    # Normal strategy - Fixed Left and Flexible Right with approximate counts
    # Left
    leftLow = self.result['Region']['Signal'][0] - JPSI_SIDEBAND_OFFSET_L
    leftUp = self.result['Region']['Signal'][1] - JPSI_SIDEBAND_OFFSET_L
    NLeft = ana_util.HistCount(self.hM, leftLow, leftUp)
    # Right
    NRight = 0
    rightLow = self.result['Region']['Signal'][1] + JPSI_SIDEBAND_OFFSET_R
    iBinRight = self.hM.FindBin(rightLow)
    for iBin in range(iBinRight, self.hM.GetNbinsX() + 1):
      NRight += self.hM.GetBinContent(iBin)
      if(NRight > NLeft):
        break
    rightUp = self.hM.GetBinCenter(iBin) + 0.5 * self.hM.GetBinWidth(iBin)
    # Result
    self.result['Region']['SidebandL'] = (leftLow, leftUp)
    self.result['Region']['SidebandR'] = (rightLow, rightUp)
    (NBkg, EBkg) = self.result['Bkg']
    self.result['SBfactor'] = (
      NBkg / (NLeft + NRight),
      math.sqrt(1/(NLeft+NRight) + (EBkg/NBkg)**2))
    # Draw
    self.DrawLine(leftLow, self.fTot, BACKGROUND_COLOR)
    self.DrawLine(leftUp, self.fTot, BACKGROUND_COLOR)
    self.DrawRegion(self.fTot, leftLow, leftUp, 'SBLeft', BACKGROUND_COLOR)
    self.DrawLine(rightLow, self.fTot, BACKGROUND_COLOR)
    self.DrawLine(rightUp, self.fTot, BACKGROUND_COLOR)
    region = self.DrawRegion(self.fTot, rightLow, rightUp, 'SBLeft', BACKGROUND_COLOR)
      # Legend
    region.SetLineWidth(0)
    region.SetLineColor(BACKGROUND_COLOR)
    region.SetMarkerColor(BACKGROUND_COLOR)
    self.lgd.AddEntry(region, "Sideband")
  def SelectSignalRegion(self, mlow = JPSI_MASS_LOWER, mup = JPSI_MASS_UPPER):
    self.result['Region'] = {}
    self.result['Region']['Signal'] = (mlow, mup)
    self.DrawLine(mlow)
    self.DrawLine(mup)
    if(not self.hSigMC):
      self.DrawRegion(self.fTot, mlow, mup, 'Signal')
    # Integral results and errors
      # DATA
    NData = ana_util.HistCount(self.hM, mlow, mup)
    EData = math.sqrt(NData)
    self.result['Data'] = (NData, EData)
      # TOTAL
    fitter = ROOT.TVirtualFitter.GetFitter() # Fitting results
    width = self.hM.GetBinWidth(1)
    NTotal = self.fTot.Integral(mlow, mup) / width
    ETotal = self.fTot.IntegralError(mlow, mup) /width
    self.result['Total'] = (NTotal, ETotal)
      # Signal and Background
    errPar = self.fTot.GetParErrors()
    if(not self.hSigMC):
      Nparam_sig = N_PARAMS_CRYSTALBALL
    else:
      Nparam_sig = N_PARAMS_MC
    Nparam_bkg = N_PARAMS_POL2
    Nparam_total = Nparam_sig + Nparam_bkg
      ## Covariant Matrix
    covTot = ROOT.TMatrixDSym(0,Nparam_total-1, fitter.GetCovarianceMatrix())
    covSig = covTot.GetSub(0, Nparam_sig-1, 0, Nparam_sig-1)
    covBkg = covTot.GetSub(Nparam_sig, Nparam_total-1, Nparam_sig, Nparam_total-1)
    errPar.SetSize(Nparam_total)
    errPar = list(errPar)
    self.fSig.SetParErrors(array('d',errPar[0:Nparam_sig]))
    self.fBkg.SetParErrors(array('d',errPar[Nparam_sig:Nparam_total]))
      ## Calculation
    NSig = self.fSig.Integral(mlow, mup) / width
    ESig = self.fSig.IntegralError(mlow, mup, self.fSig.GetParameters(), covSig.GetMatrixArray()) / width
    self.result['Signal'] = (NSig, ESig)
    NBkg = self.fBkg.Integral(mlow, mup) / width
    EBkg = self.fBkg.IntegralError(mlow, mup, self.fBkg.GetParameters(), covBkg.GetMatrixArray()) / width
    self.result['Bkg'] = (NBkg, EBkg)
      # Significance - S/B
    SBratio = NSig / NBkg
    ESBratio = SBratio * math.sqrt((ESig/NSig)**2 + (EBkg/NBkg)**2)
    self.result['SBratio'] = (SBratio, ESBratio)
      # Significance - S/#sqrt{S+B}
    SNratio = NSig / math.sqrt(NTotal)
    ESNratio = SNratio * math.sqrt((ESig/NSig)**2 + (ETotal/NTotal/2.)**2)
    self.result['SNratio'] = (SNratio, ESNratio)
      # Chi2
    self.result['Chi2'] = (self.fTot.GetChisquare(), self.fTot.GetNDF())
    # End - Signal region fitting result
  def DrawLegend(self):
    self.lgd = ROOT.TLegend(0.13, 0.68, 0.49, 0.88, "", "brNDC")
    self.lgd.SetName("lgdInvMass")
    self.lgd.SetBorderSize(0)
    self.lgd.SetTextAlign(12)
    self.lgd.SetTextFont(42)
    self.lgd.SetTextSize(0.03)
    self.lgd.AddEntry(self.hM, "e^{+}e^{-} pair")
    if(not self.hSigMC):
      self.lgd.AddEntry(self.fTot,"Total fit")
      self.lgd.AddEntry(self.fSig,"Signal fit (Crystal-Ball)")
    else:
      self.lgd.AddEntry(self.fTot,"MC shape + Bkg")
    self.lgd.AddEntry(self.fBkg,"Background fit (pol2)")
    self.lgd.Draw("same")
  def SetStyleForAll(self):
    # ROOT Style
    ROOT.gStyle.SetOptFit(0000)
    ROOT.gStyle.SetOptStat(0)
    # Data - Histogram
    self.hM.SetLineColor(DATA_COLOR)
    self.hM.SetMarkerColor(DATA_COLOR)
    self.hM.SetMarkerStyle(DATA_STYLE)
    self.hM.SetXTitle("M_{e^{+}e^{-}} (GeV/c^{2})")
    self.hM.GetXaxis().SetRangeUser(self.gHistL, self.gHistH)
    self.hM.SetYTitle("N_{pairs}")
    self.hM.GetYaxis().SetRangeUser(0.1, 2 * self.hM.GetBinContent(self.hM.GetMaximumBin()))
    # Total
    self.fTot.SetLineColor(TOTAL_COLOR)
    self.fTot.SetLineWidth(TOTAL_LINE_WIDTH)
    self.fTot.SetLineStyle(TOTAL_LINE)
    self.fTot.SetNpx(DEFAULT_NPX)
    # Signal
    self.fSig.SetLineColor(SIGNAL_COLOR)
    self.fSig.SetLineWidth(SIGNAL_LINE_WIDTH)
    self.fSig.SetLineStyle(SIGNAL_LINE)
    self.fSig.SetNpx(DEFAULT_NPX)
    # Background
    self.fBkg.SetLineColor(BACKGROUND_COLOR)
    self.fBkg.SetLineWidth(BACKGROUND_LINE_WIDTH)
    self.fBkg.SetLineStyle(BACKGROUND_LINE)
    self.fBkg.SetNpx(DEFAULT_NPX)
  def InitFittingCrystalBall(self):
    self.fTot = TF1("fTot", "fSig+fBkg", self.gHistL, self.gHistH)
    self.fTot.SetParNames("A", "#alpha", "n", "#sigma", "#mu", "a0", "a1", "a2")
    pseudo_peak = self.hM.GetBinContent(self.hM.FindBin(JPSI_MASS_PDG)) # Signal pseudo-peak
    self.fTot.SetParameter("A", pseudo_peak)
    self.fTot.SetParLimits(0, 0., 3 * pseudo_peak)
    self.fTot.SetParameter("#alpha", 0.3)
    self.fTot.SetParLimits(1, 0., 10.)
    self.fTot.SetParameter("n", 1.)
    self.fTot.SetParLimits(2, 0., 100.)
    self.fTot.SetParameter("#sigma", 0.1)
    self.fTot.SetParLimits(3, 0., 1.)
    self.fTot.SetParameter("#mu", JPSI_MASS_PDG)
    self.fTot.SetParLimits(4, self.gFitL, self.gFitH)
      # Background parameter
    self.fTot.SetParameter("a0", 250.)
    self.fTot.SetParameter("a1", -100)
    self.fTot.SetParameter("a2", 12.)
    return self.fTot
  def InitFittingMC(self):
    self.fTot = TF1("fTot", self.TotalMC, self.gHistL, self.gHistH, 4)
    self.fTot.SetParNames("A", "a0", "a1", "a2")
    pseudo_peak = self.hM.GetBinContent(self.hM.FindBin(JPSI_MASS_PDG)) # Signal pseudo-peak
    self.fTot.SetParameter("A", pseudo_peak)
    self.fTot.SetParLimits(0, 0., 3 * pseudo_peak)
      # Background parameter
    self.fTot.SetParameter("a0", 250.)
    self.fTot.SetParameter("a1", -100)
    self.fTot.SetParameter("a2", 12.)
    return self.fTot
  def UpdateParameters(self):
    # Signal
    self.fSig.SetParameter(0, self.fTot.GetParameter("A"))
    if(not self.hSigMC):
      self.fSig.SetParameter(1, self.fTot.GetParameter("#alpha"))
      self.fSig.SetParameter(2, self.fTot.GetParameter("n"))
      self.fSig.SetParameter(3, self.fTot.GetParameter("#sigma"))
      self.fSig.SetParameter(4, self.fTot.GetParameter("#mu"))
    # Background
    self.fBkg.SetParameter(0, self.fTot.GetParameter("a0"))
    self.fBkg.SetParameter(1, self.fTot.GetParameter("a1"))
    self.fBkg.SetParameter(2, self.fTot.GetParameter("a2"))
  def SignalExtraction(self, xlow, xup):
    self.gFitL = xlow
    self.gFitH = xup
    fitResult = self.hM.Fit(self.fTot, "ISN", "", self.gFitL, self.gFitH)
    self.hM.Draw("PE0")
    self.UpdateParameters()
    if(not self.hSigMC):
      self.fBkg.Draw("same C")
      self.fSig.Draw("same C")
      self.fTot.Draw("same C")
    else:
      self.DrawStack()
    self.DrawLegend()
  def TotalMC(self, x, par):
    bkg = par[1] + par[2] * x[0] + par[3] * math.pow(x[0], 2.0)
    return self.SignalMC(x,par) + bkg
  def SignalMC(self, x, par):
    if(x[0] < self.gFitL or x[0] > self.gFitH):
      return 0.0
    return par[0] * self.hSigMC.GetBinContent(self.hSigMC.FindBin(x[0]))
  def __init__(self, hInvMass, mlow = 1.5, mup = 4.5, signalMC = None):
    self.gFitL = mlow
    self.gFitH = mup
    self.hM = hInvMass
    self.fBkg = TF1("fBkg", "[0]+[1]*x+[2]*x^2", self.gHistL, self.gHistH)
    if(not signalMC):
      self.fSig = TF1("fSig", "[0]*ROOT::Math::crystalball_function(x,[1],[2],[3],[4])", self.gHistL, self.gHistH)
      self.InitFittingCrystalBall()
    else:
      signalMC.Scale(1/signalMC.Integral('width'))
      self.hSigMC = signalMC.Clone('hSigMC')
      self.fSig = TF1("fSig", self.SignalMC, self.gHistL, self.gHistH, 1)
      self.InitFittingMC()
    self.SetStyleForAll()
    self.gDrawingList = ROOT.TObjArray()

# Parameters and constants
PSEUDOLXY_PROMPT_CUT   = 0.01  # cm, |Lxy| < CUT
PSEUDOLXY_BDECAY_CUT   = 0.01  # cm, Lxy > CUT
PSEUDOLXY_PROMPT_FIT   = 0.05  # cm, range of MC shape
PSEUDOLXY_BDECAY_FIT_L = -0.02 # cm, range of MC shape
PSEUDOLXY_BDECAY_FIT_R = 0.2   # cm, range of MC shape
PSEUDOLXY_BKG_FIT      = 0.05  # cm, range of MC shape
PSEUDOLXY_TOTAL_FIT_L  = -0.05 # cm, range of fitting
PSEUDOLXY_TOTAL_FIT_R  = 0.2 # cm, range of fitting

class PseudoLxy:
  hData     = None # DATA
  hMCPrompt = None # MC shape for prompt, TH1D
  hMCBdecay = None # MC shape for b-hadron decay, TH1D
  hMCBkg    = None # MC shape for background decay, TH1D
  fPrompt   = None # TF1
  fBdecay   = None # TF1
  fBkg      = None # TF1
  fTotal    = None # TF1
  def PromptMC(self, x, par):
    if(abs(x[0]) > PSEUDOLXY_PROMPT_FIT):
      return 0.0
    return par[0] * self.hMCPrompt.GetBinContent(self.hMCPrompt.FindBin(x[0]))
  def BdecayMC(self, x, par):
    if(x[0] < PSEUDOLXY_BDECAY_FIT_L or x[0] > PSEUDOLXY_BDECAY_FIT_R):
      return 0.0
    return par[0] * self.hMCBdecay.GetBinContent(self.hMCBdecay.FindBin(x[0]))
  def BkgMC(self, x, par):
    if(abs(x[0]) > PSEUDOLXY_BKG_FIT):
      return 0.0
    return par[0] * self.hMCBkg.GetBinContent(self.hMCBkg.FindBin(x[0]))
  def TotalMC(self, x, par):
    ratioPrompt = self.PromptMC(x, par)
    ratioBdecay = self.BdecayMC(x, [par[1]])
    ratioBkg    = self.BkgMC(x, [par[2]])
    return  ratioPrompt + ratioBdecay + ratioBkg
  def Fitting(self):
    self.hData.Fit(self.fTotal, "IS", "", PSEUDOLXY_TOTAL_FIT_L, PSEUDOLXY_TOTAL_FIT_R)
  def __init__(self, Lxy, Prompt, Bdecay, Bkg):
    self.hData = Lxy.Clone('hLxyData')
    self.hMCPrompt = Prompt.Clone('hPromptMC')
    self.hMCBdecay = Bdecay.Clone('hBdecayMC')
    self.hMCBkg    = Bkg.Clone('hBkgMC')
    self.fPrompt = TF1('fPrompt', self.PromptMC, -PSEUDOLXY_PROMPT_FIT, PSEUDOLXY_PROMPT_FIT, 1)
    self.fPrompt.SetParameter(0, 0.1)
    self.fBdecay = TF1('fBdecay', self.BdecayMC, PSEUDOLXY_BDECAY_FIT_L, PSEUDOLXY_BDECAY_FIT_R, 1)
    self.fBdecay.SetParameter(0, 0.01)
    self.fBkg    = TF1('fBkg', self.BkgMC, -PSEUDOLXY_BKG_FIT, PSEUDOLXY_BKG_FIT, 1)
    self.fBkg.SetParameter(0, 0.01)
    self.fTotal  = TF1('fTotal', self.TotalMC, PSEUDOLXY_TOTAL_FIT_L, PSEUDOLXY_TOTAL_FIT_R, 3)
    self.fTotal.SetParameters(0.1, 0.01, 0.01)
    self.fTotal.SetParLimits(0, 0., 10.)
    self.fTotal.SetParLimits(1, 0., 10.)
    self.fTotal.SetParLimits(2, 0., 10.)


if __name__ == '__main__':
  invM = InvMass()