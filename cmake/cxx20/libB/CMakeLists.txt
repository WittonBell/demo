project(libB)

aux_source_directory(. SOURCE)
#这里需要去掉路径中的./
#string(REPLACE "./" "" SOURCE "${SOURCE}")

add_library(${PROJECT_NAME} ${SOURCE})
target_sources(${PROJECT_NAME}
  PUBLIC
    FILE_SET cxx_modules TYPE CXX_MODULES FILES
    apiB.cc
)
target_link_libraries(${PROJECT_NAME} stdm)
