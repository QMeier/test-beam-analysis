#include <iostream>
#include "/home/pacejohn/builddir/include/TFile.h"
#include "/home/pacejohn/builddir/include/TH1.h"
#include "/home/pacejohn/builddir/include/TTree.h"
#include "/home/pacejohn/builddir/include/TTreeReader.h"
#include "/home/pacejohn/builddir/include/TTreeReaderValue.h"
#include "/home/pacejohn/builddir/include/TBranch.h"
#include "/home/pacejohn/builddir/include/TEventList.h"
#include "/home/pacejohn/builddir/include/RooRealVar.h"
#include "/home/pacejohn/builddir/include/RooPlot.h"
#include "/home/pacejohn/builddir/include/RooDataSet.h"

class PulseAnalysis{

public:

PulseAnalysis() {};

virtual ~PulseAnalysis() {};

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

void AmpCuts(float test_min, float test_max, float cont_min, float cont_max){
  amp_cut_string = FormatAmpCuts(test_min, test_max, test_ch);
  amp_cut_string += " && ";
  amp_cut_string += FormatAmpCuts(cont_min, cont_max, cont_ch);

  cout<<"Snippy snippy cut cut: "<< amp_cut_string<<endl;
}

void SetGoodPulses(){
  MakeGoodEntryList();
  /* MakeGoodEntryHist();*/
  pulse->SetEventList(good_entry_list);
  first_cut = false;
  second_cut = true;
}

/*void MakeGoodEventTree(){
  good_file = new TFile("good_tree.root", "RECREATE");
  good_tree = pulse->CopyTree("");
  good_tree->Write(); 
  fOpen->cd();
}*/

void DrawDT(){
  pulse->Draw(FormatDTDraw());
}

void SelectGoodEntries(){
  //good_file->cd();

  if (good_entry_evt.size()>0){good_entry_evt.clear();}
  if (good_entry_DT.size()>0){good_entry_DT.clear();}
  if (good_entry_amp.size()>0){good_entry_amp.clear();}

  float gaus_mean[4];
  float amp[4];

  TBranch* gausbranch;
  TBranch* ampbranch;

  pulse->SetBranchAddress("gaus_mean", gaus_mean, &gausbranch);
  pulse->SetBranchAddress("amp", amp, &ampbranch);

  int total_good = good_entry_list->GetN();
  
  cout<<"Good entries: "<<total_good<<endl;

  for(int i = 0; i < total_good ; i++){
    pulse->GetEntry(good_entry_list->GetEntry(i));
    float DT_temp = gaus_mean[cont_ch] - gaus_mean[test_ch];
    float amp_temp = amp[test_ch];

    good_entry_DT.push_back(DT_temp);
    good_entry_amp.push_back(amp_temp);
    good_entry_evt.push_back(good_entry_list->GetEntry(i));
  }
}  

  /*Finds mean value of most probable interval. Accounts for the readings that pass the amp cuts but have absurd DT values.*/
void FindMean(){
vector<int> event_count;
vector<float> values;
vector<std::vector<float>> vector_holder;

for (int i = 0; i < good_entry_DT.size(); i++){
  for(int j = 0; j < good_entry_DT.size(); j++){
    if (j != i && fabs(good_entry_DT[j] - good_entry_DT[i] < .1)){
      values.push_back(good_entry_DT[j]);
    }  
  }

  vector_holder.push_back(values);
  event_count.push_back(values.size());
  values.clear();
  }

  int most_probable = *max_element(event_count.begin(), event_count.end());
  float avg = 0;

  for(int i = 0; i < event_count.size(); i++){
    float sum = 0;
    if (event_count[i] == most_probable){
      for (int j = 0; j < vector_holder[i].size(); j++){
        sum += vector_holder[i][j];
      }

    avg = sum/vector_holder[i].size();
    break;
    }
  }

cout<<"Mean value: "<<avg<<endl;
mean = avg;
}

void FinalCuts(){
  final_cut_string = FormatFinalCuts();
  cout<<"Final cut: "<< final_cut_string <<endl;
}

void GenerateDTDataSet(){
  RooRealVar DT("DT", "DT", mean, mean - .25, mean + .25);
  DT_DS = new RooDataSet("DT", "DT", RooArgSet(DT));

  for (int i = 0; i < good_entry_DT.size(); i++){
    DT = good_entry_DT[i];
    DT_DS->add(DT);
  }
}

private:

TTree* pulse;
TFile* fOpen;

TTree* good_tree;
TFile* good_file;

TEventList* good_entry_list;

vector<int> good_entry_evt;
vector<float> good_entry_DT;
vector<float> good_entry_amp;

RooDataSet* DT_DS;
  
int test_ch;
int cont_ch;
float test_min;
float test_max;
float cont_min;
float cont_max;

float mean = 0;

bool first_cut = true;
bool second_cut = false;


/*Root is stupid and you need to pass it a TString pseudo-boolean when making cuts on your Draw commands*/

TString amp_cut_string = "";
TString final_cut_string = "";

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

TString FormatFinalCuts(){
   TString final_cut_form = amp_cut_string;
   TString DT_cut_form = " && ((gaus_mean[%s] - gaus_mean[%s]) > (%s - .25) && (gaus_mean[%s] - gaus_mean[%s]) < (%s + .25))";
   TString cont_ch_str, test_ch_str, mean_str, output;

   final_cut_form += DT_cut_form;

   cont_ch_str.Form("%i", cont_ch);
   test_ch_str.Form("%i", test_ch);
   mean_str.Form("%f", mean);

   output.Form(final_cut_form, cont_ch_str.Data(), test_ch_str.Data(), mean_str.Data(), cont_ch_str.Data(), test_ch_str.Data(), mean_str.Data());

  return output;  
}

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

  if (first_cut == true){
    pulse->Draw(">>list1", amp_cut_string);
    good_entry_list = (TEventList*)gDirectory->Get("list1");
  }

  else if (second_cut == true){
    pulse->Draw(">>list2", final_cut_string);
    good_entry_list = (TEventList*)gDirectory->Get("list2");
  }
}

  /*void MakeGoodEntryHist(){
  pulse->Draw(">>hist", amp_cut_string);
  good_entry_hist = (TH1F*)gDirectory->Get("hist");
  }*/
 
};
