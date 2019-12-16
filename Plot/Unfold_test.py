#!/usr/bin/env python3

import ROOT
import ana_util
fRM = ROOT.TFile('JpsiBJet_DetResponse_18.root')
fDet = ROOT.TFile('FF_5GeV.root')
hDet = fDet.cFF_SubBdecay.FindObject('hFFPromptCorrected2').Clone('hDet')
RM_INFO = fRM.Jet_DetResponse
RM_INFO.GetAxis(0).SetRangeUser(0,1.0) # z-det
RM_INFO.GetAxis(1).SetRangeUser(0,1.0) # z-gen
RM_INFO.GetAxis(2).SetRangeUser(15,35) # pT,jet-det
RM_INFO.GetAxis(3).SetRangeUser(15,35) # pT,jet-gen
hRM = RM_INFO.Projection(1,0)

hTrue = RM_INFO.Projection(1)
hTrue_input = ROOT.TH1D("hTrue","Spectra from generator", 10, 0, 1)
for i in range(1,11):
    hTrue_input.SetBinContent(i,hTrue.GetBinContent(i))
hRM_input = ROOT.TH2D("hRM","Response matrix", 10, 0, 1, 10, 0, 1)


response = ROOT.RooUnfoldResponse()
response.Setup(hDet, hTrue_input)
# Fill RM
for i in range(1,11):
  for j in range(1,11):
    response.Fill(hDet.GetBinCenter(i),hTrue_input.GetBinCenter(j),hRM.GetBinContent(i,j))

bayes = ROOT.RooUnfoldBayes(response,hDet)
bayes.SetIterations(4)

hUnfold = bayes.Hreco(0)
hUnfold.SetName('hUnfold')
hUnfold.Scale(1./hUnfold.Integral(),'width')
# Refold
hRefold = response.ApplyToTruth(hUnfold)
hRefold.SetName('hRefold')
hRefold.Scale(1./hRefold.Integral(),'width')
# Error
for i in range(1,11):
  hUnfold.SetBinError(i, hDet.GetBinError(i))
  hRefold.SetBinError(i, hDet.GetBinError(i))

ana_util.SetColorAndStyle(hDet, ROOT.kBlack, ana_util.kBlock)
ana_util.SetColorAndStyle(hUnfold, ROOT.kBlue, ana_util.kBlock, 2.0)
ana_util.SetColorAndStyle(hRefold, ROOT.kRed, ana_util.kBlock, 2.0)

# Drawing
  # Label
label = fDet.cFF_SubBdecay.FindObject('TPave')
  # Cuts
pTxtCuts = fDet.cFF_SubBdecay.FindObject('pTxtCuts')
  # Legend
lgd = ROOT.TLegend(0.13, 0.65, 0.40, 0.80)
lgd.SetName('lgdPromptRawFF')
lgd.SetBorderSize(0)
lgd.SetFillColor(0)
lgd.AddEntry(hDet,'Measured')
lgd.AddEntry(hUnfold,'Unfolded')
lgd.AddEntry(hRefold,'Refolded')

ROOT.gStyle.SetPalette(ROOT.kInvertedDarkBodyRadiator)
c = ROOT.TCanvas('cUnfold','Unfolding',1600,600)
c.Divide(2)
c.cd(1)
response.Hresponse().Draw('COLZ')

c.Clear()
c.SetWindowSize(1000,1000)
padFF, padRatio = ana_util.NewRatioPads(ROOT.gPad, "cFF", "cRatio")
padFF.cd()
hDet.Draw('PE1')
hUnfold.Draw('SAME PE1')
hRefold.Draw('SAME PE1')
label.Draw('same')
lgd.Draw('same')
pTxtCuts.Draw('same')

padRatio.cd()
hDet.Sumw2()
hRefold.Sumw2()
hRatio = hRefold.Clone('hRatio')
hRatio.SetTitle('')
hRatio.SetYTitle('Refolded / Measured')
hRatio.Divide(hDet)
ana_util.SetRatioPlot(hRatio)
hRatio.Draw('PE1')

c.SaveAs('Unfold_test.pdf')
c.SaveAs('Unfold_test.root')
