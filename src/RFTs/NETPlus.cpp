#include <RFTs.hpp>

#include <queue>

using namespace rain3;

bool isRet(const char* cur_opcode) {
	int opcode = (int) (unsigned char) cur_opcode[0];
	return (opcode == 0xc3 || opcode == 0xcb);
}

bool isCall(const char* cur_opcode) {
	int opcode = (int) (unsigned char) cur_opcode[0];
	return (opcode == 0xe8);
}

void NETPlus::expand(Region* R) {
	std::queue<uint64_t> s;
	std::unordered_map<uint64_t, uint32_t> distance;
	std::unordered_map<uint64_t, uint64_t> next, parent;
	std::unordered_map<uint64_t, uint64_t> last_call;

	// Init BFS frontier
	for (auto Addrs : R->Instructions) {
		if (StaticCode->getTraceItem(Addrs).is_flow_control_inst()) {
			s.push(Addrs);
			distance[Addrs] = 0;
			parent[Addrs] = 0;
		}
	}

	uint32_t numberOfInlines = 0;
	while (!s.empty()) {
		unsigned long long current = s.front();
		s.pop();

		if (distance[current] < DepthLimit) {

			auto targets = StaticCode->getTraceItem(current).getPossibleNextAddrs();

			if (isRet(StaticCode->getOpcode(current))) 
				if (last_call.count(current) != 0)
					targets.push_back(last_call[current]);

			for (auto target : targets) {
				if (parent.count(target) != 0) continue;

				parent[target] = current;
				// Iterate over all instructions between the target and the next branch
				auto it = StaticCode->find(target);

				while (it != StaticCode->getEnd()) {
					bool isCycle = (R->isEntry(it->first) && !Extended) || (R->hasAddress(it->first) && Extended);

					if ((isCycle) && distance[current] > 0) {
						std::vector<uint64_t> newpath;
						unsigned long long begin = it->first;
						unsigned long long prev = target;
						while (true) {
							auto it = StaticCode->find(begin);
							while (true) {
								newpath.push_back(it->first);
								if (it->first == prev) break;
								--it;
							}
							begin = parent[prev];
							prev  = next[begin];
							if (prev == 0) {
								newpath.push_back(begin);
								break;
							}
						}

						for (auto NewAddrs : newpath) 
							if (!R->hasAddress(NewAddrs))
								R->addAddress(NewAddrs);

						break;
					}

					if (RegionsCache->count(it->first) != 0)
						break;

					if (StaticCode->getTraceItem(it->first).is_flow_control_inst() && distance.count(it->first) == 0) {
						s.push(it->first);

						if (last_call.count(current) != 0)
							last_call[it->first] = last_call[current]; 

						if (numberOfInlines < 20 && StaticCode->hasInstruction(it->first) && isCall(StaticCode->getOpcode(it->first))) {
							numberOfInlines++;
							last_call[it->first] = it->first + 5;
							distance[it->first] = 1;
						} else {
							distance[it->first] = distance[current] + 1;
						}

						next[it->first] = target;
						break;
					}

					++it;
				}
			}
		}
	}
}

Maybe<Region> NETPlus::handleNewInstruction(trace_io::trace_item_t& LastInst, trace_io::trace_item_t& CurrentInst, InternStateTransition LastStateTransition) {
	if (Recording) {
		if ((wasBackwardBranch(LastInst, CurrentInst)      && !Relaxed) ||
				(RecordingRegion->hasAddress(CurrentInst.addr) &&  Relaxed) || 
				RecordingRegion->getSize() > 200 || LastStateTransition == InterToNative)
			Recording = false;

		if (Recording) {
			RecordingRegion->addAddress(CurrentInst.addr);
		} else {
			RecordingRegion->setMainExit(LastInst.addr);
			expand(RecordingRegion);
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
