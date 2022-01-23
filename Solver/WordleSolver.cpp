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

#include <WordleRule.h>

// Constants
const std::string    DEFAULT_DICTIONARY_NAME     = "words_alpha.txt";
const int       DEFAULT_WORDLE_LENGTH       = 5;

void printUsage(){
    std::cout << "Usage: WordleSolver.exe <pathToDictionaryFile>" << std::endl << std::endl;
}

// Utility Functions
// TODO: refactor to separate headers, these should extend class behavior via friends or something similar.
std::string toString(std::set<std::string> strings) {
    std::stringstream result;
    for_each(begin(strings), end(strings), [&](auto str) {
            result << str << " ";
        });
    result << " ";
    return result.str();
}

/*
bool containsAny(std::string str, vector<char> chars) {
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

const std::string STRING_SOLUTION_COUNT_FORMAT = "There are {} possible solutions.";
const std::string STRING_RULE_APPLICATION_FORMAT = "After applying rule {}";

// Solver Logic
int main(int argc, char** argv){
    const std::string DEFAULT_DICTIONARY_PATH = (std::filesystem::current_path() / std::filesystem::path(DEFAULT_DICTIONARY_NAME)).string();
    int WORDLE_SIZE = DEFAULT_WORDLE_LENGTH;
    std::string dictionary_path = DEFAULT_DICTIONARY_PATH;
    std::string wordle_guess_path;
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

    std::cout << "WordleSolver" << std::endl << std::endl;
    std::cout << "Using input dictionary file: " << dictionary_path << std::endl;
    std::cout << "Using guess file: " << wordle_guess_path << std::endl;

    // populate starting std::set of words.
    // contains all our intermediate solutions.
    std::set<std::string> possible_solutions;

    // populate possible solutions.
    //TODO: refactor to its own method.
    {
        std::string line;
        std::ifstream dictionary_file(dictionary_path);

        if (dictionary_file.is_open()) {
            while (std::getline(dictionary_file, line)) {
                possible_solutions.insert(line);
            }
            dictionary_file.close();
        }
        else {
            std::cerr << "Unable to open file: " << dictionary_path << std::endl;
            return -1;
        }
        if (DEBUG_LEVEL >= 1) {
            std::cout << "There are " << possible_solutions.size() << " possible solutions." << std::endl;
            if (DEBUG_LEVEL >= 4) {
                std::cout << toString(possible_solutions) << std::endl;
            }
        }
    }

    std::queue<std::shared_ptr<WordleRule>> unappliedRules;
    std::set<std::shared_ptr<WordleRule>> appliedRules;

    WordleRule::CreateLengthRule(WORDLE_SIZE, unappliedRules);
    
    // populate our logical rules for guessing the word using the results of previous guesses.
    // TODO: this should ve refactored to a helper function for clarity
    {
        std::string line;
        std::string guess_word;
        std::string guess_result;
        std::ifstream wordle_guess_file(wordle_guess_path);

        //<typename T>
        //auto hasApplicableRule = [](const T  )

        auto generateRules = [&appliedRules](const std::string& guess_word, const std::string& guess_result) {
            // a guess result can only be three characters:
            // a b character = black, unused everywhere in the word.
            // a y character = yellow, used somewhere in the word, but not at this index.
            // a g character = green, used in the word at this index.
           
            std::list<std::shared_ptr<WordleRule>> rules;

            for (int i = 0; i < guess_result.size(); ++i) {
                WordleRule::CreateNewRule<std::list<std::shared_ptr<WordleRule>>, std::set<std::shared_ptr<WordleRule>>>(guess_word[i], i, guess_result[i], rules, appliedRules);
            }
            return rules;
        };

        if (wordle_guess_file.is_open()) {
            // the first line of a wordle guess file is the number of letters the wordle contains.
            std::getline(wordle_guess_file, line);
            WORDLE_SIZE = stoi(line);

            if (DEBUG_LEVEL > 1)
                std::cout << "Wordle is " << WORDLE_SIZE << " letters long." << std::endl;

            while (std::getline(wordle_guess_file, line)) {
                if (DEBUG_LEVEL >= 3) {
                    std::cout << "Reading input line: " << line << std::endl;
                }

                auto line_stream = std::stringstream(line);
                line_stream >> guess_word >> guess_result;
                if (DEBUG_LEVEL >= 3) {
                    std::cout << "Guess word: " << guess_word << " Guess result: " << guess_result << std::endl;
                }
                
                auto generatedRules = generateRules(guess_word, guess_result);
                for (auto itr = begin(generatedRules); itr != end(generatedRules); ++itr) {
                    unappliedRules.push(*itr);
                    if (DEBUG_LEVEL >= 1) {
                        std::cout << "Adding rule: " << itr->get()->name << std::endl;
                    }
                }
            }
            wordle_guess_file.close();
        }
        else {
            std::cerr << "Unable to open file: " << dictionary_path << std::endl;
            return -1;
        }
    }

    // TODO: this should be refactored to a helper function for clarity.
    {
        while (possible_solutions.size() > 1 && !unappliedRules.empty()) {
            auto currentRule = unappliedRules.front();
            unappliedRules.pop();

            // TODO: this should be refactored to a helper function for clarity.
            // Apply rules, pruning failures from our std::list of possible solutions.
            {
                if (DEBUG_LEVEL >= 1) {
                    std::cout << "Applying rule: " << currentRule->name << std::endl;
                }
                auto itr = begin(possible_solutions);
                while (itr != end(possible_solutions)) {
                    if (!currentRule->Validate(*itr)) {
                        if (DEBUG_LEVEL >= 3) {
                            std::cout << "word: " << *itr << " FAILS rule: " << currentRule->name << std::endl;
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
            std::cout << std::endl << std::endl;
            std::cout << std::format(STRING_RULE_APPLICATION_FORMAT, currentRule->name) << " " << std::format(STRING_SOLUTION_COUNT_FORMAT, possible_solutions.size()) << std::endl;
            if (DEBUG_LEVEL >= 3) {
                std::cout << toString(possible_solutions) << std::endl;
            }
        }
    }

    std::cout << "Possible Solutions: " << std::endl << toString(possible_solutions) << std::endl;

    /*
    // using user-defined rules, narrow down possible solutions.
    std::string currentRule;

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
        std::cout << "After applying rule \"" << currentRule << "\" There are " << possible_solutions.size() << " possible solutions." << std::endl;
        if (DEBUG_LEVEL >= 1) {
            std::cout << toString(possible_solutions) << std::endl;
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
        std::cout << "After applying rule \"" << currentRule << "\" There are " << possible_solutions.size() << " possible solutions." << std::endl;
        if (DEBUG_LEVEL >= 1) {
            std::cout << toString(possible_solutions) << std::endl;
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
        std::cout << "After applying rule \"" << currentRule << "\" There are " << possible_solutions.size() << " possible solutions." << std::endl;
        if (DEBUG_LEVEL >= 1) {
            std::cout << toString(possible_solutions) << std::endl;
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
        std::cout << "After applying rule \"" << currentRule << "\" There are " << possible_solutions.size() << " possible solutions." << std::endl;
        if (DEBUG_LEVEL >= 1) {
            std::cout << toString(possible_solutions) << std::endl;
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
        std::cout << "After applying rule \"" << currentRule << "\" There are " << possible_solutions.size() << " possible solutions." << std::endl;
        if (DEBUG_LEVEL >= 1) {
            std::cout << toString(possible_solutions) << std::endl;
        }
    }
    */

    return 0;
}