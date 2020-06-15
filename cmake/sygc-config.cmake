find_package(emp-tool)

find_path(SYGC_DIRS sygc/emp-ag2pc.h)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(SYGC DEFAULT_MSG SYGC_DIRS)

if(SYGC_FOUND)
	set(SYGC_DIRS ${SYGC_DIRS} ${EMP-TOOL_INCLUDE_DIRS})
	set(SYGC_LIBRARIES ${EMP-TOOL_LIBRARIES})
endif()
