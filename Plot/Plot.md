# README - Post processing for J/$\psi$ in jets analysis

## Input files

Analysis results output (data and MC) from JpsiJet task.

### Data: `AnaMerge_Full_Train.root`

  J/$\psi$ tagged jets information, __TagInfoL/H__ (THnSparse - $p_{\rm e^{+}e^{-},T}, M_{\rm e^{+}e^{-}} (\rm GeV/\it c^{2}), \tilde{L}_{\rm xy}, z(p_{T}^{pair}/p_{T}^{jet}), \Delta R (not~used), p_{\rm T,jet} (\rm GeV/\it c)$), generated by __merge_preiod.py__.

### MC

Generated by __JpsiJetAna.py__ with `--mc` option

* J/$\psi$ efficiency: `JpsiEff*.root/hJpsiPromptM or hJpsiBdecayM`
* MC signal shape: `JpsiEff*.root/hJpsiPromptM or hJpsiBdecayM`
* Lxy template: signal (`JpsiEff*.root/hJpsiLxyPrompt or hJpsiLxyBdecay`) from MC, background (`LxySBtest.root/hLxySB`) from sideband in data.
* response matrix: `JpsiBJet_DetResponse*.root/Jet_DetResponse` (THnSparse - $z_{\rm det}, z_{\rm gen}, p_{\rm T,jet}^{\rm det}, p_{\rm T,jet}^{\rm gen} (\rm GeV/\it c), \delta z, \delta p_{\rm T,jet}$)).

## Processing

### Raw Fragmentation function

Script: FF.py

Usage: Generate raw fragmentation function with EMCal __lower__ trigger data ($E$ > 5 GeV), cuts: 5 < $p_{\rm T,jet}$ < 35 GeV/$c$ and 15 < $p^{\rm{J}/\psi}_{\rm T}$ < 35 GeV/$c$.

`./FF.py -f AnaMerge_Full_Train.root --mc JpsiEff_MCALL_LOW.root --jetCut 5 35 --jpsiCut 15 35 --trig L -o FF_5GeV_test.root`

Output raw FF - `FF_*.root/`

- Prompt: `cFF_SubBdecay->FindObject('hFFPromptCorrected2')`
- Non-prompt: `cFF_BdecayCorrected->FindObject('hFFBdecayCorrected')`

### Unfolding

Script: Unfold_test.py

Example: set user range for $z$ and $p_{\rm T,jet}$

Projected $z$ range: 0 - 1.1, bin w = 0.1