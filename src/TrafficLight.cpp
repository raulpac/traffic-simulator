#include <iostream>
#include <random>
#include "TrafficLight.h"
#include <chrono>

/* Implementation of class "MessageQueue" */


//template <typename T>
TrafficLightPhase MessageQueue::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function. 

    //create unique_lock to modify
    std::unique_lock<std::mutex> ulock(_mutex);
    //pass unique_lock to condition variable
    _cond.wait(ulock, [this] { return !_messages.empty(); });
    // save last queue element to return it and pass ownership after removing it from queue
    TrafficLightPhase msg = std::move(_messages.back());
    //remove last element from queue
    _messages.pop_back();
    //return last element
    return msg; // will not be copied due to return value optimization (RVO) in C++
}

//template <typename T>

void MessageQueue::send(TrafficLightPhase &&msg)
    {   
        // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
        // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.

        //create lock to modify _messages queue dynamic array
        std::lock_guard<std::mutex> uLock(_mutex);
        //move message to _messages queue
        _messages.push_back(std::move(msg));
        //notify cliente after pusshing new message into queue
        _cond.notify_one();
    }

/* Implementation of class "TrafficLight" */


TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
    while(true)
    {
        if(_queue.receive() == TrafficLightPhase::green)
        {
            break;
        }
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. 
    //To do this, use the thread queue in the base class. 

    //threads belong to TrafficObject class and holds all threads that have been launched within this object (TrafficObject)
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));

}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 

    // start time measurement
    // Defining time variables AND take first time meassure.
    std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
    std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();

    
    while(true)
    {
        double cycleDuration = rand() % 3 + 4; // Random numbers between 4 and 6. rand()%(max-min + 1) + min 
        
        // Calculate duration time between two time measurements.
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>( t2 - t1 ).count();
        
        if(duration>=cycleDuration){
            //toggles _currentPhase between green and red
            if(_currentPhase == TrafficLightPhase::red){
                _currentPhase = TrafficLightPhase::green;
                _queue.send(std::move(_currentPhase));
            }else
            {
                _currentPhase = TrafficLightPhase::red;
            }
            
            //Update timer 1 after a change of _currentPhase
            t1 = std::chrono::high_resolution_clock::now();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1)); 

        //Update timer 2 to update duration in next cycle.
        //wait for 1 millisecond
        t2 = std::chrono::high_resolution_clock::now();

    }

}

