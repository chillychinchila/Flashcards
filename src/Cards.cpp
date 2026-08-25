#include <iostream>
#include <algorithm>
#include <cctype>
#include <vector>
#include <string>
#include "Cards.h"

using namespace std;

int idCt = 0;   

string toLower(string str)
{
    transform(str.begin(), str.end(), str.begin(), [](unsigned char c) { return tolower(c); });
    return str;
}

int LDistance(const string& s1, const string& s2)
{
    int m = s1.size();
    int n = s2.size();
    vector<vector<int>> dp(m + 1, vector<int>(n + 1));

    for (int i = 0; i <= m; ++i)
        dp[i][0] = i;
    for (int j = 0; j <= n; ++j)
        dp[0][j] = j;

    for (int i = 1; i <= m; ++i)
    {
        for (int j = 1; j <= n; ++j)
        {
            if (s1[i - 1] == s2[j - 1])
                dp[i][j] = dp[i - 1][j - 1];
            else
                dp[i][j] = min({dp[i - 1][j] + 1, dp[i][j - 1] + 1, dp[i - 1][j - 1] + 1});
        }
    }

    return dp[m][n];
}

bool Card::operator==(const string& answer) const
{
    string lowerFront = toLower(front);
    string lowerBack = toLower(back);
    string lowerAnswer = toLower(answer);
   
    const int MAXTYPO = 2;

    bool frontMatch = LDistance(lowerAnswer, lowerFront) <= MAXTYPO;
    bool backMatch = LDistance(lowerAnswer, lowerBack) <= MAXTYPO;

    return (frontMatch || backMatch);
}

void logResult(bool correct, Card& card)
{
    card.tracker.incrCt(correct);
    if(card.tracker.isLearned())
    {
        cout << "Card " << card.id << " is learned!" << endl;
    }
}

Card newCard(string term, string def)
{
    Card c;
    c.front = term;
    c.back = def;
    c.id = idCt++;
    return c;
}
