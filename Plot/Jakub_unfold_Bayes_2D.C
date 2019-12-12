//-----------------------------------------------------------------------
//  Author B.Trzeciak
//  Utrecht University
//  barbara.antonina.trzeciak@cern.ch
//-----------------------------------------------------------------------
//https://arxiv.org/pdf/1105.1160.pdf
//http://hep.physics.utoronto.ca/~orr/wwwroot/Unfolding/d-agostini.pdf

#include "config.h"
#include "TRatioPlot.h"
#include "TF2.h"

//====================== global =========================
TH2D *fMatrixPP;
TH2D *fMatrixProd;
TH2D *fMatrixDeltaPt;
TH2D *fRawSpectrum;
TH2D *fTrueSpectrum;
TH2D *fMeasSpectrum;
Int_t fBin;
TH2D *resSlices[fzptJetTrueN][fzptJetMeasN];
TH2D *fTrueSpectrumClosure;
TH2D *fMeasSpectrumClosure;
TH2D *fTrueSpectrumKineNum;
TH2D *fTrueSpectrumKineDen;
TH2D *fMeasSpectrumKineNum;
TH2D *fMeasSpectrumKineDen;

//RooUnfoldResponse *response = new RooUnfoldResponse();

/***********************************
    ############# define your bins #####################
    ************************************/

Color_t colortable2[] = {kMagenta, kViolet, kBlue, kCyan + 2, kRed + 1, kGreen + 4, kGreen + 1, kYellow + 1, kOrange + 1, kRed + 2};
Style_t linesytle[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13};
// Int_t fColors[] = {1,2,8,4,kOrange-1,6,kGray+1,kCyan+1,kMagenta+2,kGreen+3,kViolet+5,kYellow+2};
//int * LoadDetectorMatrix(TString fn){};
std::tuple<RooUnfoldResponse *, RooUnfoldResponse *> LoadDetectorMatrix(TString MCfile, TString out, Double_t RMfraction = 0, TH2D *prior = nullptr);
int LoadBackgroundMatrix(TString fn, TString mxname);
int LoadRawSpectrum(TString fn, TString sname);
TH2D *getResponseMatrix(bool useDeltaPt);
TH2D *ProductMatrix(TH2D *MtxA, TH2D *MtxB);
int MtxPlots(TString outDir, TString outName);
int plotSlice(TVirtualPad *p, TH2D *hMtxPP, TH2D *hMtxDpt, TH2D *hMtxRe, TH2D *hMtxPro, const double ptmin, const double ptmax);
void WeightMatrixY(TH2D *Mtx, TH1D *h, bool divide);
TH2D *Rebin2D(const char *name, TH2D *h, int nx, const double *binx, int ny, const double *biny, bool crop);
TH2D *getPriorFunction(TH2D *baseweight, TF2 *modfunc, Int_t modsign = 0, Int_t modpower = 0);
TH2D *NormMatrixY(const char *name, TH2D *Mtx);
void NormMatrixY(TH2D *Mtx);
TH2D *getPearsonCoeffs(const TMatrixD &covMatrix);
void SparseToTree(TString h);

/***********************************
############# begining of the macro ##################
************************************/
void unfold_Bayes_2D(
    TString roounfoldpwd = "",
    TString datafile = "file.root",
    TString MCfile = "mc.root",
    TString detRMfile = "detRM.root",
    TString bkgRMfile = "bkgRM.root",
    TString outDir = "out", // output directory
    const int regBayes = 5, // default reg. parameter for the bayes unfolding
    bool isPrior = 0,       // if to use prior different than the true spectrum from the sim
    int priorType = 1,      // if isPrior == 1, choose type of the prior
    Int_t zBin = 0,
    bool useDeltaPt = 1, // if to use a separate bkg. fluctuation matrix
    bool isFDUpSpec = 0,
    bool isFDDownSpec = 0,
    bool fDoWeighting = 1,
    bool fdivide = 1,
    bool overflow = 1,      // if to use overflow in the unfolding
    const int NTrials = 10, //10,  //number of total trials
    bool debug = 0)
{
  fBin = zBin;
  gSystem->Load(Form("%s", roounfoldpwd.Data()));
  gStyle->SetOptStat(0000); //Mean and RMS shown
  gSystem->Exec(Form("mkdir  %s", outDir.Data()));
  gSystem->Exec(Form("mkdir  %s/plots", outDir.Data()));

  double plotmin = fptbinsJetTrueA[0];
  double plotmax = fptbinsJetTrueA[fptbinsJetTrueN];
  if (fObservable == Observable::kFragmentation)
  {
    plotmin = fzbinsJetTrueAPrompt[fBin - 1][0];
    plotmax = fzbinsJetTrueAPrompt[fBin - 1][fzbinsJetTrueN[fBin - 1]];
  }

  TString outName = "unfoldedSpectrum";

  /***********************************
############# load raw spectrum and response matrices ##################
************************************/
  /*
    TFile *File = new TFile(MCfile,"read");
    TDirectoryFile* dir=(TDirectoryFile*)File->Get("DmesonsForJetCorrelations");
    TString histName;
    if(fDmesonSpecie) histName = "histosDStarMBN";
                    else histName = "histosD0MBN";
    TList *histList[NDMC];
    THnSparseF *sparseMC[NDMC];
    THnSparseF *h = nullptr;

    for(int i=0; i<2; i++){
        histList[i] =  (TList*)dir->Get(Form("%s%dMCrec",histName.Data(),i));
        sparseMC[i] = (THnSparseF*)histList[i]->FindObject("ResponseMatrix");
        std::cout<<sparseMC[i]->GetNbins()<<std::endl;
        if(!i)h = dynamic_cast<THnSparseF*>(sparseMC[0]->Clone("ResponseMatrixSum"));
        else h->Add(sparseMC[i]);
    }
return;*/
  if (isFDUpSpec)
    LoadRawSpectrum(datafile.Data(), "hData_binned_sub_up");
  else if (isFDDownSpec)
    LoadRawSpectrum(datafile.Data(), "hData_binned_sub_down");
  else
    LoadRawSpectrum(datafile.Data(), "hData_binned_sub");
  //RooUnfoldResponse *response = new RooUnfoldResponse();
  RooUnfoldResponse *response, *responseClosure;
  std::tie(response, responseClosure) = LoadDetectorMatrix(MCfile, outDir + "/plots", 0.48, nullptr);

  if (!fRawSpectrum)
  {
    Error("Unfold", "No raw spectrum!");
    return;
  }
  if (!fTrueSpectrum)
  {
    Error("Unfold", "No true spectrum!");
    return;
  }
  if (!fMeasSpectrum)
  {
    Error("Unfold", "No reconstructed spectrum!");
    return;
  }
  if (!fTrueSpectrumClosure)
  {
    Error("Unfold", "No true Closure spectrum!");
    return;
  }
  if (!fMeasSpectrumClosure)
  {
    Error("Unfold", "No reconstructed Closure spectrum!");
    return;
  }
  if (!response)
  {
    Error("Unfold", "No Response Matrix!");
    return;
  }
  if (!responseClosure)
  {
    Error("Unfold", "No Closure Response Matrix!");
    return;
  }

  Double_t nIntRaw = 0;
  Double_t nIntMeas = 0;
  Double_t nIntMeasC = 0;
  for (Int_t i = 1; i <= fzptJetMeasN; i++)
  {
    for (Int_t j = 1; j <= fzbinsJetMeasN[fBin - 1]; j++)
    {
      nIntRaw += fRawSpectrum->GetBinContent(j, i);
      nIntMeas += fMeasSpectrum->GetBinContent(j, i);
      nIntMeasC += fMeasSpectrumClosure->GetBinContent(j, i);
    }
  }
  std::cout << "raw Int: " << nIntRaw << " meas Int " << nIntMeas << " 10% " << nIntMeasC << " ratio " << nIntMeasC / nIntMeas * 100 << std::endl;
  fRawSpectrum->GetXaxis()->SetTitle("z_{#parallel}");
  fRawSpectrum->GetYaxis()->SetTitle("p_{T,jet}^{ch} (GeV/#it{c})");

  fMeasSpectrum->GetXaxis()->SetTitle("z_{#parallel}^{DET}");
  fMeasSpectrum->GetYaxis()->SetTitle("p_{T,jet}^{ch,DET} (GeV/#it{c})");
  fMeasSpectrumClosure->GetXaxis()->SetTitle("z_{#parallel}^{DET}");
  fMeasSpectrumClosure->GetYaxis()->SetTitle("p_{T,jet}^{ch,DET} (GeV/#it{c})");

  fTrueSpectrum->GetXaxis()->SetTitle("z_{#parallel}^{MC}");
  fTrueSpectrum->GetYaxis()->SetTitle("p_{T,jet}^{ch,MC} (GeV/#it{c})");
  fTrueSpectrumClosure->GetXaxis()->SetTitle("z_{#parallel}^{MC}");
  fTrueSpectrumClosure->GetYaxis()->SetTitle("p_{T,jet}^{ch,MC} (GeV/#it{c})");

  //get pripr
  if (isPrior == 1)
  {
    outDir += Form("prior%d", priorType);
    gSystem->Exec(Form("mkdir  %s", outDir.Data()));
    gSystem->Exec(Form("mkdir  %s/plots", outDir.Data()));
    RooUnfoldBayes unfoldPrior(response, fRawSpectrum, regBayes);
    //TH2D* weight = dynamic_cast<TH2D*>(unfoldPrior.Hreco());
    TH2D *weight = dynamic_cast<TH2D *>(fTrueSpectrum->Clone("weight"));
    for (Int_t binx = 0; binx <= weight->GetNbinsX(); binx++)
    {
      for (Int_t biny = 0; biny <= weight->GetNbinsY(); biny++)
      {
        weight->SetBinContent(binx, biny, 1);
      }
    }

    // weight->Divide(fTrueSpectrum);
    // weight->Scale(1./weight->Integral());
    TF2 *pri = nullptr;
    /*TH2D* prior = dynamic_cast<TH2D*>(fTrueSpectrum->Clone("prior"));
        for(Int_t binx = 0; binx <= prior->GetNbinsX();binx++){
            for(Int_t biny = 0; biny <= prior->GetNbinsY();biny++){
            prior->SetBinContent(binx,biny,1);
            }

            }
        prior->Divide(fTrueSpectrum);*/

    TH2D *prior = nullptr;
    switch (priorType)
    {
    case 0:
      pri = new TF2("pri", "((70-y)/70+x-1)", 0, 1, 0, 70);
      prior = getPriorFunction(weight, pri, 1, 2);
      break;
    case 1:
      pri = new TF2("pri", "((70-y)/70+x-1)", 0, 1, 0, 70);
      prior = getPriorFunction(weight, pri, -1, 2);
      break;
    case 2:
      pri = new TF2("pri", "((70-y)/70+2*x-1)", 0, 1, 0, 70);
      prior = getPriorFunction(weight, pri, 1, 3);
      break;
    case 3:
      pri = new TF2("pri", "((70-y)/70+2*x-1)", 0, 1, 0, 70);
      prior = getPriorFunction(weight, pri, -1, 3);
      break;
    case 4:
      pri = new TF2("pri", "((70-y)/70+3*x-1)", 0, 1, 0, 70);
      prior = getPriorFunction(weight, pri, 1, 4);
      break;
    case 5:
      pri = new TF2("pri", "((70-y)/70+3*x-1)", 0, 1, 0, 70);
      prior = getPriorFunction(weight, pri, -1, 4);
      break;
    case 6:
      pri = new TF2("pri", "(2*(70-y)/70+x-1)", 0, 1, 0, 70);
      prior = getPriorFunction(weight, pri, 1, 3);
      break;
    case 7:
      pri = new TF2("pri", "(2*(70-y)/70+x-1)", 0, 1, 0, 70);
      prior = getPriorFunction(weight, pri, -1, 3);
      break;
    }

    TCanvas *cprior = new TCanvas("cprior", "cprior", 800, 800);
    cprior->cd();
    prior->Draw("colz text");
    cprior->SaveAs(outDir + "/plots/priorweight.png");
    std::tie(response, responseClosure) = LoadDetectorMatrix(MCfile, outDir + "/plots", 0.48, prior);
  }

  /*
        TF1* fPriorFunction;
        if(isPrior) {
                    fPriorFunction = getPriorFunction(isPrior, priorhisto,priorType, rawspectrum);
                    priorhisto->SetTitle("");
                    priorhisto->GetXaxis()->SetTitle("p_{T, ch.jet}");
                    if (fObservable == Observable::kFragmentation)priorhisto->GetXaxis()->SetTitle("z_{#parallel}");
                    TCanvas* cPrior = new TCanvas("cPrior0", "cPrior0", 800, 600);
                    cPrior->SetLogy();
                    TH1D* histoPrior=dynamic_cast<TH1D*>(priorhisto->Clone());
                    histoPrior->Draw();
                    if(priorType == 8) rawspectrum->Draw();
                    fPriorFunction->Draw("same");

                    cPrior->SaveAs(Form("%s/plots%s/%s_prior.pdf",outDir.Data(),(zBin!=0)?Form("%d",zBin):"",outName.Data()));
                    cPrior->SaveAs(Form("%s/plots%s/%s_prior.png",outDir.Data(),(zBin!=0)?Form("%d",zBin):"",outName.Data()));
                    cPrior->SaveAs(Form("%s/plots%s/%s_prior.svg",outDir.Data(),(zBin!=0)?Form("%d",zBin):"",outName.Data()));
                }

    TH1D* hNormY;
    // weighting the matrix
        if (fDoWeighting) {
        std::cout << "==== weighting ==== " << std::endl;
                hNormY=(TH1D*)fMatrixProd->ProjectionY("hNormY");
                if (isPrior){
            std::cout << "=== using prior function ====" << std::endl;
                    if (! hNormY->Divide(fPriorFunction) ) { std::cout << "\"divide\" failed "; return; }
        }
                else{
            std::cout << "==== dividing ==== " << std::endl;
                    hNormY->Divide(priorhisto);
        }
                WeightMatrixY(fMatrixProd,hNormY,fdivide);
    }

    TH1D* hProjYeff=dynamic_cast<TH1D*>(fMatrixProd->ProjectionY("hProjYeff"));
    TH1D* hProjXeff=dynamic_cast<TH1D*>(fMatrixProd->ProjectionX("hProjXeff"));

    TH2D* Matrix = nullptr;
    TH1D* hProjXeffRebin = nullptr;
    TH1D* hProjYeffRebin = nullptr;
    TH1D* fRawRebin = nullptr;
    if (fObservable == Observable::kXsection){
        Matrix = Rebin2D("Matrix", fMatrixProd, fptbinsJetMeasN, fptbinsJetMeasA, fptbinsJetTrueN, fptbinsJetTrueA,0);
        hProjXeffRebin=dynamic_cast<TH1D*>(hProjXeff->Rebin(fptbinsJetMeasN, "hProjXeffRebin", fptbinsJetMeasA));
        hProjYeffRebin=dynamic_cast<TH1D*>(hProjYeff->Rebin(fptbinsJetTrueN, "hProjYeffRebin", fptbinsJetTrueA));
        fRawRebin=dynamic_cast<TH1D*>(fRawSpectrum->Rebin(fptbinsJetMeasN, "fRawRebin", fptbinsJetMeasA));
    }
    if (fObservable == Observable::kFragmentation){
        Matrix = Rebin2D("Matrix", fMatrixProd, fzbinsJetMeasN, fzbinsJetMeasA, fzbinsJetTrueN, fzbinsJetTrueAPrompt,0);
        hProjXeffRebin=dynamic_cast<TH1D*>(hProjXeff->Rebin(fzbinsJetMeasN, "hProjXeffRebin", fzbinsJetMeasA));
        hProjYeffRebin=dynamic_cast<TH1D*>(hProjYeff->Rebin(fzbinsJetTrueN, "hProjYeffRebin", fzbinsJetTrueAPrompt));
        fRawRebin=dynamic_cast<TH1D*>(fRawSpectrum->Rebin(fzbinsJetMeasN, "fRawRebin", fzbinsJetMeasA));
    }
*/

  TPaveText *AnaInfoPt[fzptJetMeasN];
  TPaveText *AnaInfoPtRatio[fzptJetMeasN];
  for (Int_t i = 0; i < fzptJetMeasN; i++)
  {
    std::cout << "legend " << i << std::endl;
    AnaInfoPt[i] = new TPaveText(0.6, 0.15, 0.9, 0.3, "brNDC");
    AnaInfoPt[i]->SetFillStyle(0);
    AnaInfoPt[i]->SetBorderSize(0);
    AnaInfoPt[i]->SetTextFont(42);
    AnaInfoPt[i]->SetTextSize(0.04f);
    AnaInfoPt[i]->SetTextAlign(11);
    AnaInfoPt[i]->AddText(Form("%d < p_{T,%s} < %d GeV/#it{c}", static_cast<Int_t>(fzptbinsDA[i][0]), fDmesonS.Data(), static_cast<Int_t>(fzptbinsDA[i][fzptbinsDN[i]])));
    AnaInfoPt[i]->AddText(Form("%d < p_{T,ch. jet} < %d GeV/#it{c}", static_cast<Int_t>(fzptJetMeasA[i]), static_cast<Int_t>(fzptJetMeasA[i + 1])));
    AnaInfoPtRatio[i] = new TPaveText(0.12, 0.3, 0.52, 0.5, "brNDC");
    AnaInfoPtRatio[i]->SetFillStyle(0);
    AnaInfoPtRatio[i]->SetBorderSize(0);
    AnaInfoPtRatio[i]->SetTextFont(42);
    AnaInfoPtRatio[i]->SetTextSize(0.045f);
    AnaInfoPtRatio[i]->SetTextAlign(11);
    AnaInfoPtRatio[i]->AddText(Form("%d < p_{T,%s} < %d GeV/#it{c}", static_cast<Int_t>(fzptbinsDA[i][0]), fDmesonS.Data(), static_cast<Int_t>(fzptbinsDA[i][fzptbinsDN[i]])));
    AnaInfoPtRatio[i]->AddText(Form("%d < p_{T,ch. jet} < %d GeV/#it{c}", static_cast<Int_t>(fzptJetMeasA[i]), static_cast<Int_t>(fzptJetMeasA[i + 1])));
  }

  //Kinematics MC
  TH2D *hKineRatioMC = dynamic_cast<TH2D *>(fMeasSpectrumKineNum->Clone("hKineRatioMC"));
  hKineRatioMC->Sumw2("B");
  hKineRatioMC->Divide(hKineRatioMC, fMeasSpectrumKineDen, 1, 1, "B");
  TH1D *hKineRatioProjMC[fzptJetMeasN];
  TCanvas *cKineRatioProjMC = new TCanvas("cKineRatioProjMC", "cKineRatioProjMC", 3200, 2400);
  cKineRatioProjMC->Divide(3, 2);
  TH1D *hunfoldedSpectrumKineEffMC[fzptJetMeasN];
  std::cout << "U" << std::endl;
  for (Int_t i = 1; i <= fzptJetMeasN; i++)
  {
    std::cout << i << " A" << std::endl;
    hKineRatioProjMC[i - 1] = hKineRatioMC->ProjectionX(Form("KineRatioprojMC%d", i), i, i);
    hKineRatioProjMC[i - 1]->SetTitle("Kinematic efficiency on missing MC");
    hKineRatioProjMC[i - 1]->GetYaxis()->SetMaxDigits(3);
    hKineRatioProjMC[i - 1]->GetYaxis()->SetTitle("#varepsilon_{MC}");
    hKineRatioProjMC[i - 1]->GetXaxis()->SetTitle("z_{#parallel}");
    hKineRatioProjMC[i - 1]->GetXaxis()->SetRangeUser(fzbinsJetMeasA[i - 1][0], fzbinsJetMeasA[i - 1][fzbinsJetMeasN[i - 1]]);
    hKineRatioProjMC[i - 1]->Sumw2();
    cKineRatioProjMC->cd(i);
    hKineRatioProjMC[i - 1]->Draw();
    AnaInfoPt[i - 1]->Draw("same");
    std::cout << i << " B" << std::endl;
    //hunfoldedSpectrumKineEffMC[i-1] = dynamic_cast<TH1D*>(UnfProjection[regBayes-1][i-1]->Clone(Form("hunfoldedSpectrumKineEffMC%d",i)));
    //hunfoldedSpectrumKineEffMC[i-1]->Multiply(hKineRatioProj[i-1]);
  }
  cKineRatioProjMC->SaveAs(Form("%s/plots/%s_KineDetMult.pdf", outDir.Data(), outName.Data()));
  cKineRatioProjMC->SaveAs(Form("%s/plots/%s_KineDetMult.png", outDir.Data(), outName.Data()));
  fRawSpectrum->Multiply(hKineRatioMC);
  TCanvas *te = new TCanvas("te", "te", 1600, 800);
  te->Divide(2, 1);
  te->cd(1);
  fMeasSpectrumKineNum->Draw("colz");
  te->cd(2);
  fMeasSpectrumKineDen->Draw("colz");

  /**************************************************
############# unfolding settings ##################
**************************************************/

  TH2D *fUnfoldedBayes[NTrials];
  TH2D *refolded[NTrials];
  TH2D *fPearsonCoeffs[NTrials]; //more per pT per Z
  TH2D *hRatioSpectrum[NTrials];
  TH2D *hRatio[NTrials];
  TH2D *fUnfoldedBayesClosure[NTrials];

  Int_t nCanvasX = TMath::Ceil(TMath::Sqrt((NTrials)));
  Int_t nCanvasY = TMath::Floor(TMath::Sqrt((NTrials + 0.5)));

  TCanvas *cUnfolded = new TCanvas("cUnfolded", "cUnfolded", 800 * nCanvasX, 600 * nCanvasY);
  cUnfolded->Divide(nCanvasX, nCanvasY);

  //------------ do unfolding NTrials times ------------
  for (Int_t ivar = 0; ivar < NTrials; ivar++)
  { //changes
    /***********************************
############# unfolding ##################
************************************/
    RooUnfoldBayes unfold(response, fRawSpectrum, ivar + 1);
    fUnfoldedBayes[ivar] = dynamic_cast<TH2D *>(unfold.Hreco());
    refolded[ivar] = dynamic_cast<TH2D *>(response->ApplyToTruth(fUnfoldedBayes[ivar]));
    RooUnfoldBayes unfoldClosure(responseClosure, fMeasSpectrumClosure, ivar + 1);
    fUnfoldedBayesClosure[ivar] = dynamic_cast<TH2D *>(unfoldClosure.Hreco());

    // ------------ Get Person coefficient ------------
    fPearsonCoeffs[ivar] = getPearsonCoeffs(unfold.Ereco(RooUnfold::kCovariance));
    fPearsonCoeffs[ivar]->SetName(Form("PearsonCoeffs%d", ivar + 1));

    fUnfoldedBayes[ivar]->GetXaxis()->SetTitle("z_{#parallel}");
    fUnfoldedBayes[ivar]->GetYaxis()->SetTitle("p_{T,jet}^{ch} (GeV/#it{c})");
    fUnfoldedBayes[ivar]->GetZaxis()->SetTitle("d^2N/dz_{#parallel}dp_{T,jet}^{ch}");
    fUnfoldedBayes[ivar]->SetTitle(Form("Unfolded Spectra, Reg=%d", ivar + 1));
    cUnfolded->cd(ivar + 1);
    cUnfolded->cd(ivar + 1)->SetLogz();
    fUnfoldedBayes[ivar]->Draw("colz TEXT");
  }
  cUnfolded->SaveAs(Form("%s/plots/%s_unfSpectra.pdf", outDir.Data(), outName.Data()));
  cUnfolded->SaveAs(Form("%s/plots/%s_unfSpectra.png", outDir.Data(), outName.Data()));

  TCanvas *cPearson = new TCanvas("cPearson", "cPearson", 800 * nCanvasX, 600 * nCanvasY);
  cPearson->Divide(nCanvasX, nCanvasY);
  for (Int_t ivar = 0; ivar < NTrials; ivar++)
  {
    cPearson->cd(ivar + 1);
    fPearsonCoeffs[ivar]->SetTitle(Form("k=%d", ivar + 1));
    fPearsonCoeffs[ivar]->SetMaximum(1);
    fPearsonCoeffs[ivar]->SetMinimum(-1);
    fPearsonCoeffs[ivar]->Draw("colz");
  }

  cPearson->SaveAs(Form("%s/plots/%s_Pearson.pdf", outDir.Data(), outName.Data()));
  cPearson->SaveAs(Form("%s/plots/%s_Pearson.png", outDir.Data(), outName.Data()));

  TCanvas *cRatio2 = new TCanvas("cRatio2", "cRatio2", 800 * nCanvasX, 600 * nCanvasY);
  cRatio2->Divide(nCanvasX, nCanvasY);
  TH2D *hBaseMeasure = dynamic_cast<TH2D *>(fRawSpectrum->Clone("hBaseSpectrum"));

  // =============== Refolded / Measured 2D
  for (Int_t ivar = 0; ivar < NTrials; ivar++)
  {
    hRatio[ivar] = dynamic_cast<TH2D *>(refolded[ivar]->Clone(Form("hRatio%d", ivar)));
    hRatio[ivar]->Divide(hBaseMeasure);
    hRatio[ivar]->SetTitle(Form("refolded/measured Reg=%d", ivar + 1));
    hRatio[ivar]->GetXaxis()->SetTitle("z_{#parallel}");
    hRatio[ivar]->GetYaxis()->SetTitle("p_{T,jet}^{ch} (GeV/#it{c})");
    hRatio[ivar]->GetZaxis()->SetTitle("d^2N/dz_{#parallel}dp_{T,jet}^{ch}");

    Double_t levels[] = {0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0, 1.1, 1.2, 1.3, 1.4, 1.5, 1.6, 1.7, 1.8, 1.9, 2.0, 1e10};
    hRatio[ivar]->SetContour((sizeof(levels) / sizeof(Double_t)), levels);
    hRatio[ivar]->GetZaxis()->SetRangeUser(0.6, 1.4);
    cRatio2->cd(ivar + 1);
    hRatio[ivar]->Draw("LEGO2Z");
    //hRatio[ivar]->Draw("SURF3");
    //hRatio[ivar]->Draw("colz");
  }

  cRatio2->SaveAs(Form("%s/plots/%s_foldedRatio.pdf", outDir.Data(), outName.Data()));
  cRatio2->SaveAs(Form("%s/plots/%s_foldedRatio.png", outDir.Data(), outName.Data()));

  // =============== Unfolded Refolded / Measured 2D
  TCanvas *cRatio = new TCanvas("cRatio", "cRatio", 800 * nCanvasX, 600 * nCanvasY);
  cRatio->Divide(nCanvasX, nCanvasY);
  TH2D *hBaseSpectrum = dynamic_cast<TH2D *>(fUnfoldedBayes[regBayes - 1]->Clone("hBaseSpectrum"));

  for (Int_t ivar = 0; ivar < NTrials; ivar++)
  {
    hRatioSpectrum[ivar] = dynamic_cast<TH2D *>(fUnfoldedBayes[ivar]->Clone(Form("hRatioSpectrum%d", ivar)));
    hRatioSpectrum[ivar]->Divide(hBaseSpectrum);
    hRatioSpectrum[ivar]->SetTitle(Form("Reg=%d/Reg=%d", ivar + 1, regBayes));
    hRatioSpectrum[ivar]->GetZaxis()->SetRangeUser(0.6, 1.4);
    Double_t levels[] = {0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0, 1.1, 1.2, 1.3, 1.4, 1.5, 1.6, 1.7, 1.8, 1.9, 2.0, 1e10};
    hRatioSpectrum[ivar]->SetContour((sizeof(levels) / sizeof(Double_t)), levels);
    cRatio->cd(ivar + 1);
    hRatioSpectrum[ivar]->Draw("LEGO2Z");
  }

  cRatio->SaveAs(Form("%s/plots/%s_unfRatio.pdf", outDir.Data(), outName.Data()));
  cRatio->SaveAs(Form("%s/plots/%s_unfRatio.png", outDir.Data(), outName.Data()));

  // =============== Projections in 1D

  TH1D *RawProjection[fzptJetMeasN];
  TH1D *UnfProjection[NTrials][fzptJetMeasN];
  TH1D *RefoldProjection[NTrials][fzptJetMeasN];
  TH1D *hUnfolded_Unc[NTrials][fzptJetMeasN];
  TH1D *TrueProjectionClosure[fzptJetMeasN];
  TH1D *UnfProjectionClosure[NTrials][fzptJetMeasN];
  TH1D *PriorProj[fzptJetTrueN];

  TCanvas *cUnfProj = new TCanvas("cUnfProj", "cUnfProj", 3200, 2400);
  cUnfProj->Divide(3, 2);
  TCanvas *cUnfProjErr = new TCanvas("cUnfProjErr", "cUnfProjErr", 3200, 2400);
  cUnfProjErr->Divide(3, 2);
  TCanvas *cUnfProjClosure = new TCanvas("cUnfProjClosure", "cUnfProjClosure", 3200, 2400);
  cUnfProjClosure->Divide(3, 2);
  TCanvas *cRefProjIter = new TCanvas("cRefProjIter", "cRefProjIter", 3200, 2400);
  cRefProjIter->Divide(3, 2);
  TCanvas *cUnfProjIter = new TCanvas("cUnfProjIter", "cUnfProjIter", 3200, 2400);
  cUnfProjIter->Divide(3, 2);

  TLegend *ls = new TLegend(0.15, 0.4, 0.4, 0.6);
  ls->SetBorderSize(0);
  TLegend *lsClosure = new TLegend(0.15, 0.6, 0.4, 0.85);
  lsClosure->SetBorderSize(0);
  TLegend *lsIter = new TLegend(0.12, 0.2, 0.22, 0.8);
  lsIter->SetBorderSize(0);

  TPaveText *AnaInfo = new TPaveText(0.1, 0.6, 0.7, 0.85, "brNDC");
  AnaInfo->SetFillStyle(0);
  AnaInfo->SetBorderSize(0);
  AnaInfo->SetTextFont(42);
  AnaInfo->SetTextSize(0.045f);
  AnaInfo->SetTextAlign(11);
  AnaInfo->AddText(Form("%s", fSystemS.Data()));
  AnaInfo->AddText(Form("Charged Jets, Anti-#it{k}_{T}, #it{R} = 0.%d", Rpar));
  AnaInfo->AddText("with D^{0} #rightarrow K^{-}#pi^{+} and charge conj.");
  AnaInfo->AddText(Form("|#it{#eta}_{jet}| < 0.%d", 9 - Rpar));
  TPaveText *AnaInfoRatio = new TPaveText(0.1, 0.5, 0.7, 0.85, "brNDC");
  AnaInfoRatio->SetFillStyle(0);
  AnaInfoRatio->SetBorderSize(0);
  AnaInfoRatio->SetTextFont(42);
  AnaInfoRatio->SetTextSize(0.045f);
  AnaInfoRatio->SetTextAlign(11);
  AnaInfoRatio->AddText(Form("%s", fSystemS.Data()));
  AnaInfoRatio->AddText(Form("Charged Jets, Anti-#it{k}_{T}, #it{R} = 0.%d", Rpar));
  AnaInfoRatio->AddText("with D^{0} #rightarrow K^{-}#pi^{+} and charge conj.");
  AnaInfoRatio->AddText(Form("|#it{#eta}_{jet}| < 0.%d", 9 - Rpar));
  AnaInfoRatio->AddText(Form("Spectrum:RM = %.2f:%2.f%%", nIntMeasC / nIntMeas * 100, 100 - nIntMeasC / nIntMeas * 100));

  TRatioPlot *hClosureRatio[fzptJetMeasN];
  TH1D *hUnfProjRatio[NTrials][fzptJetMeasN];
  TH1D *hRefProjRatio[NTrials][fzptJetMeasN];
  TRatioPlot *hUnfProjRatioDef[fzptJetMeasN];
  TRatioPlot *hRefProjRatioDef[fzptJetMeasN];

  //Kinematics
  TH2D *hKineRatio = dynamic_cast<TH2D *>(fTrueSpectrumKineNum->Clone("hKineRatio"));
  hKineRatio->Sumw2();
  hKineRatio->Divide(hKineRatio, fTrueSpectrumKineDen, 1, 1, "B");
  TH1D *hKineRatioProj[fzptJetMeasN];
  TCanvas *cKineRatioProj = new TCanvas("cKineRatioProj", "cKineRatioProj", 3200, 2400);
  cKineRatioProj->Divide(3, 2);

  TH1D *hunfoldedSpectrumKineEff[fzptJetMeasN];
  std::cout << "U" << std::endl;
  for (Int_t i = 1; i <= fzptJetMeasN; i++)
  {
    std::cout << i << " A" << std::endl;
    hKineRatioProj[i - 1] = hKineRatio->ProjectionX(Form("KineRatioproj%d", i), i, i);
    hKineRatioProj[i - 1]->SetTitle("Kinematic efficiency on missing DET");
    hKineRatioProj[i - 1]->GetYaxis()->SetMaxDigits(3);
    hKineRatioProj[i - 1]->GetYaxis()->SetTitle("#varepsilon_{DET}");
    hKineRatioProj[i - 1]->GetXaxis()->SetTitle("z_{#parallel}");
    hKineRatioProj[i - 1]->GetXaxis()->SetRangeUser(fzbinsJetTrueAPrompt[i - 1][0], fzbinsJetTrueAPrompt[i - 1][fzbinsJetTrueN[i - 1]]);
    hKineRatioProj[i - 1]->Sumw2();
    cKineRatioProj->cd(i);
    hKineRatioProj[i - 1]->Draw();
    AnaInfoPt[i - 1]->Draw("same");
    std::cout << i << " B" << std::endl;
    //hunfoldedSpectrumKineEff[i-1] = dynamic_cast<TH1D*>(UnfProjection[regBayes-1][i-1]->Clone(Form("hunfoldedSpectrumKineEff%d",i)));
    //hunfoldedSpectrumKineEff[i-1]->Divide(hKineRatioProj[i-1]);
  }

  std::cout << "here" << std::endl;
  for (Int_t ivar = 0; ivar < NTrials; ivar++)
  {
    for (Int_t i = 1; i <= fzptJetMeasN; i++)
    {
      for (Int_t j = 1; j <= fzptJetTrueN; j++)
      {
        if (ivar == 0 && i == 1)
        {
          PriorProj[j - 1] = fTrueSpectrum->ProjectionX(Form("Prior%d", j - 1), j, j);
          PriorProj[j - 1]->Scale(1. / PriorProj[j - 1]->Integral());
        }
        if (i != j)
          continue;
        //auro if((i == 1 && j==1)||(i == 2 && j==1)||(i == 3 && j==2)||(i == 4 && j==3)||(i == 5 && j==4)||(i == 6 && j==5) ){

        std::cout << i << " " << j << std::endl;
        Double_t binRaw = fRawSpectrum->GetYaxis()->GetBinCenter(i);
        Double_t binTrue = fUnfoldedBayes[ivar]->GetYaxis()->GetBinCenter(j);

        //if(TMath::Abs(binRaw-binTrue) > 1e-10 && i!=1) continue;
        //std::cout<<"raw "<<i<<" "<<binRaw<<" true "<<j<<" "<<binTrue<<" "<<TMath::Abs(binRaw-binTrue)<<std::endl;
        if (ivar == 0)
          RawProjection[i - 1] = fRawSpectrum->ProjectionX(Form("Rawproj%d", i), i, i);
        UnfProjection[ivar][i - 1] = fUnfoldedBayes[ivar]->ProjectionX(Form("Unfproj%d%d", ivar, i), j, j);
        UnfProjection[ivar][i - 1]->Divide(hKineRatioProj[i - 1]);
        RefoldProjection[ivar][i - 1] = refolded[ivar]->ProjectionX(Form("Refoldedproj%d%d", ivar, i), i, i);
        UnfProjectionClosure[ivar][i - 1] = fUnfoldedBayesClosure[ivar]->ProjectionX(Form("UnfprojClosure%d%d", ivar, i), j, j);
        if (ivar == 0)
          TrueProjectionClosure[i - 1] = fTrueSpectrumClosure->ProjectionX(Form("TrueprojCloseure%d", i), j, j);
        //  }//add
      }

      if (!UnfProjection[ivar][i - 1])
        std::cout << "proj no exist" << std::endl;
      hUnfolded_Unc[ivar][i - 1] = dynamic_cast<TH1D *>(UnfProjection[ivar][i - 1]->Clone("hUnfolded_Unc"));

      hUnfolded_Unc[ivar][i - 1]->GetYaxis()->SetTitle("Rel. unc.");
      hUnfolded_Unc[ivar][i - 1]->SetTitle(Form("Unfolded spectra, Bayes reg=%d", regBayes));
      hUnfolded_Unc[ivar][i - 1]->SetLineColor(kGreen + 1);
      hUnfolded_Unc[ivar][i - 1]->SetMarkerColor(kGreen + 1);

      for (int j = 1; j <= UnfProjection[ivar][i - 1]->GetNbinsX(); j++)
      {

        double err;

        if (UnfProjection[ivar][i - 1]->GetBinContent(j) > 0)
          err = UnfProjection[ivar][i - 1]->GetBinError(j) / UnfProjection[ivar][i - 1]->GetBinContent(j);
        else
          err = 0;

        hUnfolded_Unc[ivar][i - 1]->SetBinContent(j, err);
        hUnfolded_Unc[ivar][i - 1]->SetBinError(j, 0);
      }
      //hSpectraS[i-1] = new THStack(Form("hSpectraS%d",i-1),Form("Unfolded spectra, Bayes reg=%d",regBayes));

      if (i == 1 && ivar == regBayes - 1)
        ls->AddEntry(RawProjection[0], "Measured", "p");
      if (i == 1 && ivar == regBayes - 1)
        ls->AddEntry(UnfProjection[regBayes - 1][0], Form("Unfolded, Bayes reg=%d", regBayes), "p");
      if (i == 1 && ivar == regBayes - 1)
        ls->AddEntry(RefoldProjection[regBayes - 1][0], "Refolded", "l");

      hUnfolded_Unc[ivar][i - 1]->SetTitle("");
      hUnfolded_Unc[ivar][i - 1]->SetMaximum(hUnfolded_Unc[ivar][i - 1]->GetMaximum() * 1.2);
      hUnfolded_Unc[ivar][i - 1]->SetMinimum(0);
      hUnfolded_Unc[ivar][i - 1]->GetXaxis()->SetRangeUser(fzbinsJetTrueAPrompt[i - 1][0], fzbinsJetTrueAPrompt[i - 1][fzbinsJetTrueN[i - 1]]);
      std::cout << "C" << std::endl;

      if (ivar == 0)
      {
        RawProjection[i - 1]->SetLineColor(kBlue + 1);
        RawProjection[i - 1]->SetMarkerColor(kBlue + 1);
        RawProjection[i - 1]->SetMarkerStyle(22);
        RawProjection[i - 1]->GetXaxis()->SetTitle("z_{#parallel}");
        RawProjection[i - 1]->GetYaxis()->SetTitle("dN/dz_{#parallel}");
        RawProjection[i - 1]->SetTitle(Form("Unfolded spectra, Bayes reg=%d", regBayes));
        RawProjection[i - 1]->GetXaxis()->SetRangeUser(fzbinsJetTrueAPrompt[i - 1][0], fzbinsJetTrueAPrompt[i - 1][fzbinsJetTrueN[i - 1]]);
      }
      //   RefoldProjection[ivar][i-1]->SetLineColor(colortable2[ivar]);
      //   RefoldProjection[ivar][i-1]->SetMarkerColor(colortable2[ivar]);

      UnfProjection[ivar][i - 1]->SetLineColor(kRed + 1);
      RefoldProjection[ivar][i - 1]->SetLineColor(kGreen + 1);
      UnfProjection[ivar][i - 1]->SetMarkerColor(kRed + 1);
      RefoldProjection[ivar][i - 1]->SetMarkerColor(kGreen + 1);
      UnfProjection[ivar][i - 1]->SetMarkerStyle(22);
      RefoldProjection[ivar][i - 1]->SetMarkerStyle(23);
      UnfProjection[ivar][i - 1]->GetXaxis()->SetTitle("z_{#parallel}");
      UnfProjection[ivar][i - 1]->GetYaxis()->SetTitle("dN/dz_{#parallel}");
      UnfProjection[ivar][i - 1]->SetTitle(Form("Unfolded spectra, Bayes reg=%d", ivar + 1));
      RefoldProjection[ivar][i - 1]->GetXaxis()->SetTitle("z_{#parallel}");
      RefoldProjection[ivar][i - 1]->GetYaxis()->SetTitle("dN/dz_{#parallel}");
      RefoldProjection[ivar][i - 1]->SetTitle(Form("Unfolded spectra, Bayes reg=%d", ivar + 1));
      UnfProjection[ivar][i - 1]->GetXaxis()->SetRangeUser(fzbinsJetTrueAPrompt[i - 1][0], fzbinsJetTrueAPrompt[i - 1][fzbinsJetTrueN[i - 1]]);
      RefoldProjection[ivar][i - 1]->GetXaxis()->SetRangeUser(fzbinsJetTrueAPrompt[i - 1][0], fzbinsJetTrueAPrompt[i - 1][fzbinsJetTrueN[i - 1]]);

      std::cout << "D" << std::endl;

      //
      //cRefProjIterDummy->cd(ivar+1);
      //if(ivar>0)hRefProjRatio[ivar][i-1]->Draw();
      //cRefProjIter->cd(i);
      hRefProjRatio[ivar][i - 1] = dynamic_cast<TH1D *>(RefoldProjection[ivar][i - 1]->Clone(Form("refratio%d%d", ivar, i)));
      hRefProjRatio[ivar][i - 1]->Divide(RawProjection[i - 1]);
      hRefProjRatio[ivar][i - 1]->SetMarkerColor(colortable2[ivar]);
      hRefProjRatio[ivar][i - 1]->SetLineColor(colortable2[ivar]);
      hRefProjRatio[ivar][i - 1]->SetMarkerStyle(22 + ivar);
      hRefProjRatio[ivar][i - 1]->GetXaxis()->SetTitle("z_{#parallel}");
      hRefProjRatio[ivar][i - 1]->GetYaxis()->SetTitle("regX/measured");
      hRefProjRatio[ivar][i - 1]->SetTitle("Refolded spectra, Bayes");
      hRefProjRatio[ivar][i - 1]->GetXaxis()->SetRangeUser(fzbinsJetTrueAPrompt[i - 1][0], fzbinsJetTrueAPrompt[i - 1][fzbinsJetTrueN[i - 1]]);
      if (i == 1)
        hRefProjRatio[ivar][i - 1]->GetXaxis()->SetRangeUser(fzbinsJetTrueAPrompt[i - 1][0] - 0.2, fzbinsJetTrueAPrompt[i - 1][fzbinsJetTrueN[i - 1]]);
    }
  }

  std::cout << "here2" << std::endl;
  for (Int_t ivar = 0; ivar < NTrials; ivar++)
  {
    for (Int_t i = 1; i <= fzptJetMeasN; i++)
    {

      hUnfProjRatio[ivar][i - 1] = dynamic_cast<TH1D *>(UnfProjection[ivar][i - 1]->Clone(Form("unfratio%d%d", ivar, i)));
      hUnfProjRatio[ivar][i - 1]->Divide(UnfProjection[regBayes - 1][i - 1]);
      hUnfProjRatio[ivar][i - 1]->SetMarkerColor(colortable2[ivar]);
      hUnfProjRatio[ivar][i - 1]->SetLineColor(colortable2[ivar]);
      hUnfProjRatio[ivar][i - 1]->SetMarkerStyle(22 + ivar);
      hUnfProjRatio[ivar][i - 1]->GetXaxis()->SetTitle("z_{#parallel}");
      hUnfProjRatio[ivar][i - 1]->GetYaxis()->SetTitle(Form("regX/reg%d", regBayes));
      hUnfProjRatio[ivar][i - 1]->SetTitle("Unfolded spectra, Bayes");
      hUnfProjRatio[ivar][i - 1]->GetXaxis()->SetRangeUser(fzbinsJetTrueAPrompt[i - 1][0], fzbinsJetTrueAPrompt[i - 1][fzbinsJetTrueN[i - 1]]);
      if (i == 1)
        hUnfProjRatio[ivar][i - 1]->GetXaxis()->SetRangeUser(fzbinsJetTrueAPrompt[i - 1][0] - 0.2, fzbinsJetTrueAPrompt[i - 1][fzbinsJetTrueN[i - 1]]);
      if (i == 1)
        lsIter->AddEntry(hUnfProjRatio[ivar][0], Form("reg=%d", ivar + 1), "p");

      if (ivar == regBayes - 1)
        hUnfProjRatioDef[i - 1] = new TRatioPlot(UnfProjection[regBayes - 1][i - 1], RawProjection[i - 1], "divsym");
      if (ivar == regBayes - 1)
        hRefProjRatioDef[i - 1] = new TRatioPlot(RefoldProjection[regBayes - 1][i - 1], RawProjection[i - 1], "divsym");
      if (ivar == regBayes - 1)
        hunfoldedSpectrumKineEff[i - 1] = dynamic_cast<TH1D *>(UnfProjection[regBayes - 1][i - 1]->Clone(Form("hunfoldedSpectrumKineEff%d", i)));
    }
  }
  std::cout << "here3" << std::endl;
  for (Int_t ivar = 0; ivar < NTrials; ivar++)
  {
    for (Int_t i = 1; i <= fzptJetMeasN; i++)
    {
      cRefProjIter->cd(i);
      if (ivar == 0)
      {
        hRefProjRatio[ivar][i - 1]->GetYaxis()->SetRangeUser(0.7, 1.3);
      }
      ivar == 0 ? hRefProjRatio[ivar][i - 1]->Draw() : hRefProjRatio[ivar][i - 1]->Draw("same");
      if (ivar == NTrials - 1 && i == 1)
        lsIter->Draw("same");
      if (ivar == 0)
        AnaInfoPt[i - 1]->Draw("same");

      cUnfProjIter->cd(i);
      if (ivar == 0)
      {
        hUnfProjRatio[ivar][i - 1]->GetYaxis()->SetRangeUser(0.7, 1.3);
      }
      ivar == 0 ? hUnfProjRatio[ivar][i - 1]->Draw() : hUnfProjRatio[ivar][i - 1]->Draw("same");
      if (ivar == NTrials - 1 && i == 1)
        lsIter->Draw("same");
      if (ivar == 0)
        AnaInfoPt[i - 1]->Draw("same");
      /*if(ivar==0){
            hRefProjRatio[ivar][i-1]->Draw();
            hRefProjRatio[ivar][i-1]->GetLowerRefGraph()->GetYaxis()->SetRangeUser(0.7,1.3);
        }
        else{
        hRefProjRatio[0][i-1]->GetUpperPad()->cd();
        dynamic_cast<TH1D*>(hRefProjRatio[ivar][i-1]->GetUpperRefObject())->Draw("psame");
        hRefProjRatio[0][i-1]->GetLowerPad()->cd();
        hRefProjRatio[ivar][i-1]->GetLowerRefGraph()->Draw("psame");
        }*/
    }
  }

  cRefProjIter->SaveAs(Form("%s/plots/%s_RefoldingIter.png", outDir.Data(), outName.Data()));
  cUnfProjIter->SaveAs(Form("%s/plots/%s_UnfoldingIter.png", outDir.Data(), outName.Data()));
  cRefProjIter->SaveAs(Form("%s/plots/%s_RefoldingIter.pdf", outDir.Data(), outName.Data()));
  cUnfProjIter->SaveAs(Form("%s/plots/%s_UnfoldingIter.pdf", outDir.Data(), outName.Data()));

  for (Int_t i = 1; i <= fzptJetMeasN; i++)
  {
    cUnfProj->cd(i);
    /*hUnfProjRatioDef[i-1]->Draw();
        hRefProjRatioDef[i-1]->Draw();
        hRefProjRatioDef[i-1]->GetUpperPad()->cd();
        dynamic_cast<TH1D*>(hUnfProjRatioDef[i-1]->GetUpperRefObject())->Draw("same");
        if(i==1)ls->Draw("same");
        if(i==1)AnaInfo->Draw("same");
        AnaInfoPt[i-1]->Draw("same");
        hRefProjRatioDef[i-1]->GetLowerRefGraph()->GetYaxis()->SetRangeUser(0.6,1.4);
        hRefProjRatioDef[i-1]->GetLowerRefGraph()->GetYaxis()->SetTitle("X/measured");
        hRefProjRatioDef[i-1]->GetLowerPad()->cd();
        hUnfProjRatioDef[i-1]->GetLowerRefGraph()->Draw("psame");*/

    hUnfProjRatioDef[i - 1]->Draw();
    hRefProjRatioDef[i - 1]->Draw();

    hRefProjRatioDef[i - 1]->GetUpperPad()->cd();
    dynamic_cast<TH1D *>(hUnfProjRatioDef[i - 1]->GetUpperRefObject())->Draw("same");
    if (i == 1)
      ls->Draw("same");
    if (i == 1)
      AnaInfo->Draw("same");
    AnaInfoPt[i - 1]->Draw("same");
    hRefProjRatioDef[i - 1]->GetUpperRefYaxis()->SetRangeUser(0, dynamic_cast<TH1D *>(hRefProjRatioDef[i - 1]->GetUpperRefObject())->GetMaximum() * 1.2);
    hRefProjRatioDef[i - 1]->GetLowerRefGraph()->GetYaxis()->SetRangeUser(0.6, 1.4);
    hRefProjRatioDef[i - 1]->GetLowerRefGraph()->GetYaxis()->SetTitle("X/measured");
    hRefProjRatioDef[i - 1]->GetLowerPad()->cd();
    hUnfProjRatioDef[i - 1]->GetLowerRefGraph()->Draw("psame");
    //hSpectraS[i-1]->Add(RawProjection[i-1]);
    //hSpectraS[i-1]->Add(UnfProjection[i-1]);
    //hSpectraS[i-1]->Add(RefoldProjection[i-1]);
    //hSpectraS[i-1]->Draw();
    //hSpectraS[i-1]->GetXaxis()->SetTitle("z_{#parallel}");
    //hSpectraS[i-1]->GetYaxis()->SetTitle("dN/dz_{#parallel}");

    cUnfProjErr->cd(i);
    hUnfolded_Unc[regBayes - 1][i - 1]->Draw();
    AnaInfoPt[i - 1]->Draw("same");

    if (i == 1)
      lsClosure->AddEntry(UnfProjectionClosure[regBayes - 1][0], "Unfolded", "lp");
    if (i == 1)
      lsClosure->AddEntry(TrueProjectionClosure[0], "True", "lp");

    cUnfProjClosure->cd(i);
    cUnfProjClosure->SetTicks(0, 1);
    UnfProjectionClosure[regBayes - 1][i - 1]->SetLineColor(kRed + 1);
    UnfProjectionClosure[regBayes - 1][i - 1]->GetXaxis()->SetTitle("z_{#parallel}");
    UnfProjectionClosure[regBayes - 1][i - 1]->GetYaxis()->SetTitle("dN/dz_{#parallel}");
    UnfProjectionClosure[regBayes - 1][i - 1]->SetTitle(Form("Closure test, Bayes reg=%d", regBayes));
    UnfProjectionClosure[regBayes - 1][i - 1]->SetMarkerColor(kRed + 1);
    TrueProjectionClosure[i - 1]->SetLineColor(kGreen + 1);
    TrueProjectionClosure[i - 1]->SetMarkerColor(kGreen + 1);
    TrueProjectionClosure[i - 1]->SetMarkerStyle(22);
    UnfProjectionClosure[regBayes - 1][i - 1]->SetMarkerStyle(21);
    hClosureRatio[i - 1] = new TRatioPlot(UnfProjectionClosure[regBayes - 1][i - 1], TrueProjectionClosure[i - 1], "divsym");
    hClosureRatio[i - 1]->SetH1DrawOpt("E");
    hClosureRatio[i - 1]->SetH2DrawOpt("E");
    hClosureRatio[i - 1]->Draw();
    hClosureRatio[i - 1]->GetLowerRefGraph()->GetYaxis()->SetRangeUser(0.7, 1.3);
    hClosureRatio[i - 1]->GetLowerRefGraph()->GetYaxis()->SetTitle("unfolded/true");
    hClosureRatio[i - 1]->GetUpperPad()->cd();
    if (i == 1)
      AnaInfoRatio->Draw("same");
    AnaInfoPtRatio[i - 1]->Draw("same");
    if (i == 2)
      lsClosure->Draw("same");
    //UnfProjectionClosure[i-1]->Draw();
    //TrueProjectionClosure[i-1]->Draw("same");
  }

  cUnfProj->SaveAs(Form("%s/plots/%s_projections.png", outDir.Data(), outName.Data()));
  cUnfProjErr->SaveAs(Form("%s/plots/%s_projectionsUnc.png", outDir.Data(), outName.Data()));
  cUnfProjClosure->SaveAs(Form("%s/plots/%s_projectionsClosure.png", outDir.Data(), outName.Data()));
  cUnfProj->SaveAs(Form("%s/plots/%s_projections.pdf", outDir.Data(), outName.Data()));
  cUnfProjErr->SaveAs(Form("%s/plots/%s_projectionsUnc.pdf", outDir.Data(), outName.Data()));
  cUnfProjClosure->SaveAs(Form("%s/plots/%s_projectionsClosure.pdf", outDir.Data(), outName.Data()));

  cKineRatioProj->SaveAs(Form("%s/plots/%s_KineTrueDiv.pdf", outDir.Data(), outName.Data()));
  cKineRatioProj->SaveAs(Form("%s/plots/%s_KineTrueDiv.png", outDir.Data(), outName.Data()));

  std::cout << "UU" << std::endl;

  TFile *outSpectra = new TFile(Form("%s/%s_unfoldedJetSpectrum.root", outDir.Data(), outName.Data()), "recreate");
  fRawSpectrum->Write();
  fTrueSpectrum->Write();
  fMeasSpectrum->Write();
  std::cout << "S" << std::endl;
  for (Int_t i = 0; i < fzptJetTrueN; i++)
  {
    for (Int_t j = 0; j < fzptJetMeasN; j++)
    {
      if (resSlices[i][j])
        resSlices[i][j]->Write();
    }
    PriorProj[i]->Write();
  }
  std::cout << "SS" << std::endl;
  for (Int_t i = 1; i <= fzptJetMeasN; i++)
  {
    std::cout << i << " F" << std::endl;
    RawProjection[i - 1]->Write();
    std::cout << i << " F2" << std::endl;
    UnfProjection[regBayes - 1][i - 1]->Write(Form("unfoldedSpectrum%d", i));
    std::cout << i << " F3" << std::endl;
    RefoldProjection[regBayes - 1][i - 1]->Write(Form("foldedSpectrum%d", i));
    std::cout << i << " F4" << std::endl;
    hKineRatioProj[i - 1]->Write(Form("KinematicsEff%d", i));
    std::cout << i << " F5" << std::endl;
    hunfoldedSpectrumKineEff[i - 1]->Write(Form("unfoldedSpectrumKineEff%d", i));
    hKineRatioProjMC[i - 1]->Write(Form("KinematicsEffMC%d", i));
  }
  std::cout << "SSS" << std::endl;
  outSpectra->Close();
}

std::tuple<RooUnfoldResponse *, RooUnfoldResponse *> LoadDetectorMatrix(TString MCfile, TString out, Double_t RMfraction, TH2D *prior)
{
  // Read the TTree reader init
  //TFile *fTreeSparse = new TFile("/home/kvapil/work/analysis/pp_run2/D0jet/BaseCuts/Default_AnalysisResults_Run2.root/RM.root","READ");
  TFile *fTreeSparse = new TFile(MCfile, "READ");
  if (!fTreeSparse)
    std::cout << "MC FILE not found" << std::endl;
  Double_t jmatch[13];
  Double_t bincontent;
  TTree *tree_ = dynamic_cast<TTree *>(fTreeSparse->Get("ResponseMatrixSum_tree"));
  if (!tree_)
  {
    TString tmp(MCfile);
    tmp.Remove(MCfile.Length() - 5, 5).Append("TTree.root");
    std::cout << "MC file do not contain TTree, trying to open TTree sparse in default path!" << std::endl;
    std::cout << "opening: " << tmp << std::endl;
    fTreeSparse = new TFile(tmp, "READ");
    if (!(fTreeSparse->IsOpen()))
    {
      std::cout << "TTree file not found, converting MC THnSparse to TTree now!" << std::endl;
      SparseToTree(MCfile);
      fTreeSparse = new TFile(tmp, "READ");
      tree_ = dynamic_cast<TTree *>(fTreeSparse->Get("ResponseMatrixSum_tree"));
    }
    else
      tree_ = dynamic_cast<TTree *>(fTreeSparse->Get("ResponseMatrixSum_tree"));
  }

  tree_->SetBranchAddress("coord", &jmatch);
  tree_->SetBranchAddress("bincontent", &bincontent);

  //ROOunfold init
  std::cout << "RooUnfold Init" << std::endl;
  TH2D *hTrainTrue = new TH2D("traintrue", "Binning Truth", fzbinsJetTrueN[fzptJetMeasN - 1], fzbinsJetTrueAPrompt[fzptJetMeasN - 1], fzptJetTrueN, fzptJetTrueA);

  for (Int_t i = 0; i < fzbinsJetTrueN[fzptJetMeasN - 1]; i++)
  {
    std::cout << "binning   " << i << " " << fzbinsJetTrueAPrompt[fzptJetMeasN - 1][i] << std::endl;
  }
  TH2D *hTrain = new TH2D("train", "Binning Measured", fzbinsJetMeasN[fzptJetMeasN - 1], fzbinsJetMeasA[fzptJetMeasN - 1], fzptJetMeasN, fzptJetMeasA);
  TH2D *hTrainTrueClosure = new TH2D("traintrueclosure", "Binning Truth Closure", fzbinsJetTrueN[fzptJetMeasN - 1], fzbinsJetTrueAPrompt[fzptJetMeasN - 1], fzptJetTrueN, fzptJetTrueA);
  TH2D *hTrainClosure = new TH2D("trainclosure", "Binning Measured Closure", fzbinsJetMeasN[fzptJetMeasN - 1], fzbinsJetMeasA[fzptJetMeasN - 1], fzptJetMeasN, fzptJetMeasA);
  fTrueSpectrumKineNum = new TH2D("fTrueSpectrumKineNum", "True Spectrum Kine Num", fzbinsJetTrueN[fzptJetMeasN - 1], fzbinsJetTrueAPrompt[fzptJetMeasN - 1], fzptJetMeasN, fzptJetMeasA);
  fTrueSpectrumKineDen = new TH2D("fTrueSpectrumKineDen", "True Spectrum Kine Den", fzbinsJetTrueN[fzptJetMeasN - 1], fzbinsJetTrueAPrompt[fzptJetMeasN - 1], fzptJetMeasN, fzptJetMeasA);

  fMeasSpectrumKineNum = new TH2D("fMeasSpectrumKineNum", "Meas Spectrum Kine Num MC", fzbinsJetMeasN[fzptJetMeasN - 1], fzbinsJetMeasA[fzptJetMeasN - 1], fzptJetMeasN, fzptJetMeasA);
  fMeasSpectrumKineDen = new TH2D("fMeasSpectrumKineDen", "Meas Spectrum Kine Den MC", fzbinsJetMeasN[fzptJetMeasN - 1], fzbinsJetMeasA[fzptJetMeasN - 1], fzptJetMeasN, fzptJetMeasA);

  std::cout << "Response Setup" << std::endl;
  RooUnfoldResponse *response = new RooUnfoldResponse();
  RooUnfoldResponse *responseClosure = new RooUnfoldResponse();
  response->Setup(hTrain, hTrainTrue);
  responseClosure->Setup(hTrainClosure, hTrainTrueClosure);

  std::cout << "Looping over TTree to fill response matrix" << std::endl;
  TRandom3 *random = new TRandom3(0);
  //TF2 *pri = new TF2("pri","1-4*(0.0007*(70-x)+0.05*y)",0,70,0,1);
  //Int_t display = 0;
  for (int i = 0; i < tree_->GetEntries(); i++)
  {
    tree_->GetEntry(i);
    Bool_t cutOK = false;
    Bool_t cutKineOK = false;
    Bool_t cutKineMCOK = false;
    if (jmatch[0] >= 0 && jmatch[1] >= 0 && -(0.9 - fRpar) <= jmatch[4] && jmatch[4] <= 0.9 - fRpar && -(0.9 - fRpar) <= jmatch[9] && jmatch[9] <= 0.9 - fRpar)
    { //found on reco also + eta true and eta reco cut
      //DpT cuts
      for (Int_t z = 0; z < fzptJetMeasN; z++)
      {
        if (fzptJetMeasA[z] <= jmatch[1] && jmatch[1] <= fzptJetMeasA[z + 1])
        { //jet det
          if (fzptbinsDA[z][0] <= jmatch[2] && jmatch[2] <= fzptbinsDA[z][fzptbinsDN[z]])
          { //D det

            if (jmatch[0] >= fzbinsJetMeasA[z][0])
            {
              //if(fzptbinsDA[z][0] <= jmatch[7] && jmatch[7] <= fzptbinsDA[z][fzptbinsDN[z]]) //D mc
              cutOK = true;
            }
          }
        }
      }
      for (Int_t z = 0; z < fzptJetMeasN; z++)
      {
        if (fzptJetMeasA[z] <= jmatch[6] && jmatch[6] <= fzptJetMeasA[z + 1])
        { //jet mc
          if (fzptbinsDA[z][0] <= jmatch[2] && jmatch[2] <= fzptbinsDA[z][fzptbinsDN[z]])
          { //D det
            cutKineOK = true;
          }
        }
      }
      for (Int_t z = 0; z < fzptJetMeasN; z++)
      {
        if (fzptJetMeasA[z] <= jmatch[1] && jmatch[1] <= fzptJetMeasA[z + 1])
        { //jet det
          if (fzptbinsDA[z][0] <= jmatch[7] && jmatch[7] <= fzptbinsDA[z][fzptbinsDN[z]])
          { //D mc
            cutKineMCOK = true;
          }
        }
      }
      if (cutOK == true)
      {
        //fot z_true shift values z=1 into previous bin
        Double_t zShiftTrue = -1;
        if (jmatch[5] > 1.0)
          zShiftTrue = jmatch[5] - 0.02;
        else
          zShiftTrue = jmatch[5];
        //get prior weight
        Double_t priorWeight = 1;
        //Double_t prival = pri->Eval(jmatch[6],zShiftTrue);
        if (prior)
        {
          Int_t priorbin = prior->FindBin(zShiftTrue, jmatch[6]);
          priorWeight = prior->GetBinContent(priorbin);
        }
        //std::cout<<zShiftTrue<<" "<<jmatch[6]<<" "<<priorWeight<<std::endl;
        //fill response matrix and reco+true plots
        /*       if((zShiftTrue>=0 && 15 <= jmatch[6] &&jmatch[6] <= 50)||
                  (zShiftTrue>=0.2 && 10 <= jmatch[6] &&jmatch[6] <= 15)||
                    (zShiftTrue>=0.2 && 7 <= jmatch[6] &&jmatch[6] <= 10)||
                    (zShiftTrue>=0.2 && 5 <= jmatch[6] &&jmatch[6] <= 7)||
                    (zShiftTrue>=0.4 && 0 <= jmatch[6] &&jmatch[6] <= 5)

                    ){*/
        hTrainTrue->Fill(zShiftTrue, jmatch[6], bincontent * priorWeight);
        hTrain->Fill(jmatch[0], jmatch[1], bincontent * priorWeight);
        response->Fill(jmatch[0], jmatch[1], zShiftTrue, jmatch[6], bincontent * priorWeight);
        //std::cout<<display<<" "<<jmatch[1]<<" "<<jmatch[2]<<" "<<jmatch[0]<<" "<<jmatch[6]<<" "<<jmatch[7]<<" "<<jmatch[5]<<" "<<bincontent<<std::endl;
        //std::cout<<display<<std::endl;
        //display++;
        //   }
        //with a random seed divide MC between RM and spectra
        Double_t RMw = 0;
        Double_t SPw = 0;
        for (Int_t iw = 0; iw < bincontent; iw++)
        {
          if (random->Uniform(1) <= RMfraction)
            RMw++;
          else
            SPw++;
        }
        if (RMw + SPw != bincontent)
          std::cout << "WARNING CLOSURE WEIGHT VIOLATION!!! " << RMw + SPw << "!=" << bincontent << std::endl;
        //fill response matrix and reco+true plots for closure test
        responseClosure->Fill(jmatch[0], jmatch[1], zShiftTrue, jmatch[6], RMw * priorWeight);
        //std::cout<<i<<" "<<jmatch[1]<<" "<<jmatch[2]<<" "<<jmatch[0]<<" "<<jmatch[6]<<" "<<jmatch[7]<<" "<<jmatch[5]<<std::endl;
        hTrainTrueClosure->Fill(zShiftTrue, jmatch[6], SPw * priorWeight);
        hTrainClosure->Fill(jmatch[0], jmatch[1], SPw * priorWeight);
        cutOK = false;
      }
      if (cutKineOK == true)
      {
        //for z_true shift values z=1 into previous bin
        Double_t zShiftTrue = -1;
        if (jmatch[5] > 1.0)
          zShiftTrue = jmatch[5] - 0.02;
        else
          zShiftTrue = jmatch[5];

        //if(2 <= jmatch[1] && jmatch[1] <= 50){
        Double_t zShiftDet = -1;
        if (jmatch[0] > 1.0)
          zShiftDet = jmatch[0] - 0.02;
        else
          zShiftDet = jmatch[0];
        for (Int_t z = 0; z < fzptJetMeasN; z++)
        {
          if (fzptJetMeasA[z] <= jmatch[6] && jmatch[6] <= fzptJetMeasA[z + 1])
          { //jet mc
            if (fzbinsJetMeasA[z][0] <= zShiftDet && zShiftDet <= fzbinsJetMeasA[z][fzbinsJetMeasN[z]] && 2 <= jmatch[1] && jmatch[1] <= 50)
            { //z det
              fTrueSpectrumKineNum->Fill(zShiftTrue, jmatch[6]);
            }
          }
        }
        //  if(2 <= jmatch[1] && jmatch[1] <= 50){
        //std::cout<<fzptJetMeasA[0]<<" "<<jmatch[1]<<" "<<fzptJetMeasA[fzptJetMeasN]<<" "<<zShiftTrue<<" "<<jmatch[6]<<std::endl;
        // fTrueSpectrumKineNum->Fill(zShiftTrue,jmatch[6]);
        //}
        fTrueSpectrumKineDen->Fill(zShiftTrue, jmatch[6]);
        cutKineOK = false;
      }
      if (cutKineMCOK == true)
      {
        //for z_true shift values z=1 into previous bin
        Double_t zShiftTrue = -1;
        if (jmatch[5] > 1.0)
          zShiftTrue = jmatch[5] - 0.02;
        else
          zShiftTrue = jmatch[5];

        if (0 <= jmatch[6] && jmatch[6] <= 100 && 0.2 <= zShiftTrue)
        {
          //std::cout<<fzptJetMeasA[0]<<" "<<jmatch[1]<<" "<<fzptJetMeasA[fzptJetMeasN]<<" "<<zShiftTrue<<" "<<jmatch[6]<<std::endl;
          fMeasSpectrumKineNum->Fill(jmatch[0], jmatch[1]);
        }
        //   else{
        //       std::cout<<"IAM OUT"<<std::endl;
        //   }
        /*  Double_t zShiftDet = -1;
              if(jmatch[0]>1.0) zShiftDet = jmatch[0] - 0.02;
              else zShiftDet = jmatch[0];
              for(Int_t z = 0;z < fzptJetMeasN;z++){
                  if(fzptJetMeasA[z] <= jmatch[1] && jmatch[1] <= fzptJetMeasA[z+1]){ //jet mc
                      if(fzbinsJetMeasA[z][0] <= zShiftTrue && zShiftTrue <= fzbinsJetMeasA[z][fzbinsJetMeasN[z]]){ //D det
                         fMeasSpectrumKineNum->Fill(zShiftDet,jmatch[1]);
                      }
                  }
              }*/
        fMeasSpectrumKineDen->Fill(jmatch[0], jmatch[1]);
        cutKineMCOK = false;
      }
    }
  }

  TCanvas *c = new TCanvas("c", "Unfolding2D_measured_true", 1600, 800);
  c->Divide(2, 1);
  c->cd(1);
  c->cd(1)->SetLogz();
  response->Hmeasured()->Draw("colz");
  c->cd(2);
  c->cd(2)->SetLogz();
  response->Htruth()->Draw("colz");
  c->SaveAs(out + "/Unfolding2D_Response_MeasuredTrue.png");
  c->SaveAs(out + "/Unfolding2D_Response_MeasuredTrue.pdf");
  TCanvas *cClosure = new TCanvas("cClosure", "Unfolding2D_measured_true_Closure", 1600, 800);
  cClosure->Divide(2, 1);
  cClosure->cd(1);
  cClosure->cd(1)->SetLogz();
  responseClosure->Hmeasured()->Draw("colz");
  cClosure->cd(2);
  cClosure->cd(2)->SetLogz();
  responseClosure->Htruth()->Draw("colz");
  cClosure->SaveAs(out + "/Unfolding2D_Response_MeasuredTrue_Closure.png");
  cClosure->SaveAs(out + "/Unfolding2D_Response_MeasuredTrue_Closure.pdf");
  fTrueSpectrum = dynamic_cast<TH2D *>(hTrainTrue->Clone("fTrueSpectrum"));
  fMeasSpectrum = dynamic_cast<TH2D *>(hTrain->Clone("fMeasSpectrum"));
  fTrueSpectrumClosure = dynamic_cast<TH2D *>(hTrainTrueClosure->Clone("fTrueSpectrumClosure"));
  fMeasSpectrumClosure = dynamic_cast<TH2D *>(hTrainClosure->Clone("fMeasSpectrumClosure"));

  TPaveText *AnaInfo = new TPaveText(0.2, 0.1, 0.9, 0.9, "brNDC");
  AnaInfo->SetFillStyle(0);
  AnaInfo->SetBorderSize(0);
  AnaInfo->SetTextFont(42);
  AnaInfo->SetTextSize(0.065f);
  AnaInfo->SetTextAlign(11);
  AnaInfo->AddText(Form("%s", fSystemS.Data()));
  AnaInfo->AddText(Form("Charged Jets, Anti-#it{k}_{T}, #it{R} = 0.%d", Rpar));
  AnaInfo->AddText("with D^{0} #rightarrow K^{-}#pi^{+} and charge conj.");
  AnaInfo->AddText(Form("|#it{#eta}_{jet}| < 0.%d", 9 - Rpar));

  TPaveText *pvTrue[fzptJetTrueN];
  TPaveText *pvReco[fzptJetMeasN];
  for (Int_t i = 0; i < fzptJetTrueN; i++)
  {
    for (Int_t j = 0; j < fzptJetMeasN; j++)
    {
      resSlices[i][j] = new TH2D(Form("ResponseSlice%d_%d", i, j), Form("ResponseSlice%d_%d", i, j), fzbinsJetTrueN[fzptJetMeasN - 1], fzbinsJetTrueAPrompt[fzptJetMeasN - 1], fzbinsJetMeasN[fzptJetMeasN - 1], fzbinsJetMeasA[fzptJetMeasN - 1]);
      resSlices[i][j]->GetXaxis()->SetTitle("z^{true}");
      resSlices[i][j]->GetYaxis()->SetTitle("z^{reco}");
    }
  }

  for (Int_t i = 0; i < fzptJetTrueN; i++)
  {
    pvTrue[i] = new TPaveText(0.2, 0.4, 0.8, 0.6, "brNDC");
    pvTrue[i]->SetFillStyle(0);
    pvTrue[i]->SetBorderSize(0);
    pvTrue[i]->SetTextFont(42);
    pvTrue[i]->SetTextSize(0.1f);
    pvTrue[i]->SetTextAlign(11);
    pvTrue[i]->AddText(Form("%d < p_{T,jet}^{true} < %d GeV/#it{c}", static_cast<Int_t>(fzptJetTrueA[i]), static_cast<Int_t>(fzptJetTrueA[i + 1])));
  }
  for (Int_t i = 0; i < fzptJetMeasN; i++)
  {
    pvReco[i] = new TPaveText(0.2, 0.3, 0.8, 0.7, "brNDC");
    pvReco[i]->SetFillStyle(0);
    pvReco[i]->SetBorderSize(0);
    pvReco[i]->SetTextFont(42);
    pvReco[i]->SetTextSize(0.1f);
    pvReco[i]->SetTextAlign(11);
    pvReco[i]->AddText(Form("%d < p_{T,jet}^{reco} < %d GeV/#it{c}", static_cast<Int_t>(fzptJetMeasA[i]), static_cast<Int_t>(fzptJetMeasA[i + 1])));
    pvReco[i]->AddText(Form("%d < p_{T,%s}^{reco} < %d GeV/#it{c}", static_cast<Int_t>(fzptbinsDA[i][0]), fDmesonS.Data(), static_cast<Int_t>(fzptbinsDA[i][fzptbinsDN[i]])));
    pvReco[i]->AddText(Form("%.1f < z_{#parallel}^{reco} < %.1f", static_cast<Double_t>(fzbinsJetMeasA[i][0]), static_cast<Double_t>(fzbinsJetMeasA[i][fzbinsJetMeasN[i]])));
  }

  TH1 *res = response->Hresponse();
  TCanvas *cc = new TCanvas("cc", "cc", 1600, 1600);
  cc->cd();
  cc->SetLogz();
  res->Draw("colz");
  cc->SaveAs(out + "/Unfolding2D_ResponseMatrix.png");
  cc->SaveAs(out + "/Unfolding2D_ResponseMatrix.pdf");

  for (Int_t recoID = 0; recoID < res->GetNbinsX(); recoID++)
  {
    std::cout << recoID + 1 << " reco under " << res->GetBinContent(recoID + 1, 0) << " over " << res->GetBinContent(recoID + 1, res->GetNbinsY() + 1) << std::endl;
    for (Int_t trueID = 0; trueID < res->GetNbinsY(); trueID++)
    {
      //if(recoID==0)std::cout<<trueID<<" true under "<<res->GetBinContent(0,trueID+1)<<" over "<<res->GetBinContent(res->GetNbinsX()+1,trueID+1)<<std::endl;
      Int_t recoPtbin = recoID / fzbinsJetMeasN[fzptJetMeasN - 1];
      Int_t recoZbin = recoID % fzbinsJetMeasN[fzptJetMeasN - 1];
      Int_t truePtbin = trueID / fzbinsJetTrueN[fzptJetMeasN - 1];
      Int_t trueZbin = trueID % fzbinsJetTrueN[fzptJetMeasN - 1];
      resSlices[truePtbin][recoPtbin]->SetBinContent(trueZbin + 1, recoZbin + 1, res->GetBinContent(recoID + 1, trueID + 1));
    }
  }

  TCanvas *cResSlice = new TCanvas("cResSlice", "cResSlice", 800 * fzptJetTrueN + 800, 800 * fzptJetMeasN + 800);
  cResSlice->Divide(fzptJetTrueN + 1, fzptJetMeasN + 1);
  for (Int_t i = 0; i < fzptJetTrueN; i++)
  {
    for (Int_t j = 0; j < fzptJetMeasN; j++)
    {
      if (i == 0 && j == 0)
      {
        cResSlice->cd(1);
        AnaInfo->Draw();
      }
      //std::cout<<i<<" "<<j<<" "<<i+fzptJetTrueN*(j)+1+fzptJetTrueN+j+1+1<<std::endl;
      cResSlice->cd(i + fzptJetTrueN * (j) + 1 + fzptJetTrueN + j + 1 + 1);
      cResSlice->cd(i + fzptJetTrueN * (j) + 1 + fzptJetTrueN + j + 1 + 1)->SetLogz();
      resSlices[i][j]->Draw("colz");
      if (i == j)
      {
        cResSlice->cd((j + 1) * fzptJetTrueN + i + 2);
        pvReco[j]->Draw();
      }
    }
    cResSlice->cd(i + 2);
    pvTrue[i]->Draw();
  }
  cResSlice->SaveAs(out + "/Unfolding2D_Response_Slice.png");
  cResSlice->SaveAs(out + "/Unfolding2D_Response_Slice.pdf");

  return std::make_tuple(response, responseClosure);
}

/// load backround matrix
int LoadBackgroundMatrix(TString fn, TString mxname)
{
  TFile *f = TFile::Open(fn);
  if (!f)
  {
    Error("LoadBackgroundMatrix", "Background matrix file %s not found.", fn.Data());
    return 0;
  }

  fMatrixDeltaPt = dynamic_cast<TH2D *>(f->Get(mxname));
  if (!fMatrixDeltaPt)
  {
    Error("LoadBackgroundMatrix", "Background matrix %s could not be gotten from file.", mxname.Data());
    return 0;
  }
  Info("LoadBackgroundMatrix", "%s loaded.", mxname.Data());

  for (int i = 0; i <= fMatrixDeltaPt->GetNbinsX() + 1; i++)
  {
    for (int j = 0; j <= fMatrixDeltaPt->GetNbinsY() + 1; j++)
    {

      double cont = fMatrixDeltaPt->GetBinContent(i, j);
      if (i == 0 && j == 0)
        fMatrixDeltaPt->SetBinContent(i, j, 0);
      else if (i == fMatrixDeltaPt->GetNbinsX() + 1 && j == fMatrixDeltaPt->GetNbinsY() + 1)
        fMatrixDeltaPt->SetBinContent(i, j, 0);
      else if (fMatrixDeltaPt->GetXaxis()->GetBinCenter(i) < fptbinsDA[0] || fMatrixDeltaPt->GetYaxis()->GetBinCenter(j) < fptbinsDA[0])
        fMatrixDeltaPt->SetBinContent(i, j, 0);
      else
        fMatrixDeltaPt->SetBinContent(i, j, cont);
    }
  }
  return 1;
}

/// load raw spectrum (that is to be unfolded)
/// if sname is not specified, try to load according to default naming conventions
int LoadRawSpectrum(TString fn, TString sname)
{

  fRawSpectrum = new TH2D("fRawSpectrum", "fRawSpectrum", fzbinsJetMeasN[fzptJetMeasN - 1], fzbinsJetMeasA[fzptJetMeasN - 1], fzptJetMeasN, fzptJetMeasA);
  for (Int_t ipt = 1; ipt <= fzptJetMeasN; ipt++)
  {
    TString dummy(fn);
    dummy += Form("%d.root", ipt);
    std::cout << "loading: " << dummy << std::endl;
    TFile *f = TFile::Open(dummy);
    if (!f)
    {
      Error("LoadRawSpectrum", "Raw spectrum file %s not found.", fn.Data());
      return 0;
    }
    TH1D *spectrum = dynamic_cast<TH1D *>(f->Get(sname));
    if (!spectrum)
    {
      Error("LoadRawSpectrum", "Raw spectrum %s could not be gotten from file.", sname.Data());
      return 0;
    }
    Info("LoadRawSpectrum", "%s loaded.", sname.Data());

    for (Int_t iz = 0; iz < fzbinsJetMeasN[ipt - 1]; iz++)
    {
      Int_t Xbin = spectrum->FindBin((fzbinsJetMeasA[ipt - 1][iz] + fzbinsJetMeasA[ipt - 1][iz + 1]) / 2.);
      Int_t Xbin2 = fRawSpectrum->GetXaxis()->FindBin((fzbinsJetMeasA[ipt - 1][iz] + fzbinsJetMeasA[ipt - 1][iz + 1]) / 2.);
      fRawSpectrum->SetBinContent(Xbin2, ipt, spectrum->GetBinContent(Xbin));
      fRawSpectrum->SetBinError(Xbin2, ipt, spectrum->GetBinError(Xbin));
    }
    dummy.ReplaceAll(ipt, ipt + 1);
  }
  fRawSpectrum->Sumw2();
  return 1;
}

/// get response matrix. If no background, use just detector matrix, otherwise get product. Normalize if requested.
TH2D *getResponseMatrix(bool useDeltaPt)
{
  TH2D *mtx;
  TH2D *mtx2;
  TH2D *fMatrixPP2;
  TH2D *fMatrixDeltaPt2;

  if (!useDeltaPt)
  {
    if (!fMatrixPP)
    {
      Error("getResponseMatrix", "No unfolding matrix present.");
      return nullptr;
    }
    mtx = dynamic_cast<TH2D *>(fMatrixPP->Clone());
    //mtx = (TH2D*) fMatrixDeltaPt->Clone();
    mtx->Sumw2();
  }
  else
  {

    mtx = ProductMatrix(fMatrixPP, fMatrixDeltaPt);
  }

  TFile *outFileM = new TFile(Form("outMatrix%s.root", (fBin != 0) ? Form("%d", fBin) : ""), "recreate");
  outFileM->cd();
  fMatrixPP->Write();
  mtx->Write();
  outFileM->Close();
  delete outFileM;

  return mtx;
}

/// get product of two matrices
TH2D *ProductMatrix(TH2D *MtxA, TH2D *MtxB)
{
  // make sure the matrices exist
  if (!MtxA)
  {
    std::cerr << "Error in <AliHeavyUnfoldTools::ProductMatrix> : MtxA==0." << std::endl;
    return nullptr;
  }
  if (!MtxB)
  {
    std::cerr << "Error in <AliHeavyUnfoldTools::ProductMatrix> : MtxB==0." << std::endl;
    return nullptr;
  }

  Int_t binx_a = MtxA->GetNbinsX();
  Int_t biny_a = MtxA->GetNbinsY();
  Int_t binx_b = MtxB->GetNbinsX();
  Int_t biny_b = MtxB->GetNbinsY();

  // make sure the matrices are of the same size
  if (binx_b != binx_a)
  {
    std::cerr << "Error in <AliHeavyUnfoldTools::ProductMatrix> : MtxA--MtxB dimension mismatch." << std::endl;
    return nullptr;
  }

  Double_t x_low = MtxA->GetXaxis()->GetBinLowEdge(1);
  Double_t x_up = MtxA->GetXaxis()->GetBinUpEdge(binx_a);
  Double_t y_low = MtxB->GetYaxis()->GetBinLowEdge(1);
  Double_t y_up = MtxB->GetYaxis()->GetBinUpEdge(biny_a);

  TH2D *MtxC = new TH2D("Matrix_prod", "Product Matrix", binx_a, x_low, x_up, biny_b, y_low, y_up);
  Double_t c = 0;
  for (Int_t k = 0; k <= binx_a + 1; k++)
  {
    for (Int_t i = 0; i <= biny_b + 1; i++)
    {
      c = 0;
      for (Int_t j = 0; j <= biny_a + 1; j++)
      {
        Double_t a = MtxB->GetBinContent(k, j);
        Double_t b = MtxA->GetBinContent(j, i);
        c += a * b;
      }
      //MtxC->Fill(Float_t(k-0.5),Float_t(i-0.5),c);
      MtxC->SetBinContent(k, i, c);
    }
  }

  return MtxC;
}

/// Plot probability matrices
int MtxPlots(TString outDir, TString outName)
{

  TString tag = "tag";
  if (!fMatrixPP)
  {
    Error("MtxPlots", "No unfolding matrix present.");
    return 80;
  }

  TH2D *hMtxPP = nullptr;
  TH2D *hMtxDpt = nullptr;
  TH2D *hMtxRe = nullptr;
  TH2D *hMtxPro = nullptr;

  // Probabilities
  TCanvas *cMtx = new TCanvas("ProbMtx", "Probability matrices", 50, 50, 800, 800);
  cMtx->Divide(2, 2);

  cMtx->cd(1)->SetLogz();
  cMtx->SetLogz();
  hMtxPP = dynamic_cast<TH2D *>(NormMatrixY("hMtxPP" + tag, fMatrixPP));
  hMtxPP->SetTitle("Detector prob. matrix");
  NormMatrixY(hMtxPP);
  hMtxPP->Draw("colz");

  if (fMatrixDeltaPt)
  {
    cMtx->cd(2)->SetLogz();
    cMtx->SetLogz();
    hMtxDpt = dynamic_cast<TH2D *>(NormMatrixY("hMtxDpt" + tag, fMatrixDeltaPt));
    hMtxDpt->SetTitle("Background prob. matrix");
    hMtxDpt->Draw("colz");

    cMtx->cd(3)->SetLogz();
    //if(!fMatrixProd)
    //	fMatrixProd = getResponseMatrix( fMatrixDeltaPt );

    if (!fMatrixProd)
    {
      Error("MtxPlots", "Error getting product matrix!");
      return 80;
    }

    //hMtxRe = (TH2D*)NormMatrixY("hMtxRe"+tag,fMatrixProd);

    hMtxRe = dynamic_cast<TH2D *>(fMatrixProd->Clone("hMtRe"));
    hMtxRe->SetTitle("Response prob. matrix");
    hMtxRe->Draw("colz");

    cMtx->cd(4)->SetLogz();
    hMtxPro = ProductMatrix(hMtxPP, hMtxDpt);
    hMtxPro->SetName("hMtxPro" + tag);
    hMtxPro->SetTitle("Product prob. matrix");
    NormMatrixY(hMtxPro);
    hMtxPro->Draw("colz");
  }

  cMtx->SaveAs(Form("%s/plots%s/%s_probMtx.pdf", outDir.Data(), (fBin != 0) ? Form("%d", fBin) : "", outName.Data()));
  cMtx->SaveAs(Form("%s/plots%s/%s_probMtx.png", outDir.Data(), (fBin != 0) ? Form("%d", fBin) : "", outName.Data()));
  cMtx->SaveAs(Form("%s/plots%s/%s_probMtx.svg", outDir.Data(), (fBin != 0) ? Form("%d", fBin) : "", outName.Data()));

  TCanvas *cSlices = new TCanvas("ProbSlice", "Probability slices", 1000, 1000);
  cSlices->Divide(3, 2);
  plotSlice(cSlices->cd(1), hMtxPP, hMtxDpt, hMtxRe, hMtxPro, 5, 10);
  plotSlice(cSlices->cd(2), hMtxPP, hMtxDpt, hMtxRe, hMtxPro, 10, 15);
  plotSlice(cSlices->cd(3), hMtxPP, hMtxDpt, hMtxRe, hMtxPro, 15, 20);
  plotSlice(cSlices->cd(4), hMtxPP, hMtxDpt, hMtxRe, hMtxPro, 20, 25);
  plotSlice(cSlices->cd(5), hMtxPP, hMtxDpt, hMtxRe, hMtxPro, 25, 30);

  cSlices->SaveAs(Form("%s/plots%s/%s_probSlices.pdf", outDir.Data(), (fBin != 0) ? Form("%d", fBin) : "", outName.Data()));
  cSlices->SaveAs(Form("%s/plots%s/%s_probSlices.png", outDir.Data(), (fBin != 0) ? Form("%d", fBin) : "", outName.Data()));
  cSlices->SaveAs(Form("%s/plots%s/%s_probSlices.svg", outDir.Data(), (fBin != 0) ? Form("%d", fBin) : "", outName.Data()));

  return 0;
}

/// Plot probability matrices
int plotSlice(TVirtualPad *p, TH2D *hMtxPP, TH2D *hMtxDpt, TH2D *hMtxRe, TH2D *hMtxPro, const double ptmin, const double ptmax)
{

  TH1D *hSliceDpt = nullptr;
  TH1D *hSlicePP = nullptr;
  TH1D *hSliceRe = nullptr;
  TH1D *hSlicePro = nullptr;

  int imin = hMtxPP->GetYaxis()->FindBin(ptmin + 0.0001);
  int imax = hMtxPP->GetYaxis()->FindBin(ptmax - 0.0001);

  TString sname = "-" + TString::Itoa(static_cast<Int_t>(ptmin), 10) + "-" + TString::Itoa(static_cast<Int_t>(ptmax), 10);
  TString stitle = TString::Itoa(static_cast<Int_t>(ptmin), 10) + "<p_{T}^{gen}<" + TString::Itoa(static_cast<Int_t>(ptmax), 10) + " GeV/c";

  p->SetLogy();

  // takes width of 1st bin : only works for even binning!
  int binno = 0;
  if (fObservable == Observable::kXsection)
  {
    binno = static_cast<Int_t>(fptbinsJetTrueA[1] - fptbinsJetTrueA[0]);
  }
  if (fObservable == Observable::kFragmentation)
  {
    binno = static_cast<Int_t>(fzbinsJetTrueAPrompt[1] - fzbinsJetTrueAPrompt[0]);
  }

  if (hMtxDpt)
  {
    hSliceDpt = hMtxDpt->ProjectionX("hSliceDpt" + sname, imin, imax);
    hSliceDpt->SetTitle("probability matrix");
    hSliceDpt->SetMarkerColor(2);
    hSliceDpt->SetMarkerSize(0.8f);
    hSliceDpt->SetMarkerStyle(20);
    hSliceDpt->SetLineColor(2);
    //setHistStyle(hSliceDpt, 2, 21);
    hSliceDpt->Rebin(binno);
    //hSliceDpt->Draw("p");
  }

  hSlicePP = hMtxPP->ProjectionX("hSlicePP" + sname, imin, imax);
  hSlicePP->SetMarkerColor(4);
  hSlicePP->SetMarkerStyle(20);
  hSlicePP->SetMarkerSize(0.8f);
  hSliceDpt->SetLineColor(4);
  hSlicePP->Rebin(binno);
  /*if (hMtxDpt)
        hSlicePP->Draw("psame");
    else
        hSlicePP->Draw("HistP");*/

  if (hMtxRe)
  {
    hSliceRe = hMtxRe->ProjectionX("hSliceRe" + sname, imin, imax);
    //setHistStyle(hSliceRe, 4, 26);
    hSliceRe->SetMarkerColor(kMagenta);
    hSliceRe->SetMarkerStyle(20);
    hSliceRe->SetMarkerSize(0.8f);
    hSliceRe->SetLineColor(1);
    hSliceRe->Rebin(binno);
    //hSliceRe->Draw("HistPSame");
  }

  if (hMtxPro)
  {
    hSlicePro = hMtxPro->ProjectionX("hSlicePro" + sname, imin, imax);
    //setHistStyle(hSlicePro, 6, 32);
    hSlicePro->SetMarkerColor(1);
    hSlicePro->SetMarkerStyle(20);
    hSlicePro->SetMarkerSize(0.8f);
    hSlicePro->SetLineColor(1);
    hSlicePro->Rebin(binno);
    hSlicePro->Draw("HistPSame");
  }

  hSliceDpt->Draw("phist");
  hSlicePP->Draw("phistsame");
  //hSliceRe->Draw("psame");
  hSlicePro->Draw("phistsame");

  if (p)
  {
    TLegend *l = p->BuildLegend(0.5, 0.7, 0.89, 0.89, "");
    l->SetHeader(stitle);
    l->Draw();
  }
  return 0;
}

/// Weight matrix along y axis by histo values
void WeightMatrixY(TH2D *Mtx, TH1D *h, bool divide)
{
  if (!Mtx)
  {
    std::cerr << "Warning in <AliHeavyUnfoldTools::WeightMatrixY> : Mtx==0." << std::endl;
    return;
  }
  if (!h)
  {
    std::cerr << "Warning in <AliHeavyUnfoldTools::WeightMatrixY> : h==0." << std::endl;
    return;
  }

  for (int j = 1; j <= Mtx->GetNbinsY() + 1; j++)
  {

    double value = Mtx->GetYaxis()->GetBinCenter(j);
    double c = h->GetBinContent(h->GetXaxis()->FindBin(value));
    if (divide && c)
      c = 1. / c;
    //else c = 1.;
    for (int i = 1; i <= Mtx->GetNbinsX() + 1; i++)
    {
      Mtx->SetBinContent(i, j, Mtx->GetBinContent(i, j) * c);
      //Mtx->SetBinError(i, j, Mtx->GetBinError(i,j)*c);
    }
  }
}

/// rebin in 2d variable size - no such routine in Root
TH2D *Rebin2D(const char *name, TH2D *h, int nx, const double *binx, int ny, const double *biny, bool crop)
{
  if (!h)
  {
    std::cerr << "Warning in <AliHeavyUnfoldTools::Rebin2D> : h==0." << std::endl;
    return nullptr;
  }

  TAxis *xaxis = h->GetXaxis();
  TAxis *yaxis = h->GetYaxis();

  TH2D *hre = new TH2D(name, name, nx, binx, ny, biny);
  hre->Sumw2();
  for (int i = 1; i <= xaxis->GetNbins(); i++)
  {
    for (int j = 1; j <= yaxis->GetNbins(); j++)
    {
      hre->Fill(xaxis->GetBinCenter(i), yaxis->GetBinCenter(j), h->GetBinContent(i, j));
    }
  }

  /*
    const double epsilon = 0.00001;
    int ixmin = crop ? 0  : -1;
    int ixmax = crop ? nx : nx+1;
    int iymin = crop ? 0  : -1;
    int iymax = crop ? ny : ny+1;

    for(int ix=ixmin; ix<ixmax; ix++) {
        double xlo = (ix!=-1) ? binx[ix]   : h->GetXaxis()->GetBinCenter(0)-epsilon;
        double xhi = (ix!=nx) ? binx[ix+1] : h->GetXaxis()->GetBinCenter(h->GetNbinsX()+1)+epsilon;
        for(int iy=iymin; iy<iymax; iy++) {
            double ylo = (iy!=-1) ? binx[iy]   : h->GetYaxis()->GetBinCenter(0)-epsilon;
            double yhi = (iy!=ny) ? binx[iy+1] : h->GetYaxis()->GetBinCenter(h->GetNbinsY()+1)+epsilon;
            int k = 0;
            double hits = 0;
            double ersq = 0;
            for(int jx=0; jx<=h->GetNbinsX()+1; jx++) {
                if (h->GetXaxis()->GetBinCenter(jx) <  xlo) continue;
                if (h->GetXaxis()->GetBinCenter(jx) >= xhi) continue;
                for(int jy=0; jy<=h->GetNbinsY()+1; jy++) {
                    if (h->GetYaxis()->GetBinCenter(jy) <  ylo) continue;
                    if (h->GetYaxis()->GetBinCenter(jy) >= yhi) continue;

                    hits += h->GetBinContent(jx,jy);
                    ersq += pow(h->GetBinError(jx,jy)/h->GetBinContent(jx,jy),2);
                    k++;
                }
            }

            hre->SetBinContent(ix+1, iy+1, hits);
            hre->SetBinError(ix+1, iy+1, hits*sqrt(ersq));
            //hre->SetBinError(ix+1, iy+1, 0);
        }
    }
*/

  //for(int i=0;i<=hre->GetNbinsX();i++){
  for (int j = 0; j <= hre->GetNbinsY() + 1; j++)
  {
    hre->SetBinContent(0, j, 0);
    hre->SetBinError(0, j, 0);

    hre->SetBinContent(hre->GetNbinsX() + 1, j, 0);
    hre->SetBinError(hre->GetNbinsX() + 1, j, 0);
  }
  //}

  return hre;
}

/// Prior for unfolding: try different prior functions wich best describe the raw specrum.
TH2D *getPriorFunction(TH2D *baseweight, TF2 *modfunc, Int_t modsign, Int_t modpower)
{

  if (!baseweight)
  {
    Error("getPriorFunction", "Required spectrum does not exist.");
    return nullptr;
  }

  for (Int_t binx = 0; binx <= baseweight->GetNbinsX(); binx++)
  {
    for (Int_t biny = 0; biny <= baseweight->GetNbinsY(); biny++)
    {
      Double_t xval = baseweight->GetXaxis()->GetBinCenter(binx);
      Double_t yval = baseweight->GetYaxis()->GetBinCenter(biny);
      Double_t mod = modfunc->Eval(xval, yval);
      Double_t new_weight = 1 + modsign * mod / modpower;
      baseweight->SetBinContent(binx, biny, new_weight);
      if (baseweight->GetBinContent(binx, biny) <= 1e-20)
      {
        baseweight->SetBinContent(binx, biny, 1);
      }
    }
  }

  return baseweight;
}

/// Create a new, y-normalized matrix
TH2D *NormMatrixY(const char *name, TH2D *Mtx)
{
  if (!Mtx)
  {
    std::cerr << "Warning in <AliHeavyUnfoldTools::NormMatrixY> : Mtx==0." << std::endl;
    return nullptr;
  }

  TH2D *Mre = dynamic_cast<TH2D *>(Mtx->Clone(name));
  NormMatrixY(Mre);

  return Mre;
}

/// Normalize matrix along y axis projection
void NormMatrixY(TH2D *Mtx)
{
  if (!Mtx)
  {
    std::cerr << "Warning in <AliHeavyUnfoldTools::NormMatrixY> : Mtx==0." << std::endl;
    return;
  }

  TH1D *h = Mtx->ProjectionY();
  WeightMatrixY(Mtx, h, true);
}

/// get pearson coeffs from covariance matrix
TH2D *getPearsonCoeffs(const TMatrixD &covMatrix)
{

  Int_t nrows = covMatrix.GetNrows();
  Int_t ncols = covMatrix.GetNcols();

  TH2D *PearsonCoeffs = new TH2D("PearsonCoeffs", "Pearson Coefficients", nrows, 0, nrows, ncols, 0, ncols);
  for (Int_t row = 0; row < nrows; row++)
  {
    for (Int_t col = 0; col < ncols; col++)
    {
      Double_t pearson = 0.;
      if (covMatrix(row, row) != 0. && covMatrix(col, col) != 0.)
        pearson = covMatrix(row, col) / TMath::Sqrt(covMatrix(row, row) * covMatrix(col, col));
      PearsonCoeffs->SetBinContent(row + 1, col + 1, pearson);
    }
  }

  return PearsonCoeffs;
}

void SparseToTree(TString MCfile)
{
  // Read the THnSparse
  TFile *File = new TFile(MCfile, "read");
  TDirectoryFile *dir = (TDirectoryFile *)File->Get("DmesonsForJetCorrelations");
  TString histName;
  if (fDmesonSpecie)
    histName = "histosDStarMBN";
  else
    histName = "histosD0MBN";
  TList *histList[NDMC];
  THnSparseF *sparseMC[NDMC];
  THnSparseF *h = nullptr;

  for (int i = 0; i < 2; i++)
  {
    histList[i] = (TList *)dir->Get(Form("%s%dMCrec", histName.Data(), i));
    sparseMC[i] = (THnSparseF *)histList[i]->FindObject("ResponseMatrix");
    std::cout << sparseMC[i]->GetNbins() << std::endl;
    if (!i)
      h = dynamic_cast<THnSparseF *>(sparseMC[0]->Clone("ResponseMatrixSum"));
    else
      h->Add(sparseMC[i]);
  }
  //copy from here: https://root.cern.ch/root/html/tutorials/tree/drawsparse.C.html
  // Creates a TTree and fills it with the coordinates of all
  // filled bins. The tree will have one branch for each dimension,
  // and one for the bin content.
  TString outName(MCfile);
  outName.Remove(outName.Length() - 5, 5);
  outName += "TTree.root";
  std::cout << "Converting THnSparse: " << MCfile << std::endl;
  std::cout << "Into TTree: " << outName << std::endl;
  TFile *File2 = new TFile(outName, "RECREATE");
  Int_t dim = h->GetNdimensions();
  TString name(h->GetName());
  name += "_tree";
  TString title(h->GetTitle());
  title += " tree";

  TTree *tree = new TTree(name, title);
  Double_t *x = new Double_t[dim + 1];
  memset(x, 0, sizeof(Double_t) * (dim + 1));

  TString branchname;
  for (Int_t d = 0; d < dim; ++d)
  {
    if (branchname.Length())
      branchname += ":";
    TAxis *axis = h->GetAxis(d);
    branchname += axis->GetName();
    branchname += "/D";
  }
  tree->Branch("coord", x, branchname);
  tree->Branch("bincontent", &x[dim], "bincontent/D");

  const int percentPrint = 1;
  int totalWindows = h->GetNbins();
  int percent;
  int step = totalWindows / (100 / percentPrint);
  int nextPrint = step;

  Int_t *bins = new Int_t[dim];
  for (Long64_t i = 0; i < h->GetNbins(); ++i)
  {
    if (i >= nextPrint)
    {
      percent = (100 * i) / totalWindows;
      std::cout << "\r" << std::string(percent / percentPrint, '|') << percent << "%";
      std::cout.flush();
      nextPrint += step;
    }
    x[dim] = h->GetBinContent(i, bins);
    for (Int_t d = 0; d < dim; ++d)
    {
      x[d] = h->GetAxis(d)->GetBinCenter(bins[d]);
    }

    tree->Fill();
  }
  std::cout << std::endl
            << "tree entries " << tree->GetEntries() << std::endl;
  File2->Write(nullptr, TObject::kOverwrite);
  File2->Close();
}
