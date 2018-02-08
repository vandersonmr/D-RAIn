cmake_minimum_required (VERSION 2.6)

add_subdirectory(arglib)
add_subdirectory(tracelib)

add_library(rainlib 
  Simulator.cpp 
  RFTs/NET.cpp 
  RFTs/LEI.cpp 
  RFTs/MRET2.cpp 
  RFTs/RFTsCommon.cpp 
  QueuePolicies/Policies.cpp 
  IBHandlers/IBHandlers.cpp)
