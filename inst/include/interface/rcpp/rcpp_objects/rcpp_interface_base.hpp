#ifndef RCPP_INTERFACE_BASE_HPP
#define  RCPP_INTERFACE_BASE_HPP

#include "../../../common/def.hpp"



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


/**
 *@brief Base class for all interface objects
 */
class RcppInterfaceBase {
 public:
  /**< FIMS interface object vectors */
  static std::vector<RcppInterfaceBase *> interface_objects;
  
  RcppInterfaceBase(){ }

  virtual ~RcppInterfaceBase() {}

  /** @brief virtual method to inherit to add objects to the TMB model */
  virtual bool prepare() {
    return false;
  }

  
  typename model_traits<double>::data_vector assign_variable(size_t id, std::string name){

      std::map<uint32_t, std::shared_ptr<Population<double> > >
              pop; /**<hash map to link each object to its shared location in memory*/
      typedef typename std::map<
              uint32_t, std::shared_ptr<Population<double> > >::iterator
              pops_it; /**< iterator for population objects>*/
      std::map<uint32_t, std::shared_ptr<VonBertalanffy<double> > >
              vb; /**<hash map to link each object to its shared location in memory*/
      typedef typename std::map<
              uint32_t, std::shared_ptr<VonBertalanffy<double> > >::iterator
              vb_it; /**< iterator for population objects>*/
      pops_it it_pop = pop.find(id);
      vb_it it_vb = vb.find(id);
      std::shared_ptr<Population<double> > ptr_pop = (*it_pop).second;
      std::shared_ptr<VonBertalanffy<double> > ptr_vb = (*it_vb).second;
    
    //Rcout << ptr_pop->growth[0] << std::endl;

      std::unordered_map<std::string, typename model_traits<double>::data_vector> variable_map = {
        {"growth", ptr_pop->length},
        {"k", ptr_vb->k},
        {"l_inf", ptr_vb->l_inf}
      };

      return variable_map[name];
  
  }

};
std::vector<RcppInterfaceBase *>
    RcppInterfaceBase::interface_objects;
std::vector<Variable*> Variable::parameters;
std::vector<Variable*> Variable::estimated_parameters;

#endif