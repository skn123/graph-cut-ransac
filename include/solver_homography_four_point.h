#pragma once

#include "solver_engine.h"
#include "homography_estimator.h"

namespace solver
{
	// This is the estimator class for estimating a homography matrix between two images. A model estimation method and error calculation method are implemented
	class HomographyFourPointSolver : public SolverEngine
	{
	public:
		HomographyFourPointSolver()
		{
		}

		~HomographyFourPointSolver()
		{
		}

		inline bool estimateModel(
			const cv::Mat& data_,
			const size_t *sample_,
			size_t sample_number_,
			std::vector<Model> &models_) const;
	};

	inline bool HomographyFourPointSolver::estimateModel(
		const cv::Mat& data_,
		const size_t *sample_,
		size_t sample_number_,
		std::vector<Model> &models_) const
	{
		constexpr size_t equation_number = 2;
		const size_t row_number = equation_number * sample_number_;
		Eigen::MatrixXd coefficients(row_number, 8);
		Eigen::MatrixXd inhomogeneous(row_number, 1);

		constexpr size_t columns = 4;
		const double *data_ptr = reinterpret_cast<double *>(data_.data);
		size_t row_idx = 0;

		for (size_t i = 0; i < sample_number_; ++i)
		{
			const double *point_ptr = sample_ == nullptr ?
				data_ptr + i * columns :
				data_ptr + sample_[i] * columns;

			const double x1 = point_ptr[0],
				y1 = point_ptr[1],
				x2 = point_ptr[2],
				y2 = point_ptr[3];

			coefficients(row_idx, 0) = -x1;
			coefficients(row_idx, 1) = -y1;
			coefficients(row_idx, 2) = -1;
			coefficients(row_idx, 3) = 0;
			coefficients(row_idx, 4) = 0;
			coefficients(row_idx, 5) = 0;
			coefficients(row_idx, 6) = x2 * x1;
			coefficients(row_idx, 7) = x2 * y1;
			inhomogeneous(row_idx) = -x2;
			++row_idx;

			coefficients(row_idx, 0) = 0;
			coefficients(row_idx, 1) = 0;
			coefficients(row_idx, 2) = 0;
			coefficients(row_idx, 3) = -x1;
			coefficients(row_idx, 4) = -y1;
			coefficients(row_idx, 5) = -1;
			coefficients(row_idx, 6) = y2 * x1;
			coefficients(row_idx, 7) = y2 * y1;
			inhomogeneous(row_idx) = -y2;
			++row_idx;
		}

		Eigen::Matrix<double, 8, 1> h =
			coefficients.colPivHouseholderQr().solve(inhomogeneous);

		Homography model;
		model.descriptor << h(0), h(1), h(2),
			h(3), h(4), h(5),
			h(6), h(7), 1.0;
		models_.emplace_back(model);
		return true;
	}
}