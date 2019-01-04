AliAnalysisTask *AddTaskJPSIFilter_pp(
				      ULong64_t triggers=AliVEvent::kEMCEGA,
				      Bool_t storeLS = kTRUE,
				      Bool_t isMC = kFALSE){
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
  Bool_t hasMC= isMC && (AliAnalysisManager::GetAnalysisManager()->GetMCtruthEventHandler()!=0x0);
  
  //Do we run on AOD?
  Bool_t isAOD=mgr->GetInputEventHandler()->IsA()==AliAODInputHandler::Class();

  //Allow merging of the filtered aods on grid trains
  if(mgr->GetGridHandler()) {
    printf("[-] INFO - SET MERGE FILTERED AODs \n");
    mgr->GetGridHandler()->SetMergeAOD(kTRUE);
  }
  
  //Create task and add it to the analysis manager
  YatoJpsiFilterTask *task=new YatoJpsiFilterTask("jpsi2ee_EMCalFilter");
  task->SetTriggerMask(triggers);
	task->SetToMerge(kTRUE);
  if (!hasMC) task->UsePhysicsSelection();

	//Add event filter
	AliDielectronEventCuts *eventCuts = new AliDielectronEventCuts("eventCuts", "Vertex Track && |vtxZ|<10 && ncontrib>0");
	if (isAOD)
		eventCuts->SetVertexType(AliDielectronEventCuts::kVtxAny);
	eventCuts->SetRequireVertex();
	eventCuts->SetMinVtxContributors(1);
	eventCuts->SetVertexZ(-10., 10.);
	task->SetEventFilter(eventCuts);
  task->UsePhysicsSelection();

  // Add dielectron analysis
  if (!gROOT->GetListOfGlobalFunctions()->FindObject("ConfigJpsi_cj_pp")){
		gROOT->LoadMacro("ConfigJpsi_cj_pp.C");
	}
  // From trigger to trigger_index - adapt to CJ's configuration
  Int_t trigger_index = 0;
  switch (triggers)
  {
    case AliVEvent::kINT7:
      trigger_index = 0;
      break;
    case AliVEvent::kEMC7:
      trigger_index = 1;
      break;
    case AliVEvent::kEMCEGA:
      trigger_index = 2; // EMCal L1 trigger
    default:
      trigger_index = 0;
      break;
  }
  Int_t cutDefinition = 1; // kEMCal_loose
  AliDielectron *jpsi=ConfigJpsi_cj_pp(cutDefinition, kTRUE, trigger_index, hasMC);
  if(isAOD) {
    //add options to AliAODHandler to duplicate input event
    AliAODHandler *aodHandler = (AliAODHandler*)mgr->GetOutputEventHandler();
    aodHandler->SetCreateNonStandardAOD();
    aodHandler->SetNeedsHeaderReplication();
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
  containerName += ":PWGDQ_dielectron_Filter";
 
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
