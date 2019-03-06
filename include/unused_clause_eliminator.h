// MIT License
//
// Copyright (c) 2018 Benjamin Kiesl
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to 
// deal in the Software without restriction, including without limitation the 
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or 
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in 
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS 
// IN THE SOFTWARE.

#ifndef DRAT2ER_UNUSED_CLAUSE_ELIMINATOR_H_
#define DRAT2ER_UNUSED_CLAUSE_ELIMINATOR_H_

#include "proof_transformer.h"
#include <unordered_set>

namespace drat2er
{

// Takes an LRAT proof and removes all clauses that are not used in later
// proof steps
class UnusedClauseEliminator : public ProofTransformer
{

 public:
  UnusedClauseEliminator(const options::VerbosityLevel print_progress = options::NORMAL);

 protected:
  virtual void HandleProperRatAddition(const RatClause& rat) override;
  virtual void HandleRupAddition(const RupClause& rup) override;
  virtual void HandleDeletion(const Deletion& deletion) override;
  virtual void HandleExtension(const Clause& extension) override;

 private:
  std::unordered_set<int> used_clauses_;

};

} // namespace drat2er

#endif
