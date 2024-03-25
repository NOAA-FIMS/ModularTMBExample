#ifndef RCPP_INTERFACE_HPP
#define  RCPP_INTERFACE_HPP



#include "rcpp_objects/rcpp_growth.hpp"
#include "rcpp_objects/rcpp_normal_nll.hpp"
#include "rcpp_objects/rcpp_population.hpp"

bool CreateModel(){
  for (size_t i = 0; i < RcppInterfaceBase::interface_objects.size();
       i++) {
    RcppInterfaceBase::interface_objects[i]->prepare();
  }
  return true;
}
/*
bool SetNLL(uint32 m_id, const char* name, uint32 nll_id){

}
*/

/**
 * Exposes the Variable and vonBertalanffyInterface classes to R.
 */
RCPP_EXPOSED_CLASS(Variable)
RCPP_EXPOSED_CLASS(vonBertalanffyInterface)
RCPP_EXPOSED_CLASS(ObsDataInterface)
RCPP_EXPOSED_CLASS(NormalNLLInterface)
RCPP_EXPOSED_CLASS(PopulationInterface)

/**
 * Returns the initial values for the parameter set
 */
Rcpp::NumericVector get_parameter_vector(){
    Rcpp::NumericVector p;
    
    for(int i =0; i < Variable::parameters.size(); i++){
        if(Variable::parameters[i]->estimable){
            Variable::estimated_parameters.push_back(Variable::parameters[i]);
            p.push_back(Variable::parameters[i]->value);
        }
    }
    return p;
}
/**
 * Clears the vector of independent variables.
 */
void clear(){
    Variable::parameters.clear();
}

/**
 * Define the Rcpp module.
 */
RCPP_MODULE(growth) {
    Rcpp::class_<Variable>("Variable")
    .constructor()
    .field("value", &Variable::value)
    .field("estimable",&Variable::estimable);
    Rcpp::class_<vonBertalanffyInterface>("vonBertalanffy")
    .constructor()
    .method("get_module_id", &vonBertalanffyInterface::get_module_id)
    .field("k", &vonBertalanffyInterface::k)
    .field("l_inf", &vonBertalanffyInterface::l_inf)
    .field("a_min", &vonBertalanffyInterface::a_min)
    .field("alpha", &vonBertalanffyInterface::alpha)
    .field("beta", &vonBertalanffyInterface::beta);
    Rcpp::function("get_parameter_vector", get_parameter_vector);
    Rcpp::function("clear", clear);
    Rcpp::function("CreateModel", CreateModel);
    Rcpp::class_<NormalNLLInterface>("NormalNLL")
    .constructor()
    .field("x", &NormalNLLInterface::x)
    .field("mu", &NormalNLLInterface::mu)
    .field("log_sd", &NormalNLLInterface::log_sd)
    .field("estimate_x", &NormalNLLInterface::estimate_x)
    .field("estimate_mu", &NormalNLLInterface::estimate_mu)
    .field("estimate_log_sd", &NormalNLLInterface::estimate_log_sd)
    .method("SetMu", &NormalNLLInterface::SetX)
    .method("SetMu", &NormalNLLInterface::SetMu);
    Rcpp::class_<PopulationInterface>("Population")
    .constructor()
    .field("ages", &PopulationInterface::ages)
    .method("get_id", &PopulationInterface::get_id);

};

#endif