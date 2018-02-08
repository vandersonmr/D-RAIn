#include <Region.hpp>
#include <RFTs.hpp>
#include <sparsepp/spp.h>
#include <trace_io.h>
#include <iostream>
#include <fstream>
#include <memory>

namespace rain3 {
  class SimulationStatistics {
    spp::sparse_hash_map<uint64_t, uint32_t> RegionsInterEntryCounter, RegionsNativeEntryCounter, 
      RegionsTotalExecutionFreq, RegionSpannedFreq, CompiledInstFreq; 

    uint32_t NumberOfStats = 0;
    uint32_t TotalNumberOfRegions = 0;
    uint32_t NumberOfCompiledInstructions = 0;
    uint32_t InterTotalExecution = 0;
    uint32_t NativeTotalExecution = 0;
    uint32_t TotalRegionsMainExities = 0;
    uint32_t MaxQueueSize = 0;
    uint32_t TotalIndirectBranches = 0;
    uint32_t TotalMissedIndirectBranches = 0; 
    uint32_t WaitQueueSize = 0;
    uint32_t NumUsedCounters = 0;
  
    uint32_t countAllThat(spp::sparse_hash_map<uint64_t, uint32_t>& Counter, std::function<bool(uint32_t)> F) {
      uint32_t Total = 0;
      for (auto I : Counter) 
        if (F(I.second))
          Total += 1;
      return Total;
    }

    uint32_t sumUpAllCounter(spp::sparse_hash_map<uint64_t, uint32_t>& Counter) {
      uint32_t Total = 0;
      for (auto I : Counter) { 
        Total += I.second;
      }
      return Total;
    }

    void increaseCounter(uint64_t CurrentAddr, spp::sparse_hash_map<uint64_t, uint32_t>& Counter) {
        if (Counter.count(CurrentAddr) == 0) Counter[CurrentAddr] = 1; 
        else Counter[CurrentAddr] += 1;
    }

    public:
    void dumpToFile(std::string Prefix = "", bool FinalStats = true, bool FirstStats = false) {
      std::ofstream OverviewFile(Prefix + "Overview.stats", FirstStats ? std::ios::out : std::ios::app); 

      uint32_t TotalRegionsInterEntry = sumUpAllCounter(RegionsInterEntryCounter);
      uint32_t TotalRegionsNativeEntry = sumUpAllCounter(RegionsNativeEntryCounter);
      uint32_t TotalRegionsEntry = TotalRegionsInterEntry + TotalRegionsNativeEntry;

      double AverageStaticSize = ((double)NumberOfCompiledInstructions)/TotalNumberOfRegions;

      if (FirstStats) {
        OverviewFile << 
          " 80\% Cover Set, " <<
          " Total Number Of Regions," <<
          " Number Of Compiled Instructions," <<
          " Interpreter Total Execution Freq," <<
          " Native Total Execution Freq," <<
          " Regions Coverage," <<
          " Interpreter To Native Total Entries," <<
          " Native To Native Total Entries," <<
          " Average Dynamic Region Size ," <<
          " Average Dynamic Region Size (with static)," <<
          " Average Static Region Size," <<
          " Duplication Ratio," <<
          " Completion Ratio," <<
          " Number of used counters," <<
          " Cool Regions Ratio," <<
          " Spanned Loop Per Region," <<
          " Compilation Wait Queue Size," <<
          " Maximium Reached Queue Size," <<
          " Total Num of Missed IBs," <<
          " Total IBs\n";
      } 
      
      if (FinalStats) {
          
        std::vector<uint32_t> SortedRegFreq;
        for (auto R : RegionsTotalExecutionFreq)
          SortedRegFreq.push_back(R.second);
        std::sort(SortedRegFreq.rbegin(), SortedRegFreq.rend());

        uint32_t Set80 = 0;
        uint32_t AccFreq = 0;
        for (auto F : SortedRegFreq) {
          Set80 += 1;
          AccFreq += F;
          if ((((double)AccFreq) / (InterTotalExecution+NativeTotalExecution)) > 0.8)
            break;
        }
        OverviewFile << Set80 << ", ";
      }

      double AvgDynamicSize   = 0;  
      double AvgDynamicSize2  = 0; 
      double CompletionRatio  = 0; 
      double CoolRegionsRatio = 0;
      double DuplicationRatio = 0;
      double RegionCov        = ((double) NativeTotalExecution) / (NativeTotalExecution+InterTotalExecution);
      double RegionsSpanningLoop = 0;

      if (TotalRegionsEntry != 0) {
        AvgDynamicSize   = ((double) NativeTotalExecution) / TotalRegionsEntry;
        CompletionRatio  = ((double) TotalRegionsMainExities) / TotalRegionsEntry;
        if (AverageStaticSize != 0)
          AvgDynamicSize2  = ((double) NativeTotalExecution) / (TotalRegionsEntry*AverageStaticSize);
      } 

      if (TotalNumberOfRegions != 0) {
        CoolRegionsRatio = ((double) countAllThat(RegionsTotalExecutionFreq, [] (uint32_t x) -> bool { return x < 10000; } )) / TotalNumberOfRegions;
        RegionsSpanningLoop = ((double) sumUpAllCounter(RegionSpannedFreq)) / TotalNumberOfRegions;
        if (NumberOfCompiledInstructions != 0)
          DuplicationRatio = ((double) CompiledInstFreq.size()) / NumberOfCompiledInstructions;
      }

      OverviewFile << TotalNumberOfRegions << ", ";
      OverviewFile << NumberOfCompiledInstructions << ", ";
      OverviewFile << InterTotalExecution << ", ";
      OverviewFile << NativeTotalExecution << ", ";
      OverviewFile << RegionCov << ", ";
      OverviewFile << TotalRegionsInterEntry << ", ";
      OverviewFile << TotalRegionsNativeEntry << ", ";
      OverviewFile << AvgDynamicSize << ", ";
      OverviewFile << AvgDynamicSize2 << ", ";
      OverviewFile << AverageStaticSize << ", ";
      OverviewFile << DuplicationRatio << ", ";
      OverviewFile << CompletionRatio << ", ";
      OverviewFile << NumUsedCounters << ", ";
      OverviewFile << CoolRegionsRatio << ", ";
      OverviewFile << RegionsSpanningLoop << ", ";
      OverviewFile << WaitQueueSize << ", ";
      OverviewFile << MaxQueueSize << ", ";
      OverviewFile << TotalMissedIndirectBranches << ", ";
      OverviewFile << TotalIndirectBranches << "\n";

      OverviewFile.close();
    }

    void missedIndirectAddrsTranslation() {
      TotalMissedIndirectBranches += 1;
    }

    void indirectBranching() {
      TotalIndirectBranches += 1;
    }

    void updateData(std::string Prefix, uint64_t CurrentAddr, uint64_t LastAddr, uint32_t NumOfUsedCounters,
                    InternStateTransition StateTransition, Region* CurrentRegion, Region* LastRegion, uint32_t QS) {

      NumUsedCounters = NumOfUsedCounters;
      WaitQueueSize = QS;

      if (StateTransition == InterToNative) {
        NativeTotalExecution += 1;
        // Count the total number of entries in a region comming from the interpreter
        increaseCounter(CurrentAddr, RegionsInterEntryCounter); 
        increaseCounter(CurrentAddr, RegionsTotalExecutionFreq); 

      } else if (StateTransition == NativeToNative) {
        NativeTotalExecution += 1;
        // Count the total number of entries in a region comming from other regions
        increaseCounter(CurrentAddr, RegionsNativeEntryCounter); 
        increaseCounter(CurrentAddr, RegionsTotalExecutionFreq); 

        if (LastRegion->isMainExit(LastAddr))
          TotalRegionsMainExities += 1;
      } else if (StateTransition == StayedNative) {
        NativeTotalExecution += 1;
        increaseCounter(CurrentRegion->getEntry(), RegionsTotalExecutionFreq); 

        if (CurrentRegion->hasAddress(CurrentAddr) && CurrentAddr < LastAddr) 
          increaseCounter(CurrentAddr, RegionSpannedFreq); 
      } else if (StateTransition == NativeToInter) {
        InterTotalExecution += 1;

        if (LastRegion->isMainExit(LastAddr)) 
          TotalRegionsMainExities += 1;
      } else if (StateTransition == StayedInter) {
        InterTotalExecution += 1;
      }

//      if (((InterTotalExecution + NativeTotalExecution) % 10000) == 0) 
//        dumpToFile(Prefix, false, NumberOfStats++ ? false : true);
    }

    void addRegionInfo(Region* R, uint32_t RegionWaitQueueSize) {
      TotalNumberOfRegions += 1;
      NumberOfCompiledInstructions += R->getSize();
      WaitQueueSize = RegionWaitQueueSize;

      for (auto I : R->Instructions)
        increaseCounter(I, CompiledInstFreq);

			if (RegionWaitQueueSize > MaxQueueSize) 
				MaxQueueSize = RegionWaitQueueSize;
    }
  };
}
