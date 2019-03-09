/**
 * J/psi filter for nano AOD production
 * 
 * Impelement <merge> interface for AliAODExtension 
 * Based on AliAnalysisTaskDielectronFilter
 * 
 * NOTICE: Test OK AliPhysics::vAN-20180414
 * 	FAILED on vAN-20181208 because of the private variables.
 *
 * By: Yìtāo WÚ <yitao@cern.ch>
*/

#ifndef YATO_JPSI_FILTER_TASK_H
#define YATO_JPSI_FILTER_TASK_H

#include "AliLog.h"

#include "AliAODHandler.h"
#include "AliAODExtension.h"
#include "AliESDInputHandler.h"
#include "AliAODInputHandler.h"
#include "AliAnalysisManager.h"
#include "AliTriggerAnalysis.h"

#include "AliDielectron.h"
class AliDielectron;
#include "AliDielectronVarManager.h"
class AliDielectronVarManager;

#include "AliAnalysisTaskDielectronFilter.h"

class AliESDInputHandler;
class AliAODInputHandler;
class AliAnalysisManager;
class AliTriggerAnalysis;

class YatoJpsiFilterTask : public AliAnalysisTaskDielectronFilter {

public:
  YatoJpsiFilterTask();
  YatoJpsiFilterTask(const char* name);
  virtual ~YatoJpsiFilterTask(){}

  virtual void Init();
  virtual void UserExec(Option_t* option);

  Bool_t IsToMerge() { return fIsToMerge;}
  void SetToMerge(Bool_t isToMerge = kTRUE){ fIsToMerge = isToMerge;}

private:
  Bool_t fIsToMerge; // Option for AliAODExtension

};
#endif // #ifndef YATO_JPSI_FILTER_TASK_H
