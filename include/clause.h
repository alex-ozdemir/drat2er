#ifndef DRAT2ER_CLAUSE_H_
#define DRAT2ER_CLAUSE_H_

#include <vector>
#include <string>
#include <ostream>
#include <initializer_list>

namespace drat2er
{

class Clause
{
  friend void swap(Clause& lhs, Clause& rhs);
  friend std::ostream& operator<< (std::ostream& stream, const Clause& matrix); 
  
 public:
  Clause();
  Clause(std::initializer_list<int> literals);
  Clause(const Clause& other) = default;
  Clause(Clause&& other);
  Clause& operator=(Clause other);
  
  int GetIndex() const;
  void SetIndex(int index);
  std::vector<int>& GetLiterals();
  const std::vector<int>& GetLiteralsConst() const;
  void SetLiterals(const std::vector<int>& literals);
  void AddLiteral(int literal);
  bool IsUnit();
  int GetMaxVariable() const;
  bool ContainsLiteral(int literal) const;
  std::string ToDimacs() const;
  virtual std::string ToLrat() const;
  auto size() const { return literals_.size(); }
  auto empty() const { return literals_.empty(); }
  auto begin() { return literals_.begin(); }
  auto end() { return literals_.end(); }
  auto cbegin() const { return literals_.cbegin(); }
  auto cend() const { return literals_.cend(); }

 protected:
  std::vector<int> literals_;

 private:
  int index_;
};

inline void SwapLiteralToSecondPosition(const int literal, Clause& clause){
  const int other = clause.GetLiterals()[0]^clause.GetLiterals()[1]^literal;
  clause.GetLiterals()[0] = other; 
  clause.GetLiterals()[1] = literal;
}

} // namespace

#endif
