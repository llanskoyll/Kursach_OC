LIST(APPEND TESTS_FILES
	bug00003
	bug00106_gdimagerectangle
	github_bug_00172
	point_ordering
)

IF(PNG_FOUND)
LIST(APPEND TESTS_FILES
	bug00299
)
ENDIF(PNG_FOUND)

ADD_GD_TESTS()
