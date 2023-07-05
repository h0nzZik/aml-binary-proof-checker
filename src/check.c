#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

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

struct Formula {
  uint64_t formula_constructor;
  FormulaIdx  formulas[2];
};

_Static_assert(sizeof(struct Formula) % sizeof(uint64_t) == 0, "Formula entries are not aligned");

#define FORMULA_PARAMS_IN_PROOF 4
#define PROOF_PARAMS_IN_PROOF 4
struct Proof {
  uint64_t proof_rule;
  FormulaIdx statement;
  FormulaIdx formula_params[FORMULA_PARAMS_IN_PROOF];
  ProofIdx proof_params[PROOF_PARAMS_IN_PROOF];
};

_Static_assert(sizeof(struct Proof) % sizeof(uint64_t) == 0, "Proof entries are not aligned");

/*bool check_mp(ProofObject & PO, ProofIdx idx) {
  pf = PO.proofs[idx];
  pf1 = pf.proof_params[0]; // phi
  assert (pf1 < idx);
  pf2 = pf.proof_params[1];  // phi --> psi
  assert(pf2 < idx);
  //phi1 = PO.formulas[pf1.statement];
  phi_impl_psi = PO.formulas[pf2.statement];
  assert(phi_impl_psi.connective == Connective::Implication);
  assert (pf1.statement == PO.formulas[phi_impl_psi.formulas[0]]);
}
*/
struct ProofInfo {
  uint64_t n_formulas;
  uint64_t n_proofs;
  struct Formula const *formulas;
  struct Proof const *proofs;
};

bool aml_check_mp(struct ProofInfo const *pi, struct Proof const *curr_proof) {
  struct Proof const *subproof_1 = &pi->proofs[curr_proof->proof_params[0]];
  struct Proof const *subproof_2 = &pi->proofs[curr_proof->proof_params[1]];
  FormulaIdx const phi_idx = subproof_1->statement;
  FormulaIdx const phi_impl_psi_idx = subproof_2->statement;
  struct Formula const * phi_impl_psi = &pi->formulas[phi_impl_psi_idx];
  if (phi_impl_psi->formula_constructor != FormulaConstructor_Impl) {
    return false;
  }
  FormulaIdx const phi2_idx = phi_impl_psi->formulas[0];
  if (phi_idx != phi2_idx) {
    return false;
  }
  return true;
}

bool aml_check(uint64_t const *buffer, size_t len) {
  // read the header
  size_t const header_entries = 2;
  if (len < header_entries)
    return false;

  uint64_t const n_formulas = buffer[0];
  uint64_t const n_proofs = buffer[1];

  uint64_t const expected_formula_table_len = (n_formulas * sizeof(struct Formula) / sizeof(uint64_t));
  uint64_t const expected_len = header_entries + expected_formula_table_len + (n_proofs * sizeof(struct Proof) / sizeof(uint64_t));
  if (len < expected_len) {
    return false;
  }
  struct ProofInfo const pi = (struct ProofInfo const){
    .n_formulas = n_formulas,
    .n_proofs = n_proofs,
    .formulas = (struct Formula const *)(void const *)(&buffer[header_entries]),
    .proofs = (struct Proof const *)(void const *)(&buffer[header_entries + expected_formula_table_len]),
  };


  // Check the proof entries
  for (uint64_t proof_idx = 0; proof_idx < pi.n_proofs; proof_idx++) {
    struct Proof const *curr_proof = &pi.proofs[proof_idx];
    // Avoid cycles
    for (size_t i = 0; i < PROOF_PARAMS_IN_PROOF; i++) {
      if (curr_proof->proof_params[i] >= proof_idx)
        return false;
    }

    switch(curr_proof->proof_rule) {
      case ProofRule_P1: continue; // nothing to check here
      case ProofRule_P2: continue; // nothing to check here
      case ProofRule_P3: continue; // nothing to check here
      case ProofRule_MP:
        if (!aml_check_mp(&pi, curr_proof))
          return false;
        break;
      default: return false;
    }
  }

  return false;
}

bool run_test_1() {
  return false;
}

bool run_tests() {
  if (!run_test_1()) { return false; }
  return false;
}

