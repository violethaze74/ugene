set(UGENE_PLUGIN_NAME biostruct3d_view)

file(GLOB SRCS src/*.cpp src/*.c src/*.h src/gl2ps/*)

include(../../Plugin.cmake)

target_link_libraries(biostruct3d_view "-framework OpenGL")
