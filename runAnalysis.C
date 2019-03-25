// Mode : local, test, full, merge, final
void runAnalysis(TString mode="local", TString work_dir="16l_Full_CJ_MB-EG1-EG2", TString datasets="16l_pass1", TString dataDir="2016/LHC16l", TString task_name="jpsiTask")
{

    // PATH for headers 
    gROOT->ProcessLine(".include $ROOTSYS/include");
    gROOT->ProcessLine(".include $ALICE_ROOT/include");
    gROOT->ProcessLine(".include $ALICE_PHYSICS/include");

    // create the analysis manager
    AliAnalysisManager *mgr = new AliAnalysisManager("PPJpsiAnalysis");

    // Configuration from LEGO train DQ_pp_AOD
    gROOT->LoadMacro("$ALICE_ROOT/ANALYSIS/macros/train/AddAODHandler.C");
    AliVEventHandler *handler = AddAODHandler();

    gROOT->LoadMacro("$ALICE_ROOT/ANALYSIS/macros/train/AddAODOutputHandler.C");
    AliVEventHandler *handler = AddAODOutputHandler();
    ((AliAODHandler*)handler)->SetFillAOD(kTRUE);

    // TASK - Basic Jet Finder task
    gROOT->LoadMacro("AddTaskEmcalJet.C");
    AliEmcalJetTask *taskJet = AddTaskEmcalJet("usedefault", "usedefault", AliJetContainer::antikt_algorithm, 0.2, AliJetContainer::kChargedJet, 0.15, 0.3, 0.005, AliJetContainer::pt_scheme, "Jet", 1., kFALSE, kFALSE);
    if(taskJet){
        taskJet->SetForceBeamType(AliAnalysisTaskEmcal::kpp);
        taskJet->SelectCollisionCandidates(AliVEvent::kEMCEGA);
				taskJet->SetTrigClass("EG1|EG2");
        taskJet->SetUseAliAnaUtils(kFALSE, kFALSE); // Disable AnalysisUtils in Run2
        taskJet->SetZvertexDiffValue(0.5);
        taskJet->SetNeedEmcalGeom(kFALSE);
        cout << "[-] INFO - Create jet finder task" << endl;
    }else{
        cout << "[X] ERROR - Fail to create jet finder task." << endl;
        exit(1);
    }
 /*   
    // TASK - Jet rho
    gROOT->LoadMacro("AddTaskRhoSparse.C");
    AliAnalysisTaskRhoSparse *taskJetRho = AddTaskRhoSparse("usedefault","usedefault", "Rho02",0.2, AliEmcalJet::kTPCfid, AliJetContainer::kChargedJet, AliJetContainer::pt_scheme, kTRUE, "","TPC", 0.0, 0.01, 0, "");
    if(taskJetRho){
        cout << "[-] INFO - Create jet spectrum task" << endl;
        taskJetRho->SetExcludeLeadJets(2);
        taskJetRho->SetOutRhoName("Rho02");
        taskJetRho->SelectCollisionCandidates(AliVEvent::kAny);
        taskJetRho->SetForceBeamType(AliAnalysisTaskEmcal::kpp);
        taskJetRho->SetUseNewCentralityEstimation(kTRUE);

        taskJetRho->SetUseAliAnaUtils(kTRUE);
        taskJetRho->SetUseSPDTrackletVsClusterBG(kTRUE);
        taskJetRho->SetZvertexDiffValue(0.5);
        taskJetRho->SetNeedEmcalGeom(kFALSE);
    }else{
        cout << "[X] ERROR - Fail to create jet rho task." << endl;
        exit(1);
    }
*/
    // TASK - Jet spectrum
    gROOT->LoadMacro("AddTaskEmcalJetSpectraQA.C");
    AliAnalysisTaskSE *taskJetSpectrum = AddTaskEmcalJetSpectraQA("usedefault", "usedefault", 0.15, 0.30, "");
    if(taskJetSpectrum){
        cout << "[-] INFO - Create jet spectrum task" << endl;
    }else{
        cout << "[X] ERROR - Fail to create jet spectrum task." << endl;
        exit(1);
    }

    if(!mgr->InitAnalysis()) return;
		AliLog::SetGlobalLogLevel(AliLog::kMaxType);
		mgr->SetDebugLevel(2);
    mgr->PrintStatus();
    mgr->SetUseProgressBar(1, 25);

    if(mode == "local") {
        // Define data input for local analysis
        TChain* chain = new TChain("aodTree");
        // add a few files to the chain (change this so that your local files are added)
        if(datasets == "16l_pass1")
            chain->Add("AliAOD_input.root");
        else
            chain->Add(datasets.Data());
        // start the analysis locally, reading the events from the tchain
        mgr->StartAnalysis("local", chain);
    } else {

        gROOT->LoadMacro("DQ_pp_AOD.C");
        DQ_pp_AOD();
        TString runlist = DATASETS[datasets];
        if(!runlist.Length()){
            cout << "[X] ERROR - Wrong datasets : " << datasets << endl;
            exit(1);
        }

        // if we want to run on grid, we create and configure the plugin
        AliAnalysisAlien *alienHandler = new AliAnalysisAlien();
        // also specify the include (header) paths on grid
        alienHandler->AddIncludePath("-I. -I$ROOTSYS/include -I$ALICE_ROOT -I$ALICE_ROOT/include -I$ALICE_PHYSICS/include");
        // make sure your source files get copied to grid
        alienHandler->SetAdditionalLibs("AddTaskJPSIFilter_pp.C AddTask_cjahnke_JPsi.C ConfigJpsi_cj_pp.C YatoJpsiFilterTask.h YatoJpsiFilterTask.cxx");
				alienHandler->SetAnalysisSource("YatoJpsiFilterTask.cxx")
        // select the aliphysics version. all other packages
        // are LOADED AUTOMATICALLY!
        // RECOMMENDATION - Keep it the same with local version
        alienHandler->SetAliPhysicsVersion("vAN-20180414-1");
        // set the Alien API version
        alienHandler->SetAPIVersion("V1.1x");
        // select the input data
        alienHandler->SetGridDataDir("/alice/data/"+dataDir);
        alienHandler->SetDataPattern("*/pass1/AOD/*AOD.root");
        // MC has no prefix, data has prefix 000
        alienHandler->SetRunPrefix("000");
        // runnumber
        alienHandler->AddRunNumber(runlist);
        // number of runs per master job
        alienHandler->SetNrunsPerMaster(1);
        // number of files per subjob
        alienHandler->SetSplitMaxInputFileNumber(50);
        // specify how many seconds your job may take
        alienHandler->SetTTL(43200); // 12 hours
        
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
        alienHandler->SetMergeAOD(kTRUE);
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
