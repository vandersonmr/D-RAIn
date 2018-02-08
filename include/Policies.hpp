#ifndef QUEUE_HPP
#define QUEUE_HPP

#include <Region.hpp>
#include <Simulator.hpp>
#include <vector>

namespace rain3 {
  class Simulator;
  class QueuePolicies {
    public:
      uint8_t  NumThreads = 1;
      uint32_t InterNativeRatio = 50;

      QueuePolicies(uint8_t NT, uint32_t INR) : NumThreads(NT), InterNativeRatio(INR) {};

      std::vector<std::pair<Simulator*, Region*>> handleWaitQueueParallel(std::vector<std::pair<Simulator*, Region*>>&);
  };
}
#endif
