add_executable(WordleSolver WordleSolver.cpp)
target_include_directories (WordleSolver PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})


set_property(TARGET WordleSolver PROPERTY CXX_STANDARD 20)
set_property(TARGET WordleSolver PROPERTY VS_DEBUGGER_WORKING_DIRECTORY $<TARGET_FILE_DIR:WordleSolver>)
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT WordleSolver)

add_custom_command(TARGET WordleSolver POST_BUILD        # Adds a post-build event to MyTest
    COMMAND ${CMAKE_COMMAND} -E copy_if_different  # which executes "cmake - E copy_if_different..."
        "${CMAKE_CURRENT_SOURCE_DIR}/english-words/words_alpha.txt"      # <--this is in-file
        $<TARGET_FILE_DIR:WordleSolver>)                 # <--this is out-file path