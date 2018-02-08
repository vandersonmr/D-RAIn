#include <RFTs.hpp>
#include <cstdlib>
#include <iostream>

using namespace rain3;

Region* LEI::formTrace(uint64_t start, uint32_t old) {
  RecordingRegion = new Region();
  uint64_t prev = start;
  uint64_t branch = old + 1;

  RecordingRegion->setEntry(start);
  while (branch < Buf.size()) {
    uint64_t branch_src = Buf[branch].src;
    uint64_t branch_tgt = Buf[branch].tgt;

    auto it = StaticCode->find(prev);

    while (it != StaticCode->getEnd()) {
      // Stop if next instruction begins a trace
      if (RegionsCache->count(it->first) != 0) 
        break;

      if (prev >= branch_src) 
        break;

      if (!RecordingRegion->hasAddress(it->first))
        RecordingRegion->addAddress(it->first);

      if (it->first == branch_src)
        break;
      ++it;
    }

    // Stop if branch forms a cycle
    if (RecordingRegion->hasAddress(branch_tgt))  
      break;

    prev = branch_tgt;
    branch++;
  }
  return RecordingRegion;
}

uint32_t LEI::circularBufferInsert(uint64_t src, uint64_t tgt, InternStateTransition LastState) {
  if (Buf.size() > 1000000) {
    Buf.clear();
    BufHash.clear();
  }
  Buf.push_back({src, tgt, LastState});
  return Buf.size()-1;
}

Maybe<Region> LEI::handleNewInstruction(trace_io::trace_item_t& LastInst, trace_io::trace_item_t& CurrentInst, InternStateTransition LastStateTransition) {
  if ((LastStateTransition == NativeToInter || LastStateTransition == StayedInter) && std::abs((int64_t) (LastInst.addr - CurrentInst.addr)) > LastInst.length) {
    uint64_t src = LastInst.addr;
    uint64_t tgt = CurrentInst.addr;

    uint32_t LastUsedIndex = circularBufferInsert(src, tgt, LastStateTransition);

    if (BufHash.count(tgt) != 0) {
      uint32_t old = BufHash[tgt];
      BufHash[tgt] = LastUsedIndex;

      // if tgt ≤ src or old follows exit from code cache
      if (tgt <= src || Buf[old].state == NativeToInter) {
        // increment counter c associated with tgt
        HotnessCounter[tgt] += 1;

        // if c = Tcyc
        if (isHot(HotnessCounter[tgt])) {
          RecordingRegion = formTrace(tgt, old);

          // remove all elements of Buf after old
          for (uint32_t i = old+1; i < Buf.size(); i++) 
            BufHash.erase(Buf[i].tgt);
          Buf.erase(Buf.begin()+old+1, Buf.end());

          HotnessCounter[tgt] = 0;

          if (RecordingRegion->getSize() > 0)
            return Maybe<Region>(RecordingRegion);
        }
      }
    } else {
      BufHash[tgt] = LastUsedIndex;
    }
  }
  return Maybe<Region>::Nothing(); 
}
