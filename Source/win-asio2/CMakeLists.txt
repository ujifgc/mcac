project(win-asio2)

if(MSVC)
	set(image-source_PLATFORM_DEPS
		w32-pthreads)
endif()

set(win-asio2_SOURCES
	win-asio2.cpp
	asio/asio.cpp
	asio/asiolist.cpp
	plugin-main.cpp)

add_library(win-asio2 MODULE
	${win-asio2_SOURCES})
target_link_libraries(win-asio2
	libobs
	${image-source_PLATFORM_DEPS})

install_obs_plugin_with_data(win-asio2 data)
