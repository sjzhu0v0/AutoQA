#ifndef AUTOQA_H
#define AUTOQA_H
#include "MHead.h"
#include "QAFunc.h"

using namespace QAFunc;

class AutoQA {
private:
  TString fPath_file_output;

  TFile *fFile_output;
  vector<TFile *> fVec_file_input;
  TString fPath_output;

  vector<TObject *> fVec_histograms_2_be_processed;
  vector<vector<TObject *>> fVecvec_histograms_processed;
  vector<vector<TObject *>> fVecvec_histograms_results;
  vector<TString> tag_files;
  TClass *fClass_histograms;

  int fIndex_method_process;
  int fIndex_method_rebin;
  vector<int> fIndex_method_show;
  vector<array<int, 2>> fIndex_result; // [index_method, index_hist]

public:
  enum TypeVecObject { kRaw = 0, kProcessed, kResults };

  AutoQA(TString path_file_output, TString path_output);
  ~AutoQA();

  template <typename T>
  void ReadHistograms(TString path_file_input, vector<string> arg);
  template <typename T>
  void ReadHistograms(vector<TString> path_file_input, vector<string> arg);
  void ExcludeHistograms(vector<TString> arg);
  void SetTagFiles(vector<TString> tag_files);
  void SetTagsForSingleFile(vector<TString> tag_files);

  void Rebin(int rebin, int index_method = 0);
  void RebinFixBins(int bins, int index_method = 0);

  void ProcessHistograms(int index_method = 0);

  void ShowHistograms(int type, vector<int> index_method = {},
                      int index_hist = -1);

  void GetResults(int index_method, double *par = nullptr, int index_hist = -1);
  // index_hist: -1 use histograms from the input
};

AutoQA::AutoQA(TString path_file_output, TString path_output)
    : fPath_file_output(path_file_output), fPath_output(path_output) {
  fFile_output = new TFile(path_output + "/" + path_file_output, "RECREATE");
}

AutoQA::~AutoQA() { fFile_output->Close(); }

void AutoQA::SetTagFiles(vector<TString> tag_files) {
  this->tag_files = tag_files;
  if (tag_files.size() != fVec_file_input.size()) {
    cerr << "Number of tag files is not equal to number of input files" << endl;
    exit(1);
  }
}

void AutoQA::SetTagsForSingleFile(vector<TString> tag_files) {
  if (fVec_file_input.size() != 1) {
    cerr << "Number of tag files is not equal to 1" << endl;
    exit(1);
  }
  this->tag_files = tag_files;
}

template <typename T>
void AutoQA::ReadHistograms(TString path_file_input, vector<string> arg) {
  fClass_histograms = T::Class();
  TFile *file_input = new TFile(path_file_input, "READ");
  bool is_file_exist = false;
  for (auto &file : fVec_file_input) {
    if (file->GetPath() == file_input->GetPath()) {
      is_file_exist = true;
      break;
    }
  }
  if (!is_file_exist)
    fVec_file_input.push_back(file_input);

  vector<string> vec_str_name_obj;
  vector<TObject *> vec_obj = GetObjectRecursive(file_input, vec_str_name_obj);

  vector<string> vec_str_name_obj_output;
  vector<TObject *> vec_obj_temp = GetObjectVector<TObject>(
      vec_obj, vec_str_name_obj, vec_str_name_obj_output, arg);
  if (vec_obj_temp.size() == 0) {
    cerr << "No histogram found" << endl;
    exit(1);
  }
  fVec_histograms_2_be_processed.insert(fVec_histograms_2_be_processed.end(),
                                        vec_obj_temp.begin(),
                                        vec_obj_temp.end());
}

template <typename T>
void AutoQA::ReadHistograms(vector<TString> path_file_input,
                            vector<string> arg) {
  fClass_histograms = T::Class();
  vector<TObject *> vec_obj;
  vector<string> vec_str_name_obj;

  if (path_file_input.size() == 0) {
    cerr << "No input file" << endl;
    exit(1);
  }

  int index = 0;
  for (auto &path_file : path_file_input) {
    index++;
    TFile *file_input = new TFile(path_file, "READ");
    if (!file_input->IsOpen()) {
      cerr << "Could not open file " << path_file << endl;
      exit(1);
    }
    bool is_file_exist = false;
    for (auto &file : fVec_file_input) {
      if (file->GetPath() == file_input->GetPath()) {
        is_file_exist = true;
        break;
      }
    }
    if (!is_file_exist)
      fVec_file_input.push_back(file_input);

    vector<string> vec_str_name_obj_temp;
    vector<TObject *> vec_obj_temp =
        GetObjectRecursive(file_input, vec_str_name_obj_temp);
    vec_obj.insert(vec_obj.end(), vec_obj_temp.begin(), vec_obj_temp.end());
    vec_str_name_obj.insert(vec_str_name_obj.end(),
                            vec_str_name_obj_temp.begin(),
                            vec_str_name_obj_temp.end());
  }

  vector<string> vec_str_name_obj_output;
  vector<TObject *> fVec_histograms_2_be_processed_temp =
      GetObjectVector<TObject>(vec_obj, vec_str_name_obj,
                               vec_str_name_obj_output, arg);
  if (fVec_histograms_2_be_processed_temp.size() == 0) {
    cerr << "No histogram found" << endl;
    exit(1);
  }
  fVec_histograms_2_be_processed.insert(
      fVec_histograms_2_be_processed.end(),
      fVec_histograms_2_be_processed_temp.begin(),
      fVec_histograms_2_be_processed_temp.end());
}

void AutoQA::ExcludeHistograms(vector<TString> arg) {
  cout << "Excluding histograms with the following strings:" << endl;
  for (auto &str : arg) {
    cout << str << ", ";
  }
  cout << endl;
  int vec_size = fVec_histograms_2_be_processed.size();
  for (int i = 0; i < vec_size; i++) {
    for (auto &arg_hist : arg) {
      if (((TString)(fVec_histograms_2_be_processed[i]->GetName()))
              .Contains(arg_hist)) {
        fVec_histograms_2_be_processed.erase(
            fVec_histograms_2_be_processed.begin() + i);
        i--;
        vec_size--;
        break;
      }
    }
  }

  if (fVec_histograms_2_be_processed.size() == 0) {
    cerr << "No histogram to be processed" << endl;
    exit(1);
  }

  cout << "Histograms left: " << fVec_histograms_2_be_processed.size() << endl;
  for (auto &hist : fVec_histograms_2_be_processed) {
    cout << hist->GetName() << endl;
  }
}

#define VecRebin(type, class, method)                                          \
  case type:                                                                   \
    for (auto &histogram : fVec_histograms_2_be_processed) {                   \
      class *hist = reinterpret_cast<class *>(histogram);                      \
      hist->method(rebin);                                                     \
    }                                                                          \
    break;

void AutoQA::Rebin(int rebin, int index_method = 0) {
  switch (index_method) {
    VecRebin(k1D, TH1, RebinX);
    VecRebin(k2Dx, TH2, RebinX);
    VecRebin(k2Dy, TH2, RebinY);
    VecRebin(k3Dx, TH3, RebinX);
    VecRebin(k3Dy, TH3, RebinY);
    VecRebin(k3Dz, TH3, RebinZ);
  default:
    cerr << "Invalid index method" << endl;
    exit(1);
  }
}

#define VecRebinFixBins(type, class, coordinate, method)                       \
  case type:                                                                   \
    for (auto &histogram : fVec_histograms_2_be_processed) {                   \
      class *hist = reinterpret_cast<class *>(histogram);                      \
      int nrebin = hist->GetNbins##coordinate() / bins;                        \
      hist->method(nrebin);                                                    \
    }                                                                          \
    break;

void AutoQA::RebinFixBins(int bins, int index_method = 0) {
  switch (index_method) {
    VecRebinFixBins(k1D, TH1, X, RebinX);
    VecRebinFixBins(k2Dx, TH2, X, RebinX);
    VecRebinFixBins(k2Dy, TH2, Y, RebinY);
    VecRebinFixBins(k3Dx, TH3, X, RebinX);
    VecRebinFixBins(k3Dy, TH3, Y, RebinY);
    VecRebinFixBins(k3Dz, TH3, Z, RebinZ);
  default:
    cerr << "Invalid index method" << endl;
    exit(1);
  }
}

// Define the macro
#define SaveVecT(input)                                                        \
  std::vector<TObject *> output;                                               \
  for (auto &obj : input) {                                                    \
    output.push_back(obj);                                                     \
  }                                                                            \
  fVecvec_histograms_processed.push_back(output);

#define Hist2DtoProfile(type, method)                                          \
  if (fIndex_method_process == MethodProcess::type) {                          \
    vector<TH1 *> vec_hist_profile;                                            \
    for (auto &histogram : fVec_histograms_2_be_processed) {                   \
      TH2 *hist_2d = reinterpret_cast<TH2 *>(histogram);                       \
      TH1 *hist_1d = hist_2d->method();                                        \
      vec_hist_profile.push_back(hist_1d);                                     \
    }                                                                          \
    SaveVecT(vec_hist_profile);                                                \
  }

void AutoQA::ProcessHistograms(int index_method) {
  fIndex_method_process = index_method;
  if (fIndex_method_process == k2D_Profilex ||
      fIndex_method_process == k2D_Profiley) {
    int index_hist = 0;
    for (TObject *histogram : fVec_histograms_2_be_processed) {
      TH2 *hist_2d = reinterpret_cast<TH2 *>(histogram);
      int file_index = index_hist / (fVec_histograms_2_be_processed.size() /
                                     fVec_file_input.size());
      index_hist++;
      if (file_index == -1) {
        cerr << "Could not find the file of the histogram" << endl;
        exit(1);
      }
      TString tag_file = tag_files[file_index];
      hist_2d->SetTitle(TString(hist_2d->GetTitle()) + "_" + tag_file);
      hist_2d->SetName(TString(hist_2d->GetName()) + "_" + tag_file);
    }
  }

  Hist2DtoProfile(k2D_Profilex, ProfileX);
  Hist2DtoProfile(k2D_Profiley, ProfileY);
}

#define DefaultShowHist                                                        \
  gStyle->SetOptStat(0);                                                       \
  TCanvas *c = new TCanvas(Form("canvas_%d", GenerateUID()),                   \
                           Form("canvas_%d", GenerateUID()), 800, 600);        \
  int demension_hist = ((TH1 *)histogram)->GetDimension();                     \
  if (demension_hist == 1) {                                                   \
    TH1 *hist = reinterpret_cast<TH1 *>(histogram);                            \
    hist->Draw();                                                              \
  } else if (demension_hist == 2) {                                            \
    TH2 *hist = reinterpret_cast<TH2 *>(histogram);                            \
    hist->Draw("colz");                                                        \
  } else if (demension_hist == 3) {                                            \
    TH3 *hist = reinterpret_cast<TH3 *>(histogram);                            \
    hist->Draw("box");                                                         \
  }                                                                            \
  for (auto &index_method_show : index_method) {                               \
    CanvasSetting(c, histogram, index_method_show);                            \
  }                                                                            \
  TString tag2written = getLastButOnePathComponent(fPath_output);              \
  TLatex *text = new TLatex(0., 0.97, tag2written);                            \
  text->SetNDC();                                                              \
  text->SetTextSize(0.03);                                                     \
  text->Draw();                                                                \
  TString name_canvas_output =                                                 \
      Form((fPath_output + "/%s.png").Data(), histogram->GetName());           \
  c->SaveAs(name_canvas_output);

void AutoQA::ShowHistograms(int type, vector<int> index_method,
                            int index_hist) {
  // fIndex_method_show.clear();
  fIndex_method_show = index_method;

  // if there is no directory output_qa, create it
  if (gSystem->AccessPathName(fPath_output) == 1) {
    gSystem->mkdir(fPath_output);
  }

  if (type == TypeVecObject::kRaw) {
    if (index_hist == -1)
      for (auto &histogram : fVec_histograms_2_be_processed) {
        DefaultShowHist;
      }
    else {
      for (auto &histogram : fVecvec_histograms_processed[index_hist]) {
        DefaultShowHist;
      }
    }
  } else if (type == TypeVecObject::kProcessed) {
    if (index_hist == -1)
      for (auto &vec_histogram : fVecvec_histograms_processed) {
        for (auto &histogram : vec_histogram) {
          DefaultShowHist;
        }
      }
    else {
      for (auto &histogram : fVecvec_histograms_processed[index_hist]) {
        DefaultShowHist;
      }
    }
  } else if (type == TypeVecObject::kResults) {
    if (index_hist == -1)
      for (auto &vec_histogram : fVecvec_histograms_results) {
        for (auto &histogram : vec_histogram) {
          DefaultShowHist;
        }
      }
    else {
      for (auto &histogram : fVecvec_histograms_results[index_hist]) {
        DefaultShowHist;
      }
    }
  }
}

#define _QACritira(type)                                                       \
  case QAFunc::MethodCritiria::type:                                           \
    vec_hist_output = QAFunc::Critiria::func_##type(vec_hist_input, par);      \
    break;
#define QACritira(type) _QACritira(type)

#define _QACritiraRun(type)                                                    \
  case QAFunc::MethodCritiria::type:                                           \
    vec_hist_output =                                                          \
        QAFunc::Critiria::func_##type(vec_hist_input, par, tag_files);         \
    break;
#define QACritiraRun(type) _QACritiraRun(type)

// vec_hist_output = method(vec_hist_input, par, fVec_file_input.size());
void AutoQA::GetResults(int index_method, double *par, int index_hist) {
  fIndex_result.push_back({index_method, index_hist});
  vector<TObject *> vec_hist_output;
  vector<TObject *> vec_hist_input;
  if (index_hist == -1) {
    vec_hist_input = fVec_histograms_2_be_processed;
  } else {
    vec_hist_input = fVecvec_histograms_processed[index_hist];
  }
  if ((index_method > QAFunc::MethodCritiria::k1DQAVarDependentQA &&
       index_method < QAFunc::MethodCritiria::k2DQA) ||
      (index_method > QAFunc::MethodCritiria::k2DVarDependentQA)) {
    // cout << "run dependent" << endl;
    switch (index_method) {
      QACritiraRun(kProfileMeanXNSigmaVarDependent);
      QACritiraRun(kProfileSTDXNSigmaVarDependent);
      QACritiraRun(kLinearity);
      QACritiraRun(kFlat);
      QACritiraRun(kDEdxVpLowBoundary);
    default:
      break;
    }
  } else {
    switch (index_method) {
      QACritira(kProfileMeanXNSigma);
      QACritira(kProfileSTDXNSigma);
    default:
      break;
    }
  }
  fVecvec_histograms_results.push_back(vec_hist_output);
}

#endif