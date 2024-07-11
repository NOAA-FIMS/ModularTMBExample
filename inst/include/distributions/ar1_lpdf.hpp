#ifndef AR1_LPDF_HPP
#define AR1_LPDF_HPP

#include "density_components_base.hpp"
#include "../common/fims_vector.hpp"
#include "../common/def.hpp"

/**
 * Normal Negative Log-Likelihood
 */
template<typename Type>
struct AR1LPDF : public DensityComponentBase<Type> {
    fims::Vector<Type> logit_rho;
    fims::Vector<Type> rho;
    fims::Vector<Type> log_sd;
    fims::Vector<Type> sd;
    bool osa_flag = false; 
    Type log_likelihood = 0.0;
   

    AR1LPDF() : DensityComponentBase<Type>() {

    }

    virtual ~AR1LPDF() {}

    virtual const Type evaluate(){
      Rcout << "AR1 observed size is: " << this->observed_value.size() << std::endl;
      Rcout << "AR1 sd is: " << sd[0] << std::endl;
      Rcout << "AR1 rho is: " << rho[0] << std::endl;
      if(rho[0] != 1){
            rho[0] = 1 / (1 + exp(-logit_rho[0])) * 2 - 1;
        }

        sd[0] = exp(log_sd[0]);
        Rcout << "AR1 sd is: " << sd[0] << std::endl;
        Rcout << "AR1 rho is:" << rho[0] << std::endl;
        log_likelihood = -density::SCALE(density::AR1(rho[0]), sd[0])(this->observed_value);
        this->log_likelihood_vec[0] = log_likelihood;
        
        return(log_likelihood);
    }
};


#endif