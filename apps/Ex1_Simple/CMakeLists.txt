get_filename_component(FOLDERNAME ${CMAKE_CURRENT_LIST_DIR} NAME)

set(SOURCES main.cpp)

add_executable(${FOLDERNAME} ${SOURCES})

target_link_libraries(${FOLDERNAME} ${COMMON_LIBS})
