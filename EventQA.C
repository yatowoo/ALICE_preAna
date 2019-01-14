/**
 * ROOT Script for QA on event_stat.root
**/
const unsigned short N_TC = 4;
const char* TRIGGER_CLASS[4]= {"EG1", "EG2", "DG1", "DG2"};

int EventQA(TMap* fTC){
  // Count trigger class
  TPair* tc = NULL;
  std::map<TString, Long64_t> tcCounter;
  tcCounter["EG1"] = 0;
  tcCounter["EG2"] = 0;
  tcCounter["DG1"] = 0;
  tcCounter["DG2"] = 0;
  for(TIter iter = fTC->begin(); iter != fTC->end(); iter.Next()){
    tc = (TPair*)(*iter);
    TString tcName = ((TObjString*)(tc->Key()))->String();
    TParameter<Long64_t>* tcValue = (TParameter<Long64_t>*)(tc->Value());
    if(tcName.Contains("EG1")) tcCounter["EG1"] += tcValue->GetVal();
    if(tcName.Contains("EG2")) tcCounter["EG2"] += tcValue->GetVal();
    if(tcName.Contains("DG1")) tcCounter["DG1"] += tcValue->GetVal();
    if(tcName.Contains("DG2")) tcCounter["DG2"] += tcValue->GetVal();
    cout << tcValue->GetVal() << "\t" << tcName.Data() << endl;
  }

  return 0;
}