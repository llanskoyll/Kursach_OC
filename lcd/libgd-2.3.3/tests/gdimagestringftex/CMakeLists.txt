IF(FREETYPE_FOUND)
LIST(APPEND TESTS_FILES
	gdimagestringftex_returnfontpathname
)
ENDIF(FREETYPE_FOUND)

ADD_GD_TESTS()
