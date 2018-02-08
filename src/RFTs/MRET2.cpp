#include <RFTs.hpp>
#include <iostream>

using namespace rain3;

Region* MRET2::mergePhases() {
  Region* FinalRegion = new Region();

  uint32_t Index = getStoredIndex(Header);

  uint32_t Size1 = RegionsBank[Index]->Instructions.size();
  uint32_t Size2 = RecordingRegion->Instructions.size();

  Region* Smallest = Size1 < Size2 ? RegionsBank[Index] : RecordingRegion;
  Region* Other    = Size1 > Size2 ? RegionsBank[Index] : RecordingRegion;
  for (auto I : Smallest->Instructions) 
    if (Other->hasAddress(I))
      FinalRegion->addAddress(I);

  FinalRegion->setEntry(Header);
  FinalRegion->setMainExit(FinalRegion->Instructions.back());

  delete RecordingRegion;

  return FinalRegion;
}

uint32_t MRET2::getStoredIndex(uint64_t Addr) {
  for (uint32_t I = 0; I < STORE_INDEX_SIZE; I++) 
    if (RegionsBank[I]->isEntry(Addr)) return I;
  return 0;
}

Maybe<Region> MRET2::handleNewInstruction(trace_io::trace_item_t& LastInst, trace_io::trace_item_t& CurrentInst, InternStateTransition LastStateTransition) {
  if (Recording) {
    if ((wasBackwardBranch(LastInst, CurrentInst)      && !Relaxed) ||
        (RecordingRegion->hasAddress(CurrentInst.addr) &&  Relaxed) || 
        RecordingRegion->getSize() > 200 || LastStateTransition == InterToNative)
      Recording = false;

    if (Recording) {
      RecordingRegion->addAddress(CurrentInst.addr);
    } else {
      RecordingRegion->setMainExit(LastInst.addr);
      
      if (Phases[Header] == 1) {
        RegionsBank[BankIndex] = RecordingRegion;
        BankIndex++;
        if (BankIndex == STORE_INDEX_SIZE)  BankIndex = 0;

        Phases[Header] = 2;
      } else {
        return Maybe<Region>(mergePhases());
      }
    }
  } else {
    if ((LastStateTransition == StayedInter && wasBackwardBranch(LastInst, CurrentInst)) || LastStateTransition == NativeToInter) {
      HotnessCounter[CurrentInst.addr] += 1;

      if (isHot(HotnessCounter[CurrentInst.addr])) {
        Recording = true;
        RecordingRegion = new Region();
        RecordingRegion->addAddress(CurrentInst.addr);
        RecordingRegion->setEntry(CurrentInst.addr);
        HotnessCounter[CurrentInst.addr] = 0;
        Header = CurrentInst.addr;

        if (Phases.count(Header) == 0) Phases[Header] = 1;
      }
    }
  }
  return Maybe<Region>::Nothing(); 
}
