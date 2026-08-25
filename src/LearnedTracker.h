#ifndef LEARNEDTRACKER_H
#define LEARNEDTRACKER_H

#include <iostream>
#include <cstdint>

class LearnedTracker
{
    private:
        uint32_t learnCt = 0;

    public:
        void incrCt(bool value)
        {
            learnCt = ((learnCt << 1) | value) & 0x07;
        }
        bool isLearned() 
        {
            return learnCt == 0x07;
        }   
        void reset()
        {
            learnCt = 0;
        }
};

#endif