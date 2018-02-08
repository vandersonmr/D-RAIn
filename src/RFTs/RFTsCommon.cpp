#include <RFTs.hpp>


bool rain3::RFTs::isHot(uint16_t Hotness) {
  return Hotness > HotnessThreshold;
}

bool rain3::RFTs::wasBackwardBranch(trace_io::trace_item_t& LastInst, trace_io::trace_item_t& CurrentInst) {
  return LastInst.addr > CurrentInst.addr;
}

void rain3::RFTs::restart() {
  HotnessCounter.clear();
}
