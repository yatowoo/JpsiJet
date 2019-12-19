#!/usr/bin/env python3

import ROOT
import ana_util
import functools

def ApplyCutZ(hFF):
  for i in range(1,5):
    hFF.SetBinContent(i, 0)
  hFF.Scale(1./hFF.Integral(),'width')

# Input data/MC files
fRM = ROOT.TFile('JpsiBJet_DetResponse_18.root')
fDet = ROOT.TFile('FF_5GeV.root')

# Output
fOut = ROOT.TFile('Unfold.root','RECREATE')
pdfOut = 'Unfold.pdf'
ROOT.gStyle.SetPalette(ROOT.kInvertedDarkBodyRadiator)
c = ROOT.TCanvas('cUnfold','Unfolding',1600,600)
ana_util.PrintCover(c, pdfOut, title='J/#psi in jets - Unfolding')
PrintPage = functools.partial(ana_util.PrintOut, canvas=c, printFile=pdfOut)

###
# Raw spectrum & Detector response
###
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

# Unfolding
bayes = ROOT.RooUnfoldBayes(response,hDet)
bayes.SetIterations(9)

hUnfold = bayes.Hreco(0)
hUnfold.SetName('hUnfold')
ApplyCutZ(hUnfold)
# Refold
hRefold = response.ApplyToTruth(hUnfold)
hRefold.SetName('hRefold')
ApplyCutZ(hRefold)
# Error
for i in range(1,11):
  hUnfold.SetBinError(i, hDet.GetBinError(i))
  hRefold.SetBinError(i, hDet.GetBinError(i))

ana_util.SetColorAndStyle(hDet, None, ROOT.kFullSquare, 1.5)
ana_util.SetColorAndStyle(hUnfold, None, ROOT.kOpenCircle, 1.0)
ana_util.SetColorAndStyle(hRefold, None, ROOT.kOpenSquare, 1.0)
ana_util.SetColorAndStyle(hTrue_input, None, ROOT.kFullCircle, 1.5)
ApplyCutZ(hTrue_input)

###
# Drawing
###

# Detector response
c.SetWindowSize(1200, 1000)
c.cd()
response.Hresponse().Draw('COLZ')
PrintPage(title='RM')

# Unfold, Refold test
  # Label
label = fDet.cFF_SubBdecay.FindObject('TPave')
  # Cuts
pTxtCuts = fDet.cFF_SubBdecay.FindObject('pTxtCuts')
pTxtCuts.SetTextSize(0.035)
pTxtCuts.SetTextFont(42)
pTxtCuts.SetY1NDC(0.3)
pTxtCuts.SetY2NDC(0.5)
  # Legend
lgd = ROOT.TLegend(0.13, 0.55, 0.40, 0.80)
lgd.SetName('lgdPromptRawFF')
lgd.SetBorderSize(0)
lgd.SetFillColor(0)
lgd.SetTextSize(0.04)
lgd.AddEntry(hDet,'Measured')
lgd.AddEntry(hUnfold,'Unfolded')
lgd.AddEntry(hRefold,'Refolded')
lgd.AddEntry(hTrue_input,'True')

c.Clear()
c.SetWindowSize(1000,1000)
padFF, padRatio = ana_util.NewRatioPads(ROOT.gPad, "cFF", "cRatio")
padFF.cd()
hDet.GetYaxis().SetRangeUser(0, 3.5)
hDet.Draw('PE1')
hUnfold.Draw('SAME PE1')
hRefold.Draw('SAME PE1')
hTrue_input.Draw('SAME PE1')
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

PrintPage(title='RefoldTest')

# End
ana_util.PrintCover(c, pdfOut, isBack = True)
fOut.Close()
