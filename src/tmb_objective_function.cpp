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

#define TMB_FIMS_REAL_TYPE double
#define TMB_FIMS_FIRST_ORDER AD<TMB_FIMS_REAL_TYPE >
#define TMB_FIMS_SECOND_ORDER AD<TMB_FIMS_FIRST_ORDER >
#define TMB_FIMS_THIRD_ORDER AD<TMB_FIMS_SECOND_ORDER >


Rcpp::NumericVector get_parameter_vector();
Rcpp::NumericVector get_random_effects_vector();
Rcpp::List get_parameter_list();

/**
 * Rcpp representation of a variable
 * interface between R and cpp.
 */
class Variable {
public:
    static std::vector<Variable*> parameters;
    static std::vector<Variable*> estimated_parameters;
    bool estimable = false;
    bool is_random_effect = false;
    double value = 0;

    Variable() {
        Variable::parameters.push_back(this);
    }

};


std::vector<Variable*> Variable::parameters;
std::vector<Variable*> Variable::estimated_parameters;

class vonBertalanffyInterface {
public:
    Rcpp::NumericVector obs;
    Rcpp::NumericVector ages;
    Rcpp::NumericVector predicted;
    Rcpp::NumericVector log_l_inf;
    Rcpp::NumericVector log_k;
    Rcpp::IntegerVector fish;
    int nfish;
    Variable a_min;
    //    Variable log_l_inf;
    Variable log_l_inf_sigma;
    Variable log_l_inf_mean;
    //    Variable log_k;
    Variable log_k_sigma;
    Variable log_k_mean;


    bool log_l_inf_is_estimated = true;
    bool log_l_inf_is_random_effect = true;
    bool log_k_is_estimated = true;
    bool log_k_is_random_effect = true;

    static vonBertalanffyInterface* instance;

    vonBertalanffyInterface() {
        instance = this;
    }

    //  

    template<class Type>
    void prepare_template() {

        // if(this->obs.size() != this->ages.size()){
        //     std::cout<<"Error: ages vector length not equal to obs vector length, abort\n";
        //     return;
        // }

        if (this->obs.size() != this->ages.size()) {
            Rcpp::stop("ages vector length not equal to obs vector length");
        }

        VonBertalanffyModel<Type>* model =
                VonBertalanffyModel<Type>::getInstance();

        // model->clear();

        model->predicted.resize(this->obs.size());
        model->log_l_inf.resize(this->obs.size());
        model->log_k.resize(this->obs.size());
        model->ages.resize(this->ages.size());
        model->obs.resize(this->obs.size());
        model->fish.resize(this->fish.size());
        model->parameter_names.resize(100);
        for (int i = 0; i < this->obs.size(); i++) {
            model->ages[i] = this->ages[i];
            model->obs[i] = this->obs[i];
	    model->fish[i] = this->fish[i];
        }
        model->nfish = this->nfish;

        //initialize a_min
        model->a_min = this->a_min.value;

        //initialize k
        model->log_k_mean = this->log_k_mean.value;
        model->log_k_sigma = this->log_k_sigma.value;
        //initialize l_inf
        model->log_l_inf_mean = this->log_l_inf_mean.value;
        model->log_l_inf_sigma = this->log_l_inf_sigma.value;
        model->log_k.resize(this->obs.size());
        model->log_l_inf.resize(this->obs.size());
        // model->log_l_inf = this->log_l_inf;
        // model->log_k = this->log_k;
size_t index;
        for (int i = 0; i < nfish; i++) {
            model->log_l_inf[i] = (this->log_l_inf[i]);
            if (this->log_l_inf_is_estimated) {
                if (this->log_l_inf_is_random_effect) {
                    model->random_effects.push_back(&model->log_l_inf[i]);
                    index = model->parameters.size() - 1;
                } else {
                    model->parameters.push_back(&model->log_l_inf[i]);
                    
                }
                
                model->parameter_names[index] ="log_l_inf";
            }

            model->log_k[i] = (this->log_k[i]);
            if (this->log_k_is_estimated) {
                if (this->log_k_is_random_effect) {
                    model->random_effects.push_back(&model->log_k[i]);
                    
                } else {
                    model->parameters.push_back(&model->log_k[i]);
                }
                index = model->parameters.size() - 1;
                model->parameter_names[index] ="log_k";
            }
        }


        if (this->log_k_mean.estimable) {
            model->parameters.push_back(&model->log_k_mean);
            
                index = model->parameters.size() - 1;
                model->parameter_names[index] ="log_k_mean";
        }
        if (this->log_k_sigma.estimable) {
            model->parameters.push_back(&model->log_k_sigma);
            
                index = model->parameters.size() - 1;
                model->parameter_names[index] ="log_k_sigma";
        }
        if (this->log_l_inf_mean.estimable) {
            model->parameters.push_back(&model->log_l_inf_mean);
            
                index = model->parameters.size() - 1;
                model->parameter_names[index] ="log_l_inf_mean";
        }
        if (this->log_l_inf_sigma.estimable) {
            model->parameters.push_back(&model->log_l_inf_sigma);
            model->parameter_names.push_back("log_l_inf_sigma");
            
                index = model->parameters.size() - 1;
                model->parameter_names[index] ="log_l_inf_sigma";
        }
        //        for (int i = 0; i<this->nfish; i++) {
        //            if (this->log_k.estimable[i]) {
        //                model->parameters.push_back(&model->log_k[i]);
        //            }
        //            if (this->log_l_inf.estimable[i]) {
        //                model->parameters.push_back(&model->log_l_inf[i]);
        //            }
        //        }
        if (this->a_min.estimable) {
            model->parameters.push_back(&model->a_min);
            model->parameter_names.push_back("a_min");
            
                index = model->parameters.size() - 1;
                model->parameter_names[index] ="a_min";
        }


    }

    /**
     * Prepares the model to work with TMB.
     */
    void prepare() {
        prepare_template<TMB_FIMS_REAL_TYPE>();
        prepare_template<TMB_FIMS_FIRST_ORDER>();
        prepare_template<TMB_FIMS_SECOND_ORDER>();
        prepare_template<TMB_FIMS_THIRD_ORDER>();
    }

    /**
     * Update the model parameter values and finalize. Sets the parameter values and evaluates the
     * portable model once and transfers values back to the Rcpp interface.
     */
    void finalize() {

        Rcpp::NumericVector v = get_parameter_vector();
        Rcpp::NumericVector r = get_random_effects_vector();
        std::cout << "finalizing....\n";
        VonBertalanffyModel<double>* model =
                VonBertalanffyModel<double>::getInstance();

        for (int i = 0; i < v.size(); i++) {
            (*model->parameters[i]) = v[i];
        }
        for (int i = 0; i < v.size(); i++) {
            (*model->random_effects[i]) = r[i];
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
        for (int i = 0; i < this->log_k.size(); i++) {
            this->log_k[i] = model->log_k[i];
            this->log_l_inf[i] = model->log_l_inf[i];
        }

        for (int i = 0; i < model->predicted.size(); i++) {
            this->predicted[i] = model->predicted[i];
        }

    }

    /**
     * Print model values.
     */
    void show_() {
        // std::cout<<"vonBertalanffy:\n";
        // std::cout<<"k = "<<this->k.value<<"\n";
        // std::cout<<"a_min = "<<this->a_min.value<<"\n";
        // std::cout<<"l_inf = "<<this->l_inf.value<<"\n";
        // std::cout<<std::setw(15)<<"observed  "<<std::setw(15)<<"predicted\n";
        // for(int i =0; i < this->predicted.size(); i++){
        //     std::cout<<std::setw(15)<<this->obs[i]<<std::setw(15)<<this->predicted[i]<<"\n";
        // }


        Rcout << "vonBertalanffy:\n";



        Rcout << std::setw(15) << "observed  " << std::setw(15) << "predicted\n";
        for (int i = 0; i < this->predicted.size(); i++) {
            Rcout << std::setw(15) << this->obs[i] << std::setw(15) << this->predicted[i] << "\n";
        }
        Rcout << "k = " << exp(this->log_k_mean.value) << "\n";
        Rcout << "a_min = " << this->a_min.value << "\n";
        Rcout << "l_inf = " << exp(this->log_l_inf_mean.value) << "\n";
        Rcout << std::setw(15) << "log_k  " << std::setw(15) << "log_l_inf\n";
        for (int i = 0; i < this->log_k.size(); i++) {
            Rcout << std::setw(15) << this->log_k[i] << std::setw(15) << this->log_l_inf[i] << "\n";
        }
    }
};
vonBertalanffyInterface* vonBertalanffyInterface::instance = NULL;

/**
 * Exposes the Variable and vonBertalanffyInterface classes to R.
 */
RCPP_EXPOSED_CLASS(Variable)
RCPP_EXPOSED_CLASS(Variable2)
RCPP_EXPOSED_CLASS(vonBertalanffyInterface)

/**
 * Returns the initial values for the parameter set
 */
Rcpp::NumericVector get_parameter_vector() {
    Rcpp::NumericVector p;
    VonBertalanffyModel<double>* model =
            VonBertalanffyModel<double>::getInstance();


    for (int i = 0; i < model->parameters.size(); i++) {
        p.push_back(*model->parameters[i]);
    }

    return p;
}

Rcpp::List get_parameter_list(){
        VonBertalanffyModel<double>* model =
            VonBertalanffyModel<double>::getInstance();
    Rcpp::StringVector ns;
    Rcpp::NumericVector vals;

    for (int i = 0; i < model->parameters.size(); i++) {
        ns.push_back(model->parameter_names[i]);
        vals.push_back(*model->parameters[i]);
    }

    return  Rcpp::List::create(Rcpp::Named("names") = ns,
    Rcpp::Named("values") = vals);
}
 
/**
 * Returns the initial values for the parameter set
 */
Rcpp::NumericVector get_random_effects_vector() {
    Rcpp::NumericVector p;

    VonBertalanffyModel<double>* model =
            VonBertalanffyModel<double>::getInstance();


    for (int i = 0; i < model->random_effects.size(); i++) {
        p.push_back(*model->random_effects[i]);
    }

    return p;
}

/**
 * Clears the vector of independent variables.
 */
void clear() {
    VonBertalanffyModel<TMB_FIMS_REAL_TYPE>::getInstance()->parameters.clear();
    VonBertalanffyModel<TMB_FIMS_FIRST_ORDER>::getInstance()->parameters.clear();
    VonBertalanffyModel<TMB_FIMS_SECOND_ORDER>::getInstance()->parameters.clear();
    VonBertalanffyModel<TMB_FIMS_THIRD_ORDER>::getInstance()->parameters.clear();

    VonBertalanffyModel<TMB_FIMS_REAL_TYPE>::getInstance()->random_effects.clear();
    VonBertalanffyModel<TMB_FIMS_FIRST_ORDER>::getInstance()->random_effects.clear();
    VonBertalanffyModel<TMB_FIMS_SECOND_ORDER>::getInstance()->random_effects.clear();
    VonBertalanffyModel<TMB_FIMS_THIRD_ORDER>::getInstance()->random_effects.clear();
}

/**
 * Define the Rcpp module.
 */
RCPP_MODULE(growth) {
    Rcpp::class_<Variable>("Variable")
            .constructor()
            .field("value", &Variable::value)
            .field("estimable", &Variable::estimable);
    Rcpp::class_<vonBertalanffyInterface>("vonBertalanffy")
            .constructor()
            .method("prepare", &vonBertalanffyInterface::prepare)
            .method("finalize", &vonBertalanffyInterface::finalize)
            .method("show", &vonBertalanffyInterface::show_)
            .field("log_k_mean", &vonBertalanffyInterface::log_k_mean)
            .field("log_k_sigma", &vonBertalanffyInterface::log_k_sigma)
            .field("log_k", &vonBertalanffyInterface::log_k)
            .field("log_k_is_estimated", &vonBertalanffyInterface::log_k_is_estimated)
            .field("log_k_is_random_effect", &vonBertalanffyInterface::log_k_is_random_effect)
            .field("log_l_inf_mean", &vonBertalanffyInterface::log_l_inf_mean)
            .field("log_l_inf_sigma", &vonBertalanffyInterface::log_l_inf_sigma)
            .field("log_l_inf", &vonBertalanffyInterface::log_l_inf)
            .field("log_l_inf_is_estimated", &vonBertalanffyInterface::log_l_inf_is_estimated)
            .field("log_l_inf_is_random_effect", &vonBertalanffyInterface::log_l_inf_is_random_effect)
            .field("a_min", &vonBertalanffyInterface::a_min)
            .field("ages", &vonBertalanffyInterface::ages)
            .field("obs", &vonBertalanffyInterface::obs)
            .field("nfish", &vonBertalanffyInterface::nfish)
            .field("fish", &vonBertalanffyInterface::fish)
            .field("predicted", &vonBertalanffyInterface::predicted);
    //            .field("linf", &vonBertalanffyInterface::linf)
    //            .field("k", &vonBertalanffyInterface::k);
    Rcpp::function("get_parameter_vector", get_parameter_vector);
    Rcpp::function("get_random_effects_vector", get_random_effects_vector);
    Rcpp::function("get_parameter_list", get_parameter_list);
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
Type objective_function<Type>::operator()() {

    //get the singleton instance for type Type
    VonBertalanffyModel<Type>* model =
            VonBertalanffyModel<Type>::getInstance();

    //get the parameter values
    PARAMETER_VECTOR(p)
            
     PARAMETER_VECTOR(r)

    //update the parameter values for type Type
    for (int i = 0; i < model->parameters.size(); i++) {
        *model->parameters[i] = p[i];
    }

    //update random effects
    for (int i = 0; i < model->random_effects.size(); i++) {
        *model->random_effects[i] = r[i];
    }

    //evaluate the model objective function value
    Type nll = model->evaluate();

    vector<Type> pred = model->predicted;
    vector<Type> k = model->k;
    vector<Type> linf = model->linf;
    Type linf_mean = exp(model->log_l_inf_mean);
    Type k_mean = exp(model->log_k_mean);

    REPORT(pred);
    REPORT(k);
    REPORT(linf);
    REPORT(linf_mean);
    REPORT(k_mean);

    return nll;
}



