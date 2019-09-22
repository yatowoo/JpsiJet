#!/usr/bin/env python3

# Class for invariant mass spectrum processing

import math
from array import array
import ROOT
from ROOT import TH1, TH1D, TF1
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
JPSI_SIDEBAND_OFFSET  = 1.0
N_PARAMS_POL2         = 3
N_PARAMS_MC           = 1
N_PARAMS_CRYSTALBALL  = 5

class InvMass:
  hM     = None # Invariant mass spectrum, TH1D
  hMPt   = None # Invariant mass vs pT, TH2D (optional)
  hSigMC = None # MC shape for signal fitting, TH1D
  fSig   = None # Function for signal, TF1
  fBkg   = None # Function for background, TF1
  fTot   = None # Function for total spectrum, TF1
  gFitL  = 1.8  # Fitting region, lower edge
  gFitH  = 4.2  # Fitting region, higher edge
  gHistL = 1.5  # Drawing region, lower edge
  gHistH = 4.5  # Drawing region, higher edge
  lgd    = None # Drawing legend for fitting marks
  pTxtFit= None # Drawing pave text for fitting results
  def SelectRegion(self, mlow = JPSI_MASS_LOWER, mup = JPSI_MASS_UPPER):
    # Integral results and errors
      # DATA
    NData = ana_util.HistCount(self.hM, mlow, mup)
    EData = math.sqrt(NData)
      # TOTAL
    fitter = ROOT.TVirtualFitter.GetFitter() # Fitting results
    width = self.hM.GetBinWidth(1)
    NTotal = self.fTot.Integral(mlow, mup) / width
    ETotal = self.fTot.IntegralError(mlow, mup) /width
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
    NBkg = self.fBkg.Integral(mlow, mup) / width
    EBkg = self.fBkg.IntegralError(mlow, mup, self.fBkg.GetParameters(), covBkg.GetMatrixArray()) / width
    # Fitting result - TPaveText
    self.pTxtFit = ROOT.TPaveText(0.62, 0.39, 0.87, 0.88, "brNDC")
    self.pTxtFit.SetName("pTxtFit")
    self.pTxtFit.SetBorderSize(0)
    self.pTxtFit.SetTextAlign(12)
    self.pTxtFit.SetTextFont(42)
    self.pTxtFit.SetTextSize(0.03)
    self.pTxtFit.SetFillColor(0)
      # Entries - Signal region
    entry = self.pTxtFit.AddText("M_{J/#psi} #in [%.2f, %.2f] (GeV/c^{2})" % (mlow, mup))
    entry.SetTextSize(0.03)
    entry.SetTextFont(62) # Helvetica (Bold)
      # Entries - Integral
    self.pTxtFit.AddText("Data:     %.0f #pm %.0f" % (NData, EData))
    self.pTxtFit.AddText("Total:    %.0f #pm %.0f" % (NTotal, ETotal))
    self.pTxtFit.AddText("Signal:  %.0f #pm %.0f" % (NSig, ESig))
    self.pTxtFit.AddText("Bkg:      %.0f #pm %.0f" % (NBkg, EBkg))
      # Significance - S/B
    SBratio = NSig / NBkg
    ESBratio = SBratio * math.sqrt((ESig/NSig)**2 + (EBkg/NBkg)**2)
    self.pTxtFit.AddText("S/B        = %.2f #pm %.2f" % (SBratio, ESBratio))
      # Significance - S/#sqrt{S+B}
    SNratio = NSig / math.sqrt(NTotal)
    ESNratio = SNratio * math.sqrt((ESig/NSig)**2 + (ETotal/NTotal/2.)**2)
    self.pTxtFit.AddText("S/#sqrt{S+B}  = %.2f #pm %.2f" % (SNratio, ESNratio))
      # Chi2
    self.pTxtFit.AddText("#chi^{2} / NDF = %.1f / %d" % (self.fTot.GetChisquare(), self.fTot.GetNDF()))
    self.pTxtFit.Draw("same")
    # End of TPaveText - fitting result
  def DrawLegend(self):
    self.lgd = ROOT.TLegend(0.13, 0.68, 0.49, 0.88, "", "brNDC")
    self.lgd.SetName("lgdInvMass")
    self.lgd.SetBorderSize(0)
    self.lgd.SetTextAlign(12)
    self.lgd.SetTextFont(42)
    self.lgd.SetTextSize(0.03)
    self.lgd.AddEntry(self.hM, "e^{+}e^{-} pair")
    self.lgd.AddEntry(self.fTot,"Total fit")
    self.lgd.AddEntry(self.fBkg,"Background fit (pol2)")
    if(not self.hSigMC):
      self.lgd.AddEntry(self.fSig,"Signal fit (Crystal-Ball)")
    else:
      self.lgd.AddEntry(self.fSig,"Signal fit (MC shape)")
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
  def SignalExtraction(self):
    fitResult = self.hM.Fit(self.fTot, "ISN", "", self.gFitL, self.gFitH)
    self.hM.Draw("PE0")
    self.UpdateParameters()
    self.fSig.Draw("same")
    self.fBkg.Draw("same")
    self.fTot.Draw("same")
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


if __name__ == '__main__':
  invM = InvMass()