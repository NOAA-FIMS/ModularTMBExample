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
    bool prepare_local() {

        std::shared_ptr<Model<Type> > model = Model<Type>::getInstance();
        std::shared_ptr< VonBertalanffy<Type> > vb;
        vb = std::make_shared<VonBertalanffy<Type> >();


        //initialize k
        vb->k = this->k.value;

        //initialize l_inf
        vb->l_inf = this->l_inf.value;

        //initialize a_min
        vb->a_min = this->a_min.value;

        //initialize alpha
        vb->alpha = this->alpha.value;

        //initialize beta
        vb->beta = this->beta.value;


        if (this->k.estimable) {
            model->parameters.push_back(&(vb)->k);
        }

        if (this->l_inf.estimable) {
            model->parameters.push_back(&(vb)->l_inf);
        }

        if (this->a_min.estimable) {
            model->parameters.push_back(&(vb)->a_min);
        }

        if (this->alpha.estimable) {
            model->parameters.push_back(&(vb)->alpha);
        }

        if (this->beta.estimable) {
            model->parameters.push_back(&(vb)->beta);
        }
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
        std::shared_ptr< VonBertalanffy<double> > vb;
        vb = std::make_shared<VonBertalanffy<double> >();


        for (int i = 0; i < v.size(); i++) {
            (*model->parameters[i]) = v[i];
        }

        double f = model->evaluate();

        this->k.value = vb->k;
        this->a_min.value = vb->a_min;
        this->l_inf.value = vb->l_inf;


    }

    /**
     * Print model values.
     */
    void show_() {
        Rcpp::Rcout << "vonBertalanffy:\n";
        Rcpp::Rcout << "k = " << this->k.value << "\n";
        Rcpp::Rcout << "a_min = " << this->a_min.value << "\n";
        Rcpp::Rcout << "l_inf = " << this->l_inf.value << "\n";
    }

};

#endif