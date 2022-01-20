#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <format>
#include <list>
#include <set>
#include <queue>
#include <vector>
#include <algorithm>
#include <functional>
#include <filesystem>

using namespace std;

// Constants
const string    DEFAULT_DICTIONARY_NAME     = "words_alpha.txt";
const int       DEFAULT_WORDLE_LENGTH       = 5;

void printUsage(){
    cout << "Usage: WordleSolver.exe <pathToDictionaryFile>" << endl << endl;
}

// Utility Functions
// TODO: refactor to separate headers, these should extend class behavior via friends or something similar.
string toString(set<string> strings) {
    stringstream result;
    for_each(begin(strings), end(strings), [&](auto str) {
            result << str << " ";
        });
    result << " ";
    return result.str();
}

/*
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
*/

// Logging Helpers

const string STRING_SOLUTION_COUNT_FORMAT = "There are {} possible solutions.";
const string STRING_RULE_APPLICATION_FORMAT = "After applying rule {}";

// Solver Logic
int main(int argc, char** argv){
    const string DEFAULT_DICTIONARY_PATH = (filesystem::current_path() / filesystem::path(DEFAULT_DICTIONARY_NAME)).string();
    int WORDLE_SIZE = DEFAULT_WORDLE_LENGTH;
    string dictionary_path = DEFAULT_DICTIONARY_PATH;
    string wordle_guess_path;
    int DEBUG_LEVEL = 2;

    // Arg parsing...
    // TODO: if we ever need more parameters, then refactor to a separate function, utilize argument parsing libraries to make this more extensible.
    if (argc < 2){
        printUsage();
        return 0;
    }
    wordle_guess_path = argv[1];
    int current_arg = 2;
    while (current_arg < argc) {
        if (argv[current_arg] == "-d") {
            dictionary_path = argv[++current_arg];
        }
        if (argv[current_arg] == "-v") {
            DEBUG_LEVEL = 3; // TODO: we should have a nicer way of parsing/setting debug logging levels...
        }
        ++current_arg;
    }

    cout << "WordleSolver" << endl << endl;
    cout << "Using input dictionary file: " << dictionary_path << endl;
    cout << "Using guess file: " << wordle_guess_path << endl;

    // populate starting set of words.
    // contains all our intermediate solutions.
    set<string> possible_solutions;

    // populate possible solutions.
    //TODO: refactor to its own method.
    {
        string line;
        ifstream dictionary_file(dictionary_path);

        if (dictionary_file.is_open()) {
            while (getline(dictionary_file, line)) {
                possible_solutions.insert(line);
            }
            dictionary_file.close();
        }
        else {
            cerr << "Unable to open file: " << dictionary_path << endl;
            return -1;
        }
        if (DEBUG_LEVEL >= 1) {
            cout << "There are " << possible_solutions.size() << " possible solutions." << endl;
            if (DEBUG_LEVEL >= 4) {
                cout << toString(possible_solutions) << endl;
            }
        }
    }

    class WordleRule {
    public:
        string name; // what is the pretty name of this rule?

        function<bool(const string&)> validationFunc; //function pointer to logic that determines whether or not a given string is valid when checked by this rule.

        // for now, rules must be defined at creation.
        WordleRule(string name, function<bool(const string&)>inValidationFunc)
            :
            name(name),
            validationFunc(inValidationFunc)
        {}

        // for now, assume that rules with matching names are equivalent.
        bool operator ==(WordleRule& other) {
            return this->name.compare(other.name) == 0;
        }

        operator string() {
            return this->name;
        }
    };

    queue<shared_ptr<WordleRule>> unappliedRules;
    set<shared_ptr<WordleRule>> appliedRules;

    const string STRING_HAS_LENGTH_FORMAT = "Has length {}";

    unappliedRules.push(shared_ptr<WordleRule>(new WordleRule(format(STRING_HAS_LENGTH_FORMAT, WORDLE_SIZE)
        , [WORDLE_SIZE](const string& word) {
            return word.size() == WORDLE_SIZE;
        })));
    
    // populate our logical rules for guessing the word using the results of previous guesses.
    // TODO: this should ve refactored to a helper function for clarity
    {
        string line;
        string guess_word;
        string guess_result;
        ifstream wordle_guess_file(wordle_guess_path);

        auto generateRules = [](const string& guess_word, const string& guess_result) {
            // a guess result can only be three characters:
            // a b character = black, unused everywhere in the word.
            // a y character = yellow, used somewhere in the word, but not at this index.
            // a g character = green, used in the word at this index.

            const string STRING_DOES_NOT_CONTAIN_LETTER_RULE_FORMAT = "Does Not contain letter {}";
            const string STRING_DOES_CONTAIN_LETTER_RULE_FORMAT = "Does contain letter {}";
            const string STRING_DOES_NOT_CONTAIN_LETTER_AT_FORMAT = "Does Not contain letter {} at {}";
            const string STRING_DOES_CONTAIN_LETTER_AT_FORMAT = "Does contain letter {} at {}";
           
            list<shared_ptr<WordleRule>> rules;

            for (int i = 0; i < guess_result.size(); ++i) {
                char letter = guess_word[i];
                switch (guess_result[i]) {
                case 'b':
                    rules.push_back(shared_ptr<WordleRule>(new WordleRule(format(STRING_DOES_NOT_CONTAIN_LETTER_RULE_FORMAT, guess_word[i])
                        , [letter](const string& word) {
                            return word.find(letter) == word.npos;
                        })));
                    break;
                case 'y':
                    rules.push_back(shared_ptr<WordleRule>(new WordleRule(format(STRING_DOES_CONTAIN_LETTER_RULE_FORMAT, guess_word[i])
                        , [letter](const string& word) {
                            return word.find(letter) != word.npos;
                        })));
                    rules.push_back(shared_ptr<WordleRule>(new WordleRule(format(STRING_DOES_NOT_CONTAIN_LETTER_AT_FORMAT, guess_word[i], i)
                        , [letter, i](const string& word) {
                            return word[i] != letter;
                        })));
                    break;
                case 'g':
                    rules.push_back(shared_ptr<WordleRule>(new WordleRule(format(STRING_DOES_CONTAIN_LETTER_AT_FORMAT, guess_word[i], i)
                        , [letter, i](const string& word) {
                            return word[i] == letter;
                        })));
                    break;
                }
            }
            return rules;
        };

        if (wordle_guess_file.is_open()) {
            // the first line of a wordle guess file is the number of letters the wordle contains.
            getline(wordle_guess_file, line);
            WORDLE_SIZE = stoi(line);

            if (DEBUG_LEVEL > 1)
                cout << "Wordle is " << WORDLE_SIZE << " letters long." << endl;

            while (getline(wordle_guess_file, line)) {
                if (DEBUG_LEVEL >= 3) {
                    cout << "Reading input line: " << line << endl;
                }

                auto line_stream = stringstream(line);
                line_stream >> guess_word >> guess_result;
                if (DEBUG_LEVEL >= 3) {
                    cout << "Guess word: " << guess_word << " Guess result: " << guess_result << endl;
                }
                
                auto generatedRules = generateRules(guess_word, guess_result);
                for (auto itr = begin(generatedRules); itr != end(generatedRules); ++itr) {
                    unappliedRules.push(*itr);
                    if (DEBUG_LEVEL >= 1) {
                        cout << "Adding rule: " << itr->get()->name << endl;
                    }
                }
            }
            wordle_guess_file.close();
        }
        else {
            cerr << "Unable to open file: " << dictionary_path << endl;
            return -1;
        }
    }

    // TODO: this should be refactored to a helper function for clarity.
    {
        while (possible_solutions.size() > 1 && !unappliedRules.empty()) {
            auto currentRule = unappliedRules.front();
            unappliedRules.pop();

            // TODO: this should be refactored to a helper function for clarity.
            // Apply rules, pruning failures from our list of possible solutions.
            {
                if (DEBUG_LEVEL >= 1) {
                    cout << "Applying rule: " << currentRule->name << endl;
                }
                auto itr = begin(possible_solutions);
                while (itr != end(possible_solutions)) {
                    if (!currentRule->validationFunc(*itr)) {
                        if (DEBUG_LEVEL >= 3) {
                            cout << "word: " << *itr << " FAILS rule: " << currentRule->name << endl;
                        }
                        itr = possible_solutions.erase(itr);
                    }
                    else {
                        ++itr;
                    }
                }
            }
            appliedRules.insert(currentRule);

            // Log progress.
            cout << endl << endl;
            cout << format(STRING_RULE_APPLICATION_FORMAT, currentRule->name) << " " << format(STRING_SOLUTION_COUNT_FORMAT, possible_solutions.size()) << endl;
            if (DEBUG_LEVEL >= 3) {
                cout << toString(possible_solutions) << endl;
            }
        }
    }

    cout << "Possible Solutions: " << endl << toString(possible_solutions) << endl;

    /*
    // using user-defined rules, narrow down possible solutions.
    string currentRule;

    currentRule = "Limit to length 5";
    // first, narrow down the dictionary to word possibilities of matching length.
    {
        auto itr = begin(possible_solutions);
        while (itr != end(possible_solutions)) {  // this could be a helper function.
            auto str = *itr;
            if (str.length() != WORDLE_SIZE) { // this could be a fn pointer
                itr = possible_solutions.erase(itr);
            }
            else {
                ++itr;
            }
        };
        cout << "After applying rule \"" << currentRule << "\" There are " << possible_solutions.size() << " possible solutions." << endl;
        if (DEBUG_LEVEL >= 1) {
            cout << toString(possible_solutions) << endl;
        }
    }

    currentRule = "Last letter should be Y";
    // Next, find words that match a known letter formatting.
    {
        auto itr = begin(possible_solutions);
        while (itr != end(possible_solutions)) {  // this could be a helper function.
            auto str = *itr;
            if (str[str.length() - 1] != 'y') { // this could be a fn pointer
                itr = possible_solutions.erase(itr);
            }
            else {
                ++itr;
            }
        };
        cout << "After applying rule \"" << currentRule << "\" There are " << possible_solutions.size() << " possible solutions." << endl;
        if (DEBUG_LEVEL >= 1) {
            cout << toString(possible_solutions) << endl;
        }
    }

    currentRule = "Do not contain U,S,T,A,I,E,W,D";
    // Next, filter words that contain known excluded letters
    {
        vector<char> excluded({ 'u', 's', 't', 'a', 'i', 'e','w','d'});
        auto itr = begin(possible_solutions);
        while (itr != end(possible_solutions)) {  // this could be a helper function.
            auto str = *itr;
            if (containsAny(str, excluded)) { // this could be a fn pointer
                itr = possible_solutions.erase(itr);
            }
            else {
                ++itr;
            }
        };
        cout << "After applying rule \"" << currentRule << "\" There are " << possible_solutions.size() << " possible solutions." << endl;
        if (DEBUG_LEVEL >= 1) {
            cout << toString(possible_solutions) << endl;
        }
    }

    currentRule = "Contains R only at index 1";
    // Next, filter words that contain known excluded letters
    {
        auto itr = begin(possible_solutions);
        while (itr != end(possible_solutions)) {  // this could be a helper function.
            auto str = *itr;
            if (!(str[1] == 'r' && str[0] != 'r' && str[2] != 'r' && str[3] != 'r' && str[4] != 'r')) { // this could be a fn pointer
                itr = possible_solutions.erase(itr);
            }
            else {
                ++itr;
            }
        };
        cout << "After applying rule \"" << currentRule << "\" There are " << possible_solutions.size() << " possible solutions." << endl;
        if (DEBUG_LEVEL >= 1) {
            cout << toString(possible_solutions) << endl;
        }
    }

    currentRule = "Contains O, but not at index 1";
    // Next, filter words that contain known excluded letters
    {
        auto itr = begin(possible_solutions);
        while (itr != end(possible_solutions)) {  // this could be a helper function.
            auto str = *itr;
            if (containsAny(str, vector<char>({ 'o' })) && str[1] != 'o') { // this could be a fn pointer
                ++itr;
            }
            else {
                itr = possible_solutions.erase(itr);
            }
        };
        cout << "After applying rule \"" << currentRule << "\" There are " << possible_solutions.size() << " possible solutions." << endl;
        if (DEBUG_LEVEL >= 1) {
            cout << toString(possible_solutions) << endl;
        }
    }
    */

    return 0;
}