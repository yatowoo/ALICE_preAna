// Macro for result post-processing

// Fit J/psi signal in Me+e- spectrum (invariant mass of di-electron)

#define DEBUG

TF1* jpsi = NULL;
TF1* bkg = NULL;
TF1* total = NULL;

TLegend* yatoLegend(){
		TLegend* yLgd= new TLegend(0.20,0.15,0.6,0.35);
		yLgd->SetName("mgLegend");
		yLgd->SetBorderSize(0);
		yLgd->SetTextAlign(12);
		yLgd->SetTextFont(42);
		yLgd->SetTextSize(0.04);
		return yLgd;
}

// Input mass range for estimation of signal & background
int SelectSignalRegion(Double_t mlow, Double_t mup, Double_t width = 0.04){
  // Denominator check
  if( width < 1e-6 ){
    cout << "[+] WARNNING - Bin width given may be too small : " << width << endl;
  }
  // Fit result check
  auto fitter = TVirtualFitter::GetFitter();
  if(!fitter){
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
  return 0;
}

int ExtractSignal(TH1* invmass = NULL){
  // Crystalball Function = Gaus + X^n
    // Parameters : \alpha = break point, n, \sigma = width, \mu = peak
    // Unit : Y = Ncount/0.04, X=GeV/c^2
  jpsi = new TF1("fJpsi", "[0]*ROOT::Math::crystalball_function(x,[1],[2],[3],[4])",1.,5.);
  jpsi->SetLineColor(kBlack);
  jpsi->SetLineWidth(2);
  jpsi->SetLineStyle(2);
  jpsi->SetNpx(1000);

  // Backgroud Function - Pol2
  bkg = new TF1("fBkg", "[0]+[1]*x+[2]*x^2", 1., 5.);
  bkg->SetLineColor(kGreen);
  bkg->SetLineWidth(2);
  bkg->SetLineStyle(2);
  bkg->SetNpx(1000);

  // Total function for fitting
  total = new TF1("fTot","fJpsi+fBkg", 1., 5.);
  total->SetParNames("A", "#alpha", "n", "#sigma", "#mu",
    "a0", "a1", "a2");
  total->SetLineColor(kRed);
  total->SetLineWidth(3);
  total->SetNpx(1000);
    // Jpsi parameter
  total->SetParameter("A", 116);
  total->SetParameter("#alpha", 0.3);
  total->SetParameter("n", 1.);
  total->SetParameter("#sigma", 0.04);
  total->SetParameter("#mu", 3.1);
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
  invmass->Fit(total, "IS", "", 1.5, 4.5);
    // Draw marker & error bar (with empty bins)
  invmass->Draw("PE0");

  // Canvas configuration
  if(gPad){
    invmass->GetYaxis()->SetRangeUser(0,300);
    invmass->GetYaxis()->SetTitle("N_{pairs} / 0.04 GeV/c^{2}");
    invmass->GetXaxis()->SetRangeUser(1.5,4.5);
    invmass->GetXaxis()->SetTitle("M_{e^{+}e^{-}} (GeV/c^{2})");
      // Show fit parameters
    gStyle->SetOptFit(1111);
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