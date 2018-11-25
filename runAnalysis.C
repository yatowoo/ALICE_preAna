// Mode : local, test, full, merge, final
void runAnalysis(TString mode="local", TString work_dir="16l_Full_CJ_MB-EG1-EG2", TString datasets="16l_pass1", TString task_name="jpsiTask")
{
    gROOT->LoadMacro("DQ_pp_AOD.C");
    DQ_pp_AOD();
    TString runlist = DATASETS[datasets];
    if(!runlist.Length()){
        cout << "[X] ERROR - Wrong datasets : " << datasets << endl;
        exit(1);
    }

    // PATH for headers 
    gROOT->ProcessLine(".include $ROOTSYS/include");
    gROOT->ProcessLine(".include $ALICE_ROOT/include");

    // create the analysis manager
    AliAnalysisManager *mgr = new AliAnalysisManager("PPJpsiAnalysis");

    // Configuration from LEGO train DQ_pp_AOD
    gROOT->LoadMacro("$ALICE_ROOT/ANALYSIS/macros/train/AddAODHandler.C");
    AliVEventHandler *handler = AddAODHandler();

    gROOT->LoadMacro("$ALICE_ROOT/ANALYSIS/macros/train/AddAODOutputHandler.C");
    AliVEventHandler *handler = AddAODOutputHandler();
    AliAnalysisManager::SetGlobalStr("kJetDeltaAODName", "");
    AliAnalysisManager::SetGlobalInt("kFillAODForRun", 0);
    AliAnalysisManager::SetGlobalInt("kFilterAOD", 0);
    ((AliAODHandler *)handler)->SetFillAODforRun(kFALSE);
    ((AliAODHandler *)handler)->SetNeedsHeaderReplication();
    // TASK -Physics selectrion
    gROOT->LoadMacro("$ALICE_PHYSICS/OADB/macros/AddTaskPhysicsSelection.C");
    AddTaskPhysicsSelection(kFALSE, kTRUE);
    // TASK - PID response
    gROOT->LoadMacro("$ALICE_ROOT/ANALYSIS/macros/AddTaskPIDResponse.C");
    AddTaskPIDResponse();
    // TASK - PID QA
    gROOT->LoadMacro("$ALICE_ROOT/ANALYSIS/macros/AddTaskPIDqa.C");
    AddTaskPIDqa("PIDqa.root");
    // TASK - MB from C. Jahnke
    gROOT->LoadMacro("AddTask_cjahnke_JPsi.C");
    AddTask_cjahnke_JPsi("16l", 0, kFALSE, "ConfigJpsi_cj_pp", kFALSE, kTRUE, 0);
    // TASK - EG1 from C. Jahnke
    gROOT->LoadMacro("AddTask_cjahnke_JPsi.C");
    AddTask_cjahnke_JPsi("16l", 3, kFALSE, "ConfigJpsi_cj_pp", kFALSE, kTRUE, 0);
    // TASK - EG2 from C. Jahnke
    gROOT->LoadMacro("AddTask_cjahnke_JPsi.C");
    AddTask_cjahnke_JPsi("16l", 4, kFALSE, "ConfigJpsi_cj_pp", kFALSE, kTRUE, 0);

    if(!mgr->InitAnalysis()) return;
    mgr->SetDebugLevel(2);
    mgr->PrintStatus();
    mgr->SetUseProgressBar(1, 25);

    if(mode == "local") {
        // Define data input for local analysis
        TChain* chain = new TChain("aodTree");
        // add a few files to the chain (change this so that your local files are added)
        chain->Add("AliAOD_input.root");
        // start the analysis locally, reading the events from the tchain
        mgr->StartAnalysis("local", chain);
    } else {
        // if we want to run on grid, we create and configure the plugin
        AliAnalysisAlien *alienHandler = new AliAnalysisAlien();
        // also specify the include (header) paths on grid
        alienHandler->AddIncludePath("-I. -I$ROOTSYS/include -I$ALICE_ROOT -I$ALICE_ROOT/include -I$ALICE_PHYSICS/include");
        // make sure your source files get copied to grid
        alienHandler->SetAdditionalLibs("AddTask_cjahnke_JPsi.C ConfigJpsi_cj_pp.C");
        // select the aliphysics version. all other packages
        // are LOADED AUTOMATICALLY!
        // RECOMMENDATION - Keep it the same with local version
        alienHandler->SetAliPhysicsVersion("vAN-20180305-1");
        // set the Alien API version
        alienHandler->SetAPIVersion("V1.1x");
        // select the input data
        alienHandler->SetGridDataDir("/alice/data/2016/LHC16l");
        alienHandler->SetDataPattern("*/pass1/AOD/*AOD.root");
        // MC has no prefix, data has prefix 000
        alienHandler->SetRunPrefix("000");
        // runnumber
        alienHandler->AddRunNumber(runlist);
        // number of files per subjob
        alienHandler->SetSplitMaxInputFileNumber(20);
        // specify how many seconds your job may take
        alienHandler->SetTTL(43200); // Half a day
        
        // define the output folders
        alienHandler->SetGridWorkingDir(work_dir);
        alienHandler->SetGridOutputDir("OutputAOD");
        // Automatical generated files
        alienHandler->SetAnalysisMacro(task_name + ".C");
        alienHandler->SetExecutable(task_name + ".sh");
        alienHandler->SetJDLName(task_name + ".jdl");
        alienHandler->SetOutputToRunNo(kTRUE);
        alienHandler->SetKeepLogs(kTRUE);
        // merging: run with kTRUE to merge on grid
        // after re-running the jobs in SetRunMode("terminate") 
        // (see below) mode, set SetMergeViaJDL(kFALSE) 
        // to collect final results
        alienHandler->SetMaxMergeStages(1);
        if(mode == "final")
            alienHandler->SetMergeViaJDL(kFALSE);
        else
            alienHandler->SetMergeViaJDL(kTRUE);

        // connect the alien plugin to the manager
        mgr->SetGridHandler(alienHandler);
        if(mode == "test") {
            // speficy on how many files you want to run
            alienHandler->SetNtestFiles(1);
            // and launch the analysis
            alienHandler->SetRunMode("test");
        } else if(mode == "full"){
            // else launch the full grid analysis
            alienHandler->SetRunMode("full");
        } else if(mode == "merge" || mode == "final") {
            alienHandler->SetRunMode("terminate");
        } else{
            cout << "[X] Error - Unknown mode : " << mode << endl;
            exit(1);
        }
        mgr->StartAnalysis("grid");
    }
}
