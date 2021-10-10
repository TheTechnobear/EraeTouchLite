###############################
# ERAE touch library
project(erae-api)


set(ET_API_SRC
        EraeApi.cpp
        EraeApi.h
        MidiDevice.cpp
        MidiDevice.h
        RtMidiDevice.cpp
        RtMidiDevice.h
        )

include_directories(
        "${PROJECT_SOURCE_DIR}/../external/rtmidi"
        include_directories ("${PROJECT_SOURCE_DIR}/../external/libremidi/include")
)


if(USE_LIBRE_MIDI)
    message("use libre midi")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DLIBREMIDI_HEADER_ONLY=1 -DUSE_LIBRE_MIDI=1")
    if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DLIBREMIDI_COREAUDIO=1")
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DLIBREMIDI_ALSA=1")
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DLIBREMIDI_WMIDI=1")
    endif()
    set(ET_API_SRC ${ET_API_SRC} LibreMidiDevice.h LibreMidiDevice.cpp)
endif()

add_library(erae-api SHARED ${ET_API_SRC})


target_link_libraries(erae-api rtmidi moodycamel)
set_target_properties(erae-api PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS true)
add_subdirectory(tests)

target_include_directories(erae-api PUBLIC .)
