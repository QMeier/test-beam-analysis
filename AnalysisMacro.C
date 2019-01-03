#include "/home/pacejohn/builddir/macros/PulseAnalysis.hh"

void AnalysisMacro(){
  int cont_ch = 0;
  int test_ch = 2;
  float cont_low = 0;
  float cont_high = 0;
  float test_low = 0;
  float test_high = 0;

  PulseAnalysis test;

  test.ImportFile("DataNetScope_Run1276.root", "pulse");
  test.OptimizeTree();

  cout<<"Control channel? : ";
  cin>>cont_ch;
  cout<<"Test channel? : ";
  cin>>test_ch;

  test.SetControlChannel(cont_ch);
  test.SetTestChannel(test_ch);

  cout<<"Control lower amp bound? : ";
  cin>>cont_low;
  cout<<"Control upper amp bound? : ";
  cin>>cont_high;
  cout<<"Test lower amp bound? : ";
  cin>>test_low;
  cout<<"Test upper amp bound? : ";
  cin>>test_high;

  test.AmpCuts(test_low, test_high, cont_low, cont_high);
  test.SetGoodPulses();
  test.SelectGoodEntries();
  test.FindMean();
  test.FinalCuts();
  test.SetGoodPulses();
  test.SelectGoodEntries(); 
  test.GenerateDTDataSet();
}
