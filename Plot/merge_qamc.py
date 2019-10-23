#!/usr/bin/env python3

import sys, os, time, json
import argparse

# Command-line arguments
parser = argparse.ArgumentParser(description='QA script for MC with pT hard bins')
parser.add_argument('--sub',help='Subcycle: 16, 17 or 18', default='16')
parser.add_argument('-o', '--output',help='Output ROOT file', default='NewMCQA.root')
parser.add_argument('-p', '--print',help='Output PDF', default='NewMCQA.pdf')
args = parser.parse_args()

import ROOT
import ana_util

args.print = args.output.replace('.root','.pdf')

MC_TAG = {}
MC_TAG['18'] = 'LHC19i2a'
MC_TAG['17'] = 'LHC19i2b'
MC_TAG['16'] = 'LHC19i2c'

fout = ROOT.TFile(args.output,'RECREATE')
c = ROOT.TCanvas('cQA','J/#psi in jets MC production - Final QA (%s)' % MC_TAG[args.sub], 1200, 600)
c.Draw()

ana_util.PrintCover(c,args.print)
SCALE_FACTOR = [2.59e-2, 1.35e-2, 7.41e-3, 2.95e-3, 1.21e-3, 5.67e-4, 2.54e-4, 1.17e-4, 4.71e-5, 5.18e-5]
PT_HARD_BINS = [12, 16, 21, 28, 36, 45, 57, 70, 85, 100, -1]
BINNING_PT = ana_util.BINNING_JET_PT
QA_NAME = ['PtHard', 'PtHardScaled', 'VtxZ', 'ElePt','EleDCAxy','EleDCAz', 'JetPt', 'JetNtrk', 'TagJetPt', 'TagJetNtrk', 'DieleJetPt', 'DieleJetNtrk', 'JpsiPt', 'JpsiY']
QA_HIST_CONFIG = {
  'PtHard':{'Logy':True, 'X':[0,200], 'Y': [1e-8, 10.0], 'Ytitle': '1/#it{N}_{ev} d#it{N}_{evts}/d#it{p}_{T}', 'Legend': [0.65,0.1,0.9,0.5], 'Title':'Pythia event info. - pT hard (Unscale)', 'Sum':False},
  'PtHardScaled':{'Logy':True, 'X':[0,200], 'Y': [1e-12, 0.1], 'Ytitle': '1/#it{N}_{ev} d#it{N}_{evts}/d#it{p}_{T}', 'Legend': [0.65,0.55,0.9,0.9], 'Title':'Pythia event info. - pT hard (Scaled)', 'Sum':False},
  'VtxZ':{'Logy':False, 'X':[-20,20], 'Y': [0., 0.1], 'Ytitle': '1/#it{N}_{ev} d#it{N}_{ev}/dZ', 'Legend': [0.1,0.6,0.35,0.9], 'Title':'Event primary vertex Z', 'Sum':False},
  'ElePt':{'Logy':True, 'X':[0,100], 'Y': [1e-10, 10], 'Ytitle': '1/#it{N}_{ev} d#it{N}_{trk}/d#it{p}_{T}', 'Legend': [0.68,0.62,0.9,0.9], 'Title':'Selected track/electron (TPC only) - #it{p}_{T}', 'Sum':True},
  'EleDCAxy':{'Logy':True, 'X':[-2,2], 'Y': [1e-4, 100], 'Ytitle': '1/#it{N}_{ev} d#it{N}_{trk}/d#it{XY}', 'Legend': [0.1,0.6,0.35,0.9], 'Title':'Selected track/electron (TPC only) - #it{DCA}_{xy}', 'Sum':False},
  'EleDCAz':{'Logy':True, 'X':[-5,5], 'Y': [1e-6, 100], 'Ytitle': '1/#it{N}_{ev} d#it{N}_{trk}/d#it{Z}', 'Legend': [0.1,0.6,0.35,0.9], 'Title':'Selected track/electron (TPC only) - #it{DCA}_{z}', 'Sum':False},
  'JetPt':{'Logy':True, 'X':[0,100], 'Y': [1e-9, 1e2], 'Ytitle': '1/#it{N}_{ev} d#it{N}_{jet}/d#it{p}_{T}', 'Legend': [0.68,0.62,0.9,0.9], 'Title':'Inclusive jet - raw #it{p}_{T}', 'Sum':True},
  'JetNtrk':{'Logy':False, 'X':[0,100], 'Y': [0, 12], 'Ytitle': '<#it{N}_{trk}>', 'Legend': [0.1,0.6,0.3,0.9], 'Title':'Inclusive jet - N constituents', 'Sum':True},
  'TagJetPt':{'Logy':True, 'X':[0,100], 'Y': [1e-9, 5], 'Ytitle': '1/#it{N}_{ev} d#it{N}_{jet}/d#it{p}_{T}', 'Legend': [0.72,0.65,0.9,0.9], 'Title':'Inclusive jet (updated) - raw #it{p}_{T}', 'Sum':True},
  'TagJetNtrk':{'Logy':False, 'X':[0,100], 'Y': [0, 12], 'Ytitle': '<#it{N}_{trk}>', 'Legend': [0.1,0.6,0.3,0.9], 'Title':'Inclusive jet (updated) - N constituents', 'Sum':True},
  'DieleJetPt':{'Logy':True, 'X':[0,100], 'Y': [1e-10, 1e-2], 'Ytitle': '1/#it{N}_{ev} d#it{N}_{jet}/d#it{p}_{T}', 'Legend': [0.1,0.1,0.3,0.4], 'Title':'Dielectron tagged jet - raw #it{p}_{T}', 'Sum':True},
  'DieleJetNtrk':{'Logy':False, 'X':[0,100], 'Y': [0, 12], 'Ytitle': '<#it{N}_{trk}>', 'Legend': [0.1,0.6,0.3,0.9], 'Title':'Dielectron tagged jet - N constituents', 'Sum':True},
  'JpsiPt':{'Logy':True, 'X':[0,100], 'Y': [1e-8, 10], 'Ytitle': '1/#it{N}_{ev} d#it{N}_{J/#psi}/d#it{p}_{T}', 'Legend': [0.68,0.62,0.9,0.9], 'Title':'Generated J/#psi - #it{p}_{T}', 'Sum':True},
  'JpsiY':{'Logy':False, 'X':[-2, 2], 'Y': [0, 1], 'Ytitle': '1/#it{N}_{ev} d#it{N}_{J/#psi}/d#it{Y}', 'Legend': [0.75,0.5,0.9,0.9], 'Title':'Generated J/#psi - #it{Y}', 'Sum':False}
}
QA = list(range(len(PT_HARD_BINS) - 1))
for i,pTmin in enumerate(PT_HARD_BINS[:-1]):
  # Init
  QA[i] = {}
  QA[i]['pTHard'] = (PT_HARD_BINS[i], PT_HARD_BINS[i+1])
  if(PT_HARD_BINS[i+1] == -1):
    QA[i]['Title'] = "#it{p}_{T} hard bin > %d GeV/c" % PT_HARD_BINS[i]
  else:
    QA[i]['Title'] = "#it{p}_{T} hard bin: %d - %d GeV/c" % (PT_HARD_BINS[i], PT_HARD_BINS[i+1])
  f = ROOT.TFile('../output/JpsiJetMC_FullQA/JpsiJetMC_FullQA%s_bin%d_191017/AnalysisResults.root' % (args.sub, i+1))
  if(f.IsOpen()):
    print('>>> Processing ' + f.GetName())
    print('[-] Config : ' + QA[i]['Title'])
  else:
    exit(1)
  qa = f.JpsiJetAnalysis.Get('QAhistos_MC')
  qa.SetOwner(True)
  mc = f.JpsiJetAnalysis.Get('MChistos')
  mc.SetOwner(True)
  # QA
    # Generator
  genQA = mc.FindObject('Event')
  genQA.SetOwner(True)
  QA[i]['Ntrials'] = genQA.FindObject('Ntrials').GetBinContent(1)
  QA[i]['Xsec'] = genQA.FindObject('Xsec').GetBinContent(1)
  QA[i]['ScaleFactor'] = QA[i]['Xsec'] / QA[i]['Ntrials']
  QA[i]['PtHard'] = genQA.FindObject('PtHard').Clone('hPtHard_%d' % i)
  QA[i]['PtHardScaled'] = genQA.FindObject('PtHard').Clone('hPtHard_%d' % i)
  QA[i]['PtHardScaled'].Scale(QA[i]['ScaleFactor'])
    # Event
  diele = qa.FindObject('Dielectron')
  diele.SetOwner(True)
  evQA = diele.FindObject('Event')
  evQA.SetOwner(True)
  QA[i]['VtxZ'] = evQA.FindObject('VtxZ').Clone('hVtxZ_%d' % i)
  QA[i]['NEvent'] = QA[i]['VtxZ'].GetEntries()
  print('[-] N event : %1.3e' % QA[i]['NEvent'])
    # Track / Electron
  eleP = diele.FindObject('Track_ev1+')
  eleP.SetOwner(True)
  QA[i]['ElePt'] = eleP.FindObject('Pt').Rebin(len(BINNING_PT)-1,'hElePt_%d' % i, BINNING_PT)
  QA[i]['EleDCAxy'] = eleP.FindObject('dXY').Clone('hEleDXY_%d' % i)
  QA[i]['EleDCAxy'].GetXaxis().SetRangeUser(-2.,2.)
  QA[i]['EleDCAz'] = eleP.FindObject('dZ').Clone('hEleDZ_%d' % i)
  QA[i]['EleDCAz'].GetXaxis().SetRangeUser(-5.,5.)
  eleN = diele.FindObject('Track_ev1-')
  eleN.SetOwner(True)
  QA[i]['ElePt'].Add(eleN.FindObject('Pt').Rebin(len(BINNING_PT)-1,'', BINNING_PT))
  QA[i]['EleDCAxy'].Add(eleN.FindObject('dXY'))
  QA[i]['EleDCAz'].Add(eleN.FindObject('dZ'))
    # Jet
  jetQA = qa.FindObject('Jet_AKTChargedR040_tracks_pT0150_pt_scheme')
  jetQA.SetOwner(True)
  hs = jetQA.FindObject('jetVars')
  QA[i]['JetPt'] = hs.Projection(0).Rebin(len(BINNING_PT)-1,'hJetPt_%d' % i, BINNING_PT)
  QA[i]['JetNtrk'] = jetQA.FindObject('Ntracks_pT').ProfileX('hJetNtrk_%d' % i).Rebin(len(BINNING_PT)-1,'', BINNING_PT)
  taggedJetQA = qa.FindObject('JpsiJet_AKTChargedR040_tracksWithPair_pT0150_pt_scheme')
  taggedJetQA.SetOwner(True)
  hs = taggedJetQA.FindObject('jetVars')
  QA[i]['TagJetPt'] = hs.Projection(0).Rebin(len(BINNING_PT)-1,'hTagJetPt_%d' % i, BINNING_PT)
  QA[i]['TagJetNtrk'] = taggedJetQA.FindObject('Ntracks_pT').ProfileX('hTagJetNtrk_%d' % i).Rebin(len(BINNING_PT)-1,'', BINNING_PT)
    # Dielectron tagged
  tagQA = qa.FindObject('PairInJet')
  tagQA.SetOwner(True)
  hs = tagQA.FindObject('PairVars')
  QA[i]['DieleJetPt'] = hs.Projection(5).Rebin(len(BINNING_PT)-1,'hDieleJetPt_%d' % i, BINNING_PT)
  QA[i]['DieleJetNtrk'] = tagQA.FindObject('Ntracks_pT').ProfileX('hDieleJetNtrk_%d' % i).Rebin(len(BINNING_PT)-1,'', BINNING_PT)
  # MC J/psi
  jpsiQA = mc.FindObject('JpsiBdecay')
  hs = jpsiQA.FindObject('jpsiVars')
  QA[i]['JpsiPt'] = hs.Projection(0).Rebin(len(BINNING_PT)-1,'hJpsiPt_%d' % i, BINNING_PT)
  QA[i]['JpsiY'] = hs.Projection(1)
  QA[i]['JpsiY'].SetName('hJpsiY_%d' % i)
  # End
  for hist in QA_NAME:
    QA[i][hist].SetDirectory(None)
  qa.Delete()
  mc.Delete()
  f.Close()

fout.cd()
for hist in QA_NAME:
  c.Clear()
  c.SetLogy(False)
  ana_util.COLOR = ana_util.SelectColor()
  ana_util.MARKER = ana_util.SelectMarker()
  # Hist config
  cfg = QA_HIST_CONFIG[hist]
  lgd = ROOT.TLegend(cfg['Legend'][0], cfg['Legend'][1], cfg['Legend'][2], cfg['Legend'][3])
  hSum = QA[0][hist].Clone('hSum'+hist)
  hSum.Reset()
  ana_util.SetColorAndStyle(hSum, ROOT.kRed, ana_util.kRound, 1.5)
  if(cfg['Sum']):
    lgd.AddEntry(hSum, 'Sum')
  for i,pTmin in enumerate(PT_HARD_BINS[:-1]):
    color = next(ana_util.COLOR)
    ana_util.SetColorAndStyle(QA[i][hist], color)
    QA[i][hist].GetYaxis().SetTitleOffset(1.2)
    QA[i][hist].Sumw2()
    if(hist.find('Ntrk') < 0):
      QA[i][hist].Scale(1./QA[i]['NEvent'],'width')
    else:
      QA[i][hist].Scale(1.,'width')
    QA[i][hist].SetTitle(QA[i]['Title'])
    hSum.Add(QA[i][hist], QA[i]['ScaleFactor'])
    if(i == 0):
      QA[i][hist].Draw('PE')
    else:
      QA[i][hist].Draw('same PE')
    lgd.AddEntry(QA[i][hist],QA[i][hist].GetTitle())
  if(cfg['Sum']):
    hSum.Draw('same')
  # Hist config
  lgd.Draw('same')
  c.SetLogy(cfg['Logy'])
  QA[0][hist].SetYTitle(cfg['Ytitle'])
  QA[0][hist].GetXaxis().SetRangeUser(cfg['X'][0], cfg['X'][1])
  QA[0][hist].GetYaxis().SetRangeUser(cfg['Y'][0], cfg['Y'][1])
  QA[0][hist].SetTitle(cfg['Title'])
  c.Modified()
  # Output
  c.Print(args.print, 'Title:' + hist)
  c.Write('c' + hist)

for i,pTmin in enumerate(PT_HARD_BINS[:-1]):
  print('[-] INFO : pT hard bin scale factor = %.3e / %.3e = %.2e' % (QA[i]['Xsec'], QA[i]['Ntrials'], QA[i]['ScaleFactor']))

ana_util.PrintCover(c,args.print,'End of QA',isBack=True)
fout.Close()