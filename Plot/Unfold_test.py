import ROOT
import ana_util
fRM = ROOT.TFile('RM_Low.root')
fDet = ROOT.TFile('FF_5GeV.root')
hDet = fDet.cFF_SubBdecay.FindObject('hFFPromptCorrected2').Clone('hDet')
hRM = fRM.cRM.FindObject('RM_1_1').Clone('hRM')
ana_util.ResponseNorm(hRM)

response = ROOT.RooUnfoldResponse(hDet,hDet,hRM)
bayes = ROOT.RooUnfoldBayes(response,hDet)
bayes.SetIterations(4)
hTrue = bayes.Hreco(0)
hTrue.SetName('hTrue')
hTrue.Scale(1./hTrue.Integral(),'width')
for i in range(1,11):
  hTrue.SetBinError(i, hDet.GetBinError(i))

ana_util.SetColorAndStyle(hDet, ROOT.kBlack, ana_util.kBlock)
ana_util.SetColorAndStyle(hTrue, ROOT.kBlue, ana_util.kBlock, 2.0)

# Refold
mRM = ROOT.TMatrixD(hRM.GetNbinsX()+2, hRM.GetNbinsY()+2, hRM.GetArray(), 'D')
mRM[0][0] = 1.0
mRM[1][1] = 1.0
mRM[11][11] = 1.0
mRM.Invert()
hRMInv = hRM.Clone('hRMInv')
for i in range(1,11):
  for j in range(1,11):
    hRMInv.SetBinContent(i,j,mRM[i][j])

ana_util.ResponseNorm(hRMInv)

responseRefold = ROOT.RooUnfoldResponse(hTrue,hTrue,hRMInv)
bayesRefold = ROOT.RooUnfoldBayes(responseRefold,hTrue)
bayesRefold.SetIterations(4)
hRefold = bayesRefold.Hreco(0)
hRefold.Scale(1./hRefold.Integral(),'width')
for i in range(1,11):
  hRefold.SetBinError(i, hTrue.GetBinError(i))

ana_util.SetColorAndStyle(hRefold, ROOT.kRed, ana_util.kBlock, 1.0)

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
lgd.AddEntry(hTrue,'Unfolded')
lgd.AddEntry(hRefold,'Refolded')

ROOT.gStyle.SetPalette(ROOT.kInvertedDarkBodyRadiator)
c = ROOT.TCanvas('cUnfold','Unfolding',1600,600)
c.Divide(2)
c.cd(1)
hRM.Draw('COLZ')

c.cd(2)
hDet.Draw('PE1')
hTrue.Draw('SAME PE1')
hRefold.Draw('SAME PE1')
label.Draw('same')
lgd.Draw('same')
pTxtCuts.Draw('same')