/*********************                                                        */
/*! \file infer_info.h
 ** \verbatim
 ** Top contributors (to current version):
 **   Andrew Reynolds
 ** This file is part of the CVC4 project.
 ** Copyright (c) 2009-2019 by the authors listed in the file AUTHORS
 ** in the top-level source directory) and their institutional affiliations.
 ** All rights reserved.  See the file COPYING in the top-level source
 ** directory for licensing information.\endverbatim
 **
 ** \brief Inference information utility
 **/

#include "cvc4_private.h"

#ifndef CVC4__THEORY__STRINGS__INFER_INFO_H
#define CVC4__THEORY__STRINGS__INFER_INFO_H

#include <map>
#include <vector>

#include "expr/node.h"
#include "util/safe_print.h"

namespace CVC4 {
namespace theory {
namespace strings {

/** Types of inferences used in the procedure
 *
 * These are variants of the inference rules in Figures 3-5 of Liang et al.
 * "A DPLL(T) Solver for a Theory of Strings and Regular Expressions", CAV 2014.
 *
 * Note: The order in this enum matters in certain cases (e.g. inferences
 * related to normal forms), inferences that come first are generally
 * preferred.
 */
enum class Inference : uint32_t
{
  //-------------------------------------- base solver
  // initial normalize singular
  //   x1 = "" ^ ... ^ x_{i-1} = "" ^ x_{i+1} = "" ^ ... ^ xn = "" =>
  //   x1 ++ ... ++ xn = xi
  I_NORM_S,
  // initial constant merge
  //   explain_constant(x, c) => x = c
  // Above, explain_constant(x,c) is a basic explanation of why x must be equal
  // to string constant c, which is computed by taking arguments of
  // concatentation terms that are entailed to be constants. For example:
  //  ( y = "AB" ^ z = "C" ) => y ++ z = "ABC"
  I_CONST_MERGE,
  // initial constant conflict
  //    ( explain_constant(x, c1) ^ explain_constant(x, c2) ^ x = y) => false
  // where c1 != c2.
  I_CONST_CONFLICT,
  // initial normalize
  // Given two concatenation terms, this is applied when we find that they are
  // equal after e.g. removing strings that are currently empty. For example:
  //   y = "" ^ z = "" => x ++ y = z ++ x
  I_NORM,
  // The cardinality inference for strings, see Liang et al CAV 2014.
  CARDINALITY,
  //-------------------------------------- end base solver
  //-------------------------------------- core solver
  // Given two normal forms, infers that the remainder one of them has to be
  // empty. For example:
  //    If x1 ++ x2 = y1 and x1 = y1, then x2 = ""
  N_ENDPOINT_EMP,
  // Given two normal forms, infers that two components have to be the same if
  // they have the same length. For example:
  //   If x1 ++ x2 = x3 ++ x4 and len(x1) = len(x3) then x1 = x3
  N_UNIFY,
  // Given two normal forms, infers that the endpoints have to be the same. For
  // example:
  //   If x1 ++ x2 = x3 ++ x4 ++ x5 and x1 = x3 then x2 = x4 ++ x5
  N_ENDPOINT_EQ,
  // Given two normal forms with constant endpoints, infers a conflict if the
  // endpoints do not agree. For example:
  //   If "abc" ++ ... = "bc" ++ ... then conflict
  N_CONST,
  // infer empty, for example:
  //     (~) x = ""
  // This is inferred when we encounter an x such that x = "" rewrites to a
  // constant. This inference is used for instance when we otherwise would have
  // split on the emptiness of x but the rewriter tells us the emptiness of x
  // can be inferred.
  INFER_EMP,
  // string split constant propagation, for example:
  //     x = y, x = "abc", y = y1 ++ "b" ++ y2
  //       implies y1 = "a" ++ y1'
  SSPLIT_CST_PROP,
  // string split variable propagation, for example:
  //     x = y, x = x1 ++ x2, y = y1 ++ y2, len( x1 ) >= len( y1 )
  //       implies x1 = y1 ++ x1'
  // This is inspired by Zheng et al CAV 2015.
  SSPLIT_VAR_PROP,
  // length split, for example:
  //     len( x1 ) = len( y1 ) V len( x1 ) != len( y1 )
  // This is inferred when e.g. x = y, x = x1 ++ x2, y = y1 ++ y2.
  LEN_SPLIT,
  // length split empty, for example:
  //     z = "" V z != ""
  // This is inferred when, e.g. x = y, x = z ++ x1, y = y1 ++ z
  LEN_SPLIT_EMP,
  // string split constant
  //    x = y, x = "c" ++ x2, y = y1 ++ y2, y1 != ""
  //      implies y1 = "c" ++ y1'
  // This is a special case of F-Split in Figure 5 of Liang et al CAV 2014.
  SSPLIT_CST,
  // string split variable, for example:
  //    x = y, x = x1 ++ x2, y = y1 ++ y2
  //      implies x1 = y1 ++ x1' V y1 = x1 ++ y1'
  // This is rule F-Split in Figure 5 of Liang et al CAV 2014.
  SSPLIT_VAR,
  // flat form loop, for example:
  //    x = y, x = x1 ++ z, y = z ++ y2
  //      implies z = u2 ++ u1, u in ( u1 ++ u2 )*, x1 = u2 ++ u, y2 = u ++ u1
  //        for fresh u, u1, u2.
  // This is the rule F-Loop from Figure 5 of Liang et al CAV 2014.
  FLOOP,
  //-------------------------------------- end core solver
  //-------------------------------------- regexp solver
  // regular expression normal form conflict
  //   ( x in R ^ x = y ^ rewrite((str.in_re y R)) = false ) => false
  // where y is the normal form computed for x.
  RE_NF_CONFLICT,
  // regular expression unfolding
  // This is a general class of inferences of the form:
  //   (x in R) => F
  // where F is formula expressing the next step of checking whether x is in
  // R.  For example:
  //   (x in (R)*) =>
  //   x = "" V x in R V ( x = x1 ++ x2 ++ x3 ^ x1 in R ^ x2 in (R)* ^ x3 in R)
  RE_UNFOLD_POS,
  // Same as above, for negative memberships
  RE_UNFOLD_NEG,
  // intersection inclusion conflict
  //   (x in R1 ^ ~ x in R2) => false  where [[includes(R2,R1)]]
  // Where includes(R2,R1) is a heuristic check for whether R2 includes R1.
  RE_INTER_INCLUDE,
  // intersection conflict, using regexp intersection computation
  //   (x in R1 ^ x in R2) => false   where [[intersect(R1, R2) = empty]]
  RE_INTER_CONF,
  // intersection inference
  //   (x in R1 ^ y in R2 ^ x = y) => (x in re.inter(R1,R2))
  RE_INTER_INFER,
  // regular expression delta
  //   (x = "" ^ x in R) => C
  // where "" in R holds if and only if C holds.
  RE_DELTA,
  // regular expression delta conflict
  //   (x = "" ^ x in R) => false
  // where R does not accept the empty string.
  RE_DELTA_CONF,
  // regular expression derive ???
  RE_DERIVE,
  //-------------------------------------- end regexp solver
  //-------------------------------------- extended function solver
  // All extended function inferences from context-dependent rewriting produced
  // by constant substitutions. See Reynolds et al CAV 2017. These are
  // inferences of the form:
  //   X = Y => f(X) = t   when   rewrite( f(Y) ) = t
  // where X = Y is a vector of equalities, where some of Y may be constants.
  EXTF,
  // Same as above, for normal form substitutions.
  EXTF_N,
  // contain transitive
  //   ( str.contains( s, t ) ^ ~contains( s, r ) ) => ~contains( t, r ).
  CTN_TRANS,
  // contain decompose
  //  str.contains( x, str.++( y1, ..., yn ) ) => str.contains( x, yi ) or
  //  ~str.contains( str.++( x1, ..., xn ), y ) => ~str.contains( xi, y )
  CTN_DECOMPOSE,
  // contain neg equal
  //   ( len( x ) = len( s ) ^ ~contains( x, s ) ) => x != s
  CTN_NEG_EQUAL,
  // contain positive
  //   str.contains( x, y ) => x = w1 ++ y ++ w2
  // where w1 and w2 are skolem variables.
  CTN_POS,
  // All reduction inferences of the form:
  //   f(x1, .., xn) = y ^ P(x1, ..., xn, y)
  // where f is an extended function, y is the purification variable for
  // f(x1, .., xn) and P is the reduction predicate for f
  // (see theory_strings_preprocess).
  REDUCTION,
  //-------------------------------------- end extended function solver
  NONE,
};

/**
 * Converts an inference to a string. Note: This function is also used in
 * `safe_print()`. Changing this functions name or signature will result in
 * `safe_print()` printing "<unsupported>" instead of the proper strings for
 * the enum values.
 *
 * @param i The inference
 * @return The name of the inference
 */
const char* toString(Inference i);

/**
 * Writes an inference name to a stream.
 *
 * @param out The stream to write to
 * @param i The inference to write to the stream
 * @return The stream
 */
std::ostream& operator<<(std::ostream& out, Inference i);

/**
 * Length status, used for indicating the length constraints for Skolems
 * introduced by the theory of strings.
 */
enum LengthStatus
{
  // The length of the Skolem should not be constrained. This should be
  // used for Skolems whose length is already implied.
  LENGTH_IGNORE,
  // The length of the Skolem is not specified, and should be split on.
  LENGTH_SPLIT,
  // The length of the Skolem is exactly one.
  LENGTH_ONE,
  // The length of the Skolem is greater than or equal to one.
  LENGTH_GEQ_ONE
};

/**
 * This data structure encapsulates an inference for strings. This includes
 * the form of the inference, as well as the side effects it generates.
 */
class InferInfo
{
 public:
  InferInfo();
  /**
   * The identifier for the inference, indicating the kind of reasoning used
   * for this conclusion.
   */
  Inference d_id;
  /** The conclusion of the inference */
  Node d_conc;
  /**
   * The antecedant(s) of the inference, interpreted conjunctively. These are
   * literals that currently hold in the equality engine.
   */
  std::vector<Node> d_ant;
  /**
   * The "new literal" antecedant(s) of the inference, interpreted
   * conjunctively. These are literals that were needed to show the conclusion
   * but do not currently hold in the equality engine.
   */
  std::vector<Node> d_antn;
  /**
   * A list of new skolems introduced as a result of this inference. They
   * are mapped to by a length status, indicating the length constraint that
   * can be assumed for them.
   */
  std::map<LengthStatus, std::vector<Node> > d_new_skolem;
  /**
   * The pending phase requirements, see InferenceManager::sendPhaseRequirement.
   */
  std::map<Node, bool> d_pending_phase;
  /**
   * The index in the normal forms under which this inference is addressing.
   * For example, if the inference is inferring x = y from |x|=|y| and
   *   w ++ x ++ ... = w ++ y ++ ...
   * then d_index is 1, since x and y are at index 1 in these concat terms.
   */
  unsigned d_index;
  /**
   * The normal form pair that is cached as a result of this inference.
   */
  Node d_nf_pair[2];
  /** for debugging
   *
   * The base pair of strings d_i/d_j that led to the inference, and whether
   * (d_rev) we were processing the normal forms of these strings in reverse
   * direction.
   */
  Node d_i;
  Node d_j;
  bool d_rev;
};

}  // namespace strings
}  // namespace theory
}  // namespace CVC4

#endif /* CVC4__THEORY__STRINGS__INFER_INFO_H */
