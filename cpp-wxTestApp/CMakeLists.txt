project (wxTestApp)

set(wxWidgets_CONFIGURATION mswu)
find_package(wxWidgets REQUIRED COMPONENTS net core base )

add_executable(wxTestApp WIN32 main.cpp)

target_include_directories(wxTestApp PUBLIC "${wxWidgets_ROOT_DIR}/include" "${wxWidgets_LIB_DIR}/${wxWidgets_CONFIGURATION}")
target_link_libraries(wxTestApp ${wxWidgets_LIBRARIES})
