/**
 * J/psi filter for nano AOD production
 * 
 * Impelement <merge> interface for AliAODExtension 
 * Based on AliAnalysisTaskDielectronFilter
 * 
 * By: Yìtāo WÚ <yitao@cern.ch>
*/

#ifndef YATO_JPSI_FILTER_TASK_H
#define YATO_JPSI_FILTER_TASK_H

#include "AliLog.h"

#include "AliAODHandler.h"
#include "AliAODExtension.h"

#include "AliAnalysisTaskDielectronFilter.h"

class YatoJpsiFilterTask : public AliAnalysisTaskDielectronFilter {

public:
  YatoJpsiFilterTask() : AliAnalysisTaskDielectronFilter(), fIsToMerge(kFALSE){}
  YatoJpsiFilterTask(const char* name) : AliAnalysisTaskDielectronFilter(name), fIsToMerge(kFALSE);
  virtual ~YatoJpsiFilterTask(){}

  virtual void Init(){
    // Initialization
    if (fDebug > 1) AliInfo("Init() \n");
    // require AOD handler
    AliAODHandler *aodH = (AliAODHandler*)((AliAnalysisManager::GetAnalysisManager())->GetOutputEventHandler());
    if (!aodH) AliFatal("No AOD handler. Halting.");
    aodH->AddFilteredAOD("AliAOD.Dielectron.root", "DielectronEvents", fIsToMerge);
  }

  Bool_t IsToMerge() { return fIsToMerge;}
  void SetToMerge(Bool_t isToMerge = kTRUE){ fIsToMerge = isToMerge;}

private:
  Bool_t fIsToMerge; // Option for AliAODExtension

};
#endif // #ifndef YATO_JPSI_FILTER_TASK_H