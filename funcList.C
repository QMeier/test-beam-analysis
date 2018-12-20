#include <iostream>
#include "/usr/local/Cellar/root/6.14.06/include/root/TFile.h"
#include "/usr/local/Cellar/root/6.14.06/include/root/TH1F.h"
#include "/usr/local/Cellar/root/6.14.06/include/root/TTree.h"
#include "/usr/local/Cellar/root/6.14.06/include/root/TBranch.h"
#include "/usr/local/Cellar/root/6.14.06/include/root/TEventList.h"
#include "/usr/local/Cellar/root/6.14.06/include/root/RooPlot.h"
#include "/usr/local/Cellar/root/6.14.06/include/root/RooDataSet.h"

class someShit{

public:

someShit() {};

virtual ~someShit() {};

void ImportFile(TString filename, TString treename){
   fOpen = TFile::Open(filename);
   pulse = (TTree*)fOpen->Get(treename);
}

void OptimizeTree(){
  pulse->SetBranchStatus("*", 0);
  pulse->SetBranchStatus("amp", 1);
  pulse->SetBranchStatus("channel", 1);
  pulse->SetBranchStatus("gaus_mean", 1);
}

void SetTestChannel(int ch){
  test_ch = ch;
  cout<<"Test channel set to: "<<test_ch<<endl;
  cout<<"Hope that was valid input"<<endl;
}

void SetControlChannel(int ch){
  cont_ch = ch;
  cout<<"Control channel set to: "<<cont_ch<<endl;
  cout<<"Hope that was valid input"<<endl;
}

void InitialAmpCuts(int test_ch, int cont_ch, float test_min, float test_max, float cont_min, float cont_max){
  amp_cut_string = FormatAmpCuts(test_min, test_max, test_ch);
  amp_cut_string += " && ";
  amp_cut_string += FormatAmpCuts(cont_min, cont_max, cont_ch);

  cout<<"Snippy snippy cut cut: "<< amp_cut_string<<endl;
}

void DTCuts(){
  
}

void SetGoodPulses(){
  MakeGoodEntryList();
  MakeGoodEntryHist();
  SelectGoodEntries();
}

vector<int> GetGoodEntryVec(){return good_entry_vec;}

void IsolateGoodPulses(){
  pulse->SetEventList(good_entry_list); 
}

void DrawDT(){
  pulse->Draw(FormatDTDraw());
}  

private:

TTree* pulse;
TFile* fOpen;

int total_good;
TEventList* good_entry_list;
vector<int> good_entry_vec;
TH1F* good_entry_hist;
  
int test_ch;
int cont_ch;
float test_min;
float test_max;
float cont_min;
float cont_max;

RooDataSet good_entry_data("good_entry_data", "good_entry_data", pulse, gaus_mean[0]-gaus_mean[2]);

/*Root is stupid and you need to pass it a TString pseudo-boolean when making cuts on your Draw commands*/

TString amp_cut_string = "";
TString DT_cut_string = "";

/*This function fills the previously declared variable in initialCuts*/
TString FormatAmpCuts(float min, float max, int ch){
  TString cutForm = "(amp[%s] > %s && amp[%s] < %s)";
  TString ch_str, min_str, max_str, output;

  ch_str.Form("%i", ch);
  min_str.Form("%f", min);
  max_str.Form("%f", max);

  output.Form(cutForm, ch_str.Data(), min_str.Data(), ch_str.Data(), max_str.Data());

  return output;    
}

  TString FormatDTCuts(){

TString FormatDTDraw(){
  TString draw_form = "gaus_mean[%s] - gaus_mean[%s]";
    TString cont_str, test_str, output;
  
  cont_str.Form("%i", cont_ch);
  test_str.Form("%i", test_ch);

  output.Form(draw_form, cont_str.Data(), test_str.Data());
  return output;
}

void MakeGoodEntryList(){
  OptimizeTree();
  pulse->Draw(">>list", amp_cut_string);
  good_entry_list = (TEventList*)gDirectory->Get("list");
}

void MakeGoodEntryHist(){
  pulse->Draw(">>hist", amp_cut_string);
  good_entry_hist = (TH1F*)gDirectory->Get("hist");
}

void SelectGoodEntries(){
  if (good_entry_vec.size()>0){good_entry_vec.clear();}

  MakeGoodEntryList();
  total_good = good_entry_list->GetN();
  
  cout<<"Good entries: "<<total_good<<endl;

  for(int i = 0; i < total_good ; i++){
    good_entry_vec.push_back(good_entry_list->GetEntry(i));
  }
}  
 
};
