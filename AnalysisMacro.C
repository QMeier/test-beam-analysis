#define NUM_CHANNEL 4
#define NUM_SAMPLES 1000

#include <iostream>
#include "TString.h"
#include "PulseAnalyzer.h"

int AnalysisMacro(){
  TString filename = "DataNetScope_Run1276.root";
  TString treename = "pulse";
  //cout << "What file would you like to analyze?" << endl;
  //cin >> filename;
  //cout << "Which tree from this file would you like to analyze?" << endl;
  //cin >> treename; 
  PulseAnalyzer Analyzer = PulseAnalyzer();
  Analyzer.GetTree(filename, treename);
  Analyzer.ApplyAmpCuts(0,2,40.0,160.0,30.0,180.0);
  Analyzer.SetGoodPulses();
  Analyzer.ApplyDeltaTCuts();
  Analyzer.PlotTimeRes();
  
  return 0;
}
