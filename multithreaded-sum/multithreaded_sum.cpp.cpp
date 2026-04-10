// This program uses multiple threads to sum integers read from a file.
// Each thread computes a partial sum and the total is written

#include <iostream>
#include <fstream>
#include <vector>
#include <thread>
#include <mutex>
using namespace std;

//Global variables
long long totalSum = 0;
mutex sumLock;

//Function each thread will execute
void partialSum(const vector<int>& data, int start, int end) {
    long long localSum = 0;
    for (int i = start; i < end; ++i) {
        localSum += data[i];
    }

    //Updating the global sum
    lock_guard<mutex> guard(sumLock);
    totalSum += localSum;
}

int main(int argc, char* argv[]) {
    //Validating cmd line arguments
    if (argc != 3) {
        cerr << "Usage: " << argv[0] << " <num_threads> <input_file>\n";
        return 1;
    }

    int numThreads = stoi(argv[1]);
    string inputFile = argv[2];

    //Reading the file data
    ifstream inFile(inputFile);
    if (!inFile.is_open()) {
        cerr << "Error: Cannot open file '" << inputFile << "'\n";
        return 1;
    }

    int count;
    inFile >> count; //making the first element equal to the number of items following it
    vector<int> numbers(count);

    for (int i = 0; i < count; ++i) {
        if (!(inFile >> numbers[i])) {
            cerr << "Error: insufficient integers in file.\n";
            return 1;
        }
    }
    inFile.close();

    //Spliting Work Among Threads
    vector<thread> threads;
    int chunk = count / numThreads;
    int remainder = count % numThreads;
    int start = 0;

    for (int i = 0; i < numThreads; ++i) {
        int end = start + chunk + (i < remainder ? 1 : 0);
        threads.emplace_back(partialSum, cref(numbers), start, end);
        start = end;
    }

    for (auto& t : threads)
        t.join();


    cout << "Total Sum = " << totalSum << endl;

    return 0;
}
