# задаем набор исходников
set(SOURCES
Server.h
Server.cpp
)

# создаем статическую библиотеку
add_library(server_lib ${SOURCES})

# добавлем путь к исходникам для возожности подключения библиотеки
target_include_directories(server_lib PUBLIC ${SRC_SERVER})

# группируем исходники по типу по категориям
source_group(TREE ${CMAKE_CURRENT_LIST_DIR} FILES ${SOURCES})

# В solution эта библиотека лежит в server_lib
set_property(TARGET server_lib PROPERTY FOLDER "server_lib")

# включаем в сборку проекnа подпроект с тестами
add_subdirectory(server_start)