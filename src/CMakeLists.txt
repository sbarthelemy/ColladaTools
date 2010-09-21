
find_package(Boost COMPONENTS program_options filesystem system)
find_package(HDF5 COMPONENTS CXX)
find_package(ZLIB)
find_package(COLLADA)

if (COLLADA_FOUND)
    # this section was adapted from
    # osg/src/osgPlugins/dae/CMakeLists.txt
    if(COLLADA_DYNAMIC_LIBRARY AND COLLADA_STATIC_LIBRARY)
        option(COLLADA_USE_STATIC "Set to ON to build with static Collada support." OFF)
    elseif(COLLADA_STATIC_LIBRARY)
        set(COLLADA_USE_STATIC "YES")
    else()
        set(COLLADA_USE_STATIC "NO")
    endif()

    IF(COLLADA_USE_STATIC)
        IF(WIN32)
            SET(COLLADA_LIBRARIES
                ${COLLADA_STATIC_LIBRARY}
                ${COLLADA_LIBXML_LIBRARY}
                ${COLLADA_ZLIB_LIBRARY}
                ${COLLADA_PCRECPP_LIBRARY}
                ${COLLADA_PCRE_LIBRARY}
                ${COLLADA_MINIZIP_LIBRARY}
                ${COLLADA_BOOST_FILESYSTEM_LIBRARY}
                ${COLLADA_BOOST_SYSTEM_LIBRARY})
        ELSE()
            SET(COLLADA_LIBRARIES
                ${COLLADA_STATIC_LIBRARY}
                ${COLLADA_LIBXML_LIBRARY}
                ${COLLADA_PCRECPP_LIBRARY}
                ${COLLADA_PCRE_LIBRARY}
                ${COLLADA_BOOST_FILESYSTEM_LIBRARY}
                ${COLLADA_BOOST_SYSTEM_LIBRARY})
        ENDIF()
    ELSE()
        IF(WIN32)
            SET(COLLADA_LIBRARIES ${COLLADA_DYNAMIC_LIBRARY})
        ELSE()
            SET(COLLADA_LIBRARIES
                ${COLLADA_DYNAMIC_LIBRARY}
                ${COLLADA_BOOST_FILESYSTEM_LIBRARY})
        ENDIF()
    ENDIF()
endif (COLLADA_FOUND)

include_directories(${HDF5_INCLUDE_DIRS})
add_library(
    avreader
    STATIC
    avreader.cpp
)
add_library(
    avreaderh5
    STATIC
    avreaderh5.cpp
)

target_link_libraries(
    avreaderh5
    avreader
    ${HDF5_LIBRARIES}
    ${ZLIB_LIBRARIES}
)

include_directories(
    ${Boost_INCLUDE_DIR}
)
include_directories(
    ${COLLADA_INCLUDE_DIR}
    ${COLLADA_INCLUDE_DIR}/1.4
)

add_executable(
    h5toanim
    h5toAnim.cpp
)
target_link_libraries(
    h5toanim
    avreader
    avreaderh5
    ${Boost_SYSTEM_LIBRARY}
    ${Boost_FILESYSTEM_LIBRARY}
    ${COLLADA_LIBRARIES}
)


add_executable(
    daefilter
    main.cpp
)
target_link_libraries(
    daefilter
    ${COLLADA_LIBRARIES}
    ${Boost_SYSTEM_LIBRARY}
    ${Boost_FILESYSTEM_LIBRARY}
)

install(
    TARGETS h5toanim daefilter
    RUNTIME
    DESTINATION bin)
