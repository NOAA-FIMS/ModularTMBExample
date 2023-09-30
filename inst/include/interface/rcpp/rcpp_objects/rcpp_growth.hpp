#ifndef RCPP_GROWTH_HPP
#define  RCPP_GROWTH_HPP


#include "../../../common/model.hpp"
#include "rcpp_interface_base.hpp"
#include "../../../pop_dy/von_bertalanffy.hpp"

/****************************************************************
 * Growth Rcpp interface                                   *
 ***************************************************************/
/**
 * @brief Rcpp interface that serves as the parent class for
 * Rcpp growth interfaces. This type should be inherited and not
 * called from R directly.
 *
 */
class GrowthInterfaceBase : public RcppInterfaceBase {
public:
  static uint32_t id_g; /**< static id of the GrowthInterfaceBase object */
uint32_t id;          /**< local id of the GrowthInterfaceBase object */
static std::map<uint32_t, GrowthInterfaceBase*> growth_objects; /**<
 map relating the ID of the GrowthInterfaceBase to the GrowthInterfaceBase
 objects */

GrowthInterfaceBase() {
  this->id = GrowthInterfaceBase::id_g++;
  GrowthInterfaceBase::growth_objects[this->id] = this;
  RcppInterfaceBase::interface_objects.push_back(this);
}

virtual ~GrowthInterfaceBase() {}
};

uint32_t GrowthInterfaceBase::id_g = 1;
std::map<uint32_t, GrowthInterfaceBase*> GrowthInterfaceBase::growth_objects;


class vonBertalanffyInterface : public GrowthInterfaceBase{

public:
    Variable k;
    Variable l_inf;
    Variable a_min;
    Variable alpha;
    Variable beta;
    
    vonBertalanffyInterface() : GrowthInterfaceBase(){}
    
    virtual ~vonBertalanffyInterface() {}

    template<typename Type>
    bool prepare_local() {
        Model<Type>* model = Model<Type>::getInstance();
        VonBertalanffy<Type>* vb = new VonBertalanffy<Type>;
        //Rcpp::XPtr< Model<Type> > model_ptr(model, true);
        //Rcpp::XPtr< VonBertalanffy<Type> > vb_ptr(vb, true);


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

        model->vb = vb;
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
    /*
    template<typename Type>
    void finalize(Rcpp::NumericVector v) {
        Model<double>* model = Model<double>::getInstance();
        Rcpp::XPtr< Model<double> > model_ptr(model, true);
        //std::shared_ptr< VonBertalanffy<double> > vb;
        //vb = std::make_shared<VonBertalanffy<double> >();


        for (int i = 0; i < v.size(); i++) {
            (*model_ptr->parameters[i]) = v[i];
        }

        double f = model_ptr->evaluate();

        this->k.value = model_ptr->vb->k;
        this->a_min.value = model_ptr->vb->a_min;
        this->l_inf.value = model_ptr->vb->l_inf;


    }
*/
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