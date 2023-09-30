
#ifndef MODEL_HPP
#define MODEL_HPP

#include "def.hpp"

#include "../pop_dy/von_bertalanffy.hpp"
#include "../common/data.hpp"

template<typename Type>
class Model{
    public:

    std::vector<Type> predicted;
    VonBertalanffy<Type>* vb = new VonBertalanffy<Type>;
    ObsData<Type>* obsdata = new ObsData<Type>;

    std::vector<Type*> parameters;
    
    Model(){}

    //singleton instance based on Type
  static Model<Type>* instance;
  
  /**
   * Returns the sigleton instance of VonBertalanffyModel
   * of type Type.
   */
  static Model<Type>* getInstance(){
    return Model<Type>::instance; //TODO: make sure not null and create if null
  }

     /**
   * Objective function to compute least squares
   * of observed and predicted length.
   */
  Type evaluate(){
    Type norm2 = 0.0;
    for(int i =0; i < obsdata -> ages.size(); i++){
        Type pred = vb -> evaluate(obsdata -> ages[i]);
        this->predicted[i] = pred;
        norm2+=(pred-obsdata -> data[i])*(pred-obsdata -> data[i]);
    }
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
Model<Type>* Model<Type>::instance = new Model<Type>();
#endif