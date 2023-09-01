#define RCPP_NO_SUGAR
#include <Rcpp.h>

/**
 * Rcpp representation of a variable
 * interface between R and cpp.
 */
class Variable{
public:
    static std::vector<Variable*> parameters;
    static std::vector<Variable*> estimated_parameters;
    bool estimable = FALSE;
    double value = 0;
    
    Variable(){
        Variable::parameters.push_back(this);
    }
    
};

std::vector<Variable*> Variable::parameters;
std::vector<Variable*> Variable::estimated_parameters;
