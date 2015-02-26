project(obs-vce)

include_directories(include)

set(obs-vce_SOURCES
	obs-vce.cpp
	obs-vce-amf.cpp
	obs-vce-amf-d3d11.cpp
	obs-vce-amf-converters.cpp
	obs-vce-optional.cpp
	obs-vce-plugin-main.cpp)

set(obs-vce_HEADERS
	obs-vce.h)

add_library(obs-vce MODULE
	${obs-vce_SOURCES}
	${obs-vce_HEADERS})

if(MSVC)
	if(APPLE)
		set(_bit_suffix "")
	elseif(CMAKE_SIZEOF_VOID_P EQUAL 8)
		set(_bit_suffix "64bit/")
	else()
		set(_bit_suffix "32bit/")
	endif()

	add_library(amfcore STATIC IMPORTED)
	set_target_properties(amfcore PROPERTIES
		IMPORTED_LOCATION ${CMAKE_CURRENT_LIST_DIR}/lib/amf/x86/amf-core-windesktop32.lib)
	file(GLOB amf_dll "${CMAKE_CURRENT_LIST_DIR}/dll/amf/x86/*.dll")
	foreach(dll ${amf_dll})
		add_custom_command(TARGET obs-vce POST_BUILD COMMAND
			${CMAKE_COMMAND} -E copy_if_different
			${dll}
			"${OBS_OUTPUT_DIR}/$<CONFIGURATION>/bin/${_bit_suffix}")
	endforeach()
endif(MSVC)

target_link_libraries(obs-vce
	amfcore
	libobs
	)

install_obs_plugin_with_data(obs-vce data)
