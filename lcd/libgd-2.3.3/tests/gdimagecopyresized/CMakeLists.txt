IF(PNG_FOUND)
LIST(APPEND TESTS_FILES
	gdimagecopyresized
)
ENDIF(PNG_FOUND)

ADD_GD_TESTS()
