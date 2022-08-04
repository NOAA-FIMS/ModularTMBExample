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
    Rcpp::NumericVector data;
    Rcpp::NumericVector ages;
    Rcpp::NumericVector predicted;
    Variable k;
    Variable l_inf;
    Variable a_min;
    Variable alpha;
    Variable beta;
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
        
        // if(this->data.size() != this->ages.size()){
        //     std::cout<<"Error: ages vector length not equal to data vector length, abort\n";
        //     return;
        // }

        if(this->data.size() != this->ages.size()){
            Rcpp::stop("ages vector length not equal to data vector length");
        }
        
        VonBertalanffyModel<Type>* model =
        VonBertalanffyModel<Type>::getInstance();
        
        model->clear();
        
        model->predicted.resize(this->data.size());
        
        model->ages.resize(this->ages.size());
        model->data.resize(this->data.size());
        
        for(int i =0; i < this->data.size(); i++){
            model->ages[i] = this->ages[i];
            model->data[i] = this->data[i];
        }
        
        //initialize k
        model->k = this->k.value;
        
        //initialize l_inf
        model->l_inf = this->l_inf.value;
        
        //initialize a_min
        model->a_min = this->a_min.value;
        
        //initialize alpha
        model->alpha = this->alpha.value;
        
        //initialize beta
        model->beta = this->beta.value;
        
        if(this->k.estimable){
            model->parameters.push_back(&model->k);
        }
        
        if(this->l_inf.estimable){
            model->parameters.push_back(&model->l_inf);
        }
        
        if(this->a_min.estimable){
            model->parameters.push_back(&model->a_min);
        }
        
        if(this->alpha.estimable){
            model->parameters.push_back(&model->alpha);
        }
        
        if(this->beta.estimable){
            model->parameters.push_back(&model->beta);
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
        
        this->k.value = model->k;
        this->a_min.value = model->a_min;
        this->l_inf.value = model->l_inf;
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
        //     std::cout<<std::setw(15)<<this->data[i]<<std::setw(15)<<this->predicted[i]<<"\n";
        // }
        
        
        Rcout<<"vonBertalanffy:\n";
        Rcout<<"k = "<<this->k.value<<"\n";
        Rcout<<"a_min = "<<this->a_min.value<<"\n";
        Rcout<<"l_inf = "<<this->l_inf.value<<"\n";
        Rcout<<std::setw(15)<<"observed  "<<std::setw(15)<<"predicted\n";
        for(int i =0; i < this->predicted.size(); i++){
            Rcout<<std::setw(15)<<this->data[i]<<std::setw(15)<<this->predicted[i]<<"\n";
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



