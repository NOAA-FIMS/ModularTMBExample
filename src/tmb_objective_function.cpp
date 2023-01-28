/**
 * Simple example showing how to interface TMB with portable
 * models and Rcpp objects.
 */

#include <math.h>
#include <iomanip>
#include <iostream>
#include "von_bertalanffy.hpp"
#include "init.hpp"

#define RCPP_NO_SUGAR
#include <Rcpp.h>


/**
 * Rcpp representation of a variable
 * interface between R and cpp.
 */
class Variable{
public:
    static std::vector<Variable*> parameters;
    static std::vector<Variable*> estimated_parameters;
    bool estimable = FALSE;
    double value = 0;
    
    Variable(){
        Variable::parameters.push_back(this);
    }
    
};

std::vector<Variable*> Variable::parameters;
std::vector<Variable*> Variable::estimated_parameters;

class vonBertalanffyInterface{
public:
    Rcpp::NumericVector obs;
    Rcpp::NumericVector ages;
    Rcpp::NumericVector predicted;
    Rcpp::IntegerVector fish;
    int nfish;
    Variable a_min;
    Rcpp::NumericVector log_l_inf;
    Variable log_l_inf_sigma;
    Variable log_l_inf_mean; 
  Rcpp::NumericVector log_k;
  Variable log_k_sigma;
    Variable log_k_mean; 

    static vonBertalanffyInterface* instance;
    vonBertalanffyInterface(){
        instance = this;
    }
    /**
     * Prepares the model to work with TMB.
     */
    void prepare(){
        prepare_template<double>();
    }
    template<class Type>
    void prepare_template(){
        
        // if(this->obs.size() != this->ages.size()){
        //     std::cout<<"Error: ages vector length not equal to obs vector length, abort\n";
        //     return;
        // }

        if(this->obs.size() != this->ages.size()){
            Rcpp::stop("ages vector length not equal to obs vector length");
        }
        
        VonBertalanffyModel<Type>* model =
        VonBertalanffyModel<Type>::getInstance();
        
        model->clear();
        
        model->predicted.resize(this->obs.size());
        
        model->ages.resize(this->ages.size());
        model->obs.resize(this->obs.size());
        
        for(int i =0; i < this->obs.size(); i++){
            model->ages[i] = this->ages[i];
            model->obs[i] = this->obs[i];
        }
        
        //initialize k
        model->log_k_mean = this->log_k_mean.value;
	model->log_k_sigma = this->log_k_sigma.value;

        //initialize l_inf
        model->log_l_inf_mean = this->log_l_inf_mean.value;
	model->log_l_inf_sigma = this->log_l_inf_sigma.value;
	// model->log_l_inf = this->log_l_inf;
	// model->log_k = this->log_k;
	
	for(int i=0; i<nfish;i++){
	  model->log_l_inf[i] = this->log_l_inf[i];
	  model->log_k[i] = this->log_k[i];
	}
        
        //initialize a_min
        model->a_min = this->a_min.value;
        
        
        if(this->log_k_mean.estimable){
            model->parameters.push_back(&model->log_k_mean);
        }
        if(this->log_k_sigma.estimable){
            model->parameters.push_back(&model->log_k_sigma);
        }
        if(this->log_l_inf_mean.estimable){
            model->parameters.push_back(&model->log_l_inf_mean);
        }
        if(this->log_l_inf_sigma.estimable){
            model->parameters.push_back(&model->log_l_inf_sigma);
        }
	for(int i=0; i<nfish;i++){
	  // if(this->log_k[i].estimable){
            model->parameters.push_back(&model->log_k[i]);
	    // }
	    // if(this->log_l_inf[i].estimable){
            model->parameters.push_back(&model->log_l_inf[i]);
	    //  }
	}
        if(this->a_min.estimable){
            model->parameters.push_back(&model->a_min);
        }
        
    }
    
    /**
     * Update the model parameter values and finalize. Sets the parameter values and evaluates the
     * portable model once and transfers values back to the Rcpp interface.
     */
    void finalize(Rcpp::NumericVector v){
        VonBertalanffyModel<double>* model =
        VonBertalanffyModel<double>::getInstance();
      
        for(int i =0; i < v.size(); i++){
            (*model->parameters[i]) = v[i];
        }
        
        double f = model->evaluate();
        
        this->log_k_mean.value = model->log_k_mean;
	this->log_k_sigma.value = model->log_k_sigma;
	// for(int i=0; i<nfish;i++){
	//   this->log_k[i] = model->log_k[i];
	//   this->log_l_inf[i] = model->log_l_inf[i];
	// }
        this->log_l_inf_mean.value = model->log_l_inf_mean;
	this->log_l_inf_sigma.value = model->log_l_inf_sigma;
        this->a_min.value = model->a_min;
        this->predicted = Rcpp::NumericVector(model->predicted.size());
        for(int i =0; i < model->predicted.size(); i++){
            this->predicted[i] = model->predicted[i];
        }

    }
    
    /**
     * Print model values.
     */
    void show_(){
        // std::cout<<"vonBertalanffy:\n";
        // std::cout<<"k = "<<this->k.value<<"\n";
        // std::cout<<"a_min = "<<this->a_min.value<<"\n";
        // std::cout<<"l_inf = "<<this->l_inf.value<<"\n";
        // std::cout<<std::setw(15)<<"observed  "<<std::setw(15)<<"predicted\n";
        // for(int i =0; i < this->predicted.size(); i++){
        //     std::cout<<std::setw(15)<<this->obs[i]<<std::setw(15)<<this->predicted[i]<<"\n";
        // }
        
        
        Rcout<<"vonBertalanffy:\n";
        Rcout<<"k = "<< exp(this->log_k_mean.value)<<"\n";
        Rcout<<"a_min = "<<this->a_min.value<<"\n";
        Rcout<<"l_inf = "<<exp(this->log_l_inf_mean.value)<<"\n";
        Rcout<<std::setw(15)<<"observed  "<<std::setw(15)<<"predicted\n";
        for(int i =0; i < this->predicted.size(); i++){
            Rcout<<std::setw(15)<<this->obs[i]<<std::setw(15)<<this->predicted[i]<<"\n";
        }
        
    }
};
vonBertalanffyInterface* vonBertalanffyInterface::instance = NULL;

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
    .field("log_k_mean", &vonBertalanffyInterface::log_k_mean)
    .field("log_k_sigma", &vonBertalanffyInterface::log_k_sigma)
    .field("log_k", &vonBertalanffyInterface::log_k)
    .field("log_l_inf_mean", &vonBertalanffyInterface::log_l_inf_mean)
    .field("log_l_inf_sigma", &vonBertalanffyInterface::log_l_inf_sigma)
    .field("log_l_inf", &vonBertalanffyInterface::log_l_inf) 
    .field("a_min", &vonBertalanffyInterface::a_min)
    .field("ages", &vonBertalanffyInterface::ages)
    .field("obs", &vonBertalanffyInterface::obs)
    .field("nfish", &vonBertalanffyInterface::nfish)
    .field("fish", &vonBertalanffyInterface::fish)
    .field("predicted", &vonBertalanffyInterface::predicted);
    Rcpp::function("get_parameter_vector", get_parameter_vector);
    Rcpp::function("clear", clear);
};


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
    vonBertalanffyInterface::instance->prepare_template<Type>();
    VonBertalanffyModel<Type>* model =
    VonBertalanffyModel<Type>::getInstance();
    
    //get the parameter values
    PARAMETER_VECTOR(p)
    
    //update the parameter values for type Type
    for(int i =0; i < model->parameters.size(); i++){
        *model->parameters[i] = p[i];
    }
    
    //evaluate the model objective function value
    return model->evaluate();
}



