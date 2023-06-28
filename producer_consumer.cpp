#include <pthread.h>
#include <unistd.h>
#include <atomic>
#include <iostream>
#include <vector>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t consumers_condition = PTHREAD_COND_INITIALIZER;
pthread_cond_t producer_condition = PTHREAD_COND_INITIALIZER;

int* shared_variable = nullptr;
bool ready_flag = false;
bool finish_flag = false;
bool debug_flag = false;

int max_sleep_time = 0;

int run_threads(size_t num_consumers, int max_sleep,
    std::vector<int>& input_values, bool debug);

int get_tid() {
    static std::atomic<int> unique_tid{ 0 };
    thread_local int* tid;
    if (tid == nullptr) {
        unique_tid++;
        tid = new int(unique_tid);
    }
    return *tid;
}

void* producer_routine(void* arg) {
    (void)arg;
    auto data = (std::vector<int>*)arg;
    if (data->empty()) {
        pthread_mutex_lock(&mutex);
        ready_flag = true;
        finish_flag = true;
        pthread_mutex_unlock(&mutex);
        pthread_cond_signal(&consumers_condition);
        return nullptr;
    }
    for (size_t i = 0; i < data->size(); i++) {
        pthread_mutex_lock(&mutex);
        while (ready_flag) {
            pthread_cond_wait(&producer_condition, &mutex);
        }
        if (shared_variable == nullptr) {
            shared_variable = new int;
        }
        *shared_variable = (*data)[i];
        ready_flag = true;
        if (i == data->size() - 1) {
            finish_flag = true;
        }
        pthread_mutex_unlock(&mutex);
        pthread_cond_signal(&consumers_condition);
    }
    return nullptr;
}

void* consumer_routine(void* arg) {
    (void)arg;
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, nullptr);

    int* result = new int(0);
    while (true) {
        pthread_mutex_lock(&mutex);
        while (!ready_flag) {
            pthread_cond_wait(&consumers_condition, &mutex);
        }
        if (finish_flag && shared_variable == nullptr) {
            pthread_mutex_unlock(&mutex);
            pthread_cond_signal(&consumers_condition);
            break;
        }
        *result += *shared_variable;
        if (finish_flag) {
            delete shared_variable;
            shared_variable = nullptr;
            pthread_mutex_unlock(&mutex);
            pthread_cond_signal(&consumers_condition);
            break;
        }
        if (debug_flag) {
            std::cout << "Thread: " << get_tid() << "; Partial result: " << *result
                << ";" << std::endl;
        }
        ready_flag = false;
        pthread_mutex_unlock(&mutex);
        pthread_cond_signal(&producer_condition);

        usleep((rand() % (max_sleep_time + 1)) * 1000);
    }
    return result;
}

void* consumer_interruptor_routine(void* arg) {
    (void)arg;
    auto consumers = (std::vector<pthread_t>*)arg;

    while (!finish_flag) {
        int random_consumer = rand() % (*consumers).size();
        pthread_cancel((*consumers)[random_consumer]);
    }
    return nullptr;
}

int run_threads(size_t num_consumers, int max_sleep,
    std::vector<int>& input_values, bool debug) {
    debug_flag = debug;
    max_sleep_time = max_sleep;

    pthread_t producer;
    pthread_t interruptor;
    std::vector<pthread_t> consumers(num_consumers);
    std::vector<int> partial_sums(num_consumers);

    pthread_create(&producer, nullptr, producer_routine, &input_values);
    for (size_t i = 0; i < num_consumers; i++) {
        pthread_create(&consumers[i], nullptr, consumer_routine, nullptr);
    }
    pthread_create(&interruptor, nullptr, consumer_interruptor_routine,
        &consumers);

    pthread_join(producer, nullptr);
    pthread_join(interruptor, nullptr);

    for (size_t i = 0; i < consumers.size(); i++) {
        int** sum = new int*;
        pthread_join(consumers[i], (void**)(sum));
        partial_sums[i] = **sum;
        delete* sum;
        delete sum;
    }

    int final_sum = 0;
    for (size_t i = 0; i < num_consumers; ++i) {
        final_sum += partial_sums[i];
    }

    return final_sum;
}