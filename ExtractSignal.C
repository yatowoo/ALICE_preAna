// Macro for result post-processing

// Fit Me+e- (invariant mass of di-electron)

TLegend* yatoLegend(){
		TLegend* yLgd= new TLegend(0.20,0.15,0.6,0.35);
		yLgd->SetName("mgLegend");
		yLgd->SetBorderSize(0);
		yLgd->SetTextAlign(12);
		yLgd->SetTextFont(42);
		yLgd->SetTextSize(0.04);
		return yLgd;
}

TVirtualPad* DrawFitResult(TH1* invmass, TF1* total, TF1* jpsi, TF1* bkg){
  TVirtualPad* fCanvas = gPad;
  if(!fCanvas){
    fCanvas = new TCanvas("cAna", "Canvas for Jpsi signal extraction", 800, 600);
  }

  return fCanvas;
}

int ExtractSignal(TH1* invmass = NULL){
  // Crystalball Function = Gaus + X^n
    // Parameters : \alpha = break point, n, \sigma = width, \mu = peak
    // Unit : Y = Ncount/0.04, X=GeV/c^2
  auto jpsi = new TF1("fJpsi", "[A]*ROOT::Math::crystalball_function(x,[#alpha],[n],[#sigma],[#mu])",1.,5.);
  jpsi->SetLineColor(kBlack);
  jpsi->SetLineWidth(2);
  jpsi->SetLineStyle(2);
  jpsi->SetNpx(1000);

  // Backgroud Function - Pol2
  auto bkg = new TF1("fBkg", "[a0]+[a1]*x+[a2]*x^2", 1., 5.);
  bkg->SetLineColor(kGreen);
  bkg->SetLineWidth(2);
  bkg->SetLineStyle(2);
  bkg->SetNpx(1000);

  // Total function for fitting
  auto total = new TF1("fTot","fJpsi+fBkg", 1., 5.);
  total->SetLineColor(kRed);
  total->SetLineWidth(3);
  total->SetNpx(1000);
    // Jpsi parameter
  total->SetParameter("A", 116);
  total->SetParameter("#alpha", 0.3);
  total->SetParameter("n", 2.4e8);
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

  invmass->Fit(total, "IS", "", 1.5, 4.5);
  invmass->Draw("PE0");
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

  jpsi->SetParameter("A", total->GetParameter("A"));
  jpsi->SetParameter("#alpha", total->GetParameter("#alpha"));
  jpsi->SetParameter("n", total->GetParameter("n"));
  jpsi->SetParameter("#sigma", total->GetParameter("#sigma"));
  jpsi->SetParameter("#mu", total->GetParameter("#mu"));
  jpsi->Draw("same");
  
  bkg->SetParameter("a0", total->GetParameter("a0"));
  bkg->SetParameter("a1", total->GetParameter("a1"));
  bkg->SetParameter("a2", total->GetParameter("a2"));
  bkg->Draw("same");

  // Setup for legend
  auto lgd = yatoLegend();
  lgd->AddEntry(invmass, "e^{+}e^{-} signal");
  lgd->AddEntry(total,"Total fit");
  lgd->AddEntry(bkg,"Bkg fit (pol2)");
  lgd->AddEntry(jpsi,"Signal fit (Crystal-Ball)");
  lgd->Draw("same");

  return 0;
}