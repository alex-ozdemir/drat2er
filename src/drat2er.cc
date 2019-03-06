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

#include "drat2er.h"
#include <iostream>
#include <fstream>
#include <ostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <algorithm>
#include "formula.h"
#include "formula_parser.h"
#include "rat_eliminator.h"
#include "proof_reverser.h"
#include "unused_clause_eliminator.h"
#include "rup_to_resolution_transformer.h"
#include "proof_step_renamer.h"
#include "lrat_parser.h"
#include "proof_stat_collector.h"
#include "drat_trim_interface.h"
#include "drat2er_options.h"

using std::string;
using std::make_shared;
using std::max;
using std::cout;
using std::cerr;
using std::endl;

namespace drat2er {

using namespace drat2er::options;

// File names for temp files used during the transformation.
const string kTempFileLRAT = "temp.lrat";
const string kTempFileERUP = "temp.erup";
const string kTempFileERUPShrinked = "temp.erups";
const string kTempFileER = "temp_unrenamed.er";

// Takes as input the path to a DIMACS file, parses the file and creates
// a formula object. Returns a pointer to the formula object.
auto ParseFormula(const string& formula_file, VerbosityLevel verbosity)
{
  if (verbosity > QUIET)
  {
    cout << "c drat2er: Parsing input formula." << endl;
  }
  FormulaParser parser {};
  return parser.ParseFormula(formula_file);
}

// Takes a DRAT proof and calls drat-trim to turn the DRAT proof into an
// LRAT proof. The resulting proof is written to the file 'output_proof_file'
void TransformDRATToLRAT(const string& input_formula_file,
                         const string& input_proof_file,
                         const string& output_proof_file,
                         VerbosityLevel verbosity)
{
  if (verbosity > QUIET)
  {
      cout << "c drat2er: Converting DRAT proof to LRAT using drat-trim." << endl;
  }
  if(drat_trim::CheckAndConvertToLRAT(input_formula_file, input_proof_file,
                                      output_proof_file, verbosity)) {
    throw std::runtime_error(
        "Could not parse the input proof with drat-trim.");
  }
}

// Takes as input a formula and an LRAT proof and replaces all proper RAT
// additions in the proof with extensions and RUP additions. In the course of
// this, deletions are also eliminated from the LRAT proof.
void EliminateProperRATs(const Formula& original_formula,
                         const string& input_proof_file,
                         const string& output_proof_file,
                         VerbosityLevel verbosity)
{
  auto formula_copy = make_shared<Formula>(original_formula);
  LratParser lrat_parser {};
  ProofStatCollector stat_collector(formula_copy);
  lrat_parser.RegisterObserver(&stat_collector);
  lrat_parser.ParseFile(input_proof_file);
  auto max_variable = max(original_formula.GetMaxVariable(), 
                          stat_collector.GetMaxVariable());

  RatEliminator rat_eliminator(formula_copy, max_variable,
                               stat_collector.GetMaxInstruction(),
                               verbosity);
  rat_eliminator.Transform(input_proof_file, output_proof_file);
  std::remove(input_proof_file.c_str());
}

// Takes as input a an LRAT proof and deletes all clauses that are not used
// in later proof steps.
void EliminateUnusedClauses(const string& input_proof_file,
                            const string& output_proof_file, VerbosityLevel is_verbose)
{
  const string kTempProofFile = "temp_unused.erup";

  UnusedClauseEliminator eliminator(is_verbose);
  eliminator.Transform(input_proof_file, kTempProofFile);

  ProofReverser reverser(is_verbose);
  reverser.Transform(kTempProofFile, output_proof_file);

  std::remove(kTempProofFile.c_str());
  std::remove(input_proof_file.c_str());
}


// Takes as input an ERUP proof (i.e., a proof containing only extensions
// and RUP additions) and transforms all RUP additions into resolution chains.
void TransformRUPsToResolutions(const Formula& original_formula,
                                const string& input_proof_file,
                                const string& output_proof_file,
                                bool is_output_drat, VerbosityLevel verbosity)
{
  RupToResolutionTransformer
  rup_to_resolution_transformer(make_shared<Formula>(original_formula),
                                is_output_drat, verbosity);
  rup_to_resolution_transformer.Transform(input_proof_file, output_proof_file);
  std::remove(input_proof_file.c_str());
}

// Takes as input a TRACECHECK proof (input_proof_file) whose proof steps are
// not necessarily numbered incrementally and renames the proof steps such
// that the steps in the resulting proof (output_proof_file) are numbered
// incrementally.
void RenameProofStepsIncrementally(const Formula& original_formula,
                                   const string& input_proof_file,
                                   const string& output_proof_file,
                                   VerbosityLevel verbosity)
{
  bool write_to_standard_output = output_proof_file == "";
  ProofStepRenamer proof_step_renamer(original_formula.GetNumberOfClauses() + 1,
                                      write_to_standard_output ? QUIET : verbosity);
  if(write_to_standard_output) {
    WriteToOutputStreamInTRACECHECKFormat(original_formula, cout);
    proof_step_renamer.Transform(input_proof_file, cout);
  } else {
    std::ofstream output_stream(output_proof_file);
    WriteToOutputStreamInTRACECHECKFormat(original_formula, output_stream);
    proof_step_renamer.Transform(input_proof_file, output_stream);
  }
  std::remove(input_proof_file.c_str());
}

// Takes as input a DIMACS file (input_formula_file) and a DRAT proof file
// (input_proof_file) and transforms the DRAT proof into an extended-resolution
// proof. The resulting proof is written to the path 'output_file'.
// The transformation is described in the paper "Extended Resolution Simulates
// DRAT" (IJCAR 2018).
void TransformDRATToExtendedResolution(const string& input_formula_file,
                                       const string& input_proof_file,
                                       const string& output_file,
                                       bool is_output_drat,
                                       VerbosityLevel verbosity,
                                       bool is_compressed)
{
  TransformDRATToLRAT(input_formula_file, input_proof_file,
                      kTempFileLRAT, verbosity);

  auto formula = ParseFormula(input_formula_file, verbosity);

  EliminateProperRATs(formula, kTempFileLRAT, kTempFileERUP, verbosity);

  if(is_compressed){
    EliminateUnusedClauses(kTempFileERUP, kTempFileERUPShrinked, verbosity);
  }

  auto input_proof = is_compressed ? kTempFileERUPShrinked : kTempFileERUP;
  TransformRUPsToResolutions(formula, input_proof, kTempFileER,
                             is_output_drat, verbosity);

  if(is_output_drat){ 
    // We are already done, just rename the existing temp file.
    std::rename(kTempFileER.c_str(), output_file.c_str());
  } else {
    RenameProofStepsIncrementally(formula, kTempFileER, 
                                  output_file, verbosity);
  }
}
}
