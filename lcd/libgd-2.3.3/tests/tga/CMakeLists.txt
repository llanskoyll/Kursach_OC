LIST(APPEND TESTS_FILES
	bug00084
	bug00247
	bug00247a
	bug00248
	bug00248a
	heap_overflow
	tga_null
)

IF(PNG_FOUND)
LIST(APPEND TESTS_FILES
	tga_read
)
ENDIF(PNG_FOUND)

ADD_GD_TESTS()
