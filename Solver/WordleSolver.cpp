#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <set>
#include <vector>
#include <algorithm>

using namespace std;

void printUsage(){
    cout << "Usage: WordleSolver.exe <pathToDictionaryFile>" << endl << endl;
}

string toString(set<string> strings) {
    stringstream result;
    for_each(begin(strings), end(strings), [&](auto str) {
            result << str << " ";
        });
    result << " ";
    return result.str();
}

bool containsAny(string str, vector<char> chars) {
    auto itr = begin(chars);
    while (itr != end(chars)) {
        if (str.find(*itr) != str.npos) {
            return true;
        }
        ++itr;
    }
    return false;
}

int main(int argc, char** argv){
    //cout << argc << endl;
    if (argc != 2){
        printUsage();
        return 0;
    }
    //cout << "test" << endl;

    const int WORDLE_SIZE = 5;
    const bool DEBUG = true;
    const int DEBUG_LEVEL = 1;

    string inputFilePath = argv[1];

    cout << "WordleSolver" << endl << endl;
    cout << "Using input dictionary file: " << inputFilePath << endl;

    string line;
    ifstream inputFile(inputFilePath);

    set<string> validWords;

    // populate possible solutions.
    if (inputFile.is_open()){
        while(getline(inputFile, line)){
            validWords.insert(line);
        }
        inputFile.close();
    } else {
        cerr << "Unable to open file: " << inputFilePath << endl;
        return -1;
    }

    cout << "There are " << validWords.size() << " possible solutions." << endl;
    if (DEBUG && DEBUG_LEVEL >= 2) {
        cout << toString(validWords) << endl;
    }

    // using user-defined rules, narrow down possible solutions.
    string currentRule;

    currentRule = "Limit to length 5";
    // first, narrow down the dictionary to word possibilities of matching length.
    {
        auto itr = begin(validWords);
        while (itr != end(validWords)) {  // this could be a helper function.
            auto str = *itr;
            if (str.length() != WORDLE_SIZE) { // this could be a fn pointer
                itr = validWords.erase(itr);
            }
            else {
                ++itr;
            }
        };
        cout << "After applying rule \"" << currentRule << "\" There are " << validWords.size() << " possible solutions." << endl;
        if (DEBUG && DEBUG_LEVEL >= 1) {
            cout << toString(validWords) << endl;
        }
    }

    currentRule = "Last letter should be Y";
    // Next, find words that match a known letter formatting.
    {
        auto itr = begin(validWords);
        while (itr != end(validWords)) {  // this could be a helper function.
            auto str = *itr;
            if (str[str.length() - 1] != 'y') { // this could be a fn pointer
                itr = validWords.erase(itr);
            }
            else {
                ++itr;
            }
        };
        cout << "After applying rule \"" << currentRule << "\" There are " << validWords.size() << " possible solutions." << endl;
        if (DEBUG && DEBUG_LEVEL >= 1) {
            cout << toString(validWords) << endl;
        }
    }

    currentRule = "Do not contain U,S,T,A,I,E,W,D";
    // Next, filter words that contain known excluded letters
    {
        vector<char> excluded({ 'u', 's', 't', 'a', 'i', 'e','w','d'});
        auto itr = begin(validWords);
        while (itr != end(validWords)) {  // this could be a helper function.
            auto str = *itr;
            if (containsAny(str, excluded)) { // this could be a fn pointer
                itr = validWords.erase(itr);
            }
            else {
                ++itr;
            }
        };
        cout << "After applying rule \"" << currentRule << "\" There are " << validWords.size() << " possible solutions." << endl;
        if (DEBUG && DEBUG_LEVEL >= 1) {
            cout << toString(validWords) << endl;
        }
    }

    currentRule = "Contains R only at index 1";
    // Next, filter words that contain known excluded letters
    {
        auto itr = begin(validWords);
        while (itr != end(validWords)) {  // this could be a helper function.
            auto str = *itr;
            if (!(str[1] == 'r' && str[0] != 'r' && str[2] != 'r' && str[3] != 'r' && str[4] != 'r')) { // this could be a fn pointer
                itr = validWords.erase(itr);
            }
            else {
                ++itr;
            }
        };
        cout << "After applying rule \"" << currentRule << "\" There are " << validWords.size() << " possible solutions." << endl;
        if (DEBUG && DEBUG_LEVEL >= 1) {
            cout << toString(validWords) << endl;
        }
    }

    currentRule = "Contains O, but not at index 1";
    // Next, filter words that contain known excluded letters
    {
        auto itr = begin(validWords);
        while (itr != end(validWords)) {  // this could be a helper function.
            auto str = *itr;
            if (containsAny(str, vector<char>({ 'o' })) && str[1] != 'o') { // this could be a fn pointer
                ++itr;
            }
            else {
                itr = validWords.erase(itr);
            }
        };
        cout << "After applying rule \"" << currentRule << "\" There are " << validWords.size() << " possible solutions." << endl;
        if (DEBUG && DEBUG_LEVEL >= 1) {
            cout << toString(validWords) << endl;
        }
    }


    return 0;
}