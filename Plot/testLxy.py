#!/usr/bin/env python3

import ana_util
import ana_phys
import ROOT
from ROOT import TFile

c = ROOT.TCanvas('cTest','Plot Test', 800, 600)
c.Draw()
c.SetLogy()

fData = TFile('../output/QM19/PairVars_LHC16_G2.root')
fMC = TFile('mc16.root')

fData.PairVars.GetAxis(1).SetRangeUser(ana_phys.JPSI_MASS_LOWER, ana_phys.JPSI_MASS_UPPER)
#f.PairVars.GetAxis(5).SetRangeUser(15., 50.)

Lxy = ana_phys.PseudoLxy(
  fData.PairVars.Projection(2),
  fMC.hJpsiLxyPrompt,
  fMC.hJpsiLxyBdecay,
  fMC.hJpsiLxyBkg
)

Lxy.hData.Draw('PE')

Lxy.Fitting()

c.SaveAs("LxyFit.root")
fData.Close()
fMC.Close()
