AliAnalysisTask *AddTaskJPSIFilter_pp(TString cfg="ConfigJpsi_nano_pp.C",
				      ULong64_t triggers=AliVEvent::kINT7,
				      TString period="LHC16l",
				      Bool_t storeLS = kTRUE,
				      Bool_t hasMC_aod = kFALSE){
  //get the current analysis manager
  AliAnalysisManager *mgr = AliAnalysisManager::GetAnalysisManager();
  if (!mgr) {
    Error("AddTaskJPSIFilter", "No analysis manager found.");
    return 0;
  }
  
  //check for output aod handler
  if (!mgr->GetOutputEventHandler()||mgr->GetOutputEventHandler()->IsA()!=AliAODHandler::Class()) {
    Warning("AddTaskJPSIFilter","No AOD output handler available. Not adding the task!");
    return 0;
  }

  //Do we have an MC handler?
  Bool_t hasMC=(AliAnalysisManager::GetAnalysisManager()->GetMCtruthEventHandler()!=0x0)||hasMC_aod;
  
  //Do we run on AOD?
  Bool_t isAOD=mgr->GetInputEventHandler()->IsA()==AliAODInputHandler::Class();

  //Allow merging of the filtered aods on grid trains
  if(mgr->GetGridHandler()) {
    printf(" SET MERGE FILTERED AODs \n");
    //mgr->GetGridHandler()->SetMergeAOD(kTRUE);
  }
  
  //Create task and add it to the analysis manager
  AliAnalysisTaskDielectronFilter *task=new AliAnalysisTaskDielectronFilter("jpsi_DielectronFilter");
  task->SetTriggerMask(triggers);
  if (!hasMC) task->UsePhysicsSelection();

	//Add event filter
	AliDielectronEventCuts *eventCuts = new AliDielectronEventCuts("eventCuts", "Vertex Track && |vtxZ|<10 && ncontrib>0");
	if (isAOD)
		eventCuts->SetVertexType(AliDielectronEventCuts::kVtxAny);
	eventCuts->SetRequireVertex();
	eventCuts->SetMinVtxContributors(1);
	eventCuts->SetVertexZ(-10., 10.);
	task->SetEventFilter(eventCuts);

  // Add dielectron analysis
  if (!gROOT->GetListOfGlobalFunctions()->FindObject("ConfigJpsi_nano_PbPb")){
		gROOT->LoadMacro(cfg.Data());
    isFilter = kTRUE;
	}
  // From trigger to trigger_index - adapt to CJ's configuration
  Int_t trigger_index = 0;
  switch (triggers)
  {
    case AliVEvent::kINT7:
      trigger_index = 100;
      break;
    case AliVEvent::kEMCEGA:
      trigger_index = 4; // for EG2, also cover all EG1?
    default:
      trigger_index = 0;
      break;
  }
  Int_t cutDefinition = 1; // Default EMCal from CJ's LEGO train
  AliDielectron *jpsi=ConfigJpsi_cj_pp(cutDefinition, kTRUE, trigger_index, hasMC, 0);
  if(isAOD) {
    //add options to AliAODHandler to duplicate input event
    AliAODHandler *aodHandler = (AliAODHandler*)mgr->GetOutputEventHandler();
    aodHandler->SetCreateNonStandardAOD();
    aodHandler->SetNeedsHeaderReplication();
    if(!period.Contains("LHC10h"))
      aodHandler->SetNeedsTOFHeaderReplication();
    aodHandler->SetNeedsVZEROReplication();
    if(hasMC)
      aodHandler->SetNeedsMCParticlesBranchReplication();
    jpsi->SetHasMC(hasMC);
  }
  task->SetDielectron(jpsi);
  task->SetCreateNanoAODs(kTRUE);
  task->SetStoreHeader(kFALSE);
  task->SetStoreLikeSignCandidates(storeLS);
  task->SetStoreEventsWithSingleTracks(kFALSE);
  mgr->AddTask(task);

  //----------------------
  //create data containers
  //----------------------
  
  
  TString containerName = mgr->GetCommonFileName();
  containerName += ":PWGDQ_dielectronFilter";
 
  //create output container
  
  AliAnalysisDataContainer *cOutputHist1 =
    mgr->CreateContainer("jpsi_FilterQA",
                         THashList::Class(),
                         AliAnalysisManager::kOutputContainer,
                         containerName.Data());
  
  AliAnalysisDataContainer *cOutputHist2 =
    mgr->CreateContainer("jpsi_FilterEventStat",
                         TH1D::Class(),
                         AliAnalysisManager::kOutputContainer,
                         containerName.Data());
  
  
  mgr->ConnectInput(task,  0, mgr->GetCommonInputContainer());
  mgr->ConnectOutput(task, 0, mgr->GetCommonOutputContainer());
  mgr->ConnectOutput(task, 1, cOutputHist1);
  mgr->ConnectOutput(task, 2, cOutputHist2);
  
  return task;
}
