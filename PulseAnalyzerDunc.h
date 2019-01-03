#include <TFile.h>

#ifndef PULSE_ANALYZER_H
#define PULSE_ANALYZER_H

#define NUM_CHANNEL 4
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
#include "RooRealVar.h"
#include "RooArgSet.h"
#include "RooDataSet.h"
#include "TTreeReader.h"
#include "TTreeReaderValue.h"
#include "RooDataHist.h"
#include "RooHistPdf.h"
#include "RooPlot.h"
#include "RooAbsPdf.h"
#include "RooBinning.h"


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
       TBranch* amp_br;
       main_tree->SetBranchStatus("*", 0);
       main_tree->SetBranchStatus("channel", 1);
       main_tree->SetBranchStatus("amp", 1);
       main_tree->SetBranchStatus("time", 1);
       main_tree->SetBranchStatus("gaus_mean",1);
       main_tree->SetBranchAddress("channel", channel, &channel_br);
       main_tree->SetBranchAddress("amp", amp, &amp_br);
       main_tree->SetBranchAddress("time", time, &time_br);
       main_tree->SetBranchAddress("gaus_mean", gaus_mean, &gaus_br);
       
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

    void ApplyAmpCuts(int dut_ch, int pho_ch, float dut_min, float dut_max, float pho_min, float pho_max){

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
	
	void ApplyDeltaTCuts(){
	   MakeTimeVector(GetGoodEntry());
       MakeDeltaTCuts(GetTimeVector());
       
    }  
    
    void PlotTimeRes(){
       TimeResFit(GetFinalVector());
    }
    
    
    

  private:

    TTree * main_tree; TFile * input_file;
    int dut_channel; float ptk_channel;
    float dut_min; float dut_max;
    float ptk_min; float ptk_max;
    TString amp_cut_string;

    float channel[NUM_CHANNEL][NUM_SAMPLES];
    float amp[NUM_CHANNEL];
    float time[1][NUM_SAMPLES];
    float gaus_mean[NUM_CHANNEL];

    vector<int> good_entries;
    vector<TH1F> hist_vec;
    TEventList* good_entry_list = NULL;
    int total_good_entries = -999;
    vector<float> good_time_samples;
    vector<float> final_time_samples;
    int final_counter;
    float approx_mean;

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
        cout << amp_cut_string << endl;
        main_tree->Draw(">>list",amp_cut_string);
        good_entry_list = (TEventList*)gDirectory->Get("list");
      }
      else cout << "No ROOT file opened!" << endl;
    }

    void SelectGoodEntries(){
      if(good_entries.size()>0) good_entries.clear();

      total_good_entries = good_entry_list->GetN();

      cout << "Entries in selection: " << total_good_entries  << endl;

      for(int i = 0; i < total_good_entries; i++){
        good_entries.push_back(good_entry_list->GetEntry(i));
      }
    }
    
    vector<int> GetGoodEntry(){return good_entries;}
    
    void MakeTimeVector(std::vector<int> entries){
      for(int p = 0; p < entries.size(); p++){
        main_tree->GetEntry(entries[p]);
        //cout << list->GetEntry(i) << endl; 
        float time_temp = gaus_mean[0]-gaus_mean[2];
        good_time_samples.push_back(time_temp);
      }
    }
    
    vector<float> GetTimeVector(){
		return good_time_samples;
	}
    
    double FindMean(std::vector<float> list){
    vector<int> count;
    vector<float> countval;
    vector<std::vector<float>> countvec;

    for(int i = 0; i < list.size(); i++)
    {
      for(int j = 0; j < list.size(); j++)
      {
	    if (j != i && fabs(list[j]-list[i]) < 0.1)
	    {
	       countval.push_back(list[j]);
	  	}	
      }						
      countvec.push_back(countval);
      count.push_back(countval.size());
      countval.clear();
    }
   	

    int max = *max_element(count.begin(), count.end());
    float avg = 0.0;
    
    for(int i = 0; i < count.size(); i++)
    {
      float sum = 0;
      if (count[i] == max)
      {
        for(int j = 0; j < countvec[i].size(); j++)
        {
	      sum += countvec[i][j];
	    }
	   avg = sum/countvec[i].size();
	   break;
      }
    }
    return avg;
   }
   
   void MakeDeltaTCuts(std::vector<float> samples)
   {
    approx_mean = FindMean(samples);
    cout << "Mean = " << approx_mean << endl;
       for (int l = 0; l<samples.size(); l++)
       {
       	if(fabs(samples[l] - approx_mean)<0.25)
       	{
       	  final_time_samples.push_back(good_time_samples[l]);
       	}
       }
    cout << "There are " << final_time_samples.size() << " data points within 250 ps of the mean.";
   }
   
   vector<float> GetFinalVector(){
		return final_time_samples;
	}
   
   void TimeResFit(std::vector<float> times)
   {
   	 RooRealVar Time("Time","Time",2.0, 3.0);
     RooRealVar Mean("Mean","Mean",2.2, 2.6);
     RooRealVar Sigma("Sigma","Sigma",0.25, 0.00, 5.00);
     RooDataSet TimeDifData("TimeDifData","TimeDifData",RooArgSet(Time));
     for(int k=0; k<times.size(); k++)
      {
		Time.setVal(times[k]);
		TimeDifData.add(RooArgSet(Time));
      }
     RooDataHist * h1 = new RooDataHist("h1","h1",RooArgSet(Time),TimeDifData,1.0);
     RooGaussian f1("f1","f1",Time,Mean,Sigma);
     f1.fitTo(TimeDifData);
     RooPlot * p1 = Time.frame(RooFit::Range(approx_mean-0.25,approx_mean+0.25));
     int bins = 50;
     RooBinning tbins(approx_mean-0.25,approx_mean+0.25);
     tbins.addUniform(bins,approx_mean-0.25,approx_mean+0.25);
     h1->plotOn(p1,RooFit::Binning(tbins));
     f1.plotOn(p1);
     p1->Draw();
   }
};
   

#endif
