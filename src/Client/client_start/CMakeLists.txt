# задаем набор исходников
set(SOURCES
    client_start.cpp
)

# создаем исполняемый файл
add_executable(client_start ${SOURCES})

target_link_libraries(client_start PUBLIC client_lib)
