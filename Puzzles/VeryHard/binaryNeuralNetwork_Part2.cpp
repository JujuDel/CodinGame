#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <random>
#include <algorithm>

using namespace std;

double sigmoid(const double x) {
    return 1. / (1. + exp(-x));
}

class XavierRandom {
public:
    XavierRandom() : distribution{ 0., 1. }
    {}

    double rand(const int sizePrev) {
        double number = distribution(generator);
        return number * sqrt(1. / sizePrev);
    }

private:
    std::default_random_engine generator;
    std::normal_distribution<double> distribution;
};

class Matrix {
public:
    Matrix(const int prevSize = 0, const int currSize = 0) :
        width{ currSize }
    {
        for (int i = 0; i < currSize * prevSize; ++i) {
            data.push_back(0);
        }
    }

    void set(const int prev, const int curr, const double value) {
        data[curr * width + prev] = value;
    }

    double get(const int prev, const int curr) const {
        return data[curr * width + prev];
    }

    string size() const {
        string size = to_string(width);
        size += " x ";
        size += (width == 0 ? "0" : to_string(int(data.size() / width)));
        return ("[" + size + "]");
    }

private:
    const int width;
    vector<double> data;
};

typedef vector<double> Layer;

class NN {
public:
    NN(const int inputs, const int hiddenLayers, const int outputs) :
        lr{ 0.5 },
        initBias( (double)rand() / RAND_MAX )
    {
        Layer ins;
        for (int i = 0; i < inputs; ++i) {
            ins.push_back(0);
        }
        layers.push_back(ins);
        
        for (int i = 0; i < hiddenLayers; ++i) {
            layers.push_back({});
        }

        Layer outs;
        for (int i = 0; i < outputs; ++i) {
            outs.push_back(0);
        }
        layers.push_back(outs);
    }

    void setHiddenLayer(const int idx, const int size) {
        Layer hs;
        for (int i = 0; i < size; ++i) {
            hs.push_back(0);
        }
        layers[idx + 1] = hs;
    }

    void init() {
        weights.push_back({});
        biases.push_back({});
        
        for (int k = 1; k < layers.size(); ++k) {
            Matrix ws(layers[k - 1].size(), layers[k].size());
            vector<double> bs;
            for (int h = 0; h < layers[k].size(); ++h) {
                for (int i = 0; i < layers[k - 1].size(); ++i) {
                    ws.set(i, h, xavier.rand(layers[k - 1].size()));
                }
                // Bias node
                bs.push_back(initBias);
            }
            weights.push_back(ws);
            biases.push_back(bs);
        }
    }

    vector<double> forward(const vector<double>& f_input, const bool verbose = false) {
        for (int i = 0; i < f_input.size(); ++i) {
            layers[0][i] = f_input[i];
        }
        if (verbose) {
            cerr << "Forward: " << endl;
            cerr << "     In: ";
            for (const double& o : layers[0]) cerr << o;
            cerr << endl;
        }

        for (int l = 1; l < layers.size(); ++l) {
            for (int neuron = 0; neuron < layers[l].size(); neuron++) {
                double res = biases[l][neuron];
                for (int prev = 0; prev < layers[l - 1].size(); ++prev) {
                    res += layers[l-1][prev] * weights[l].get(prev, neuron);
                }

                layers[l][neuron] = sigmoid(res);
            }
        }
        if (verbose) {
            cerr << "    Out: ";
            for (const double& o : layers[layers.size() - 1]) cerr << o;
            cerr << endl;
        }
        return layers[layers.size() - 1];
    }

    void train(const vector<double>& f_input, const vector<double>& f_label, const bool verbose = false) {
        forward(f_input, verbose);

        if (verbose) {
            cerr << "Training: " << endl;
            cerr << "  Target: ";
            for (const double& o : f_label) cerr << o;
            cerr << endl;
        }

        // Error output
        const int outLayerIdx = layers.size() - 1;
        for (int o = 0; o < layers[outLayerIdx].size(); ++o) {
            double Oo = layers[outLayerIdx][o];
            // Err value
            double value = Oo * (1. - Oo) * (Oo - f_label[o]);
            layers[outLayerIdx][o] = value;
            // Update Bias
            biases[outLayerIdx][o] -= lr * value;
            // Update Weights
            for (int left = 0; left < layers[outLayerIdx - 1].size(); ++left) {
                double valueW = weights[outLayerIdx].get(left, o) - lr * value * layers[outLayerIdx - 1][left];
                weights[outLayerIdx].set(left, o, valueW);
            }
        }

        // For all layers
        for (int l = layers.size() - 2; l > 0; --l) {
            // For all neurons
            for (int j = 0; j < layers[l].size(); ++j) {
                double sigma = 0;
                for (int right = 0; right < layers[l + 1].size(); right++) {
                    sigma += layers[l + 1][right] * weights[l + 1].get(j, right);
                }
                sigma = layers[l][j] * (1. - layers[l][j]) * sigma;
                // Err value
                layers[l][j] = sigma;
                // Update bias
                biases[l][j] -= lr * layers[l][j];
                for (int left = 0; left < layers[l - 1].size(); ++left) {
                    double valueW = weights[l].get(left, j) - lr * sigma * layers[l - 1][left];
                    weights[l].set(left, j, valueW);
                }
            }
        }
    }

    void debug() const {
        cerr << "------------------MODEL------------------" << endl << endl;

        cerr << "Biases:" << endl;
        for (int i = 0; i < biases.size(); ++i) {
            cerr << " " << i << " - [1 x " << biases[i].size() << "]: ";
            for (const double& b : biases[i]) cerr << b << " ";
            cerr << endl;
        }
    
        cerr << "Weights:" << endl;
        cerr << " 0 - " << weights[0].size() << ":" << endl;
        for (int i = 1; i < weights.size(); ++i) {
            cerr << " " << i << " - " << weights[i].size() << ":" << endl;
            cerr << "    ";
            for (int prev = 0; prev < layers[i-1].size(); prev++) {
                for (int curr = 0; curr < layers[i].size(); curr++) cerr << weights[i].get(prev, curr) << " ";
                cerr << endl << "    ";
            }
            cerr << endl;
        }
        cerr << "-----------------------------------------" << endl;
    }
        
private:
    vector<Layer> layers;
    vector<Matrix> weights;
    vector<Layer> biases;
    
    double lr;

    XavierRandom xavier;

    double initBias;
};

int main()
{
    NN model(8, 1, 8);
    model.setHiddenLayer(0, 8);

    model.init();

    int tests;
    int trainingSets;
    cin >> tests >> trainingSets; cin.ignore();

    vector<vector<double>> testSet;
    for (int i = 0; i < tests; i++) {
        string testInputs;
        cin >> testInputs; cin.ignore();
        vector<double> test;
        for (const auto& c : testInputs) test.push_back(double(c) - double('0'));
        testSet.push_back(test);
    }

    vector<pair<vector<double>, vector<double>>> trainSet;
    for (int i = 0; i < trainingSets; i++) {
        string trainingInputs;
        string expectedOutputs;
        cin >> trainingInputs >> expectedOutputs; cin.ignore();
        vector<double> train;
        for (const auto& c : trainingInputs) train.push_back(double(c) - double('0'));
        vector<double> label;
        for (const auto& c : expectedOutputs) label.push_back(double(c) - double('0'));
        trainSet.push_back({ train, label });
    }

    for (int i = 0; i < 150; i++) {
        for (int t = 0; t < trainingSets; t++) {
            model.train(trainSet[t].first, trainSet[t].second);
        }
    }

    for (int i = 0; i < tests; i++) {
        vector<double> predict = model.forward(testSet[i], false);
        for (const auto& p : predict) {
            cout << int(p + .5);
        }
        cout << endl;
    }
}
