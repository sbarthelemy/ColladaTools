
FIND_PATH(COLLADA_INCLUDE_DIR dae.h
  	/usr/local/include
    /usr/local/include/colladadom
    /usr/include/
    /usr/include/colladadom
)

FIND_LIBRARY(COLLADA_DYNAMIC_LIBRARY 
    NAMES collada_dom collada14dom Collada14Dom libcollada14dom21 libcollada14dom22
    PATHS
    $ENV{COLLADA_DIR}/lib
    $ENV{COLLADA_DIR}/lib-dbg
    $ENV{COLLADA_DIR}
    ~/Library/Frameworks
    /Library/Frameworks
    /usr/local/lib
    /usr/local/lib64
    /usr/lib
    /usr/lib64
    /sw/lib
    /opt/local/lib
    /opt/csw/lib
    /opt/lib
    /usr/freeware/lib64
)

FIND_LIBRARY(COLLADA_DYNAMIC_LIBRARY_DEBUG 
    NAMES collada_dom-d collada14dom-d Collada14Dom-d libcollada14dom21-d libcollada14dom22-d
    PATHS
    $ENV{COLLADA_DIR}/lib
    $ENV{COLLADA_DIR}/lib-dbg
    $ENV{COLLADA_DIR}
    ~/Library/Frameworks
    /Library/Frameworks
    /usr/local/lib
    /usr/local/lib64
    /usr/lib
    /usr/lib64
    /sw/lib
    /opt/local/lib
    /opt/csw/lib
    /opt/lib
    /usr/freeware/lib64
)

FIND_PATH(LIBXML_INCLUDE_DIR libxml
	/usr/include/libxml2
	/usr/local/include/libxml2
)


SET(COLLADA_FOUND "NO")
IF(COLLADA_DYNAMIC_LIBRARY OR COLLADA_STATIC_LIBRARY)
    IF   (COLLADA_INCLUDE_DIR)
        SET(COLLADA_FOUND "YES")
        SET(COLLADA_LIBRARIES ${COLLADA_DYNAMIC_LIBRARY} ${BOOST_SYSTEM_LIBRARY} ${BOOST_FILESYSTEM_LIBRARY} ${LIBXML2_LIBRARY} ${CMAKE_DL_LIBS})
    	SET(COLLADA_DEBUG_LIBRARIES ${COLLADA_DYNAMIC_LIBRARY_DEBUG}
${CMAKE_DL_LIBS})
    ENDIF(COLLADA_INCLUDE_DIR)
ENDIF(COLLADA_DYNAMIC_LIBRARY OR COLLADA_STATIC_LIBRARY)
