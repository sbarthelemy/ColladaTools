
add_test(
    test_simplearm
    ${ColladaTools_SOURCE_DIR}/tests/generate_and_compare.py
    ${EXECUTABLE_OUTPUT_PATH}
    ${ColladaTools_SOURCE_DIR}/tests/simplearm_notflat.h5
    ${ColladaTools_SOURCE_DIR}/tests/simplearm_scene_notflat.dae
    ${ColladaTools_SOURCE_DIR}/tests/simplearm_anim_notflat.dae
)
