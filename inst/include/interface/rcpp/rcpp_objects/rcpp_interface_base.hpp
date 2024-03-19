#ifndef RCPP_INTERFACE_BASE_HPP
#define  RCPP_INTERFACE_BASE_HPP

#include "../../../common/def.hpp"



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


/**
 *@brief Base class for all interface objects
 */
class RcppInterfaceBase {
 public:
  /**< FIMS interface object vectors */
  static std::vector<RcppInterfaceBase *> interface_objects;

  /** @brief virtual method to inherit to add objects to the TMB model */
  virtual bool prepare() {
    return false;
  }
};
std::vector<RcppInterfaceBase *>
    RcppInterfaceBase::interface_objects;
std::vector<Variable*> Variable::parameters;
std::vector<Variable*> Variable::estimated_parameters;

#endif