project(flowsolver CXX)
cmake_minimum_required(VERSION 3.15)
find_package(Z3 REQUIRED CONFIG)
find_package(raylib REQUIRED)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
  add_compile_options(/W4 /WX)
else()
  add_compile_options(-Wall -Wextra -pedantic -Werror)
endif()

add_executable(flowsolver src/main.cpp)
target_include_directories(flowsolver PRIVATE ${Z3_CXX_INCLUDE_DIRS})
target_include_directories(flowsolver PRIVATE ${RAYLIB_INCLUDE_DIRS})
target_link_libraries(flowsolver PRIVATE ${Z3_LIBRARIES})
target_link_libraries(flowsolver PRIVATE ${RAYLIB_LIBRARIES})

if(MSVC)
  target_link_options(flowsolver PRIVATE /LTCG)
  target_link_libraries(flowsolver PRIVATE winmm.lib)
endif()

if (CMAKE_SYSTEM_NAME MATCHES "[Ww]indows")
  # On Windows we need to copy libraries
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
  foreach (raylib_lib ${RAYLIB_LIBRARIES})
    add_custom_command(TARGET flowsolver
      POST_BUILD
      COMMAND
        ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:${raylib_lib}>
        $<TARGET_FILE_DIR:flowsolver>
    )
  endforeach()
endif()
