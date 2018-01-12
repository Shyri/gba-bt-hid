macro(OBJCOPY_FILE EXE_NAME)
    set(FO ${CMAKE_CURRENT_BINARY_DIR}/${EXE_NAME}.gba)
    set(FI ${CMAKE_CURRENT_BINARY_DIR}/${EXE_NAME})
    message(STATUS ${FO})
    add_custom_command(
            OUTPUT "${FO}"
            COMMAND ${DEVKITARM}/bin/arm-none-eabi-objcopy
            ARGS -O binary ${FI} ${FO}
            DEPENDS ${FI})
    get_filename_component(TGT "${EXE_NAME}" NAME)
    add_custom_target("TargetObjCopy_${TGT}" ALL DEPENDS ${FO} VERBATIM)
    get_directory_property(extra_clean_files ADDITIONAL_MAKE_CLEAN_FILES)
    set_directory_properties(
            PROPERTIES
            ADDITIONAL_MAKE_CLEAN_FILES "${extra_clean_files};${FO}")
    set_source_files_properties("${FO}" PROPERTIES GENERATED TRUE)
endmacro(OBJCOPY_FILE)

if(NOT GBAFIX_EXE)
    message(STATUS "Looking for arm-none-eabi-objcopy")
    find_program(GBAFIX_EXE gbafix ${DEVKITARM}/bin)
    if(GBAFIX_EXE)
        message(STATUS "Looking for arm-none-eabi-objcopy -- ${GBAFIX_EXE}")
    endif(GBAFIX_EXE)
endif(NOT GBAFIX_EXE)

if(GBAFIX_EXE)
    macro(GBAFIX_FILE EXE_NAME)
        set(FO ${CMAKE_CURRENT_BINARY_DIR}/${EXE_NAME}.gba)
        set(BIN ${CMAKE_CURRENT_BINARY_DIR}/${EXE_NAME}.bin)
        get_filename_component(TGT "${EXE_NAME}" NAME)
        add_custom_target("Target_GBA${TGT}" ALL DEPENDS ${FO} VERBATIM COMMAND ${GBAFIX_EXE} ${FO})
        get_directory_property(extra_clean_files ADDITIONAL_MAKE_CLEAN_FILES)
        set_directory_properties(
                PROPERTIES
                ADDITIONAL_MAKE_CLEAN_FILES "${extra_clean_files};${FO}")
        set_source_files_properties(${FO} PROPERTIES GENERATED TRUE)
    endmacro(GBAFIX_FILE)
endif(GBAFIX_EXE)