project(two_view_geometry)
add_executable(two_view_geometry two_view_geometry.cc)
# Change the level of optimization to -O2 or -O3 if don't debug.
target_compile_options(two_view_geometry PRIVATE -O3 -march=native)
target_link_libraries(two_view_geometry
  ${OpenCV_LIBRARIES}
  ${GLOG_LIBRARY}
  ${ARMADILLO_LIBRARIES}
  ${PCL_LIBRARIES}
  ${CERES_LIBRARIES}
)