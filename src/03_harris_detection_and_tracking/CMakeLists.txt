project(harris_detection_and_tracking)
add_executable(harris_detection_and_tracking harris_detection_and_tracking.cc)
target_compile_options(harris_detection_and_tracking PRIVATE -O3 -march=native)
target_link_libraries(harris_detection_and_tracking
  ${OpenCV_LIBRARIES}
  ${GLOG_LIBRARY}
  ${ARMADILLO_LIBRARIES}
)