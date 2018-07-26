#ifndef DRAT2ER_SIMPLE_PARSER_H_
#define DRAT2ER_SIMPLE_PARSER H_

#include <string>
#include <memory>
#include <fstream>
#include <vector>
#include <unordered_set>
#include "lrat_parser.h"

namespace drat2er
{

class Formula;
//class Clause;
//class RatClause;

class SimpleParser : public LratParserObserver
{
 public:
  SimpleParser(std::shared_ptr<Formula> formula);
  virtual void HandleDeletion(const Deletion& deletion) override;
  virtual void HandleProperRatAddition(const RatClause& rat) override;
  virtual void HandleRupAddition(const RupClause& rup) override;
  virtual void HandleComment(const std::string& comment_line) override;

  std::unordered_set<int> deleted_clauses_;
  std::unordered_set<int> added_clauses_;
 private:
  std::shared_ptr<Formula> formula_;
};

}// namespace

#endif