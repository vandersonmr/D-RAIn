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

#include <Simulator.hpp>
#include <RFTs.hpp>
#include <Policies.hpp>
#include <IBHandlers.hpp>
#include <Region.hpp>
#include <Global.hpp>

int main(int argv, char** argc) {
  rain3::GlobalContext GC;

  auto Sim = new rain3::Simulator(argc[1], std::string(argc[1])+"-NET-HT?"+std::to_string(1024)+"-Relaxed?"+std::to_string(true), 
      new rain3::PerfectIBHandler, new rain3::QueuePolicies(1, 0), new rain3::NET(1024, true), 0, 80);

  GC.insertDevice(Sim , rain3::GlobalContext::DeviceType::class0);

  return 0;
}
