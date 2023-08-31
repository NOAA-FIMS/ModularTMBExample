
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
        
        VonBertalanffyModel<Type>* model =
        VonBertalanffyModel<Type>::getInstance();
        
        model->clear();
        
        model->predicted.resize(model->data.size());
        
        model->ages.resize(this->ages.size());
        model->data.resize(this->data.size());
        
        for(int i =0; i < this->data.size(); i++){
            model->ages[i] = this->ages[i];
            model->data[i] = this->data[i];
        }
        
        //initialize k
        model->k = this->k.value;
        
        //initialize l_inf
        model->l_inf = this->l_inf.value;
        
        //initialize a_min
        model->a_min = this->a_min.value;
        
        //initialize alpha
        model->alpha = this->alpha.value;
        
        //initialize beta
        model->beta = this->beta.value;
        
        if(this->k.estimable){
            model->parameters.push_back(&model->k);
        }
        
        if(this->l_inf.estimable){
            model->parameters.push_back(&model->l_inf);
        }
        
        if(this->a_min.estimable){
            model->parameters.push_back(&model->a_min);
        }
        
        if(this->alpha.estimable){
            model->parameters.push_back(&model->alpha);
        }
        
        if(this->beta.estimable){
            model->parameters.push_back(&model->beta);
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