/*
 * test.cpp
 *
 *  Created on: Dec 10, 2017
 *      Author: Viktor Csomor
 */

#include <cmath>
#include <Dimensions.h>
#include <iostream>
#include <Layer.h>
#include <Loss.h>
#include <NeuralNetwork.h>
#include <Optimizer.h>
#include <Preprocessor.h>
#include <RegularizationPenalty.h>
#include <vector>
#include <WeightInitialization.h>

typedef double Scalar;

int main() {
	std::cout << "Number of threads: " << Eigen::nbThreads() << std::endl;
	cppnn::Tensor4<Scalar> data(5, 32, 32, 3);
	cppnn::Tensor4<Scalar> obj(5, 1, 1, 1);
	data = data.setRandom();
	obj = obj.setRandom();
//	cppnn::NormalizationPreprocessor<Scalar> preproc(true);
//	preproc.fit(data);
//	preproc.transform(data);
	cppnn::HeWeightInitialization<Scalar> init;
//	std::vector<cppnn::Layer<Scalar>*> layers(7);
//	layers[0] = new cppnn::ConvLayer<Scalar>(cppnn::Dimensions(32, 32, 3), init, 10);
//	layers[1] = new cppnn::LeakyReLUActivationLayer<Scalar>(layers[0]->get_output_dims());
//	layers[2] = new cppnn::MaxPoolingLayer<Scalar>(layers[1]->get_output_dims());
//	layers[3] = new cppnn::ConvLayer<Scalar>(layers[2]->get_output_dims(), init, 5, 5, 3, 3);
//	layers[4] = new cppnn::LeakyReLUActivationLayer<Scalar>(layers[3]->get_output_dims());
//	layers[5] = new cppnn::MaxPoolingLayer<Scalar>(layers[4]->get_output_dims());
//	layers[6] = new cppnn::FCLayer<Scalar>(layers[5]->get_output_dims(), 1, init);
//	cppnn::SequentialNeuralNetwork<Scalar> nn(layers);
	cppnn::Dimensions input_dims(32, 32, 3);
	std::vector<cppnn::ParallelNeuralNetwork<Scalar>> modules;
	modules.push_back(cppnn::ParallelNeuralNetwork<Scalar>(new cppnn::MaxPoolingLayer<Scalar>(input_dims, 2, 2)));
	modules.push_back(cppnn::ParallelNeuralNetwork<Scalar>(std::vector<cppnn::EmbeddedSequentialNeuralNetwork<Scalar>>({
			cppnn::EmbeddedSequentialNeuralNetwork<Scalar>(new cppnn::ConvLayer<Scalar>(modules[0].get_output_dims(), init, 5, 1, 1, 0)),
			cppnn::EmbeddedSequentialNeuralNetwork<Scalar>(std::vector<cppnn::Layer<Scalar>*>({
				new cppnn::ConvLayer<Scalar>(modules[0].get_output_dims(), init, 5, 1, 1, 0),
				new cppnn::ConvLayer<Scalar>(cppnn::Dimensions(16, 16, 5), init, 3)
			})),
			cppnn::EmbeddedSequentialNeuralNetwork<Scalar>(std::vector<cppnn::Layer<Scalar>*>({
				new cppnn::ConvLayer<Scalar>(modules[0].get_output_dims(), init, 5, 1, 1, 0),
				new cppnn::ConvLayer<Scalar>(cppnn::Dimensions(16, 16, 5), init, 4, 5, 1, 2)
			}))})));
	modules.push_back(cppnn::ParallelNeuralNetwork<Scalar>(new cppnn::MaxPoolingLayer<Scalar>(modules[1].get_output_dims(), 2, 2)));
	modules.push_back(cppnn::ParallelNeuralNetwork<Scalar>(new cppnn::LeakyReLUActivationLayer<Scalar>(modules[2].get_output_dims())));
	modules.push_back(cppnn::ParallelNeuralNetwork<Scalar>(new cppnn::FCLayer<Scalar>(modules[3].get_output_dims(), 1, init)));
	cppnn::InceptionNeuralNetwork<Scalar> nn(modules);
	nn.init();
	cppnn::QuadraticLoss<Scalar> loss;
	cppnn::ElasticNetRegularizationPenalty<Scalar> reg(5e-5, 1e-4);
	cppnn::NadamOptimizer<Scalar> opt(loss, reg, 20);
	std::cout << nn.to_string() << std::endl << std::endl;
	std::cout << opt.verify_gradients(nn, data, obj) << std::endl;
//	opt.optimize(nn, data, obj, 500);
	return 0;
};
