# требуем минимальную версию cmake
cmake_minimum_required(VERSION 3.9.3)

# устанавливаем имя проекта
project(Simple_TCP_Server)

# устанавливаем директории в которые будут собираться библиотеки и исполняемые файлы
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# устанавливаем дополнительные флаги компиляции для visual studio
if(MSVC)
    add_compile_options(
        $<$<CONFIG:>:/MT>
        $<$<CONFIG:Debug>:/MTd>
        $<$<CONFIG:Release>:/MT>
    )
endif()

# устанавливаем стандарт c++
set(CMAKE_CXX_STANDARD 17)

# Debug сборка по умолчанию
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

# set(CMAKE_CXX_FLAGS "-Wall -Wextra")
# set(CMAKE_CXX_FLAGS_DEBUG "-g")
# set(CMAKE_CXX_FLAGS_RELEASE "-O3")

# для *nix потоков
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

# органезуем таргеты по директорям
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# устанавливаем переменные, в которых содержится пути к папкам с исходниками
set(SRC_CLIENT ${CMAKE_CURRENT_LIST_DIR}/src/Client)
set(SRC_SERVER ${CMAKE_CURRENT_LIST_DIR}/src/Server)

# включаем в сборку проекта подпроект с тестами
add_subdirectory(src)
