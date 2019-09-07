set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 COMPONENTS Widgets Core Gui REQUIRED)

if(WIN32)
	set(EXECUTABLE_TYPE WIN32)
endif()

add_executable(bookmark-qt ${EXECUTABLE_TYPE}
	main.cpp
	mainwindow.cpp
	mainwindow.h
	mainwindow.ui
)

target_link_libraries(bookmark-qt core Qt5::Widgets)

if(WIN32)
	install(
		FILES
			$<TARGET_FILE:Qt5::Widgets>
			$<TARGET_FILE:Qt5::Core>
			$<TARGET_FILE:Qt5::Gui>
		DESTINATION
			"${BINARY_INSTALL_DIR}"
	)
	install(TARGETS bookmark-qt RUNTIME DESTINATION "${BINARY_INSTALL_DIR}")

	add_custom_command(TARGET bookmark-qt POST_BUILD
		COMMAND "${CMAKE_COMMAND}" -E copy_if_different
			$<TARGET_FILE:Qt5::Widgets>
			$<TARGET_FILE:Qt5::Core>
			$<TARGET_FILE:Qt5::Gui>
			$<TARGET_PROPERTY:bookmark-qt,RUNTIME_OUTPUT_DIRECTORY>
	)
endif()
