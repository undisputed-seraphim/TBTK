#ADD_SUBDIRECTORY(ArnoldiIterator/TBTK/PropertyExtractor/)

INSTALL(
	DIRECTORY .
	DESTINATION include/TBTK/PropertyExtractor
	FILES_MATCHING PATTERN *.h
	PATTERN ArnoldiIterator.h EXCLUDE
	PATTERN ArnoldiIterator* EXCLUDE
)

IF(${COMPILE_ARNOLDI_ITERATOR})
	INSTALL(
		DIRECTORY .
		DESTINATION include/TBTK/PropertyExtractor
		FILES_MATCHING PATTERN ArnoldiIterator.h
	)
ENDIF(${COMPILE_ARNOLDI_ITERATOR})
