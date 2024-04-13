#pragma once

#include "Default.h"
#include "Hash.h"
#include "calyx/Calyx.h"


namespace epi {

struct GeneralizedVar {
private:
  GeneralizedVar(var_index_t var_idx, bool is_local = false) :
      idx{var_idx}, is_local{is_local} {

  }

public:
  var_index_t idx;
  bool is_local = false;

  static GeneralizedVar Var(var_index_t var_idx) { return {var_idx}; }
  static GeneralizedVar Local(var_index_t var_idx) { return {var_idx, true}; }
  
  i64 NodeUID() const { return is_local ? -idx : idx; }
  auto operator<=>(const GeneralizedVar& other) const = default;
};

}

namespace std {

template<>
struct hash<epi::GeneralizedVar> {
  size_t operator()(const epi::GeneralizedVar& var) const {
    return var.NodeUID();
  }
};

}