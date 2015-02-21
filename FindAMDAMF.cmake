# Once done these will be defined:
#
#  AMDAMF_FOUND
#  AMDAMF_INCLUDE_DIRS
#  AMDAMF_LIBRARIES

# Only finds Windows' libs

SET(VS "vs12")

# Get environment variable, define it as ENV_$var and make sure backslashes are converted to forward slashes
macro(getenv_path VAR)
	set(ENV_${VAR} $ENV{${VAR}})
	# replace won't work if var is blank
	if (ENV_${VAR})
		#Didn't work for me
		#string( REGEX REPLACE "\\\\" "/" ENV_${VAR} ${ENV_${VAR}} )
		string( REGEX REPLACE "\\\\" "/" ${VAR} ${ENV_${VAR}} )
	endif ()
endmacro(getenv_path)

getenv_path(USERPROFILE)
getenv_path(AMDMediaSDK)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
	set(_lib_suffix 64)
	set(_arch_dir x86_64)
else()
	set(_lib_suffix 32)
	set(_arch_dir x86)
endif()

SET(AMF_HINTS 
	"${AMDMediaSDK}"
	"../../../AMD Media SDK/1.1"
	"../../../AMD Media SDK/1.1 Beta"
	"${USERPROFILE}/AMD Media SDK/1.1"
	"${USERPROFILE}/AMD Media SDK/1.1 Beta")
	
find_path(AMF_INCLUDE_DIR
	NAMES Core.h
	HINTS
		${AMF_HINTS}
	PATHS
		/usr/include /usr/local/include /opt/local/include /sw/include
	PATH_SUFFIXES
		inc/amf)

find_library(AMF_LIB_REL
	NAMES amf-core-windesktop${_lib_suffix}
	HINTS
		${AMF_HINTS}
	PATHS
		/usr/lib /usr/local/lib /opt/local/lib /sw/lib
	PATH_SUFFIXES
		lib/amf/${_arch_dir}/Release/${VS}
		../lib/amf/${_arch_dir}/Release/${VS}
		dll/amf/${_arch_dir}/Release/${VS}
		../dll/amf/${_arch_dir}/Release/${VS})
		
find_library(AMF_LIB_DBG
	NAMES amf-core-windesktop${_lib_suffix} 
	HINTS
		${AMF_HINTS}
	PATHS
		/usr/lib /usr/local/lib /opt/local/lib /sw/lib
	PATH_SUFFIXES
		lib/amf/${_arch_dir}/Debug/${VS}
		../lib/amf/${_arch_dir}/Debug/${VS}
		dll/amf/${_arch_dir}/Debug/${VS}
		../dll/amf/${_arch_dir}/Debug/${VS})

set(AMF_BIN_CORE "amf-core-windesktop${_lib_suffix}.dll")
set(AMF_BIN_VCE "amf-component-vce-windesktop${_lib_suffix}.dll")

foreach(bin CORE VCE)
	find_file(AMF_BIN_${bin}_REL
		NAMES ${AMF_BIN_${bin}}
		HINTS
			${AMF_HINTS}
		PATHS
			/usr/lib /usr/local/lib /opt/local/lib /sw/lib
		PATH_SUFFIXES
			dll/amf/${_arch_dir}/Release/${VS}
			../dll/amf/${_arch_dir}/Release/${VS})
			
	find_file(AMF_BIN_${bin}_DBG
		NAMES ${AMF_BIN_${bin}}
		HINTS
			${AMF_HINTS}
		PATHS
			/usr/lib /usr/local/lib /opt/local/lib /sw/lib
		PATH_SUFFIXES
			dll/amf/${_arch_dir}/Debug/${VS}
			../dll/amf/${_arch_dir}/Debug/${VS})
endforeach()
		
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(AMDAMF DEFAULT_MSG AMF_LIB_REL AMF_LIB_DBG AMF_INCLUDE_DIR)
mark_as_advanced(AMF_INCLUDE_DIR AMF_LIB_REL AMF_LIB_DBG)

if(AMDAMF_FOUND)
	set(AMDAMF_INCLUDE_DIRS ${AMF_INCLUDE_DIR} ${AMF_INCLUDE_DIR}/core)
	set(AMDAMF_LIBRARIES optimized ${AMF_LIB_REL} debug ${AMF_LIB_DBG})
	#set(AMDAMF_BINARIES optimized ${AMF_BIN_REL} debug ${AMF_BIN_DBG})
	set(AMDAMF_BIN_FILES ${AMF_BIN_CORE_REL} ${AMF_BIN_VCE_REL})
	set(AMDAMF_DEBUG_BIN_FILES ${AMF_BIN_CORE_DBG} ${AMF_BIN_VCE_DBG})
	mark_as_advanced(AMDAMF_LIBRARIES)
endif()
