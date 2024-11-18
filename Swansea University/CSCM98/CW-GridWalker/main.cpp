//To be completed:
// Student id:  2374690
// Student name:Priyanshu Dubey
// date:        14/11/2023


#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

#define N 20
#define S 10
#define MAX_WALKERS_PER_LOCATION 3
#define MAX_WALKERS_PER_EDGE 4
#define VERTEX_SLEEP_TIME 1
#define EDGE_SLEEP_TIME 1

// Mutexes for synchronization
std::mutex locationMutex;  // For synchronizing location updates
std::mutex edgeMutex;      // For synchronizing edge crossings

// Structure representing each walker
typedef struct _Walker
{
    int currentX, currentY, finalX, finalY;
    bool hasArrived;
    void Init()
    {
        currentX = rand() % S;
        currentY = rand() % S;

        // Ensure that the final destination is within bounds
        do {
            finalX = rand() % S;
            finalY = rand() % S;
        } while (currentX == finalX && currentY == finalY);  // Ensure start and end positions are different

        hasArrived = false;
    }

    bool RandomWalk(int& newX, int& newY, int& edgeIndex)
    {
        int direction = rand() % 2; // Pick a random direction (0: horizontal, 1: vertical)
        int sign = (rand() % 2) ? +1 : -1; // Random step, either +1 or -1

        // Moving vertically
        if (direction == 1) {
            newX = currentX;
            newY = currentY + sign;
            edgeIndex = (S * (S - 1)) + newX * (S - 1) + newY + (sign > 0 ? -1 : 0);
        }
        // Moving horizontally
        else {
            newX = currentX + sign;
            newY = currentY;
            edgeIndex = newY * (S - 1) + newX + (sign > 0 ? -1 : 0);
        }

        // Check if the new position is within bounds
        if (newX < 0 || newX >= S || newY < 0 || newY >= S) {
            std::cout << "Walker out of bounds! Current position: (" << currentX << ", " << currentY << ")\n";
            return false;  // Out of bounds, return false
        }

        return true;  // Valid move
    }
} Walker;

// Global grid arrays for tracking walker locations and final destinations
int originalGridCount[S][S];
int finalGridCount[S][S];
int obtainedGridCount[S][S];
Walker walkers[N];

// Helpers for thread synchronization
void Lock(std::mutex* m, int t = VERTEX_SLEEP_TIME)
{
    m->lock();
    std::this_thread::sleep_for(std::chrono::milliseconds(t));  // Sleep to simulate execution time and increase lock contention
}

void Unlock(std::mutex* m)
{
    m->unlock();
}

// Simulate the time spent while crossing an edge or waiting at a location
void CrossTheStreet()
{
    std::this_thread::sleep_for(std::chrono::milliseconds(EDGE_SLEEP_TIME));
}

void WaitAtLocation()
{
    std::this_thread::sleep_for(std::chrono::milliseconds(VERTEX_SLEEP_TIME));
}

// Print the grid
void PrintGrid(std::string message, int grid[S][S])
{
    std::cout << message;
    for (int i = 0; i < S; i++)
    {
        for (int j = 0; j < S; j++)
            std::cout << (grid[i][j] < 10 ? "  " : " ") << grid[i][j];
        std::cout << "\n";
    }
}

// Set the obtained grid with walkers' final positions
void SetObtainedGrid()
{
    for (int i = 0; i < N; i++)  // Set walkers' final locations
    {
        obtainedGridCount[walkers[i].currentY][walkers[i].currentX]++;
        if (!walkers[i].hasArrived)
        {
            std::cout << "\nAt least one walker has not arrived!\n";
            return;
        }
    }
}

// Compare the original grid with the obtained grid
void CompareGrids(int a[S][S], int b[S][S])
{
    for (int i = 0; i < S; i++)
        for (int j = 0; j < S; j++)
            if (a[i][j] != b[i][j])
            {
                std::cout << "\nError: results are different!\n";
                return;
            }
    std::cout << "\nSeems to be OK!\n";
}

// Thread function for each walker
void WalkerI(int id)
{
    int newX, newY, edgeIndex;

    while (!walkers[id].hasArrived ) { 
        std::cout << "Walker " << id << "is moving.\n";

        if (walkers[id].RandomWalk(newX, newY, edgeIndex)) {
            Lock(&locationMutex, VERTEX_SLEEP_TIME);
            Lock(&edgeMutex, EDGE_SLEEP_TIME);

            walkers[id].currentX = newX;
            walkers[id].currentY = newY;

            WaitAtLocation();
            CrossTheStreet();

            // Unlock after moving
            Unlock(&locationMutex);
            Unlock(&edgeMutex);

            if (walkers[id].currentX == walkers[id].finalX && walkers[id].currentY == walkers[id].finalY) {
                walkers[id].hasArrived = true;
                std::cout << "Walker " << id << " has arrived at destination.\n";
            }
        }
        else {  
            std::cout << "Walker " << id << " cannot move (out of bounds)."  << "\n";
        }
    }
    if (!walkers[id].hasArrived) {
        std::cout << "Walker " << id << " could not reach its destination."  << " \n";
    }
}

// Initialize the game with random walker positions
void InitGame()
{
    for (int i = 0; i < S; i++)
        for (int j = 0; j < S; j++)
            originalGridCount[i][j] = finalGridCount[i][j] = obtainedGridCount[i][j] = 0; // Initializing grids

    // Initialize walkers' locations
    for (int i = 0; i < N; i++) {
        do walkers[i].Init();
        while (originalGridCount[walkers[i].currentY][walkers[i].currentX] >= MAX_WALKERS_PER_LOCATION);  // Repeat init until condition is met
        originalGridCount[walkers[i].currentY][walkers[i].currentX]++;
    }

    // Initialize walkers' final destinations
    for (int i = 0; i < N; i++)
        finalGridCount[walkers[i].finalY][walkers[i].finalX]++;
}

// Main function
int main()
{
    InitGame();
    std::vector<std::thread> threads;

    for (int i = 0; i < N; ++i) {
        threads.push_back(std::thread(WalkerI, i));  
    }
    for (auto& t : threads) {
        t.join();
    }

    // Print the grids and compare results
    PrintGrid("Original locations:\n\n", originalGridCount);
    PrintGrid("\n\nIntended Result:\n\n", finalGridCount);
    SetObtainedGrid();
    PrintGrid("\n\nObtained Result:\n\n", obtainedGridCount);
    CompareGrids(finalGridCount, obtainedGridCount);
    return 0;
}
