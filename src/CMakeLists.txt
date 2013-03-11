
find_package(Boost COMPONENTS program_options filesystem system)
find_package(HDF5 COMPONENTS CXX)
find_package(ZLIB)
find_package(COLLADA)

# handle collada libraries

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


# build h5toanim
include_directories(
    ${HDF5_INCLUDE_DIRS}
    ${ColladaTools_SOURCE_DIR}/avreader
)
include_directories(
    ${COLLADA_INCLUDE_DIR}
    ${COLLADA_INCLUDE_DIR}/1.4
)
include_directories(
    ${Boost_INCLUDE_DIR}
)
add_executable(
    h5toanim
    h5toAnim.cpp
)
target_link_libraries(
    h5toanim
    avreader
    ${HDF5_LIBRARIES}
    ${ZLIB_LIBRARIES}
    ${Boost_SYSTEM_LIBRARY}
    ${Boost_FILESYSTEM_LIBRARY}
    ${Boost_PROGRAM_OPTIONS_LIBRARY}
    ${COLLADA_LIBRARIES}
)

# build dae filter
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

# install
install(
    TARGETS h5toanim daefilter
    RUNTIME
    DESTINATION bin)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/FixBundle.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/FixBundle.cmake
    @ONLY
)
install(
    SCRIPT
    ${CMAKE_CURRENT_BINARY_DIR}/FixBundle.cmake
)
