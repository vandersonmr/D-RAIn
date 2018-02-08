/***************************************************************************
 *   Copyright (C) 2017 by:                                                *
 *   Vanderson M. Rosario (vandersonmr2@gmail.com)                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <arglib.h>
#include <trace_io.h>
#include <sparsepp/spp.h>
#include <fstream>
#include <cmath>
#include <string>

#define addr_limit 0xF9CCD8A1C5080000 //0xB2D05E00

int main(int argc, char** argv) {
  if (argc < 4) return 1;
  trace_io::raw_input_pipe_t InstStream(std::string(argv[1]).c_str(), atoi(argv[2]), atoi(argv[3]));
  trace_io::raw_input_pipe_t InstStream2(std::string(argv[1]).c_str(), atoi(argv[2]), atoi(argv[3]));
  trace_io::raw_input_pipe_t InstStream3(std::string(argv[1]).c_str(), atoi(argv[2]), atoi(argv[3]));

  spp::sparse_hash_map<uint64_t, uint64_t> Counter;
  spp::sparse_hash_map<uint64_t, bool> Sources;
  spp::sparse_hash_map<uint64_t, bool> Targets;
  spp::sparse_hash_map<uint64_t, uint64_t> BBs;
  spp::sparse_hash_map<uint64_t, uint64_t> BBsEnd;
  spp::sparse_hash_map<uint64_t, uint64_t> BBsSize;

  std::vector<trace_io::trace_item_t> Insts;
  std::vector<uint64_t> Touched;
  spp::sparse_hash_map<std::string, std::pair<uint64_t, uint64_t>> Edges;
  spp::sparse_hash_map<std::string, uint64_t> EdgesCounter;
  
  trace_io::trace_item_t first;
  InstStream.get_next_instruction(first);
  uint64_t lastAddr = first.addr;
  uint32_t lastSize = first.length;
  uint64_t Total = 0;

  // Iterate over all the instructions
  do {
    Insts.clear();

    trace_io::trace_item_t I;
    while (InstStream.get_next_instruction(I) && Insts.size() < 1000000) {
      if (I.addr < addr_limit) { 
        Insts.push_back(I);
        if (lastAddr + lastSize != I.addr) {
          Sources[lastAddr] = true;
          Targets[I.addr] = true;
        }
        lastAddr = I.addr;
        lastSize = I.length;
      } 
    }

    for (auto CurrentInst : Insts) {
      Total++;
      if (Counter.count(CurrentInst.addr) != 0) {
        Counter[CurrentInst.addr]++; 
      } else {
        Touched.push_back(CurrentInst.addr);
        Counter[CurrentInst.addr] = 1; 
      }
    }
  } while (Insts.size() != 0  && Total < 2500000000);

  /* Construct the Edges */
  lastAddr = first.addr;
  lastSize = first.length;
  do {
    Insts.clear();

    trace_io::trace_item_t I;
    while (InstStream2.get_next_instruction(I) && Insts.size() < 1000000)
      if (I.addr < addr_limit)
        Insts.push_back(I);

    for (auto CurrentInst : Insts) {
      if (Sources.count(lastAddr) != 0 || Targets.count(CurrentInst.addr) != 0) {
        if (Edges.count(std::to_string(lastAddr) + std::to_string(CurrentInst.addr)) == 0) {
          Edges[std::to_string(lastAddr) + std::to_string(CurrentInst.addr)] = {lastAddr, CurrentInst.addr};
          EdgesCounter[std::to_string(lastAddr) + std::to_string(CurrentInst.addr)] = 1;
        } else {
          EdgesCounter[std::to_string(lastAddr) + std::to_string(CurrentInst.addr)] += 1;
        }
        Sources[lastAddr] = true;
        Targets[CurrentInst.addr] = true;
      } 

      lastAddr = CurrentInst.addr;
      lastSize = CurrentInst.length;
    }
  } while (Insts.size() != 0  && Total < 2500000000);

  /* Construct the BBs */
  std::vector<uint64_t> BBsOrder;
  spp::sparse_hash_map<uint64_t, uint64_t> Snapshot;

  uint64_t CurrentBB = first.addr;
  uint64_t CurrentFreq = 0;
  uint64_t NumInst = 1;
  uint64_t TotalBBEntries = 0;
  uint64_t LastI = first.addr;
  for (auto I : Touched) { 
    if (Targets.count(I) != 0 || Sources.count(LastI) != 0) {
      BBs[CurrentBB] = CurrentFreq;
      BBsEnd[LastI] = CurrentBB;
      BBsSize[CurrentBB] = NumInst;
      Snapshot[CurrentBB] = 0;
      BBsOrder.push_back(CurrentBB);

      TotalBBEntries += CurrentFreq * NumInst;
      CurrentBB = I;
      CurrentFreq = 0;
      NumInst = 0;
    }

    CurrentFreq = Counter[I];
    NumInst += 1;
    LastI = I;
  }
  BBs[CurrentBB] = CurrentFreq;
  BBsSize[CurrentBB] = NumInst;
  BBsEnd[LastI] = CurrentBB;
  TotalBBEntries += CurrentFreq * NumInst;

  
  std::ofstream SnapFile((std::string(argv[1]) + std::string("snapshots.table")).c_str());
  for (uint64_t BBEntry : BBsOrder)
    SnapFile << BBEntry << "\t";
  SnapFile << "\n";
  int i;
  do {
    i = 0;
    trace_io::trace_item_t I;
    while (InstStream3.get_next_instruction(I) && i < 2500000)
      if (I.addr < addr_limit) {
        i++;
        if (Snapshot.count(I.addr) != 0)
          Snapshot[I.addr] += 1;
      }

    for (uint64_t BBEntry : BBsOrder)
      SnapFile << Snapshot[BBEntry] << "\t";
    SnapFile << "\n";

    for (auto S : Snapshot) 
      Snapshot[S.first] = 0;
  } while (i != 0  && Total < 2500000000);

  /* PRINT BBs and Edges */
  std::ofstream HistFile((std::string(argv[1]) + std::string("bbs.table")).c_str());
  double Entropy = 0;
  uint64_t TotalFreq = 0;
  for (auto I : Touched) {
    if (BBs.count(I) != 0) {
      HistFile << I << "\t" << BBs[I] << "\t" << BBsSize[I] <<  "\n"; 
      double pOfBB = ((double) BBs[I]) / TotalBBEntries;
      Entropy += pOfBB * log2(pOfBB);
    }
    TotalFreq += Counter[I];
  }

  std::ofstream EdgesFile((std::string(argv[1]) + std::string("edges.table")).c_str());
  for (auto E : Edges) {
    EdgesFile << BBsEnd[E.second.first] << "\t" << E.second.second << "\t" << EdgesCounter[E.first] << "\n";
  }
  
  std::cout << TotalFreq << " / " << TotalBBEntries << " Entropy: " << -Entropy << " / " << (-1)*(BBs.size()*((1/(double)BBs.size())*log2(1/((double)BBs.size())))) << "\n";

	return 0;
}
