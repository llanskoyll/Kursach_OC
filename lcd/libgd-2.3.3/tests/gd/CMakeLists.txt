LIST(APPEND TESTS_FILES
	gd_versiontest
)

IF(ENABLE_GD_FORMATS)
LIST(APPEND TESTS_FILES
	bug00383
	gd_null
	gd_num_colors
	gd_im2im
)
ENDIF(ENABLE_GD_FORMATS)

ADD_GD_TESTS()
