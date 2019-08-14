void AddTaskJpsiQA(Bool_t doJpsiFilter, TString datasets="16l_pass1"){
  if(!doJpsiFilter)
    gROOT->LoadMacro("ConfigJpsi_cj_pp.C");
  cout << "[-] Add task : MultiDie_0 for kINT7/MB" << endl;
  gInterpreter->ExecuteMacro("AddTask_cjahnke_JPsi.C(0)");
  if(datasets.Contains("16l")){
    cout << "[-] Add task : MultiDie_6 for kEMCEGA EG1" << endl;
    gInterpreter->Execute("AddTask_cjahnke_JPsi","6");
  }
  else
  {
    cout << "[-] Add task : MultiDie_3 for kEMCEGA EG1" << endl;
    gInterpreter->Execute("AddTask_cjahnke_JPsi","3");
  }
  cout << "[-] Add task : MultiDie_4 for kEMCEGA EG2" << endl;
  gInterpreter->Execute("AddTask_cjahnke_JPsi","4");
  cout << "[-] Add task : MultiDie_30 for kEMCEGA DG1" << endl;
  gInterpreter->Execute("AddTask_cjahnke_JPsi","30");
  cout << "[-] Add task : MultiDie_40 for kEMCEGA DG2" << endl;
  gInterpreter->Execute("AddTask_cjahnke_JPsi","40");
}
