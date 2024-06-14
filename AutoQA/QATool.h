#ifndef QATool_h
#define QATool_h

#include "MHead.h"

namespace QATool {
typedef struct BadRun_Str {
  double high_bound;
  double low_bound;
  double mean;
  double sigma;
  std::vector<double> vals;
  std::vector<int> bad_run;
} BadRun_Str;

BadRun_Str findBadRun(vector<double> vals, double nsigma) {
  BadRun_Str brs;
  // This function is used to select bad runs from a histogram
  // return a struct with the histogram, the high and low bound, and the bad run
  // list
  // reject until there is no run value outside the range of mean +/-
  // nsigma*sigma
  bool is_all_rejected = false;
  double mean, sigma;

  while (!is_all_rejected) {
    // calculate mean and sigma of the histogram without bad run bins and zero
    // value bins
    mean = 0;
    sigma = 0;
    int n = 0;
    for (int i = 0; i < vals.size(); i++) {
      if (vals[i] == 0 || vals[i] != vals[i])
        continue;
      bool is_badrun = false;
      for (auto j : brs.bad_run) {
        if (i == j) {
          is_badrun = true;
          break;
        }
      }
      if (is_badrun)
        continue;
      mean += vals[i];
      n++;
    }
    mean /= n;

    for (int i = 0; i < vals.size(); i++) {
      if (vals[i] == 0 || vals[i] != vals[i])
        continue;
      // if in badrun,continue.
      bool is_badrun = false;
      for (auto j : brs.bad_run) {
        if (i == j) {
          is_badrun = true;
          break;
        }
      }
      if (is_badrun)
        continue;
      sigma += (vals[i] - mean) * (vals[i] - mean);
    }

    sigma = sqrt(sigma / (n - 1));
    brs.high_bound = mean + nsigma * sigma;
    brs.low_bound = mean - nsigma * sigma;
    // reject bad run bins
    is_all_rejected = true;
    for (int i = 0; i < vals.size(); i++) {
      if (vals[i] == 0 || vals[i] != vals[i])
        continue;
      if (vals[i] > brs.high_bound || vals[i] < brs.low_bound) {
        bool is_badrun = false;
        for (auto j : brs.bad_run) {
          if (i == j) {
            is_badrun = true;
            break;
          }
        }
        if (is_badrun)
          continue;
        brs.bad_run.push_back(i);
        is_all_rejected = false;
      }
    }
  }

  brs.mean = mean;
  brs.sigma = sigma;

  return brs;
}

} // namespace QATool

#endif