#ifndef RCPP_NORMAL_NLL_HPP
#define  RCPP_NORMAL_NLL_HPP


#include "../../../common/model.hpp"
#include "rcpp_interface_base.hpp"
#include "../rcpp_interface.hpp"
#include "../../../common/nll/normal_nll.hpp"

/****************************************************************
 * Univariate NLL Rcpp interface                                   *
 ***************************************************************/
/**
 * @brief Rcpp interface that serves as the parent class for
 * Rcpp nll interfaces. This type should be inherited and not
 * called from R directly.
 *
 */
class UnivariateNLLInterface : public RcppInterfaceBase {
public:
  static uint32_t id_g; /**< static id of the NormalNLLInterface object */
    uint32_t id;          /**< local id of the NormalNLLInterface object */
    static std::map<uint32_t, UnivariateNLLInterface*> univariate_nll_objects; /**<
 map relating the ID of the UnivariateNLLInterface to the objects
 objects */

UnivariateNLLInterface() {
  this->id = UnivariateNLLInterface::id_g++;
  UnivariateNLLInterface::univariate_nll_objects[this->id] = this;
  RcppInterfaceBase::interface_objects.push_back(this);
}

virtual ~UnivariateNLLInterface() {}
};

uint32_t UnivariateNLLInterface::id_g = 1;
std::map<uint32_t, UnivariateNLLInterface*> UnivariateNLLInterface::univariate_nll_objects;


class NormalNLLInterface : public UnivariateNLLInterface{

public:
    Rcpp::NumericVector x;
    Rcpp::NumericVector mu;
    Rcpp::NumericVector log_sd;

    bool estimate_x = false;
    bool estimate_mu = false;
    bool estimate_log_sd = false;
    
    NormalNLLInterface() : UnivariateNLLInterface(){}
    
    virtual ~NormalNLLInterface() {}

//Here mu is a vector, but does it need to be a pointer for this to work??
    void SetX(size_t id, std::string name){
        this -> x = assign_variable(id, name)
    }
    void SetMu(size_t id, std::string name){
        this -> mu = assign_variable(id, name)
    }

    template<typename Type>
    bool prepare_local() {

        std::shared_ptr<Model<Type> > model = Model<Type>::getInstance();
        std::shared_ptr< NormalNLL<Type> > normal = 
            std::make_shared<NormalNLL<Type> >();
        


        //initialize x and mu
        for(size_t i=0; i<this->x.size(); i++){
            normal->x[i] = this->x[i];
        }
        for(size_t i=0; i<this->mu.size(); i++){
            normal->mu[i] = this->mu[i];
        }
        for(size_t i=0; i<this->log_sd.size(); i++){
            normal->log_sd[i] = this->log_sd[i];
        }
        
        if (estimate_x) {
            for(int i=0; i<x.size(); i++){
                model->parameters.push_back(&(normal)->x[i]);
            }
        }

        if (estimate_mu) {
            for(int i=0; i<mu.size(); i++){
                model->parameters.push_back(&(normal)->mu[i]);
            }
        }
        
        if (estimate_log_sd) {
            for(int i=0; i<log_sd.size(); i++){
                model->parameters.push_back(&(normal)->log_sd[i]);
            }
        }

        model->normal = normal;
        return true;
    }
};

#endif