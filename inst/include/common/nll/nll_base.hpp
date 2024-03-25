#ifndef NLL_BASE_HPP
#define NLL_BASE_HPP

#include "../def.hpp"

template <typename Type>
struct NLLBase {

static uint32_t id_g; /**< The ID of the instance of the NLLBase class */
static uint32_t id; /**< The local ID of the instance of the NLLBase class */

  /** @brief Constructor.
   */
  NLLBase() {
    // increment id of the singleton selectivity class
    this->id = NLLBase::id_g++;
  }

  virtual ~NLLBase() {}

  /**
   * @brief Calculates the NLL.
   */
  virtual const Type evaluate() = 0;
};

// default id of the singleton selectivity class
template <typename Type>
uint32_t NLLBase<Type>::id_g = 0;

#endif