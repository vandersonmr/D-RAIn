#include <IBHandlers.hpp>
#include <Region.hpp>

using namespace rain3;


bool PerfectIBHandler::handleIB(trace_io::trace_item_t&, uint64_t, Region*) {
  return true;
}

bool OneIfIBHandler::handleIB(trace_io::trace_item_t& IB, uint64_t RealTarget, Region* R) {
  if (R == nullptr) {
    LastTarget[IB.addr] = RealTarget;
    return true; 
  } else {
    if (OneIfHash.count(IB.addr + R->getEntry()) == 0) 
      OneIfHash[IB.addr + R->getEntry()] = LastTarget[IB.addr]; 
    return OneIfHash[IB.addr + R->getEntry()] == RealTarget;
  }
}

bool SharedCacheIBHandler::handleIB(trace_io::trace_item_t& IB, uint64_t RealTarget, Region* R) {
  bool isInCache = false;
  for (uint64_t addr : SharedCache) {
    if (addr == RealTarget) {
      isInCache = true;
      break;
    }
  }

  if (!isInCache && R != nullptr) {
    SharedCache[Last] = RealTarget;
    Last++;
    if (Last > SharedCache.size()) Last = 0;
  }

  return isInCache;
}

bool SoleCacheIBHandler::handleIB(trace_io::trace_item_t& IB, uint64_t RealTarget, Region* R) {
  if (SoleCache.count(IB.addr) == 0) {
    SoleCache[IB.addr];
    SoleCacheLast[IB.addr] = 0;
  }

  bool isInCache = false;
  for (uint64_t addr : SoleCache[IB.addr]) {
    if (addr == RealTarget) {
      isInCache = true;
      break;
    }
  }

  if (!isInCache && R != nullptr) {
    SoleCache[IB.addr][SoleCacheLast[IB.addr]] = RealTarget;
    SoleCacheLast[IB.addr] += 1;
    if (SoleCacheLast[IB.addr] > SoleCache[IB.addr].size()) SoleCacheLast[IB.addr] = 0;
  }

  return isInCache;
}
