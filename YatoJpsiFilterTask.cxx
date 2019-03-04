#include "YatoJpsiFilterTask.h"

ClassImp(YatoJpsiFilterTask)

YatoJpsiFilterTask::YatoJpsiFilterTask() : 
  AliAnalysisTaskDielectronFilter(),
  fIsToMerge(kFALSE)
{}

YatoJpsiFilterTask::YatoJpsiFilterTask(const char* name) : 
  AliAnalysisTaskDielectronFilter(name),
  fIsToMerge(kFALSE)
{};

void YatoJpsiFilterTask::Init(){
  // Initialization
  if (fDebug > 1) AliInfo("Init() \n");
  // require AOD handler
  AliAODHandler *aodH = (AliAODHandler*)((AliAnalysisManager::GetAnalysisManager())->GetOutputEventHandler());
  if (!aodH) AliFatal("No AOD handler. Halting.");
  aodH->AddFilteredAOD("AliAOD.Dielectron.root", "DielectronEvents", fIsToMerge);
}

// Update: copy title for primary vertex
void YatoJpsiFilterTask::UserExec(Option_t*){
	//
	// Main loop. Called for every event
	//

	if (!fDielectron)
		return;

	AliAnalysisManager *man = AliAnalysisManager::GetAnalysisManager();
	Bool_t isESD = man->GetInputEventHandler()->IsA() == AliESDInputHandler::Class();
	Bool_t isAOD = man->GetInputEventHandler()->IsA() == AliAODInputHandler::Class();

	AliInputEventHandler *inputHandler = (AliInputEventHandler *)(man->GetInputEventHandler());
	if (!inputHandler)
		return;

	if (inputHandler->GetPIDResponse())
	{
		AliDielectronVarManager::SetPIDResponse(inputHandler->GetPIDResponse());
	}
	else
	{
		AliFatal("This task needs the PID response attached to the input event handler!");
	}

	// Was event selected ?
	ULong64_t isSelected = AliVEvent::kAny;
	Bool_t isRejected = kFALSE;
	if (fSelectPhysics && inputHandler)
	{
		if ((isESD && inputHandler->GetEventSelection()) || isAOD)
		{
			isSelected = inputHandler->IsEventSelected();
			if (fExcludeTriggerMask && (isSelected & fExcludeTriggerMask))
				isRejected = kTRUE;
			if (fTriggerLogic == kAny)
				isSelected &= fTriggerMask;
			else if (fTriggerLogic == kExact)
				isSelected = ((isSelected & fTriggerMask) == fTriggerMask);
		}
	}

	//before physics selection
	fEventStat->Fill(kAllEvents);
	if (isSelected == 0 || isRejected)
	{
		PostData(2, fEventStat);
		return;
	}
	//after physics selection
	fEventStat->Fill(kSelectedEvents);

	//V0and
	if (fTriggerOnV0AND)
	{
		if (isESD)
		{
			if (!fTriggerAnalysis->IsOfflineTriggerFired(static_cast<AliESDEvent *>(InputEvent()), AliTriggerAnalysis::kV0AND))
				return;
		}
		if (isAOD)
		{
			if (!((static_cast<AliAODEvent *>(InputEvent()))->GetVZEROData()->GetV0ADecision() == AliVVZERO::kV0BB &&
						(static_cast<AliAODEvent *>(InputEvent()))->GetVZEROData()->GetV0CDecision() == AliVVZERO::kV0BB))
				return;
		}
	}

	fEventStat->Fill(kV0andEvents);

	//Fill Event histograms before the event filter
	AliDielectronHistos *h = fDielectron->GetHistoManager();

	Double_t values[AliDielectronVarManager::kNMaxValues] = {0};
	Double_t valuesMC[AliDielectronVarManager::kNMaxValues] = {0};
	if (h)
		AliDielectronVarManager::SetFillMap(h->GetUsedVars());
	else
		AliDielectronVarManager::SetFillMap(0x0);
	AliDielectronVarManager::SetEvent(InputEvent());
	AliDielectronVarManager::Fill(InputEvent(), values);
	AliDielectronVarManager::Fill(InputEvent(), valuesMC);

	Bool_t hasMC = AliDielectronMC::Instance()->HasMC();
	if (hasMC)
	{
		if (AliDielectronMC::Instance()->ConnectMCEvent())
			AliDielectronVarManager::Fill(AliDielectronMC::Instance()->GetMCEvent(), valuesMC);
	}

	if (h)
	{
		if (h->GetHistogramList()->FindObject("Event_noCuts"))
			h->FillClass("Event_noCuts", AliDielectronVarManager::kNMaxValues, values);
		if (hasMC && h->GetHistogramList()->FindObject("MCEvent_noCuts"))
			h->FillClass("Event_noCuts", AliDielectronVarManager::kNMaxValues, valuesMC);
	}

	//event filter
	if (fEventFilter)
	{
		if (!fEventFilter->IsSelected(InputEvent()))
			return;
	}
	fEventStat->Fill(kFilteredEvents);

	//pileup
	if (fRejectPileup)
	{
		if (InputEvent()->IsPileupFromSPD(3, 0.8, 3., 2., 5.))
			return;
	}
	fEventStat->Fill(kPileupEvents);

	//bz for AliKF
	Double_t bz = InputEvent()->GetMagneticField();
	AliKFParticle::SetField(bz);

	AliDielectronPID::SetCorrVal((Double_t)InputEvent()->GetRunNumber());

	fDielectron->Process(InputEvent());

	Bool_t hasCand = kFALSE;
	if (fStoreLikeSign)
		hasCand = (fDielectron->HasCandidates() || fDielectron->HasCandidatesLikeSign());
	else
		hasCand = (fDielectron->HasCandidates());

	if (fStoreRotatedPairs)
		hasCand = (hasCand || fDielectron->HasCandidatesTR());

	if (fStoreEventsWithSingleTracks)
		hasCand = (hasCand || fDielectron->GetTrackArray(0) || fDielectron->GetTrackArray(1));

	AliAODHandler *aodH = (AliAODHandler *)((AliAnalysisManager::GetAnalysisManager())->GetOutputEventHandler());
	AliAODExtension *extDielectron = aodH->GetFilteredAOD("AliAOD.Dielectron.root");
	if (hasCand)
	{
		AliAODEvent *aod = aodH->GetAOD();

		// reset bit for all tracks
		if (isAOD)
		{
			for (Int_t it = 0; it < aod->GetNumberOfTracks(); it++)
			{
				aod->GetTrack(it)->ResetBit(kIsReferenced);
				aod->GetTrack(it)->SetUniqueID(0);
			}
		}

		//replace the references of the legs with the AOD references
		TObjArray *obj = 0x0;
		for (Int_t i = 0; i < 11; i++)
		{
			obj = (TObjArray *)((*(fDielectron->GetPairArraysPointer()))->UncheckedAt(i));
			if (!obj)
				continue;
			for (int j = 0; j < obj->GetEntriesFast(); j++)
			{
				AliDielectronPair *pairObj = (AliDielectronPair *)obj->UncheckedAt(j);
				Int_t id1 = ((AliVTrack *)pairObj->GetFirstDaughterP())->GetID();
				Int_t id2 = ((AliVTrack *)pairObj->GetSecondDaughterP())->GetID();

				for (Int_t it = 0; it < aod->GetNumberOfTracks(); it++)
				{
					if (aod->GetTrack(it)->GetID() == id1)
						pairObj->SetRefFirstDaughter(aod->GetTrack(it));
					if (aod->GetTrack(it)->GetID() == id2)
						pairObj->SetRefSecondDaughter(aod->GetTrack(it));
				}
			}
		}

		extDielectron->SelectEvent();
		Int_t ncandidates = fDielectron->GetPairArray(1)->GetEntriesFast();
		if (ncandidates == 1)
			fEventStat->Fill((kNbinsEvent));
		else if (ncandidates > 1)
			fEventStat->Fill((kNbinsEvent + 1));

		//see if dielectron candidate branch exists, if not create is
		TTree *t = extDielectron->GetTree();

		if (!t->GetListOfBranches()->GetEntries() && isAOD)
			t->Branch(aod->GetList());

		if (!t->GetBranch("dielectrons"))
			t->Bronch("dielectrons", "TObjArray", fDielectron->GetPairArraysPointer());

		// store positive and negative tracks
		if (fCreateNanoAOD && isAOD)
		{
			Int_t nTracks = (fDielectron->GetTrackArray(0))->GetEntries() + (fDielectron->GetTrackArray(1))->GetEntries();
			AliAODEvent *nanoEv = extDielectron->GetAOD();
			nanoEv->GetTracks()->Clear();
			nanoEv->GetVertices()->Clear();
			nanoEv->GetCaloClusters()->Clear();

			AliAODVertex *tmp = ((static_cast<AliAODEvent *>(InputEvent()))->GetPrimaryVertex())->CloneWithoutRefs();
			tmp->SetTitle(((static_cast<AliAODEvent *>(InputEvent()))->GetPrimaryVertex())->GetTitle());
			nanoEv->AddVertex(tmp);
			AliAODVertex *tmpSpd = ((static_cast<AliAODEvent *>(InputEvent()))->GetPrimaryVertexSPD())->CloneWithoutRefs();
			nanoEv->AddVertex(tmpSpd);
			nanoEv->GetVertex(0)->SetNContributors((static_cast<AliAODEvent *>(InputEvent()))->GetPrimaryVertex()->GetNContributors());
			nanoEv->GetVertex(1)->SetNContributors((static_cast<AliAODEvent *>(InputEvent()))->GetPrimaryVertexSPD()->GetNContributors());

			AliAODHeader *header = dynamic_cast<AliAODHeader *>(nanoEv->GetHeader());
			AliAODHeader *inputHeader = dynamic_cast<AliAODHeader *>(InputEvent()->GetHeader());

			if (!header)
				AliFatal("Not a standard AOD");

			// Set header information
			SetHeaderData(inputHeader, header, values);

			// Store Eventplane information from the 2016 est. QnCorrections Framework
			// A new branch containing the q-Vectors in a list is created to store the information
			if (fStoreEventplanes)
			{
				if (AliAnalysisTaskFlowVectorCorrections *flowQnVectorTask =
								dynamic_cast<AliAnalysisTaskFlowVectorCorrections *>(man->GetTask("FlowQnVectorCorrections")))
				{
					if (flowQnVectorTask != NULL)
					{
						AliQnCorrectionsManager *flowQnVectorMgr = flowQnVectorTask->GetAliQnCorrectionsManager();
						fQnList = flowQnVectorMgr->GetQnVectorList();
						if (fQnList != NULL)
						{
							fQnList->SetName("qnVectorList");
							if (!t->GetBranch("qnVectorList"))
							{
								aodH->AddBranch("TList", &fQnList);
							}
						}
					}
				}
			}

			//______________________________________________________________________________

			for (int kj = 0; kj < (fDielectron->GetTrackArray(0))->GetEntries(); kj++)
			{
				Int_t posit = nanoEv->AddTrack((AliAODTrack *)fDielectron->GetTrackArray(0)->At(kj));
				Int_t posVtx = nanoEv->AddVertex(((AliAODTrack *)fDielectron->GetTrackArray(0)->At(kj))->GetProdVertex());
				nanoEv->GetVertex(posVtx)->ResetBit(kIsReferenced);
				nanoEv->GetVertex(posVtx)->SetUniqueID(0);
				nanoEv->GetVertex(posVtx)->RemoveDaughters();
				nanoEv->GetTrack(posit)->ResetBit(kIsReferenced);
				nanoEv->GetTrack(posit)->SetUniqueID(0);
				// calo cluster
				Int_t caloIndex = ((AliAODTrack *)fDielectron->GetTrackArray(0)->At(kj))->GetEMCALcluster();
				if (caloIndex > 0 && (static_cast<AliAODEvent *>(InputEvent()))->GetCaloCluster(caloIndex))
				{
					Int_t posCaloCls = nanoEv->AddCaloCluster(static_cast<AliAODEvent *>(InputEvent())->GetCaloCluster(caloIndex));
					nanoEv->GetTrack(posit)->SetEMCALcluster(posCaloCls);
					AliAODCaloCluster *clCls = nanoEv->GetCaloCluster(posCaloCls);
					for (int u = 0; u < clCls->GetNTracksMatched(); u++)
						clCls->RemoveTrackMatched(clCls->GetTrackMatched(u));
					nanoEv->GetCaloCluster(posCaloCls)->AddTrackMatched((AliAODTrack *)nanoEv->GetTrack(posit));
				}
				// set references for vtx
				AliAODTrack *trk = dynamic_cast<AliAODTrack *>(nanoEv->GetTrack(posit));
				if (!trk)
					AliFatal("Not a standard AOD");
				trk->SetProdVertex(nanoEv->GetVertex(posVtx));
			}

			for (int kj = 0; kj < (fDielectron->GetTrackArray(1))->GetEntries(); kj++)
			{
				Int_t negat = nanoEv->AddTrack((AliAODTrack *)fDielectron->GetTrackArray(1)->At(kj));
				Int_t negVtx = nanoEv->AddVertex(((AliAODTrack *)fDielectron->GetTrackArray(1)->At(kj))->GetProdVertex());
				nanoEv->GetVertex(negVtx)->ResetBit(kIsReferenced);
				nanoEv->GetVertex(negVtx)->SetUniqueID(0);
				nanoEv->GetVertex(negVtx)->RemoveDaughters();
				nanoEv->GetTrack(negat)->ResetBit(kIsReferenced);
				nanoEv->GetTrack(negat)->SetUniqueID(0);
				// calo cluster
				Int_t caloIndex = ((AliAODTrack *)fDielectron->GetTrackArray(1)->At(kj))->GetEMCALcluster();
				if (caloIndex > 0 && (static_cast<AliAODEvent *>(InputEvent()))->GetCaloCluster(caloIndex))
				{
					Int_t negCaloCls = nanoEv->AddCaloCluster(static_cast<AliAODEvent *>(InputEvent())->GetCaloCluster(caloIndex));
					nanoEv->GetTrack(negat)->SetEMCALcluster(negCaloCls);
					AliAODCaloCluster *clCls = nanoEv->GetCaloCluster(negCaloCls);
					for (int u = 0; u < clCls->GetNTracksMatched(); u++)
						clCls->RemoveTrackMatched(clCls->GetTrackMatched(u));
					nanoEv->GetCaloCluster(negCaloCls)->AddTrackMatched((AliAODTrack *)nanoEv->GetTrack(negat));
				}
				AliAODTrack *trk = dynamic_cast<AliAODTrack *>(nanoEv->GetTrack(negat));
				if (!trk)
					AliFatal("Not a standard AOD");
				trk->SetProdVertex(nanoEv->GetVertex(negVtx));
			}
			delete tmp;
			delete tmpSpd;
			nanoEv->GetTracks()->Expand(nTracks);
			nanoEv->GetVertices()->Expand(nTracks + 2);
			nanoEv->GetCaloClusters()->Expand(nanoEv->GetNumberOfCaloClusters());
		}
		if (isAOD)
			t->Fill();
	}

	if (fCreateNanoAOD && isAOD && (!hasCand) && fStoreHeader)
	{
		// set event plane
		AliAODHeader *header = dynamic_cast<AliAODHeader *>(extDielectron->GetAOD()->GetHeader());
		if (!header)
			AliFatal("Not a standard AOD");
		header->SetEventplane(((AliAODHeader *)(static_cast<AliAODEvent *>(InputEvent()))->GetHeader())->GetEventplaneP());
		header->ResetEventplanePointer();
		extDielectron->GetTree()->Fill(); // fill header for all events without tracks
	}

	PostData(1, const_cast<THashList *>(fDielectron->GetHistogramList()));
	PostData(2, fEventStat);
	return;
} 
