#include "AutoQA.h"

void phi_tree_common(
    TString path_file_input =
        "/home/szhu/work/alice/tpc_pid/AutoQA/test/data/output.root",
    TString path_dir_output = "phi_common_tree",
    TString name_file_output = "qa_phi_common_tree.root") {
  AutoQA autoqa(name_file_output, path_dir_output);

  autoqa.ReadHistograms<TH2F>(path_file_input, {"Phi"});

  double par[1] = {0.2};
  // autoqa.GetResults(QAFunc::MethodCritiria::kFlat, par, -1);
  autoqa.ShowHistograms(AutoQA::kRaw,
                        {QAFunc::MethodShow::kLogz,
                         QAFunc::MethodShow::kTPCPhiXBoundary2D ,
                         QAFunc::MethodShow::kTPCPhiXSectorMeanSigma2D});
}