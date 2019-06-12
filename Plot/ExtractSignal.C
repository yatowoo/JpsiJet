// Macro for result post-processing

// Fit J/psi signal in Me+e- spectrum (invariant mass of di-electron)

#define DEBUG

TF1* jpsi = NULL;
TF1* bkg = NULL;
TF1* total = NULL;

TLegend* yatoLegend(){
  // NDC x1, y1, x2, y2, bottom-right
  auto yLgd= new TLegend(0.13,0.60,0.49, 0.88, "", "brNDC");
  yLgd->SetName("yLgd");
  yLgd->SetBorderSize(0);
  yLgd->SetTextAlign(12);
  yLgd->SetTextFont(42);
  yLgd->SetTextSize(0.04);
  return yLgd;
}

TPaveText* yatoPaveText(){
  // NDC x1, y1, x2, y2, bottom-right
  auto pTxt = new TPaveText(0.62, 0.39, 0.87, 0.88, "brNDC");
  pTxt->SetName("yTxt");
  pTxt->SetBorderSize(0);
  pTxt->SetTextAlign(12);
  pTxt->SetTextFont(42);
  pTxt->SetTextSize(0.04);
  pTxt->SetFillColor(0);
  return pTxt;
}

// Input mass range for estimation of signal & background
int SelectSignalRegion(TH1* InvMass, Double_t mlow, Double_t mup, Double_t width = 0.04){
  /*
  * Draw signal region
  */
  if(!InvMass){
    cout << "[X] ERROR - Histogram in NULL" << endl;
    return 1;
  }
  // Lines
  Double_t ymin = InvMass->GetMinimum();
  Double_t ymax = InvMass->GetMaximum();
  TLine* yLine = new TLine(mlow, ymin, mlow, ymax);
  yLine->SetLineColor(kRed);
  yLine->SetLineStyle(3);
  yLine->Draw("same");
  yLine = (TLine*)(yLine->Clone("lJpsiHigh"));
  yLine->SetX1(mup);
  yLine->SetX2(mup);
  yLine->Draw("same");
  // Fill area
  TF1* fRegion = new TF1("fRegion", total->GetName(), mlow, mup);
  TGraph* gr = new TGraph(fRegion);
  gr->SetName("grRegion");
  gr->SetFillColor(kRed);
  gr->SetFillStyle(3004);
  gr->Draw("same B");

  
  /*
  * Count data points
  */
  Int_t xBinLow = InvMass->FindBin(mlow);
  Int_t xBinHigh = InvMass->FindBin(mup);
  Double_t errData = 0.0;
  Double_t Ndata = InvMass->IntegralAndError(xBinLow, xBinHigh, errData);


  /*
  * Integral errors
  */
  // Denominator check
  if( width < 1e-6 ){
    cout << "[+] WARNNING - Bin width given may be too small : " << width << endl;
  }
  // Fit result check
  auto fitter = TVirtualFitter::GetFitter();
  if(!fitter && !fitter->GetCovarianceMatrix()){
    cout << "[X] ERROR - No fit result found." << endl;
    return 1;
  }
  // Calculate signal & background number with TF1::Integral
  Double_t Ntotal = total->Integral(mlow, mup) / width;
  Double_t Njpsi = jpsi->Integral(mlow, mup) / width;
  Double_t Nbkg = bkg->Integral(mlow, mup) / width;
  // Integral error with sub-covariance matrix
  Double_t errTot = total->IntegralError(mlow, mup) / width;
  
  TMatrixDSym* covTot = new TMatrixDSym(0, 7, fitter->GetCovarianceMatrix());
  TMatrixDSym covJpsi = covTot->GetSub(0,4,0,4);
  TMatrixDSym covBkg = covTot->GetSub(5,7,5,7);
#ifdef DEBUG
  covTot->Print();
  covJpsi.Print();
  covBkg.Print();
#endif
  delete covTot;

  jpsi->SetParErrors(total->GetParErrors());
  bkg->SetParErrors(total->GetParErrors() + 5);
  Double_t errJpsi = jpsi->IntegralError(mlow, mup, jpsi->GetParameters(), covJpsi.GetMatrixArray()) / width;
  if(errJpsi < 1.)
    cout << "[+] WARNNING - Njpsi error would be too small : " << errJpsi << endl;
  Double_t errBkg = bkg->IntegralError(mlow, mup, bkg->GetParameters(), covBkg.GetMatrixArray()) / width;
  if(errBkg < 1.)
    cout << "[+] WARNNING - Nbackground error would be too small : " << errBkg << endl;
  // Result output
  cout << "[-] INFO - Estimation of J/psi signal & background in mass range [" << mlow << ", " << mup << "] (GeV/c^2)" << endl;
  cout << "--> Total:      " << Ntotal << " +/- " << errTot << endl;
  cout << "--> Signal:     " << Njpsi << " +/- " << errJpsi << endl;
  cout << "--> Background: " << Nbkg << " +/- " << errBkg << endl;


  /*
  * Build result pave on canvas
  */
  auto pTxt = yatoPaveText();
    // J/psi mass range
  auto entry = pTxt->AddText(
    Form("M_{J/#psi} #in [%.2f, %.2f] (GeV/c^{2})", mlow, mup));
  entry->SetTextSize(0.03);
  entry->SetTextFont(62); // Helvetica (Bold)
  entry = pTxt->AddText(
    Form("Data:     %.0f #pm %.0f", Ndata, errData));
  entry = pTxt->AddText(
    Form("Total:    %.0f #pm %.0f", Ntotal, errTot));
  entry = pTxt->AddText(
    Form("Signal:  %.0f #pm %.0f", Njpsi, errJpsi));
  entry = pTxt->AddText(
    Form("Bkg:      %.0f #pm %.0f", Nbkg, errBkg));
    // S/B significance
  Double_t errSB = Njpsi / Nbkg;
  errSB *= TMath::Sqrt(errJpsi*errJpsi/Njpsi/Njpsi + errBkg*errBkg/Nbkg/Nbkg);
  entry = pTxt->AddText(
    Form("S/B        = %.2f #pm %.2f", Njpsi / Nbkg, errSB));
    // S/#sqrt{S+B} significance
  Double_t errSN = Njpsi / TMath::Sqrt(Ntotal);
  errSN *= TMath::Sqrt(errJpsi*errJpsi/Njpsi/Njpsi + errTot*errTot/Ntotal/Ntotal/4);
  entry = pTxt->AddText(
    Form("S/#sqrt{S+B}  = %.2f #pm %.2f", Njpsi / TMath::Sqrt(Ntotal), errSN));
    // Chi2
  entry = pTxt->AddText(
    Form("#chi^{2} / NDF = %.1f / %d", total->GetChisquare(), total->GetNDF()));
  pTxt->Draw("same");

  return 0;
}

int ExtractSignal(TH1* invmass, Double_t mlow = 1.5, Double_t mup = 4.5){
  // Crystalball Function = Gaus + X^n
    // Parameters : \alpha = break point, n, \sigma = width, \mu = peak
    // Unit : Y = Ncount/0.04, X=GeV/c^2
  if(!jpsi)
    jpsi = new TF1("fJpsi", "[0]*ROOT::Math::crystalball_function(x,[1],[2],[3],[4])", mlow, mup);
  jpsi->SetLineColor(kBlack);
  jpsi->SetLineWidth(2);
  jpsi->SetLineStyle(2);
  jpsi->SetNpx(1000);

  // Backgroud Function - Pol2
  if(!bkg)
    bkg = new TF1("fBkg", "[0]+[1]*x+[2]*x^2", mlow, mup);
  bkg->SetLineColor(kGreen);
  bkg->SetLineWidth(2);
  bkg->SetLineStyle(2);
  bkg->SetNpx(1000);

  // Total function for fitting
  if(!total)
  total = new TF1("fTot","fJpsi+fBkg", mlow, mup);
  total->SetParNames("A", "#alpha", "n", "#sigma", "#mu",
    "a0", "a1", "a2");
  total->SetLineColor(kRed);
  total->SetLineWidth(3);
  total->SetNpx(1000);
    // Jpsi parameter
  const Double_t MASS_JPSI = 3.096; // GeV/c^2
  Double_t pseudo_peak = invmass->GetBinContent(invmass->FindBin(MASS_JPSI)); // Signal pseudo-peak
  total->SetParameter("A", pseudo_peak);
  total->SetParLimits(0, 0., 3 * pseudo_peak);
  total->SetParameter("#alpha", 0.3);
  total->SetParLimits(1, 0., 10.);
  total->SetParameter("n", 1.);
  total->SetParLimits(2, 0., 100.);
  total->SetParameter("#sigma", 0.1);
  total->SetParLimits(3, 0., 1.);
  total->SetParameter("#mu", MASS_JPSI);
  total->SetParLimits(4, mlow, mup);
    // Background parameter
  total->SetParameter("a0", 250.);
  total->SetParameter("a1", -100);
  total->SetParameter("a2", 12.);

  // Check input histogram
  if(invmass == NULL) return 1;

  // Fit & draw
  invmass->SetLineColor(kBlue);
  invmass->SetMarkerColor(kBlue);
  invmass->SetMarkerStyle(20);
    // Fit with bin integration and return fit result
  TFitResultPtr fitResult = invmass->Fit(total, "IS", "", mlow, mup);
    // TODO: Should repeat for N times?
  if(fitResult->Status())
    fitResult = invmass->Fit(total, "IS", "", mlow, mup);
    // Draw marker & error bar (with empty bins)
  invmass->Draw("PE0");

  // Canvas configuration
  if(gPad){
    invmass->GetXaxis()->SetRangeUser(1.5,4.5);
    invmass->GetXaxis()->SetTitle("M_{e^{+}e^{-}} (GeV/c^{2})");
    // Y axis -> 2 * HIST max
    invmass->GetYaxis()->SetRangeUser(0.1, 
      2 * invmass->GetBinContent(invmass->GetMaximumBin()));
    invmass->GetYaxis()->SetTitle(
      Form("N_{pairs} / %.3f GeV/c^{2}", invmass->GetBinWidth(1)));
      // Show fit parameters
    gStyle->SetOptFit(0000);
    gStyle->SetOptStat(0);
    gPad->Update();
  }

  total->Draw("same");

  jpsi->SetParameter(0, total->GetParameter("A"));
  jpsi->SetParameter(1, total->GetParameter("#alpha"));
  jpsi->SetParameter(2, total->GetParameter("n"));
  jpsi->SetParameter(3, total->GetParameter("#sigma"));
  jpsi->SetParameter(4, total->GetParameter("#mu"));
  jpsi->Draw("same");
  
  bkg->SetParameter(0, total->GetParameter("a0"));
  bkg->SetParameter(1, total->GetParameter("a1"));
  bkg->SetParameter(2, total->GetParameter("a2"));
  bkg->Draw("same");

  // Setup for legend
  auto lgd = yatoLegend();
  lgd->AddEntry(invmass, "e^{+}e^{-} signal");
  lgd->AddEntry(total,"Total fit");
  lgd->AddEntry(bkg,"Background fit (pol2)");
  lgd->AddEntry(jpsi,"Signal fit (Crystal-Ball)");
  lgd->Draw("same");

  return 0;
}