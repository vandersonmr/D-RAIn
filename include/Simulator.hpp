#ifndef SIMULATOR_HPP
#define SIMULATOR_HPP

#include <Region.hpp>
#include <IBHandlers.hpp>
#include <Policies.hpp>
#include <SimulationStatistics.hpp>
#include <sparsepp/spp.h>
#include <trace_io.h>

#include <memory>

namespace rain3 {
  class QueuePolicies;

  typedef std::function<Maybe<Region>(trace_io::trace_item_t&, trace_io::trace_item_t&, InternStateTransition)> RFTHandler;

  class Simulator {
    enum State { Interpreting, NativeExecuting };

    std::string FilePrefix;

    spp::sparse_hash_map<uint64_t, std::unique_ptr<Region>> RegionsCache; 
    std::vector<std::pair<rain3::Simulator*, rain3::Region*>> RegionWaitQueue;
    std::vector<std::pair<rain3::Simulator*, rain3::Region*>> OffloadCompiledQueue;

    State   CurrentState  = State::Interpreting; 
    Region *CurrentRegion = nullptr;
    Region* LastRegion = nullptr;
    trace_io::trace_item_t LastInst;

    IBHandlers* IBH;
    QueuePolicies* QP;
    RFTs* RFT;
    std::vector<trace_io::trace_item_t> InputStream; 

    uint32_t VPC = 0;
    uint32_t InterpretationCost = 5;
    uint8_t DeviceClass = 0;

    SimulationStatistics Statistics;

    Region* getRegionByEntry(uint64_t Addrs) { return RegionsCache[Addrs].get(); }

    bool isWaitingCompile(uint64_t Addrs) {
      for (auto R : RegionWaitQueue)
        if (R.second->getEntry() == Addrs) return true;
      return false; 
    }

    bool isRegionEntrance(uint64_t Addrs) { return RegionsCache.count(Addrs) != 0; }

    InternStateTransition updateInternState(uint64_t, bool);

    public:
    Simulator(std::string TracePath, std::string Prefix, IBHandlers* IB, QueuePolicies* Q, RFTs* R, int Class, uint32_t IC) {
      QP = Q;
      IBH = IB;
      RFT = R;
      FilePrefix = Prefix;
      DeviceClass = Class;
      InterpretationCost = IC;

      // Just load if something is going to run on it, so only for class 0 and 1
      if (DeviceClass < 2) {
        trace_io::raw_input_pipe_t InstStream(TracePath.c_str(), 0, 0);

        trace_io::trace_item_t I;
        while (InstStream.get_next_instruction(I))  
          InputStream.push_back(I);
      }
    }

    void configureRFT(auto SC) { RFT->configure(SC, &RegionsCache); }

    std::pair<rain3::Simulator*, rain3::Region*> getNextRegionToCompile() { 
      if (RegionWaitQueue.size() == 0) return {nullptr, nullptr};
      else {
        auto R = RegionWaitQueue.front();
        RegionWaitQueue.erase(RegionWaitQueue.begin());
        return R;
      }
    }

    void addRegionToCompilationQueue(rain3::Simulator* Target, rain3::Region* R) {
      RegionWaitQueue.push_back({Target, R});
    }

    void addRegion(Region* R) { RegionsCache[R->getEntry()].reset(R); }

    ~Simulator() {
      Statistics.dumpToFile(FilePrefix + "final", true, true);
    }

    uint8_t getDeviceClass() { return DeviceClass; }

    bool tickClock();
  };
}
#endif
