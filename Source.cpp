#include <iostream>
#include <mutex>
#include <thread>
#include <vector>
#include <Windows.h>

// Constants
const int PULSE_MONITOR_INTERVAL_MS = 5000;
const int MAX_APPLICATION_WAIT_INTERVAL_MS = 10000; // Sleep duration for simulating application execution
const int MAX_WARNING_COUNT = 5;

// IS ALIVE signalling of watchdog
std::mutex g_heartbeatMutex;
bool g_applicationIsAlive = false;

void SendHeartbeatPulse();
void MainApplication();

//--------------------------------------------------------------------------------
// @name                : RunMainApplication
//
// @description         : Spawns the main application in a thread.
//--------------------------------------------------------------------------------
void RunMainApplication()
{
    std::thread mainApplicationThread(MainApplication);
    mainApplicationThread.detach();
}


//--------------------------------------------------------------------------------
// @name                : Main application
//
// @description         : This simulates the actual application. For simulation
//                        purpose a random sleep duration is used and the application
//                        closes randomly.
//--------------------------------------------------------------------------------
void MainApplication()
{
    srand(time(nullptr));
    std::cout << "\nApplication started" << std::endl;

    while (1 /* Continue forever */)
    {
        SendHeartbeatPulse();

        // Randomly value for this iteration
        int randomVal = rand() % MAX_APPLICATION_WAIT_INTERVAL_MS;
        std::cout << "Thread Id: " << GetCurrentThreadId() << " | Iteration duration: " << randomVal << " ms." << std::endl;

        int thresholdVal = 0.8 * MAX_APPLICATION_WAIT_INTERVAL_MS;
        if (randomVal > thresholdVal)
        {
            // This break simulates some crash or abnormal termination in the main application
            std::cout << "Application terminated" << std::endl;
            break;
        }

        Sleep(randomVal);
    }
}

//--------------------------------------------------------------------------------
// @name                : Main Watchdog
//
// @description         : This simulates the Watchdog application. It continuosly
//                        checks the pulse of the main application and shows a 
//                        warning if not received. The interval of monitoring is 
//                        controlled by PULSE_MONITOR_INTERVAL_MS
//
//                        After MAX_WARNING_COUNT attempts, it tries to restart
//                        the applicaiton.
//--------------------------------------------------------------------------------
void Watchdog()
{
    int warningCount = 0;

    std::cout << "\nWatchdog started" << std::endl;

    while (1 /* Continue forever */)
    {
        if (!g_applicationIsAlive)
        {
            // No pulse received
            warningCount++;
            if (warningCount > MAX_WARNING_COUNT)
            {
                std::cout << "Application has terminated" << std::endl;
                Sleep(1000);
                std::cout << "\nTrying to start application again..." << std::endl;

                RunMainApplication();
            }
            else
            {
                std::cout << "Watchdog: Application pulse not received #" << warningCount << std::endl;

            }
        }
        else
        {
            // Reset warning count
            warningCount = 0;

            g_heartbeatMutex.lock();
            g_applicationIsAlive = false;
            g_heartbeatMutex.unlock();
        }

        Sleep(PULSE_MONITOR_INTERVAL_MS);
    }
}

//--------------------------------------------------------------------------------
// @name                : Main SendHeartbeatPulse
//
// @description         : Sends the IS ALIVE signal. This should be called from
//                        the main application on each iteration.
//--------------------------------------------------------------------------------
void SendHeartbeatPulse()
{
    g_heartbeatMutex.lock();
    g_applicationIsAlive = true;
    g_heartbeatMutex.unlock();
}

//--------------------------------------------------------------------------------
// M A I N
//--------------------------------------------------------------------------------
int main()
{
    // Spawn Pulse monitoring thread
    std::thread watchdogThread(Watchdog);
    Sleep(100);

    // Spawn the main applicaiton
    RunMainApplication();

    watchdogThread.join();

    return 0;
}