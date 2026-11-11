# Este archivo se incluye desde el CMakeLists.txt de la raiz.

# Programa para leer una instancia y mostrar sus tareas.
add_executable(arp_tasks
    "${CMAKE_CURRENT_LIST_DIR}/arp_instance.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/arp_tasks.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/list_tasks.cpp"
)

target_include_directories(arp_tasks
    PRIVATE "${CMAKE_CURRENT_LIST_DIR}"
)

target_compile_features(arp_tasks PRIVATE cxx_std_11)

# Las pruebas se compilan solo cuando se solicitan.
add_executable(arp_tasks_test EXCLUDE_FROM_ALL
    "${CMAKE_CURRENT_LIST_DIR}/arp_tasks.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/tests/test_tasks.cpp"
)

target_include_directories(arp_tasks_test
    PRIVATE "${CMAKE_CURRENT_LIST_DIR}"
)

target_compile_features(arp_tasks_test PRIVATE cxx_std_11)