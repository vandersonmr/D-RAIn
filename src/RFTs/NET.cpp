#include <RFTs.hpp>

using namespace rain3;

Maybe<Region> NET::handleNewInstruction(trace_io::trace_item_t& LastInst, trace_io::trace_item_t& CurrentInst, InternStateTransition LastStateTransition) {
  if (Recording) {
    if ((wasBackwardBranch(LastInst, CurrentInst)      && !Relaxed) ||
        (RecordingRegion->hasAddress(CurrentInst.addr) &&  Relaxed) || 
        RecordingRegion->getSize() > 200 || LastStateTransition == InterToNative)
      Recording = false;

    if (Recording) {
      RecordingRegion->addAddress(CurrentInst.addr);
    } else {
      RecordingRegion->setMainExit(LastInst.addr);
      return Maybe<Region>(RecordingRegion);
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
      }
    }
  }
  return Maybe<Region>::Nothing(); 
}
