// Toy MC for J/psi decay
#define DEBUG

// Require ROOT6
#include "TLorentzVector.h"

// Use Float_t instead of double to save disk size
const Float_t MASS_JPSI = 3.096; // GeV/c^2
const Float_t MASS_ELECTRON = 511e-6; // GeV/c^2

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

int JpsiToyMC(){
  gRandom->SetSeed(time(NULL));

  // Au-Au 200 GeV/c
  TF1 *fPt = new TF1("fPt", "(3.4948*TMath::Power(TMath::Exp((-0.395305)*x)+x/2.91793,(-8.46161)))*4*TMath::Pi()*x", 0, 20);

  TLorentzVector jpsi = getJpsi(fPt, 0., 10.);
  TLorentzVector eplus = twoBodyDecay(jpsi, MASS_ELECTRON);
  TLorentzVector eminus = jpsi - eplus;
#ifdef DEBUG
  jpsi.Print();
  eplus.Print();
  eminus.Print();
#endif // DEBUG
  return 0;
}