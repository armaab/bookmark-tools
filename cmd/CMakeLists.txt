add_executable (
	pdfmark
	pdfmark.cpp
)

target_link_libraries(pdfmark core)

install(TARGETS pdfmark RUNTIME DESTINATION "${BINARY_INSTALL_DIR}")
