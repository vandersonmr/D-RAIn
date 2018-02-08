#ifndef IBHANDLERS_HPP
#define IBHANDLERS_HPP

#include <sparsepp/spp.h>
#include <trace_io.h>
#include <Region.hpp>
#include <array>

namespace rain3 {
  struct IBHandlers {
    virtual bool handleIB(trace_io::trace_item_t&, uint64_t, Region*) = 0;
    virtual void restart() = 0;
  };

  struct PerfectIBHandler : public IBHandlers {
    bool handleIB(trace_io::trace_item_t&, uint64_t, Region*);

    void restart() {
    }
  };

  struct SharedCacheIBHandler : public IBHandlers {
    uint32_t Last = 0;
    std::array<uint64_t, 500> SharedCache; 

    bool handleIB(trace_io::trace_item_t&, uint64_t, Region*);

    void restart() {
      for (uint32_t i = 0; i < SharedCache.size(); i++) SharedCache[i] = 0;
    }
  };

  struct SoleCacheIBHandler : public IBHandlers {
    spp::sparse_hash_map<uint64_t, uint8_t> SoleCacheLast; 
    spp::sparse_hash_map<uint64_t, std::array<uint64_t, 2>> SoleCache; 

    bool handleIB(trace_io::trace_item_t&, uint64_t, Region*);

    void restart() {
      SoleCache.clear();
      SoleCacheLast.clear();
    }
  };

  struct OneIfIBHandler : public IBHandlers {
    spp::sparse_hash_map<uint64_t, uint64_t> LastTarget; 
    spp::sparse_hash_map<uint64_t, uint64_t> OneIfHash; 

    bool handleIB(trace_io::trace_item_t&, uint64_t, Region*);

    void restart() {
      LastTarget.clear();
      OneIfHash.clear();
    }
  };
}

#endif
