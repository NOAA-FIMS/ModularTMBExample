
template< class Type>
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
    
      /**
     * Prepares the model to work with TMB.
     */
    void prepare(){
        
        // if(this->data.size() != this->ages.size()){
        //     std::cout<<"Error: ages vector length not equal to data vector length, abort\n";
        //     return;
        // }

        if(this->data.size() != this->ages.size()){
            Rcpp::stop("ages vector length not equal to data vector length");
        }
        
        VonBertalanffyModel<double>* model_1 =
        VonBertalanffyModel<double>::getInstance();
        
        VonBertalanffyModel<AD<double> >* model_2 =
        VonBertalanffyModel<AD<double> >::getInstance();
        
        VonBertalanffyModel<AD<AD<double> > >* model_3 =
        VonBertalanffyModel<AD<AD<double> > >::getInstance();
        
        VonBertalanffyModel<AD<AD<AD<double> > > >* model_4 =
        VonBertalanffyModel<AD<AD<AD<double> > > >::getInstance();
        
        
        model_1->clear();
        model_2->clear();
        model_3->clear();
        model_4->clear();
        
        model_1->predicted.resize(model_1->data.size());
        model_2->predicted.resize(model_1->data.size());
        model_3->predicted.resize(model_1->data.size());
        model_4->predicted.resize(model_1->data.size());
        
        model_1->ages.resize(this->ages.size());
        model_1->data.resize(this->data.size());
        model_2->ages.resize(this->ages.size());
        model_2->data.resize(this->data.size());
        model_3->ages.resize(this->ages.size());
        model_3->data.resize(this->data.size());
        model_4->ages.resize(this->ages.size());
        model_4->data.resize(this->data.size());
        
        for(int i =0; i < this->data.size(); i++){
            model_1->ages[i] = this->ages[i];
            model_1->data[i] = this->data[i];
            
            model_2->ages[i] = this->ages[i];
            model_2->data[i] = this->data[i];
            
            model_2->ages[i] = this->ages[i];
            model_2->data[i] = this->data[i];
            
            model_3->ages[i] = this->ages[i];
            model_3->data[i] = this->data[i];
            
            model_4->ages[i] = this->ages[i];
            model_4->data[i] = this->data[i];
        }
        
        //initialize k
        model_1->k = this->k.value;
        model_2->k = this->k.value;
        model_3->k = this->k.value;
        model_4->k = this->k.value;
        
        //initialize l_inf
        model_1->l_inf = this->l_inf.value;
        model_2->l_inf = this->l_inf.value;
        model_3->l_inf = this->l_inf.value;
        model_4->l_inf = this->l_inf.value;
        
        //initialize a_min
        model_1->a_min = this->a_min.value;
        model_2->a_min = this->a_min.value;
        model_3->a_min = this->a_min.value;
        model_4->a_min = this->a_min.value;
        
        //initialize alpha
        model_1->alpha = this->alpha.value;
        model_2->alpha = this->alpha.value;
        model_3->alpha = this->alpha.value;
        model_4->alpha = this->alpha.value;
        
        //initialize beta
        model_1->beta = this->beta.value;
        model_2->beta = this->beta.value;
        model_3->beta = this->beta.value;
        model_4->beta = this->beta.value;
        
        if(this->k.estimable){
            model_1->parameters.push_back(&model_1->k);
            model_2->parameters.push_back(&model_2->k);
            model_3->parameters.push_back(&model_3->k);
            model_4->parameters.push_back(&model_4->k);
        }
        
        if(this->l_inf.estimable){
            model_1->parameters.push_back(&model_1->l_inf);
            model_2->parameters.push_back(&model_2->l_inf);
            model_3->parameters.push_back(&model_3->l_inf);
            model_4->parameters.push_back(&model_4->l_inf);
        }
        
        if(this->a_min.estimable){
            model_1->parameters.push_back(&model_1->a_min);
            model_2->parameters.push_back(&model_2->a_min);
            model_3->parameters.push_back(&model_3->a_min);
            model_4->parameters.push_back(&model_4->a_min);
        }
        
        if(this->alpha.estimable){
            model_1->parameters.push_back(&model_1->alpha);
            model_2->parameters.push_back(&model_2->alpha);
            model_3->parameters.push_back(&model_3->alpha);
            model_4->parameters.push_back(&model_4->alpha);
        }
        
        if(this->beta.estimable){
            model_1->parameters.push_back(&model_1->beta);
            model_2->parameters.push_back(&model_2->beta);
            model_3->parameters.push_back(&model_3->beta);
            model_4->parameters.push_back(&model_4->beta);
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