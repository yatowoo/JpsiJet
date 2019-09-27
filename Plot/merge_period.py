#!/usr/bin/env python3

# Merge result from different period (DATA and MC)

import ROOT
import ana_phys
import ana_util
from ana_util import *

# DATA outputs: Ana_[Period]_MultiTasks.root
# MC outputs: Ana_MC[Period].root

PERIOD_LIST = 'ghijklop' # LHC16

OUTPUT_DIR = '../output/QM19/'

DATA_MARKER = [kRound,  kBlock, kDelta, kNabla, kPenta, kDiamond, kCross, kClover, kClover4, kStar, kIronCross, kXMark]

MC_MARKER = [kRoundHollow, kBlockHollow, kDeltaHollow, kNablaHollow, kPentaHollow, kDiamondHollow, kCrossHollow, kCloverHollow, kClover4Hollow, kStarHollow, kIronCrossHollow, kXMarkHollow]

padMerge = ROOT.TCanvas('cMerge','Jpsi in jets - period merged', 1600, 600)
padMerge.Divide(2)
padMerge.Draw()
padMerge.SetLogy()

HistMerge = {}
HistMerge['DCAxy'] = {}
HistMerge['DCAz'] = {}

# Global Settings and Variables
ROOT.gStyle.SetOptStat(0)
ROOT.gStyle.SetPalette()

lgd = ROOT.TLegend(0.15, 0.5, 0.3, 0.85)
lgd.SetBorderSize(0)
lgd.SetFillColor(0)
lgd.SetNColumns(2)

def MergePeriod(i, period, MC=False):
  period = '16' + period
  if(not MC):
    fAna = ROOT.TFile(OUTPUT_DIR + 'Ana_' + period + '_MultiTasks.root')
  else:
    fAna = ROOT.TFile(OUTPUT_DIR + 'Ana_MC' + period + '.root')
  if(fAna.IsOpen()):
    print('[-] INFO - Processing ' + fAna.GetName())
  else:
    return
  if(MC):
    period = 'MC' + period
  EvTag = 'MC' if MC else 'EG2'
  qa = fAna.JpsiJetAnalysis.Get('QAhistos_' +EvTag)
  qa.SetOwner(True)
  # Event stats
  hEv = qa.FindObject('EventStats')
  N_EVENT = hEv.GetBinContent(6)
  # Dielectron
  die = qa.FindObject('Dielectron')
  die.SetOwner(True)
    # Electron / Track
  trkQA = die.FindObject('Track_ev1+')
  trkQA.SetOwner(True)
  trkNegative = die.FindObject('Track_ev1-')
  trkNegative.SetOwner(True)
  trkQA.Add(trkNegative)
    # Style
  color = COLOR_SET[i]
  marker = MC_MARKER[i] if MC else DATA_MARKER[i]
    # QA histograms - DCY xy
  padMerge.cd(1)
  ROOT.gPad.SetLogy()
  htmp = trkQA.FindObject('dXY')
  hDCAxy = htmp.Clone('hDCAxy_' + period)
  hDCAxy.SetDirectory(0x0)
  ana_util.SetColorAndStyle(hDCAxy, color, marker)
  hDCAxy.Scale(1./N_EVENT)
  hDCAxy.SetTitle('')
  hDCAxy.SetYTitle('N_{trk} / N_{event}')
  hDCAxy.GetXaxis().SetRangeUser(-2,2)
  hDCAxy.GetYaxis().SetRangeUser(1e-7,1)
  hDCAxy.Draw('same PE')
  HistMerge['DCAxy'][period] = hDCAxy
    # QA histograms - DCY z
  padMerge.cd(2)
  ROOT.gPad.SetLogy()
  htmp = trkQA.FindObject('dZ')
  hDCAz = htmp.Clone('hDCAz_' + period)
  hDCAz.SetDirectory(0x0)
  ana_util.SetColorAndStyle(hDCAz, color, marker)
  hDCAz.Scale(1./N_EVENT)
  hDCAz.SetTitle('')
  hDCAz.SetYTitle('N_{trk} / N_{event}')
  hDCAz.GetXaxis().SetRangeUser(-5,5)
  hDCAz.GetYaxis().SetRangeUser(1e-7,1)
  hDCAz.Draw('same PE')
  lgd.AddEntry(hDCAz,period)
  HistMerge['DCAz'][period] = hDCAz
  # End
  qa.Delete()
  fAna.Close()

for i,period in enumerate(PERIOD_LIST):
  MergePeriod(i, period)
  MergePeriod(i,period,MC=True)

padMerge.cd(1)
lgd.Draw('same')
padMerge.cd(2)
lgd.Draw('same')
padMerge.SaveAs('DCA.pdf')
padMerge.SaveAs('DCA.root')
