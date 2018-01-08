/*
 * Initialization.h
 *
 *  Created on: 13.12.2017
 *      Author: Viktor Csomor
 */

#ifndef WEIGHTINITIALIZATION_H_
#define WEIGHTINITIALIZATION_H_

#include <cmath>
#include <Eigen/Dense>
#include <Matrix.h>
#include <random>
#include <string>

namespace cppnn {

template<typename Scalar>
class WeightInitialization {
public:
	virtual ~WeightInitialization() = default;
	virtual void apply(Matrix<Scalar>& weights) const = 0;
};

template<typename Scalar>
class ZeroWeightInitialization : public WeightInitialization<Scalar> {
public:
	ZeroWeightInitialization(Scalar bias = 0) :
			bias(bias) { };
	void apply(Matrix<Scalar>& weights) const {
		weights.setZero(weights.rows(), weights.cols());
	};
private:
	Scalar bias;
};

template<typename Scalar>
class OneWeightInitialization : public WeightInitialization<Scalar> {
public:
	OneWeightInitialization(Scalar bias = 0) :
			bias(bias) { };
	void apply(Matrix<Scalar>& weights) const {
		weights.setOnes(weights.rows(), weights.cols());
		weights.row(weights.rows() - 1).setZero();
	};
private:
	Scalar bias;
};

template<typename Scalar>
class NDRandWeightInitialization : public WeightInitialization<Scalar> {
public:
	NDRandWeightInitialization(Scalar bias_value) :
			bias_value(bias_value) { };
	virtual ~NDRandWeightInitialization() = default;
	virtual void apply(Matrix<Scalar>& weights) const {
		int rows = weights.rows();
		int cols = weights.cols();
		std::default_random_engine gen;
		std::normal_distribution<Scalar> dist(0, sd(rows -  1, cols));
		for (int i = 0; i < rows; i++) {
			if (i == rows - 1) { // Bias row.
				weights.row(i).setConstant(bias_value);
			} else {
				for (int j = 0; j < cols; j++)
					weights(i,j) = dist(gen);
			}
		}
	};
protected:
	Scalar bias_value;
	virtual Scalar sd(unsigned fan_ins, unsigned fan_outs) const = 0;
};

template<typename Scalar>
class LeCunWeightInitialization : public NDRandWeightInitialization<Scalar> {
public:
	LeCunWeightInitialization(Scalar bias_value = 0) :
		NDRandWeightInitialization<Scalar>::NDRandWeightInitialization(bias_value) { };
protected:
	Scalar sd(unsigned fan_ins, unsigned fan_outs) const {
		return sqrt(1.0 / (Scalar) fan_ins);
	};
};

template<typename Scalar>
class GlorotWeightInitialization : public NDRandWeightInitialization<Scalar> {
public:
	GlorotWeightInitialization(Scalar bias_value = 0) :
		NDRandWeightInitialization<Scalar>::NDRandWeightInitialization(bias_value) { };
protected:
	Scalar sd(unsigned fan_ins, unsigned fan_outs) const {
		return sqrt(2.0 / (Scalar) (fan_ins + fan_outs));
	};
};

template<typename Scalar>
class HeWeightInitialization : public NDRandWeightInitialization<Scalar> {
public:
	HeWeightInitialization(Scalar bias_value = 0) :
		NDRandWeightInitialization<Scalar>::NDRandWeightInitialization(bias_value) { };
protected:
	Scalar sd(unsigned fan_ins, unsigned fan_outs) const {
		return sqrt(2.0 / (Scalar) fan_ins);
	};
};

template<typename Scalar>
class OrthogonalWeightInitialization : public WeightInitialization<Scalar> {
public:
	OrthogonalWeightInitialization(Scalar bias = 0, Scalar scale = 1) :
			bias(bias),
			scale(scale) { };
	void apply(Matrix<Scalar>& weights) const {
		int rows = weights.rows() - 1;
		int cols = weights.cols();
		bool more_rows = rows > cols;
		std::default_random_engine gen;
		std::normal_distribution<Scalar> dist(0, scale);
		Matrix<Scalar> ortho_weights(rows, cols);
		for (int i = 0; i < rows; i++) {
			for (int j = 0; j < cols; j++)
				ortho_weights(i,j) = dist(gen);
		}
		Eigen::BDCSVD<Matrix<Scalar>> svd;
		weights.block(0, 0, rows, cols) = more_rows ?
				svd.compute(ortho_weights, Eigen::ComputeFullU).matrixU().block(0, 0, rows, cols) :
				svd.compute(ortho_weights, Eigen::ComputeFullV).matrixV().block(0, 0, rows, cols);
		weights.row(rows).setConstant(bias);
	};
private:
	Scalar bias;
	Scalar scale;
};

// TODO: Orthogonal initialization.

} /* namespace cppnn */

#endif /* WEIGHTINITIALIZATION_H_ */
