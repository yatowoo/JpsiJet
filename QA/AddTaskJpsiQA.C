void AddTaskJpsiQA(){
  cout << "[-] Add task : MultiDie_0 for kINT7/MB" << endl;
  gInterpreter->ExecuteMacro("AddTask_cjahnke_JPsi.C(0)");
  cout << "[-] Task added." << endl;
  cout << "[-] Add task : MultiDie_3 for kEMCEGA EG1" << endl;
  gInterpreter->Execute("AddTask_cjahnke_JPsi","3");
  cout << "[-] Task added." << endl;
  cout << "[-] Add task : MultiDie_4 for kEMCEGA EG2" << endl;
  gInterpreter->Execute("AddTask_cjahnke_JPsi","4");
  cout << "[-] Task added." << endl;
  //gInterpreter->ExecuteMacro("AddTask_cjahnke_JPsi.C(30,kFALSE)");
  //gInterpreter->ExecuteMacro("AddTask_cjahnke_JPsi.C(40,kFALSE)");
}
