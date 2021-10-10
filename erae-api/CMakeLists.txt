###############################
# ERAE touch library
project(erae-api)


set(ET_API_SRC
        erae_api.cpp
        erae_api.h
        )

include_directories(
        .
)

add_library(erae-api SHARED ${ET_API_SRC})


# target_link_libraries(erae-api cjson oscpack rtmidi portaudio moodycamel)
set_target_properties(erae-api PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS true)
add_subdirectory(tests)

target_include_directories(erae-api PUBLIC .)
