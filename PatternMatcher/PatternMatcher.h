#include <unordered_map>
#include <set>
#include <vector>
#include <string>
#include "../DFA/DFA.h"

#ifndef SYNTAX_DIRECTED_TRANSLATOR_PATTERNMATCHER_H
#define SYNTAX_DIRECTED_TRANSLATOR_PATTERNMATCHER_H

using namespace std;

class PatternMatcher
{
public:
    explicit PatternMatcher(unordered_map<int, DFA::State> minimizedDfa, int startState);

    unordered_map<int, DFA::State> getDfa();

    void setDfa(unordered_map<int, DFA::State> minimizedDfa);

    vector<pair<string, string>> matchExpression(string expression);

private:
    unordered_map<int, DFA::State> dfa;

    int startState;

    static int getNextTransition(const unordered_map<string, set<int>> &transitions, const string &s);

    static void modifyAcceptor(const DFA::State &currentState, DFA::State &acceptor, int &counter, bool &acceptorIsPresent);
};

#endif // SYNTAX_DIRECTED_TRANSLATOR_PATTERNMATCHER_H
