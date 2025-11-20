add_library(ramulator-frontend OBJECT)

target_sources(
  ramulator-frontend PRIVATE
  frontend.h
  
  impl/my_wrapper/my_wrapper.cpp
)

target_link_libraries(
  ramulator
  PRIVATE
  ramulator-frontend
)