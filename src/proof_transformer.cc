#include "proof_transformer.h"
#include <string>
#include <vector>
#include <memory>
#include <fstream>
#include <algorithm>
#include <cassert>
#include "rat_clause.h"
#include "rup_clause.h"
#include "clause.h"
#include "deletion.h"

using std::string;
using std::vector;
using std::endl;
using std::make_shared;
using std::ofstream;
using std::ifstream;

namespace drat2er {

ProofTransformer::ProofTransformer(bool print_progress) 
                                     : output_stream_{nullptr},
                                       print_progress_{print_progress},
                                       is_output_lrat_{true},
                                       number_of_lines_processed_{0},
                                       progress_bar_{} { }

void ProofTransformer::Transform(const std::string& input_file, 
                                 std::ostream& output_stream){
  InitProgressBar(input_file);
  output_stream_ = &output_stream;
  LratParser lrat_parser;
  lrat_parser.RegisterObserver(this);
  lrat_parser.ParseFile(input_file);
}

void ProofTransformer::Transform(const string& input_file, 
                                 const string& output_file){
  ofstream output_file_stream(output_file);
  Transform(input_file, output_file_stream);
}

void ProofTransformer::ObserveProperRatAddition(const RatClause& rat){
  HandleProperRatAddition(rat);
  PrintProgress();
}

void ProofTransformer::ObserveRupAddition(const RupClause& rup){
  HandleRupAddition(rup);
  PrintProgress();
}

void ProofTransformer::ObserveDeletion(const Deletion& deletion){
  HandleDeletion(deletion);
  PrintProgress();
}

void ProofTransformer::ObserveComment(const string& comment_line){
  HandleComment(comment_line);
  PrintProgress();
}

void ProofTransformer::ObserveExtension(const Clause& extension){
  HandleExtension(extension);
  PrintProgress();
}

void ProofTransformer::InitProgressBar(const std::string& input_file){
  if(print_progress_){
    progress_bar_.SetOverallNumberOfItems(1);
    PrintProgress(false);
    progress_bar_.SetOverallNumberOfItems(GetNumberOfLines(input_file));
    number_of_lines_processed_ = 0;
  }
}

void ProofTransformer::PrintProgress(bool increment_number_of_lines_processed){
  if(print_progress_){
    if(increment_number_of_lines_processed){
      ++number_of_lines_processed_;
    }
    progress_bar_.PrintProgress(number_of_lines_processed_);
  }
}

std::ostream& ProofTransformer::OutputStream(){
  assert(output_stream_ != nullptr);
  return *output_stream_;
}

void ProofTransformer::HandleProperRatAddition(const RatClause& rat){
  // Does nothing by default. Provide functionality in subclasses.
}

void ProofTransformer::HandleRupAddition(const RupClause& rup){
  // Does nothing by default. Provide functionality in subclasses.
}

void ProofTransformer::HandleDeletion(const Deletion& deletion){
  // Does nothing by default. Provide functionality in subclasses.
}

void ProofTransformer::HandleComment(const string& comment_line){
  // Does nothing by default. Provide functionality in subclasses.
}

void ProofTransformer::HandleExtension(const Clause& extension){
  // Does nothing by default. Provide functionality in subclasses.
}


void ProofTransformer::WriteClauseToOutput(const Clause& clause) {
  if(output_stream_ == nullptr){
    return;
  }
  if(is_output_lrat_){
    *output_stream_ << clause.ToLrat() << endl;
  } else {
    *output_stream_ << clause.ToDimacs() << endl;
  }
}

void ProofTransformer::WriteClausesToOutput(const vector<Clause>& clauses) {
  for(auto& clause : clauses){
    WriteClauseToOutput(clause);
  }
}

int ProofTransformer::GetNumberOfLines(const string& file){
  ifstream input_file(file); 
  if(input_file.is_open()){
    return std::count(std::istreambuf_iterator<char>(input_file), 
                      std::istreambuf_iterator<char>(), '\n');
  }
  return -1;
}

} // namespace
