//===============================================================================================================
// addtask for J/psi - hadron correlation analysis in pp 13TeV (last updated: 20/03/2018)
//===============================================================================================================

// MC signals, in order of appearance in AddTask_laltenka_dst_correlations.C
enum MCFilters {
  kJpsiInclusive=0,
  kJpsiNonPrompt,
  kJpsiPrompt,
  kJpsiRadiative,
  kJpsiNonRadiative,
  kJpsiNonPromptRadiative,
  kJpsiNonPromptNonRadiative,
  kJpsiDecayElectron,
  kJpsiNonPromptDecayElectron,
  kJpsiPromptDecayElectron,
  kJpsiRadiativeDecayElectron,
  kJpsiNonRadiativeDecayElectron,
  kJpsiDecayPhoton
};

// bins for mixing variables
const Int_t gNVtxZLimits              = 21;
Double_t    gVtxZLimits[gNVtxZLimits] = {-10.,-9.,-8.,7.,-6.,-5.,-4.,-3.,-2.,-1.,0.,1.,2.,3.,4.,5.,6.,7.,8.,9.,10.};
const Int_t gNMultLimits              = 11;
Double_t    gMultLimits[gNMultLimits] = {1, 15, 25, 35, 45, 55, 65, 80, 110, 150, 200};

// forward declaration
TString SetRunNumbers(TString prod="");
void SetEventCuts(AliReducedAnalysisJpsi2eeCorrelations* task, Int_t triggerChoice=0, TString prod="");
void SetJpsiElectronTrackCuts(AliReducedAnalysisJpsi2eeCorrelations* task, TString prod="");
void SetAssociatedTrackCuts(AliReducedAnalysisJpsi2eeCorrelations* task, TString prod="");
void SetTrackPrefilterCuts(AliReducedAnalysisJpsi2eeCorrelations* task, TString prod="");
void SetPairCuts(AliReducedAnalysisJpsi2eeCorrelations* task, TString prod="");
void SetPairPrefilterCuts(AliReducedAnalysisJpsi2eeCorrelations* task, TString prod="");
void SetMCSignalCutsLeg(AliReducedAnalysisJpsi2eeCorrelations* task, TString prod="");
void SetMCSignalCutsJPsi(AliReducedAnalysisJpsi2eeCorrelations* task, TString prod="");
void SetupMixingHandlers(AliReducedAnalysisJpsi2eeCorrelations* task, TString prod ="");
void SetupHistogramManager(AliReducedAnalysisJpsi2eeCorrelations* task, TString prod="", TString runNumbers="");
void DefineHistograms(AliReducedAnalysisJpsi2eeCorrelations* task, TString prod="", TString runNumbers="");

//_______________________________________________________________________________________________________________
AliAnalysisTask* AddTask_laltenka_jpsi2ee_correlations(Bool_t isAliRoot=kTRUE, Int_t runMode=1, TString prodString="LHC16l") {

  //
  // settings:
  //  - isAliroot = kTRUE for ESD/AOD analysis, = KFALSE for reduced trees
  //  - runMode = 1 for AliAnalysisTaskReducedEventProcessor::kUseOnTheFlyReducedEvents, = 2 for AliAnalysisTaskReducedEventProcessor::kUseEventsFromTree
  //
  printf("INFO on AddTask_laltenka_jpsi2ee_correlations(): (isAliRoot, runMode) :: (%d,%d)\n", isAliRoot, runMode);

  // settings according to production
  //-----------------------------------------------------------------------------------------------------------
  TObjArray*  prodStringArr = prodString.Tokenize("_");
  TString     prod          = prodStringArr->At(0)->GetName();
  // MC choice
  Bool_t                        setMC = kFALSE;
  if (prod.Contains("LHC17h2")) setMC = kTRUE;
  // trigger choice
  // 0 -> AliReducedVarManager::kINT7 | AliReducedVarManager::kHighMultV0
  // 1 -> AliReducedVarManager::kINT7
  // 2 -> AliReducedVarManager::kHighMultV0
  // 3 -> AliReducedVarManager::kEMCEJE | AliReducedVarManager::kEMCEGA
  Int_t   triggerChoice = 0;
  TString triggerString = "";
  if (prodStringArr->GetEntries()>1) {
    triggerString = prodStringArr->At(1)->GetName();
    if (!triggerString.CompareTo("MB"))       triggerChoice = 1;
    if (!triggerString.CompareTo("highMult")) triggerChoice = 2;
    if (!triggerString.CompareTo("EMC"))      triggerChoice = 3;
  }
  printf("INFO on AddTask_laltenka_jpsi2ee_correlations(): trigger choice = %d (%s)\n", triggerChoice, triggerString.Data());

  // initialize analysis task
  //-----------------------------------------------------------------------------------------------------------
  AliReducedAnalysisJpsi2eeCorrelations* jpsi2eeCorrAnalysis = new AliReducedAnalysisJpsi2eeCorrelations("Jpsi2eeCorrelationAnalysis","Jpsi->ee - hadron correlation analysis");
  jpsi2eeCorrAnalysis->Init();
  // ----------------------------------------------------------------------------------
  // NOTE: At some point these options can be moved in Init() since they all refer to the 
  //  jpsi2ee task and are likely not to be changed often.
  //  They can be anyway overriden
  jpsi2eeCorrAnalysis->SetLoopOverTracks(kTRUE);
  jpsi2eeCorrAnalysis->SetRunEventMixing(kTRUE);
  jpsi2eeCorrAnalysis->SetRunLikeSignPairing(kTRUE);
  jpsi2eeCorrAnalysis->SetRunPairing(kTRUE);
  jpsi2eeCorrAnalysis->SetRunPrefilter(kTRUE);
  jpsi2eeCorrAnalysis->SetStoreJpsiCandidates(kTRUE);
  // -----------------------------------------------------------------------------------
  jpsi2eeCorrAnalysis->SetRunCorrelation(kTRUE);
  jpsi2eeCorrAnalysis->SetRunCorrelationMixing(kTRUE);
  jpsi2eeCorrAnalysis->SetRunOverMC(setMC);

  // set run numbers for runwise histograms
  //-----------------------------------------------------------------------------------------------------------
  TString runNumbers = SetRunNumbers(prod);
  
  // set analysis and prefilter cuts
  //-----------------------------------------------------------------------------------------------------------
  SetEventCuts(jpsi2eeCorrAnalysis, triggerChoice, prod);
  SetJpsiElectronTrackCuts(jpsi2eeCorrAnalysis, prod);
  SetAssociatedTrackCuts(jpsi2eeCorrAnalysis, prod);
  SetTrackPrefilterCuts(jpsi2eeCorrAnalysis, prod);
  SetPairCuts(jpsi2eeCorrAnalysis, prod);
  SetPairPrefilterCuts(jpsi2eeCorrAnalysis, prod);

  // set MC signal selection
  //-----------------------------------------------------------------------------------------------------------
  Bool_t isMC = jpsi2eeCorrAnalysis->GetRunOverMC();
  if (isMC) {
    SetMCSignalCutsLeg(jpsi2eeCorrAnalysis, prod);
    SetMCSignalCutsJPsi(jpsi2eeCorrAnalysis, prod);
  }

  // define histograms histograms
  //-----------------------------------------------------------------------------------------------------------
  SetupHistogramManager(jpsi2eeCorrAnalysis, prod, runNumbers);

  // mixing handler
  //-----------------------------------------------------------------------------------------------------------
  SetupMixingHandlers(jpsi2eeCorrAnalysis, prod);

  // initialize wrapper AliAnalysisTask
  // (in order to run AliReducedAnalysisJpsi2eeCorrelations in an aliroot analysis train )
  //-----------------------------------------------------------------------------------------------------------
  AliAnalysisTaskReducedEventProcessor* task = new AliAnalysisTaskReducedEventProcessor("ReducedEventAnalysisManager", runMode);
  task->AddTask(jpsi2eeCorrAnalysis);
  
  // intercept isAliRoot=kFALSE (nothing to be done yet)
  //-----------------------------------------------------------------------------------------------------------
  if (!isAliRoot) return 0;

  // get analysis manager
  //-----------------------------------------------------------------------------------------------------------
  AliAnalysisManager *mgr = AliAnalysisManager::GetAnalysisManager();
  if (!mgr) { Error("AddTask_laltenka_jpsi2ee_correlations", "No analysis manager found."); return 0; }

  // get data container
  //-----------------------------------------------------------------------------------------------------------
  AliAnalysisDataContainer* cReducedEvent = NULL;
  if (runMode==AliAnalysisTaskReducedEventProcessor::kUseOnTheFlyReducedEvents) {
    printf("INFO on AddTask_laltenka_jpsi2ee_correlations(): use on the fly events\n");
    cReducedEvent = (AliAnalysisDataContainer*)mgr->GetContainers()->FindObject("ReducedEventDQ");
    if (!cReducedEvent) { printf("ERROR: In AddTask_laltenka_jpsi2ee_correlations(), ReducedEvent exchange container could not be found!\n"); return 0x0; }
  }

  // add task to analysis manager
  //-----------------------------------------------------------------------------------------------------------
  mgr->AddTask(task);

  // connect input data containers
  //-----------------------------------------------------------------------------------------------------------
  if (runMode==AliAnalysisTaskReducedEventProcessor::kUseEventsFromTree)              mgr->ConnectInput(task, 0, mgr->GetCommonInputContainer());
  else if (runMode==AliAnalysisTaskReducedEventProcessor::kUseOnTheFlyReducedEvents)  mgr->ConnectInput(task, 0, cReducedEvent);
  else { printf("ERROR: In AddTask_laltenka_jpsi2ee_correlations(), runMode %d not defined!\n", runMode); return 0; }

  // connect output data containers
  //-----------------------------------------------------------------------------------------------------------
  TString                           outputName = "dstCorrelationsAnalysisHistograms.root";
  if (triggerString.CompareTo(""))  outputName = Form("dstCorrelationsAnalysisHistograms_%s.root", triggerString.Data());
  if (isMC)                         outputName.ReplaceAll(".root", "_MC.root");
  printf("INFO on AddTask_laltenka_jpsi2ee_correlations(): output container name = %s\n", outputName.Data());
  AliAnalysisDataContainer *cOutputHist = mgr->CreateContainer("jpsi2eeHistos", THashList::Class(), AliAnalysisManager::kOutputContainer, outputName);
  mgr->ConnectOutput(task, 1, cOutputHist);
  
  // done
  //-----------------------------------------------------------------------------------------------------------
  return task;
}

//_______________________________________________________________________________________________________________
TString SetRunNumbers(TString prod /*=""*/) {
  
  TString runNumbers = "";
  // 2016 data
  if (prod.Contains("LHC16d") || prod.Contains("LHC17h2a")) {
    // 10 runs: runList_LHC16d_pass1_CentralBarrelTracking_20170531_v0
    runNumbers = "252330;252326;252325;252322;252319;252317;252310;252271;252248;252235";
  } else if (prod.Contains("LHC16e") || prod.Contains("LHC17h2b")) {
    // 11 runs: runList_LHC16e_pass1_CentralBarrelTracking_20170626_v0
    runNumbers = "253591;253589;253563;253530;253529;253517;253488;253482;253481;253478;253437";
  } else if (prod.Contains("LHC16g") || prod.Contains("LHC17h2d")) {
    // 18 runs: runList_LHC16g_pass1_CentralBarrelTracking_20170502_v0
    runNumbers = "254332;254331;254330;254304;254303;254302;254293;254205;254204;254199;254193;254178;254175;254174;254149;254148;254147;254128";
  } else if (prod.Contains("LHC16h") || prod.Contains("LHC17h2e")) {
    // 68 runs: runList_LHC16h_pass1_CentralBarrelTracking_20170531_v0
    runNumbers = "255467;255466;255465;255463;255447;255442;255440;255421;255420;255419;255418;255415;255407;255402;255398;255352;255351;255350;255283;255280;255276;255275;255256;255255;255253;255252;255251;255249;255248;255247;255242;255240;255182;255181;255180;255177;255176;255174;255173;255171;255167;255162;255159;255154;255111;255091;255086;255085;255082;255079;254984;254983;254654;254653;254652;254651;254649;254648;254646;254644;254640;254632;254630;254629;254621;254608;254606;254604";
  } else if (prod.Contains("LHC16i") || prod.Contains("LHC17h2f")) {
    // 14 runs: runList_LHC16i_pass1_CentralBarrelTracking_20170330_v0
    runNumbers = "255618;255617;255616;255615;255614;255591;255583;255582;255577;255543;255542;255541;255540;255539";
  } else if (prod.Contains("LHC16j") || prod.Contains("LHC17h2g")) {
    // 44 runs: runList_LHC16j_pass1_CentralBarrelTracking_20170518_v0
    runNumbers = "256420;256418;256417;256415;256373;256372;256371;256368;256366;256365;256364;256363;256362;256361;256357;256356;256311;256309;256307;256302;256299;256298;256297;256295;256292;256290;256289;256287;256284;256283;256282;256281;256231;256228;256227;256223;256222;256219;256215;256213;256212;256210;256207;256204";
  } else if (prod.Contains("LHC16k") || prod.Contains("LHC17h2h")) {
    // 202 runs: runList_LHC16k_pass1_CentralBarrelTracking_20170516_v3
    runNumbers = "258537;258499;258477;258456;258454;258452;258426;258393;258391;258387;258359;258336;258332;258307;258306;258303;258302;258301;258299;258278;258274;258273;258271;258270;258258;258257;258256;258204;258203;258202;258198;258197;258178;258117;258114;258113;258109;258108;258107;258063;258062;258060;258059;258053;258049;258048;258045;258042;258041;258039;258019;258017;258014;258012;258008;258003;257992;257989;257986;257979;257963;257960;257958;257957;257939;257937;257936;257893;257892;257855;257853;257851;257850;257804;257803;257800;257799;257798;257797;257773;257765;257757;257754;257737;257735;257734;257733;257727;257725;257724;257697;257694;257692;257691;257689;257688;257687;257685;257684;257682;257644;257642;257636;257635;257632;257630;257606;257605;257604;257601;257595;257594;257592;257590;257588;257587;257566;257562;257561;257560;257541;257540;257539;257537;257531;257530;257492;257491;257490;257488;257487;257474;257468;257457;257433;257364;257358;257330;257322;257320;257318;257260;257224;257209;257206;257204;257145;257144;257142;257141;257140;257139;257138;257137;257136;257100;257095;257092;257086;257084;257083;257082;257080;257077;257028;257026;257021;257012;257011;256944;256942;256941;256697;256695;256694;256692;256691;256684;256681;256677;256676;256658;256620;256619;256592;256591;256589;256567;256565;256564;256562;256561;256560;256557;256556;256554;256552;256514;256512;256510;256506;256504";
  } else if (prod.Contains("LHC16l") || prod.Contains("LHC17h2i")) {
    // 70 runs: runList_LHC16l_pass1_CentralBarrelTracking_20170509_v2
    runNumbers = "259888;259868;259867;259866;259860;259842;259841;259822;259789;259788;259781;259756;259752;259751;259750;259748;259747;259713;259711;259705;259704;259703;259700;259697;259668;259650;259649;259477;259473;259396;259395;259394;259389;259388;259382;259378;259342;259341;259340;259339;259336;259334;259307;259305;259303;259302;259274;259273;259272;259271;259270;259269;259264;259263;259261;259257;259204;259164;259162;259118;259117;259099;259096;259091;259090;259088;258964;258962;258923;258919";
  } else if (prod.Contains("LHC16o") || prod.Contains("LHC17h2j")) {
    // 74 runs: runList_LHC16o_pass1_CentralBarrelTracking_20170425_v1
    runNumbers = "264035;264033;263985;263984;263981;263978;263977;263923;263920;263917;263916;263905;263866;263863;263861;263810;263803;263793;263792;263790;263787;263786;263785;263784;263744;263743;263741;263739;263738;263737;263691;263690;263689;263682;263663;263662;263657;263654;263653;263652;263647;263529;263497;263496;263490;263487;263332;263331;262858;262855;262853;262849;262847;262844;262842;262841;262778;262777;262776;262768;262760;262727;262725;262723;262719;262717;262713;262708;262706;262705;262428;262426;262425;262424";
  } else if (prod.Contains("LHC16p") || prod.Contains("LHC17h2k")) {
    // 42 runs: runList_LHC16p_pass1_CentralBarrelTracking_20170602_v0
    runNumbers = "264347;264346;264345;264341;264336;264312;264306;264305;264281;264279;264277;264273;264267;264266;264265;264264;264262;264261;264260;264259;264238;264235;264233;264232;264198;264197;264194;264190;264188;264168;264164;264139;264138;264137;264129;264110;264109;264086;264085;264082;264078;264076";
  } else {
    cout << "WARNING in AliReducedAnalysisJpsi2eeCorrelations::SetRunNumbers(), production " << prod.Data() << " not known, run numbers not defined!" << endl;
  }
  
  AliReducedVarManager::SetRunNumbers(runNumbers);
  return runNumbers;
}

//_______________________________________________________________________________________________________________
void SetEventCuts(AliReducedAnalysisJpsi2eeCorrelations* task, Int_t triggerChoice /*=0*/, TString prod /*=""*/) {
  AliReducedEventCut* eventCut = new AliReducedEventCut("EventCut","Event selection");
  if (triggerChoice==0)       eventCut->AddEventTriggerFilter(AliReducedVarManager::kINT7 | AliReducedVarManager::kHighMultV0); // MB + high mult.
  else if (triggerChoice==1)  eventCut->AddEventTriggerFilter(AliReducedVarManager::kINT7);                                     // MB
  else if (triggerChoice==2)  eventCut->AddEventTriggerFilter(AliReducedVarManager::kHighMultV0);                               // high mult.
  else if (triggerChoice==3)  eventCut->AddEventTriggerFilter(AliReducedVarManager::kEMCEJE | AliReducedVarManager::kEMCEGA);   // EMCal
  else                        eventCut->AddEventTriggerFilter(AliReducedVarManager::kINT7 | AliReducedVarManager::kHighMultV0 | AliReducedVarManager::kEMCEJE | AliReducedVarManager::kEMCEGA); // all
  eventCut->AddCut(AliReducedVarManager::kVtxZ, -10.0, 10.0);
  if (!task->GetRunOverMC()) eventCut->AddCut(AliReducedVarManager::kIsPhysicsSelection, 0.1, 2.);  // request physics selection
  //eventCut->AddCut(static_cast<AliReducedVarManager::Variables>(AliReducedVarManager::kIRIntClosestIntMap+1), 0.99, 5000., kTRUE);  // exclude out-of-bunch pileup,
  //eventCut->AddCut(AliReducedVarManager::kTZEROpileup, -0.1, 0.1);                                                                  // NOTE: doesn't do anything (at least in LHC16d)
  // if(event->IsPileupFromSPD(3,0.8,3.,2.,5.)) fReducedEvent->fEventTag |= (ULong64_t(1)<<9);  -> DPG proposed for MB (https://twiki.cern.ch/twiki/bin/view/ALICE/AliDPGtoolsPileup)
  // if(event->IsPileupFromSPD(5,0.8,3.,2.,5.)) fReducedEvent->fEventTag |= (ULong64_t(1)<<11); -> DPG proposed for HM (https://twiki.cern.ch/twiki/bin/view/ALICE/AliDPGtoolsPileup)
  eventCut->AddEventTagFilterBit(1, kTRUE); // reject multi-vertexer pileup events, min. weight. dist. 15
  //eventCut->AddEventTagFilterBit(4, kTRUE);   // reject multi-vertexer pileup events, min. weight. dist. 5
  task->AddEventCut(eventCut);
}

//_______________________________________________________________________________________________________________
void SetJpsiElectronTrackCuts(AliReducedAnalysisJpsi2eeCorrelations* task, TString prod /*=""*/) {
  // standard cut
  AliReducedTrackCut* standardCut = new AliReducedTrackCut("standardElectron","Electron selection standard");
  standardCut->SetTrackFilterBit(0);      // NOTE: The filter bits in the track cut class are 0-based -> they will be then applied to tracks with an offset of 32
                                          //  0 - these are electrons in my trees
  standardCut->AddCut(AliReducedVarManager::kPt, 1.0,30.0);
  standardCut->AddCut(AliReducedVarManager::kEta, -0.9,0.9);
  standardCut->AddCut(AliReducedVarManager::kDcaXY, -1.0,1.0);
  standardCut->AddCut(AliReducedVarManager::kDcaZ, -3.0,3.0);
  standardCut->AddCut(AliReducedVarManager::kTPCncls, 70.,160.0);
  standardCut->SetRejectKinks();
  standardCut->SetRequestITSrefit();
  standardCut->SetRequestTPCrefit();
  standardCut->SetRequestSPDany();
  standardCut->AddCut(AliReducedVarManager::kTPCchi2, 0., 4.0);
  standardCut->AddCut(AliReducedVarManager::kITSchi2, 0., 30.);
  standardCut->AddCut(AliReducedVarManager::kTPCNclusBitsFired, 6., 9.);
  standardCut->AddCut(AliReducedVarManager::kTPCnclsSharedRatio, 0.3, 2., kTRUE);
  standardCut->AddCut(AliReducedVarManager::kTPCcrossedRowsOverFindableClusters, 0.8, 2.);
  //standardCut->AddCut(AliReducedVarManager::kChi2TPCConstrainedVsGlobal, 36., 1.0e+10, kTRUE);
  standardCut->AddCut(static_cast<AliReducedVarManager::Variables>(AliReducedVarManager::kTPCnSig+AliReducedVarManager::kElectron), -3.0, 3.0);
  standardCut->AddCut(static_cast<AliReducedVarManager::Variables>(AliReducedVarManager::kTPCnSig+AliReducedVarManager::kProton), 3.5, 30000.0);
  standardCut->AddCut(static_cast<AliReducedVarManager::Variables>(AliReducedVarManager::kTPCnSig+AliReducedVarManager::kPion), 3.5, 30000.0);
  task->AddTrackCut(standardCut);

  // standard cut
  AliReducedTrackCut* standardCut1 = new AliReducedTrackCut("standardElectron1","Electron selection standard");
  standardCut1->SetTrackFilterBit(0);      // NOTE: The filter bits in the track cut class are 0-based -> they will be then applied to tracks with an offset of 32
  standardCut1->AddCut(AliReducedVarManager::kPt, 1.0,30.0);
  standardCut1->AddCut(AliReducedVarManager::kEta, -0.9,0.9);
  standardCut1->AddCut(AliReducedVarManager::kDcaXY, -1.0,1.0);
  standardCut1->AddCut(AliReducedVarManager::kDcaZ, -3.0,3.0);
  standardCut1->AddCut(AliReducedVarManager::kTPCncls, 70.,160.0);
  standardCut1->SetRejectKinks();
  standardCut1->SetRequestITSrefit();
  standardCut1->SetRequestTPCrefit();
  standardCut1->SetRequestSPDany();
  standardCut1->AddCut(AliReducedVarManager::kTPCchi2, 0., 4.0);
  standardCut1->AddCut(AliReducedVarManager::kITSchi2, 0., 30.);
  standardCut1->AddCut(AliReducedVarManager::kTPCNclusBitsFired, 6., 9.);
  standardCut1->AddCut(AliReducedVarManager::kTPCnclsSharedRatio, 0.3, 2., kTRUE);
  standardCut1->AddCut(AliReducedVarManager::kTPCcrossedRowsOverFindableClusters, 0.8, 2.);
  //standardCut1->AddCut(AliReducedVarManager::kChi2TPCConstrainedVsGlobal, 36., 1.0e+10, kTRUE);
  standardCut1->AddCut(static_cast<AliReducedVarManager::Variables>(AliReducedVarManager::kTPCnSig+AliReducedVarManager::kElectron), -3.0, 3.0);
  standardCut1->AddCut(static_cast<AliReducedVarManager::Variables>(AliReducedVarManager::kTPCnSig+AliReducedVarManager::kProton), 3.5, 30000.0);
  standardCut1->AddCut(static_cast<AliReducedVarManager::Variables>(AliReducedVarManager::kTPCnSig+AliReducedVarManager::kPion), 3.5, 30000.0);
  task->AddTrackCut(standardCut1);

  // standard cut
  AliReducedTrackCut* standardCut2 = new AliReducedTrackCut("standardElectron2","Electron selection standard");
  standardCut2->SetTrackFilterBit(0);      // NOTE: The filter bits in the track cut class are 0-based -> they will be then applied to tracks with an offset of 32
  standardCut2->AddCut(AliReducedVarManager::kPt, 1.0,30.0);
  standardCut2->AddCut(AliReducedVarManager::kEta, -0.9,0.9);
  standardCut2->AddCut(AliReducedVarManager::kDcaXY, -1.0,1.0);
  standardCut2->AddCut(AliReducedVarManager::kDcaZ, -3.0,3.0);
  standardCut2->AddCut(AliReducedVarManager::kTPCncls, 70.,160.0);
  standardCut2->SetRejectKinks();
  standardCut2->SetRequestITSrefit();
  standardCut2->SetRequestTPCrefit();
  standardCut2->SetRequestSPDany();
  standardCut2->AddCut(AliReducedVarManager::kTPCchi2, 0., 4.0);
  standardCut2->AddCut(AliReducedVarManager::kITSchi2, 0., 30.);
  standardCut2->AddCut(AliReducedVarManager::kTPCNclusBitsFired, 6., 9.);
  standardCut->AddCut(AliReducedVarManager::kTPCnclsSharedRatio, 0.3, 2., kTRUE);
  standardCut2->AddCut(AliReducedVarManager::kTPCcrossedRowsOverFindableClusters, 0.8, 2.);
  //standardCut2->AddCut(AliReducedVarManager::kChi2TPCConstrainedVsGlobal, 36., 1.0e+10, kTRUE);
  standardCut2->AddCut(static_cast<AliReducedVarManager::Variables>(AliReducedVarManager::kTPCnSig+AliReducedVarManager::kElectron), -3.0, 3.0);
  standardCut2->AddCut(static_cast<AliReducedVarManager::Variables>(AliReducedVarManager::kTPCnSig+AliReducedVarManager::kProton), 3.5, 30000.0);
  standardCut2->AddCut(static_cast<AliReducedVarManager::Variables>(AliReducedVarManager::kTPCnSig+AliReducedVarManager::kPion), 3.5, 30000.0);
  task->AddTrackCut(standardCut2);

  // open cut
  AliReducedTrackCut* openCut = new AliReducedTrackCut("openElectron","Electron selection open");
  openCut->SetTrackFilterBit(0);
  openCut->AddCut(AliReducedVarManager::kPt, 1.0,30.0);
  openCut->AddCut(AliReducedVarManager::kEta, -0.9,0.9);
  openCut->AddCut(AliReducedVarManager::kDcaXY, -1.0,1.0);
  openCut->AddCut(AliReducedVarManager::kDcaZ, -3.0,3.0);
  openCut->AddCut(AliReducedVarManager::kTPCncls, 70.,160.0);
  openCut->SetRejectKinks();
  openCut->SetRequestITSrefit();
  openCut->SetRequestTPCrefit();
  openCut->SetRequestSPDany();
  openCut->AddCut(AliReducedVarManager::kTPCchi2, 0., 4.0);
  openCut->AddCut(AliReducedVarManager::kITSchi2, 0., 30.);
  openCut->AddCut(AliReducedVarManager::kTPCNclusBitsFired, 6., 9.);
  openCut->AddCut(AliReducedVarManager::kTPCnclsSharedRatio, 0.3, 2., kTRUE);
  openCut->AddCut(AliReducedVarManager::kTPCcrossedRowsOverFindableClusters, 0.8, 2.);
  //openCut->AddCut(AliReducedVarManager::kChi2TPCConstrainedVsGlobal, 36., 1.0e+10, kTRUE);
  openCut->AddCut(static_cast<AliReducedVarManager::Variables>(AliReducedVarManager::kTPCnSig+AliReducedVarManager::kElectron), -3.0, 3.0);
  openCut->AddCut(static_cast<AliReducedVarManager::Variables>(AliReducedVarManager::kTPCnSig+AliReducedVarManager::kProton), 3.0, 30000.0);
  openCut->AddCut(static_cast<AliReducedVarManager::Variables>(AliReducedVarManager::kTPCnSig+AliReducedVarManager::kPion), 3.0, 30000.0);
  task->AddTrackCut(openCut);

  // strict cut
  AliReducedTrackCut* strictCut = new AliReducedTrackCut("strictElectron","Electron selection strict");
  strictCut->SetTrackFilterBit(0);
  strictCut->AddCut(AliReducedVarManager::kPt, 1.0,30.0);
  strictCut->AddCut(AliReducedVarManager::kEta, -0.9,0.9);
  strictCut->AddCut(AliReducedVarManager::kDcaXY, -1.0,1.0);
  strictCut->AddCut(AliReducedVarManager::kDcaZ, -3.0,3.0);
  strictCut->AddCut(AliReducedVarManager::kTPCncls, 70.,160.0);
  strictCut->SetRejectKinks();
  strictCut->SetRequestITSrefit();
  strictCut->SetRequestTPCrefit();
  strictCut->SetRequestSPDfirst(); // SPDany in standard & open
  strictCut->AddCut(AliReducedVarManager::kTPCchi2, 0., 4.0);
  strictCut->AddCut(AliReducedVarManager::kITSchi2, 0., 30.);
  strictCut->AddCut(AliReducedVarManager::kTPCNclusBitsFired, 6., 9.);
  strictCut->AddCut(AliReducedVarManager::kTPCnclsSharedRatio, 0.3, 2., kTRUE);
  strictCut->AddCut(AliReducedVarManager::kTPCcrossedRowsOverFindableClusters, 0.8, 2.);
  //strictCut->AddCut(AliReducedVarManager::kChi2TPCConstrainedVsGlobal, 36., 1.0e+10, kTRUE);
  strictCut->AddCut(static_cast<AliReducedVarManager::Variables>(AliReducedVarManager::kTPCnSig+AliReducedVarManager::kElectron), -3.0, 3.0);
  strictCut->AddCut(static_cast<AliReducedVarManager::Variables>(AliReducedVarManager::kTPCnSig+AliReducedVarManager::kProton), 4.0, 30000.0);
  strictCut->AddCut(static_cast<AliReducedVarManager::Variables>(AliReducedVarManager::kTPCnSig+AliReducedVarManager::kPion), 4.0, 30000.0);
  task->AddTrackCut(strictCut);
  
  // strict cut - PID only
  AliReducedTrackCut* strictCut2 = new AliReducedTrackCut("strictPIDElectron","Electron selection strict PID");
  strictCut2->SetTrackFilterBit(0);
  strictCut2->AddCut(AliReducedVarManager::kPt, 1.0,30.0);
  strictCut2->AddCut(AliReducedVarManager::kEta, -0.9,0.9);
  strictCut2->AddCut(AliReducedVarManager::kDcaXY, -1.0,1.0);
  strictCut2->AddCut(AliReducedVarManager::kDcaZ, -3.0,3.0);
  strictCut2->AddCut(AliReducedVarManager::kTPCncls, 70.,160.0);
  strictCut2->SetRejectKinks();
  strictCut2->SetRequestITSrefit();
  strictCut2->SetRequestTPCrefit();
  strictCut2->SetRequestSPDany();
  strictCut2->AddCut(AliReducedVarManager::kTPCchi2, 0., 4.0);
  strictCut2->AddCut(AliReducedVarManager::kITSchi2, 0., 30.);
  strictCut2->AddCut(AliReducedVarManager::kTPCNclusBitsFired, 6., 9.);
  strictCut2->AddCut(AliReducedVarManager::kTPCnclsSharedRatio, 0.3, 2., kTRUE);
  strictCut2->AddCut(AliReducedVarManager::kTPCcrossedRowsOverFindableClusters, 0.8, 2.);
  //strictCut2->AddCut(AliReducedVarManager::kChi2TPCConstrainedVsGlobal, 36., 1.0e+10, kTRUE);
  strictCut2->AddCut(static_cast<AliReducedVarManager::Variables>(AliReducedVarManager::kTPCnSig+AliReducedVarManager::kElectron), -3.0, 3.0);
  strictCut2->AddCut(static_cast<AliReducedVarManager::Variables>(AliReducedVarManager::kTPCnSig+AliReducedVarManager::kProton), 4.0, 30000.0);
  strictCut2->AddCut(static_cast<AliReducedVarManager::Variables>(AliReducedVarManager::kTPCnSig+AliReducedVarManager::kPion), 4.0, 30000.0);
  task->AddTrackCut(strictCut2);
}

//_______________________________________________________________________________________________________________
void SetAssociatedTrackCuts(AliReducedAnalysisJpsi2eeCorrelations* task, TString prod /*=""*/) {
  // standard cut
  AliReducedTrackCut* standardCut = new AliReducedTrackCut("standardHadron","Associated hadron selection standard");
  standardCut->SetTrackFilterBit(1); // NOTE: fQualityFlag bit 33 (see AddTask_laltenka_dst_correlations.C)
  //assocCut->SetTrackFilterBit(4); // NOTE: fQualityFlag bit 36 (see AddTask_laltenka_dst_correlations.C)
  //assocCut->SetTrackFilterBit(5); // NOTE: fQualityFlag bit 37 (see AddTask_laltenka_dst_correlations.C)
  task->AddAssociatedTrackCut(standardCut);
  
  // open cut: no ITS refit
  AliReducedTrackCut* openCut = new AliReducedTrackCut("openHadron","Associated hadron selection open");
  openCut->SetTrackFilterBit(2); // NOTE: fQualityFlag bit 34 (see AddTask_laltenka_dst_correlations.C)
  task->AddAssociatedTrackCut(openCut);

  // strict cut: SPD any
  AliReducedTrackCut* strictCut = new AliReducedTrackCut("strictHadron","Associated hadron selection strict");
  strictCut->SetTrackFilterBit(3); // NOTE: fQualityFlag bit 35 (see AddTask_laltenka_dst_correlations.C)
  task->AddAssociatedTrackCut(strictCut);
  
  // standard cut
  AliReducedTrackCut* standardCut1 = new AliReducedTrackCut("standardHadron1","Associated hadron selection standard");
  standardCut1->SetTrackFilterBit(1); // NOTE: fQualityFlag bit 33 (see AddTask_laltenka_dst_correlations.C)
  task->AddAssociatedTrackCut(standardCut1);

  // standard cut
  AliReducedTrackCut* standardCut2 = new AliReducedTrackCut("standardHadron2","Associated hadron selection standard");
  standardCut2->SetTrackFilterBit(1); // NOTE: fQualityFlag bit 33 (see AddTask_laltenka_dst_correlations.C)
  task->AddAssociatedTrackCut(standardCut2);

  // standard cut
  AliReducedTrackCut* standardCut3 = new AliReducedTrackCut("standardCut3","Associated hadron selection standard");
  standardCut3->SetTrackFilterBit(1); // NOTE: fQualityFlag bit 33 (see AddTask_laltenka_dst_correlations.C)
  task->AddAssociatedTrackCut(standardCut3);
}

//_______________________________________________________________________________________________________________
void SetTrackPrefilterCuts(AliReducedAnalysisJpsi2eeCorrelations* task, TString prod /*=""*/) {
  AliReducedTrackCut* trackPrefilterCut = new AliReducedTrackCut("TrackPrefilterCut","Track prefilter selection");
  trackPrefilterCut->SetTrackFilterBit(0);       // NOTE: we may want to setup a filter bit with open kinematic electron cuts for the prefilter ...
  trackPrefilterCut->AddCut(AliReducedVarManager::kPt, 0.7,100.0);
  trackPrefilterCut->SetRequestTPCrefit();
  task->AddPrefilterTrackCut(trackPrefilterCut);
}

//_______________________________________________________________________________________________________________
void SetPairCuts(AliReducedAnalysisJpsi2eeCorrelations* task, TString prod /*=""*/) {
  AliReducedTrackCut* pairCut = new AliReducedTrackCut("PairCut","Pair selection");
  // NOTE: this is an extra selection on jpsi candidates.
  //           You may want to e.g. select jpsi candidates in a mass range around the jpsi peak, to save CPU.
  //           Normally we will need the correlation function in the peak region and possibly in side bands around.
  //           Remember that the flags which are used during correlations are those inherited from the leg cuts
  pairCut->AddCut(AliReducedVarManager::kPt, 0.0,100.0);
  pairCut->AddCut(AliReducedVarManager::kRap, -0.9,0.9);
  task->AddPairCut(pairCut);
}

//_______________________________________________________________________________________________________________
void SetPairPrefilterCuts(AliReducedAnalysisJpsi2eeCorrelations* task, TString prod /*=""*/) {
  AliReducedVarCut* pairPrefilterCut = new AliReducedVarCut("PairPrefilterCut","Pair prefilter selection");
  pairPrefilterCut->AddCut(AliReducedVarManager::kMass, 0.0, 0.05, kTRUE);
  task->AddPrefilterPairCut(pairPrefilterCut);
}

//_______________________________________________________________________________________________________________
void SetMCSignalCutsLeg(AliReducedAnalysisJpsi2eeCorrelations* task, TString prod /*=""*/) {
  // electron from inclusive J/psi
  AliReducedTrackCut* trueElectron = new AliReducedTrackCut("TrueElectron", "reconstructed electrons with MC truth");
  trueElectron->SetMCFilterBit(kJpsiDecayElectron);
  task->AddLegCandidateMCcut(trueElectron);
  
  // electron from non-prompt J/psi
  AliReducedTrackCut* trueElectronNonPrompt = new AliReducedTrackCut("TrueElectronNonPrompt", "reconstructed electrons with MC truth");
  trueElectronNonPrompt->SetMCFilterBit(kJpsiNonPromptDecayElectron);
  task->AddLegCandidateMCcut(trueElectronNonPrompt);
  
  // electron from prompt J/psi
  AliReducedTrackCut* trueElectronPrompt = new AliReducedTrackCut("TrueElectronPrompt", "reconstructed electrons with MC truth");
  trueElectronPrompt->SetMCFilterBit(kJpsiPromptDecayElectron);
  task->AddLegCandidateMCcut(trueElectronPrompt);
}

//_______________________________________________________________________________________________________________
void SetMCSignalCutsJPsi(AliReducedAnalysisJpsi2eeCorrelations* task, TString prod /*=""*/) {
  // inclusive J/psi
  AliReducedTrackCut* mcTruthJpsi = new AliReducedTrackCut("mcTruthJpsi", "Pure MC truth J/psi");
  mcTruthJpsi->SetMCFilterBit(kJpsiInclusive);
  mcTruthJpsi->AddCut(AliReducedVarManager::kRap, -0.9, 0.9);
  AliReducedTrackCut* mcTruthJpsiElectron = new AliReducedTrackCut("mcTruthJpsiElectron", "Pure MC truth electron from J/psi");
  mcTruthJpsiElectron->SetMCFilterBit(kJpsiDecayElectron);
  mcTruthJpsiElectron->AddCut(AliReducedVarManager::kEta, -0.9, 0.9);
  mcTruthJpsiElectron->AddCut(AliReducedVarManager::kPt, 1.0, 100.0);
  task->AddJpsiMotherMCCut(mcTruthJpsi, mcTruthJpsiElectron);
  
  // non-prompt J/psi
  AliReducedTrackCut* mcTruthJpsiNonPrompt = new AliReducedTrackCut("mcTruthJpsiNonPrompt", "Pure MC truth J/psi");
  mcTruthJpsiNonPrompt->SetMCFilterBit(kJpsiNonPrompt);
  mcTruthJpsiNonPrompt->AddCut(AliReducedVarManager::kRap, -0.9, 0.9);
  AliReducedTrackCut* mcTruthJpsiElectronNonPrompt = new AliReducedTrackCut("mcTruthJpsiElectronNonPrompt", "Pure MC truth electron from J/psi");
  mcTruthJpsiElectronNonPrompt->SetMCFilterBit(kJpsiNonPromptDecayElectron);
  mcTruthJpsiElectronNonPrompt->AddCut(AliReducedVarManager::kEta, -0.9, 0.9);
  mcTruthJpsiElectronNonPrompt->AddCut(AliReducedVarManager::kPt, 1.0, 100.0);
  task->AddJpsiMotherMCCut(mcTruthJpsiNonPrompt, mcTruthJpsiElectronNonPrompt);

  // prompt J/psi
  AliReducedTrackCut* mcTruthJpsiPrompt = new AliReducedTrackCut("mcTruthJpsiPrompt", "Pure MC truth J/psi");
  mcTruthJpsiPrompt->SetMCFilterBit(kJpsiPrompt);
  mcTruthJpsiPrompt->AddCut(AliReducedVarManager::kRap, -0.9, 0.9);
  AliReducedTrackCut* mcTruthJpsiElectronPrompt = new AliReducedTrackCut("mcTruthJpsiElectronPrompt", "Pure MC truth electron from J/psi");
  mcTruthJpsiElectronPrompt->SetMCFilterBit(kJpsiPromptDecayElectron);
  mcTruthJpsiElectronPrompt->AddCut(AliReducedVarManager::kEta, -0.9, 0.9);
  mcTruthJpsiElectronPrompt->AddCut(AliReducedVarManager::kPt, 1.0, 100.0);
  task->AddJpsiMotherMCCut(mcTruthJpsiPrompt, mcTruthJpsiElectronPrompt);
}

//_______________________________________________________________________________________________________________
void SetupMixingHandlers(AliReducedAnalysisJpsi2eeCorrelations* task, TString prod /*=""*/) {
  //
  // Setup both the mixing handler for jpsi signal extraction (if needed) and the correlation handler
  //
  // The jpsi signal extraction mixing handler
  AliMixingHandler* handler = task->GetMixingHandler();
  handler->SetPoolDepth(100);
  handler->SetMixingThreshold(1.0);
  handler->SetDownscaleEvents(1);
  handler->SetDownscaleTracks(1);
  handler->AddMixingVariable(AliReducedVarManager::kVtxZ,           gNVtxZLimits, gVtxZLimits);
  handler->AddMixingVariable(AliReducedVarManager::kSPDntracklets,  gNMultLimits, gMultLimits); // kVZEROTotalMult
  
  // The correlation mixing handler
  handler = task->GetCorrelationMixingHandler();
  handler->SetPoolDepth(100);
  handler->SetMixingThreshold(1.0);
  handler->SetDownscaleEvents(1);
  handler->SetDownscaleTracks(1);
  handler->AddMixingVariable(AliReducedVarManager::kVtxZ,           gNVtxZLimits, gVtxZLimits);
  handler->AddMixingVariable(AliReducedVarManager::kSPDntracklets,  gNMultLimits, gMultLimits); // kVZEROTotalMult
}

//_______________________________________________________________________________________________________________
void SetupHistogramManager(AliReducedAnalysisJpsi2eeCorrelations* task, TString prod /*=""*/, TString runNumbers /*=""*/) {
  AliReducedVarManager::SetDefaultVarNames();
  DefineHistograms(task, prod, runNumbers);
  AliReducedVarManager::SetUseVars((Bool_t*)task->GetHistogramManager()->GetUsedVars());
}

//_______________________________________________________________________________________________________________
void DefineHistograms(AliReducedAnalysisJpsi2eeCorrelations* task, TString prod /*=""*/, TString runNumbers /*=""*/) {
  AliHistogramManager* man = task->GetHistogramManager();

  // event quantities
  TString histClasses = "";
  histClasses += "Event_BeforeCuts;";
  histClasses += "Event_AfterCuts;";
  if (!task->GetRunOverMC()) {
    histClasses += "EventTag_BeforeCuts;";
    histClasses += "EventTag_AfterCuts;";
    histClasses += "EventTriggers_BeforeCuts;";
    histClasses += "EventTriggers_AfterCuts;";
  }
  
  // MC J/psi tracks
  if(task->GetRunOverMC()) {
    for(Int_t mcSel=0; mcSel<task->GetNJpsiMotherMCCuts(); ++mcSel) {
      histClasses += Form("%s_PureMCTruth_BeforeSelection;", task->GetJpsiMotherMCcutName(mcSel));
      histClasses += Form("%s_PureMCTruth_AfterSelection;", task->GetJpsiMotherMCcutName(mcSel));
    }
  }
  
  if (task->GetLoopOverTracks()) {
    // track quantites
    histClasses += "Track_BeforeCuts;";
    histClasses += "TrackITSclusterMap_BeforeCuts;";
    histClasses += "TrackTPCclusterMap_BeforeCuts;";
    histClasses += "TrackStatusFlags_BeforeCuts;";

    // per cut
    for (Int_t i=0; i<task->GetNTrackCuts(); ++i) {
      TString cutName = task->GetTrackCutName(i);
      
      histClasses += Form("Track_%s;", cutName.Data());
      histClasses += Form("TrackITSclusterMap_%s;", cutName.Data());
      histClasses += Form("TrackTPCclusterMap_%s;", cutName.Data());
      if (!task->GetRunOverMC())
        histClasses += Form("TrackStatusFlags_%s;", cutName.Data());
      
      if (task->GetRunOverMC()) {
        for(Int_t mcSel=0; mcSel<task->GetNLegCandidateMCcuts(); ++mcSel) {
          histClasses += Form("Track_%s_%s;", cutName.Data(), task->GetLegCandidateMCcutName(mcSel));
          histClasses += Form("TrackStatusFlags_%s_%s;", cutName.Data(), task->GetLegCandidateMCcutName(mcSel));
          histClasses += Form("TrackITSclusterMap_%s_%s;", cutName.Data(), task->GetLegCandidateMCcutName(mcSel));
          histClasses += Form("TrackTPCclusterMap_%s_%s;", cutName.Data(), task->GetLegCandidateMCcutName(mcSel));
        }
      }
      
      // pair quantities
      if (!task->GetRunLikeSignPairing()) histClasses += Form("PairSEPM_%s;", cutName.Data());
      else                                histClasses += Form("PairSEPP_%s;PairSEPM_%s;PairSEMM_%s;", cutName.Data(), cutName.Data(), cutName.Data());
      
      // pair quantities MC
      if (!task->GetRunOverMC())
        histClasses += Form("PairMEPP_%s;PairMEPM_%s;PairMEMM_%s;", cutName.Data(), cutName.Data(), cutName.Data());
      else {
        for(Int_t mcSel=0; mcSel<task->GetNLegCandidateMCcuts(); ++mcSel)
          histClasses += Form("PairSEPM_%s_%s;", cutName.Data(), task->GetLegCandidateMCcutName(mcSel));
      }
    }
  }
  
  // associated track quantities
  if (task->GetRunCorrelation()) {
    histClasses += "AssociatedTrack_BeforeCuts;";
    histClasses += "AssociatedTrackITSclusterMap_BeforeCuts;";
    histClasses += "AssociatedTrackTPCclusterMap_BeforeCuts;";

    // per cut
    for (Int_t i=0; i<task->GetNAssociatedTrackCuts(); ++i) {
      TString assocCutName = task->GetAssociatedTrackCutName(i);
      
      histClasses += Form("AssociatedTrack_%s;", assocCutName.Data());
      histClasses += Form("AssociatedTrackITSclusterMap_%s;", assocCutName.Data());
      histClasses += Form("AssociatedTrackTPCclusterMap_%s;", assocCutName.Data());

      if (!task->GetRunOverMC())
        histClasses += Form("AssociatedTrackStatusFlags_%s;", assocCutName.Data());
      //if (task->GetRunOverMC()) {
      //  histClasses += Form("AssociatedTrack_%s_MCTruth;", assocCutName.Data());
      //  histClasses += Form("AssociatedTrackStatusFlags_%s_MCTruth;", assocCutName.Data());
      //  histClasses += Form("AssociatedITSclusterMap_%s_MCTruth;", assocCutName.Data());
      //  histClasses += Form("AssociatedTPCclusterMap_%s_MCTruth;", assocCutName.Data());
      //}
    }
  }
  
  // correlation quantities
  if(task->GetRunCorrelation()) {
    for (Int_t i=0; i<task->GetNTrackCuts(); ++i) {
      TString cutName       = task->GetTrackCutName(i);
      TString assocCutName  = task->GetAssociatedTrackCutName(i);
      histClasses += Form("CorrSEPP_%s_%s;CorrSEPM_%s_%s;CorrSEMM_%s_%s;",
                          cutName.Data(), assocCutName.Data(), cutName.Data(), assocCutName.Data(), cutName.Data(), assocCutName.Data());
      //if(task->GetRunOverMC())
      //  histClasses += Form("CorrSEPP_%s_%s_MCTruth;CorrSEPM_%s_%s_MCTruth;CorrSEMM_%s_%s_MCTruth;",
      //                      cutName.Data(), assocCutName.Data(), cutName.Data(), assocCutName.Data(), cutName.Data(), assocCutName.Data());
      if(task->GetRunCorrelationMixing())
        histClasses += Form("CorrMEPP_%s_%s;CorrMEPM_%s_%s;CorrMEMM_%s_%s;",
                            cutName.Data(), assocCutName.Data(), cutName.Data(), assocCutName.Data(), cutName.Data(), assocCutName.Data());
      //if(task->GetRunCorrelationMixing() && task->GetRunOverMC())
      //  histClasses += Form("CorrMEPP_%s_%s_MCTruth;CorrMEPM_%s_%s_MCTruth;CorrMEMM_%s_%s_MCTruth;",
      //                      cutName.Data(), assocCutName.Data(), cutName.Data(), assocCutName.Data(), cutName.Data(), assocCutName.Data());
    }
  }

  // run numbers
  Int_t     runNBins        = 0;
  Double_t  runHistRange[2] = {0.0,0.0};
  if (runNumbers.CompareTo("")) {
    TObjArray* runArr = runNumbers.Tokenize(";");
    runNBins = runArr->GetEntries();
    runHistRange[1] = (Double_t)runNBins;
  } else {
    cout << "WARNING in AliReducedAnalysisJpsi2eeCorrelations::DefineHistograms(), production " << prod.Data() << " not known, run ranges not defined!" << endl;
  }
  
  // pt binning
  const Int_t kNPtBins = 6;
  Double_t ptBins[kNPtBins] = {0.0, 3.0, 5.0, 8.0, 16.0, 30.0};
  
  // rapidity binning
  const Int_t kNRapidityBins = 6; // abs. rap
  Double_t rapidityBins[kNRapidityBins] = {0.0, 0.5, 0.6, 0.7, 0.8, 0.9};

  // mass binning
  const Int_t kNMassBins = 71; // 1.4 - 4.2
  Double_t massBins[kNMassBins];
  for (Int_t i=0; i<kNMassBins; i++) massBins[i] = 1.4+i*0.04;
  const Int_t kNMassBinsCorr = 31; // 1.4 - 4.2
  Double_t massBinsCorr[kNMassBinsCorr];
  for (Int_t i=0; i<6; i++)   massBinsCorr[i]     = 1.4+i*0.2;  // 1.40 - 2.20
  for (Int_t i=0; i<20; i++)  massBinsCorr[i+6]   = 2.6+i*0.04; // 2.60 - 3.36
  for (Int_t i=0; i<5; i++)   massBinsCorr[i+26]  = 3.4+i*0.2;  // 3.40 - 4.20

  // trigger names
  TString triggerNames = "";
  for (Int_t i=0; i<64; ++i) { triggerNames += AliReducedVarManager::fgkOfflineTriggerNames[i]; triggerNames+=";"; }
  
  // add histograms according to class to histogram manager
  TString classesStr(histClasses);
  TObjArray* arr=classesStr.Tokenize(";");

  // loop over histogram classes and add histograms
  printf("INFO on AddTask_laltenka_jpsi2ee_correlations(): Histgram classes included in histogram manager\n");
  for(Int_t iclass=0; iclass<arr->GetEntries(); ++iclass) {
    TString classStr = arr->At(iclass)->GetName();
  
    //-----------------------------------------------------------------------------------------------------------
    // MC histograms
    //-----------------------------------------------------------------------------------------------------------
    if(classStr.Contains("MCTruth")) {
      man->AddHistClass(classStr.Data());
      printf("%s\n",classStr.Data());
      
      man->AddHistogram(classStr.Data(), "MassMC",      "MC mass",      kFALSE, 200, 0., 5.0,   AliReducedVarManager::kMassMC);
      man->AddHistogram(classStr.Data(), "RapidityMC",  "MC rapidity",  kFALSE, 48, -1.2, 1.2,  AliReducedVarManager::kRapMC);
      man->AddHistogram(classStr.Data(), "PtMC",        "p_{T} MC",     kFALSE, 1000, 0., 10.0, AliReducedVarManager::kPtMC);
      man->AddHistogram(classStr.Data(), "PtMC_coarse", "p_{T} MC",     kFALSE, 20, 0., 20.0,   AliReducedVarManager::kPtMC);
      man->AddHistogram(classStr.Data(), "PhiMC",       "MC #varphi",   kFALSE, 100, 0., 6.3,   AliReducedVarManager::kPhiMC);
      man->AddHistogram(classStr.Data(), "EtaMC",       "MC #eta",      kFALSE, 100, -1.5, 1.5, AliReducedVarManager::kEtaMC);
      man->AddHistogram(classStr.Data(), "PtMC_RapMC",  "",             kFALSE, 100, -1.2, 1.2, AliReducedVarManager::kRapMC,
                                                                                100, 0., 15.,   AliReducedVarManager::kPtMC);
      
      continue;
    }
    
    //-----------------------------------------------------------------------------------------------------------
    // event tag histograms
    //-----------------------------------------------------------------------------------------------------------
    if(classStr.Contains("EventTag_")) {
      man->AddHistClass(classStr.Data());
      printf("%s\n",classStr.Data());
      TString tagNames = "";
      tagNames += "AliAnalysisUtils 2013 selection;";
      tagNames += "AliAnalysisUtils MV pileup;";
      tagNames += "AliAnalysisUtils MV pileup, no BC check;";
      tagNames += "AliAnalysisUtils MV pileup, min wght dist 10;";
      tagNames += "AliAnalysisUtils MV pileup, min wght dist 5;";
      tagNames += "IsPileupFromSPD(3,0.6,3.,2.,5.);";
      tagNames += "IsPileupFromSPD(4,0.6,3.,2.,5.);";
      tagNames += "IsPileupFromSPD(5,0.6,3.,2.,5.);";
      tagNames += "IsPileupFromSPD(6,0.6,3.,2.,5.);";
      tagNames += "IsPileupFromSPD(3,0.8,3.,2.,5.);";
      tagNames += "IsPileupFromSPD(4,0.8,3.,2.,5.);";
      tagNames += "IsPileupFromSPD(5,0.8,3.,2.,5.);";
      tagNames += "IsPileupFromSPD(6,0.8,3.,2.,5.);";
      tagNames += "vtx distance selected;";
      
      man->AddHistogram(classStr.Data(), "EventTag", "Event Tag", kFALSE, 20, -0.5, 19.5, AliReducedVarManager::kEventTag, 0, 0.0, 0.0, AliReducedVarManager::kNothing, 0, 0.0, 0.0, AliReducedVarManager::kNothing, tagNames.Data());
      
      continue;
    }

    //-----------------------------------------------------------------------------------------------------------
    // trigger histograms
    //-----------------------------------------------------------------------------------------------------------
    if(classStr.Contains("EventTriggers_")) {
      man->AddHistClass(classStr.Data());
      printf("%s\n",classStr.Data());
      
      man->AddHistogram(classStr.Data(), "Triggers", "", kFALSE, 32, -0.5, 31.5, AliReducedVarManager::kOnlineTriggerFired, 0, 0.0, 0.0, AliReducedVarManager::kNothing, 0, 0.0, 0.0, AliReducedVarManager::kNothing, triggerNames.Data());
      
      continue;
    }

    //-----------------------------------------------------------------------------------------------------------
    // event histograms
    //-----------------------------------------------------------------------------------------------------------
    if(classStr.Contains("Event_")) {
      man->AddHistClass(classStr.Data());
      printf("%s\n",classStr.Data());
      
      // run numbers
      man->AddHistogram(classStr.Data(), "RunNo", "Run numbers", kFALSE,
                        runNBins, runHistRange[0], runHistRange[1],   AliReducedVarManager::kRunID,
                        0, 0, 0,                                      AliReducedVarManager::kNothing,
                        0, 0, 0,                                      AliReducedVarManager::kNothing,
                        runNumbers.Data());
      man->AddHistogram(classStr.Data(), "INT7_RunNo_QA", "MB trigger vs run number", kFALSE,
                        runNBins, runHistRange[0], runHistRange[1],   AliReducedVarManager::kRunID,
                        2, 0.0, 2.,                                   AliReducedVarManager::kINT7Triggered,
                        0, 0, 0,                                      AliReducedVarManager::kNothing,
                        runNumbers.Data());
      man->AddHistogram(classStr.Data(), "HighMultV0_RunNo_QA", "V0 high mult. trigger vs run number", kFALSE,
                        runNBins, runHistRange[0], runHistRange[1],   AliReducedVarManager::kRunID,
                        2, 0.0, 2.,                                   AliReducedVarManager::kHighMultV0Triggered,
                        0, 0, 0,                                      AliReducedVarManager::kNothing,
                        runNumbers.Data());

      // multiplicity
      man->AddHistogram(classStr.Data(), "VZEROmult_QA", "VZEROTotalMult", kFALSE,
                        100, 0.0, 2000.,                              AliReducedVarManager::kVZEROTotalMult);
      man->AddHistogram(classStr.Data(), "VZEROmult_RunNo_QA", "Run IDs vs VZEROTotalMult", kFALSE,
                        runNBins, runHistRange[0], runHistRange[1],   AliReducedVarManager::kRunID,
                        100, 0.0, 2000.,                              AliReducedVarManager::kVZEROTotalMult,
                        0,0,0,                                        AliReducedVarManager::kNothing,
                        runNumbers.Data());
      man->AddHistogram(classStr.Data(), "VZEROmultProfile_RunNo_QA", "Run IDs vs VZEROTotalMult", kTRUE,
                        runNBins, runHistRange[0], runHistRange[1],   AliReducedVarManager::kRunID,
                        100, 0.0, 2000.,                              AliReducedVarManager::kVZEROTotalMult,
                        0,0,0,                                        AliReducedVarManager::kNothing,
                        runNumbers.Data());
      man->AddHistogram(classStr.Data(), "SPDnTracklets", "SPD #tracklets in |#eta|<1.0", kFALSE,
                        200, 0.0, 200.,                               AliReducedVarManager::kSPDntracklets);
      man->AddHistogram(classStr.Data(), "SPDtracklets_RunNo_QA", "SPD <#tracklets> in |#eta|<1.0 vs run number", kFALSE,
                        runNBins, runHistRange[0], runHistRange[1],   AliReducedVarManager::kRunID,
                        200, 0.0, 200.,                               AliReducedVarManager::kSPDntracklets,
                        0,0,0,                                        AliReducedVarManager::kNothing,
                        runNumbers.Data());
      man->AddHistogram(classStr.Data(), "SPDtrackletsProfile_RunNo_QA", "Run IDs vs SPD no. tracklets", kTRUE,
                        runNBins, runHistRange[0], runHistRange[1],   AliReducedVarManager::kRunID,
                        200, 0.0, 200.,                               AliReducedVarManager::kSPDntracklets,
                        0,0,0,                                        AliReducedVarManager::kNothing,
                        runNumbers.Data());
      man->AddHistogram(classStr.Data(), "SPDtracklets_VZEROmult_QA", "SPD no. tracklets", kFALSE,
                        200, 0.0, 200.,                               AliReducedVarManager::kSPDntracklets,
                        100, 0.0, 2000.,                              AliReducedVarManager::kVZEROTotalMult);
      
      // physics selection
      man->AddHistogram(classStr.Data(), "IsPhysicsSelection", "Physics selection flag", kFALSE,
                        2, -0.5, 1.5,                                 AliReducedVarManager::kIsPhysicsSelection,
                        0, 0, 0,                                      AliReducedVarManager::kNothing,
                        0, 0, 0,                                      AliReducedVarManager::kNothing,
                        "off;on");
      
      // vertex histograms
      man->AddHistogram(classStr.Data(), "VtxX", "Vtx X", kFALSE,
                        300, -0.4, 0.4,                               AliReducedVarManager::kVtxX);
      man->AddHistogram(classStr.Data(), "VtxX_RunNo_QA", "<Vtx X> vs run number", kTRUE,
                        runNBins, runHistRange[0], runHistRange[1],   AliReducedVarManager::kRunID,
                        300, -0.4, 0.4,                               AliReducedVarManager::kVtxX,
                        0, 0, 0,                                      AliReducedVarManager::kNothing,
                        runNumbers.Data());
      man->AddHistogram(classStr.Data(), "VtxY", "Vtx Y", kFALSE,
                        300, -0.4, 0.4,                               AliReducedVarManager::kVtxY);
      man->AddHistogram(classStr.Data(), "VtxY_RunNo_QA", "<Vtx Y> vs run number", kTRUE,
                        runNBins, runHistRange[0], runHistRange[1],   AliReducedVarManager::kRunID,
                        300, -0.4, 0.4,                               AliReducedVarManager::kVtxY,
                        0, 0, 0,                                      AliReducedVarManager::kNothing,
                        runNumbers.Data());
      man->AddHistogram(classStr.Data(), "VtxZ", "Vtx Z", kFALSE,
                        300, -15.0, 15.0,                             AliReducedVarManager::kVtxZ);
      man->AddHistogram(classStr.Data(), "VtxZ_RunNo_QA", "<Vtx Z> vs run number", kTRUE,
                        runNBins, runHistRange[0], runHistRange[1],   AliReducedVarManager::kRunID,
                        300, -15.0, 15.0,                             AliReducedVarManager::kVtxZ,
                        0, 0, 0,                                      AliReducedVarManager::kNothing,
                        runNumbers.Data());
      man->AddHistogram(classStr.Data(), "DeltaVtxZ", "Z_{global}-Z_{TPC}", kFALSE,
                        300, -1., 1.,                                 AliReducedVarManager::kDeltaVtxZ);
      man->AddHistogram(classStr.Data(), "DeltaVtxZ_RunNo_QA", "Z_{global}-Z_{TPC} vs run number", kTRUE,
                        runNBins, runHistRange[0], runHistRange[1],   AliReducedVarManager::kRunID,
                        300, -1., 1.,                                 AliReducedVarManager::kDeltaVtxZ,
                        0, 0, 0,                                      AliReducedVarManager::kNothing,
                        runNumbers.Data());
      man->AddHistogram(classStr.Data(), "NVtxContributors_RunNo_QA", "number of vertex contributors vs run number", kTRUE,
                        runNBins, runHistRange[0], runHistRange[1],   AliReducedVarManager::kRunID,
                        500, 0., 500.,                                AliReducedVarManager::kNVtxContributors,
                        0, 0, 0,                                      AliReducedVarManager::kNothing,
                        runNumbers.Data());

      // pile-up
      man->AddHistogram(classStr.Data(), "NTracksTPCoutFromPileup", "number of tracks from (kNTracksPerTrackingStatus+kTPCout) minus the no-pileup expectation", kFALSE,
                        2000, -10000., 30000.,                        AliReducedVarManager::kNTracksTPCoutFromPileup);
      man->AddHistogram(classStr.Data(), "NTracksTPCoutFromPileup_RunNo_QA", "<number of tracks from (kNTracksPerTrackingStatus+kTPCout) minus the no-pileup expectation> per run", kTRUE,
                        runNBins, runHistRange[0], runHistRange[1],   AliReducedVarManager::kRunID,
                        2000, -10000., 30000.,                        AliReducedVarManager::kNTracksTPCoutFromPileup,
                        0, 0, 0,                                      AliReducedVarManager::kNothing,
                        runNumbers.Data());
      man->AddHistogram(classStr.Data(), "SPDPileUp_RunNo_QA", "SPD pileup per run", kFALSE,
                        runNBins, runHistRange[0], runHistRange[1],   AliReducedVarManager::kRunID,
                        100, 0., 10.,                                 AliReducedVarManager::kIsSPDPileup,
                        0, 0, 0,                                      AliReducedVarManager::kNothing,
                        runNumbers.Data());
      man->AddHistogram(classStr.Data(), "SPDPileUp5_RunNo_QA", "SPD pileup per run", kFALSE,
                        runNBins, runHistRange[0], runHistRange[1],   AliReducedVarManager::kRunID,
                        100, 0., 10.,                                 AliReducedVarManager::kIsSPDPileup5,
                        0, 0, 0,                                      AliReducedVarManager::kNothing,
                        runNumbers.Data());
      man->AddHistogram(classStr.Data(), "TZEROPileUp_RunNo_QA", "<TZERO pileup> per run", kFALSE,
                        runNBins, runHistRange[0], runHistRange[1],   AliReducedVarManager::kRunID,
                        100, 0., 10.,                                 AliReducedVarManager::kTZEROpileup,
                        0, 0, 0,                                      AliReducedVarManager::kNothing,
                        runNumbers.Data());
      man->AddHistogram(classStr.Data(), "MVPileUp_RunNo_QA", "<multi vertexer pileup> per run", kTRUE,
                        runNBins, runHistRange[0], runHistRange[1],   AliReducedVarManager::kRunID,
                        100, 0., 10.,                                 AliReducedVarManager::kIsPileupMV,
                        0, 0, 0,                                      AliReducedVarManager::kNothing,
                        runNumbers.Data());
      man->AddHistogram(classStr.Data(), "MVPileUp_RunNo_2D_QA", "<multi vertexer pileup> per run", kFALSE,
                        runNBins, runHistRange[0], runHistRange[1],   AliReducedVarManager::kRunID,
                        100, 0., 10.,                                 AliReducedVarManager::kIsPileupMV,
                        0, 0, 0,                                      AliReducedVarManager::kNothing,
                        runNumbers.Data());
      man->AddHistogram(classStr.Data(), "BunchCrossing", "Bunch crossing", kFALSE,
                        10000.,0.,10000.,                             AliReducedVarManager::kBC);
      man->AddHistogram(classStr.Data(), "BunchCrossing_RunNo_QA", "Bunch crossing", kTRUE,
                        runNBins, runHistRange[0], runHistRange[1],   AliReducedVarManager::kRunID,
                        10000.,0.,10000.,                             AliReducedVarManager::kBC,
                        0, 0, 0,                                      AliReducedVarManager::kNothing,
                        runNumbers.Data());

      // SPD information
      man->AddHistogram(classStr.Data(), "SPDnSingleClusters", "SPD single clusters", kFALSE,
                        500, 0., 1000.,                                AliReducedVarManager::kSPDnSingleClusters);
      man->AddHistogram(classStr.Data(), "SPDnSingleClusters_RunNo_QA", "SPD single <#clusters> vs run number", kTRUE,
                        runNBins, runHistRange[0], runHistRange[1],   AliReducedVarManager::kRunID,
                        500, 0., 1000.,                                AliReducedVarManager::kSPDnSingleClusters,
                        0, 0, 0,                                      AliReducedVarManager::kNothing,
                        runNumbers.Data());
      man->AddHistogram(classStr.Data(), "SPDnSingleClustersVsSPDnTracklets", "SPD single clusters vs SPD tracklets", kFALSE,
                        200, 0., 200.,                                AliReducedVarManager::kSPDntracklets,
                        500, 0., 1000.,                               AliReducedVarManager::kSPDnSingleClusters);
      man->AddHistogram(classStr.Data(), "SPDnClustersLayer1", "SPD clusters layer 1", kFALSE,
                        500, 0., 1000.,                               AliReducedVarManager::kITSnClusters+0);
      man->AddHistogram(classStr.Data(), "SPDnClustersLayer1VsSPDnTracklets", "SPD clusters layer 1 vs SPD tracklets", kFALSE,
                        200, 0., 200.,                                AliReducedVarManager::kSPDntracklets,
                        500, 0., 1000.,                               AliReducedVarManager::kITSnClusters+0);
      man->AddHistogram(classStr.Data(), "SPDnClustersLayer2", "SPD clusters layer 2", kFALSE,
                        500, 0., 1000.,                               AliReducedVarManager::kITSnClusters+1);
      man->AddHistogram(classStr.Data(), "SPDnClustersLayer2VsSPDnTracklets", "SPD clusters layer 2 vs SPD tracklets", kFALSE,
                        200, 0., 200.,                                AliReducedVarManager::kSPDntracklets,
                        500, 0., 1000.,                               AliReducedVarManager::kITSnClusters+1);
      for(Int_t il=0; il<2; ++il) {
        man->AddHistogram(classStr.Data(), Form("SPDfiredChips_layer%d",il+1), Form("SPD fired chips in layer %d",il+1), kFALSE,
                          200, 0., 1000.,                             AliReducedVarManager::kSPDFiredChips+il);
        man->AddHistogram(classStr.Data(), Form("SPDfiredChips_layer%d_RunNo_QA",il+1), Form("SPD <#fired chips> in layer %d vs run number",il+1), kTRUE,
                          runNBins, runHistRange[0], runHistRange[1], AliReducedVarManager::kRunID,
                          200, 0., 1000.,                             AliReducedVarManager::kSPDFiredChips+il,
                          0, 0, 0,                                    AliReducedVarManager::kNothing,
                          runNumbers.Data());
        man->AddHistogram(classStr.Data(), Form("SPDfirecChips_layer%d_Vs_SPDnTracklets", il+1), Form("SPD fired chips in layer %d vs SPD tracklets",il+1), kFALSE,
                          200, 0., 200.,                              AliReducedVarManager::kSPDntracklets,
                          500, 0., 1000.,                             AliReducedVarManager::kSPDFiredChips+il);
      }
      man->AddHistogram(classStr.Data(), "SPDnTrackletsVsTPCoutTracks", "SPD tracklets vs tracks with TPC out flag", kFALSE,
                        200, 0., 200.,                                AliReducedVarManager::kSPDntracklets,
                        2000, 0., 2000.,                              AliReducedVarManager::kNTracksPerTrackingStatus+AliReducedVarManager::kTPCout);

      // number of tracks
      man->AddHistogram(classStr.Data(), "NTracksTotal", "Number of total tracks per event", kFALSE,
                        500,0.,3000.,                                 AliReducedVarManager::kNtracksTotal);
      man->AddHistogram(classStr.Data(), "NTrackstotal_RunNo_QA", "Averaged number of total tracks per event vs run number", kTRUE,
                        runNBins, runHistRange[0], runHistRange[1],   AliReducedVarManager::kRunID,
                        100,0.,100.,                                  AliReducedVarManager::kNtracksTotal,
                        0, 0, 0,                                      AliReducedVarManager::kNothing,
                        runNumbers.Data());
      man->AddHistogram(classStr.Data(), "NTracksAnalyzed", "#tracks analyzed per event", kFALSE,
                        100,0.,100.,                                  AliReducedVarManager::kNtracksAnalyzed);
      man->AddHistogram(classStr.Data(), "NTracksAnalyzed_RunNo_QA", "<number of tracks analyzed> vs. run number", kTRUE,
                        runNBins, runHistRange[0], runHistRange[1],   AliReducedVarManager::kRunID,
                        100,0.,100.,                                  AliReducedVarManager::kNtracksAnalyzed,
                        0, 0, 0,                                      AliReducedVarManager::kNothing,
                        runNumbers.Data());
      man->AddHistogram(classStr.Data(), "NTracksSelected", "#tracks selected per event", kFALSE,
                        100,0.,100.,                                  AliReducedVarManager::kNtracksSelected);
      man->AddHistogram(classStr.Data(), "NTracksSelected_RunNo_QA", "<number of tracks selected> vs. run number", kTRUE,
                        runNBins, runHistRange[0], runHistRange[1],   AliReducedVarManager::kRunID,
                        100,0.,100.,                                  AliReducedVarManager::kNtracksSelected,
                        0, 0, 0,                                      AliReducedVarManager::kNothing,
                        runNumbers.Data());
      
      // electron and positron tracks
      man->AddHistogram(classStr.Data(), "NPosTracksAnalyzed", "#positrons per event", kFALSE,
                        100,0.,100.,                                  AliReducedVarManager::kNtracksPosAnalyzed);
      man->AddHistogram(classStr.Data(), "NPosTracksAnalyzed_RunNo_QA", "<#positrons> vs run number", kTRUE,
                        runNBins, runHistRange[0], runHistRange[1],   AliReducedVarManager::kRunID,
                        100,0.,100.,                                  AliReducedVarManager::kNtracksPosAnalyzed,
                        0, 0, 0,                                      AliReducedVarManager::kNothing,
                        runNumbers.Data());
      man->AddHistogram(classStr.Data(), "NNegTracksAnalyzed", "#electrons per event", kFALSE,
                        100,0.,100.,                                  AliReducedVarManager::kNtracksNegAnalyzed);
      man->AddHistogram(classStr.Data(), "NNegTracksAnalyzed_RunNo_QA", "<#electrons> vs run number", kTRUE,
                        runNBins, runHistRange[0], runHistRange[1],   AliReducedVarManager::kRunID,
                        100,0.,100.,                                  AliReducedVarManager::kNtracksNegAnalyzed,
                        0, 0, 0,                                      AliReducedVarManager::kNothing,
                        runNumbers.Data());
      
      // pair candidates
      man->AddHistogram(classStr.Data(), "NPairCandidates", "number of pair candidates per event", kFALSE,
                        10, 0., 10.,                                  AliReducedVarManager::kNpairsSelected);
      man->AddHistogram(classStr.Data(), "NPairCandidates_RunNo_QA", "<number of pair candidates> vs. run number", kTRUE,
                        runNBins, runHistRange[0], runHistRange[1],   AliReducedVarManager::kRunID,
                        10, 0., 10.,                                  AliReducedVarManager::kNpairsSelected,
                        0, 0, 0,                                      AliReducedVarManager::kNothing,
                        runNumbers.Data());

      // tracking status
      man->AddHistogram(classStr.Data(), "EventAverageTPCchi2", "Average TPC chi2 per track", kFALSE,
                        100,0.,5.,                                    AliReducedVarManager::kEvAverageTPCchi2);
      man->AddHistogram(classStr.Data(), "EventAverageTPCchi2_NTPCout", "Average TPC chi2 per track vs n TPCout", kFALSE,
                        100,0.,5.,                                    AliReducedVarManager::kEvAverageTPCchi2,
                        500, 0.0, 15000.,                             AliReducedVarManager::kNTracksPerTrackingStatus+AliReducedVarManager::kTPCout);
      man->AddHistogram(classStr.Data(), "EventAverageTPCchi2_run_QA", "Average TPC chi2 per track vs run number", kTRUE,
                        runNBins, runHistRange[0], runHistRange[1],   AliReducedVarManager::kRunID,
                        100,0.,5.,                                    AliReducedVarManager::kEvAverageTPCchi2,
                        0, 0, 0,                                      -1,
                        runNumbers.Data());

      continue;
    }
    
    //-----------------------------------------------------------------------------------------------------------
    // ITS cluster track histograms
    //-----------------------------------------------------------------------------------------------------------
    if(classStr.Contains("TrackITSclusterMap_")) {
      man->AddHistClass(classStr.Data());
      printf("%s\n",classStr.Data());
      
      man->AddHistogram(classStr.Data(), "ITSlayerHit_Phi", "Hits in the ITS layers vs #varphi", kFALSE,
                        180, 0.0, 6.29,   AliReducedVarManager::kPhi,
                        6, 0.5, 6.5,      AliReducedVarManager::kITSlayerHit);
      man->AddHistogram(classStr.Data(), "ITSlayerHit_Eta", "Hits in the ITS layers vs #eta", kFALSE,
                        100, -1.0, 1.0,   AliReducedVarManager::kEta,
                        6, 0.5, 6.5,      AliReducedVarManager::kITSlayerHit);
      
      continue;
    }

    //-----------------------------------------------------------------------------------------------------------
    // TPC cluster track histograms
    //-----------------------------------------------------------------------------------------------------------
    if(classStr.Contains("TrackTPCclusterMap_")) {
      man->AddHistClass(classStr.Data());
      printf("%s\n",classStr.Data());
      
      man->AddHistogram(classStr.Data(), "TPCclusterMap_Phi", "TPC cluster map vs #varphi", kFALSE,
                        180, 0.0, 6.29,   AliReducedVarManager::kPhi,
                        8, -0.5, 7.5,     AliReducedVarManager::kTPCclusBitFired);
      man->AddHistogram(classStr.Data(), "TPCclusterMap_Eta", "TPC cluster map vs #eta", kFALSE,
                        100, -1.0, 1.0,   AliReducedVarManager::kEta,
                        8, -0.5, 7.5,     AliReducedVarManager::kTPCclusBitFired);
      
      continue;
    }

    //-----------------------------------------------------------------------------------------------------------
    // track status histograms
    //-----------------------------------------------------------------------------------------------------------
    if(classStr.Contains("TrackStatusFlags_")) {
      man->AddHistClass(classStr.Data());
      printf("%s\n",classStr.Data());
      
      TString trkStatusNames = "";
      for(Int_t iflag=0; iflag<AliReducedVarManager::kNTrackingStatus; ++iflag) {
        trkStatusNames += AliReducedVarManager::fgkTrackingStatusNames[iflag];
        trkStatusNames += ";";
      }
      
      man->AddHistogram(classStr.Data(), "TrackingFlags", "Tracking flags", kFALSE,
                        AliReducedVarManager::kNTrackingStatus, -0.5, AliReducedVarManager::kNTrackingStatus-0.5, AliReducedVarManager::kTrackingFlag,
                        0, 0.0, 0.0, AliReducedVarManager::kNothing,
                        0, 0.0, 0.0, AliReducedVarManager::kNothing, trkStatusNames.Data());
      
      continue;
    }

    //-----------------------------------------------------------------------------------------------------------
    // track and associated track histograms
    //-----------------------------------------------------------------------------------------------------------
    if(classStr.Contains("Track_")) {
      man->AddHistClass(classStr.Data());
      printf("%s\n",classStr.Data());
      
      // momenta
      man->AddHistogram(classStr.Data(), "Pt", "p_{T} distribution", kFALSE,
                        1000, 0.0, 50.0,                              AliReducedVarManager::kPt);
      man->AddHistogram(classStr.Data(), "Pt_RunNo_QA", "<p_{T}> vs run number", kTRUE,
                        runNBins, runHistRange[0], runHistRange[1],   AliReducedVarManager::kRunID,
                        1000, 0.0, 50.0,                              AliReducedVarManager::kPt,
                        0, 0, 0,                                      AliReducedVarManager::kNothing,
                        runNumbers.Data());
      
      // angles
      man->AddHistogram(classStr.Data(), "Eta_Phi", "", kFALSE,
                        100, -1.0, 1.0,                               AliReducedVarManager::kEta,
                        100, 0., 6.3,                                 AliReducedVarManager::kPhi);
      man->AddHistogram(classStr.Data(), "Eta_RunNo_QA", "<#eta> vs run number", kTRUE,
                        runNBins, runHistRange[0], runHistRange[1],   AliReducedVarManager::kRunID,
                        1000, -1.5, 1.5,                              AliReducedVarManager::kEta,
                        0, 0, 0,                                      AliReducedVarManager::kNothing,
                        runNumbers.Data());
      man->AddHistogram(classStr.Data(), "Phi_RunNo_QA", "<#varphi> vs run number", kTRUE,
                        runNBins, runHistRange[0], runHistRange[1],   AliReducedVarManager::kRunID,
                        1000, 0.0, 6.3,                               AliReducedVarManager::kPhi,
                        0, 0, 0,                                      AliReducedVarManager::kNothing,
                        runNumbers.Data());
      
      if(classStr.Contains("MCTruth")) {
        man->AddHistogram(classStr.Data(), "PtMC", "p_{T} MC", kFALSE,
                          150, 0., 15.0,            AliReducedVarManager::kPtMC);
        man->AddHistogram(classStr.Data(), "PtRec_PtMC", "p_{T} MC vs p_{T} reconstructed", kFALSE,
                          150, 0., 15.0,            AliReducedVarManager::kPtMC,
                          150, 0., 15.0,            AliReducedVarManager::kPt);
        man->AddHistogram(classStr.Data(), "PhiMC", "#varphi MC", kFALSE,
                          180, 0., 6.3,             AliReducedVarManager::kPhiMC);
        man->AddHistogram(classStr.Data(), "PhiRec_PhiMC", "#varphi MC vs #varphi reconstructed", kFALSE,
                          180, 0., 6.3,             AliReducedVarManager::kPhiMC,
                          180, 0., 6.3,             AliReducedVarManager::kPhi);
        man->AddHistogram(classStr.Data(), "EtaMC", "#eta MC", kFALSE,
                          100, -1.0, 1.0,           AliReducedVarManager::kEtaMC);
        man->AddHistogram(classStr.Data(), "EtaRec_EtaMC", "#eta MC vs #eta reconstructed", kFALSE,
                          100, -1.0, 1.0,           AliReducedVarManager::kEtaMC,
                          100, -1.0, 1.0,           AliReducedVarManager::kEta);
      }
      
      // skip rest for associated (i.e. base) tracks
      if (classStr.Contains("Associated")) continue;
      
      // DCA
      man->AddHistogram(classStr.Data(), "DCAxy_Pt", "DCAxy", kFALSE,
                        100, -2.0, 2.0,                               AliReducedVarManager::kDcaXY,
                        50, 0.0, 5.0,                                 AliReducedVarManager::kPt);
      man->AddHistogram(classStr.Data(), "DCAxy", "DCAxy", kFALSE,
                        200, -5.0, 5.0,                               AliReducedVarManager::kDcaXY);
      man->AddHistogram(classStr.Data(), "DCAxy_RunNo_QA", "<DCAxy> vs run number", kTRUE,
                        runNBins, runHistRange[0], runHistRange[1],   AliReducedVarManager::kRunID,
                        200, -5.0, 5.0,                               AliReducedVarManager::kDcaXY,
                        0, 0, 0,                                      AliReducedVarManager::kNothing,
                        runNumbers.Data());
      man->AddHistogram(classStr.Data(), "DCAz", "DCAz", kFALSE,
                        200, -5.0, 5.0,                               AliReducedVarManager::kDcaZ);
      man->AddHistogram(classStr.Data(), "DCAz_Pt", "DCAz", kFALSE,
                        100, -3.0, 3.0,                               AliReducedVarManager::kDcaZ,
                        50, 0.0, 5.0,                                 AliReducedVarManager::kPt);
      man->AddHistogram(classStr.Data(), "DCAz_RunNo_QA", "<DCAz> vs run number", kTRUE,
                        runNBins, runHistRange[0], runHistRange[1],   AliReducedVarManager::kRunID,
                        200, -5.0, 5.0,                               AliReducedVarManager::kDcaZ,
                        0, 0, 0,                                      AliReducedVarManager::kNothing,
                        runNumbers.Data());
      
      // ITS
      man->AddHistogram(classStr.Data(), "ITSncls_RunNo_QA", "<ITS nclusters> vs run number", kTRUE,
                        runNBins, runHistRange[0], runHistRange[1],   AliReducedVarManager::kRunID,
                        7, -0.5, 6.5,                                 AliReducedVarManager::kITSncls,
                        0, 0, 0,                                      AliReducedVarManager::kNothing,
                        runNumbers.Data());
      man->AddHistogram(classStr.Data(), "ITSnclsShared_RunNo_QA", "<ITS nclusters shared> vs run number", kTRUE,
                        runNBins, runHistRange[0], runHistRange[1],   AliReducedVarManager::kRunID,
                        7, -0.5, 6.5,                                 AliReducedVarManager::kITSnclsShared,
                        0, 0, 0,                                      AliReducedVarManager::kNothing,
                        runNumbers.Data());
      man->AddHistogram(classStr.Data(), "ITSchi2_RunNo_QA", "<ITS #chi^{2}> vs run number", kTRUE,
                        runNBins, runHistRange[0], runHistRange[1],   AliReducedVarManager::kRunID,
                        200, 0.0, 20.0,                               AliReducedVarManager::kITSchi2,
                        0, 0, 0,                                      AliReducedVarManager::kNothing,
                        runNumbers.Data());
      
      // TPC
      man->AddHistogram(classStr.Data(),  "TPCncls_RunNo_QA", "<TPC #cls> vs run number", kTRUE,
                        runNBins, runHistRange[0], runHistRange[1],   AliReducedVarManager::kRunID,
                        10, 0., 10.,                                  AliReducedVarManager::kTPCncls,
                        0, 0, 0,                                      AliReducedVarManager::kNothing,
                        runNumbers.Data());
      man->AddHistogram(classStr.Data(),  "TPCnclsShared_RunNo_QA", "", kTRUE,
                        runNBins, runHistRange[0], runHistRange[1],   AliReducedVarManager::kRunID,
                        160, -0.5, 159.5,                             AliReducedVarManager::kTPCnclsShared,
                        0, 0, 0,                                      AliReducedVarManager::kNothing,
                        runNumbers.Data());
      man->AddHistogram(classStr.Data(),  "TPCnclsSharedRatio_RunNo_QA", "", kTRUE,
                        runNBins, runHistRange[0], runHistRange[1],   AliReducedVarManager::kRunID,
                        100, 0., 1.,                                  AliReducedVarManager::kTPCnclsSharedRatio,
                        0, 0, 0,                                      AliReducedVarManager::kNothing,
                        runNumbers.Data());
      man->AddHistogram(classStr.Data(), "TPCchi2_RunNo_QA", "TPC <#chi^{2}> vs run number", kTRUE,
                        runNBins, runHistRange[0], runHistRange[1],   AliReducedVarManager::kRunID,
                        10, 0., 10.,                                  AliReducedVarManager::kTPCchi2,
                        0, 0, 0,                                      AliReducedVarManager::kNothing,
                        runNumbers.Data());
      man->AddHistogram(classStr.Data(), "TPCsignal_Pin", "TPC dE/dx vs. inner param P", kFALSE,
                        300, 0.0, 30.0,                               AliReducedVarManager::kPin,
                        200, -0.5, 199.5,                             AliReducedVarManager::kTPCsignal);
      man->AddHistogram(classStr.Data(), "TPCsignal_Pin_Profile", "TPC dE/dx vs. inner param P", kTRUE,
                        300, 0.0, 30.0,                               AliReducedVarManager::kPin,
                        200, -0.5, 199.5,                             AliReducedVarManager::kTPCsignal);
      man->AddHistogram(classStr.Data(), "TPCsignalN_RunNumber_QA", "TPC <#cls pid> vs run number", kTRUE,
                        runNBins, runHistRange[0], runHistRange[1],   AliReducedVarManager::kRunID,
                        160, -0.5, 159.5,                             AliReducedVarManager::kTPCsignalN,
                        0, 0, 0,                                      AliReducedVarManager::kNothing,
                        runNumbers.Data());
      man->AddHistogram(classStr.Data(), "TPCnsigElectron_RunNo_QA", "<TPC N_{#sigma} electron> vs. run number", kFALSE,
                        runNBins, runHistRange[0], runHistRange[1],   AliReducedVarManager::kRunID,
                        100, -5.0, 5.0,                               AliReducedVarManager::kTPCnSig+AliReducedVarManager::kElectron,
                        0, 0, 0,                                      AliReducedVarManager::kNothing,
                        runNumbers.Data());
      man->AddHistogram(classStr.Data(), "TPCnsigElectron_Pin", "TPC N_{#sigma} electron vs. inner param P", kFALSE,
                        100, 0.0, 10.0,                               AliReducedVarManager::kPin,
                        100, -5.0, 5.0,                               AliReducedVarManager::kTPCnSig+AliReducedVarManager::kElectron);
      man->AddHistogram(classStr.Data(), "TPCnsigElectron_Pin_Profile", "TPC N_{#sigma} electron vs. inner param P", kTRUE,
                        100, 0.0, 10.0,                               AliReducedVarManager::kPin,
                        100, -5.0, 5.0,                               AliReducedVarManager::kTPCnSig+AliReducedVarManager::kElectron);
      man->AddHistogram(classStr.Data(), "TPCnsigElectron_Eta", "TPC N_{#sigma} electron vs. #eta", kFALSE,
                        36, -0.9, 0.9,                                AliReducedVarManager::kEta,
                        100, -5.0, 5.0,                               AliReducedVarManager::kTPCnSig+AliReducedVarManager::kElectron);
      man->AddHistogram(classStr.Data(), "TPCnsigElectron_Eta_Profile", "TPC N_{#sigma} electron vs. #eta", kTRUE,
                        36, -0.9, 0.9,                                AliReducedVarManager::kEta,
                        100, -5.0, 5.0,                               AliReducedVarManager::kTPCnSig+AliReducedVarManager::kElectron);
      man->AddHistogram(classStr.Data(), "TPCnsigElectron_Run", "TPC N_{#sigma} electron vs. run", kTRUE,
                        runNBins, runHistRange[0], runHistRange[1],   AliReducedVarManager::kRunID,
                        100, -5.0, 5.0,                               AliReducedVarManager::kTPCnSig+AliReducedVarManager::kElectron,
                        0, 0, 0,                                      AliReducedVarManager::kNothing,
                        runNumbers.Data());
      man->AddHistogram(classStr.Data(), "TPCnsigPion_Run", "TPC N_{#sigma} pion vs. run", kTRUE,
                        runNBins, runHistRange[0], runHistRange[1],   AliReducedVarManager::kRunID,
                        200, -10.0, 10.0,                             AliReducedVarManager::kTPCnSig+AliReducedVarManager::kPion,
                        0, 0, 0,                                      AliReducedVarManager::kNothing,
                        runNumbers.Data());
      man->AddHistogram(classStr.Data(), "TPCnsigProton_Run", "TPC N_{#sigma} proton vs. run", kTRUE,
                        runNBins, runHistRange[0], runHistRange[1],   AliReducedVarManager::kRunID,
                        200, -10.0, 10.0,                             AliReducedVarManager::kTPCnSig+AliReducedVarManager::kProton,
                        0, 0, 0,                                      AliReducedVarManager::kNothing,
                        runNumbers.Data());

      // MC quantities
      if(classStr.Contains("MCTruth")) {
        man->AddHistogram(classStr.Data(), "PDGcode0", "PDG code of the track", kFALSE,
                          12000, -6000., 6000.0,    AliReducedVarManager::kPdgMC);
        man->AddHistogram(classStr.Data(), "PDGcode1", "PDG code of the track's mother", kFALSE,
                          12000, -6000., 6000.0,    AliReducedVarManager::kPdgMC+1);
        man->AddHistogram(classStr.Data(), "PDGcode2", "PDG code of the track's grand-mother", kFALSE,
                          12000, -6000., 6000.0,    AliReducedVarManager::kPdgMC+2);
        man->AddHistogram(classStr.Data(), "PDGcode3", "PDG code of the track's grand-grand mother", kFALSE,
                          12000, -6000., 6000.0,    AliReducedVarManager::kPdgMC+3);
      }
      
      continue;
    }

    //-----------------------------------------------------------------------------------------------------------
    // pair histograms
    //-----------------------------------------------------------------------------------------------------------
    if(classStr.Contains("Pair")) {
      man->AddHistClass(classStr.Data());
      printf("%s\n",classStr.Data());
      
      // variables
      const Int_t kNVars = 3; //5
      Int_t vars[kNVars] = {AliReducedVarManager::kMass, AliReducedVarManager::kPt, AliReducedVarManager::kRapAbs}; // AliReducedVarManager::kVtxZ, AliReducedVarManager::kSPDntracklets};
      TArrayD pairHistBinLimits[kNVars];
      pairHistBinLimits[0] = TArrayD(kNMassBins,massBins);
      pairHistBinLimits[1] = TArrayD(kNPtBins,ptBins);
      pairHistBinLimits[2] = TArrayD(kNRapidityBins,rapidityBins);
      //pairHistBinLimits[3] = TArrayD(gNVtxZLimits,gVtxZLimits);
      //pairHistBinLimits[4] = TArrayD(gNMultLimits,gMultLimits);

      // histograms
      man->AddHistogram(classStr.Data(), "PairInvMass", "Differential pair inv. mass",
                        kNVars, vars, pairHistBinLimits);
      man->AddHistogram(classStr.Data(), "PairType", "Pair type", kFALSE,
                        4, -0.5, 3.5,                                 AliReducedVarManager::kPairType);
      man->AddHistogram(classStr.Data(), "Mass", "Invariant mass", kFALSE,
                        125, 0.0, 5.0,                                AliReducedVarManager::kMass);
      man->AddHistogram(classStr.Data(), "Mass_RunNo_QA", "<Invariant mass> vs run number", kTRUE,
                        runNBins, runHistRange[0], runHistRange[1],   AliReducedVarManager::kRunID,
                        125, 0.0, 5.0,                                AliReducedVarManager::kMass,
                        0, 0, 0,                                      AliReducedVarManager::kNothing,
                        runNumbers.Data());
      man->AddHistogram(classStr.Data(), "Pt", "", kFALSE,
                        1000, 0.0, 10.0,                              AliReducedVarManager::kPt);
      man->AddHistogram(classStr.Data(), "Pt_coarse", "", kFALSE,
                        20, 0.0, 20.0,                                AliReducedVarManager::kPt);
      man->AddHistogram(classStr.Data(), "Rapidity", "Rapidity", kFALSE,
                        240, -1.2, 1.2,                               AliReducedVarManager::kRap);
      man->AddHistogram(classStr.Data(), "Phi", "Azimuthal distribution", kFALSE,
                        315, 0.0, 6.3,                                AliReducedVarManager::kPhi);
      man->AddHistogram(classStr.Data(), "Leg1TPCchi2_Leg2TPCchi2", "", kFALSE,
                        100, 0.0, 6.0,                                AliReducedVarManager::kPairLegTPCchi2,
                        100, 0.0, 6.0,                                AliReducedVarManager::kPairLegTPCchi2+1);
      man->AddHistogram(classStr.Data(), "Leg1ITSchi2_Leg2ITSchi2", "", kFALSE,
                        100, 0.0, 6.0,                                AliReducedVarManager::kPairLegITSchi2,
                        100, 0.0, 6.0,                                AliReducedVarManager::kPairLegITSchi2+1);
      
      // MC histograms
      if(classStr.Contains("True")) {
        man->AddHistogram(classStr.Data(), "PtMC", "p_{T} MC", kFALSE,
                          1000, 0., 10.0,   AliReducedVarManager::kPtMC);
        man->AddHistogram(classStr.Data(), "PtMC_coarse", "p_{T} MC", kFALSE,
                          20, 0., 20.0,     AliReducedVarManager::kPtMC);
        man->AddHistogram(classStr.Data(), "MassMC_Mass", "Invariant mass, MC vs reconstructed", kFALSE,
                          150, 2.0, 3.5,    AliReducedVarManager::kMass,
                          150, 2.0, 3.5,    AliReducedVarManager::kMassMC);
        man->AddHistogram(classStr.Data(), "PtMC_Pt", "pair pT, MC vs reconstructed", kFALSE,
                          150, 0.0, 15.,    AliReducedVarManager::kPt,
                          150, 0.0, 15.0,   AliReducedVarManager::kPtMC);
        man->AddHistogram(classStr.Data(), "PtMC_MassMC", "", kFALSE,
                          100, 0.0, 1.,     AliReducedVarManager::kPtMC,
                          30, 2.7, 3.3,     AliReducedVarManager::kMassMC);
        man->AddHistogram(classStr.Data(), "Pt_Mass", "", kFALSE,
                          100, 0.0, 1.,     AliReducedVarManager::kPt,
                          30, 2.7, 3.3,     AliReducedVarManager::kMass);
        man->AddHistogram(classStr.Data(), "PtMC_Mass", "", kFALSE,
                          100, 0.0, 1.,     AliReducedVarManager::kPtMC,
                          30, 2.7, 3.3,     AliReducedVarManager::kMass);
        man->AddHistogram(classStr.Data(), "Pt_MassMC", "", kFALSE,
                          100, 0.0, 1.,     AliReducedVarManager::kPt,
                          30, 2.7, 3.3,     AliReducedVarManager::kMassMC);
        man->AddHistogram(classStr.Data(), "PtMC_Pt_lowPtZoom", "pair pT, MC vs reconstructed", kFALSE,
                          100, 0.0, 0.5,    AliReducedVarManager::kPt,
                          100, 0.0, 0.5,    AliReducedVarManager::kPtMC);
        man->AddHistogram(classStr.Data(), "PtMC_Pt_Mass", "pair pT, MC vs reconstructed, vs mass", kFALSE,
                          100, 0.0, 0.5,    AliReducedVarManager::kPt,
                          100, 0.0, 0.5,    AliReducedVarManager::kPtMC,
                          15, 2.72, 3.32,   AliReducedVarManager::kMass);
        man->AddHistogram(classStr.Data(), "PtMC_Pt_Mass_coarse", "pair pT, MC vs reconstructed, vs mass", kFALSE,
                          25, 0.0, 0.5,     AliReducedVarManager::kPt,
                          25, 0.0, 0.5,     AliReducedVarManager::kPtMC,
                          15, 2.72, 3.32,   AliReducedVarManager::kMass);
      }
      
      continue;
    }

    //-----------------------------------------------------------------------------------------------------------
    // correlation histograms
    //-----------------------------------------------------------------------------------------------------------
    if(classStr.Contains("Corr")) {
      man->AddHistClass(classStr.Data());
      printf("%s\n",classStr.Data());

      // pt binning associated hadron
      const Int_t kNPtAssocBins = 7;
      Double_t ptAssocBins[kNPtAssocBins] = {0.15, 0.3, 1.0, 3.0, 5.0, 10.0, 30.0};

      // delta phi binning
      const Int_t kNDeltaPhiBins = 9; // [0, pi]
      Double_t deltaPhiBins[kNDeltaPhiBins];
      for (Int_t i=0; i<kNDeltaPhiBins; i++) deltaPhiBins[i] = i*TMath::Pi()/8.;  // bin width ~ 0.4

      // delta eta binning
      const Int_t kNDeltaEtaBins = 14;
      Double_t deltaEtaBins[kNDeltaEtaBins];
      for (Int_t i=0; i<kNDeltaEtaBins; i++) deltaEtaBins[i] = -2.6+i*0.4; // bin width = 0.4
      
      // variables
      const Int_t kNVarsCorr = 6;
      Int_t varsCorr[kNVarsCorr] = {AliReducedVarManager::kMass, AliReducedVarManager::kTriggerRapAbs, AliReducedVarManager::kTriggerPt, AliReducedVarManager::kAssociatedPt, AliReducedVarManager::kDeltaPhiSym, AliReducedVarManager::kDeltaEta};
      TArrayD corrHistBinLimits[kNVarsCorr];
      corrHistBinLimits[0] = TArrayD(kNMassBinsCorr,massBinsCorr);
      corrHistBinLimits[1] = TArrayD(kNRapidityBins,rapidityBins);
      corrHistBinLimits[2] = TArrayD(kNPtBins,ptBins);
      corrHistBinLimits[3] = TArrayD(kNPtAssocBins,ptAssocBins);
      corrHistBinLimits[4] = TArrayD(kNDeltaPhiBins,deltaPhiBins);
      corrHistBinLimits[5] = TArrayD(kNDeltaEtaBins,deltaEtaBins);
      
      // correlation histogram
      man->AddHistogram(classStr.Data(), "CorrHist", "Differential correlation histogram",
                        kNVarsCorr, varsCorr, corrHistBinLimits);

      // J/psi candidates
      man->AddHistogram(classStr.Data(), "NJpsiCandidates", "number of J/psi candidates per event", kFALSE,
                        100, 0., 100.,                                AliReducedVarManager::kNpairsSelected);
      man->AddHistogram(classStr.Data(), "NJpsiCandidates_RunNo_QA", "<number of J/psi candidates> vs. run number", kTRUE,
                        runNBins, runHistRange[0], runHistRange[1],   AliReducedVarManager::kRunID,
                        100, 0., 100.,                                AliReducedVarManager::kNpairsSelected,
                        0, 0, 0,                                      AliReducedVarManager::kNothing,
                        runNumbers.Data());
      
      // associated tracks
      man->AddHistogram(classStr.Data(), "NAssociatedTracks", "number of associated tracks per event", kFALSE,
                        1000, 0., 1000.,                              AliReducedVarManager::kNtracksAnalyzed);
      man->AddHistogram(classStr.Data(), "NAssociatedTracks_RunNo_QA", "<number of associated tracks> vs. run number", kTRUE,
                        runNBins, runHistRange[0], runHistRange[1],   AliReducedVarManager::kRunID,
                        1000, 0., 1000.,                              AliReducedVarManager::kNtracksAnalyzed,
                        0, 0, 0,                                      AliReducedVarManager::kNothing,
                        runNumbers.Data());

      continue;
    }
  }
}
