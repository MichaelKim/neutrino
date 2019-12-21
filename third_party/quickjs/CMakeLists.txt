cmake_minimum_required (VERSION 3.8)
project (quickjs)

set(quickjs_cflags -Wall)
if(CMAKE_C_COMPILER_ID MATCHES "AppleClang|Clang")
    list(APPEND quickjs_cflags
        -Wextra
        -Wno-sign-compare
        -Wno-missing-field-initializers
        -Wno-unused-parameter
        -Wno-unused-variable
        -funsigned-char)
else()
    list(APPEND quickjs_cflags
        -Wno-array-bounds
        -Wno-unused-variable
        -Wno-unused-but-set-variable)
endif()

add_library(quickjs STATIC
    src/cutils.c
    src/libbf.c
    src/libregexp.c
    src/libunicode.c
    src/quickjs-libc.c
    src/quickjs.c
)
# set_target_properties(quickjs PROPERTIES
#     C_STANDARD 11
#     C_STANDARD_REQUIRED ON
# )

target_compile_options(quickjs PRIVATE ${quickjs_cflags})
target_compile_definitions(quickjs PRIVATE
    CONFIG_BIGNUM
)
target_include_directories(quickjs PUBLIC include)
target_include_directories(quickjs PRIVATE src)