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

/**
 * Rcpp representation of a variable
 * interface between R and cpp.
 */
class Variable {
public:
    static size_t id_g;
    size_t id;
    static std::vector<Variable*> parameters;
    static std::vector<Variable*> estimated_parameters;
    bool estimable = false;
    bool is_random_effect = false;
    double value = 0;
    int parameter_index = -999;
    int variable_index = -999;
    int random_parameter_index = -999;

    Variable() {
        this->id = Variable::id_g++;
        Variable::parameters.push_back(this);
    }



};
size_t Variable::id_g = 0;

std::vector<Variable*> Variable::parameters;
std::vector<Variable*> Variable::estimated_parameters;

class VariableVector {
    std::vector<Variable> x;
public:

    VariableVector() {
    }

    VariableVector(size_t size) {
        x.resize(size);
    }

    void resize(size_t size) {
        std::cout << "resizing to " << size << std::endl;
        x.resize(size);
        std::cout << x.size() << std::endl;
    }

    Variable& at(size_t index) {
        std::cout << "variable at index " << index << " = " << x[index].value << "\n";
        return x[index];
    }

    Variable& operator[](size_t index) {
        return x[index];
    }

    void set(size_t index, double v) {
        if (index >= x.size()) {
            Rcout << "index out of bounds: " << index << " < " << x.size() - 1 << "\n";
            return;
        }
        x[index].value = v;
        std::cout << "variable at index " << index << " = " << x[index].value << "\n";
    }

    void estimated(size_t index, bool v) {
        this->x[index].estimable = true;
        std::cout << "estimable at index " << index << "   " << this->x[index].estimable << "\n";
    }

    bool is_estimated(size_t index) {
        std::cout << this->x[index].estimable << "\n";
        return this->x[index].estimable;
    }

    void random_effect(size_t index, bool v) {
        std::cout << "random effect " << v << "\n";
        this->x[index].is_random_effect = v;
    }

    bool is_random_effect(size_t index) {
        return this->x[index].is_random_effect;
    }

    size_t size() {
        return x.size();
    }

    int get_parameter_index(size_t index) {
        return x[index].parameter_index;
    }

    size_t get_random_parameter_index(size_t index) {
        return x[index].random_parameter_index;
    }

    void show() {
        for (int i = 0; i < x.size(); i++) {
            std::cout << x[i].value << " ";
        }
        std::cout << std::endl;
    }
};

class vonBertalanffyInterface {
public:
    Rcpp::NumericVector obs;
    Rcpp::NumericVector ages;
    Rcpp::NumericVector predicted;
    Rcpp::List log_l_inf;
    Rcpp::List log_k;
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

    vonBertalanffyInterface(size_t nfish) {
        this->nfish = nfish;
        this->log_k = Rcpp::List(nfish);
        this->log_l_inf = Rcpp::List(nfish);
        for (int i = 0; i < this->log_l_inf.size(); i++) {
            this->log_l_inf[i] = Variable();
            std::cout << "can access here: " << Rcpp::as<Variable>(this->log_l_inf[i]).value << "\n";
            this->log_k[i] = Variable();
        }

    }

    void check_list() {
        for (int i = 0; i < this->log_l_inf.size(); i++) {

            std::cout << 1 << " can access here: " << Rcpp::as<Variable>(this->log_l_inf[i]).value << "\n";

        }
    }

    template<class Type>
    void prepare_template() {



        if (this->obs.size() != this->ages.size()) {
            Rcpp::stop("ages vector length not equal to obs vector length");
        }

        VonBertalanffyModel<Type>* model =
                VonBertalanffyModel<Type>::getInstance();


        model->predicted.resize(this->obs.size());
        model->log_l_inf.resize(this->obs.size());
        model->log_k.resize(this->obs.size());
        model->ages.resize(this->ages.size());
        model->obs.resize(this->obs.size());
        model->fish.resize(this->fish.size());
        for (int i = 0; i < this->obs.size(); i++) {
            model->ages[i] = this->ages[i];
            model->obs[i] = this->obs[i];
            model->fish[i] = this->fish[i];
        }
        model->nfish = this->nfish;

        //initialize a_min
        model->a_min = this->a_min.value;
        model->variable_map[this->a_min.id] = &model->a_min;

        //initialize k
        model->log_k_mean = this->log_k_mean.value;
        model->variable_map[this->log_k_mean.id] = &model->log_k_mean;

        model->log_k_sigma = this->log_k_sigma.value;
        model->variable_map[this->log_k_sigma.id] = &model->log_k_sigma;

        //initialize l_inf
        model->log_l_inf_mean = this->log_l_inf_mean.value;
        model->variable_map[this->log_l_inf_mean.id] = &model->log_l_inf_mean;

        model->log_l_inf_sigma = this->log_l_inf_sigma.value;
        model->variable_map[this->log_l_inf_sigma.id] = &model->log_l_inf_sigma;


        for (int i = 0; i < this->log_l_inf.size(); i++) {
            Rcpp::Rcout << "can't access here: " << Rcpp::as<Variable>(this->log_l_inf[i]).value << "\n";
            model->log_l_inf[i] = (Rcpp::as<Variable>(this->log_l_inf[i]).value);
            model->variable_map[Rcpp::as<Variable>(this->log_l_inf[i]).id] = &model->log_l_inf[i];

            if (Rcpp::as<Variable>(this->log_l_inf[i]).estimable) {
                if (Rcpp::as<Variable>(this->log_l_inf[i]).is_random_effect) {
                    model->random_effects.push_back(&model->log_l_inf[i]);
                    //                    Rcpp::as<Variable>(this->log_l_inf[i]).random_parameter_index = model->random_effects.size() - 1;
                } else {
                    model->parameters.push_back(&model->log_l_inf[i]);
                    //                    Rcpp::as<Variable>(this->log_l_inf[i]).parameter_index = model->parameters.size() - 1;
                }
            }

             model->variable_map[Rcpp::as<Variable>(this->log_k[i]).id] = &model->log_k[i];
            if (Rcpp::as<Variable>(this->log_k[i]).estimable) {
                if (Rcpp::as<Variable>(this->log_k[i]).is_random_effect) {
                    model->random_effects.push_back(&model->log_k[i]);
                    //                    Rcpp::as<Variable>(this->log_k[i]).random_parameter_index = model->random_effects.size() - 1;
                } else {
                    model->parameters.push_back(&model->log_k[i]);
                    //                    Rcpp::as<Variable>(this->log_k[i]).parameter_index = model->parameters.size() - 1;
                }
            }
        }


        if (this->log_k_mean.estimable) {
            model->parameters.push_back(&model->log_k_mean);
            this->log_k_mean.parameter_index = model->parameters.size() - 1;
        }
        if (this->log_k_sigma.estimable) {
            model->parameters.push_back(&model->log_k_sigma);
            this->log_k_sigma.parameter_index = model->parameters.size() - 1;
        }
        if (this->log_l_inf_mean.estimable) {
            model->parameters.push_back(&model->log_l_inf_mean);
            this->log_l_inf_mean.parameter_index = model->parameters.size() - 1;
        }
        if (this->log_l_inf_sigma.estimable) {
            model->parameters.push_back(&model->log_l_inf_sigma);
            this->log_l_inf_sigma.parameter_index = model->parameters.size() - 1;
        }

        if (this->a_min.estimable) {
            model->parameters.push_back(&model->a_min);
            this->a_min.parameter_index = model->parameters.size() - 1;
        }


    }

    /**
     * Prepares the model to work with TMB.
     */
    void prepare() {
        this->check_list();
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
                    Variable v;
                    v.value =model->log_k[i];
                    v.id = Rcpp::as<Variable>(this->log_k[i]).id;
                    this->log_k[i] = v;
                    Variable v1;
                    v1.value =model->log_k[i];
                    v1.id = Rcpp::as<Variable>(this->log_k[i]).id;
                    this->log_k[i] = v1;
                    
//                    Rcpp::as<Variable>(this->log_k[i]).value = model->log_k[i];
//                    Rcpp::as<Variable>(this->log_l_inf[i]).value = model->log_l_inf[i];
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
                    Rcout << std::setw(15) << Rcpp::as<Variable>(this->log_k[i]).value << std::setw(15) << Rcpp::as<Variable>(this->log_l_inf[i]).value << "\n";
                }
    }
};
vonBertalanffyInterface* vonBertalanffyInterface::instance = NULL;

void MapTo(const Variable& a, const Variable& b) {

    if (a.variable_index == -999) {
        Rcout << "error: variable \"a\" not on variable list\n";
    }

    if (b.variable_index == -999) {
        Rcout << "error: variable \"b\" not on variable list\n";
    }

    std::pair<size_t, size_t> p;
    p.first = a.id;
    p.second = b.id;
    VonBertalanffyModel<TMB_FIMS_REAL_TYPE>::getInstance()->variable_pairs.push_back(p);
    VonBertalanffyModel<TMB_FIMS_FIRST_ORDER>::getInstance()->variable_pairs.push_back(p);
    VonBertalanffyModel<TMB_FIMS_SECOND_ORDER>::getInstance()->variable_pairs.push_back(p);
    VonBertalanffyModel<TMB_FIMS_THIRD_ORDER>::getInstance()->variable_pairs.push_back(p);
}

/**
 * Exposes the Variable and vonBertalanffyInterface classes to R.
 */
RCPP_EXPOSED_CLASS(Variable)
RCPP_EXPOSED_CLASS(VariableVector)
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

    VonBertalanffyModel<TMB_FIMS_REAL_TYPE>::getInstance()->variables.clear();
    VonBertalanffyModel<TMB_FIMS_FIRST_ORDER>::getInstance()->variables.clear();
    VonBertalanffyModel<TMB_FIMS_SECOND_ORDER>::getInstance()->variables.clear();
    VonBertalanffyModel<TMB_FIMS_THIRD_ORDER>::getInstance()->variables.clear();

    VonBertalanffyModel<TMB_FIMS_REAL_TYPE>::getInstance()->variable_map.clear();
    VonBertalanffyModel<TMB_FIMS_FIRST_ORDER>::getInstance()->variable_map.clear();
    VonBertalanffyModel<TMB_FIMS_SECOND_ORDER>::getInstance()->variable_map.clear();
    VonBertalanffyModel<TMB_FIMS_THIRD_ORDER>::getInstance()->variable_map.clear();
}

/**
 * Define the Rcpp module.
 */
RCPP_MODULE(growth) {
    Rcpp::class_<Variable>("Variable")
            .constructor()
            .field("value", &Variable::value)
            .field("estimable", &Variable::estimable)
            .field("is_random_effect", &Variable::is_random_effect)
            .field("parameter_index", &Variable::parameter_index)
            .field("random_parameter_index", &Variable::random_parameter_index);
    Rcpp::class_<VariableVector > ("VariableVector")
            .constructor()
            .constructor<size_t>()
            .method("at", &VariableVector::at)
            .method("set", &VariableVector::set)
            .method("size", &VariableVector::size)
            .method("resize", &VariableVector::resize)
            .method("show", &VariableVector::show)
            .method("estimated", &VariableVector::estimated)
            .method("is_estimated", &VariableVector::is_estimated)
            .method("random_effect", &VariableVector::random_effect)
            .method("is_random_effect", &VariableVector::is_random_effect);
    Rcpp::class_<vonBertalanffyInterface>("vonBertalanffy")
            .constructor<size_t>()
            .method("prepare", &vonBertalanffyInterface::prepare)
            .method("check_list", &vonBertalanffyInterface::check_list)
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
    Rcpp::function("map_to", MapTo);
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
    //update mapped variable
    for (int i = 0; i < model->variable_pairs.size(); i++) {
        *model->variable_map[model->variable_pairs[i].first] = *model->variable_map[model->variable_pairs[i].second];
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



