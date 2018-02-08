#ifndef RFTS_HPP
#define RFTS_HPP
#include <sparsepp/spp.h>

#include <Region.hpp>

namespace rain3 {
  class RFTs {
    protected:
      spp::sparse_hash_map<uint64_t, uint16_t> HotnessCounter;
      bool Recording = false;
      Region *RecordingRegion = nullptr;
      uint16_t HotnessThreshold;

      bool isHot(uint16_t); 
      bool wasBackwardBranch(trace_io::trace_item_t&, trace_io::trace_item_t&); 

      InstructionSet* StaticCode;
      spp::sparse_hash_map<uint64_t, std::unique_ptr<Region>>* RegionsCache;

    public:
      void configure(auto SC, auto RC) { StaticCode = SC; RegionsCache = RC; }

      uint32_t getNumUsedCounters() { return HotnessCounter.size(); };

      uint16_t getHotnessThreshold() { return HotnessThreshold; }
      void setHotnessThreshold(uint16_t HT) { HotnessThreshold = HT; }

      RFTs(uint16_t HT) : HotnessThreshold(HT) {};

      virtual Maybe<Region> handleNewInstruction(trace_io::trace_item_t&, trace_io::trace_item_t&, InternStateTransition) = 0;
      void restart();
  };

  class NET : public RFTs {
    private:
      bool Relaxed = false;
    public:
      NET(uint16_t HT, bool Rel = false) : RFTs(HT), Relaxed(Rel) {};
      Maybe<Region> handleNewInstruction(trace_io::trace_item_t&, trace_io::trace_item_t&, InternStateTransition);
  };

  class NETPlus : public RFTs {
    private:
      bool Relaxed = false;
      bool Extended = false;
      uint16_t DepthLimit = 10;

			void expand(rain3::Region*);

    public:
      NETPlus(uint16_t HT, bool Rel = false, bool Ext = false, uint16_t DL = 10) : RFTs(HT), Relaxed(Rel), 
        Extended(Ext),  DepthLimit(DL) {};

      Maybe<Region> handleNewInstruction(trace_io::trace_item_t&, trace_io::trace_item_t&, InternStateTransition);
  };

  class MRET2 : public RFTs {
    private:
      static constexpr uint32_t STORE_INDEX_SIZE = 100000;

      bool Relaxed = false;

      uint64_t Header;
      spp::sparse_hash_map<uint64_t, uint8_t> Phases;

      uint32_t BankIndex;
      Region* RegionsBank[STORE_INDEX_SIZE];

      Region* mergePhases();
      uint32_t getStoredIndex(uint64_t);
    public:
      MRET2(uint16_t HT, bool Rel = false) : RFTs(HT/2), Relaxed(Rel) {};
      Maybe<Region> handleNewInstruction(trace_io::trace_item_t&, trace_io::trace_item_t&, InternStateTransition);
  };

  class LEI : public RFTs {
    private:
      bool Relaxed = false;

      struct branch_t {
        uint64_t src, tgt;
        InternStateTransition state;
      };

      std::vector<branch_t> Buf;
      spp::sparse_hash_map<uint64_t, int32_t> BufHash;

      uint32_t circularBufferInsert(uint64_t, uint64_t, InternStateTransition);
      Region* formTrace(uint64_t, uint32_t);
    public:
      LEI(uint16_t HT, bool Rel = false) : RFTs(HT), Relaxed(Rel) {};
      Maybe<Region> handleNewInstruction(trace_io::trace_item_t&, trace_io::trace_item_t&, InternStateTransition);
  };

}
#endif
