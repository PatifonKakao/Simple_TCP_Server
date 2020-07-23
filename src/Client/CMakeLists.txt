# задаем набор исходников
set(SOURCES
Client.h
Client.cpp
)

# создаем статическую библиотеку
add_library(client_lib ${SOURCES})

# добавлем путь к исходникам для возожности подключения библиотеки
target_include_directories(client_lib PUBLIC ${SRC_CLIENT})

# группируем исходники по типу по категориям
source_group(TREE ${CMAKE_CURRENT_LIST_DIR} FILES ${SOURCES})

# В solution эта библиотека лежит в client_lib
set_property(TARGET client_lib PROPERTY FOLDER "clinet_lib")

# включаем в сборку проекnа подпроект с тестами
add_subdirectory(client_start)