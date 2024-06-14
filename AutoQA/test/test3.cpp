#include "AutoQA.h"

void test3() {
  // Test code here
  vector<TString> vec_path_input = {
      //   "./data/LHC23_PbPb_pass3_fullTPC_QC_sampling/LHC23zzf_apass3/"
      //   "AnalysisResults.root",
      //   "./data/LHC23_PbPb_pass3_fullTPC_QC_sampling/LHC23zzg_apass3/"
      //   "AnalysisResults.root",
      //   "./data/LHC23_PbPb_pass3_fullTPC_QC_sampling/LHC23zzk_apass3/"
      //   "AnalysisResults.root",
      //   "./data/LHC23_PbPb_pass3_fullTPC_QC_sampling/LHC23zzi_apass3/"
      //   "AnalysisResults.root",
      "./data/LHC23_PbPb_pass3_fullTPC_QC_sampling/LHC23zzh_apass3/"
      "AnalysisResults.root"};

  AutoQA autoqa("test.root", "output_ncls_dedx");

  autoqa.ReadHistograms<TH2F>("data/output.root", {"NclsTPC", "dEdx", "pion"});
  //   autoqa.ExcludeHistograms({"Ncls"});

  // autoqa.ExcludeHistograms({"Pt"});
  //   autoqa.RebinFixBins(18, QAFunc::MethodRebin::k2Dx);
  // {0.6, 0.8, 1.0, 1.2, 1.5, 2.0, 2.5, 3.0,
  // 3.5, 4.0, 4.5, 5.0, 6.0, 8.0, 15.0}
  // autoqa.SetTagsForSingleFile({"all","ncls0","ncls1"});
  autoqa.SetTagsForSingleFile({"p0.6-0.8", "p0.8-1.0", "p1.0-1.2", "p1.2-1.5",
                               "p1.5-2.0", "p2.0-2.5", "p2.5-3.0", "p3.0-3.5",
                               "p3.5-4.0", "p4.0-4.5", "p4.5-5.0", "p5.0-6.0",
                               "p6.0-8.0", "p8.0-15.0"});
  //   autoqa.ProcessHistograms(QAFunc::MethodProcess::k2D_Profilex);
  double par[1] = {0.2};
  autoqa.GetResults(QAFunc::MethodCritiria::kFlat, par, -1);
  autoqa.ShowHistograms(AutoQA::kRaw, {QAFunc::MethodShow::kLogz});
  autoqa.ShowHistograms(AutoQA::kResults, {QAFunc::MethodShow::kSeverity1D});
  //   autoqa.ShowHistograms(AutoQA::kProcessed,
  //   {QAFunc::MethodShow::kTPCPhiBoundary1D}); double par[1] = {3.};
  //   autoqa.GetResults(QAFunc::MethodCritiria::kProfileMeanXNSigma, par, 0);
  //   autoqa.GetResults(QAFunc::MethodCritiria::kProfileMeanXNSigmaVarDependent,
  //                     par, 0);
  //   autoqa.ShowHistograms(
  //       AutoQA::kResults,
  //       {QAFunc::MethodShow::kSeverity1D,
  //       QAFunc::MethodShow::kTPCPhiBoundary1D}, 0);
  //   autoqa.ShowHistograms(
  //       AutoQA::kResults,
  //       {QAFunc::MethodShow::kTPCPhiXBoundary2D,
  //       QAFunc::MethodShow::kSeverity2D}, 1);

  // autoqa.ProcessHistograms(QAFunc::MethodProcess::k2D_Profiley);
  // double par[1] = {3.};
  // autoqa.GetResults(QAFunc::MethodCritiria::kProfileMeanXNSigma, par, 0);
  // autoqa.GetResults(QAFunc::MethodCritiria::kProfileMeanXNSigmaVarDependent,
  //                   par, 0);

  // autoqa.ShowHistograms(AutoQA::kRaw,
  // {QAFunc::MethodShow::kTPCPhiYBoundary2D});
  // autoqa.ShowHistograms(AutoQA::kProcessed,
  //                       {QAFunc::MethodShow::kTPCPhiBoundary1D});
  // autoqa.ShowHistograms(AutoQA::kResults,
  //                       {QAFunc::MethodShow::kSeverity1D,
  //                       QAFunc::MethodShow::kTPCPhiBoundary1D},0);
  // autoqa.ShowHistograms(AutoQA::kResults,
  //                       {QAFunc::MethodShow::kTPCPhiXBoundary2D,
  //                        QAFunc::MethodShow::kSeverity2D},1);
}