#ifndef RCPP_NORMAL_NLL_HPP
#define  RCPP_NORMAL_NLL_HPP


#include "../../../common/model.hpp"
#include "rcpp_interface_base.hpp"
#include "../rcpp_interface.hpp"
#include "../../../nll/normal_nll.hpp"

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
virtual uint32_t get_id() = 0;
};

uint32_t UnivariateNLLInterface::id_g = 1;
std::map<uint32_t, UnivariateNLLInterface*> UnivariateNLLInterface::univariate_nll_objects;


class NormalNLLInterface : public UnivariateNLLInterface{

public:
    Rcpp::NumericVector x;
    Rcpp::NumericVector mu;
    Rcpp::NumericVector log_sd;
    std::string nll_type;

    bool estimate_x = false;
    bool estimate_mu = false;
    bool estimate_log_sd = false;

    bool simulate_prior_flag = false;
    bool simulate_data_flag = false;
    
    NormalNLLInterface() : UnivariateNLLInterface(){}
    
    virtual ~NormalNLLInterface() {}
    /** @brief returns the id for the logistic selectivity interface */
    virtual uint32_t get_id() { return this->id; }

/*
    void SetX(size_t id, std::string name){
        //how do I set the id?
        normal -> x = assign_variable(id, name);
    }
    void SetMu(size_t id, std::string name){
        //how do I set the id?
        normal -> mu = assign_variable(id, name);
    }
*/
    template<typename Type>
    bool prepare_local() {
        std::shared_ptr<NormalNLL<Type> > normal = 
            std::make_shared<NormalNLL<Type> >();
        std::shared_ptr<Model<Type> > model = Model<Type>::getInstance();
        
        normal->simulate_prior_flag = this->simulate_prior_flag;
        normal->simulate_data_flag = this->simulate_data_flag;
        if(this->nll_type == "data"){
            normal->osa_flag = true;
        } else {
            normal->osa_flag = false;
        }
        //initialize x and mu : how do I differentiate this from the SetX and SetMu functions above? flags?
        normal->x.resize(this->x.size());
        for(size_t i=0; i<this->x.size(); i++){
            normal->x[i] = this->x[i];
            if(this ->estimate_x){
                model->parameters.push_back(&(normal)->x[i]);
            }
        }
        normal->mu.resize(this->mu.size());
        for(size_t i=0; i<this->mu.size(); i++){
            normal->mu[i] = this->mu[i];
            if(this ->estimate_mu){
                model->parameters.push_back(&(normal)->mu[i]);
            }
        }
        normal->log_sd.resize(this->log_sd.size());
        for(size_t i=0; i<this->log_sd.size(); i++){
            normal->log_sd[i] = this->log_sd[i];
            if(this ->estimate_log_sd){
                model->parameters.push_back(&(normal)->log_sd[i]);
            }
        }
        
        model->normal = normal;
        //model->normal_models[this->id] = normal;
        return true;
    }

 /**
     * Prepares the model to work with TMB.
     */
    virtual bool prepare() {

#ifdef TMB_MODEL
        this->prepare_local<TMB_FIMS_REAL_TYPE>();
        this->prepare_local<TMB_FIMS_FIRST_ORDER>();
        this->prepare_local<TMB_FIMS_SECOND_ORDER>();
        this->prepare_local<TMB_FIMS_THIRD_ORDER>();
        
#endif
        return true;

    }

};

#endif