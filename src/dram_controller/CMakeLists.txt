add_library(ramulator-controller OBJECT)

target_sources(
  ramulator-controller PRIVATE
  controller.h 
  scheduler.h 
  refresh.h
  rowpolicy.h

  impl/generic_dram_controller.cpp
  
  impl/scheduler/generic_scheduler.cpp

  impl/refresh/all_bank_refresh.cpp
  
  impl/rowpolicy/basic_rowpolicies.cpp
)

target_link_libraries(
  ramulator
  PRIVATE
  ramulator-controller
)