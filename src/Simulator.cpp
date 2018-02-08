#include <Simulator.hpp>
#include <functional>

#include <iostream>

using namespace rain3;

//#define CongestionControl

InternStateTransition Simulator::updateInternState(uint64_t NextAddrs, bool ForceNativeExiting) {
  // Entrying Region: Interpreter -> NativeExecuting
  if (CurrentState == State::Interpreting && isRegionEntrance(NextAddrs) && !ForceNativeExiting) {
    CurrentState  = State::NativeExecuting;
    CurrentRegion = getRegionByEntry(NextAddrs); 
    return InternStateTransition::InterToNative;
  } 
  // Exiting Region: NativeExecution -> Dispatcher
  else if (CurrentState == State::NativeExecuting && (!CurrentRegion->hasAddress(NextAddrs) || ForceNativeExiting)) {
    // Region Transition: NativeExecution (Dispatcher) -> NativeExecution
    if (isRegionEntrance(NextAddrs)) {
      LastRegion    = CurrentRegion;
      CurrentRegion = getRegionByEntry(NextAddrs);
      return InternStateTransition::NativeToNative;
    } 
    // Exiting Native Execution: NativeExecution (Dispatcher) -> Interpreter
    else {
      CurrentState  = State::Interpreting;
      LastRegion    = CurrentRegion;
      CurrentRegion = nullptr;
      return InternStateTransition::NativeToInter;
    }
  } else {
    if (CurrentState == State::Interpreting) return InternStateTransition::StayedInter;
    else return InternStateTransition::StayedNative;
  }
}

uint32_t InternClock = 0;
bool Simulator::tickClock() {
  if (InternClock == 0) {
    InternClock = InterpretationCost;

    trace_io::trace_item_t CurrentInst;
    InternStateTransition LastStateTransition;
    ////////////////////////////////////////////////////
    // Interpretation: for devices from class 0 and 1 //
    ///////////////////////////////////////////////////
    if (DeviceClass < 2) {
      CurrentInst = InputStream[VPC++];

      if (VPC == InputStream.size()) {
        VPC = 0;  
      }

      // Simulate Indirect Branch Handlers
      bool ForceNativeExiting = false;

      LastStateTransition = updateInternState(CurrentInst.addr, ForceNativeExiting);

      if (CurrentState == Interpreting && isWaitingCompile(CurrentInst.addr)) {
        for (auto R : RegionWaitQueue) { 
          if (R.second->getEntry() == CurrentInst.addr) {
            R.second->increaseWeight();
            break;
          }
        }
      }

      // Only handle when not in native execution
      if (CurrentState == Interpreting && !isWaitingCompile(CurrentInst.addr)) {
        Maybe<Region> MayRegion = RFT->handleNewInstruction(LastInst, CurrentInst, LastStateTransition);

        // If a region was indeed created, then call the region queue handler and update the statistics
        if (!MayRegion.isNothing()) { 
          addRegionToCompilationQueue(this, MayRegion.get());	
          Statistics.addRegionInfo(MayRegion.get(), RegionWaitQueue.size());

#ifdef CongestionControl      
          double Ratio = ((double)RegionWaitQueue.size()) / QP->NumThreads;
          if (Ratio < 0.5) 
            RFT->setHotnessThreshold((RFT->getHotnessThreshold()/2)+1); 
          else if (Ratio < 1)
            RFT->setHotnessThreshold(RFT->getHotnessThreshold()*0.9 + 1); 
          else if (Ratio > 4)
            RFT->setHotnessThreshold(RFT->getHotnessThreshold()*2); 
          else if (Ratio > 2)
            RFT->setHotnessThreshold(RFT->getHotnessThreshold()*1.1); 
#endif
        }
      }
    }

    ////////////////////////////////////////////////////
    // Compilation: for devices from classes 1 and 2 //
    ///////////////////////////////////////////////////
    if (DeviceClass >= 1) {
      std::vector<std::pair<Simulator*, Region*>> Compiled = QP->handleWaitQueueParallel(RegionWaitQueue);

      for (auto P : Compiled) { 
        if (P.first == this) {
          Region* R = P.second;
          addRegion(R);

          if (R->getEntry() == CurrentInst.addr) {
            LastRegion = nullptr;
            LastStateTransition = updateInternState(CurrentInst.addr, false);
          }
        } else {
          OffloadCompiledQueue.push_back(P);
        }
      }
    }

    if (DeviceClass < 2) {
      Statistics.updateData(FilePrefix, CurrentInst.addr, LastInst.addr, RFT->getNumUsedCounters(), LastStateTransition, 
          CurrentRegion, LastRegion, RegionWaitQueue.size());

      LastInst   = CurrentInst;
      LastRegion = CurrentRegion;
    }
  }
  InternClock--;
  return true;
}
