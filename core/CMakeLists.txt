find_package(GS REQUIRED)

add_library(core STATIC
	pdf_toc.cpp
    toc.cpp
	utf8.cpp
	utils.cpp
	exception.h
	pdf_toc.h
	toc.h
	utf8.h
	utils.h
)

target_link_libraries(core PRIVATE GS::gs)
target_include_directories(core PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

if(WIN32)
	install(FILES $<TARGET_FILE:GS::gs> DESTINATION "${BINARY_INSTALL_DIR}")

	add_custom_command(TARGET core POST_BUILD
		COMMAND "${CMAKE_COMMAND}" -E copy_if_different
			$<TARGET_FILE:GS::gs>
			$<TARGET_PROPERTY:core,RUNTIME_OUTPUT_DIRECTORY>
	)
endif()
