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
    DATA_VECTOR(y);
    DATA_VECTOR_INDICATOR(keep,y);
    //get the parameter values
    PARAMETER_VECTOR(p);
    PARAMETER_VECTOR(re);
    
    Rcout << "inside cpp, model data size is: " << model->data.size() << std::endl;
    Rcout << "inside cpp, y size is: " << y.size() << std::endl;
    Rcout << "inside cpp, model parameter size is: " << model->parameters.size() << std::endl;
    Rcout << "inside cpp, p size is: " << p.size() << std::endl;
    Rcout << "inside cpp, model random effects size is: " << model->random_effects.size() << std::endl;
    Rcout << "inside cpp, re size is: " << re.size() << std::endl;;

    //update the data values for type Type
    for(int i =0; i < model->data.size(); i++){
        *model->data[i] = y[i];
    }
    
    //update the parameter values for type Type
    for(int i =0; i < model->parameters.size(); i++){
        *model->parameters[i] = p[i];
    }
    for(int i =0; i < model->random_effects.size(); i++){
      *model->random_effects[i] = re[i];
    }

    model -> of = this;
    model -> keep = keep;

    
    //evaluate the model objective function value
    return model->evaluate();
}



