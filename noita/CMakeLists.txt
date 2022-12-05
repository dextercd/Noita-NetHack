add_library(process-manager SHARED
    process_manager.cpp
)

target_link_libraries(process-manager
    PUBLIC terminal-static
    PRIVATE minhook
)

add_executable(nhnoita
    main.cpp
)

target_link_libraries(nhnoita PRIVATE process-manager)

install(TARGETS process-manager
    DESTINATION ${ROOT}
    COMPONENT NoitaNetHack
)
