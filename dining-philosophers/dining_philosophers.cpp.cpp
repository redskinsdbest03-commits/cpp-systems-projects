#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <random>
#include <chrono>
#include <string>

using namespace std;

//each philosopher's state
enum class seat_state {thinking, hungry, eating};

//monitor structure to control access to utensils
struct table {
    int seats;                                   //number of philosophers and utensils
    vector<seat_state> state;                    //tracking each philosopher's state
    vector<condition_variable> gate;             //one condition per philosopher
    mutex lk;                                    //monitor lock

    //init table with all thinking
    table(int n): seats(n), state(n, seat_state::thinking), gate(n) {}

    //helper to find left and right neighbors
    int left_of(int i)  { return (i + seats - 1) % seats; }
    int right_of(int i) { return (i + 1) % seats; }

    //check if philosopher i can eat
    void try_start_eating(int i){
        //ok to eat only if hungry AND both neighbors not eating
        if(state[i] == seat_state::hungry &&
           state[left_of(i)] != seat_state::eating &&
           state[right_of(i)] != seat_state::eating){
            
            state[i] = seat_state::eating;
            gate[i].notify_one(); //wake philosopher i
        }
    }

    //attempt to pick up utensils
    void pick_up(int i){
        unique_lock<mutex> guard(lk);
        state[i] = seat_state::hungry;      //declare hunger
        try_start_eating(i);                //see if available
        while(state[i] != seat_state::eating){
            gate[i].wait(guard);            //wait until neighbors free
        }
    }

    //put utensils down + signal neighbors
    void put_down(int i){
        unique_lock<mutex> guard(lk);
        state[i] = seat_state::thinking;        //done eating can return to think
        try_start_eating(left_of(i));           //check if left can eat now
        try_start_eating(right_of(i));          //check if right can eat now
    }
};

static mutex io_lock; //sync printing so output stays readable

//utilityused to read integer from user when running interactively
static int read_int(const string& prompt){
    while(true){
        {
            lock_guard<mutex> guide(io_lock);
            cout<<prompt;
            cout.flush();
        }
        string line;
        if(!getline(cin, line)) continue;
        try{
            size_t p=0;
            int v = stoi(line, &p);
            if(p != line.size()) throw runtime_error("extra chars");
            return v;
        }catch(...){
            lock_guard<mutex> guide(io_lock);
            cout<<"invalid input, try again\n";
        }
    }
}

int main(int argc, char* argv[]){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    //intro and usage message
    {
        lock_guard<mutex> guide(io_lock);
        cout<<"rules:\n";
        cout<<"  * philosophers == utensils\n";
        cout<<"  * both must be greater than 4\n";
    }

    int philosopher_count = 0;
    int utensil_count = 0;

    //if user typed ./a8 value 1 value2 use them
    if(argc == 3){
        try{
            philosopher_count = stoi(argv[1]);
            utensil_count = stoi(argv[2]);
        }catch(...){
            cerr<<"error: arguments must be integers\n";
            return 2;
        }
    }
    //if user typed only ./a8 proceed to ask them
    else if(argc == 1){
        philosopher_count = read_int("enter number of philosophers (greater than 4): ");
        utensil_count    = read_int("enter number of utensils (must match): ");
    }
    else{
        cerr<<"usage: "<<argv[0]<<" <num_philosophers> <num_utensils>\n";
        cerr<<"or just "<<argv[0]<<" for interactive mode\n";
        return 1;
    }

    //validate inputs
    if(philosopher_count < 4 || utensil_count < 4){
        cerr<<"error: need at least 4 philosophers and 4 utensils\n";
        return 3;
    }
    if(philosopher_count != utensil_count){
        cerr<<"error: utensils must equal philosophers\n";
        return 4;
    }

    const int eat_target = 5; //each philosopher eats 5 meals before program ends

    table table(philosopher_count);
    vector<thread> workers;
    vector<atomic<int>> meals(philosopher_count);  //meal count per philosopher
    for(int i=0;i<philosopher_count;++i) meals[i] = 0;

    atomic<int> done = 0; //tracks completed philosophers

    //thread function for each philosopher
    auto routine = [&](int id){
        //random think/eat times different seed per philosopher
        mt19937_64 rng(id + (uint64_t)chrono::high_resolution_clock::now().time_since_epoch().count());
        uniform_int_distribution<int> think_ms(12, 45);
        uniform_int_distribution<int> eat_ms(12, 45);

        while(meals[id] < eat_target){
            this_thread::sleep_for(chrono::milliseconds(think_ms(rng))); //thinking time
            
            table.pick_up(id); //request chopsticks

            {
                lock_guard<mutex> guard(io_lock);
                cout<<"Philosopher "<<(id+1)<<" begins meal "<<(meals[id]+1)<<"\n";
            }

            this_thread::sleep_for(chrono::milliseconds(eat_ms(rng))); //eat time
            
            meals[id]++; //record meal
            table.put_down(id); //put utensils down

            {
                lock_guard<mutex> guard(io_lock);
                cout<<"Philosopher "<<(id+1)<<" ends meal "<<meals[id]<<"\n";
            }
        }
        done++;
    };

    //launch philosophers
    for(int i=0;i<philosopher_count;++i){
        workers.emplace_back(routine, i);
    }

    //wait for all to finish
    for(auto& t: workers) t.join();

    //final output summary
{
    lock_guard<mutex> guard(io_lock);
    cout<<"complete: all philosophers finished "<<eat_target<<" meals\n";
    cout<<"summary:\n";
    for(int i=0;i<philosopher_count;++i){
        cout<<"Philosopher "<<(i+1)<<": "<<meals[i].load()<<" meals\n";
    }
}


    return 0;
}
