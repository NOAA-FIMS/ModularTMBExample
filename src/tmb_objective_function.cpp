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
#include <sstream>

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
    std::string name;
    static std::vector<Variable*> parameters;
    static std::vector<Variable*> estimated_parameters;
    bool estimable = false;
    bool is_random_effect = false;
    double value = 0;

    Variable() {
        this->id = Variable::id_g++;
        Variable::parameters.push_back(this);
    }



};
size_t Variable::id_g = 0;

std::vector<Variable*> Variable::parameters;
std::vector<Variable*> Variable::estimated_parameters;

static void Tokenize(const std::string& str, std::vector<std::string>& tokens,
        const std::string& delimiters = " ", const bool trimEmpty = true) {
    std::string::size_type pos, lastPos = 0;
    while (true) {
        pos = str.find_first_of(delimiters, lastPos);
        if (pos == std::string::npos) {
            pos = str.size();

            if (pos != lastPos || !trimEmpty)
                tokens.push_back(std::vector<std::string>::value_type(str.data() + lastPos,
                    (std::vector<std::string>::value_type::size_type)pos - lastPos));

            break;
        } else {
            if (pos != lastPos || !trimEmpty)
                tokens.push_back(std::vector<std::string>::value_type(str.data() + lastPos,
                    (std::vector<std::string>::value_type::size_type)pos - lastPos));
        }

        lastPos = pos + 1;
    }
}

template <typename T>
T StringToNumber(const std::string &Text) {
    std::istringstream ss(Text);
    T result;
    return (ss >> result) ? result : 0;
}

bool StartsWith(const std::string &value1, const std::string &value2) {
    return value1.find(value2) == 0;
}

class vonBertalanffyInterface {
public:
    static size_t id_g;
    size_t id = 0;
    std::string name;
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
    double objective_function_value;

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
            this->log_k[i] = Variable();
        }

        id = vonBertalanffyInterface::id_g++;
    }

    void check_list() {
        for (int i = 0; i < this->log_l_inf.size(); i++) {


        }
    }

    void set_name() {

        Rcpp::Environment env = Rcpp::Environment::global_env();
        Rcpp::List l = Rcpp::as<Rcpp::List>(env.ls(true));
        SEXP e, E, EE;


        for (int i = 0; i < l.size(); i++) {
            std::stringstream ss;
            ss << "capture.output(show(" << Rcpp::as<std::string>(l[i]) << "))"; //, file = NULL, append = FALSE, type = c(\"output\", \"message\"), split = FALSE)";
            if (StartsWith(Rcpp::as<std::string>(l[i]), ".")) {
                continue;
            }
            SEXP expression, result;
            ParseStatus status;

            PROTECT(expression = R_ParseVector(Rf_mkString(ss.str().c_str()), 1, &status, R_NilValue));

            if (status != PARSE_OK) {
                std::cout << "Error parsing expression" << std::endl;
                UNPROTECT(1);
            }

            PROTECT(result = Rf_eval(VECTOR_ELT(expression, 0), R_GlobalEnv));

            if (TYPEOF(result) == STRSXP) {
                for (int j = 0; j < LENGTH(result); j++) {
                    std::string str(CHAR(STRING_ELT(result, j)));
                    if (str == "vonBertalanffy") {

                        std::string line(CHAR(STRING_ELT(result, j + 1)));
                        std::vector<std::string> tokens;
                        Tokenize(line, tokens, ":");
                        if (StringToNumber<size_t> (tokens[1]) == this->id) {
                            this->name = Rcpp::as<std::string>(l[i]);
                        }
                    } else {
                        break;
                    }
                    std::cout << CHAR(STRING_ELT(result, j)) << std::endl;
                }
            }
        }
        UNPROTECT(2);



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

        }

        for (int i = 0; i < this->log_l_inf.size(); i++) {
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
        }
        if (this->log_k_sigma.estimable) {
            model->parameters.push_back(&model->log_k_sigma);
        }
        if (this->log_l_inf_mean.estimable) {
            model->parameters.push_back(&model->log_l_inf_mean);
        }
        if (this->log_l_inf_sigma.estimable) {
            model->parameters.push_back(&model->log_l_inf_sigma);
        }

        if (this->a_min.estimable) {
            model->parameters.push_back(&model->a_min);
        }


        //
        //        Rcpp::Environment env = Rcpp::Environment::global_env();
        //        Rcpp::List l = Rcpp::as<Rcpp::List>(env.ls(true));
        //
        //        for (int i = 0; i < l.size(); i++) {
        //            Rcpp::Vector<19, Rcpp::PreserveStorage>* V = l[i].parent;
        //            std::cout << "Rcpp::PreserveStorage:\n";
        //            std::cout << Rcpp::as<std::string>(l[i]) << "\n";
        //            for (int j = 0; j < V->size(); j++) {
        //                std::cout << "     " << V[j].get__() << "  " << Rcpp::wrap(*this) << "\n";
        //            }
        //
        //        }
        //        exit(0);



        //        std::cout << Rf_type2char(Rcpp::as<SEXPTYPE>(*this)) << "\n";
    }

    /**
     * Prepares the model to work with TMB.
     */
    void prepare() {

        this->set_name();
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
        Rcpp::Rcout << "finalizing...." << std::endl;
        VonBertalanffyModel<double>* model =
                VonBertalanffyModel<double>::getInstance();


        objective_function_value = model->evaluate();



        this->log_k_mean.value = model->log_k_mean;
        this->log_k_sigma.value = model->log_k_sigma;

        this->log_l_inf_mean.value = model->log_l_inf_mean;
        this->log_l_inf_sigma.value = model->log_l_inf_sigma;
        this->a_min.value = model->a_min;
        this->predicted = Rcpp::NumericVector(model->predicted.size());
        for (int i = 0; i < this->log_k.size(); i++) {
            Variable v;
            v.value = model->log_k[i];
            v.id = Rcpp::as<Variable>(this->log_k[i]).id;
            this->log_k[i] = v;
            Variable v1;
            v1.value = model->log_l_inf[i];
            v1.id = Rcpp::as<Variable>(this->log_l_inf[i]).id;
            this->log_l_inf[i] = v1;

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

        Rcout << "vonBertalanffy\n";
        Rcout << "id:" << this->id << "\n";
        Rcout << "function value: " << this->objective_function_value << "\n";

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
size_t vonBertalanffyInterface::id_g = 1;



void MapTo(const Variable& a, const Variable& b) {

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
            .field("is_random_effect", &Variable::is_random_effect);
    Rcpp::class_<vonBertalanffyInterface>("vonBertalanffy")
            .constructor<size_t>()
            .method("prepare", &vonBertalanffyInterface::prepare)
            .method("check_list", &vonBertalanffyInterface::check_list)
            .method("finalize", &vonBertalanffyInterface::finalize)
            .method("show", &vonBertalanffyInterface::show_)
            .field("name", &vonBertalanffyInterface::name)
            .field("id", &vonBertalanffyInterface::id)
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



