#pragma once
#include <vector>
using namespace std;

// the declaration of run threads can be changed as you like
int run_threads(size_t num_consumers, int max_sleep, vector<int>& input_values, bool debug);
