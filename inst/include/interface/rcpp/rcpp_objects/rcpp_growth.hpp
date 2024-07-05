#ifndef RCPP_GROWTH_HPP
#define  RCPP_GROWTH_HPP


#include <sstream>
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
    uint32_t id; /**< local id of the GrowthInterfaceBase object */
    static std::map<uint32_t, GrowthInterfaceBase*> growth_objects; /**<
    map relating the ID of the GrowthInterfaceBase to the GrowthInterfaceBase
    objects */
    uint32_t module_id;

    GrowthInterfaceBase() {
        this->id = GrowthInterfaceBase::id_g++;
        GrowthInterfaceBase::growth_objects[this->id] = this;
        RcppInterfaceBase::interface_objects.push_back(this);
    }

    virtual ~GrowthInterfaceBase() {
    }

    virtual uint32_t get_id() = 0;
};

uint32_t GrowthInterfaceBase::id_g = 1;
std::map<uint32_t, GrowthInterfaceBase*> GrowthInterfaceBase::growth_objects;

class vonBertalanffyInterface : public GrowthInterfaceBase {
public:
    Variable logk;
    Variable l_inf;
    Variable a_min;
    Variable alpha;
    Variable beta;

    vonBertalanffyInterface() : GrowthInterfaceBase() {
    }

    virtual ~vonBertalanffyInterface() {
    }

    virtual uint32_t get_id() {
        return this->id;
    }

    virtual std::string get_module_name() {
        return "vonBertalanffy";
    }

    template<typename Type>
    bool prepare_local() {
        std::shared_ptr<Information<Type> > info =
                Information<Type>::getInstance();

        std::shared_ptr<Model<Type> > model = Model<Type>::getInstance();
        std::shared_ptr< VonBertalanffy<Type> > vb =
                std::make_shared<VonBertalanffy<Type> >();

        this->set_r_name();

        std::stringstream ss;

        ss << this->get_module_name() << "_" << this->id;// << "_" << this->r_name;

         std::string key = ss.str();
        ss.str("");

        vb->id = this->id;

        //initialize k
        vb->logk.resize(1);
        vb->logk[0] = this->logk.value;
        //need to set up maps even when parameters are not estimable (e.g. for penalties or random effects)
        ss << key << "_logk";
        info->variable_map[ss.str()] = &(vb)->logk;

        if (this->logk.estimable) {
            model->parameters.push_back(&(vb)->logk[0]);
            model->pnames.push_back(ss.str());
        }
        ss.str("");

        //initialize l_inf
        vb->l_inf.resize(1);
        vb->l_inf[0] = this->l_inf.value;
        ss << key << "_l_inf";
        info->variable_map[ss.str()] = &(vb)->l_inf;

        if (this->l_inf.estimable) {
            model->parameters.push_back(&(vb)->l_inf[0]);
            model->pnames.push_back(ss.str());
        }
        ss.str("");

        //initialize a_min
        vb->a_min.resize(1);
        vb->a_min[0] = this->a_min.value;
        ss << key << "_a_min";
        info->variable_map[ss.str()] = &(vb)->a_min;

        if (this->a_min.estimable) {
            model->parameters.push_back(&(vb)->a_min[0]);
            model->pnames.push_back(ss.str());
        }
        ss.str("");

        info->vb_models[vb->id] = vb;
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
     * Print model values.
     */
 /*
    void show()const{

        Rcout << "vonBertalanffy\n";
        Rcout << "id:" << this->id << "\n";
//        Rcout << "function value: " << this->objective_function_value << "\n";
//
//        Rcout << std::setw(15) << "observed  " << std::setw(15) << "predicted\n";
//        for (int i = 0; i < this->predicted.size(); i++) {
//            Rcout << std::setw(15) << this->obs[i] << std::setw(15) << this->predicted[i] << "\n";
//        }
//        Rcout << "k = " << exp(this->log_k_mean.value) << "\n";
//        Rcout << "a_min = " << this->a_min.value << "\n";
//        Rcout << "l_inf = " << exp(this->log_l_inf_mean.value) << "\n";
//        Rcout << std::setw(15) << "log_k  " << std::setw(15) << "log_l_inf\n";
//        for (int i = 0; i < this->log_k.size(); i++) {
//            Rcout << std::setw(15) << Rcpp::as<Variable>(this->log_k[i]).value << std::setw(15) << Rcpp::as<Variable>(this->log_l_inf[i]).value << "\n";
//        }
    }
*/
    /**
     * Update the model parameter values and finalize. Sets the parameter values and evaluates the
     * portable model once and transfers values back to the Rcpp interface.
     */
    void finalize(Rcpp::NumericVector v) {
        std::shared_ptr< Information<double> > info = Information<double>::getInstance();
        std::shared_ptr< Model<double> > model = Model<double>::getInstance();

        for (int i = 0; i < v.size(); i++) {
            (*model->parameters[i]) = v[i];
        }

        double f = model->evaluate();

        this->logk.value = info->vb_models[this->id]->logk[0];
        this->a_min.value = info->vb_models[this->id]->a_min[0];
        this->l_inf.value = info->vb_models[this->id]->l_inf[0];


    } 

//    /**
//     * Print model values.
//     */
//    void show_() {
//        Rcpp::Rcout << "vonBertalanffy:\n";
//        Rcpp::Rcout << "k = " << this->k.value << "\n";
//        Rcpp::Rcout << "a_min = " << this->a_min.value << "\n";
//        Rcpp::Rcout << "l_inf = " << this->l_inf.value << "\n";
//    }

};



#endif