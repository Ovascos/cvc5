/******************************************************************************
 * Top contributors (to current version):
 *   Aina Niemetz, Gereon Kremer
 *
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2025 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * Wrapper for Kissat SAT Solver.
 *
 * Wrapper for the Kissat SAT solver (for theory of bit-vectors).
 */

#include "prop/kissat.h"

#ifdef CVC5_USE_KISSAT

#include "base/check.h"
#include "util/statistics_registry.h"

namespace cvc5::internal {
namespace prop {

using KissatLit = int32_t;
using KissatVar = int32_t;

// helper functions
namespace {

SatValue toSatValue(int32_t result)
{
  if (result == 10) return SAT_VALUE_TRUE;
  if (result == 20) return SAT_VALUE_FALSE;
  Assert(result == 0);
  return SAT_VALUE_UNKNOWN;
}

/**
 * Helper to convert the value of a literal as returned by Kissat to the
 * corresponding true/false SAT_VALUE.
 * Note: Kissat returns lit/-lit for true/false. Older versions returned 1/-1.
 */
SatValue toSatValueLit(int value)
{
  if (value > 0) return SAT_VALUE_TRUE;
  Assert(value < 0);
  return SAT_VALUE_FALSE;
}

/** Helper to convert SatLiteral to KissatLit. */
KissatLit toKissatLit(const SatLiteral lit)
{
  return lit.isNegated() ? -lit.getSatVariable() : lit.getSatVariable();
}

/** Helper to convert a SatVariable to a KissatVar. */
KissatVar toKissatVar(SatVariable var) { return var; }

}  // namespace

KissatSolver::KissatSolver(StatisticsRegistry& registry,
                           const std::string& name)
    : d_solver(nullptr),
      d_okay(false),
      // Note: Kissat variables start with index 1 rather than 0 since negated
      //       literals are represented as the negation of the index.
      d_nextVarIdx(1),
      d_statistics(registry, name)
{
}

void KissatSolver::initialize()
{
  d_true = newVar(false, true);
  d_false = newVar(false, true);
}

KissatSolver::~KissatSolver()
{
  if (d_solver) { kissat_release(d_solver); }
}

ClauseId KissatSolver::addClause(SatClause& clause, bool removable)
{
  d_clauses.push_back(clause);
  ++d_statistics.d_numClauses;
  return ClauseIdError;
}

ClauseId KissatSolver::addXorClause(SatClause& clause, bool rhs, bool removable)
{
  Unreachable() << "Kissat does not support adding XOR clauses.";
  return 0;
}

SatVariable KissatSolver::newVar(bool isTheoryAtom, bool canErase)
{
  ++d_statistics.d_numVariables;
  return d_nextVarIdx++;
}

SatVariable KissatSolver::trueVar() { return d_true; }

SatVariable KissatSolver::falseVar() { return d_false; }

SatValue KissatSolver::solve()
{
  return solve({});
}

SatValue KissatSolver::solve(long unsigned int&)
{
  Unimplemented() << "Setting limits for Kissat not supported yet";
  return SAT_VALUE_UNKNOWN;
}

SatValue KissatSolver::solve(const std::vector<SatLiteral>& assumptions)
{
  if (d_solver) { kissat_release(d_solver); }
  d_solver = kissat_init();
  d_okay = false;

  kissat_set_option(d_solver, "quiet", 1);
  kissat_reserve(d_solver, d_nextVarIdx);
  kissat_add(d_solver, toKissatVar(d_true));
  kissat_add(d_solver, 0);
  kissat_add(d_solver, -toKissatVar(d_false));
  kissat_add(d_solver, 0);

  for (const SatClause& c : d_clauses)
  {
    for (const SatLiteral l : c)
    {
      kissat_add(d_solver, toKissatLit(l));
    }
    kissat_add(d_solver, 0);
  }

  for (const SatLiteral& l : assumptions)
  {
    kissat_add(d_solver, toKissatLit(l));
    kissat_add(d_solver, 0);
  }

  TimerStat::CodeTimer codeTimer(d_statistics.d_solveTime);
  SatValue res = toSatValue(kissat_solve(d_solver));
  d_okay = (res == SAT_VALUE_TRUE);
  ++d_statistics.d_numSatCalls;
  return res;
}

void KissatSolver::interrupt() { kissat_terminate(d_solver); }

SatValue KissatSolver::value(SatLiteral l)
{
  Assert(d_okay);
  Assert(d_solver);
  return toSatValueLit(kissat_value(d_solver, toKissatLit(l)));
}

SatValue KissatSolver::modelValue(SatLiteral l)
{
  return value(l);
}

uint32_t KissatSolver::getAssertionLevel() const
{
  Unreachable() << "Kissat does not support assertion levels.";
  return -1;
}

bool KissatSolver::ok() const { return d_okay; }

KissatSolver::Statistics::Statistics(StatisticsRegistry& registry,
                                     const std::string& prefix)
    : d_numSatCalls(registry.registerInt(prefix + "Kissat::calls_to_solve")),
      d_numVariables(registry.registerInt(prefix + "Kissat::variables")),
      d_numClauses(registry.registerInt(prefix + "Kissat::clauses")),
      d_solveTime(registry.registerTimer(prefix + "Kissat::solve_time"))
{
}

}  // namespace prop
}  // namespace cvc5::internal

#endif  // CVC5_USE_KISSAT
