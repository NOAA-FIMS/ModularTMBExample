#ifndef NORMAL_LPDF_HPP
#define NORMAL_LPDF_HPP

#include "density_components_base.hpp"
#include "../common/fims_vector.hpp"
#include "../common/def.hpp"

/**
 * Normal Negative Log-Likelihood
 */
template<typename Type>
struct NormalLPDF : public DensityComponentBase<Type> {
    //need: dnorm(Type x, Type eta, Type sd)
    //      dnorm(fims::Vector<Type> x, Type eta, Type sd)
    //      dnorm(fims::Vector<Type> x, fims::Vector<Type> eta, Type sd)
    //      dnorm(fims::Vector<Type> x, fims::Vector<Type> eta, fims::Vector<Type> sd)
    fims::Vector<Type> log_sd;
    fims::Vector<Type> mu;
    fims::Vector<Type> sd;
    bool osa_flag;
    Type log_likelihood = 0.0;
    //data_indicator<tmbutils::vector<Type> , Type> keep;
   

    NormalLPDF() : DensityComponentBase<Type>() {

    }

    virtual ~NormalLPDF() {}

    virtual const Type evaluate(){
        this->mu.resize(this->observed_value.size());
        this->sd.resize(this->observed_value.size());
        for(int i=0; i<this->expected_value.size(); i++){
            if(this->expected_value.size() == 1){
                this->mu[i] = this->expected_value[0];
            } else {
                this->mu[i] = this->expected_value[i];
            }
       
            if(log_sd.size() == 1){
                sd[i] = exp(log_sd[0]);
            } else {
                sd[i] = exp(log_sd[i]);
            }
        }
        this->log_likelihood_vec.resize(this->observed_value.size());
        Type cdf;
        for(int i=0; i<this->observed_value.size(); i++){
          if(osa_flag){
            this->log_likelihood_vec[i] = this->keep[i] * dnorm(this->observed_value[i], mu[i], sd[i], true);
          } else {
            this->log_likelihood_vec[i] = dnorm(this->observed_value[i], mu[i], sd[i], true);
          }
          log_likelihood += this->log_likelihood_vec[i];
          #ifdef TMB_MODEL
            if(this->simulate_flag){
                SIMULATE_F(this->of){
                    this->observed_value[i] = rnorm(mu[i], sd[i]);
                }
                
            }
            
            if(osa_flag){//data observation type implements osa residuals
                //code for osa cdf method
                cdf = pnorm(this->observed_value[i], mu[i], sd[i]);
                this->log_likelihood_vec[i] = this->keep.cdf_lower[i] * log( squeeze(cdf) );
                this->log_likelihood_vec[i] = this->keep.cdf_upper[i] * log( 1.0 - squeeze(cdf) );
            }
          #endif
           
            
        }
        #ifdef TMB_MODEL
            vector<Type> normal_observed_value = this->observed_value;
            REPORT_F(normal_observed_value, this->of);
        #endif
        return(log_likelihood);
    }

};
#endif;