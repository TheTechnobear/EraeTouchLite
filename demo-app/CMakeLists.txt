project(demo-app)

set(APP_SRC
        demo_app.h
        demo_app.cpp
        )

include_directories(
        "${PROJECT_SOURCE_DIR}/../erae-api"
        #     "${PROJECT_SOURCE_DIR}/../external/oscpack"
        #     "${PROJECT_SOURCE_DIR}/../external/portaudio"
        #     "${PROJECT_SOURCE_DIR}/../external/cJSON"
)

add_executable(demo-app ${APP_SRC})

#target_link_libraries(demo-app erae-api oscpack rtmidi)
target_link_libraries(demo-app erae-api rtmidi)

if (UNIX AND NOT APPLE)
    target_link_libraries(demo-app pthread)
endif ()

if (APPLE)
    target_link_libraries(demo-app "-framework CoreMIDI")
endif (APPLE)
