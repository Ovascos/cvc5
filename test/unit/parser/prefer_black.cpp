/******************************************************************************
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2026 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * Black box testing of the (prefer <term>) command.
 */

#include <cvc5/cvc5.h>

#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "parser/commands.h"
#include "parser/parser.h"
#include "parser/sym_manager.h"
#include "test.h"

namespace cvc5::internal {
namespace test {

class TestParserBlackPrefer : public TestInternal
{
 protected:
  /**
   * Parse and invoke all commands of the given input, and return the preferred
   * terms that were recorded by the symbol manager.
   */
  std::vector<cvc5::Term> getPreferTerms(const std::string& input)
  {
    cvc5::Solver solver(d_tm);
    cvc5::parser::SymManager sm(d_tm);
    std::unique_ptr<cvc5::parser::Parser> p(cvc5::parser::Parser::mkParser(
        modes::InputLanguage::SMT_LIB_2_6, &solver, &sm));
    p->setStringInput(input, "prefer_black");
    std::stringstream out;
    while (true)
    {
      std::unique_ptr<cvc5::parser::Cmd> cmd = p->nextCommand();
      if (cmd == nullptr)
      {
        break;
      }
      cmd->invoke(&solver, &sm, out);
    }
    d_lastOutput = out.str();
    return sm.getPreferTerms();
  }

  cvc5::TermManager d_tm;
  /** The output produced by the last call to getPreferTerms */
  std::string d_lastOutput;
};

TEST_F(TestParserBlackPrefer, order_of_occurrence)
{
  std::vector<cvc5::Term> prefers = getPreferTerms(
      "(set-logic QF_UF)\n"
      "(declare-fun a () Bool)\n"
      "(declare-fun b () Bool)\n"
      "(declare-fun c () Bool)\n"
      "(assert (or a (not b)))\n"
      "(assert (=> c a))\n"
      "(prefer (not b))\n"
      "(prefer c)\n"
      "(prefer a)\n");
  ASSERT_EQ(prefers.size(), 3);
  ASSERT_EQ(prefers[0].toString(), "(not b)");
  ASSERT_EQ(prefers[1].toString(), "c");
  ASSERT_EQ(prefers[2].toString(), "a");
  ASSERT_EQ(d_lastOutput, "");
}

TEST_F(TestParserBlackPrefer, not_in_any_assert)
{
  // b occurs in an assertion, but (not b) does not, so the second prefer is
  // reported as an error and dropped, while the rest of the input is still
  // processed.
  std::vector<cvc5::Term> prefers = getPreferTerms(
      "(set-logic QF_UF)\n"
      "(declare-fun a () Bool)\n"
      "(declare-fun b () Bool)\n"
      "(assert (or a b))\n"
      "(prefer a)\n"
      "(prefer (not b))\n"
      "(prefer b)\n");
  ASSERT_EQ(prefers.size(), 2);
  ASSERT_EQ(prefers[0].toString(), "a");
  ASSERT_EQ(prefers[1].toString(), "b");
  ASSERT_NE(d_lastOutput.find("does not occur in any assertion"),
            std::string::npos);
}

TEST_F(TestParserBlackPrefer, before_any_assert)
{
  std::vector<cvc5::Term> prefers = getPreferTerms(
      "(set-logic QF_UF)\n"
      "(declare-fun a () Bool)\n"
      "(prefer a)\n"
      "(assert a)\n");
  ASSERT_TRUE(prefers.empty());
  ASSERT_NE(d_lastOutput.find("does not occur in any assertion"),
            std::string::npos);
}

TEST_F(TestParserBlackPrefer, not_a_variable)
{
  // (> x 0) is a Boolean term but not a variable, and x is a variable but not
  // Boolean, so both are reported as errors and dropped.
  std::vector<cvc5::Term> prefers = getPreferTerms(
      "(set-logic QF_LIA)\n"
      "(declare-fun a () Bool)\n"
      "(declare-fun x () Int)\n"
      "(assert (or a (> x 0)))\n"
      "(prefer a)\n"
      "(prefer (> x 0))\n"
      "(prefer x)\n");
  ASSERT_EQ(prefers.size(), 1);
  ASSERT_EQ(prefers[0].toString(), "a");
  ASSERT_NE(d_lastOutput.find("(> x 0) is not a Boolean variable"),
            std::string::npos);
  ASSERT_NE(d_lastOutput.find("x is not a Boolean variable"),
            std::string::npos);
}

TEST_F(TestParserBlackPrefer, deep_subterm)
{
  // c only occurs nested deep inside an assertion
  std::vector<cvc5::Term> prefers = getPreferTerms(
      "(set-logic QF_UF)\n"
      "(declare-fun a () Bool)\n"
      "(declare-fun b () Bool)\n"
      "(declare-fun c () Bool)\n"
      "(assert (or a (and b (= b c))))\n"
      "(prefer c)\n");
  ASSERT_EQ(prefers.size(), 1);
  ASSERT_EQ(prefers[0].toString(), "c");
  ASSERT_EQ(d_lastOutput, "");
}

}  // namespace test
}  // namespace cvc5::internal
