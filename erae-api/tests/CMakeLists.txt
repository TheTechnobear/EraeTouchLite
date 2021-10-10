
include_directories ("${PROJECT_SOURCE_DIR}")

# et_1
add_executable(et_1 "et_1.cpp")
target_link_libraries (et_1 erae-api)

if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
  target_link_libraries(et_1 atomic)
endif()
if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    target_link_libraries(et_1  "-framework CoreServices -framework CoreFoundation -framework IOKit -framework CoreAudio")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_link_libraries(et_1 pthread)
endif()




