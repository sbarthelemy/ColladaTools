#FIND_PACKAGE(HDF5)
SET(HDF5_INCLUDE_DIRS
    ${COLLADATOOLS_SOURCE_DIR}/hdf5/include)
SET(HDF5_LIBRARIES
    ${COLLADATOOLS_SOURCE_DIR}/hdf5/lib/libhdf5_cpp.a
    ${COLLADATOOLS_SOURCE_DIR}/hdf5/lib/libhdf5.a
    ${COLLADATOOLS_SOURCE_DIR}/szip/lib/libsz.a)
FIND_PACKAGE(ZLIB)

include_directories(${HDF5_INCLUDE_DIRS})
message(STATUS ${Boost_LIBRARIES})
add_library(
    avreader
    avreader.cpp
)

add_library(
    avreaderh5
    avreaderh5.cpp
)
target_link_libraries(
    avreaderh5
    avreader
    ${HDF5_LIBRARIES}
    ${ZLIB_LIBRARIES}
)

FIND_PACKAGE(Boost 1.40.0 COMPONENTS system filesystem)
#FIND_PACKAGE(LibXml2 REQUIRED)
INCLUDE_DIRECTORIES(
	${Boost_INCLUDE_DIR}
	#${LIBXML2_INCLUDE_DIR}
)
INCLUDE_DIRECTORIES(
	${COLLADA_INCLUDE_DIR}
	${COLLADA_INCLUDE_DIR}/1.4
)
add_executable(
    h5toAnim
    h5toAnim.cpp
)
target_link_libraries(
    h5toAnim
    avreader
    avreaderh5
    ${Boost_SYSTEM_LIBRARY}
    ${Boost_FILESYSTEM_LIBRARY}
    ${COLLADA_LIBRARIES}
)


add_executable(
	daeFilter
	main.cpp
)

target_link_libraries(
	daeFilter
	${COLLADA_LIBRARIES}
	${Boost_SYSTEM_LIBRARY}
	${Boost_FILESYSTEM_LIBRARY}
)
