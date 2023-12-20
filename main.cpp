#include "patternmatcher/PatternMatcher.h"
#include "NFA/NFA.h"
#include "DFA/DFA.h"
#include "Parser/parser.h"
#include <sstream>

using namespace std;

unordered_map<string, int>
getPriority(vector<pair<string, vector<string>>> exprs, const vector<string> &keywords,
            const vector<string> &punctuations);

NFA convert_exprs_postfix_to_NFA(const vector<pair<string, vector<string>>> &exprs, const vector<string> &keywords,
                                 const vector<string> &punctuations);

int main()
{
    string projectPath = R"(D:\E\Collage\Year_4_1\Compilers\Project\Syntax-Directed-Translator\)";
    string outPath = projectPath + R"(Outputs\)";

    vector<string> rules = Parser::get_rules_lines(projectPath + "lexical_rules.txt");

    vector<string> keywords_lines = Parser::get_keywords_lines(rules);
    vector<string> keywords = Parser::parse_keywords(keywords_lines);

    vector<string> punctuations_lines = Parser::get_punctuation_lines(rules);
    vector<string> punctuations = Parser::parse_keywords(punctuations_lines);

    vector<pair<string, string>> defs_lines = Parser::get_regular_def_lines(rules);
    unordered_map<string, vector<string>> defs = Parser::parse_defs(defs_lines);

    vector<pair<string, string>> expr_lines = Parser::get_regular_expr_lines(rules);
    vector<pair<string, vector<string>>> exprs = Parser::parse_expr(expr_lines, defs);
    exprs = Parser::convert_exprs_to_pos(exprs);

    unordered_map<string, int> priority = getPriority(exprs, keywords, punctuations);

    NFA exprs_nfa = convert_exprs_postfix_to_NFA(exprs, keywords, punctuations);
    exprs_nfa.toJSON(outPath + "NFA.json");
    cout << "NFA created successfully with size = " << exprs_nfa.states.size() << endl;

    unordered_map<int, DFA::State> dfa_states = DFA::constructDFA(exprs_nfa, priority);
    unordered_map<int, DFA::State> modified_dfa_states = DFA::processTransitions(dfa_states);

    // Open a file for writing
    ofstream outFile(outPath + "dfa_states_output.txt");

    // Check if the file is open
    if (!outFile.is_open())
    {
        cerr << "Error opening file for writing." << endl;
        return 1;
    }

    // Iterate through the DFA states map and write to the file
    for (const auto &entry : modified_dfa_states)
    {
        int dfa_state_id = entry.first;
        const DFA::State &dfa_state = entry.second;
        outFile << "DFA State ID: " << dfa_state_id << "\n";
        outFile << "NFA States: ";

        for (const int &nfa_state : dfa_state.nfa_states)
        {
            outFile << nfa_state << " ";
        }

        outFile << "\n";
        outFile << "Is Acceptance: " << (dfa_state.is_acceptance ? "true" : "false") << "\n";
        outFile << "Token: " << dfa_state.token << "\n";
        outFile << "Transitions:\n";
        for (const auto &transition : dfa_state.transitions)
        {
            const string &input_symbol = transition.first;
            const set<int> &target_states = transition.second;
            outFile << "  " << input_symbol << " -> ";
            for (const int &target_state : target_states)
            {
                outFile << target_state << " ";
            }
            outFile << "\n";
        }

        outFile << "\n";
    }

    // Close the file
    outFile.close();

    cout << "DFA states written to dfa_states_output.txt" << endl;
    unordered_map<int, DFA::State> minimzed_dfa_states = DFA::minimizeDFA(dfa_states, priority);

    size_t mapSize2 = minimzed_dfa_states.size();
    cout << "Size of the unordered_map: " << mapSize2 << endl;
    ofstream outFile2(outPath + "dfa_states_output2.txt");

    // Check if the file is open
    if (!outFile2.is_open())
    {
        cerr << "Error opening file for writing." << endl;
        return 1;
    }

    // Iterate through the DFA states map and write to the file
    for (const auto &entry : minimzed_dfa_states)
    {
        int dfa_state_id = entry.first;
        const DFA::State &dfa_state = entry.second;

        outFile2 << "DFA State Minimzed ID: " << dfa_state_id << "\n";
        outFile2 << "DFA States: ";
        for (const int &nfa_state : dfa_state.nfa_states)
        {
            outFile2 << nfa_state << " ";
        }
        outFile2 << "\n";
        outFile2 << "Is Acceptance: " << (dfa_state.is_acceptance ? "true" : "false") << "\n";
        outFile2 << "Token: " << dfa_state.token << "\n";

        outFile2 << "Transitions:\n";
        for (const auto &transition : dfa_state.transitions)
        {
            const string &input_symbol = transition.first;
            const set<int> &target_states = transition.second;

            outFile2 << "  " << input_symbol << " -> ";
            for (const int &target_state : target_states)
            {
                outFile2 << target_state << " ";
            }
            outFile2 << "\n";
        }

        outFile2 << "\n";
    }

    // Close the file
    outFile2.close();

    // get start state
    int startState = 0;
    for (const auto &entry : minimzed_dfa_states)
    {
        int dfa_state_id = entry.first;
        const DFA::State &dfa_state = entry.second;
        if (dfa_state.nfa_states.find(0) != dfa_state.nfa_states.end())
        {
            startState = dfa_state_id;
            break;
        }
    }

    PatternMatcher pm(minimzed_dfa_states, startState);

    string testProgram = "int sum , count , pass , mnt; while (pass !=\n"
                         "10)\n"
                         "{\n"
                         "pass = pass + 1 ;\n"
                         "}";

    vector<pair<string, string>> symbolTable = pm.matchExpression(testProgram);

    for (const auto &pair : symbolTable)
    {
        cout << "TOKEN = " << pair.first << " --- MATCHED PATTERN = " << pair.second << endl;
    }

    string testString = "int n = 3\n"
                        "float f = 56.7;\n"
                        "float f2 = 5.67E1\n"
                        "x x,x 5 n \n"
                        "if (f >50) { f = f2 / 2}\n"
                        "else { f = f2 * 2}";

    symbolTable = pm.matchExpression(testString);

    for (const auto &pair : symbolTable)
    {
        cout << "TOKEN = " << pair.first << " --- MATCHED PATTERN = " << pair.second << endl;
    }

    string test2 = "int x = 70 e b);";

    symbolTable = pm.matchExpression(test2);

    for (const auto &pair : symbolTable)
    {
        cout << "TOKEN = " << pair.first << " --- MATCHED PATTERN = " << pair.second << endl;
    }

    string test3 = "int x = 70&y;";

    symbolTable = pm.matchExpression(test3);

    for (const auto &pair : symbolTable)
    {
        cout << "TOKEN = " << pair.first << " --- MATCHED PATTERN = " << pair.second << endl;
    }

    string test4 = "boolean x = 0\n"
                   "boolean x = false ";

    symbolTable = pm.matchExpression(test4);

    for (const auto &pair : symbolTable)
    {
        cout << "TOKEN = " << pair.first << " --- MATCHED PATTERN = " << pair.second << endl;
    }

    return 0;
}

unordered_map<string, int>
getPriority(vector<pair<string, vector<string>>> exprs, const vector<string> &keywords,
            const vector<string> &punctuations)
{
    unordered_map<string, int> priority;

    for (const string &keyword : keywords)
    {
        priority[keyword] = 0;
    }
    for (const string &punctuation : punctuations)
    {
        priority[punctuation] = 0;
    }
    for (size_t i = 0; i < exprs.size(); i++)
    {
        priority[exprs[i].first] = i + 1;
    }
    return priority;
}

NFA convert_exprs_postfix_to_NFA(
    const vector<pair<string, vector<string>>> &exprs,
    const vector<string> &keywords,
    const vector<string> &punctuations)
{
    NFA res;
    for (const string &keyword : keywords)
    {
        res.processSymbol(keyword);
        NFA expr_nfa = res.pop();
        for (int state_i : expr_nfa.end_states) // set token of acceptance states
        {
            expr_nfa.states[state_i].token = keyword;
        }
        res.push(expr_nfa);
    }
    for (const string &punctuation : punctuations)
    {
        res.processSymbol(punctuation);
        NFA expr_nfa = res.pop();
        for (int state_i : expr_nfa.end_states) // set token of acceptance states
        {
            expr_nfa.states[state_i].token = punctuation;
        }
        res.push(expr_nfa);
    }
    for (const auto &expr : exprs)
    {
        for (const string &token : expr.second)
        {
            if (token == "*")
            {
                res.kleeneStar();
            }
            else if (token == "+")
            {
                res.positiveClosure();
            }
            else if (token == "|")
            {
                res.orOp();
            }
            else if (token == ".")
            {
                res.concatenate();
            }
            else
            {
                res.processSymbol(token);
            }
        }
        NFA expr_nfa = res.pop();
        for (int state_i : expr_nfa.end_states) // set token of acceptance states
        {
            if (expr_nfa.states[state_i].token.empty())
                expr_nfa.states[state_i].token = expr.first;
        }
        res.push(expr_nfa);
    }

    res.concatenateAllStack();
    return res;
}
