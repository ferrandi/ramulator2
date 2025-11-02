add_library(ramulator-addrmapper OBJECT)

target_sources(
  ramulator-addrmapper PRIVATE
  addr_mapper.h 

  impl/linear_mappers.cpp
)

target_link_libraries(
  ramulator
  PRIVATE
  ramulator-addrmapper
)