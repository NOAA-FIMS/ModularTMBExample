#ifndef RCPP_GROWTH
#define RCPP_GROWTH

#include "rcpp_interface_base.hpp"

class vonBertalanffyInterface  : public RcppInterfaceBase {
public:
    Variable k;
    Variable l_inf;
    Variable a_min;
    Variable alpha;
    Variable beta;
    
      /**
     * Prepares the model to work with TMB.
     */
    virtual bool prepare(){
               
        VonBertalanffy<double>* vb_1;
        Model<double>* model_1 = Model<double>::getInstance();
        
        VonBertalanffy<double>* vb_2;
        Model<double>* model_2 = Model<double>::getInstance();
        
        VonBertalanffy<double>* vb_3;
        Model<double>* model_3 = Model<double>::getInstance();
        
        VonBertalanffy<double>* vb_4;
        Model<double>* model_4 = Model<double>::getInstance();
        
        model_1->clear();
        model_2->clear();
        model_3->clear();
        model_4->clear();
       
        //initialize k
        vb_1->k = this->k.value;
        vb_2->k = this->k.value;
        vb_3->k = this->k.value;
        vb_4->k = this->k.value;
        
        //initialize l_inf
        vb_1->l_inf = this->l_inf.value;
        vb_2->l_inf = this->l_inf.value;
        vb_3->l_inf = this->l_inf.value;
        vb_4->l_inf = this->l_inf.value;
        
        //initialize a_min
        vb_1->a_min = this->a_min.value;
        vb_2->a_min = this->a_min.value;
        vb_3->a_min = this->a_min.value;
        vb_4->a_min = this->a_min.value;
        
        //initialize alpha
        vb_1->alpha = this->alpha.value;
        vb_2->alpha = this->alpha.value;
        vb_3->alpha = this->alpha.value;
        vb_4->alpha = this->alpha.value;
        
        //initialize beta
        vb_1->beta = this->beta.value;
        vb_2->beta = this->beta.value;
        vb_3->beta = this->beta.value;
        vb_4->beta = this->beta.value;
        
        if(this->k.estimable){
            model_1->parameters.push_back(&vb_1->k);
            model_2->parameters.push_back(&vb_2->k);
            model_3->parameters.push_back(&vb_3->k);
            model_4->parameters.push_back(&vb_4->k);
        }
        
        if(this->l_inf.estimable){
            model_1->parameters.push_back(&vb_1->l_inf);
            model_2->parameters.push_back(&vb_2->l_inf);
            model_3->parameters.push_back(&vb_3->l_inf);
            model_4->parameters.push_back(&vb_4->l_inf);
        }
        
        if(this->a_min.estimable){
            model_1->parameters.push_back(&vb_1->a_min);
            model_2->parameters.push_back(&vb_2->a_min);
            model_3->parameters.push_back(&vb_3->a_min);
            model_4->parameters.push_back(&vb_4->a_min);
        }
        
        if(this->alpha.estimable){
            model_1->parameters.push_back(&vb_1->alpha);
            model_2->parameters.push_back(&vb_2->alpha);
            model_3->parameters.push_back(&vb_3->alpha);
            model_4->parameters.push_back(&vb_4->alpha);
        }
        
        if(this->beta.estimable){
            model_1->parameters.push_back(&vb_1->beta);
            model_2->parameters.push_back(&vb_2->beta);
            model_3->parameters.push_back(&vb_3->beta);
            model_4->parameters.push_back(&vb_4->beta);
        }
        
    }
    
    /**
     * Update the model parameter values and finalize. Sets the parameter values and evaluates the
     * portable model once and transfers values back to the Rcpp interface.
     */
    void finalize(Rcpp::NumericVector v){
        Model<double>* model = Model<double>::getInstance();
        VonBertalanffy<double>* vb;
        
        for(int i =0; i < v.size(); i++){
            (*model->parameters[i]) = v[i];
        }
        
        double f = model->evaluate();
        
        this->k.value = vb->k;
        this->a_min.value = vb->a_min;
        this->l_inf.value = vb->l_inf;
        
    }
            
};

#endif