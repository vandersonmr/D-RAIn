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

int main() {

  trace_io::raw_input_pipe_t InstStream("/home/vanderson/dev/mestrado/Rain3/input/out", 201, 203);

  spp::sparse_hash_map<uint64_t, uint32_t> Counter;

  std::vector<trace_io::trace_item_t> Insts;
  std::vector<uint64_t> Touched;

  // Iterate over all the instructions
  do {
    Insts.clear();

    trace_io::trace_item_t I;
    while (InstStream.get_next_instruction(I) && Insts.size() < 1000000) {
      if (I.addr < 0xC0000000)
        Insts.push_back(I);
    }

    for (auto CurrentInst : Insts) {
      if (Counter.count(CurrentInst.addr) != 0) {
        Counter[CurrentInst.addr]++; 
      } else {
        Touched.push_back(CurrentInst.addr);
        Counter[CurrentInst.addr] = 1; 
      }
    }
  } while (Insts.size() != 0);

  std::ofstream HistFile("hist.table");
  for (auto I : Touched) 
    HistFile << I << "\t" << Counter[I] << "\n"; 

	return 0;
}
