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

int ExtractSignal(TH1* invmass = NULL){
  // Crystalball Function = Gaus + X^n
    // Parameters : \alpha = break point, n, \sigma = width, \mu = peak
    // Unit : Y = Ncount/0.04, X=GeV/c^2
  auto jpsi = new TF1("fJpsi", "[0]*ROOT::Math::crystalball_function(x,[1],[2],[3],[4])",1.,5.);
  jpsi->SetParameter(0, 100);
  jpsi->SetParameter(1, 2.9);
  jpsi->SetParameter(2, 3);
  jpsi->SetParameter(3, 0.05);
  jpsi->SetParameter(4, 3.1);
  jpsi->SetLineColor(kBlack);
  jpsi->SetLineWidth(2);
  jpsi->SetLineStyle(2);
  jpsi->SetNpx(1000);

  // Backgroud Function - Pol2
  auto bkg = new TF1("fBkg", "[0]+[1]*x+[2]*x^2", 1., 5.);
  bkg->SetParameter(0, 100);
  bkg->SetParameter(1, -20);
  bkg->SetParameter(2, 1.);
  bkg->SetLineColor(kGreen);
  bkg->SetLineWidth(2);
  bkg->SetLineStyle(2);
  bkg->SetNpx(1000);

  // Total function for fitting
  auto total = new TF1("fTot","fJpsi+fBkg", 1., 5.);
  total->SetLineColor(kRed);
  total->SetLineWidth(3);
  total->SetNpx(1000);
  total->SetParameter(0, 140);
  total->SetParameter(1, 0.05);
  total->SetParameter(2, 1e6);
  total->SetParameter(3, 0.01);
  total->SetParameter(4, 3.1);
  total->SetParameter(5, 240.);
  total->SetParameter(6, -100);
  total->SetParameter(7, 10.);

  // Check input histogram
  if(invmass == NULL) return 1;

  // Fit & draw
  invmass->SetLineColor(kBlue);
  invmass->SetMarkerColor(kBlue);
  invmass->SetMarkerStyle(20);

  invmass->Fit(total, "", "", 1.5, 4.5);
  invmass->Draw("PE0");

  Double_t pars[8];
  total->GetParameters(pars);
  jpsi->SetParameter(0, pars[0]);
  jpsi->SetParameter(1, pars[1]);
  jpsi->SetParameter(2, pars[2]);
  jpsi->SetParameter(3, pars[3]);
  jpsi->SetParameter(4, pars[4]);
  jpsi->Draw("same");
  
  bkg->SetParameter(0, pars[5]);
  bkg->SetParameter(1, pars[6]);
  bkg->SetParameter(2, pars[7]);
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