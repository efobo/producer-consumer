#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <iostream>
#include <vector>
#include "producer_consumer.h"
#include "doctest.h"
#include <ctime>

using namespace std;

int get_tid();

TEST_CASE("Zero Data Test") {
  vector<int> data{};
  CHECK(run_threads(100, 0, data, false) == 0);
}

TEST_CASE("One Thread Test") {
    vector<int> data{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    CHECK(run_threads(1, 100, data, false) == 45);
}

TEST_CASE("Many Threads Test") {
  vector<int> data{0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
  CHECK(run_threads(3, 100, data, false) == 45);
}

TEST_CASE("Time One Thread Test") {
  vector<int> data{0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
  time_t before_time = time(nullptr); 
  run_threads(1, 1000, data, false);
  time_t after_time = time(nullptr);
  CHECK(after_time - before_time <= 10);
}

TEST_CASE("Time Many Threads Test") {
  vector<int> data{0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
  time_t before_time = time(nullptr);
  run_threads(3, 1000, data, false);
  time_t after_time = time(nullptr);
  CHECK(after_time - before_time <= 4);
}

void* tid_test_routine (void* arg) {
    (void)arg;
    
    int* result = new int;
    *result = get_tid();

    return result;
}

TEST_CASE("Get Tid Test") {
    pthread_t thr1;
    pthread_t thr2;
    
    pthread_create(&thr1, nullptr, tid_test_routine, nullptr);
    pthread_create(&thr2, nullptr, tid_test_routine, nullptr);
    
    int** ret1 = new int*;
    int** ret2 = new int*;
    
    pthread_join(thr1, (void**)ret1);
    pthread_join(thr2, (void**)ret2);

    CHECK(((**ret1 != **ret2) && (**ret1 + **ret2 == 3)));

    delete *ret1;
    delete ret1;
    delete *ret2;
    delete ret2;
}
