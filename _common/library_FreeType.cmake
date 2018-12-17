
# FreeType to render TTF fonts
ExternalProject_Add(
    FREETYPE_Local
    PREFIX ${CMAKE_BINARY_DIR}/external/FreeType
    URL ${CMAKE_SOURCE_DIR}/_Libs/_sources/freetype-2.7.1.tar.gz
    CMAKE_ARGS
        "-DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}"
        "-DCMAKE_INSTALL_PREFIX=${DEPENDENCIES_INSTALL_DIR}"
        "-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}"
        $<$<BOOL:APPLE>:-DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET}>
        -DWITH_ZLIB=OFF -DWITH_BZip2=OFF -DWITH_PNG=OFF -DWITH_HarfBuzz=OFF
)

