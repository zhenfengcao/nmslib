/**
 * Non-metric Space Library
 *
 * Authors: Bilegsaikhan Naidan (https://github.com/bileg), Leonid Boytsov (http://boytsov.info).
 * With contributions from Lawrence Cayton (http://lcayton.com/) and others.
 *
 * For the complete list of contributors and further details see:
 * https://github.com/searchivarius/NonMetricSpaceLib 
 * 
 * Copyright (c) 2014
 *
 * This code is released under the
 * Apache License Version 2.0 http://www.apache.org/licenses/.
 *
 */
#pragma once

#include <string>
#include <vector>
#include <memory>

#include "index.h"
#include "params.h"
#include "ported_boost_progress.h"

#include "trigen/cSPModifier.h"
#include "trigen/cTriGen.h"

#define METH_VPTREE_TRIGEN     "vptree_trigen"

namespace similarity {

using std::string;
using std::vector;
using std::unique_ptr;

// Vantage point tree

template <typename dist_t> class Space;

template <typename dist_t, typename SearchOracle>
class VPTreeTrigen : public Index<dist_t> {
 public:
  VPTreeTrigen(bool PrintProgress,
         Space<dist_t>& space,
         const ObjectVector& data,
         bool use_random_center = true);

  void CreateIndex(const AnyParams& IndexParams) override;

  ~VPTreeTrigen();

  const std::string StrDesc() const override;

  void Search(RangeQuery<dist_t>* query, IdType) const override;
  void Search(KNNQuery<dist_t>* query, IdType) const override;

  const vector<string>& getQueryTimeParams() const { return QueryTimeParams_; }


  void SetQueryTimeParams(const AnyParams& QueryTimeParams) override {
    AnyParamManager pmgr(QueryTimeParams);
    // Trigen must use the standard metric oracle, 
    // so we don't pass any parameters to the oracle (it will use default, i.e., metric ones).
    //oracle_.SetQueryTimeParams(pmgr); 
    pmgr.GetParamOptional("maxLeavesToVisit", MaxLeavesToVisit_, FAKE_MAX_LEAVES_TO_VISIT);
    LOG(LIB_INFO) << "Set VP-tree query-time parameters:";
    LOG(LIB_INFO) << "maxLeavesToVisit=" << MaxLeavesToVisit_;
    pmgr.CheckUnused();
  }

  virtual bool DuplicateData() const override { return ChunkBucket_; }
 private:
  void BuildTrigen();

  class VPNode {
   public:
    // We want trees to be balanced
    const size_t BalanceConst = 4; 

    VPNode(unsigned level,
           ProgressDisplay* progress_bar,
           const SearchOracle&  oracle,
           const Space<dist_t>& space, const ObjectVector& data,
           size_t max_pivot_select_attempts,
           size_t BucketSize, bool ChunkBucket,
           bool use_random_center);
    ~VPNode();

    template <typename QueryType>
    void GenericSearch(QueryType* query, int& MaxLeavesToVisit) const;

   private:
    void CreateBucket(bool ChunkBucket, const ObjectVector& data, 
                      ProgressDisplay* progress_bar);
    const SearchOracle& oracle_; // The search oracle must be accessed by reference,
                                 // so that VP-tree may be able to change its parameters
    const Object* pivot_;
    /* 
     * Even if dist_t is double, or long double
     * storing the median as the single-precision number (i.e., float)
     * should be good enough.
     */
    float         mediandist_;
    VPNode*       left_child_;
    VPNode*       right_child_;
    ObjectVector* bucket_;
    char*         CacheOptimizedBucket_;

    friend class VPTreeTrigen;
  };

  Space<dist_t>&      space_;
  const ObjectVector& data_;
  bool                PrintProgress_;
  bool                use_random_center_;
  size_t              max_pivot_select_attempts_;

  SearchOracle        oracle_;
  unique_ptr<VPNode>  root_;
  size_t              BucketSize_;
  int                 MaxLeavesToVisit_;
  bool                ChunkBucket_;

  float               TrigenAcc_;
  unsigned            TrigenSampleQty_;
  unsigned            TrigenSampleTripletQty_;

	vector<cSPModifier*>  AllModifiers_;
  unique_ptr<cTriGen>   trigen_;
  cSPModifier*          resultModifier_ = nullptr;

  vector<string>  QueryTimeParams_;

  // disable copy and assign
  DISABLE_COPY_AND_ASSIGN(VPTreeTrigen);
};


}   // namespace similarity

