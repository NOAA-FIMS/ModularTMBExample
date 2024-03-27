#ifndef Normal_NLL_HPP
#define Normal_NLL_HPP

#include "nll_base.hpp"
#include "../common/fims_vector.hpp"
#include "../common/def.hpp"

/**
 * Normal Negative Log-Likelihood
 */
template<typename Type>
struct NormalNLL : public NLLBase<Type> {
    //need: dnorm(Type x, Type eta, Type sd)
    //      dnorm(fims::Vector<Type> x, Type eta, Type sd)
    //      dnorm(fims::Vector<Type> x, fims::Vector<Type> eta, Type sd)
    //      dnorm(fims::Vector<Type> x, fims::Vector<Type> eta, fims::Vector<Type> sd)
    typename model_traits<Type>::data_vector x;
    typename model_traits<Type>::data_vector mu;
    typename model_traits<Type>::data_vector log_sd;
    typename model_traits<Type>::data_vector eta;
    typename model_traits<Type>::data_vector sd;
    Type nll = 0.0;
    bool osa_flag;
    bool simulate_prior_flag;
    bool simulate_data_flag;
    //data_indicator<tmbutils::vector<Type> , Type> keep;
    ::objective_function<Type>
      *of;  

    NormalNLL() : NLLBase<Type>() {}

    virtual ~NormalNLL() {}

    virtual const Type evaluate(){
        eta.resize(this->x.size());
        sd.resize(this->x.size());
        for(int i=0; i<x.size(); i++){
            if(mu.size() == 1){
                eta[i] = mu[0];
            } else {
                eta[i] = mu[i];
            }
            //std::fill(eta.begin(), eta.end(), mu[0]);
            //eta.fill(mu[0]);
       
            if(log_sd.size() == 1){
            //std::fill(sd.begin(), sd.end(), exp(log_sd[0]));
                sd[i] = exp(log_sd[0]);
            } else {
                sd[i] = exp(log_sd[i]);
            }
            //sd.fill(exp(log_sd[0]));
        }
        for(int i=0; i<x.size(); i++){
            nll -= dnorm(x[i], eta[i], sd[i], true);
           // nll -= keep[i] * dnorm(x[i], eta[i], sd[i], true);
            if(osa_flag){//data observation type implements osa residuals
                //code for osa cdf method
                //nll -= keep.cdf_lower[i] * log( pnorm(x[i], eta[i], sd[i]) );
                //nll -= keep.cdf_upper[i] * log( 1.0 - pnorm(x[i], eta[i], sd[i]) );
            }
        }
    }
  /* not working yet      
        if(simulate_prior_flag){
            SIMULATE_F(of){
                x = rnorm(eta, sd);
            }
            REPORT_F(x, of);
        }
        if(simulate_data_flag){
            SIMULATE_F(of){
                x = rnorm(eta, sd);
            }
            REPORT_F(x, of);
        }
    } */
/*
    virtual const Type evaluate(){
        Type sd = exp(log_sd);
        for(int i; i < x.size(); i++){
            nll -= dnorm(x(i), eta, sd, true);
            if(osa_flag){//data observation type implements osa residuals
                //code for osa cdf method
                nll -= keep.cdf_lower(i) * log( pnorm(x(i), eta, sd) );
                nll -= keep.cdf_upper(i) * log( 1.0 - pnorm(x((i)), eta, sd) );
            }
        }
            if(simulate_prior_flag){
            SIMULATE_F(of){
                x = rnorm(eta, sd);
            }
            REPORT_F(x, of);
        }
            if(simulate_data_flag){
            SIMULATE_F(of){
                x = rnorm(eta, sd);
            }
            REPORT_F(x, of);
        }
    }

    virtual const Type evaluate(fims::Vector<Type>& x, fims::Vector<Type>& eta, Type& log_sd){
        if(x.size != eta.size){
            Rcout << "Error: Observation and expected value are not the same dimensions"
                << std::endl;
            exit(1);
        }
        Type sd = exp(log_sd);
        for(int i; i < x.size(); i++){
            nll -= dnorm(x(i), eta(i), sd, true);
            if(osa_flag){//data observation type implements osa residuals
                //code for osa cdf method
                nll -= keep.cdf_lower(i) * log( pnorm(x(i), eta(i), sd) );
                nll -= keep.cdf_upper(i) * log( 1.0 - pnorm(x((i)), eta(i), sd) );
            }
        }
            if(simulate_prior_flag){
            SIMULATE_F(of){
                x = rnorm(eta, sd);
            }
            REPORT_F(x, of);
        }
            if(simulate_data_flag){
            SIMULATE_F(of){
                x = rnorm(eta, sd);
            }
            REPORT_F(x, of);
        }
    }

    virtual const Type evaluate(fims::Vector<Type>& x, fims::Vector<Type>& eta, fims::Vector<Type>& log_sd){
        if(x.size != eta.size){
            Rcout << "Error: Observation and expected value are not the same dimensions"
                << std::endl;
            exit(1);
        }
        if(x.size != log_sd.size){
            Rcout << "Error: Observation and standard error vector are not the same dimensions"
                << std::endl;
            exit(1);
        }
        fims::Vector<Type> sd;
        for(int i; i < x.size(); i++){
            sd(i) = exp(log_sd(i));
            nll -= dnorm(x(i), eta(i), sd(i), true);
            if(osa_flag){//data observation type implements osa residuals
                //code for osa cdf method
                nll -= keep.cdf_lower(i) * log( pnorm(x(i), eta(i), sd(i)) );
                nll -= keep.cdf_upper(i) * log( 1.0 - pnorm(x((i)), eta(i), sd(i)) );
            }
        }
            if(simulate_prior_flag){
            SIMULATE_F(of){
                x = rnorm(eta, sd);
            }
            REPORT_F(x, of);
        }
            if(simulate_data_flag){
            SIMULATE_F(of){
                x = rnorm(eta, sd);
            }
            REPORT_F(x, of);
        }
    }*/
};
#endif;
