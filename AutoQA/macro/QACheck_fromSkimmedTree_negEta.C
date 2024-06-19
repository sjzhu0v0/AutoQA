#include "TCanvas.h"
#include "TColor.h"
#include "TF1.h"
#include "TFile.h"
#include "TH1.h"
#include "TH2.h"
#include "THashList.h"
#include "THn.h"
#include "THnSparse.h"
#include "TKey.h"
#include "TLegend.h"
#include "TLine.h"
#include "TList.h"
#include "TMath.h"
#include "TObjArray.h"
#include "TPDF.h"
#include "TPaveText.h"
#include "TROOT.h"
#include "TString.h"
#include "TStyle.h"
#include "TSystem.h"
#include "TTree.h"
#include <array>
#include <fstream>
#include <set>
#include <sstream>
using namespace std;

//===================================================================================
///
///   check 2D distributions of measured dE/dx vs number of clusters, in bins of
///   momentum
///
///===================================================================================

// const TString outputDir =
// "/lustre/alice/users/tcheng/TPC_BB/ServiceTask_PIDQA/plotsQA_LHC22o_seperateCharge_woMixedRuns";

TString outputDir;

void DrawLine(TCanvas* c, TH2* h) {
	c->cd();
	double low = h->GetYaxis()->GetXmin();
	double high = h->GetYaxis()->GetXmax();
	for (int i = 0; i < 18; i++) {
		double x = i*TMath::Pi()/9.;
		TLine* line = new TLine(x,low,x,high);
		line->SetLineStyle(10);
		line->Draw();
		x += TMath::Pi()/25.;
		TLatex* tex = new TLatex(x,high,Form("%d",i));
		tex->SetTextSize(0.03);
		tex->Draw();
	}
}

//***************************************************************************************

void QACheck_fromSkimmedTree_negEta(TString par_input, TString par_output) {

  gStyle->SetOptStat(0);

  outputDir = par_output;
  TFile *f = TFile::Open(par_input);
  TFile *f_output = new TFile(outputDir+"/output.root","recreate");
  f->ls(); // // O2tpctofv0Tree

  // cout << " *********** check file ********  " << endl;

  // TDirectoryFile *f_DF = (TDirectoryFile*)f -> Get("DF_2270036716913792"); //
  // O2tpctofskimtree, O2tpcskimv0tree

  TDirectoryFile *f_DF =
      (TDirectoryFile *)((TKey *)f->GetListOfKeys()->At(0))->ReadObj();
  std::string TreeName;
  //  // O2tpctofv0Tree

  f_DF->ls();

  cout << " *********** check file ********  " << endl;

  TreeName = "O2tpcskimv0tree";
  TTree *Tree_skimmed_V0 = (TTree *)f_DF->Get(TreeName.c_str());

  cout << " *********** check tree ********  " << endl;
  Tree_skimmed_V0->ls();

  cout << " *********** check tree branch names ********  " << endl;

  if (!Tree_skimmed_V0) {
    printf("Could not find TTree '%s' in file '%s'  \n", TreeName.c_str(),
           f->GetName());
  }

  ///---- defined variable
  Float_t fEta;
  UChar_t fPidIndex; // 0: electron; 2: pion; 3: kaon; 4: proton;
  Float_t fMass;
  Float_t fTPCSignal;        // dE/dx signal in the TPC
  Float_t fTPCInnerParam;    // momentum at inner wall in the TPC
  Float_t fNormNClustersTPC; // TMath::sqrt(nClNorm/ncl); nClNorm is the total
                             // number of TPC (152 for Run 3 ), ncl is the
                             // number of TPC found
  Float_t fNSigTPC;          //
  Float_t fBetaGamma;
  Float_t fInvDeDxExpTPC;
  Int_t fRunNumber;
  Float_t fSigned1Pt;
  Float_t fPhi;
  Float_t fTgl;

  ///-----------------------------------------------------------
  ///-------- here defined all the histograms
  ///-----------------------------------------------------------

  ////------------------- O2tpctofskimtree
  Double_t Vary_pT_pr_pi[15] = {0.6, 0.8, 1.0, 1.2, 1.5, 2.0, 2.5, 3.0,
                                3.5, 4.0, 4.5, 5.0, 6.0, 8.0, 15.0};
  TH2D *h_proton_dEdx_vs_NclTPC_TPCTOF[14];
  TH2D *h_pion_dEdx_vs_NclTPC_TPCTOF[14];

  ////------------------ O2tpcskimv0tree
  //----- Electron
  Double_t Vary_pT[9] = {0.6, 0.8, 1.0, 1.2, 1.5, 2.0, 2.5, 3.0, 5.0};
  TH2D *h_Electron_dEdx_vs_NclTPC_V0[8];
  Double_t pTMax_Ele, pTMin_Ele;

  TH1D *h_dEdxExp_ele_V0[8];

  ///---- proton, pion
  TH2D *h_proton_dEdx_vs_NclTPC_V0[14];
  TH2D *h_pion_dEdx_vs_NclTPC_V0[14];

  TH1D *h_dEdxExp_proton_V0[14];
  TH1D *h_dEdxExp_pion_V0[14];

  ////----------- Ncls check

  TH1D *h_Ncls_ele = new TH1D("h_Ncls_ele", "", 151, 9.5, 160.5);
  h_Ncls_ele->GetYaxis()->SetTitle("Entries");
  h_Ncls_ele->GetYaxis()->SetRangeUser(10, 300000);
  h_Ncls_ele->GetXaxis()->SetTitle("NclsTPC");
  h_Ncls_ele->SetLineColor(kBlack);
  h_Ncls_ele->SetTitle("electron");

  TH1D *h_Ncls_pion = new TH1D("h_Ncls_pion", "", 151, 9.5, 160.5);
  h_Ncls_pion->SetLineColor(kBlack);
  h_Ncls_pion->GetYaxis()->SetRangeUser(10, 750000);
  h_Ncls_pion->GetYaxis()->SetTitle("Entries");
  h_Ncls_pion->GetXaxis()->SetTitle("NclsTPC");
  h_Ncls_pion->SetTitle("pion");

  TH1D *h_Ncls_proton = new TH1D("h_Ncls_proton", "", 151, 9.5, 160.5);
  h_Ncls_proton->SetLineColor(kBlack);
  h_Ncls_proton->GetYaxis()->SetRangeUser(10, 1200000);
  h_Ncls_proton->GetYaxis()->SetTitle("Entries");
  h_Ncls_proton->GetXaxis()->SetTitle("NclsTPC");
  h_Ncls_proton->SetTitle("proton");

  ///----- dE/dx vs pIn check
  TH2D *h_electron_dEdx_vs_pIn_V0 =
      new TH2D("h_electron_dEdx_vs_pIn_V0", "", 8, Vary_pT, 150, 10, 160);
  h_electron_dEdx_vs_pIn_V0->GetXaxis()->SetTitle("PIn");
  h_electron_dEdx_vs_pIn_V0->GetYaxis()->SetTitle("dEdx");
  h_electron_dEdx_vs_pIn_V0->SetTitle("Electron");

  TH2D *h_pion_dEdx_vs_pIn_V0 =
      new TH2D("h_pion_dEdx_vs_pIn_V0", "", 14, Vary_pT_pr_pi, 150, 10, 160);
  h_pion_dEdx_vs_pIn_V0->GetXaxis()->SetTitle("PIn");
  h_pion_dEdx_vs_pIn_V0->GetYaxis()->SetTitle("dEdx");
  h_pion_dEdx_vs_pIn_V0->SetTitle("Pion");

  TH2D *h_proton_dEdx_vs_pIn_V0 =
      new TH2D("h_proton_dEdx_vs_pIn_V0", "", 14, Vary_pT_pr_pi, 150, 10, 160);
  h_proton_dEdx_vs_pIn_V0->GetXaxis()->SetTitle("PIn");
  h_proton_dEdx_vs_pIn_V0->GetYaxis()->SetTitle("dEdx");
  h_proton_dEdx_vs_pIn_V0->SetTitle("Proton");

  ///---- Ncls vs pT
  TH1D *h_Ncls_ele_pT[8];
  TH1D *h_Ncls_pion_pT[14];
  TH1D *h_Ncls_proton_pT[14];

  ///-------------------- Check dEdx vx Ncs with positive/negative tracks

  TH2D *h_electron_dEdx_NclsTPC_Chagre[8][2];

  const char *Tracks_Charge[] = {"PositiveCharge", "NegativeCharge"};
  std::string Chagre_range;
  Double_t Vary_Eta[10] = {-0.9, -0.7, -0.5, -0.3, -0.1,
                           0.1,  0.3,  0.5,  0.7,  0.9};

  TH2D *h_pion_dEdx_NclsTPC_Charge[14][2];

  TH2D *h_proton_dEdx_NclsTPC_Charge[14][2];

  TH1D *h_NSigTPC_electron_PIn[8][2];

  ////------------- check PIn vs P with NclsTPC (> 130, or < 80 )
  TH2D *h_electron_PIn_P_Ncls[2];
  TH2D *h_pion_PIn_P_Ncls[2];
  TH2D *h_proton_PIn_P_Ncls[2];
  const char *Ncls[] = {"NclsTPC > 130", "NclsTPC < 80"};
  std::string Ncls_range;

  TH2D *h_AllTracks_PIn_P_Ncls[2];

  TH2D *h_AllTrack_PIn_P_AllNcls =
      new TH2D("h_AllTrack_PIn_P_AllNcls", "", 44, 0.6, 5, 44, 0.6, 5);
  h_AllTrack_PIn_P_AllNcls->SetTitle("All tracks");
  h_AllTrack_PIn_P_AllNcls->GetYaxis()->SetTitle("PIn");
  h_AllTrack_PIn_P_AllNcls->GetXaxis()->SetTitle("P");

  TH2D *h_electron_PIn_P_AllNcls =
      new TH2D("h_electron_PIn_P_AllNcls", "", 44, 0.6, 5, 44, 0.6, 5);
  h_electron_PIn_P_AllNcls->SetTitle("electron");
  h_electron_PIn_P_AllNcls->GetYaxis()->SetTitle("PIn");
  h_electron_PIn_P_AllNcls->GetXaxis()->SetTitle("P");

  TH2D *h_pion_PIn_P_AllNcls =
      new TH2D("h_pion_PIn_P_AllNcls", "", 44, 0.6, 5, 44, 0.6, 5);
  h_pion_PIn_P_AllNcls->SetTitle("pion");
  h_pion_PIn_P_AllNcls->GetYaxis()->SetTitle("PIn");
  h_pion_PIn_P_AllNcls->GetXaxis()->SetTitle("P");

  TH2D *h_proton_PIn_P_AllNcls =
      new TH2D("h_proton_PIn_P_AllNcls", "", 44, 0.6, 5, 44, 0.6, 5);
  h_proton_PIn_P_AllNcls->SetTitle("proton");
  h_proton_PIn_P_AllNcls->GetYaxis()->SetTitle("PIn");
  h_proton_PIn_P_AllNcls->GetXaxis()->SetTitle("P");

  for (Int_t p = 0; p < 2; p++) {

    Ncls_range = Ncls[p];
    h_electron_PIn_P_Ncls[p] = new TH2D(Form("h_electron_PIn_P_Ncls_%d", p), "",
                                        44, 0.6, 5, 44, 0.6, 5);
    h_electron_PIn_P_Ncls[p]->SetTitle(
        Form("electron, %s", Ncls_range.c_str()));
    h_electron_PIn_P_Ncls[p]->GetYaxis()->SetTitle("PIn");
    h_electron_PIn_P_Ncls[p]->GetXaxis()->SetTitle("P");

    // h_pion_PIn_P_Ncls[p] = new TH2D(Form("h_pion_PIn_P_Ncls%d", p), "", 144,
    // 0.6, 15, 144, 0.6, 15 );
    h_pion_PIn_P_Ncls[p] =
        new TH2D(Form("h_pion_PIn_P_Ncls%d", p), "", 44, 0.6, 5, 44, 0.6, 5);
    h_pion_PIn_P_Ncls[p]->SetTitle(Form("pion, %s", Ncls_range.c_str()));
    h_pion_PIn_P_Ncls[p]->GetYaxis()->SetTitle("PIn");
    h_pion_PIn_P_Ncls[p]->GetXaxis()->SetTitle("P");

    // h_proton_PIn_P_Ncls[p] = new TH2D(Form("h_proton_PIn_P_Ncls%d", p), "",
    // 144, 0.6, 15, 144, 0.6, 15 );

    h_proton_PIn_P_Ncls[p] =
        new TH2D(Form("h_proton_PIn_P_Ncls%d", p), "", 44, 0.6, 5, 44, 0.6, 5);
    h_proton_PIn_P_Ncls[p]->SetTitle(Form("proton, %s", Ncls_range.c_str()));
    h_proton_PIn_P_Ncls[p]->GetYaxis()->SetTitle("PIn");
    h_proton_PIn_P_Ncls[p]->GetXaxis()->SetTitle("P");

    h_AllTracks_PIn_P_Ncls[p] = new TH2D(Form("h_AllTracks_PIn_P_Ncls%d", p),
                                         "", 44, 0.6, 5, 44, 0.6, 5);
    h_AllTracks_PIn_P_Ncls[p]->SetTitle(
        Form("All tracks, %s", Ncls_range.c_str()));
    h_AllTracks_PIn_P_Ncls[p]->GetYaxis()->SetTitle("PIn");
    h_AllTracks_PIn_P_Ncls[p]->GetXaxis()->SetTitle("P");
  }

  //////-----------------------------------------------------------------
  //////-----------------------------------------------------------------

  ///----- check NclsTPC vs Phi information
  TH2D *h_Ncls_ele_Phi[8];
  TH2D *h_Ncls_pion_Phi[14];
  TH2D *h_Ncls_proton_Phi[14];

  ///---- check  dE/dx vs Phi information
  TH2D *h_dEdx_Phi_ele[8];
  TH2D *h_dEdx_Phi_pion[14];
  TH2D *h_dEdx_Phi_proton[14];

  ///---- check  dE/dx vs Phi information with TPCNcls two cuts
  TH2D *h_dEdx_Phi_ele_Ncls[8][2];
  TH2D *h_dEdx_Phi_pion_Ncls[14][2];
  TH2D *h_dEdx_Phi_proton_Ncls[14][2];

  for (Int_t i = 0; i < 8; i++) {

    pTMin_Ele = Vary_pT[i];
    pTMax_Ele = Vary_pT[i + 1];

    ///--- O2tpcskimv0tree
    h_Electron_dEdx_vs_NclTPC_V0[i] =
        new TH2D(Form("h_Electron_dEdx_vs_NclTPC_V0%d", i), "", 151, 9.5, 160.5,
                 150, 10, 160);
    h_Electron_dEdx_vs_NclTPC_V0[i]->SetTitle(
        Form("%0.1f < p(electron) < %0.1f", pTMin_Ele, pTMax_Ele));
    h_Electron_dEdx_vs_NclTPC_V0[i]->GetYaxis()->SetTitle("dE/dx");
    h_Electron_dEdx_vs_NclTPC_V0[i]->GetXaxis()->SetTitle("NclsTPC");
    h_Electron_dEdx_vs_NclTPC_V0[i]->GetYaxis()->SetRangeUser(1, 160);
    // h_Electron_dEdx_vs_NclTPC_V0[i] -> GetXaxis() -> SetRangeUser(60, 160);
    h_Electron_dEdx_vs_NclTPC_V0[i]->GetXaxis()->SetRangeUser(20, 160);

    //--- dEdxExp
    h_dEdxExp_ele_V0[i] =
        new TH1D(Form("h_dEdxExp_ele_V0%d", i), "", 150, 10, 160);
    h_dEdxExp_ele_V0[i]->SetLineColor(kBlack);
    h_dEdxExp_ele_V0[i]->SetLineWidth(2);

    ///---- Ncls vs pT
    h_Ncls_ele_pT[i] =
        new TH1D(Form("h_Ncls_ele_pT%d", i), "", 151, 9.5, 160.5);
    h_Ncls_ele_pT[i]->SetTitle(
        Form("%0.1f < p(electron) < %0.1f", pTMin_Ele, pTMax_Ele));
    h_Ncls_ele_pT[i]->GetXaxis()->SetTitle("NclsTPC");
    h_Ncls_ele_pT[i]->GetYaxis()->SetTitle("Entries");
    h_Ncls_ele_pT[i]->GetYaxis()->SetRangeUser(10, 100000);
    h_Ncls_ele_pT[i]->SetLineColor(kBlack);

    ///--- Ncls vs Phi
    h_Ncls_ele_Phi[i] = new TH2D(Form("h_Ncls_ele_Phi%d", i), "", 200, 0,
                                 2. * TMath::Pi(), 151, 9.5, 160.5);
    h_Ncls_ele_Phi[i]->SetTitle(
        Form("%0.1f < p(electron) < %0.1f", pTMin_Ele, pTMax_Ele));
    h_Ncls_ele_Phi[i]->GetYaxis()->SetTitle("NclsTPC");
    h_Ncls_ele_Phi[i]->GetXaxis()->SetTitle("#phi");

    ///---- dEdx vs Phi
    h_dEdx_Phi_ele[i] = new TH2D(Form("h_dEdx_Phi_ele%d", i), "", 200, 0,
                                 2. * TMath::Pi(), 150, 10, 160);
    h_dEdx_Phi_ele[i]->SetTitle(
        Form("%0.1f < p(electron) < %0.1f", pTMin_Ele, pTMax_Ele));
    h_dEdx_Phi_ele[i]->GetYaxis()->SetTitle("dE/dx");
    h_dEdx_Phi_ele[i]->GetXaxis()->SetTitle("#phi");

    ///----------- define
    for (Int_t p = 0; p < 2; p++) {

      Chagre_range = Tracks_Charge[p];

      h_electron_dEdx_NclsTPC_Chagre[i][p] =
          new TH2D(Form("h_electron_dEdx_NclsTPC_Chagre_%d_%d", i, p), "", 151,
                   9.5, 160.5, 150, 10, 160);
      h_electron_dEdx_NclsTPC_Chagre[i][p]->SetTitle(
          Form("%0.1f < p(electron) < %0.1f (%s)", pTMin_Ele, pTMax_Ele,
               Chagre_range.c_str()));
      h_electron_dEdx_NclsTPC_Chagre[i][p]->GetYaxis()->SetTitle("dE/dx");
      h_electron_dEdx_NclsTPC_Chagre[i][p]->GetXaxis()->SetTitle("NclsTPC");
      h_electron_dEdx_NclsTPC_Chagre[i][p]->GetYaxis()->SetRangeUser(1, 160);
      h_electron_dEdx_NclsTPC_Chagre[i][p]->GetXaxis()->SetRangeUser(20, 160);

      ///----- check Ncls cuts
      Ncls_range = Ncls[p];
      h_dEdx_Phi_ele_Ncls[i][p] =
          new TH2D(Form("h_dEdx_Phi_ele_Ncls%d_%d", i, p), "", 200, 0,
                   2. * TMath::Pi(), 150, 10, 160);
      h_dEdx_Phi_ele_Ncls[i][p]->SetTitle(
          Form("%0.1f < p(electron) < %0.1f, %s", pTMin_Ele, pTMax_Ele,
               Ncls_range.c_str()));
      h_dEdx_Phi_ele_Ncls[i][p]->GetYaxis()->SetTitle("dE/dx");
      h_dEdx_Phi_ele_Ncls[i][p]->GetXaxis()->SetTitle("#phi");
    }
  }

  for (Int_t k = 0; k < 14; k++) {

    ///----- O2tpctofskimtree
    h_proton_dEdx_vs_NclTPC_TPCTOF[k] =
        new TH2D(Form("h_proton_dEdx_vs_NclTPC_TPCTOF%d", k), "", 151, 9.5,
                 160.5, 200, 10, 210);
    h_proton_dEdx_vs_NclTPC_TPCTOF[k]->SetTitle(Form(
        "%0.1f < p(proton) < %0.1f", Vary_pT_pr_pi[k], Vary_pT_pr_pi[k + 1]));
    h_proton_dEdx_vs_NclTPC_TPCTOF[k]->GetYaxis()->SetTitle("dE/dx");
    h_proton_dEdx_vs_NclTPC_TPCTOF[k]->GetXaxis()->SetTitle("NclsTPC");

    h_pion_dEdx_vs_NclTPC_TPCTOF[k] =
        new TH2D(Form("h_pion_dEdx_vs_NclTPC_TPCTOF%d", k), "", 151, 9.5, 160.5,
                 200, 10, 210);
    h_pion_dEdx_vs_NclTPC_TPCTOF[k]->SetTitle(Form(
        "%0.1f < p(pion) < %0.1f", Vary_pT_pr_pi[k], Vary_pT_pr_pi[k + 1]));
    h_pion_dEdx_vs_NclTPC_TPCTOF[k]->GetYaxis()->SetTitle("dE/dx");
    h_pion_dEdx_vs_NclTPC_TPCTOF[k]->GetXaxis()->SetTitle("NclsTPC");

    ///---- O2tpcskimv0tree

    h_pion_dEdx_vs_NclTPC_V0[k] =
        new TH2D(Form("h_pion_dEdx_vs_NclTPC_V0%d", k), "", 151, 9.5, 160.5,
                 200, 10, 210);
    h_pion_dEdx_vs_NclTPC_V0[k]->SetTitle(Form(
        "%0.1f < p(pion) < %0.1f", Vary_pT_pr_pi[k], Vary_pT_pr_pi[k + 1]));
    h_pion_dEdx_vs_NclTPC_V0[k]->GetYaxis()->SetTitle("dE/dx");
    h_pion_dEdx_vs_NclTPC_V0[k]->GetXaxis()->SetTitle("NclsTPC");
    h_pion_dEdx_vs_NclTPC_V0[k]->GetYaxis()->SetRangeUser(1, 120);
    // h_pion_dEdx_vs_NclTPC_V0[k] -> GetXaxis() -> SetRangeUser(60, 160);
    h_pion_dEdx_vs_NclTPC_V0[k]->GetXaxis()->SetRangeUser(20, 160);

    h_proton_dEdx_vs_NclTPC_V0[k] =
        new TH2D(Form("h_proton_dEdx_vs_NclTPC_V0%d", k), "", 151, 9.5, 160.5,
                 200, 10, 210);
    h_proton_dEdx_vs_NclTPC_V0[k]->SetTitle(Form(
        "%0.1f < p(proton) < %0.1f", Vary_pT_pr_pi[k], Vary_pT_pr_pi[k + 1]));
    h_proton_dEdx_vs_NclTPC_V0[k]->GetYaxis()->SetTitle("dE/dx");
    h_proton_dEdx_vs_NclTPC_V0[k]->GetXaxis()->SetTitle("NclsTPC");
    h_proton_dEdx_vs_NclTPC_V0[k]->GetYaxis()->SetRangeUser(1, 160);
    // h_proton_dEdx_vs_NclTPC_V0[k] -> GetXaxis() -> SetRangeUser(60, 160);
    h_proton_dEdx_vs_NclTPC_V0[k]->GetXaxis()->SetRangeUser(20, 160);

    ////------ Ncls with pT

    h_Ncls_pion_pT[k] =
        new TH1D(Form("h_Ncls_pion_pT%d", k), "", 151, 9.5, 160.5);
    h_Ncls_pion_pT[k]->SetTitle(Form("%0.1f < p(pion) < %0.1f",
                                     Vary_pT_pr_pi[k], Vary_pT_pr_pi[k + 1]));
    h_Ncls_pion_pT[k]->GetXaxis()->SetTitle("NclsTPC");
    h_Ncls_pion_pT[k]->GetYaxis()->SetTitle("Entries");
    h_Ncls_pion_pT[k]->GetYaxis()->SetRangeUser(10, 100000);
    h_Ncls_pion_pT[k]->SetLineColor(kBlack);

    h_Ncls_proton_pT[k] =
        new TH1D(Form("h_Ncls_proton%d", k), "", 151, 9.5, 160.5);
    h_Ncls_proton_pT[k]->SetTitle(Form("%0.1f < p(proton) < %0.1f",
                                       Vary_pT_pr_pi[k], Vary_pT_pr_pi[k + 1]));
    h_Ncls_proton_pT[k]->GetXaxis()->SetTitle("NclsTPC");
    h_Ncls_proton_pT[k]->GetYaxis()->SetTitle("Entries");
    h_Ncls_proton_pT[k]->GetYaxis()->SetRangeUser(10, 1200000);
    h_Ncls_proton_pT[k]->SetLineColor(kBlack);

    ///--- Ncls vs Phi

    h_Ncls_pion_Phi[k] = new TH2D(Form("h_Ncls_pion_Phi%d", k), "", 200, 0,
                                  2. * TMath::Pi(), 151, 9.5, 160.5);
    h_Ncls_pion_Phi[k]->SetTitle(Form("%0.1f < p(pion) < %0.1f",
                                      Vary_pT_pr_pi[k], Vary_pT_pr_pi[k + 1]));
    h_Ncls_pion_Phi[k]->GetYaxis()->SetTitle("NclsTPC");
    h_Ncls_pion_Phi[k]->GetXaxis()->SetTitle("#phi");

    h_Ncls_proton_Phi[k] = new TH2D(Form("h_Ncls_proton_Phi%d", k), "", 200, 0,
                                    2. * TMath::Pi(), 151, 9.5, 160.5);
    h_Ncls_proton_Phi[k]->SetTitle(Form(
        "%0.1f < p(proton) < %0.1f", Vary_pT_pr_pi[k], Vary_pT_pr_pi[k + 1]));
    h_Ncls_proton_Phi[k]->GetYaxis()->SetTitle("NclsTPC");
    h_Ncls_proton_Phi[k]->GetXaxis()->SetTitle("#phi");

    ///---- dEdx vs Phi
    h_dEdx_Phi_pion[k] = new TH2D(Form("h_dEdx_Phi_pion%d", k), "", 200, 0,
                                  2. * TMath::Pi(), 150, 10, 160);
    h_dEdx_Phi_pion[k]->SetTitle(Form("%0.1f < p(pion) < %0.1f",
                                      Vary_pT_pr_pi[k], Vary_pT_pr_pi[k + 1]));
    h_dEdx_Phi_pion[k]->GetYaxis()->SetTitle("dE/dx");
    h_dEdx_Phi_pion[k]->GetXaxis()->SetTitle("#phi");

    h_dEdx_Phi_proton[k] = new TH2D(Form("h_dEdx_Phi_proton%d", k), "", 200, 0,
                                    2. * TMath::Pi(), 150, 10, 160);
    h_dEdx_Phi_proton[k]->SetTitle(Form(
        "%0.1f < p(proton) < %0.1f", Vary_pT_pr_pi[k], Vary_pT_pr_pi[k + 1]));
    h_dEdx_Phi_proton[k]->GetYaxis()->SetTitle("dE/dx");
    h_dEdx_Phi_proton[k]->GetXaxis()->SetTitle("#phi");

    ////--- define
    for (Int_t p = 0; p < 2; p++) {

      Chagre_range = Tracks_Charge[p];

      ///---- pion

      //  cout << " ****** check  " << k << "   Vary_pT_pr_pi[k],   " <<
      //  Vary_pT_pr_pi[k] << "   to     " <<  Vary_pT_pr_pi[k+1] << endl;

      h_pion_dEdx_NclsTPC_Charge[k][p] =
          new TH2D(Form("h_pion_dEdx_NclsTPC_Charge%d_%d", k, p), "", 151, 9.5,
                   160.5, 200, 10, 210);
      h_pion_dEdx_NclsTPC_Charge[k][p]->SetTitle(
          Form("%0.1f < p(pion) < %0.1f (%s)", Vary_pT_pr_pi[k],
               Vary_pT_pr_pi[k + 1], Chagre_range.c_str()));
      h_pion_dEdx_NclsTPC_Charge[k][p]->GetYaxis()->SetTitle("dE/dx");
      h_pion_dEdx_NclsTPC_Charge[k][p]->GetXaxis()->SetTitle("NclsTPC");
      h_pion_dEdx_NclsTPC_Charge[k][p]->GetYaxis()->SetRangeUser(1, 160);
      h_pion_dEdx_NclsTPC_Charge[k][p]->GetXaxis()->SetRangeUser(20, 160);

      ///--- proton

      h_proton_dEdx_NclsTPC_Charge[k][p] =
          new TH2D(Form("h_proton_dEdx_NclsTPC_Charge%d_%d", k, p), "", 151,
                   9.5, 160.5, 200, 10, 210);
      h_proton_dEdx_NclsTPC_Charge[k][p]->SetTitle(
          Form("%0.1f < p(proton) < %0.1f (%s)", Vary_pT_pr_pi[k],
               Vary_pT_pr_pi[k + 1], Chagre_range.c_str()));
      h_proton_dEdx_NclsTPC_Charge[k][p]->GetYaxis()->SetTitle("dE/dx");
      h_proton_dEdx_NclsTPC_Charge[k][p]->GetXaxis()->SetTitle("NclsTPC");
      h_proton_dEdx_NclsTPC_Charge[k][p]->GetYaxis()->SetRangeUser(1, 160);
      h_proton_dEdx_NclsTPC_Charge[k][p]->GetXaxis()->SetRangeUser(20, 160);

      ///----- check Ncls cuts : dEdx vs Phi
      Ncls_range = Ncls[p];

      h_dEdx_Phi_pion_Ncls[k][p] =
          new TH2D(Form("h_dEdx_Phi_pion_Ncls%d_%d", k, p), "", 200, 0,
                   2. * TMath::Pi(), 150, 10, 160);
      h_dEdx_Phi_pion_Ncls[k][p]->SetTitle(
          Form("%0.1f < p(pion) < %0.1f, %s", Vary_pT_pr_pi[k],
               Vary_pT_pr_pi[k + 1], Ncls_range.c_str()));
      h_dEdx_Phi_pion_Ncls[k][p]->GetYaxis()->SetTitle("dE/dx");
      h_dEdx_Phi_pion_Ncls[k][p]->GetXaxis()->SetTitle("#phi");

      h_dEdx_Phi_proton_Ncls[k][p] =
          new TH2D(Form("h_dEdx_Phi_proton_Ncls%d_%d", k, p), "", 200, 0,
                   2. * TMath::Pi(), 150, 10, 160);
      h_dEdx_Phi_proton_Ncls[k][p]->SetTitle(
          Form("%0.1f < p(proton) < %0.1f, %s", Vary_pT_pr_pi[k],
               Vary_pT_pr_pi[k + 1], Ncls_range.c_str()));
      h_dEdx_Phi_proton_Ncls[k][p]->GetYaxis()->SetTitle("dE/dx");
      h_dEdx_Phi_proton_Ncls[k][p]->GetXaxis()->SetTitle("#phi");

    } // dEdx range
  }

  ///-----------------------------------------------------------------------
  ///----- read the tree
  ///-----------------------------------------------------------------------

  Tree_skimmed_V0->SetBranchAddress("fEta", &fEta);
  Tree_skimmed_V0->SetBranchAddress("fPidIndex", &fPidIndex);
  Tree_skimmed_V0->SetBranchAddress("fMass", &fMass);
  Tree_skimmed_V0->SetBranchAddress("fTPCSignal", &fTPCSignal);
  Tree_skimmed_V0->SetBranchAddress("fTPCInnerParam", &fTPCInnerParam);
  Tree_skimmed_V0->SetBranchAddress("fNormNClustersTPC", &fNormNClustersTPC);
  Tree_skimmed_V0->SetBranchAddress("fNSigTPC", &fNSigTPC);
  Tree_skimmed_V0->SetBranchAddress("fBetaGamma", &fBetaGamma);
  Tree_skimmed_V0->SetBranchAddress("fInvDeDxExpTPC", &fInvDeDxExpTPC);
  Tree_skimmed_V0->SetBranchAddress("fRunNumber", &fRunNumber);
  Tree_skimmed_V0->SetBranchAddress("fSigned1Pt", &fSigned1Pt);
  Tree_skimmed_V0->SetBranchAddress("fPhi", &fPhi);
  Tree_skimmed_V0->SetBranchAddress("fTgl", &fTgl);

  ///--------------------

  /////----------------- check Run number --------------------
  ////----- create a set to store the values of Run Number
  std::set<Int_t> uniqueValues;

  std::set<Int_t> IndexCheck;

  /////---- check the number of
  Long64_t nTrk = Tree_skimmed_V0->GetEntries();

  /////------
  const int RunNumber_RCT_Mixed[] = {539622, 539556};

  const int particleIndex[] = {59889664};

  for (Int_t i = 0; i < nTrk; i++) {

    Tree_skimmed_V0->GetEntry(i);
    if (fEta >= 0) continue;
   // if (fPhi <= 11.*TMath::Pi()/9. || fPhi >= 12.*TMath::Pi()/9.) continue;
    // /////----------------------- check Run Number ---------------------

    //////------- exclude all those mixed runs from the tree
    ///// Use std::find to check if the run number is in the array

    //  if( std::find(std::begin(RunNumber_RCT_Mixed),
    //  std::end(RunNumber_RCT_Mixed), fRunNumber) !=
    //  std::end(RunNumber_RCT_Mixed) )   continue;

    // if(std::find(std::begin(particleIndex), std::end(particleIndex),
    // fPidIndex ) != std::end(particleIndex)  ) continue;

    ////----  Add the value to the set
    uniqueValues.insert(fRunNumber);

    IndexCheck.insert(fPidIndex);

    /////-----------------------------------------------------------
    ////------ here have to calculate to get momentum:
    ///https://github.com/AliceO2Group/AliceO2/blob/dev/Framework/Core/include/Framework/AnalysisDataModel.h#L156-L157
    ///--- 0.5 *  ( TMath::Tan( TMath::PI/4 - 0.5* TMath::ATan(fTgl))  +  1/(
    ///TMath::Tan( TMath::PI/4 - 0.5* TMath::ATan(fTgl))) )  * (1/fSigned1Pt)

    ////---- all tracks
    if (fTPCInnerParam > 0.6 && fTPCInnerParam < 5.0) {

      //---- check pt vs Pin
      if (152 / (fNormNClustersTPC * fNormNClustersTPC) > 130) {

        h_AllTracks_PIn_P_Ncls[0]->Fill(
            0.5 *
                (TMath::Tan(TMath::Pi() / 4 - 0.5 * TMath::ATan(fTgl)) +
                 1 / (TMath::Tan(TMath::Pi() / 4 - 0.5 * TMath::ATan(fTgl)))) *
                (1 / fSigned1Pt),
            fTPCInnerParam);
      }

      if (152 / (fNormNClustersTPC * fNormNClustersTPC) < 80) {

        h_AllTracks_PIn_P_Ncls[1]->Fill(
            0.5 *
                (TMath::Tan(TMath::Pi() / 4 - 0.5 * TMath::ATan(fTgl)) +
                 1 / (TMath::Tan(TMath::Pi() / 4 - 0.5 * TMath::ATan(fTgl)))) *
                (1 / fSigned1Pt),
            fTPCInnerParam);
      }

      h_AllTrack_PIn_P_AllNcls->Fill(
          0.5 *
              (TMath::Tan(TMath::Pi() / 4 - 0.5 * TMath::ATan(fTgl)) +
               1 / (TMath::Tan(TMath::Pi() / 4 - 0.5 * TMath::ATan(fTgl)))) *
              (1 / fSigned1Pt),
          fTPCInnerParam);
    }

    ///---------------------- electron
    if (fabs(fPidIndex) == 0) {

      //---- fill NclsTPC
      if (fTPCInnerParam > 0.6 && fTPCInnerParam < 5.0) {

        h_Ncls_ele->Fill(152 / (fNormNClustersTPC * fNormNClustersTPC));

        ///---- fill dE/dx vs pT
        h_electron_dEdx_vs_pIn_V0->Fill(fTPCInnerParam, fTPCSignal);

        //---- check p vs Pin
        if (152 / (fNormNClustersTPC * fNormNClustersTPC) > 130) {

          h_electron_PIn_P_Ncls[0]->Fill(
              0.5 *
                  (TMath::Tan(TMath::Pi() / 4 - 0.5 * TMath::ATan(fTgl)) +
                   1 / (TMath::Tan(TMath::Pi() / 4 -
                                   0.5 * TMath::ATan(fTgl)))) *
                  (1 / fSigned1Pt),
              fTPCInnerParam);
        }
        if (152 / (fNormNClustersTPC * fNormNClustersTPC) < 80) {

          h_electron_PIn_P_Ncls[1]->Fill(
              0.5 *
                  (TMath::Tan(TMath::Pi() / 4 - 0.5 * TMath::ATan(fTgl)) +
                   1 / (TMath::Tan(TMath::Pi() / 4 -
                                   0.5 * TMath::ATan(fTgl)))) *
                  (1 / fSigned1Pt),
              fTPCInnerParam);
        }

        h_electron_PIn_P_AllNcls->Fill(
            0.5 *
                (TMath::Tan(TMath::Pi() / 4 - 0.5 * TMath::ATan(fTgl)) +
                 1 / (TMath::Tan(TMath::Pi() / 4 - 0.5 * TMath::ATan(fTgl)))) *
                (1 / fSigned1Pt),
            fTPCInnerParam);
      }

      for (Int_t j = 0; j < 8; j++) {

        if (fTPCInnerParam > Vary_pT[j] && fTPCInnerParam < Vary_pT[j + 1]) {

          //-- here there is a calculation to get the number of TPCCluster
          //founded, 152/fNormNClustersTPC^2
          h_Electron_dEdx_vs_NclTPC_V0[j]->Fill(
              152 / (fNormNClustersTPC * fNormNClustersTPC), fTPCSignal);

          h_dEdxExp_ele_V0[j]->Fill(1 / fInvDeDxExpTPC);

          //--- fill NCls
          h_Ncls_ele_pT[j]->Fill(152 / (fNormNClustersTPC * fNormNClustersTPC));

          ///--------------------------------------------------------------

          if (fSigned1Pt > 0) {

            h_electron_dEdx_NclsTPC_Chagre[j][0]->Fill(
                152 / (fNormNClustersTPC * fNormNClustersTPC), fTPCSignal);

          } else {

            h_electron_dEdx_NclsTPC_Chagre[j][1]->Fill(
                152 / (fNormNClustersTPC * fNormNClustersTPC), fTPCSignal);
          }

          ////--- fill Ncls vs Phi
          h_Ncls_ele_Phi[j]->Fill(
              fPhi, 152 / (fNormNClustersTPC * fNormNClustersTPC));

          ///--- fill dEdx vs Phi
          h_dEdx_Phi_ele[j]->Fill(fPhi, fTPCSignal);

          ////------- check with NclsTPC

          if (152 / (fNormNClustersTPC * fNormNClustersTPC) > 130) {

            h_dEdx_Phi_ele_Ncls[j][0]->Fill(fPhi, fTPCSignal);
          }
          if (152 / (fNormNClustersTPC * fNormNClustersTPC) < 80) {

            h_dEdx_Phi_ele_Ncls[j][1]->Fill(fPhi, fTPCSignal);
          }
        }

      } // pT interval

    } // electron

    ///-----------------------------------------------------------------------------------------------

    ///---------------------- pion
    if (fabs(fPidIndex) == 2) {

      ///---- fill NClsTPC
      if (fTPCInnerParam > 0.6 && fTPCInnerParam < 15) {

        h_Ncls_pion->Fill(152 / (fNormNClustersTPC * fNormNClustersTPC));

        ///---- fill dE/dx vs pT

        h_pion_dEdx_vs_pIn_V0->Fill(fTPCInnerParam, fTPCSignal);

        //---- check p vs Pin
        if (152 / (fNormNClustersTPC * fNormNClustersTPC) > 130) {

          h_pion_PIn_P_Ncls[0]->Fill(
              0.5 *
                  (TMath::Tan(TMath::Pi() / 4 - 0.5 * TMath::ATan(fTgl)) +
                   1 / (TMath::Tan(TMath::Pi() / 4 -
                                   0.5 * TMath::ATan(fTgl)))) *
                  (1 / fSigned1Pt),
              fTPCInnerParam);
        }
        if (152 / (fNormNClustersTPC * fNormNClustersTPC) < 80) {

          h_pion_PIn_P_Ncls[1]->Fill(
              0.5 *
                  (TMath::Tan(TMath::Pi() / 4 - 0.5 * TMath::ATan(fTgl)) +
                   1 / (TMath::Tan(TMath::Pi() / 4 -
                                   0.5 * TMath::ATan(fTgl)))) *
                  (1 / fSigned1Pt),
              fTPCInnerParam);
        }

        h_pion_PIn_P_AllNcls->Fill(
            0.5 *
                (TMath::Tan(TMath::Pi() / 4 - 0.5 * TMath::ATan(fTgl)) +
                 1 / (TMath::Tan(TMath::Pi() / 4 - 0.5 * TMath::ATan(fTgl)))) *
                (1 / fSigned1Pt),
            fTPCInnerParam);
      }

      for (Int_t j = 0; j < 14; j++) {

        if (fTPCInnerParam > Vary_pT_pr_pi[j] &&
            fTPCInnerParam < Vary_pT_pr_pi[j + 1]) {

          h_pion_dEdx_vs_NclTPC_V0[j]->Fill(
              152 / (fNormNClustersTPC * fNormNClustersTPC), fTPCSignal);

          ////--- fill NCls

          h_Ncls_pion_pT[j]->Fill(152 /
                                  (fNormNClustersTPC * fNormNClustersTPC));

          if (fSigned1Pt > 0) {
            h_pion_dEdx_NclsTPC_Charge[j][0]->Fill(
                152 / (fNormNClustersTPC * fNormNClustersTPC), fTPCSignal);

          } else {
            h_pion_dEdx_NclsTPC_Charge[j][1]->Fill(
                152 / (fNormNClustersTPC * fNormNClustersTPC), fTPCSignal);
          }

          ////--- fill Ncls vs Phi

          h_Ncls_pion_Phi[j]->Fill(
              fPhi, 152 / (fNormNClustersTPC * fNormNClustersTPC));

          ///--- fill dEdx vs Phi
          h_dEdx_Phi_pion[j]->Fill(fPhi, fTPCSignal);

          ////------- check with NclsTPC

          if (152 / (fNormNClustersTPC * fNormNClustersTPC) > 130) {

            h_dEdx_Phi_pion_Ncls[j][0]->Fill(fPhi, fTPCSignal);
          }
          if (152 / (fNormNClustersTPC * fNormNClustersTPC) < 80) {

            h_dEdx_Phi_pion_Ncls[j][1]->Fill(fPhi, fTPCSignal);
          }

        } /// pt interval loop
      }
    }

    ///-------------------- proton

    if (fabs(fPidIndex) == 4) {

      ///---- fill NClsTPC
      if (fTPCInnerParam > 0.6 && fTPCInnerParam < 15) {

        h_Ncls_proton->Fill(152 / (fNormNClustersTPC * fNormNClustersTPC));

        ///---- fill dE/dx vs pT

        h_proton_dEdx_vs_pIn_V0->Fill(fTPCInnerParam, fTPCSignal);

        //---- check pt vs Pin
        if (152 / (fNormNClustersTPC * fNormNClustersTPC) > 130) {

          h_proton_PIn_P_Ncls[0]->Fill(
              0.5 *
                  (TMath::Tan(TMath::Pi() / 4 - 0.5 * TMath::ATan(fTgl)) +
                   1 / (TMath::Tan(TMath::Pi() / 4 -
                                   0.5 * TMath::ATan(fTgl)))) *
                  (1 / fSigned1Pt),
              fTPCInnerParam);
        }
        if (152 / (fNormNClustersTPC * fNormNClustersTPC) < 80) {

          h_proton_PIn_P_Ncls[1]->Fill(
              0.5 *
                  (TMath::Tan(TMath::Pi() / 4 - 0.5 * TMath::ATan(fTgl)) +
                   1 / (TMath::Tan(TMath::Pi() / 4 -
                                   0.5 * TMath::ATan(fTgl)))) *
                  (1 / fSigned1Pt),
              fTPCInnerParam);
        }

        h_proton_PIn_P_AllNcls->Fill(
            0.5 *
                (TMath::Tan(TMath::Pi() / 4 - 0.5 * TMath::ATan(fTgl)) +
                 1 / (TMath::Tan(TMath::Pi() / 4 - 0.5 * TMath::ATan(fTgl)))) *
                (1 / fSigned1Pt),
            fTPCInnerParam);
      }

      for (Int_t j = 0; j < 14; j++) {

        if (fTPCInnerParam > Vary_pT_pr_pi[j] &&
            fTPCInnerParam < Vary_pT_pr_pi[j + 1]) {

          h_proton_dEdx_vs_NclTPC_V0[j]->Fill(
              152 / (fNormNClustersTPC * fNormNClustersTPC), fTPCSignal);

          //--- fill NCls
          h_Ncls_proton_pT[j]->Fill(152 /
                                    (fNormNClustersTPC * fNormNClustersTPC));

          ////---- fill Eta

          // if( fTPCSignal < 70 ){
          //     h_proton_dEdx_NclsTPC_Charge[j][0] -> Fill(fEta);

          // }else{
          //     h_proton_dEdx_NclsTPC_Charge[j][1] -> Fill(fEta);

          // }

          if (fSigned1Pt > 0) {
            h_proton_dEdx_NclsTPC_Charge[j][0]->Fill(
                152 / (fNormNClustersTPC * fNormNClustersTPC), fTPCSignal);

          } else {
            h_proton_dEdx_NclsTPC_Charge[j][1]->Fill(
                152 / (fNormNClustersTPC * fNormNClustersTPC), fTPCSignal);
          }

          ////--- fill Ncls vs Phi

          h_Ncls_proton_Phi[j]->Fill(
              fPhi, 152 / (fNormNClustersTPC * fNormNClustersTPC));

          ///--- fill dEdx vs Phi
          h_dEdx_Phi_proton[j]->Fill(fPhi, fTPCSignal);

          ////------- check with NclsTPC

          if (152 / (fNormNClustersTPC * fNormNClustersTPC) > 130) {

            h_dEdx_Phi_proton_Ncls[j][0]->Fill(fPhi, fTPCSignal);
          }
          if (152 / (fNormNClustersTPC * fNormNClustersTPC) < 80) {

            h_dEdx_Phi_proton_Ncls[j][1]->Fill(fPhi, fTPCSignal);
          }
        }
      }
    }

    ///-----------------------------------------------------
  } // Tree_skimmed_V0 entry loop
  ///-----------------------------------------------------

  // ///-------------------------------------------------------------------------
  // ///-------------------------------------------------------------------------

  // for (const auto &uniqueValue : uniqueValues) {
  //         printf("%.2d , ", uniqueValue);

  //     }

  //     cout << " ***************************" << endl;

  //  for( const auto &IndexCheck1 :  IndexCheck){

  //       printf("%.7d , ", IndexCheck1);
  //  }

  // ////-------------------------------------------------------------

  // ///------- draw histogram
  for (Int_t j = 0; j < 8; j++) {

    ///---------- TH2D: Ncls vs dE/dx
    TCanvas *c1 = new TCanvas();
    c1->SetTickx(1);
    c1->SetTicky(1);
    // c1 -> SetLogx();
    c1->SetLogz();
    c1->cd(j + 1);
    h_Electron_dEdx_vs_NclTPC_V0[j]->Draw("colz");
    TLine *l_NclsTPC_130 = new TLine(130, 10, 130, 160);
    l_NclsTPC_130->SetLineColor(kRed);
    l_NclsTPC_130->SetLineStyle(9);
    l_NclsTPC_130->SetLineStyle(9);
    l_NclsTPC_130->Draw("same");
    //  h_dEdxExp_ele_V0[j] -> Draw("same");
    c1->SaveAs(outputDir + "/h_2D_Electron_dEdx_vs_NclTPC__" +
               Form("%0.1f_pT_%0.1f__skimtreeV0_NclsTPC_20_to_160.png",
                    Vary_pT[j], Vary_pT[j + 1]));

    //   ////---------- TH1D: Ncls in pT

    //     TCanvas *c_n = new TCanvas();
    //     c_n -> SetTickx(1);
    //     c_n -> SetTicky(1);
    //     c_n -> SetLogy();
    //     h_Ncls_ele_pT[j] -> Draw("hist");
    //   c_n -> SaveAs(outputDir + "/h_1D_Electron_NclTPC_in_" +
    //   Form("%0.1f_pT_%0.1f__skimtreeV0_.png", Vary_pT[j], Vary_pT[j+1] ));

    ////---- TH2D: Ncls vs Phi
    TCanvas *c_phi_e = new TCanvas();
    c_phi_e->SetTickx(1);
    c_phi_e->SetTicky(1);
    c_phi_e->SetLogz();
    h_Ncls_ele_Phi[j]->Draw("colz");
    DrawLine(c_phi_e,h_Ncls_ele_Phi[j]);
    c_phi_e->SaveAs(outputDir + "/h_2D_electron_NclTPC_vs_Phi_in_" +
                    Form("%0.1f_pT_%0.1f__skimtreeV0_.png", Vary_pT_pr_pi[j],
                         Vary_pT_pr_pi[j + 1]));

    ///--- TH2D: dEdx vs Phi

    TCanvas *c_phi_e1 = new TCanvas();
    c_phi_e1->SetTickx(1);
    c_phi_e1->SetTicky(1);
    c_phi_e1->SetLogz();
    h_dEdx_Phi_ele[j]->Draw("colz");
    DrawLine(c_phi_e1,h_dEdx_Phi_ele[j]);
    c_phi_e1->SaveAs(outputDir + "/h_2D_electron_dEdx_vs_Phi_in_" +
                     Form("%0.1f_pT_%0.1f__skimtreeV0_.png", Vary_pT_pr_pi[j],
                          Vary_pT_pr_pi[j + 1]));
  }
  ////------------- pion, proton

  for (Int_t j = 0; j < 14; j++) {

    TCanvas *c2 = new TCanvas();
    c2->SetLogz();
    // c2 -> SetLogx();
    c2->SetTickx(1);
    c2->SetTicky(1);
    c2->cd(j + 1);
    h_pion_dEdx_vs_NclTPC_V0[j]->Draw("colz");
    TLine *l_NclsTPC_130 = new TLine(130, 10, 130, 120);
    l_NclsTPC_130->SetLineColor(kRed);
    l_NclsTPC_130->SetLineStyle(9);
    l_NclsTPC_130->SetLineWidth(2);
    l_NclsTPC_130->Draw("same");

    c2->SaveAs(outputDir + "/h_2D_Pion_dEdx_vs_NclTPC__" +
               Form("%0.1f_pT_%0.1f__skimtreeV0_NclsTPC_20_to_160.png",
                    Vary_pT_pr_pi[j], Vary_pT_pr_pi[j + 1]));

    TCanvas *c3 = new TCanvas();
    c3->SetLogz();
    // c3 -> SetLogx();
    c3->SetTickx(1);
    c3->SetTicky(1);
    c3->cd(j + 1);
    h_proton_dEdx_vs_NclTPC_V0[j]->Draw("colz");

    TLine *l_NclsTPC_130_pr = new TLine(130, 10, 130, 160);
    l_NclsTPC_130_pr->SetLineColor(kRed);
    l_NclsTPC_130_pr->SetLineStyle(9);
    l_NclsTPC_130_pr->SetLineWidth(2);
    l_NclsTPC_130_pr->Draw("same");

    c3->SaveAs(outputDir + "/h_2D_Proton_dEdx_vs_NclTPC__" +
               Form("%0.1f_pT_%0.1f__skimtreeV0_NclsTPC_20_to_160.png",
                    Vary_pT_pr_pi[j], Vary_pT_pr_pi[j + 1]));

    ///------------------

    ////---------- TH1D: Ncls in pT

    // TCanvas *c_n_pion = new TCanvas();
    // c_n_pion -> SetTickx(1);
    // c_n_pion -> SetTicky(1);
    // c_n_pion -> SetLogy();
    // h_Ncls_pion_pT[j] -> Draw("hist");
    // c_n_pion -> SaveAs(outputDir + "/h_1D_pion_NclTPC_in_" +
    // Form("%0.1f_pT_%0.1f__skimtreeV0_.png", Vary_pT_pr_pi[j],
    // Vary_pT_pr_pi[j+1] ));

    // TCanvas *c_n_proton = new TCanvas();
    // c_n_proton -> SetTickx(1);
    // c_n_proton -> SetTicky(1);
    // c_n_proton -> SetLogy();
    // h_Ncls_proton_pT[j] -> Draw("hist");
    // c_n_proton -> SaveAs(outputDir + "/h_1D_proton_NclTPC_in_" +
    // Form("%0.1f_pT_%0.1f__skimtreeV0_.png", Vary_pT_pr_pi[j],
    // Vary_pT_pr_pi[j+1] ));

    ///----- TH2D: Ncls vs Phi
    TCanvas *c_phi_pion = new TCanvas();
    c_phi_pion->SetTickx(1);
    c_phi_pion->SetTicky(1);
    c_phi_pion->SetLogz();
    h_Ncls_pion_Phi[j]->Draw("colz");
    DrawLine(c_phi_pion,h_Ncls_pion_Phi[j]);

    c_phi_pion->SaveAs(outputDir + "/h_2D_pion_NclTPC_vs_Phi_in_" +
                       Form("%0.1f_pT_%0.1f__skimtreeV0_.png", Vary_pT_pr_pi[j],
                            Vary_pT_pr_pi[j + 1]));

    TCanvas *c_phi_pr = new TCanvas();
    c_phi_pr->SetTickx(1);
    c_phi_pr->SetTicky(1);
    c_phi_pr->SetLogz();
    h_Ncls_proton_Phi[j]->Draw("colz");
    DrawLine(c_phi_pr,h_Ncls_proton_Phi[j]);
    c_phi_pr->SaveAs(outputDir + "/h_2D_proton_NclTPC_vs_Phi_in_" +
                     Form("%0.1f_pT_%0.1f__skimtreeV0_.png", Vary_pT_pr_pi[j],
                          Vary_pT_pr_pi[j + 1]));

    ///--- TH2D: dEdx vs Phi

    TCanvas *c_phi_pion1 = new TCanvas();
    c_phi_pion1->SetTickx(1);
    c_phi_pion1->SetTicky(1);
    c_phi_pion1->SetLogz();
    h_dEdx_Phi_pion[j]->Draw("colz");
    DrawLine(c_phi_pion1,h_dEdx_Phi_pion[j]);
    c_phi_pion1->SaveAs(outputDir + "/h_2D_pion_dEdx_vs_Phi_in_" +
                        Form("%0.1f_pT_%0.1f__skimtreeV0_.png",
                             Vary_pT_pr_pi[j], Vary_pT_pr_pi[j + 1]));

    TCanvas *c_phi_pr1 = new TCanvas();
    c_phi_pr1->SetTickx(1);
    c_phi_pr1->SetTicky(1);
    c_phi_pr1->SetLogz();
    h_dEdx_Phi_proton[j]->Draw("colz");
    DrawLine(c_phi_pr1,h_dEdx_Phi_proton[j]);
    c_phi_pr1->SaveAs(outputDir + "/h_2D_proton_dEdx_vs_Phi_in_" +
                      Form("%0.1f_pT_%0.1f__skimtreeV0_.png", Vary_pT_pr_pi[j],
                           Vary_pT_pr_pi[j + 1]));

    ////////////
  }
  ///////

  //////--------------------------------------------------------------------
  //////--------------------  NclsTPC check
  //////--------------------------------------------------------------------

  // TCanvas *n_1 = new TCanvas();
  // n_1 -> SetLogy();
  // n_1 -> SetTickx(1);
  // n_1 -> SetTicky(1);
  // h_Ncls_ele -> Draw("hist");
  // n_1 -> SaveAs(outputDir + "/h_1D_NClsTPC_IntegratedCheck_electron.png"  );

  // TCanvas *n_2 = new TCanvas();
  // n_2 -> SetLogy();
  // n_2 -> SetTickx(1);
  // n_2 -> SetTicky(1);
  // h_Ncls_pion -> Draw("hist");
  // n_2 -> SaveAs(outputDir + "/h_1D_NClsTPC_IntegratedCheck_pion.png"  );

  // TCanvas *n_3 = new TCanvas();
  // n_3 -> SetLogy();
  // n_3 -> SetTickx(1);
  // n_3 -> SetTicky(1);
  // h_Ncls_proton -> Draw("hist");
  // n_3 -> SaveAs(outputDir + "/h_1D_NClsTPC_IntegratedCheck_proton.png"  );

  // ////--------------- draw TH2D: dEdx vs PIn

  // TCanvas *cc1 = new TCanvas();
  // cc1 -> SetTickx(1);
  // cc1 -> SetTicky(1);
  // cc1 -> SetLogz();
  // h_electron_dEdx_vs_pIn_V0 -> Draw("colz");
  // cc1 -> SaveAs(outputDir + "/h_2D_dEdx_vs_pT_electron.png"  );

  // TCanvas *cc2 = new TCanvas();
  // cc2 -> SetTickx(1);
  // cc2 -> SetTicky(1);
  // cc2 -> SetLogz();
  // h_pion_dEdx_vs_pIn_V0 -> Draw("colz");
  // cc2 -> SaveAs(outputDir + "/h_2D_dEdx_vs_pT_pion.png"  );

  // TCanvas *cc3 = new TCanvas();
  // cc3 -> SetTickx(1);
  // cc3 -> SetTicky(1);
  // cc3 -> SetLogz();
  // h_proton_dEdx_vs_pIn_V0 -> Draw("colz");
  // cc3 -> SaveAs(outputDir + "/h_2D_dEdx_vs_pT_proton.png"  );

  // /////------------------ Check  distribution with different pT intervals,
  // and different dEdx ranges

  for (Int_t j = 0; j < 8; j++) {

    for (Int_t p = 0; p < 2; p++) {

      Chagre_range = Tracks_Charge[p];

      TCanvas *c1 = new TCanvas();
      c1->SetTickx(1);
      c1->SetTicky(1);
      c1->SetLogz();
      h_electron_dEdx_NclsTPC_Chagre[j][p]->Draw("colz");
      c1->SaveAs(outputDir +
                 Form("/h_2D_electron_dEdx_vs_NClsTPC__pIn_%.1f__%.1f__%s.png",
                      Vary_pT[j], Vary_pT[j + 1], Chagre_range.c_str()));

      ///---- check Ncls two ranges:  dEdx vs Phi
      Ncls_range = Ncls[p];

      TCanvas *c11 = new TCanvas();
      c11->SetTickx(1);
      c11->SetTicky(1);
      c11->SetLogz();
      h_dEdx_Phi_ele_Ncls[j][p]->Draw("colz");
      DrawLine(c11,h_dEdx_Phi_ele_Ncls[j][p]);
      c11->SaveAs(outputDir +
                  Form("/h_2D_electron_dEdx_vs_Phi_Ncls_%.1f__%.1f__%s.png",
                       Vary_pT[j], Vary_pT[j + 1], Ncls_range.c_str()));
    }
  }

  for (Int_t j = 0; j < 14; j++) {

    for (Int_t p = 0; p < 2; p++) {

      Chagre_range = Tracks_Charge[p];
      Ncls_range = Ncls[p];

      ///--------- pion

      TCanvas *c1 = new TCanvas();
      c1->SetTickx(1);
      c1->SetTicky(1);
      c1->SetLogz();
      h_pion_dEdx_NclsTPC_Charge[j][p]->Draw("colz");

      c1->SaveAs(outputDir +
                 Form("/h_2D_pion_dEdx_vs_NClsTPC_pIn_%0.1f__%0.1f__%s.png",
                      Vary_pT_pr_pi[j], Vary_pT_pr_pi[j + 1],
                      Chagre_range.c_str()));

      /////---- check Ncls two ranges:  dEdx vs Phi
      TCanvas *c11 = new TCanvas();
      c11->SetTickx(1);
      c11->SetTicky(1);
      c11->SetLogz();
      h_dEdx_Phi_pion_Ncls[j][p]->Draw("colz");
      DrawLine(c11,h_dEdx_Phi_pion_Ncls[j][p]);
      c11->SaveAs(outputDir +
                  Form("/h_2D_pion_dEdx_vs_Phi_NCls_%0.1f__%0.1f__%s.png",
                       Vary_pT_pr_pi[j], Vary_pT_pr_pi[j + 1],
                       Ncls_range.c_str()));

      ///----- proton
      TCanvas *c2 = new TCanvas();
      c2->SetTickx(1);
      c2->SetTicky(1);
      c2->SetLogz();
      h_proton_dEdx_NclsTPC_Charge[j][p]->Draw("colz");
      c2->SaveAs(outputDir +
                 Form("/h_2D_proton_dEdx_vs_NClsTPC_pIn_%0.1f__%0.1f__%s.png",
                      Vary_pT_pr_pi[j], Vary_pT_pr_pi[j + 1],
                      Chagre_range.c_str()));

      TCanvas *c22 = new TCanvas();
      c22->SetTickx(1);
      c22->SetTicky(1);
      c22->SetLogz();
      h_dEdx_Phi_proton_Ncls[j][p]->Draw("colz");
      DrawLine(c22,h_dEdx_Phi_proton_Ncls[j][p]);
      c22->SaveAs(outputDir +
                  Form("/h_2D_proton_dEdx_vs_Phi_NCls_%0.1f__%0.1f__%s.png",
                       Vary_pT_pr_pi[j], Vary_pT_pr_pi[j + 1],
                       Ncls_range.c_str()));
    }
  }

  ////-------------------------------------------------------------------
  ///-------------- check the Pt vs PIn
  //-------------------------------------------------------------------
  for (Int_t p = 0; p < 2; p++) {

    Ncls_range = Ncls[p];

    ///--- electron
    TCanvas *c1e = new TCanvas();
    c1e->SetTickx(1);
    c1e->SetTicky(1);
    c1e->SetLogz();
    h_electron_PIn_P_Ncls[p]->Draw("colz");
    c1e->SaveAs(outputDir + Form("/h_2D_electron_Pt_vs_PIn_NclsRange__%s_.png",
                                 Ncls_range.c_str()));

    ///--- pion
    TCanvas *c1p = new TCanvas();
    c1p->SetTickx(1);
    c1p->SetTicky(1);
    c1p->SetLogz();
    h_pion_PIn_P_Ncls[p]->Draw("colz");
    c1p->SaveAs(outputDir + Form("/h_2D_pion_Pt_vs_PIn_NclsRange__%s_.png",
                                 Ncls_range.c_str()));

    ///--- proton
    TCanvas *c1pr = new TCanvas();
    c1pr->SetTickx(1);
    c1pr->SetTicky(1);
    c1pr->SetLogz();
    h_proton_PIn_P_Ncls[p]->Draw("colz");
    c1pr->SaveAs(outputDir + Form("/h_2D_proton_Pt_vs_PIn_NclsRange__%s_.png",
                                  Ncls_range.c_str()));

    ///---- all tracks
    TCanvas *call = new TCanvas();
    call->SetTickx(1);
    call->SetTicky(1);
    call->SetLogz();
    h_AllTracks_PIn_P_Ncls[p]->Draw("colz");
    call->SaveAs(outputDir +
                 Form("/h_2D_AllTracks_Pt_vs_PIn_NclsRange__%s_.png",
                      Ncls_range.c_str()));
  }

  TCanvas *c_allN = new TCanvas();
  c_allN->SetTickx(1);
  c_allN->SetTicky(1);
  c_allN->SetLogz();
  h_AllTrack_PIn_P_AllNcls->Draw("colz");
  c_allN->SaveAs(outputDir + "/h_2D_AllTracks_P_vs_PIn_.png");

  ///------ here first try diagnoal cuts on All Tracks:  X-axis: O

  // TLine *l_cut1;
  // Double_t slope1 = 0.95;
  // Double_t intercept1 = -2;
  // for(Int_t binx = 1; binx <= h_AllTrack_PIn_P_AllNcls -> GetNbinsX(); binx++
  // ){
  //     for(Int_t biny = 1; biny <= h_AllTrack_PIn_P_AllNcls -> GetNbinsY();
  //     biny++   )
  //     {
  //         double x_center =
  //         h_AllTrack_PIn_P_AllNcls->GetXaxis()->GetBinCenter(binx); double
  //         y_center =
  //         h_AllTrack_PIn_P_AllNcls->GetYaxis()->GetBinCenter(biny);

  //         // Calculate the expected y value on the diagonal line
  //         double expected_y = slope1 * x_center + intercept1;

  //         // Define the parameters for the diagonal line
  //         double xmin = h_AllTrack_PIn_P_AllNcls->GetXaxis()->GetXmin();
  //         double xmax = h_AllTrack_PIn_P_AllNcls->GetXaxis()->GetXmax();
  //         double ymin = slope1 * xmin + intercept1;
  //         double ymax = slope1 * xmax + intercept1;

  //         cout << "  ****** check  ymin :  " << ymin << "  and ymax   " <<
  //         ymax << endl;

  //         // Create a TLine object for the diagonal line
  //         l_cut1 = new TLine(xmin, ymin, xmax, ymax);
  //         l_cut1->SetLineColor(kRed); // Set line color to red
  //         l_cut1 -> SetLineWidth(2);

  //     }

  // }

  // TCanvas *c_line1 = new TCanvas();
  // c_line1 -> SetTickx(1);
  // c_line1 -> SetTicky(1);
  // c_line1 -> SetLogz();
  // h_AllTrack_PIn_P_AllNcls -> Draw("colz");
  // l_cut1 -> Draw("same");
  // c_line1 -> SaveAs( outputDir + "/h_2D_AllTracks_P_vs_PIn_DiagnoalCut.png");

  TCanvas *c_alle = new TCanvas();
  c_alle->SetTickx(1);
  c_alle->SetTicky(1);
  c_alle->SetLogz();
  h_electron_PIn_P_AllNcls->Draw("colz");
  c_alle->SaveAs(outputDir + "/h_2D_electron__P_vs_PIn_.png");

  TCanvas *c_allpi = new TCanvas();
  c_allpi->SetTickx(1);
  c_allpi->SetTicky(1);
  c_allpi->SetLogz();
  h_pion_PIn_P_AllNcls->Draw("colz");
  c_allpi->SaveAs(outputDir + "/h_2D_pion_P_vs_PIn_.png");

  TCanvas *c_allpr = new TCanvas();
  c_allpr->SetTickx(1);
  c_allpr->SetTicky(1);
  c_allpr->SetLogz();
  h_proton_PIn_P_AllNcls->Draw("colz");
  c_allpr->SaveAs(outputDir + "/h_2D_proton_P_vs_PIn_.png");
  f_output->Write();
  f_output->Close();
  ///***/********-****************************************************
} // QACheck_fromSkimmedTree
