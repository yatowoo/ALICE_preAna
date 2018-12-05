// Toy MC for J/psi decay
//#define DEBUG

// Require ROOT6
#include<TLorentzVector.h>

// Use Float_t instead of double to save disk size
const Float_t MASS_JPSI = 3.096; // GeV/c^2
const Float_t MASS_ELECTRON = 511e-6; // GeV/c^2

// Levy-Tsallis Function for Pt 
// Variable : Pt - Traverse Momentum
// Parameters : dNdy, n, C, mass
Double_t LevyFCN (Double_t* x, Double_t* par) {
	Double_t pt = x[0];
	Double_t dNdy = par[0];
	Double_t n = par[1];
	Double_t C = par[2];
	Double_t mass = par[3];
	Double_t mt = TMath::Sqrt(pt*pt+mass*mass);
	
	return pt * dNdy * (n-1) * (n-2) / (n*C * (n*C + mass * ( n -2))) \
		* TMath::Power( 1 +( mt -mass) / (n*C), -n);
	
}

/*
* Methods from LIU, Zhen (刘圳)
*/
// Boost daughter into LAB
TLorentzVector myBoost(TLorentzVector parent, TLorentzVector daughter)
{
  Float_t betax = parent.Px() / parent.E();
  Float_t betay = parent.Py() / parent.E();
  Float_t betaz = parent.Pz() / parent.E();
  daughter.Boost(betax, betay, betaz);
  return daughter;
}
// J/psi decay at rest
TLorentzVector twoBodyDecay(TLorentzVector parent, Float_t dmass)
{
  Float_t e = parent.M() / 2.;
  Float_t p = sqrt(e * e - dmass * dmass);
  Float_t costheta = gRandom->Uniform(-1.0, 1.0);
  Float_t phi = gRandom->Uniform(-TMath::Pi(), TMath::Pi());
  Float_t pz = p * costheta;
  Float_t px = p * sqrt(1. - costheta * costheta) * cos(phi);
  Float_t py = p * sqrt(1. - costheta * costheta) * sin(phi);
  TLorentzVector daughter(px, py, pz, e);
  return myBoost(parent, daughter);
}
// J/psi generator
TLorentzVector getJpsi(TF1 *fPt, Float_t pt_lower, Float_t pt_upper)
{

  Float_t mc_pt = fPt->GetRandom(pt_lower, pt_upper);
  Float_t mc_phi = gRandom->Uniform(-1 * TMath::Pi(), TMath::Pi());
  Float_t mc_y = gRandom->Uniform(-0.5, 0.5);
  Float_t mc_px = mc_pt * TMath::Cos(mc_phi);
  Float_t mc_py = mc_pt * TMath::Sin(mc_phi);
  Float_t mc_pz = sqrt(mc_pt * mc_pt + MASS_JPSI * MASS_JPSI) * TMath::SinH(mc_y);
  TLorentzVector parent;
  parent.SetXYZM(mc_px, mc_py, mc_pz, MASS_JPSI);
#ifdef DEBUG
    cout << "[-] INFO - jpsi info; pt:" << mc_pt << "; phi:" << mc_phi << endl;
#endif // DEBUG
  return parent;
}
// END - Methods from LIU, Zhen (刘圳)

// Draw histogram for MC result
int DrawMC(TString fileName, TString resultFileName = "JpsiToyMC_result.root", Int_t nEvent = 1000){
  TFile* fdata = NULL;
  TFile* _file0 = (TFile*)gDirectory;
  if(_file0) // after 'root -l *.root
    fdata = _file0;
  else{
    fdata = new TFile(fileName, "READ");
    if(!fdata->IsOpen()){
      cout << "[X] ERROR - File not found : " << fileName << endl;
      return -1;
    }
  }// Create file object

  TTree* event = (TTree*)(fdata->Get("event"));
  TLorentzVector* jpsi = new TLorentzVector;
  TLorentzVector* eplus = new TLorentzVector;
  TLorentzVector* eminus = new TLorentzVector;
  event->SetBranchAddress("jpsi", &jpsi);
  event->SetBranchAddress("eplus", &eplus);
  event->SetBranchAddress("eminus", &eminus);

  // EP = eplus, EM = eminus
    // J/psi - pt, eta, phi
  TH1* hJpsi_pt = new TH1F("hJpt", "J/#psi p_{t} distribution;p_{t} (GeV/c);N_{J/#psi} / (.05 GeV/c)", 600, 0, 30.);
  TH1* hJpsi_eta = new TH1F("hJeta", "J/#psi #eta distribution;#eta;N_{J/#psi} / 0.01", 1000, -5., 5.);
  TH1* hJpsi_phi = new TH1F("hJphi", "J/#psi #phi distribution;#phi (rad);N_{J/#psi} / 0.01 rad", 1000, -5., 5.);
    // Eplus - pt, eta, phi
  TH1* hEP_pt = new TH1F("hEPpt", "e^{+} p_{t} distribution;p_{t} (GeV/c);N_{e^{+}} / (0.05 GeV/c)", 600, 0, 30.);
  TH1* hEP_eta = new TH1F("hEPeta", "e^{+} #eta distribution;#eta;N_{e^{+}} / 0.01", 1000, -5., 5.);
  TH1* hEP_phi = new TH1F("hEPphi", "e^{+} #phi distribution;#phi (rad);N_{e^{+}} / 0.01 rad", 1000, -5., 5.);
    // Eminus - pt, eta, phi
  TH1* hEM_pt = new TH1F("hEMpt", "e^{-} p_{t} distribution;p_{t} (GeV/c);N_{e^{-}} / (.05 GeV/c)", 600, 0, 30.);
  TH1* hEM_eta = new TH1F("hEMeta", "e^{-} #eta distribution;#eta;N_{e^{-}} / 0.01", 1000, -5., 5.);
  TH1* hEM_phi = new TH1F("hEMphi", "e^{-} #phi distribution;#phi (rad);N_{e^{-}} / 0.01 rad", 1000, -5., 5.);
    // Pair - EPpt vs EMpt, OpeningAngle
  TH2* hPair_pt = new TH2F("hPairPt", "e^{+}e^{-} p_{t} correlation;p_{t}(e^{+}) (GeV/c);p_{t}(e^{-}) (GeV/c);N_{pair}", 600, 0, 30., 600, 0, 30.);
  TH1* hPair_angle = new TH1F("hPairAngle", "e^{+}e^{-} Opening Angle;angle (rad);N_{pair} / 0.01 rad", 500, 0, 5.);

  if(nEvent >= event->GetEntries())
    nEvent = event->GetEntries();
  for(Int_t iev = 0; iev < nEvent; iev++){
    event->GetEntry(iev);
    hJpsi_pt->Fill(jpsi->Pt());
    hJpsi_eta->Fill(jpsi->Eta());
    hJpsi_phi->Fill(jpsi->Phi());
    hEP_pt->Fill(eplus->Pt());
    hEP_eta->Fill(eplus->Eta());
    hEP_phi->Fill(eplus->Phi());
    hEM_pt->Fill(eminus->Pt());
    hEM_eta->Fill(eminus->Eta());
    hEM_phi->Fill(eminus->Phi());
    hPair_pt->Fill(eplus->Pt(), eminus->Pt());
    hPair_angle->Fill(eplus->Angle(eminus->Vect()));
    if(iev % (nEvent / 25) == 0){
      cout << "[-] Progress : " << int(iev/1./nEvent*100) 
        << "% (" << iev << "/" << nEvent << ")" << endl;
    }
  }// Event loop
  cout << endl;
  //fdata->Close();

  TFile* result = new TFile(resultFileName,"RECREATE");
  hJpsi_pt->Write();
  hJpsi_eta->Write();
  hJpsi_phi->Write();
  hEP_pt->Write();
  hEP_eta->Write();
  hEP_phi->Write();
  hEM_pt->Write();
  hEM_eta->Write();
  hEM_phi->Write();
  hPair_pt->Write();
  hPair_angle->Write();
  result->Close();

  return 0;
}

int JpsiToyMC(Int_t nEvent = 1e3, Double_t PT_LOWER = 0., Double_t PT_UPPER = 30., TString output_dir = "."){
  gRandom->SetSeed(time(NULL));

  // Au-Au 200 GeV/c
  //TF1 *fPt = new TF1("fPt", "(3.4948*TMath::Power(TMath::Exp((-0.395305)*x)+x/2.91793,(-8.46161)))*4*TMath::Pi()*x", 0, 20);
  TF1* fPt = new TF1("fLevy", LevyFCN, PT_LOWER, PT_UPPER, 4);
  fPt->SetParName(0, "dN/dy");
  fPt->SetParName(1, "C");
  fPt->SetParName(2, "n");
  fPt->SetParName(3, "m_{0}");
    // pp 13TeV - Levy fit with ALICE-2017
  fPt->SetParameters(7413., 7.79, 0.7053, MASS_JPSI);

  TFile* f = new TFile(
    Form("%s/JpsiDecay_%0.f-%0.f.root", output_dir.Data(), PT_LOWER, PT_UPPER), 
    "RECREATE");
  TTree* tree = new TTree("event", "J/psi decay event");
  TLorentzVector* jpsi = new TLorentzVector;
  TLorentzVector& obj_jpsi = *jpsi;
  TLorentzVector* eplus = new TLorentzVector;
  TLorentzVector& obj_eplus = *eplus;
  TLorentzVector* eminus = new TLorentzVector;
  TLorentzVector& obj_eminus = *eminus;
  tree->Branch("jpsi","TLorentzVector",&jpsi);
  tree->Branch("eplus","TLorentzVector",&eplus);
  tree->Branch("eminus","TLorentzVector",&eminus);
  for (Int_t iev = 0; iev < nEvent ; iev++){
    obj_jpsi = getJpsi(fPt, PT_LOWER, PT_UPPER);
    obj_eplus = twoBodyDecay(obj_jpsi, MASS_ELECTRON);
    obj_eminus = obj_jpsi - obj_eplus;
#ifdef DEBUG
    jpsi->Print();
    eplus->Print();
    eminus->Print();
#endif // DEBUG
    tree->Fill();
    if(iev % (nEvent / 25) == 0){
      cout << "[-] Progress : " << int(iev/1./nEvent*100) 
        << "% (" << iev << "/" << nEvent << ")" << endl;
    }
  }
  cout << "[-] INFO - Event loop finished : " << nEvent << " events" << endl;
#ifdef DEBUG
  tree->Print();
#endif // DEBUG
  f->Write();
  f->Close();
  return 0;
}