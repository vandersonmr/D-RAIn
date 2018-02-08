#include <Policies.hpp>
#include <algorithm>
#include <iostream>

using namespace rain3;

#define FIFO 0
#define STACK 1
#define DYN 2
#define QueueType FIFO

std::vector<std::pair<Simulator*, Region*>> QueuePolicies::handleWaitQueueParallel(std::vector<std::pair<Simulator*, Region*>>& RegionsQueue) {
  // Start given new tickets to new regions
  for (auto P : RegionsQueue) {
    Region* R = P.second;
    if (R->getTicket() == 0)
      R->setTicket(R->getSize()*InterNativeRatio+1);
  }

  std::vector<std::pair<Simulator*, Region*>> Compiled;

  uint32_t Processed = 0;
  for (auto P : RegionsQueue) {
    Region* R = P.second;

    if (R->isCompiling()) {
      Processed += 1;
      if (Processed > NumThreads) break;

      R->setTicket(R->getTicket() - 1);
      if (R->isCompiled())  
        Compiled.push_back(P);
    }
  }

  if (QueueType == FIFO) {
    for (auto P : RegionsQueue) {
      Region* R = P.second;

      if (!R->isCompiling()) {
        Processed += 1;
        if (Processed > NumThreads) break;

        R->startCompiling();
      }
    }
  } else if (QueueType == STACK) {
    while (true) {
      Region* LastR = nullptr;
      for (auto P : RegionsQueue) {
        Region* R = P.second;

        if (!R->isCompiling()) {
          LastR = R;
        }
      }

      Processed += 1;
      if (Processed > NumThreads) break;
      if (LastR != nullptr) LastR->startCompiling();
    }
  } else if (QueueType == DYN) {
    while (true) {
      uint32_t Maximum = 0;
      uint32_t Value = 0;
      bool found = false;
      for (uint32_t i = 0; i < RegionsQueue.size(); i++) {
        if (!RegionsQueue[i].second->isCompiling()) {
          if (RegionsQueue[i].second->getWeight() > Value || !found) {
            found = true;
            Maximum = i;
            Value = RegionsQueue[i].second->getWeight();
          }
        }
      }

      Processed += 1;
      if (Processed > NumThreads) break;
      if (found) RegionsQueue[Maximum].second->startCompiling();
    }
  }

  RegionsQueue.erase(
      std::remove_if(RegionsQueue.begin(), RegionsQueue.end(), [](std::pair<Simulator*, Region*> R) -> bool { return R.second->isCompiled(); }),
      RegionsQueue.end());

  return Compiled;
}
