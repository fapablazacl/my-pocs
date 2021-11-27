
#include <iostream>
#include <vector>
#include <algorithm>
#include <cassert>
#include <cmath>

/**
 * Generate a random number between 0 and 1, inclusive.
 */
float random() {
    return static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
}

float sigmoid(const float value) {
    return 1.0f / ( 1.0f + std::exp(-value) );
}

float heaviside(const float value) {
    if (value > 0.0f) {
        return 1.0f;
    }

    return 0.0f;
}


class OrPerceptron {
public:
    float evaluate(const float input1, const float input2) const {
        return sigmoid(input1*weights[0] + input2*weights[1] + bias*weights[2]);
    }

    void train() {
        for (size_t i=0; i<50; i++) {
            calibrate(0.0f, 0.0f, 0.0f);
            calibrate(0.0f, 1.0f, 1.0f);
            calibrate(1.0f, 0.0f, 1.0f);
            calibrate(1.0f, 1.0f, 1.0f);
        }
    }

private:
    /**
     * function that calibrates the weights (train the perceptron)
     */
    void calibrate(const float input1, const float input2, const float output) {
        // evaluate the perceptron
        const float temp_output = evaluate(input1, input2);

        // compute the relative error
        const float error = output - temp_output;
    
        // calibrate the weights, based on the error and the learning rate
        weights[0] += error * input1 * lr;
        weights[1] += error * input2 * lr;
        weights[2] += error * bias * lr;
    }

private:
    // bias
    const float bias = 1.0f;

    // learning rate
    const float lr = 1.0f;

    // weights
    std::vector<float> weights = { random(), random(), random() };
};

int main() {
    std::srand(1);

    OrPerceptron perceptron;

    perceptron.train();

    std::cout << "(0.0, 0.0) => " << perceptron.evaluate(0.0f, 0.0f) << std::endl;
    std::cout << "(0.0, 1.0) => " << perceptron.evaluate(0.0f, 1.0f) << std::endl;
    std::cout << "(1.0, 0.0) => " << perceptron.evaluate(1.0f, 0.0f) << std::endl;
    std::cout << "(1.0, 1.0) => " << perceptron.evaluate(1.0f, 1.0f) << std::endl;

    return 0;
}
