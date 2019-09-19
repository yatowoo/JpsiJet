
  const double JPSI_PT_LOWER = 5.0;
  const double JPSI_PT_UPPER = 50.0;
  const double JET_PT_LOWER = 20.0;
  const double JET_PT_UPPER = 50.0;
  const double JPSI_LXY_PROMPT = 0.01;
  const double JPSI_LXY_BDECAY = 0.01;

  auto hs = PairVars;
  auto f = new TFile("JpsiJetAna_16Low_jet20.root","RECREATE");
  auto c = new TCanvas("cAna","J/psi in jets", 1600, 600);
  c->Divide(2);
  c->Draw();
  c->Print("JpsiJet.pdf(","Title:Header");

  //auto outputs = (TList*)(_file0->Get("JpsiJetAnalysis/QAhistos_ALL"));
  //outputs->SetOwner(kTRUE);

  gROOT->ProcessLine(".L ExtractSignal.C");
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
  hM->SetYTitle("N_{pairs}");
  TPaveText* pTxt = DrawCuts(JPSI_PT_LOWER, JPSI_PT_UPPER, 0.9);
  pTxt->AddText("|#eta_{jet}| < 0.5");
  pTxt->AddText(Form("%.1f < p_{T,jet} < %.1f GeV/c", JET_PT_LOWER, JET_PT_UPPER));
  gPad->Modified();
  // Lxy
  hs->GetAxis(1)->SetRangeUser(JPSI_MASS_LOWER , JPSI_MASS_UPPER);
  auto hLxy = hs->Projection(2);
  hLxy->SetTitle("Dielctron pair in jets - Pseudo-proper decay length");
  hLxy->GetYaxis()->SetTitle("N_{pairs}");
  hLxy->SetMarkerStyle(20);
  hLxy->SetMarkerColor(kBlack);
  hLxy->SetLineColor(kBlack);
  c->cd(2);
  hLxy->Draw("E");
  gPad->SetLogy(kTRUE);
    // Region
  auto hLxyRegion = (TH1D*)(hLxy->Clone("hLxyRegion"));
  hLxyRegion->GetXaxis()->SetRangeUser(-JPSI_LXY_PROMPT, JPSI_LXY_PROMPT);
  hLxyRegion->SetFillColor(kRed);
  hLxyRegion->SetFillStyle(3005);
  hLxyRegion->Draw("same B")
  auto hLxyRegionB = (TH1D*)(hLxy->Clone("hLxyRegionB"));
  hLxyRegionB->GetXaxis()->SetRangeUser(JPSI_LXY_BDECAY, 0.3);
  hLxyRegionB->SetFillColor(kBlue);
  hLxyRegionB->SetFillStyle(3004);
  hLxyRegionB->Draw("same B")
    // Legend
  auto lgdLxy = new TLegend(0.15, 0.65, 0.45, 0.85);
  lgdLxy->SetBorderSize(0);
  lgdLxy->SetFillColor(0);
  lgdLxy->SetTextSize(0.03);
  lgdLxy->AddEntry(hLxyRegion, Form("|L_{xy}| < %.3f (Prompt)", JPSI_LXY_PROMPT));
  lgdLxy->AddEntry(hLxyRegionB, Form("L_{xy}  > %.3f (Non-prompt)", JPSI_LXY_BDECAY));
  lgdLxy->Draw("same");
  c->Print("JpsiJet.pdf","Title:InvMass");
  c->Write("cJpsiMLxy");
  // End - invariant mass spectrum
  
  // Tagged jets
  // hs->GetAxis(5)->SetRangeUser(0., 100); // Reset jet pT range
    // Prompt - |Lxy| < 0.01
  hs->GetAxis(2)->SetRangeUser(-JPSI_LXY_PROMPT, JPSI_LXY_PROMPT);
      // J/psi mass region
  hs->GetAxis(1)->SetRangeUser(JPSI_MASS_LOWER, JPSI_MASS_UPPER);
  auto hJetPrompt = (TH1D*)(hs->Projection(5)->Clone("hJetPrompt"));
  hJetPrompt->SetTitle("Prompt J/psi tagged jet p_{T} spectra");
  hJetPrompt->GetXaxis()->SetTitle("p_{T,jet} (GeV/c)");
  hJetPrompt->GetYaxis()->SetTitle("N_{pairs}");
  auto hZPrompt = (TH1D*)(hs->Projection(3)->Clone("hZPrompt"));
  hZPrompt->SetTitle("Prompt J/psi tagged jet - fragmentation function");
  hZPrompt->GetYaxis()->SetTitle("N_{pairs}");
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
  hJetPrompt->Rebin(5); // 1 -> 5 GeV
  hJetPrompt->SetMarkerStyle(20);
  hJetPrompt->SetMarkerColor(kRed);
  hJetPrompt->SetLineColor(kRed);
  hJetPrompt->Draw("PE");
  hJetPromptSB->Rebin(5); // 1 -> 5 GeV
  hJetPromptSB->SetMarkerStyle(20);
  hJetPromptSB->SetMarkerColor(kGreen);
  hJetPromptSB->SetLineColor(kGreen);
  hJetPromptSB->Draw("same PE");
  hJetPromptAfter->Rebin(5); // 1 -> 5 GeV
  hJetPromptAfter->SetMarkerStyle(20);
  hJetPromptAfter->SetMarkerColor(kBlue);
  hJetPromptAfter->SetLineColor(kBlue);
  hJetPromptAfter->Draw("same PE");
    // Legend
  auto lgd = new TLegend(0.6, 0.5, 0.8, 0.8);
  lgd->SetBorderSize(0);
  lgd->AddEntry(hJetPrompt, "Total");
  lgd->AddEntry(hJetPromptSB, "Sideband");
  lgd->AddEntry(hJetPromptAfter, "Total - SB");
  lgd->Draw("same");
  c->cd(2);
  hZPrompt->SetMarkerStyle(20);
  hZPrompt->SetMarkerColor(kRed);
  hZPrompt->SetLineColor(kRed);
  hZPrompt->SetBins(10, 0., 1.0);
  hZPrompt->Draw("PE");
  hZPromptSB->SetMarkerStyle(20);
  hZPromptSB->SetMarkerColor(kGreen);
  hZPromptSB->SetLineColor(kGreen);
  hZPromptSB->SetBins(10, 0., 1.0);
  hZPromptSB->Draw("same PE");
  hZPromptAfter->SetMarkerStyle(20);
  hZPromptAfter->SetMarkerColor(kBlue);
  hZPromptAfter->SetLineColor(kBlue);
  hZPromptAfter->SetBins(10, 0., 1.0);
  hZPromptAfter->Draw("same PE");
  c->Print("JpsiJet.pdf","Title:PromptJetZ");
    // Non-prompt - Lxy > 0.01
  hs->GetAxis(2)->SetRangeUser(JPSI_LXY_BDECAY, 0.3);
      // J/psi mass region
  hs->GetAxis(1)->SetRangeUser(JPSI_MASS_LOWER, JPSI_MASS_UPPER);
  auto hJetBdecay = (TH1D*)(hs->Projection(5)->Clone("hJetBdecay"));
  hJetBdecay->SetTitle("Non-Prompt J/psi tagged jet p_{T} spectra");
  hJetBdecay->GetXaxis()->SetTitle("p_{T,jet} (GeV/c)");
  hJetBdecay->GetYaxis()->SetTitle("N_{pairs}");
  auto hZBdecay = (TH1D*)(hs->Projection(3)->Clone("hZBdecay"));
  hZBdecay->SetTitle("Non-prompt J/psi tagged jet - fragmentation function");
  hZBdecay->GetYaxis()->SetTitle("N_{pairs}");
      // Sideband
  auto hJetBdecaySB = (TH1D*)(hJetBdecay->Clone("hJetBdecaySB"));
  auto hZBdecaySB = (TH1D*)(hZBdecay->Clone("hZBdecaySB"));
  hJetBdecaySB->Reset();
  hZBdecaySB->Reset();
  hs->GetAxis(1)->SetRangeUser(JPSI_MASS_LOWER - JPSI_SIDEBAND_OFFSET, JPSI_MASS_UPPER - JPSI_SIDEBAND_OFFSET);
  hJetBdecaySB->Add(hs->Projection(5), sidebandFactor);
  hZBdecaySB->Add(hs->Projection(3), sidebandFactor);
  hs->GetAxis(1)->SetRangeUser(JPSI_MASS_LOWER + JPSI_SIDEBAND_OFFSET, JPSI_MASS_UPPER + JPSI_SIDEBAND_OFFSET);
  hJetBdecaySB->Add(hs->Projection(5), sidebandFactor);
  hZBdecaySB->Add(hs->Projection(3), sidebandFactor);
      // Substracted
  auto hJetBdecayAfter = (TH1D*)(hJetBdecay->Clone("hJetBdecayAfter"));
  hJetBdecayAfter->Add(hJetBdecaySB, -1.0);
  auto hZBdecayAfter = (TH1D*)(hZBdecay->Clone("hZBdecayAfter"));
  hZBdecayAfter->Add(hZBdecaySB, -1.0);
      // Drawing
  c->Clear();
  c->Divide(2);
  c->cd(1);
  hJetBdecay->Rebin(5); // 1 -> 5 GeV
  hJetBdecay->SetMarkerStyle(20);
  hJetBdecay->SetMarkerColor(kRed);
  hJetBdecay->SetLineColor(kRed);
  hJetBdecay->Draw("PE");
  hJetBdecaySB->Rebin(5); // 1 -> 5 GeV
  hJetBdecaySB->SetMarkerStyle(20);
  hJetBdecaySB->SetMarkerColor(kGreen);
  hJetBdecaySB->SetLineColor(kGreen);
  hJetBdecaySB->Draw("same PE");
  hJetBdecayAfter->Rebin(5); // 1 -> 5 GeV
  hJetBdecayAfter->SetMarkerStyle(20);
  hJetBdecayAfter->SetMarkerColor(kBlue);
  hJetBdecayAfter->SetLineColor(kBlue);
  hJetBdecayAfter->Draw("same PE");
  lgd->Clear("C");
  lgd->AddEntry(hJetBdecay, "Total");
  lgd->AddEntry(hJetBdecaySB, "Sideband");
  lgd->AddEntry(hJetBdecayAfter, "Total - SB");
  lgd->Draw("same");
  c->cd(2);
  hZBdecay->SetMarkerStyle(20);
  hZBdecay->SetMarkerColor(kRed);
  hZBdecay->SetLineColor(kRed);
  hZBdecay->SetBins(10, 0., 1.0);
  hZBdecay->Draw("PE");
  hZBdecaySB->SetMarkerStyle(20);
  hZBdecaySB->SetMarkerColor(kGreen);
  hZBdecaySB->SetLineColor(kGreen);
  hZBdecaySB->SetBins(10, 0., 1.0);
  hZBdecaySB->Draw("same PE");
  hZBdecayAfter->SetMarkerStyle(20);
  hZBdecayAfter->SetMarkerColor(kBlue);
  hZBdecayAfter->SetLineColor(kBlue);
  hZBdecayAfter->SetBins(10, 0., 1.0);
  hZBdecayAfter->Draw("same PE");
  c->Print("JpsiJet.pdf","Title:BdecayJetZ");
  // FF z
  c->Clear();
  c->Print("JpsiJet.pdf)","Title:End");
  f->Write();
  f->Close();
