project(flowsolver CXX)
cmake_minimum_required(VERSION 3.4)
find_package(Z3 REQUIRED CONFIG)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
  add_compile_options(/W4 /WX)
else()
  add_compile_options(-Wall -Wextra -pedantic -Werror)
endif()

add_executable(flowsolver src/main.cpp)
target_include_directories(flowsolver PRIVATE ${Z3_CXX_INCLUDE_DIRS})
target_link_libraries(flowsolver PRIVATE ${Z3_LIBRARIES})

if (CMAKE_SYSTEM_NAME MATCHES "[Ww]indows")
  # On Windows we need to copy the Z3 libraries
  # into the same directory as the executable
  # so that they can be found.
  foreach (z3_lib ${Z3_LIBRARIES})
    add_custom_command(TARGET flowsolver
      POST_BUILD
      COMMAND
        ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:${z3_lib}>
        $<TARGET_FILE_DIR:flowsolver>
    )
  endforeach()
endif()
