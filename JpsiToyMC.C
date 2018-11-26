// Toy MC for J/psi decay
//#define DEBUG

// Require ROOT6
#include "TLorentzVector.h"

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

int JpsiToyMC(Int_t nEvent = 1e3, TString output_dir = "."){
  gRandom->SetSeed(time(NULL));

  // Au-Au 200 GeV/c
  //TF1 *fPt = new TF1("fPt", "(3.4948*TMath::Power(TMath::Exp((-0.395305)*x)+x/2.91793,(-8.46161)))*4*TMath::Pi()*x", 0, 20);
  const Double_t PT_LOWER = 0.;
  const Double_t PT_UPPER = 30.; // GeV/c
  TF1* fPt = new TF1("fLevy", LevyFCN, PT_LOWER, PT_UPPER, 4);
  // pp 13TeV - Levy fit with ALICE-2017
  fPt->SetParName(0, "dN/dy");
  fPt->SetParName(1, "C");
  fPt->SetParName(2, "n");
  fPt->SetParName(3, "m_{0}");
  fPt->SetParameters(7413., 7.79, 0.7053, MASS_JPSI);

  TFile* f = new TFile(output_dir+"/JpsiDecay.root", "RECREATE");
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