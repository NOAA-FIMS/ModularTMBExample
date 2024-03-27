
#ifndef MODEL_HPP
#define MODEL_HPP

#include "def.hpp"

#include "../pop_dy/von_bertalanffy.hpp"
#include "../pop_dy/population.hpp"
#include "../nll/normal_nll.hpp"
#include "../nll/nll_base.hpp"

template<typename Type>
class Model{
    public:

    std::vector<Type> predicted;
    std::shared_ptr< Population<Type> > pop;
    std::shared_ptr< NormalNLL<Type> > normal;
    /*std::map<uint32_t, std::shared_ptr<NormalNLL<Type> > >
      normal_models;
    typedef typename std::map<
      uint32_t, std::shared_ptr<NormalNLL<Type> > >::iterator
      normal_iterator;
*/
    std::vector<Type*> parameters;

    Model(){ 
      this->pop = std::make_shared<Population<Type> >();
      this->normal = std::make_shared<NormalNLL<Type> >();
    }
    //singleton instance based on Type
    static std::shared_ptr<Model<Type> > instance;

   
/*
    std::shared_ptr<Population<Type> >  pop;
    std::shared_ptr<NormalNLL<Type> >  norm;

    std::vector<Type*> parameters;

  
    Model(){
      this->pop = std::make_shared<Population<Type> >();
      this->norm = std::make_shared<NormalNLL<Type> >();
    } 
*/
 
  /**
   * Returns the sigleton instance of VonBertalanffyModel
   * of type Type.
   */
  static std::shared_ptr<Model<Type> > getInstance(){
    return Model<Type>::instance; //TODO: make sure not null and create if null
  }

     /**
   * Objective function to compute least squares
   * of observed and predicted length.
   */
  const Type evaluate(){

    this->pop ->evaluate();
        /*
    Type jnll = 0.0;
    for(normal_iterator it = this->normal_models.begin(); it!= this->normal_models.end(); ++it){
      std::shared_ptr<NormalNLL<Type> > n = (*it).second;
      jnll += n->evaluate();
    }
    return jnll;
*/

    /*
    Type norm2 = 0.0;
    for(int i =0; i < obsdata -> ages.size(); i++){
        Type pred = vb -> evaluate(obsdata -> ages[i]);
        this->predicted[i] = pred;
        norm2+=(pred-obsdata -> data[i])*(pred-obsdata -> data[i]);
    }
    return norm2;
*/

    Type norm2 = 0.0;
    predicted.resize(this->pop -> ages.size());
    for(int i =0; i < this->pop -> ages.size(); i++){
      predicted[i] = this->pop->length[i];
      // this->predicted[i] = pred;
      norm2+=(predicted[i]-normal -> x[i])*(predicted[i]-normal -> x[i]);
      Rcout << "normal x, " << i << ", is: " << normal -> x[i] << std::endl;
      Rcout << "predicted, " << i << ", is: " << predicted[i] << std::endl;
      Rcout << "pop->length, " << i << ", is: " << this->pop->length[i] << std::endl;
    }
    Rcout << "norm2 is: " << norm2 << std::endl;
    return norm2;
    


  }
  
  /**
   * clears the estimated parameter list
   */
  void clear(){
    this->parameters.clear();
  }
  
  };

template<typename Type>
std::shared_ptr<Model<Type> > Model<Type>::instance = std::make_shared<Model<Type> >();

#endif