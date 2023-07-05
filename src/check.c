#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#define UNUSED_INDEX 0 // It has to be smaller than any other index

enum FormulaConstructor {
  FormulaConstructor_Bot,
  FormulaConstructor_Impl,
  FormulaConstructor_EVar,
  FormulaConstructor_Symbol
};

enum ProofRule {
  ProofRule_P1,
  ProofRule_P2,
  ProofRule_P3,
  ProofRule_MP
}; 

typedef uint64_t FormulaIdx;
typedef uint64_t ProofIdx;

uint64_t fi_to_idx(FormulaIdx idx) {
  assert(idx >= 1);
  return idx - 1;
}

uint64_t pi_to_idx(ProofIdx idx) {
  assert(idx >= 1);
  return idx - 1;
}

// if formula_constructor == FormulaConstructor_EVar, then formulas[0] is not an index, but denotes a variable.
// if formula_constructor == FormulaConstructor_Symbol, then formulas[0] is not an index, but denotes a symbol.
struct Formula {
  uint64_t formula_constructor;
  FormulaIdx  formulas[2];
};

_Static_assert(sizeof(struct Formula) % sizeof(uint64_t) == 0, "Formula entries are not aligned");

#define FORMULA_PARAMS_IN_PROOF 4
#define PROOF_PARAMS_IN_PROOF 2
struct Proof {
  FormulaIdx statement;
  uint64_t proof_rule;
  FormulaIdx formula_params[FORMULA_PARAMS_IN_PROOF];
  ProofIdx proof_params[PROOF_PARAMS_IN_PROOF];
};

_Static_assert(sizeof(struct Proof) % sizeof(uint64_t) == 0, "Proof entries are not aligned");

struct ProofInfo {
  uint64_t n_formulas;
  uint64_t n_proofs;
  struct Formula const *formulas;
  struct Proof const *proofs;
};

struct Proof const *get_proof(struct ProofInfo const *pi, ProofIdx idx) {
  return &pi->proofs[pi_to_idx(idx)];
}

struct Formula const *get_formula(struct ProofInfo const *pi, FormulaIdx idx) {
  return &pi->formulas[fi_to_idx(idx)];
}

bool aml_check_mp(struct ProofInfo const *pi, struct Proof const *curr_proof) {
  struct Proof const *subproof_1 = get_proof(pi, curr_proof->proof_params[0]);
  struct Proof const *subproof_2 = get_proof(pi, curr_proof->proof_params[1]);
  FormulaIdx const phi_idx = subproof_1->statement;
  FormulaIdx const phi_impl_psi_idx = subproof_2->statement;
  struct Formula const * phi_impl_psi = get_formula(pi, phi_impl_psi_idx);
  if (phi_impl_psi->formula_constructor != FormulaConstructor_Impl) {
    printf("%s:%d", __FILE__, __LINE__);
    return false;
  }
  FormulaIdx const phi2_idx = phi_impl_psi->formulas[0];
  if (phi_idx != phi2_idx) {
    printf("%s:%d", __FILE__, __LINE__);
    return false;
  }
  // TODO check that the conclusion matches
  return true;
}

#define HEADER_ENTRIES 2




bool aml_check(uint64_t const *buffer, size_t len) {
  // read the header
  if (len < HEADER_ENTRIES) {
    printf("%s:%d", __FILE__, __LINE__);
    return false;
  }

  uint64_t const n_formulas = buffer[0];
  uint64_t const n_proofs = buffer[1];

  uint64_t const expected_formula_table_len = (n_formulas * sizeof(struct Formula) / sizeof(uint64_t));
  uint64_t const expected_len = HEADER_ENTRIES + expected_formula_table_len + (n_proofs * sizeof(struct Proof) / sizeof(uint64_t));
  if (len < expected_len) {
    printf("%s:%d\n", __FILE__, __LINE__);
    return false;
  }
  struct ProofInfo const pi = (struct ProofInfo const){
    .n_formulas = n_formulas,
    .n_proofs = n_proofs,
    .formulas = (struct Formula const *)(void const *)(&buffer[HEADER_ENTRIES]),
    .proofs = (struct Proof const *)(void const *)(&buffer[HEADER_ENTRIES + expected_formula_table_len]),
  };


  // Check the proof entries
  for (uint64_t proof_idx = 0; proof_idx < pi.n_proofs; proof_idx++) {
    struct Proof const *curr_proof = &pi.proofs[proof_idx];
    // Avoid cycles
    for (size_t i = 0; i < PROOF_PARAMS_IN_PROOF; i++) {
      if (curr_proof->proof_params[i] != UNUSED_INDEX && curr_proof->proof_params[i] >= proof_idx) {
        printf("%s:%d, i == %ld, references to %ld while at %ld\n", __FILE__, __LINE__, i, curr_proof->proof_params[i], proof_idx);
        return false;
      }
    }

    switch(curr_proof->proof_rule) {
      case ProofRule_P1: continue; // TODO
      case ProofRule_P2: continue; // TODO
      case ProofRule_P3: continue; // TODO
      case ProofRule_MP:
        if (!aml_check_mp(&pi, curr_proof)) {
          printf("%s:%d", __FILE__, __LINE__);
          return false;
        }
        break;
      default:
        printf("%s:%d", __FILE__, __LINE__);
        return false;
    }
  }

  return true;
}


struct SerializedPI {
  uint64_t * data;
  size_t len;
};

struct SerializedPI serialize(struct ProofInfo const *pi) {
  size_t const len = HEADER_ENTRIES
    + (pi->n_formulas * sizeof(struct Formula) / sizeof(uint64_t))
    + (pi->n_proofs * sizeof(struct Proof) / sizeof(uint64_t));
  uint64_t * memory = (uint64_t *) malloc(sizeof(uint64_t) * len);
  if (memory == NULL) {
    fprintf(stderr, "Malloc failed\n");
    exit(1);
  }
  memory[0] = pi->n_formulas;
  memory[1] = pi->n_proofs;
  memcpy(&memory[2], pi->formulas, sizeof(struct Formula) * pi->n_formulas);
  memcpy(&memory[2 + (pi->n_formulas * sizeof(struct Formula) / sizeof(uint64_t))], pi->proofs, sizeof(struct Proof) * pi->n_proofs);
  return (struct SerializedPI) {
    .data = memory,
    .len = len
  };
}

bool run_test_1() {
  struct Formula formulas[3] = {
    // Formula 1: \bot
    (struct Formula) {
      .formula_constructor = FormulaConstructor_Bot,
      .formulas = {UNUSED_INDEX, UNUSED_INDEX} // unused
    },
    // Formula 2: \bot ---> \bot
    (struct Formula) {
      .formula_constructor = FormulaConstructor_Impl,
      .formulas = {1, 1}
    },
    // Formula 3: \bot ---> (\bot ---> bot)
    (struct Formula) {
      .formula_constructor = FormulaConstructor_Impl,
      .formulas = {1, 2}
    }
  };
  struct Proof proofs[1] = {
    // Proof of \bot ---> (\bot ---> bot) using P1
    (struct Proof) {
      .statement = 3,
      .proof_rule = ProofRule_P1,
      .formula_params = { 1, 1, UNUSED_INDEX, UNUSED_INDEX },
      .proof_params = { UNUSED_INDEX, UNUSED_INDEX }
    }
  };
  struct ProofInfo pi = (struct ProofInfo) {
    .n_formulas = 3,
    .n_proofs = 1,
    .formulas = &formulas[0],
    .proofs = &proofs[0]
  };


  struct SerializedPI spi = serialize(&pi);
  bool const result = aml_check(spi.data, spi.len);
  free(spi.data);
  return result;
}

bool run_tests() {
  if (!run_test_1()) { return false; }
  return true;
}

