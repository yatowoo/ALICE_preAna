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
#include "AliAnalysisManager.h"

#include "AliDielectronVarManager.h"
class AliDielectronVarManager;

#include "AliAnalysisTaskDielectronFilter.h"

class AliAnalysisManager;

class YatoJpsiFilterTask : public AliAnalysisTaskDielectronFilter {

public:
  YatoJpsiFilterTask();
  YatoJpsiFilterTask(const char* name);
  virtual ~YatoJpsiFilterTask(){}

  virtual void Init();

  Bool_t IsToMerge() { return fIsToMerge;}
  void SetToMerge(Bool_t isToMerge = kTRUE){ fIsToMerge = isToMerge;}

private:
  Bool_t fIsToMerge; // Option for AliAODExtension

};
#endif // #ifndef YATO_JPSI_FILTER_TASK_H
