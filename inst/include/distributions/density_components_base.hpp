#ifndef DENSITY_COMPONENTS_BASE_HPP
#define DENSITY_COMPONENTS_BASE_HPP

#include "../common/def.hpp"
#include "../common/model_object.hpp"
#include "../common/fims_vector.hpp"
template <typename Type>
struct DensityComponentBase : public ModelObject<Type> {
static uint32_t id_g; /**< The ID of the instance of the DensityComponentBase class */

fims::Vector<Type> observed_value; 
fims::Vector<Type> expected_value;
fims::Vector<Type> log_likelihood_vec;
std::string input_type;
bool simulate_flag;
std::vector<uint32_t> key; 
#ifdef TMB_MODEL
::objective_function<Type>
      *of;  
using DataIndicator = typename model_traits<Type>::data_indicator_;
DataIndicator keep;
#endif

  /** @brief Constructor.
   */
  DensityComponentBase() : ModelObject<Type>() {
    // increment id of the singleton selectivity class
    this->id = DensityComponentBase::id_g++;
  }

  virtual ~DensityComponentBase() {}

  /**
   * @brief Calculates the NLL.
   */
  virtual const Type evaluate() = 0;
};

// default id of the singleton selectivity class
template <typename Type>
uint32_t DensityComponentBase<Type>::id_g = 0;

#endif