#ifndef UZH_MATLAB_PORT_IMAGESC_H_
#define UZH_MATLAB_PORT_IMAGESC_H_

#include "armadillo"
#include "opencv2/core.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include "transfer/arma2cv.h"

namespace uzh {

//@brief Imitate matlab's imagesc. Apply colormap to the input image to display
// image with scaled colors.
cv::Mat imagesc(const cv::Mat& image, bool show_at_once = true,
                const std::string& winname = "Colormapped image",
                const int colormap = cv::COLORMAP_PARULA, const int delay = 0) {
  cv::Mat img;
  cv::normalize(image, img, 0, 255, cv::NORM_MINMAX, CV_8UC1);
  cv::applyColorMap(img, img, colormap);
  if (show_at_once) {
    cv::namedWindow(winname, cv::WINDOW_NORMAL);
    cv::imshow(winname, img);
    cv::waitKey(delay);
  }
  return img;
}

//@brief Overloaded for arma::mat.
// image with scaled colors.
cv::Mat imagesc(const arma::umat& image, bool show_at_once = true,
                const std::string& winname = "Colormapped image",
                const int colormap = cv::COLORMAP_PARULA, const int delay = 0) {
  cv::Mat cv_image =
      uzh::arma2cv<double>(arma::conv_to<arma::mat>::from(image));
  cv::Mat img;
  cv::normalize(cv_image, img, 0, 255, cv::NORM_MINMAX, CV_8UC1);
  cv::applyColorMap(img, img, colormap);
  if (show_at_once) {
    cv::namedWindow(winname, cv::WINDOW_NORMAL);
    cv::imshow(winname, img);
    cv::waitKey(delay);
  }
  return img;
}

}  // namespace uzh

#endif  // UZH_MATLAB_PORT_IMAGESC_H_