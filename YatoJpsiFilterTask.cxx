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