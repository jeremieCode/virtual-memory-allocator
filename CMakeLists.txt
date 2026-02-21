cmake_minimum_required(VERSION 3.20)

project(ensimalloc)

###
# Configuration de CMake et du C
###
enable_testing()
set(CMAKE_VERBOSE_MAKEFILE ON)

set(CMAKE_C_STANDARD 23)
set(CMAKE_CXX_STANDARD 23)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Wall -Werror -fanalyzer -fsanitize=undefined -fsanitize-address-use-after-scope")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Werror -fanalyzer -fsanitize-address-use-after-scope")

#######
# Compilation standard avec googletest
#######

find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

##
# Bibliothèque principale
##
add_library(ensimalloc SHARED
  src/mem.c
  src/mem_internals.c
  src/mem_small.c
  src/mem_medium.c
  src/mem_large.c
)

# Lien avec pthread pour protéger l'arène globale avec mutex
target_link_libraries(ensimalloc pthread)

##
# Programme de tests unitaires
##
add_executable(alloctest
  tests/alloctest.cc
  tests/test_mark.cc
  tests/test_generic.cc
  tests/test_buddy.cc
  tests/test_run_cpp.cc
)

target_link_libraries(alloctest gtest gtest_main ensimalloc)
add_test(AllTestsAllocator alloctest)

##
# Cible pour lancer les tests facilement
##
add_custom_target(check COMMAND alloctest)

##
# Test overflow C
##
add_executable(test_overflow tests/test_overflow.c)
target_link_libraries(test_overflow ensimalloc)

##
# Construction du shell
##
add_executable(memshell src/memshell.c)
target_link_libraries(memshell ensimalloc)

##
# Construction de l'archive
##
set(CPACK_PACKAGE_VERSION_MAJOR "4")
set(CPACK_PACKAGE_VERSION_MINOR "0")
set(CPACK_PACKAGE_VERSION_PATCH "")
set(CPACK_SOURCE_GENERATOR "TGZ")
set(CPACK_SOURCE_IGNORE_FILES
  "~$"
  "\\\\.o$"
  "^${PROJECT_SOURCE_DIR}/build/"
)
include(CPack)
