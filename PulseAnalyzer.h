/** @file PulseAnalyzer.h
 *
 * @brief
 *
 * @par
 */
#include <TFile.h>

#ifndef PULSE_ANALYZER_H
#define PULSE_ANALYZER_H

#define NUM_CHANNEL 3
#define NUM_SAMPLES 1000
#include <iostream>
#include <map>
#include <vector>
#include <math.h>
#include "TFile.h"
#include "TCanvas.h"
#include "TMath.h"
#include "TAxis.h"
#include "TH1.h"
#include "TTree.h"
#include "TRandom.h"
#include "TEventList.h"
#include "TColor.h"


class PulseAnalyzer{

  public:
    PulseAnalyzer(){};
    virtual ~PulseAnalyzer(){};

    void GetTree(TString file_name, TString tree_name){
       input_file = TFile::Open(file_name);
       main_tree = (TTree*)input_file->Get(tree_name);
     }

    void SetBranches(){
       TBranch* channel_br;
       TBranch* time_br;
       TBranch* gaus_br;
       main_tree->SetBranchStatus("*", 0);
       main_tree->SetBranchStatus("amp", 1);
       main_tree->SetBranchStatus("channel", 1);
       main_tree->SetBranchStatus("time", 1);
       //main_tree->SetBranchStatus("gaus_mean",1);
       main_tree->SetBranchAddress("channel",channel, &channel_br);
       main_tree->SetBranchAddress("time", time, &time_br);
       //main_tree->SetBranchAddress("gaus_mean", gaus_mean, &gaus_br);
     }

    /*void SetDutChannel(int ch, TString name){
      if (ch >=0 || ch < NUM_CHANNEL){
        dut_channel = ch;
        dut_name = name;
        cout << endl;
        cout << "DUT Channel set to: " << dut_channel << endl;
        cout << "DUT Name set to: " << dut_name << endl;
      }
      else cout << "Channel " << ch << " is not a valid channel!" << endl;
    }*/

    void SetChannel(int ch){
      if (ch >=0 || ch < NUM_CHANNEL){
        dut_channel = ch;
        cout << "DUT Channel set to: " << dut_channel << endl;
      }
      else cout << "Channel " << ch << " is not a valid channel!" << endl;
    }

    void ApplyAllCuts(int dut_ch, int pho_ch, float dut_min, float dut_max, float pho_min, float pho_max){

       amp_cut_string = FormatAmpCuts(dut_min,dut_max,dut_ch);
       amp_cut_string += " && ";
       TString temp_string = FormatAmpCuts(pho_min,pho_max,pho_ch);
       amp_cut_string += temp_string;

       cout << "Cuts Applied: " << amp_cut_string << endl;

    }


    void SetGoodPulses(){

       SetBranches();
       MakeGoodEntryList();
       SelectGoodEntries();
     }

     vector<int> GetEntryVec(){
       return good_entries;
    }


    void ApplyCuts(float dut_min, float dut_max){
      if(dut_channel >= 0 || dut_channel < NUM_CHANNEL) {
        amp_cut_string=FormatAmpCuts(dut_min,dut_max,dut_channel);
        cout << "Cuts Applied: " << amp_cut_string << endl;
      }
    }

  private:

    TTree * main_tree; TFile * input_file;
    int dut_channel; float ptk_channel;
    float dut_min; float dut_max;
    float ptk_min; float ptk_max;
    TString amp_cut_string;


    float channel[NUM_CHANNEL][NUM_SAMPLES];
    float time[1][NUM_SAMPLES];
    float gaus_mean[NUM_CHANNEL];




    vector<int> good_entries;
    vector<TH1F> hist_vec;
    TEventList* good_entry_list;
    int total_good_entries = -999;

    TString FormatAmpCuts(float xmin, float xmax, int channel){

        TString genericCut = "(amp[%s] > %s && amp[%s] < %s)";
        TString channel_str, xmin_str, xmax_str, fstring;

        channel_str.Form("%i",channel);
        xmin_str.Form("%f",xmin);
        xmax_str.Form("%f",xmax);

        fstring.Form(genericCut, channel_str.Data(), xmin_str.Data(), channel_str.Data(), xmax_str.Data());
        return fstring;
    }


    void MakeGoodEntryList(){
      if(input_file){
        SetBranches();
        main_tree->Draw(">>list",amp_cut_string);
        good_entry_list = (TEventList*)gDirectory->Get("list");
      }
      else cout << "No ROOT file opened!" << endl;
    }

    void SelectGoodEntries(){
      if(good_entries.size()>0) good_entries.clear();

      MakeGoodEntryList();
      total_good_entries = good_entry_list->GetN();

      cout << "Entries in selection: " << total_good_entries  << endl;

      for(int i = 0; i < total_good_entries; i++){
        good_entries.push_back(good_entry_list->GetEntry(i));
      }
    }

    


};



#endif /* PULSE_ANALYZER_H */
/*** end of file ***/
