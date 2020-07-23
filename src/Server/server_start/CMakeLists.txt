# задаем набор исходников
set(SOURCES
    server_start.cpp
)

# создаем исполняемый файл
add_executable(server_start ${SOURCES})

target_link_libraries(server_start PUBLIC server_lib)
