{
  const double JPSI_PT_LOWER = 5.0;
  const double JPSI_PT_UPPER = 50.0;
  const double JET_PT_LOWER = 20.0;
  const double JET_PT_UPPER = 50.0;
  const double JPSI_LXY_PROMPT = 0.01;
  const double JPSI_LXY_BDECAY = 0.01;

  auto c = new TCanvas("cAna","J/psi in jets", 1600, 600);
  c->Divide(2);
  c->Draw();
  c->Print("JpsiJet.pdf(","Title:Header");

  auto outputs = (TList*)(_file0->Get("JpsiJetAnalysis/QAhistos_ALL"));
  outputs->SetOwner(kTRUE);

  gROOT->ProcessLine(".L Plot/ExtractSignal.C");
  ana = (TList*)(outputs->FindObject("PairInJet"));
  ana->SetOwner(kTRUE);
  auto hs = (THnSparse*)(ana->FindObject("PairVars"));
  // Dielectron pT
  hs->GetAxis(0)->SetRangeUser(JPSI_PT_LOWER, JPSI_PT_UPPER);
  // Jet pT
  hs->GetAxis(5)->SetRangeUser(JET_PT_LOWER, JET_PT_UPPER);
  // Inv. Mass
  hM = hs->Projection(1);
  hM->SetTitle("Dielctron pair in jets - invariant mass");
  c->cd(1);
  hM->Draw();
  ExtractSignal(1.8, 4.2);
  Double_t sidebandFactor = SelectSignalRegion(JPSI_MASS_LOWER, JPSI_MASS_UPPER, hM->GetBinWidth(1));
  TPaveText* pTxt = DrawCuts(JPSI_PT_LOWER, JPSI_PT_UPPER, 0.9);
  pTxt->AddText("|#eta_{jet}| < 0.5");
  pTxt->AddText(Form("%.1f < p_{T,jet} < %.1f GeV/c", JET_PT_LOWER, JET_PT_UPPER));
  gPad->Modified();
  // Lxy
  hs->GetAxis(1)->SetRangeUser(JPSI_MASS_LOWER , JPSI_MASS_UPPER);
  auto hLxy = hs->Projection(2);
  hLxy->SetTitle("Dielctron pair in jets - Pseudo-proper decay length");
  hLxy->GetYaxis()->SetTitle("N_{pairs} / 0.005 cm");
  hLxy->SetMarkerStyle(20);
  hLxy->SetMarkerColor(kBlack);
  hLxy->SetLineColor(kBlack);
  c->cd(2);
  hLxy->Draw("E");
  gPad->SetLogy(kTRUE);
  c->Print("JpsiJet.pdf","Title:InvMass");
  // End - invariant mass spectrum
  
  // Tagged jets
  // hs->GetAxis(5)->SetRangeUser(0., 100); // Reset jet pT range
    // Prompt - |Lxy| < 0.01
  hs->GetAxis(2)->SetRangeUser(-JPSI_LXY_PROMPT, JPSI_LXY_PROMPT);
      // J/psi mass region
  hs->GetAxis(1)->SetRangeUser(JPSI_MASS_LOWER, JPSI_MASS_UPPER);
  auto hJetPrompt = (TH1D*)(hs->Projection(5)->Clone("hJetPrompt"));
  auto hZPrompt = (TH1D*)(hs->Projection(3)->Clone("hZPrompt"));
      // Sideband
  auto hJetPromptSB = (TH1D*)(hJetPrompt->Clone("hJetPromptSB"));
  auto hZPromptSB = (TH1D*)(hZPrompt->Clone("hZPromptSB"));
  hJetPromptSB->Reset();
  hZPromptSB->Reset();
  hs->GetAxis(1)->SetRangeUser(JPSI_MASS_LOWER - JPSI_SIDEBAND_OFFSET, JPSI_MASS_UPPER - JPSI_SIDEBAND_OFFSET);
  hJetPromptSB->Add(hs->Projection(5), sidebandFactor);
  hZPromptSB->Add(hs->Projection(3), sidebandFactor);
  hs->GetAxis(1)->SetRangeUser(JPSI_MASS_LOWER + JPSI_SIDEBAND_OFFSET, JPSI_MASS_UPPER + JPSI_SIDEBAND_OFFSET);
  hJetPromptSB->Add(hs->Projection(5), sidebandFactor);
  hZPromptSB->Add(hs->Projection(3), sidebandFactor);
      // Substracted
  auto hJetPromptAfter = (TH1D*)(hJetPrompt->Clone("hJetPromptAfter"));
  hJetPromptAfter->Add(hJetPromptSB, -1.0);
  auto hZPromptAfter = (TH1D*)(hZPrompt->Clone("hZPromptAfter"));
  hZPromptAfter->Add(hZPromptSB, -1.0);
      // Drawing
  c->Clear();
  c->Divide(2);
  c->cd(1);
  hJetPrompt->SetMarkerStyle(20);
  hJetPrompt->SetMarkerColor(kRed);
  hJetPrompt->SetLineColor(kRed);
  hJetPrompt->Draw("PE");
  hJetPromptSB->SetMarkerStyle(20);
  hJetPromptSB->SetMarkerColor(kGreen);
  hJetPromptSB->SetLineColor(kGreen);
  hJetPromptSB->Draw("same PE");
  hJetPromptAfter->SetMarkerStyle(20);
  hJetPromptAfter->SetMarkerColor(kBlue);
  hJetPromptAfter->SetLineColor(kBlue);
  hJetPromptAfter->Draw("same PE");
  c->cd(2);
  hZPrompt->SetMarkerStyle(20);
  hZPrompt->SetMarkerColor(kRed);
  hZPrompt->SetLineColor(kRed);
  hZPrompt->Draw("PE");
  hZPromptSB->SetMarkerStyle(20);
  hZPromptSB->SetMarkerColor(kGreen);
  hZPromptSB->SetLineColor(kGreen);
  hZPromptSB->Draw("same PE");
  hZPromptAfter->SetMarkerStyle(20);
  hZPromptAfter->SetMarkerColor(kBlue);
  hZPromptAfter->SetLineColor(kBlue);
  hZPromptAfter->Draw("same PE");
  c->Print("JpsiJet.pdf","Title:PromptJetZ");
    // Non-prompt - Lxy > 0.01

  // FF z
  c->Clear();
  c->Print("JpsiJet.pdf)","Title:End");
  delete outputs;
}