#ifndef RCPP_DISTRIBUTIONS_HPP
#define  RCPP_DISTRIBUTIONS_HPP


#include "../../../common/model.hpp"
#include "rcpp_interface_base.hpp"
#include "../../../distributions/normal_lpdf.hpp"
#include "../../../distributions/mvnorm_lpdf.hpp"
#include "../../../distributions/ar1_lpdf.hpp"
#include "../rcpp_interface.hpp"

/****************************************************************
 * Density Components Rcpp interface                                   *
 ***************************************************************/
/**
 * @brief Rcpp interface that serves as the parent class for
 * Rcpp density components interfaces. This type should be inherited and not
 * called from R directly.
 *
 */
class DensityComponentsInterface : public RcppInterfaceBase {
public:
  static uint32_t id_g; /**< static id of the NormalLPDFInterface object */
    uint32_t id;          /**< local id of the NormalLPDFInterface object */
    static std::map<uint32_t, DensityComponentsInterface*> density_components_objects; /**<
 map relating the ID of the UnivariateDensityComponentsInterface to the objects
 objects */
  uint32_t module_id;

DensityComponentsInterface() {
  this->id = DensityComponentsInterface::id_g++;
  DensityComponentsInterface::density_components_objects[this->id] = this;
  RcppInterfaceBase::interface_objects.push_back(this);
}

virtual ~DensityComponentsInterface() {}

virtual uint32_t get_id() = 0;
};

uint32_t DensityComponentsInterface::id_g = 1;
std::map<uint32_t, DensityComponentsInterface*> DensityComponentsInterface::density_components_objects;


class NormalLPDFInterface : public DensityComponentsInterface{

public:
    VariableVector observed_value;
    VariableVector expected_value;
    VariableVector log_sd;
    std::vector<double> log_likelihood_vec;
    std::string input_type;
    std::vector<std::string> key;

    bool simulate_flag = false;
    bool osa_flag;
    
    NormalLPDFInterface() : DensityComponentsInterface(){}
    
    virtual ~NormalLPDFInterface() {}
    virtual uint32_t get_id() { return this->id; }
    
    virtual std::string get_module_name(){
        return "NormalLPDFInterface";
    }
    
    void SetDistributionLinks(std::string input_type, Rcpp::IntegerVector module_id, 
        Rcpp::StringVector module_name, Rcpp::StringVector name){
        this->input_type = input_type;

        std::stringstream ss;
        this->key.resize(module_id.size());
        for(int i=0; i<module_id.size(); i++){
            ss << module_name[i] << "_" << module_id[i] << "_" << name[i];
            this->key[i] = ss.str();
            ss.str("");
        }
    }

    template<typename Type>
    bool prepare_local() {
    std::shared_ptr<Information<Type> > info =
        Information<Type>::getInstance();
     std::shared_ptr<NormalLPDF<Type> > normal =
        std::make_shared<NormalLPDF<Type> >();

        std::shared_ptr<Model<Type> > model = Model<Type>::getInstance();
        normal->input_type = this->input_type;
        normal->id = this->id;
        normal->key.resize(this->key.size());
        for(int i=0; i<key.size(); i++){
            normal->key[i] = this-> key[i];
        }
        normal->simulate_flag = this->simulate_flag;
        if(this->input_type == "data"){
            normal->osa_flag = true;
        } else {
            normal->osa_flag = false;
        }
        normal->observed_value.resize(this->observed_value.size());
        for(size_t i=0; i<this->observed_value.size(); i++){
            normal->observed_value[i] = this->observed_value[i].value;
            if(this->input_type == "data"){
                model->data.push_back(&(normal)->observed_value[i]);
            }
        }
        normal->expected_value.resize(this->expected_value.size());
        for(size_t i=0; i<this->expected_value.size(); i++){
            normal->expected_value[i] = this->expected_value[i].value;
            if(this->expected_value[i].estimable){
                model->parameters.push_back(&(normal)->expected_value[i]);
            }
        }
        normal->log_sd.resize(this->log_sd.size());
        std::stringstream ss;
        ss << this->get_module_name() << "_" << this->id << "_log_sd";
        for(size_t i=0; i<this->log_sd.size(); i++){
            normal->log_sd[i] = this->log_sd[i].value;
            if(this->log_sd[i].estimable){
                model->parameters.push_back(&(normal)->log_sd[i]);
                model->pnames.push_back(ss.str());
            }
        }
        ss.str("");
        
        model->density_components[normal->id] = normal;
        info->density_components[normal->id] = normal;
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

    /**
     * Update the model parameter values and finalize. Sets the parameter values and evaluates the
     * portable model once and transfers values back to the Rcpp interface.
     */
     
    void finalize(Rcpp::NumericVector v) {
        std::shared_ptr< Model<double> > model = Model<double>::getInstance();
        std::shared_ptr<DensityComponentBase<double> > density_components_base = model->density_components[this->id];
        NormalLPDF<double>* norm = (NormalLPDF<double>*) density_components_base.get();    

        for (int i = 0; i < v.size(); i++) {
            (*model->parameters[i]) = v[i];
        }

        double f = model->evaluate();
        /*
        this->observed_value.resize(norm->observed_value.size());
        for(int i=0; i<norm->observed_value.size(); i++){
            this->observed_value[i].value = norm->observed_value[i];
        }
        this->expected_value.resize(norm->expected_value.size());
        for(int i=0; i<norm->expected_value.size(); i++){
            this->expected_value[i].value = norm->expected_value[i];
        }
        this->log_sd.resize(norm->log_sd.size());
        for(int i=0; i<norm->log_sd.size(); i++){
            this->log_sd[i].value = norm->log_sd[i];
        }
        */
        this->log_likelihood_vec.resize(norm->log_likelihood_vec.size());
        for(int i=0; i<norm->log_likelihood_vec.size(); i++){
            this->log_likelihood_vec[i] = norm->log_likelihood_vec[i];
        }


    }

};


class MVNormLPDFInterface : public DensityComponentsInterface{

public:
    VariableVector observed_value;
    VariableVector expected_value;
    VariableVector log_sd;
    VariableVector logit_phi;
    std::vector<double> log_likelihood_vec;
    std::string input_type;
    std::vector<std::string> key;

    bool simulate_flag = false;
    
    MVNormLPDFInterface() : DensityComponentsInterface(){}
    
    virtual ~MVNormLPDFInterface() {}
    virtual uint32_t get_id() { return this->id; }
    
    virtual std::string get_module_name(){
        return "MVNormLPDFInterface";
    }
    
    void SetDistributionLinks(std::string input_type, Rcpp::IntegerVector module_id, 
        Rcpp::StringVector module_name, Rcpp::StringVector name){
        this->input_type = input_type;

        std::stringstream ss;
        this->key.resize(module_id.size());
        for(int i=0; i<module_id.size(); i++){
            ss << module_name[i] << "_" << module_id[i] << "_" << name[i];
            this->key[i] = ss.str();
            ss.str("");
        }
    }

    template<typename Type>
    bool prepare_local() {
    std::shared_ptr<Information<Type> > info =
        Information<Type>::getInstance();
     std::shared_ptr<MVNormLPDF<Type> > mvnorm =
        std::make_shared<MVNormLPDF<Type> >();

        std::shared_ptr<Model<Type> > model = Model<Type>::getInstance();
        mvnorm->input_type = this->input_type;
        mvnorm->id = this->id;
        mvnorm->key.resize(this->key.size());
        for(int i=0; i<key.size(); i++){
            mvnorm->key[i] = this-> key[i];
        }
        mvnorm->simulate_flag = this->simulate_flag;
        if(this->input_type == "data"){
           mvnorm->osa_flag = true;
        } else {
            mvnorm->osa_flag = false;
        }
         //initialize x and mu : how do I differentiate this from the SetX and SetMu functions above? flags?
        mvnorm->observed_value.resize(this->observed_value.size());
        for(size_t i=0; i<this->observed_value.size(); i++){
            mvnorm->observed_value[i] = this->observed_value[i].value;
            if(this->observed_value[i].estimable){
                model->parameters.push_back(&(mvnorm)->observed_value[i]);
            }
        }
        mvnorm->expected_value.resize(this->expected_value.size());
        for(size_t i=0; i<this->expected_value.size(); i++){
            mvnorm->expected_value[i] = this->expected_value[i].value;
            if(this->expected_value[i].estimable){
                model->parameters.push_back(&(mvnorm)->expected_value[i]);
            }
        }
        mvnorm->log_sd.resize(this->log_sd.size());
        std::stringstream ss;
        ss << this->get_module_name() << "_" << this->id << "_log_sd";
        for(size_t i=0; i<this->log_sd.size(); i++){
            mvnorm->log_sd[i] = this->log_sd[i].value;
            if(this->log_sd[i].estimable){
                model->parameters.push_back(&(mvnorm)->log_sd[i]);
                model->pnames.push_back(ss.str());
            }
        }
        ss.str("");

        mvnorm->logit_phi.resize(this->logit_phi.size());
        ss << this->get_module_name() << "_" << this->id << "_logit_phi";
        for(size_t i=0; i<this->logit_phi.size(); i++){
            mvnorm->logit_phi[i] = this->logit_phi[i].value;
            if(this->logit_phi[i].estimable){
                model->parameters.push_back(&(mvnorm)->logit_phi[i]);
                model->pnames.push_back(ss.str());
            }
        }
        ss.str("");
    
        
        model->density_components[mvnorm->id] = mvnorm;
        info->density_components[mvnorm->id] = mvnorm;
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

    /**
     * Update the model parameter values and finalize. Sets the parameter values and evaluates the
     * portable model once and transfers values back to the Rcpp interface.
     */
    void finalize(Rcpp::NumericVector v) {
        
        std::shared_ptr< Model<double> > model = Model<double>::getInstance();
        std::shared_ptr<DensityComponentBase<double> > density_components_base = model->density_components[this->id];
        MVNormLPDF<double>* mvnorm = (MVNormLPDF<double>*) density_components_base.get();  

        for (int i = 0; i < v.size(); i++) {
            (*model->parameters[i]) = v[i];
        }

        double f = model->evaluate();

/*
        for(int i=0; i<mvnorm->observed_value.size(); i++){
            this->observed_value[i].value = mvnorm->observed_value[i];
        }
        for(int i=0; i<mvnorm->expected_value.size(); i++){
            this->expected_value[i].value = mvnorm->expected_value[i];
        }
        for(int i=0; i<mvnorm->Sigma.col(0).size(); i++){
            for(int j=0; j<mvnorm->Sigma.row(0).size(); j++){
                this->Sigma[i,j] = mvnorm->Sigma(i,j);
            }
        }
        */
        this->log_likelihood_vec.resize(mvnorm->log_likelihood_vec.size());
        for(int i=0; i<mvnorm->log_likelihood_vec.size(); i++){
            this->log_likelihood_vec[i] = mvnorm->log_likelihood_vec[i];
        }

    }

};


class AR1LPDFInterface : public DensityComponentsInterface{

public:
    VariableVector observed_value;
    VariableVector log_sd;
    VariableVector logit_rho;
    VariableVector rho;
    std::vector<double> log_likelihood_vec;
    std::string input_type = "re";
    std::vector<std::string> key;

    bool simulate_flag = false;
    
    AR1LPDFInterface() : DensityComponentsInterface(){}
    
    virtual ~AR1LPDFInterface() {}
    virtual uint32_t get_id() { return this->id; }
    
    virtual std::string get_module_name(){
        return "AR1LPDFInterface";
    }
    
    void SetDistributionLinks(std::string input_type, Rcpp::IntegerVector module_id, 
        Rcpp::StringVector module_name, Rcpp::StringVector name){
        this->input_type = input_type;

        std::stringstream ss;
        this->key.resize(module_id.size());
        for(int i=0; i<module_id.size(); i++){
            ss << module_name[i] << "_" << module_id[i] << "_" << name[i];
            this->key[i] = ss.str();
            ss.str("");
        }
    }

    template<typename Type>
    bool prepare_local() {
    std::shared_ptr<Information<Type> > info =
        Information<Type>::getInstance();
     std::shared_ptr<AR1LPDF<Type> > ar1 =
        std::make_shared<AR1LPDF<Type> >();

        std::shared_ptr<Model<Type> > model = Model<Type>::getInstance();
        ar1->input_type = this->input_type;
        ar1->id = this->id;
        ar1->key.resize(this->key.size());
        for(int i=0; i<key.size(); i++){
            ar1->key[i] = this-> key[i];
        }
        ar1->simulate_flag = this->simulate_flag;
        ar1->osa_flag = false;
        
        ar1->observed_value.resize(this->observed_value.size());
        for(size_t i=0; i<this->observed_value.size(); i++){
            ar1->observed_value[i] = this->observed_value[i].value;
            if(this->observed_value[i].is_random_effect){
                model->random_effects.push_back(&(ar1)->observed_value[i]);
            }
        }
        ar1->log_sd.resize(this->log_sd.size());
        std::stringstream ss;
        ss << this->get_module_name() << "_" << this->id << "_log_sd";
        for(size_t i=0; i<this->log_sd.size(); i++){
            ar1->log_sd[i] = this->log_sd[i].value;
            if(this->log_sd[i].estimable){
                model->parameters.push_back(&(ar1)->log_sd[i]);
                model->pnames.push_back(ss.str());
            }
        }
        ss.str("");

        ar1->logit_rho.resize(this->logit_rho.size());
        ss << this->get_module_name() << "_" << this->id << "_logit_rho";
        for(size_t i=0; i<this->logit_rho.size(); i++){
            ar1->logit_rho[i] = this->logit_rho[i].value;
            if(this->logit_rho[i].estimable){
                model->parameters.push_back(&(ar1)->logit_rho[i]);
                model->pnames.push_back(ss.str());
            }
        }
        ss.str("");

        ar1->rho.resize(this->rho.size());
        ss << this->get_module_name() << "_" << this->id << "_rho";
        for(size_t i=0; i<this->rho.size(); i++){
            ar1->rho[i] = this->rho[i].value;
            if(this->rho[i].estimable){
                model->parameters.push_back(&(ar1)->rho[i]);
                model->pnames.push_back(ss.str());
            }
        }
        ss.str("");
    
        
        model->density_components[ar1->id] = ar1;
        info->density_components[ar1->id] = ar1;
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

    /**
     * Update the model parameter values and finalize. Sets the parameter values and evaluates the
     * portable model once and transfers values back to the Rcpp interface.
     */
    void finalize(Rcpp::NumericVector v) {
        
        std::shared_ptr< Model<double> > model = Model<double>::getInstance();
        std::shared_ptr<DensityComponentBase<double> > density_components_base = model->density_components[this->id];
        AR1LPDF<double>* ar1 = (AR1LPDF<double>*) density_components_base.get();  

        for (int i = 0; i < v.size(); i++) {
            (*model->parameters[i]) = v[i];
        }

        double f = model->evaluate();

/*
        for(int i=0; i<mvnorm->observed_value.size(); i++){
            this->observed_value[i].value = mvnorm->observed_value[i];
        }
        for(int i=0; i<mvnorm->expected_value.size(); i++){
            this->expected_value[i].value = mvnorm->expected_value[i];
        }
        for(int i=0; i<mvnorm->Sigma.col(0).size(); i++){
            for(int j=0; j<mvnorm->Sigma.row(0).size(); j++){
                this->Sigma[i,j] = mvnorm->Sigma(i,j);
            }
        }
        */
        this->log_likelihood_vec.resize(ar1->log_likelihood_vec.size());
        for(int i=0; i<ar1->log_likelihood_vec.size(); i++){
            this->log_likelihood_vec[i] = ar1->log_likelihood_vec[i];
        }

    }

};



#endif