#ifndef VONBERTALANFFY_HPP
#define  VONBERTALANFFY_HPP

#include <vector>
#include "interface.hpp"

/**
 * Portable von Bertalanffy model. Can be used in multiple
 * modeling platforms.
 */
template<typename Type>
class VonBertalanffyModel {
public:
    //using traits for modeling platform specific structures
    typename model_traits<Type>::data_vector obs;
    std::vector<Type> predicted;
    std::vector<Type> linf;
    std::vector<Type> k;
    std::vector<double> ages;
    std::vector<int> fish;
    std::vector<Type*> parameters;
     std::vector<Type*> variables;
    std::vector<std::pair<int, int > > variable_map;
    std::vector<Type*> random_effects;
    int nfish;
    Type log_k_mean;
    Type log_l_inf_mean;
    Type log_k_sigma;
    Type log_l_inf_sigma;
    Type a_min;

    typename model_traits<Type>::parameter_vector log_k;
    typename model_traits<Type>::parameter_vector log_l_inf;


    //singleton instance based on Type
    static VonBertalanffyModel<Type>* instance;

    /**
     * Returns the sigleton instance of VonBertalanffyModel
     * of type Type.
     */
    static VonBertalanffyModel<Type>* getInstance() {
        return VonBertalanffyModel<Type>::instance;
    }

    /**
     * Objective function to compute least squares
     * of observed and predicted length.
     */
    Type evaluate() {
        if (this->predicted.size() == 0) {
            this->predicted.resize(ages.size());
        }

        this->linf.resize(ages.size());
        this->k.resize(ages.size());
        this->fish.resize(ages.size());
        Type norm2 = 0.0;
        for (int i = 0; i < obs.size(); i++) {
            this->linf[i] = exp(this->log_l_inf_mean + this->log_l_inf[this->fish[i]]);
            this->k[i] = exp(this->log_k_mean + this->log_k[this->fish[i]]);
            Type temp = this->linf[i]*(1.0 - exp(-this->k[i]*(ages[i] - this->a_min)));
            this->predicted[i] = temp;
            norm2 -= dnorm(temp, obs[i], Type(0.1), true);
        }
        // probability of the random effects
        for (int i = 0; i < nfish; i++) {
            norm2 -= dnorm(log_l_inf[i], Type(0.0), exp(log_l_inf_sigma), true);
            norm2 -= dnorm(log_k[i], Type(0.0), exp(log_k_sigma), true);
        }
        return norm2;
    }

    /**
     * clears the estimated parameter list
     */
    void clear() {
        this->parameters.clear();
    }
};

template<class Type>
VonBertalanffyModel<Type>* VonBertalanffyModel<Type>::instance = new VonBertalanffyModel<Type>();


#endif
