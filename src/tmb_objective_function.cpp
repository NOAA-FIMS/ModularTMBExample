/**
 * Simple example showing how to interface TMB with portable
 * models and Rcpp objects.
 */

#include <math.h>
#include <iomanip>
#include <iostream>




#include "../inst/include/interface/rcpp/rcpp_interface.hpp"
#include "../inst/include/interface/interface.hpp"
#include "init.hpp"
#include "../inst/include/common/model.hpp"

/**
 * TMB objective function. Calls the portable von Bertalanffy
 * object.
 * TMB template specifications (see lines 56 -66):
 *  1. double
 *  2. AD<double>
 *  3. AD<AD<double> >
 *  4. AD<AD<AD<double> > >
 */
template<typename Type>
Type objective_function<Type>::operator()(){
    
    //get the singleton instance for type Type
   std::shared_ptr< Model<Type> > model =
    Model<Type>::getInstance();
    
    //get the parameter values
    PARAMETER_VECTOR(p)
    
    //update the parameter values for type Type
    for(int i =0; i < model->parameters.size(); i++){
        *model->parameters[i] = p[i];
    }
    
    //evaluate the model objective function value
    return model->evaluate();
}



