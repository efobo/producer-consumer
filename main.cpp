#include <iostream>
#include <sstream>
#include <vector>
#include "producer_consumer.h"

using namespace std;

int main(int argc, char* argv[]) {
    vector<int> input_values;
    
    bool debug = false;
    int index = 1;
    if ((argc == 4) && string(argv[index]) == "--debug") {
        index++;
        debug = true;
    }
    size_t num_consumers = atoi(argv[index++]);
    int max_sleep = atoi(argv[index]);

    string str;
    int x;
    getline(cin, str);
    stringstream inp(str);
    while (inp >> x) {
        input_values.push_back(x);
    }

    int final_sum = run_threads(num_consumers, max_sleep, input_values, debug);

    cout << "Final sum: " << final_sum << endl;

    return 0;
}