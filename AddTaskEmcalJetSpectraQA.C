// AddTaskEmcalJetSpectraQA.C

AliAnalysisTaskEmcalJetSpectraQA* AddTaskEmcalJetSpectraQA(
  const char *ntracks            = "usedefault",
  const char *nclusters          = "usedefault",
  Double_t    trackPtCut         = 0.15,
  Double_t    clusECut           = 0.30,
  const char *suffix             = ""
)
{
  AliAnalysisTaskEmcalJetSpectraQA* __R_ADDTASK__ = AliAnalysisTaskEmcalJetSpectraQA::AddTaskEmcalJetSpectraQA(ntracks, nclusters, trackPtCut, clusECut, suffix);

  AliJetContainer* jetContCh02 = __R_ADDTASK__->AddJetContainer(AliJetContainer::kChargedJet, AliJetContainer::antikt_algorithm, AliJetContainer::pt_scheme, 0.2, AliJetContainer::kTPCfid,  "tracks", "", "Jet");
  //jetContCh02->SetRhoName("Rho02");
  jetContCh02->SetPercAreaCut(0.6);

  __R_ADDTASK__->SetHistoType(AliAnalysisTaskEmcalJetSpectraQA::kTH2);
  __R_ADDTASK__->SetPtBin(1.,200.);
  __R_ADDTASK__->SelectCollisionCandidates(AliVEvent::kEMCEGA);
  __R_ADDTASK__->SetForceBeamType(AliAnalysisTaskEmcalLight::kpp);
  __R_ADDTASK__->SetCentralityEstimation(1);
  __R_ADDTASK__->SetCentRange(0, 100);
  __R_ADDTASK__->SetCentralityEstimator("V0M");
  __R_ADDTASK__->SetMinNVertCont(1);
  __R_ADDTASK__->SetZvertexDiffValue(0.5);
  __R_ADDTASK__->SetNeedEmcalGeom(kFALSE);

  return __R_ADDTASK__;
}
