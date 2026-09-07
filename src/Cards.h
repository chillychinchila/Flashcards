#ifndef CARDS_H
#define CARDS_H
#include "LearnedTracker.h"

#include <string>
#include <vector>
#include <iostream>
//#include <cctype>

using namespace std;

extern int idCt;

struct Card 
{
    string front;
    string back;
    int id;
    LearnedTracker tracker;

    void resetTracker()
    {
        tracker.reset();
    }

//custom bool operator 
    bool operator==(const string& answer) const;
};

Card newCard(string term, string def);
string toLower (string str);
int LDistance(const string& s1, const string& s2);
void logResult(bool correct, Card& card);


// bool FrontAnswer(string answer, const Card& card)
// {
//     cout << "Correct!" << endl;
//     return answer == card.front;
// }

// bool BackAnswer(string answer, const Card& card)
// {
//     cout << "Correct!" << endl;
//     return answer == card.back;
// }

#endif
