#ifndef QAFunc_h
#define QAFunc_h

#include "MHead.h"
#include "QATool.h"

namespace QAFunc {
enum MethodRebin { k1D = 0, k2Dx = 1, k2Dy = 2, k3Dx = 3, k3Dy = 4, k3Dz = 5 };

enum MethodProcess {
  k1D_End = 0,
  k2D_Profilex,
  k2D_Profiley,
  k2D_End,
  k3D_Profilexy,
  k3D_Profilexz,
  k3D_Profileyz,
  k3D_End,
};

enum MethodShow {
  kLogx = 0,
  kLogy,
  kLogz,
  kTPCPhiBoundary1D,
  kTPCPhiXBoundary2D,
  kTPCPhiXSectorMeanSigma2D,
  kTPCPhiYBoundary2D,
  kSeverity1D,
  kSeverity2D,
};

enum MethodCritiria {
  k1DQA = 0,
  kProfileMeanXNSigma,
  kProfileSTDXNSigma,

  k1DQAVarDependentQA,
  kLinearity, // to be added
  kFlat,
  kDEdxVpLowBoundary,

  k2DQA,
  // to be added
  k2DVarDependentQA,
  kProfileMeanXNSigmaVarDependent,
  kProfileSTDXNSigmaVarDependent,
  kLinearityVarDependent, // to be added
};

void CanvasSetting(TCanvas *c, TObject *obj_h, int index_method_show) {
  switch (index_method_show) {
  case MethodShow::kLogx:
    c->SetLogx();
    break;
  case MethodShow::kLogy:
    c->SetLogy();
    break;
  case MethodShow::kLogz:
    c->SetLogz();
    break;
  case MethodShow::kTPCPhiBoundary1D: {
    c->cd();
    TH1 *h1 = reinterpret_cast<TH1 *>(obj_h);
    double low1_temp = h1->GetMinimum();
    double high1_temp = h1->GetMaximum();
    double low1 = low1_temp - 0.1 * (high1_temp - low1_temp);
    double high1 = high1_temp + 0.1 * (high1_temp - low1_temp);
    h1->GetYaxis()->SetRangeUser(low1, high1);
    for (int i = 0; i < 18; i++) {
      double x = i * TMath::Pi() / 9.;
      TLine *line = new TLine(x, low1, x, high1);
      line->SetLineStyle(10);
      line->Draw();
      x += TMath::Pi() / 25.;
      TLatex *tex = new TLatex(x, high1, Form("%d", i));
      tex->SetTextSize(0.03);
      tex->Draw();
    }
  } break;
  case MethodShow::kTPCPhiXBoundary2D: {
    c->cd();
    TH2 *h = reinterpret_cast<TH2 *>(obj_h);
    double low = h->GetYaxis()->GetXmin();
    double high = h->GetYaxis()->GetXmax();
    for (int i = 0; i < 18; i++) {
      double x = i * TMath::Pi() / 9.;
      TLine *line = new TLine(x, low, x, high);
      line->SetLineStyle(10);
      line->Draw("same");
      x += TMath::Pi() / 25.;
      TLatex *tex = new TLatex(x, high, Form("%d", i));
      tex->SetTextSize(0.03);
      tex->Draw();
    }
  } break;
  case MethodShow::kTPCPhiYBoundary2D: {
    c->cd();
    TH2 *h = reinterpret_cast<TH2 *>(obj_h);
    double low = h->GetXaxis()->GetXmin();
    double high = h->GetXaxis()->GetXmax();
    for (int i = 0; i < 18; i++) {
      double y = i * TMath::Pi() / 9.;
      TLine *line = new TLine(low, y, high, y);
      line->SetLineStyle(10);
      line->Draw("same");
      y += TMath::Pi() / 25.;
      TLatex *tex = new TLatex(low, y, Form("%d", i));
      tex->SetTextSize(0.03);
      tex->Draw();
    }
  } break;
  case MethodShow::kTPCPhiXSectorMeanSigma2D: {
    c->cd();
    TH2 *h = (TH2 *)obj_h->Clone(
        Form("%s_clone_%d", obj_h->GetName(), GenerateUID()));
    int n = h->GetXaxis()->GetNbins() / 18;
    h->RebinX(n);
    TProfile *profile =
        h->ProfileX(Form("%s_profilex_%d", obj_h->GetName(), GenerateUID()));
    TH1F *h1_mean = new TH1F(Form("%s_mean", profile->GetName()),
                             Form("%s;%s;mean", profile->GetTitle(),
                                  profile->GetXaxis()->GetTitle()),
                             profile->GetNbinsX(), 0, 2 * TMath::Pi());
    TH1F *h1_sigma = new TH1F(Form("%s_sigma", profile->GetName()),
                              Form("%s;%s;sigma", profile->GetTitle(),
                                   profile->GetXaxis()->GetTitle()),
                              profile->GetNbinsX(), 0, 2 * TMath::Pi());
    for (int i = 1; i <= profile->GetNbinsX(); i++) {
      double mean = profile->GetBinContent(i);
      double sigma = profile->GetBinError(i);
      h1_mean->SetBinContent(i, mean);
      h1_mean->SetBinError(i, 0);
      h1_sigma->SetBinContent(i, sigma);
      h1_sigma->SetBinError(i, 0);
    }

    double low = h->GetYaxis()->GetXmin();
    double high = h->GetYaxis()->GetXmax();
    double scale_sigma =
        (high + low) / (h1_sigma->GetMaximum() + h1_sigma->GetMinimum());
    h1_sigma->Scale(scale_sigma);

    h1_mean->SetLineColor(kBlue);
    h1_mean->SetLineWidth(2);
    h1_mean->SetMarkerStyle(20);
    h1_mean->SetMarkerColor(kBlue);
    h1_mean->SetMarkerSize(0.5);
    h1_sigma->SetLineColor(kRed);
    h1_sigma->SetLineWidth(2);
    h1_sigma->SetMarkerStyle(20);
    h1_sigma->SetMarkerColor(kRed);
    h1_sigma->SetMarkerSize(0.5);

    h1_mean->Draw("same");
    h1_sigma->Draw("same");

    // legend at right top
    TLegend *legend = new TLegend(0.7, 0.1, 0.9, 0.3);
    legend->AddEntry(h1_mean, "Mean", "l");
    legend->AddEntry(h1_sigma, Form("Sigma x %.1f", scale_sigma), "l");
    legend->Draw();

  } break;
  case MethodShow::kSeverity1D: {
    c->cd();
    TH1 *h1 = reinterpret_cast<TH1 *>(obj_h);
    h1->SetLineColor(kBlack);
    double low_temp = h1->GetMinimum();
    double high_temp = h1->GetMaximum();
    double low = low_temp - 0.1 * (high_temp - low_temp);
    double high = high_temp + 0.1 * (high_temp - low_temp);
    h1->GetYaxis()->SetRangeUser(low, high);

    for (int i_severity = int(low); i_severity <= high; i_severity++) {
      if (i_severity == 0 || abs(i_severity) > 5 || i_severity > high)
        continue;
      TLine *line = new TLine(h1->GetXaxis()->GetXmin(), i_severity,
                              h1->GetXaxis()->GetXmax(), i_severity);
      line->SetLineStyle(10);
      line->SetLineColor(MColorSpace::Color_Severity[abs(i_severity) - 1]);
      line->Draw();
      TLatex *tex = new TLatex(h1->GetXaxis()->GetXmin(), i_severity,
                               Form("Serverity %d", abs(i_severity)));
      tex->SetTextColor(MColorSpace::Color_Severity[abs(i_severity) - 1]);
      tex->SetTextSize(0.03);
      tex->Draw();
    }
  } break;
  case MethodShow::kSeverity2D: {
    c->cd();
    TH2 *h2 = reinterpret_cast<TH2 *>(obj_h);
    // set z-axis palette to const int Color_Severity[4] = {kCyan+2, kGreen + 2,
    // kOrange + 7, kRed + 1}; set the color of z-axis to Color_Severity
    int colors[5] = {
        MColorSpace::Color_Severity[0], MColorSpace::Color_Severity[1],
        MColorSpace::Color_Severity[2], MColorSpace::Color_Severity[3],
        MColorSpace::Color_Severity[4]};
    gStyle->SetPalette(5, colors);
    h2->GetZaxis()->SetRangeUser(0, 5);
  }
  default:
    break;
  }
}

namespace Critiria {
// 1D QA
vector<TObject *> func_kProfileMeanXNSigma(vector<TObject *> vec_h1,
                                           double *par) {
  // par[0] is the nsigma to reject the bad bin
  vector<TObject *> vec_h1_new;
  for (int i = 0; i < vec_h1.size(); i++) {
    TProfile *profile_input = reinterpret_cast<TProfile *>(vec_h1[i]);
    // profile_input->SetErrorOption("s");
    vector<double> vec_val;
    for (int j = 0; j < profile_input->GetNbinsX(); j++) {
      double val = profile_input->GetBinContent(j + 1);
      // val != val ? vec_val.push_back(0) : void();
      vec_val.push_back(val);
    }
    QATool::BadRun_Str brs = QATool::findBadRun(vec_val, par[0]);
    const int n_bins = profile_input->GetNbinsX();
    double low_bound = profile_input->GetXaxis()->GetXmin();
    double high_bound = profile_input->GetXaxis()->GetXmax();

    TH1F *h1_new =
        new TH1F(Form("%s_ProfileMeanX_NSigma", profile_input->GetName()),
                 Form("%s_%s", profile_input->GetTitle(), "_mean;;n#sigma"),
                 n_bins, low_bound, high_bound);
    h1_new->GetXaxis()->SetTitle(profile_input->GetXaxis()->GetTitle());
    for (int j = 0; j < n_bins; j++) {
      double val =
          (profile_input->GetBinContent(j + 1) - brs.mean) / abs(brs.sigma);
      h1_new->SetBinContent(j + 1, val);
    }
    vec_h1_new.push_back(h1_new);
  }
  return vec_h1_new;
};

vector<TObject *> func_kProfileSTDXNSigma(vector<TObject *> vec_h1,
                                          double *par) {
  // par[0] is the nsigma to reject the bad bin
  vector<TObject *> vec_h1_new;
  for (int i = 0; i < vec_h1.size(); i++) {
    TProfile *profile_input = reinterpret_cast<TProfile *>(vec_h1[i]);
    profile_input->SetErrorOption("s");
    vector<double> vec_val;
    for (int j = 0; j < profile_input->GetNbinsX(); j++) {
      double val = profile_input->GetBinError(j + 1);
      val != val ? vec_val.push_back(0) : void();
      vec_val.push_back(val);
    }
    QATool::BadRun_Str brs = QATool::findBadRun(vec_val, par[0]);
    const int n_bins = profile_input->GetNbinsX();
    const double *bins = profile_input->GetXaxis()->GetXbins()->GetArray();
    TH1F *h1_new =
        new TH1F(Form("%s_ProfileMeanX_NSigma", profile_input->GetName()),
                 Form("%s_%s", profile_input->GetTitle(), "_std;;n#sigma"),
                 n_bins, bins);
    h1_new->GetXaxis()->SetTitle(profile_input->GetXaxis()->GetTitle());
    for (int j = 0; j < n_bins; j++) {
      double val =
          (profile_input->GetBinContent(j + 1) - brs.mean) / abs(brs.sigma);
      h1_new->SetBinContent(j + 1, val);
    }
    vec_h1_new.push_back(h1_new);
  }
  return vec_h1_new;
};

// 2D QA

// 1D Run-dependent QA
vector<TObject *> func_kLinearity(vector<TObject *> vec_h1, double *par,
                                  vector<TString> tag_xaxis) {
  int n_run = tag_xaxis.size();
  cout << "n_run: " << n_run << endl;
  vector<TObject *> vec_h1_new;
  TH1F *h1_result = new TH1F(
      Form("%s_linearity", reinterpret_cast<TH2F *>(vec_h1[0])->GetName()),
      ";;Severity", n_run, 0, n_run);
  for (int i = 0; i < n_run; i++)
    h1_result->GetXaxis()->SetBinLabel(i + 1, tag_xaxis[i]);

  for (int i = 0; i < vec_h1.size(); i++) {
    TH2F *h2_input = reinterpret_cast<TH2F *>(vec_h1[i]);
    double bound_x_low = h2_input->GetXaxis()->GetXmin();
    double bound_x_high = h2_input->GetXaxis()->GetXmax();
    TF1 *line1 = new TF1(Form("func_linearity_%d", GenerateUID()), "pol1",
                         bound_x_low, bound_x_high);
    h2_input->Fit(line1);
    h1_result->SetBinContent(i + 1, line1->GetParameter(0) / (*par));
  }
  vec_h1_new.push_back(h1_result);
  return vec_h1_new;
};

vector<TObject *> func_kFlat(vector<TObject *> vec_h1, double *par,
                             vector<TString> tag_xaxis) {
  int n_run = tag_xaxis.size();
  cout << "n_run: " << n_run << endl;
  vector<TObject *> vec_h1_new;
  TH1F *h1_result = new TH1F(
      Form("%s_linearity", reinterpret_cast<TH2F *>(vec_h1[0])->GetName()),
      ";;Severity", n_run, 0, n_run);
  for (int i = 0; i < n_run; i++)
    h1_result->GetXaxis()->SetBinLabel(i + 1, tag_xaxis[i]);

  for (int i = 0; i < vec_h1.size(); i++) {
    TH2F *h2_input = reinterpret_cast<TH2F *>(vec_h1[i]);
    double bound_x_low = h2_input->GetXaxis()->GetXmin();
    double bound_x_high = h2_input->GetXaxis()->GetXmax();
    TF1 *line1 = new TF1(Form("func_linearity_%d", GenerateUID()), "pol1",
                         bound_x_low, bound_x_high);
    h2_input->Fit(line1);
    h1_result->SetBinContent(i + 1, abs(line1->GetParameter(0)) / (*par));
  }
  vec_h1_new.push_back(h1_result);
  return vec_h1_new;
};

vector<TObject *> func_kDEdxVpLowBoundary(vector<TObject *> vec_h1, double *par,
                                          vector<TString> tag_xaxis) {
  int n_run = tag_xaxis.size();
  cout << "n_run: " << n_run << endl;
  vector<TObject *> vec_h1_new;
  TH1F *h1_result = new TH1F(
      Form("%s_linearity", reinterpret_cast<TH2F *>(vec_h1[0])->GetName()),
      ";;Severity", n_run, 0, n_run);
  for (int i = 0; i < n_run; i++)
    h1_result->GetXaxis()->SetBinLabel(i + 1, tag_xaxis[i]);

  for (int i = 0; i < vec_h1.size(); i++) {
    TH2F *h2_input = reinterpret_cast<TH2F *>(vec_h1[i]);
    int bin_bound_x_high_desired = h2_input->GetXaxis()->FindBin(*par);
    TH1D *h1_temp = h2_input->ProjectionY(
        Form("DEdxVpLowBoundary_ProjectionY%d", GenerateUID()), 0,
        bin_bound_x_high_desired);
    double n = h1_temp->Integral();
    double mean = h1_temp->GetMean();
    double n_low = h1_temp->Integral(0, h1_temp->FindBin(mean));
    double frac = n_low / n / (*(par + 1));
    h1_result->SetBinContent(i + 1, frac);
  }
  vec_h1_new.push_back(h1_result);
  return vec_h1_new;
};

// 2D Run-dependent QA
vector<TObject *>
func_kProfileMeanXNSigmaVarDependent(vector<TObject *> vec_h1, double *par,
                                     vector<TString> tag_yaxis) {
  // par[0] is the nsigma to reject the bad bin
  int n_run = tag_yaxis.size();
  cout << "n_run: " << n_run << endl;
  vector<TObject *> vec_h1_new;
  for (int i_hist_run = 0; i_hist_run < vec_h1.size(); i_hist_run += n_run) {
    const int n_bins =
        reinterpret_cast<TProfile *>(vec_h1[i_hist_run])->GetNbinsX();
    double low_bound =
        reinterpret_cast<TProfile *>(vec_h1[i_hist_run])->GetXaxis()->GetXmin();
    double high_bound =
        reinterpret_cast<TProfile *>(vec_h1[i_hist_run])->GetXaxis()->GetXmax();
    TH2F *h2_new = new TH2F(
        Form("%s_ProfileMeanX_NSigma_VarDependent",
             reinterpret_cast<TProfile *>(vec_h1[i_hist_run])->GetName()),
        Form("%s_%s",
             reinterpret_cast<TProfile *>(vec_h1[i_hist_run])->GetTitle(),
             ";;n#sigma"),
        n_bins, low_bound, high_bound, n_run, -0.5, n_run - 0.5);
    h2_new->GetXaxis()->SetTitle(
        reinterpret_cast<TProfile *>(vec_h1[i_hist_run])
            ->GetXaxis()
            ->GetTitle());
    for (int i_run = 0; i_run < n_run; i_run++) {
      h2_new->GetYaxis()->SetBinLabel(i_run + 1, tag_yaxis[i_run]);
    }
    for (int i_hist = i_hist_run; i_hist < i_hist_run + n_run; i_hist++) {
      TProfile *profile_input = reinterpret_cast<TProfile *>(vec_h1[i_hist]);
      vector<double> vec_val;
      for (int j = 0; j < profile_input->GetNbinsX(); j++) {
        double val = profile_input->GetBinContent(j + 1);
        val != val ? vec_val.push_back(0) : void();
        vec_val.push_back(val);
      }
      QATool::BadRun_Str brs = QATool::findBadRun(vec_val, par[0]);
      for (int j = 0; j < n_bins; j++) {
        double val =
            (profile_input->GetBinContent(j + 1) - brs.mean) / abs(brs.sigma);
        val = abs(val);
        h2_new->SetBinContent(j + 1, i_hist - i_hist_run + 1, val);
      }
    }
    vec_h1_new.push_back(h2_new);
    // set the color of z-axis to Color_Severity
  }
  return vec_h1_new;
};

vector<TObject *>
func_kProfileSTDXNSigmaVarDependent(vector<TObject *> vec_h1, double *par,
                                    vector<TString> tag_yaxis) {
  // par[0] is the nsigma to reject the bad bin
  vector<TObject *> vec_h1_new;
  int n_run = tag_yaxis.size();
  for (int i_hist_run = 0; i_hist_run < vec_h1.size(); i_hist_run += n_run) {
    const int n_bins =
        reinterpret_cast<TProfile *>(vec_h1[i_hist_run])->GetNbinsX();
    const double *bins = reinterpret_cast<TProfile *>(vec_h1[i_hist_run])
                             ->GetXaxis()
                             ->GetXbins()
                             ->GetArray();
    TH2F *h2_new = new TH2F(
        Form("%s_ProfileMeanX_NSigma_VarDependent",
             reinterpret_cast<TProfile *>(vec_h1[i_hist_run])->GetName()),
        Form("%s_%s",
             reinterpret_cast<TProfile *>(vec_h1[i_hist_run])->GetTitle(),
             ";;n#sigma"),
        n_bins, bins, n_run, -0.5, n_run - 0.5);
    for (int i_run = 0; i_run < n_run; i_run++) {
      h2_new->GetYaxis()->SetBinLabel(i_run + 1, tag_yaxis[i_run]);
    }
    h2_new->GetXaxis()->SetTitle(
        reinterpret_cast<TProfile *>(vec_h1[i_hist_run])
            ->GetXaxis()
            ->GetTitle());
    for (int i_hist = i_hist_run; i_hist < i_hist_run + n_run; i_hist++) {
      TProfile *profile_input = reinterpret_cast<TProfile *>(vec_h1[i_hist]);
      profile_input->SetErrorOption("s");
      vector<double> vec_val;
      for (int j = 0; j < profile_input->GetNbinsX(); j++) {
        double val = profile_input->GetBinError(j + 1);
        val != val ? vec_val.push_back(0) : void();
        vec_val.push_back(val);
      }
      QATool::BadRun_Str brs = QATool::findBadRun(vec_val, par[0]);
      for (int j = 0; j < n_bins; j++) {
        double val =
            (profile_input->GetBinContent(j + 1) - brs.mean) / abs(brs.sigma);
        val = abs(val);
        h2_new->SetBinContent(j + 1, i_hist - i_hist_run + 1, val);
      }
    }
    vec_h1_new.push_back(h2_new);
  }
  return vec_h1_new;
};

} // namespace Critiria
} // namespace QAFunc
#endif