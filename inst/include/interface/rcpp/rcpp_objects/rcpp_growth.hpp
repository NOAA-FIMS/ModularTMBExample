#ifndef RCPP_GROWTH_HPP
#define  RCPP_GROWTH_HPP


#include "../../../common/model.hpp"
#include "rcpp_interface_base.hpp"
#include "../../../pop_dy/von_bertalanffy.hpp"

class vonBertalanffyInterface {

public:
    Variable k;
    Variable l_inf;
    Variable a_min;
    Variable alpha;
    Variable beta;


    template<typename Type>
    void prepare_local() {

        std::shared_ptr<Model<Type> > model_1 = Model<Type>::getInstance();

        //        model_1->clear();
        model_1->predicted.resize(this->data.size());
        model_1->ages.resize(this->ages.size());
        model_1->data.resize(this->data.size());
        for (int i = 0; i < this->data.size(); i++) {
            model_1->ages[i] = this->ages[i];
            model_1->data[i] = this->data[i];
        }

        //initialize k
        model_1->vb->k = this->k.value;


        //initialize l_inf
        model_1->vb->l_inf = this->l_inf.value;


        //initialize a_min
        model_1->vb->a_min = this->a_min.value;


        //initialize alpha
        model_1->vb->alpha = this->alpha.value;


        //initialize beta
        model_1->vb->beta = this->beta.value;


        if (this->k.estimable) {
            model_1->parameters.push_back(&(model_1->vb)->k);
        }

        if (this->l_inf.estimable) {
            model_1->parameters.push_back(&(model_1->vb)->l_inf);
        }

        if (this->a_min.estimable) {
            model_1->parameters.push_back(&(model_1->vb)->a_min);
        }

        if (this->alpha.estimable) {
            model_1->parameters.push_back(&(model_1->vb)->alpha);
        }

        if (this->beta.estimable) {
            model_1->parameters.push_back(&(model_1->vb)->beta);
        }
        
    }

    /**
     * Prepares the model to work with TMB.
     */
    void prepare() {


        if (this->data.size() != this->ages.size()) {
            Rcpp::stop("ages vector length not equal to data vector length");
        }

        this->predicted = Rcpp::NumericVector(this->ages.size());

#ifdef TMB_MODEL

        this->prepare_local<TMB_FIMS_REAL_TYPE>();
        this->prepare_local<TMB_FIMS_FIRST_ORDER>();
        this->prepare_local<TMB_FIMS_SECOND_ORDER>();
        this->prepare_local<TMB_FIMS_THIRD_ORDER>();
#endif

    }

    /**
     * Update the model parameter values and finalize. Sets the parameter values and evaluates the
     * portable model once and transfers values back to the Rcpp interface.
     */
    void finalize(Rcpp::NumericVector v) {
        std::shared_ptr< Model<double> > model = Model<double>::getInstance();
        std::shared_ptr<VonBertalanffy<double> > vb = model->vb;

        if (this->data.size() != this->ages.size()) {
            Rcpp::stop("finalize: ages vector length not equal to data vector length");
        }

        for (int i = 0; i < v.size(); i++) {
            (*model->parameters[i]) = v[i];
        }

        double f = model->evaluate();

        this->k.value = vb->k;
        this->a_min.value = vb->a_min;
        this->l_inf.value = vb->l_inf;


        for (int i = 0; i < this->predicted.size(); i++) {
            this->predicted[i] = model->predicted[i];
        }
    }

    /**
     * Print model values.
     */
    void show_() {
        Rcpp::Rcout << "vonBertalanffy:\n";
        Rcpp::Rcout << "k = " << this->k.value << "\n";
        Rcpp::Rcout << "a_min = " << this->a_min.value << "\n";
        Rcpp::Rcout << "l_inf = " << this->l_inf.value << "\n";
        Rcpp::Rcout << std::setw(15) << "observed  " << std::setw(15) << "predicted\n";
        //Rcpp::Rcout << "Predicted size: " << this->predicted.size() << std::endl;
        for (int i = 0; i < this->predicted.size(); i++) {
            Rcpp::Rcout << std::left << std::setw(15) << this->data[i] << std::setw(15) << this->predicted[i] << "\n";
        }

    }

};

#endif