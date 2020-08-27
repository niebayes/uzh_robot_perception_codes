#ifndef UZH_SIFT_COMPUTE_DESCRIPTORS_H_
#define UZH_SIFT_COMPUTE_DESCRIPTORS_H_

#include <cmath>
#include <iostream>

#include "algorithm/weightedhist.h"
#include "armadillo"
#include "matlab_port/cell2mat.h"
#include "matlab_port/fspecial.h"
#include "matlab_port/imgradient.h"
#include "sift/derotate.h"
#include "glog/logging.h"

//@brief Compute descriptors from the patches around the putative keypoints.
//@param blurred_images Blurred images computed from the ComputeBlurredImages
// function. These images are used to locate the patches around the putative
// keypoints.
//@param keypoints Putative keypoints computed from the ExtractKeypoints
// function. These keypoints are then refined to find the final keypoints.
//@param descriptors containing the returned descriptors where each column is a
// descriptor vector.
//@param final_keypoints TODO
//@param rotation_invariant Boolean value used to denote whether the computed
// descriptors are invariant to rotation or not. If true, a dominant orientation
// will be assigned to each descriptor.
void ComputeDescriptors(const arma::field<arma::cube>& blurred_images,
                        const arma::field<arma::umat>& keypoints,
                        arma::mat& descriptors, arma::umat& final_keypoints,
                        const bool rotation_invariant = false) {
  if (blurred_images.size() != keypoints.size())
    LOG(ERROR) << "The number of octaves are not consistent.";
  const int kNumOctaves = blurred_images.size();

  // The magic number 1.5 is taken from Lowe's paper.
  const arma::mat gaussian_kernel =
      uzh::cv2arma<double>(uzh::fspecial(uzh::GAUSSIAN, 16, 16.0 * 1.5)).t();

  // Construct fields of descriptors and keypoints to be populated.
  arma::field<arma::mat> descs;
  arma::field<arma::umat> kpts;

  // Iterate over each octave
  for (int o = 0; o < kNumOctaves; ++o) {
    // Get the blurred images and keypoints in current octave
    // oct_blurred_images [w x h x num_images_per_octave]
    // oct_keypoints [3 x n] where the last row is the scale.
    const arma::cube oct_blurred_images = blurred_images(o);
    const arma::umat oct_keypoints = keypoints(o);

    // Only consider relevant images involved in the extraction of the
    // keypoints.
    // Extract the scale indices of the coordinates of the keypoints and
    // unique them. These indices are indicators from which we can tell which
    // images in the blurred images of the current octave have contributed to
    // the extraction of keypoints.
    const arma::urowvec kImageIndices = arma::unique(oct_keypoints.row(2));

    // Iterate over each involved image
    for (int img_idx : kImageIndices) {
      // Filter out irrelevant keypoints based on the image index
      const arma::uvec is_kept_in_image = (oct_keypoints.row(2) == img_idx);
      const arma::umat kept_keypoints = oct_keypoints(is_kept_in_image);
      arma::umat kept_keypoints_xy = kept_keypoints.head_rows(2);

      // Compute image gradient for use of Histogram of Oriented Gradients.
      const arma::mat image = oct_blurred_images.slice(img_idx);
      const arma::field<arma::mat> gradient = uzh::imgradient(image);
      const arma::mat grad_magnitude = gradient(0);
      const arma::mat grad_direction = gradient(1);

      // Construct descriptor matrix for the current image.
      const int kNumKeypoints = kept_keypoints_xy.n_cols;
      // For each keypoints in this image, there's a 128-length descriptor
      // vector computed from Histogram of Oriented Gradients.
      arma::mat img_descriptor(128, kNumKeypoints, arma::fill::zeros);

      // Mask to mask out all keypoints around which the patches are out of the
      // image boundary.
      arma::uvec is_valid(kNumKeypoints, arma::fill::zeros);

      // Iterate over each kept keypoints
      for (int corner_idx = 0; corner_idx < kNumKeypoints; ++corner_idx) {
        // Get the row and col indices. Note y -> row, x -> col.
        const int row = kept_keypoints_xy(1, corner_idx);
        const int col = kept_keypoints_xy(0, corner_idx);

        // Ensure all the pixels inside the patch are within the image boundary.
        // The patch is 16 x 16, so we take the point 8 pixels away from the
        // upper left and 7 pixels aways from the lower right as the anchor
        // point.
        if (row >= 8 && col >= 8 && row < image.n_rows - 7 &&
            col < image.n_cols - 7) {
          is_valid(corner_idx) = 1;
          // Convolve the patch with the Gaussian window.
          const arma::mat patch_grad_mag =
              grad_magnitude(row, col, arma::size(16, 16));
          // Note the gaussian_kernel is symmetric, hence no need to flip it
          // around. The % operator is element-wise multiplication.
          const arma::mat patch_grad_mag_w = patch_grad_mag % gaussian_kernel;
          // Gradient direction is unchanged after convolution.
          const arma::mat patch_grad_dir =
              grad_direction(row, col, arma::size(16, 16));

          // If rotation_invariant is true, derotate the patch based on the
          // dominant orientation to achieve rotation invariance.
          // Clone the patch to be derotated.
          arma::mat derotated_patch_grad_mag_w = patch_grad_mag_w;
          arma::mat derotated_patch_grad_dir = patch_grad_dir;
          if (rotation_invariant) {
            // TODO(bayes)
          }

          // Compute the descriptor for this patch using Histogram of Oriented
          // Gradients.
          // The current 16 x 16 patch is divided into 4 subpatches, and each
          // [8 x 8] subpatches is then divided into 4 quadrants, i.e.
          // 4 * [4 x 4] subsubpatches. For each quadrant, an 8-bit histogram of
          // the orientations of the gradients is computed. The 8 values of the
          // histogram is then populated into the corresponding subvector of the
          // current descriptor vector. Hence the length of the descriptor
          // vector is 4 * 4 * 8 = 128.

          // Starting index of the current subvector in the descriptor vector.
          int start_idx = 0;
          // Iterate over each quadrant in the current patch. There's in total
          // 4 * 4 = 16 quadrants.
          for (int j = 0; j < 4; ++j) {
            for (int i = 0; i < 4; ++i) {
              // Compute histogram for the current (i, j)-th quadrant.
              // Before creating the histogram, the orientations of gradients
              // are weighted according to the gradients magnitude.
              const arma::vec hist =
                  uzh::weightedhist(arma::vectorise(derotated_patch_grad_dir(
                                        4 * j, 4 * i, arma::size(4, 4))),
                                    arma::vectorise(derotated_patch_grad_mag_w(
                                        4 * j, 4 * i, arma::size(4, 4))),
                                    arma::linspace<arma::vec>(180, 180, 9));

              // Populate the subvector of the descriptor vector.
              img_descriptor.col(corner_idx).subvec(start_idx, start_idx + 7) =
                  hist.head(8);
              std::cout << start_idx << '\n';
            }
          }
        }
      }
      // Adapt keypoint coordinates such that they correspond to the original
      // image resolutions. This adaption is for presentation of superimposed
      // keypoints on original image.

      // Upper one octave, the image is domwsampled by a factor of 2. To inverse
      // this, multiply each coordinates with the octave_idx-th power
      // of 2, where octave_idx starts from 0.
      kept_keypoints_xy *= std::pow(2, o);

      // Only store valid keypoints and the corresponding descriptors.
      descs << img_descriptor.cols(is_valid);
      kpts << kept_keypoints_xy.cols(is_valid);
    }
  }

  // Normalize each descriptor vector such that they have unit Euclidean norm.
  descriptors = arma::normalise(uzh::cell2mat<double>(descs));
  final_keypoints = uzh::cell2mat<arma::uword>(kpts);
}

#endif  // UZH_SIFT_COMPUTE_DESCRIPTORS_H_