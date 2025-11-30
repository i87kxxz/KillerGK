# Examples CMakeLists.txt

# Basic example
add_executable(basic_example basic/main.cpp)
target_link_libraries(basic_example PRIVATE KillerGK)

# Add more examples as they are created
# add_executable(file_explorer file_explorer/main.cpp)
# target_link_libraries(file_explorer PRIVATE KillerGK KGK2D)
