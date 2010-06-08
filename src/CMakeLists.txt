#FIND_PACKAGE(HDF5)
SET(HDF5_INCLUDE_DIRS
    ${FILTRECOLLADA_SOURCE_DIR}/hdf5/include)
SET(HDF5_LIBRARIES
    ${FILTRECOLLADA_SOURCE_DIR}/hdf5/lib/libhdf5_cpp.a
    ${FILTRECOLLADA_SOURCE_DIR}/hdf5/lib/libhdf5.a
    ${FILTRECOLLADA_SOURCE_DIR}/szip/lib/libsz.a)
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

FIND_PACKAGE(Boost 1.38.0 COMPONENTS program_options filesystem)
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(
	${COLLADA_INCLUDE_DIR}
	${COLLADA_INCLUDE_DIR}/1.4
	${LIBXML2_INCLUDE_DIR}
)

add_executable(
    h5toAnim
    h5toAnim.cpp
)
target_link_libraries(
    h5toAnim
    avreader
    avreaderh5
    ${Boost_LIBRARIES}
    ${COLLADA_LIBRARIES}
)

add_executable(
	coherencytest
	coherencytest.cpp
)

target_link_libraries(
	coherencytest
	${COLLADA_LIBRARIES}
)

add_executable(
	daeFilter
	main.cpp
)

target_link_libraries(
	daeFilter
	${COLLADA_LIBRARIES}
)
