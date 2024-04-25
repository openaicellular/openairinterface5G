add_library(CONFIG_LIB
            config_common.c
            config_load_configmodule.c
            config_userapi.c
            config_cmdline.c
)
target_link_libraries(CONFIG_LIB PRIVATE dl)
target_link_libraries(CONFIG_LIB PUBLIC ${T_LIB})

# CONFIG_LIB is a wrapper around a particular config implementation, default is
# libconfig in params_libconfig below
pkg_check_modules(libconfig REQUIRED libconfig)
add_library(params_libconfig MODULE libconfig/config_libconfig.c)
target_include_directories(params_libconfig PRIVATE ${libconfig_INCLUDE_DIRS})
target_link_libraries(params_libconfig PRIVATE CONFIG_LIB ${libconfig_LIBRARIES})
set_target_properties(params_libconfig PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
