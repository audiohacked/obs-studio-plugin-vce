project(obs-vce)

find_package(AMDAMF)
if(AMDAMF_FOUND)
include_directories(${AMDAMF_INCLUDE_DIRS})
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

target_link_libraries(obs-vce
	libobs
	${AMDAMF_LIBRARIES}
	)

install_obs_plugin_with_data(obs-vce data)
else(AMDAMF_FOUND)
message("VCE AMF encoder disabled due to missing libs")
endif(AMDAMF_FOUND)

