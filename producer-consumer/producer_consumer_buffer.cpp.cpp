#include <iostream>
#include <thread>
#include <vector>
#include <queue>
#include <semaphore>
#include <mutex>
#include <chrono>
using namespace std;

//Shared buffer and synchronization
queue<int> sharedBuffer;
mutex bufferMutex;

//counts of filled and empty slots
counting_semaphore<1000> availableItems(0);
counting_semaphore<1000> emptySlots(0);

//Deterministic item IDs
int nextItemID = 0;
mutex idMutex;

//Per-thread work sizes
vector<int> producerSize;
vector<int> consumerSize;

//Header
static void printHeader(int np, int nc, int buf) {
    cout << "Number of Producers: " << np << "\n";
    cout << "Number of Consumers: " << nc << "\n";
    cout << "Buffer Size: " << buf << "\n\n";
}

//Producer thread
void producer(int producerID) {
    int size = producerSize[producerID];
    for (int i = 0; i < size; ++i) {
        //If buffer full, print exact message then block
        if (!emptySlots.try_acquire()) {
            cout << "Producers blocked!  Need to consume from buffer first!\n";
            emptySlots.acquire();
        }

        this_thread::sleep_for(chrono::milliseconds(1000)); //pacing for readability

        int item;
        { lock_guard<mutex> g(idMutex); item = ++nextItemID; }

        {
            lock_guard<mutex> g(bufferMutex);
            sharedBuffer.push(item);
            cout << "Producer " << producerID << " produced " << item << "\n";
        }

        availableItems.release();
    }
}

//Consumer thread 
void consumer(int consumerID) {
    int size = consumerSize[consumerID];
    for (int i = 0; i < size; ++i) {
        availableItems.acquire();

        int item;
        {
            lock_guard<mutex> g(bufferMutex);
            item = sharedBuffer.front();
            sharedBuffer.pop();
            cout << "Consumer " << consumerID << " consumed " << item << "\n";
        }

        emptySlots.release();
        this_thread::sleep_for(chrono::milliseconds(1000));
    }
}

int main(int argc, char* argv[]) {
    int numProducers = 0, numConsumers = 0, bufferSize = 0;

    //If all 3 args supplied upon running the program, use them, otherwise prompt user
    //Ex. ./assignment7 5 6 9 will run just akin to being individually asked numbers
    if (argc == 4) {
        try {
            numProducers = stoi(argv[1]);
            numConsumers = stoi(argv[2]);
            bufferSize   = stoi(argv[3]);
        } catch (...) {
            cerr << "Error: arguments must be integers.\n";
            return 1;
        }
    } else {
        cout << "Enter number of producers: ";
        cin >> numProducers;
        cout << "Enter number of consumers: ";
        cin >> numConsumers;
        cout << "Enter buffer size: ";
        cin >> bufferSize;
    }

    if (numProducers <= 0 || numConsumers <= 0 || bufferSize <= 0) {
        cerr << "Error: all values must be positive (> 0).\n";
        return 1;
    }

    const int TARGET_ITEMS = 10; //exactly 10 total items
    emptySlots.release(bufferSize);
    printHeader(numProducers, numConsumers, bufferSize);

    //Distribute 10 items evenly among threads
    producerSize.assign(numProducers, TARGET_ITEMS / numProducers);
    for (int i = 0; i < TARGET_ITEMS % numProducers; ++i) producerSize[i]++;

    consumerSize.assign(numConsumers, TARGET_ITEMS / numConsumers);
    for (int i = 0; i < TARGET_ITEMS % numConsumers; ++i) consumerSize[i]++;

    vector<thread> producers, consumers;
    producers.reserve(numProducers);
    consumers.reserve(numConsumers);

    for (int p = 0; p < numProducers; ++p) producers.emplace_back(producer, p);
    for (int c = 0; c < numConsumers; ++c) consumers.emplace_back(consumer, c);

    for (auto& t : producers) t.join();
    for (auto& t : consumers) t.join();

    //No footer ends on last produced/consumed event
    return 0;
}
