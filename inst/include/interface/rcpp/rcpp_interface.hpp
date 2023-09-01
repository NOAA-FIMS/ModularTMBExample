#include "rcpp_objects/rcpp_growth.hpp"



/**
 * Exposes the Variable and vonBertalanffyInterface classes to R.
 */
RCPP_EXPOSED_CLASS(Variable)
RCPP_EXPOSED_CLASS(vonBertalanffyInterface)

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
    .method("prepare", &vonBertalanffyInterface::prepare)
    .method("finalize", &vonBertalanffyInterface::finalize)
    .method("show", &vonBertalanffyInterface::show_)
    .field("k", &vonBertalanffyInterface::k)
    .field("l_inf", &vonBertalanffyInterface::l_inf)
    .field("a_min", &vonBertalanffyInterface::a_min)
    .field("alpha", &vonBertalanffyInterface::alpha)
    .field("beta", &vonBertalanffyInterface::beta)
    .field("ages", &vonBertalanffyInterface::ages)
    .field("data", &vonBertalanffyInterface::data)
    .field("predicted", &vonBertalanffyInterface::predicted);
    Rcpp::function("get_parameter_vector", get_parameter_vector);
    Rcpp::function("clear", clear);
};
