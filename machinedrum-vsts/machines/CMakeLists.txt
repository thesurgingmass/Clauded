include(${CMAKE_SOURCE_DIR}/cmake/AddMachine.cmake)

# EFM family
add_subdirectory(EFMBD)

# Remaining EFM/TRX/PI machines are added incrementally as they're built.
