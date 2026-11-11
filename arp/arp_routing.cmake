# Biblioteca reutilizable del modulo ARP.
add_library(arp_routing_core STATIC
    "${CMAKE_CURRENT_LIST_DIR}/arp_instance.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/arp_tasks.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/arp_routing.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/arp_decoder.cpp"
)

target_include_directories(arp_routing_core
    PUBLIC "${CMAKE_CURRENT_LIST_DIR}"
)

target_compile_features(arp_routing_core
    PUBLIC cxx_std_11
)

# Programa de construccion y validacion de rutas.
add_executable(arp_routes
    "${CMAKE_CURRENT_LIST_DIR}/solve_routes.cpp"
)

target_link_libraries(arp_routes
    PRIVATE arp_routing_core
)

# Pruebas existentes.
add_executable(arp_routing_test EXCLUDE_FROM_ALL
    "${CMAKE_CURRENT_LIST_DIR}/tests/test_routing.cpp"
)

target_link_libraries(arp_routing_test
    PRIVATE arp_routing_core
)

# Pruebas del procedimiento de primera travesia.
add_executable(arp_decoder_test EXCLUDE_FROM_ALL
    "${CMAKE_CURRENT_LIST_DIR}/tests/test_decoder.cpp"
)

target_link_libraries(arp_decoder_test
    PRIVATE arp_routing_core
)