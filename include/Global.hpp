# ifndef GLOBAL_H
#define GLOBAL_H

#include <vector>

namespace rain3 {
  class GlobalContext {
    std::vector<std::unique_ptr<rain3::Simulator>> DevicesClass0;
    std::vector<std::unique_ptr<rain3::Simulator>> DevicesClass1;
    std::vector<std::unique_ptr<rain3::Simulator>> Cloud;    

    public:
    enum DeviceType {
      class0, class1, server
    };

    void insertDevice(rain3::Simulator* S, DeviceType T) {
      if (T == class0) 
        DevicesClass0.push_back(std::unique_ptr<rain3::Simulator>(S));
      if (T == class1) 
        DevicesClass1.push_back(std::unique_ptr<rain3::Simulator>(S));
      if (T == server) 
        Cloud.push_back(std::unique_ptr<rain3::Simulator>(S));
    }

    uint32_t calculateNetworkCost(rain3::Simulator* A, rain3::Simulator* B, uint32_t Size) {   
      uint8_t ClassA = A->getDeviceClass();
      uint8_t ClassB = B->getDeviceClass();

      auto Dist = std::abs(ClassA-ClassB);
      if (Dist == 0) return 9090 + Size * 4;
      else if (Dist == 1 && (ClassA == 0 || ClassB == 0)) return 90900 + Size * 4;
      else if (Dist == 1 && (ClassA == 2 || ClassB == 2)) return 181800 + Size * 4;
      else if (Dist == 2) return 227250 + Size * 4;
    }

    void runAll() {
      ////////////////////////////////////////////////////////////////
      // Clock every machine (one tick is 0,000011001 milleseconds) //
      ///////////////////////////////////////////////////////////////
      for (auto& DeviceSim : DevicesClass0) // Cost should be 80
        DeviceSim->tickClock(); 

      for (auto& DeviceSim : DevicesClass1) // Cost should be 8
        DeviceSim->tickClock();

      for (auto& DeviceSim : Cloud) // Cost should 1
        DeviceSim->tickClock();

      ///////////////////////////////////////////////////////////
      // Run DDBT Manager Protocol by offloading when necessery//
      ///////////////////////////////////////////////////////////
      std::vector<std::tuple<rain3::Simulator*, rain3::Simulator*, rain3::Region*, uint32_t>> NetworkQueueToCompile;
      std::vector<std::tuple<rain3::Simulator*, rain3::Simulator*, rain3::Region*, uint32_t>> NetworkQueueCompiled;

      for (auto& DeviceSim : DevicesClass0) {
        std::pair<Simulator*, Region*> P = DeviceSim->getNextRegionToCompile();
        if (P.second != nullptr) 
          NetworkQueueToCompile.push_back({P.first, DevicesClass1[0].get(), P.second, calculateNetworkCost(DeviceSim.get(), DevicesClass1[0].get(), P.second->getSize())});
      }

      for (auto& T : NetworkQueueToCompile) {
        std::get<3>(T) -= 1;
        if (std::get<3>(T) == 0) 
          std::get<1>(T)->addRegionToCompilationQueue(std::get<0>(T), std::get<2>(T));
      }

    }
  };
}

#endif
