#pragma once

#include <string>
#include <functional>


#define WORDLE_RULE_STRING_HAS_LENGTH_FORMAT                    "Has length {}"
#define WORDLE_RULE_STRING_DOES_NOT_CONTAIN_LETTER_RULE_FORMAT  "Does Not contain letter {}"
#define WORDLE_RULE_STRING_DOES_CONTAIN_LETTER_RULE_FORMAT      "Does contain letter {}"
#define WORDLE_RULE_STRING_DOES_NOT_CONTAIN_LETTER_AT_FORMAT    "Does Not contain letter {} at {}"
#define WORDLE_RULE_STRING_DOES_CONTAIN_LETTER_AT_FORMAT        "Does contain letter {} at {}"

class WordleRule {
    public:
        std::string name; // what is the pretty name of this rule?

        char letter;
        int index;

        static bool DoesNotContainLetterRuleFunc (const std::string& word, const char& letter) {
            return word.find(letter) == word.npos;
        }

        static bool DoesNotContainLetterAtRuleFunc(const std::string& word, const char& letter, const int& index ) {
            return word[index] != letter;
        }

        static bool DoesContainLetterRuleFunc(const std::string& word, const char& letter) {
            return word.find(letter) != word.npos;
        }

        static bool DoesContainLetterAtRuleFunc(const std::string& word, const char& letter, const int& index) {
            return word[index] == letter;
        }

        static bool HasLengthRuleFunc(const std::string& word, const char& length) {
            return word.size() == length;
        }

        template<typename T, typename U>
        static void CreateNewRule(char letter, char index, char result, T& existingRules, U& appliedRules) {
            switch (result) {
            case 'b':
                if (!HasIncludingRule<T>(existingRules, letter) && !HasIncludingRule<U>(appliedRules, letter)) {
                existingRules.push_back(std::shared_ptr<WordleRule>(new WordleRule(letter
                    , index
                    , RuleType::RuleTypeDoesNotContainLetter)));
                }
                break;
            case 'y':
                existingRules.push_back(std::shared_ptr<WordleRule>(new WordleRule(letter
                    , index
                    , RuleType::RuleTypeDoesContainLetter)));
                existingRules.push_back(std::shared_ptr<WordleRule>(new WordleRule(letter
                    , index
                    , RuleType::RuleTypeDoesNotContainLetterAt)));
                break;
            case 'g':
                existingRules.push_back(std::shared_ptr<WordleRule>(new WordleRule(letter
                    , index
                    , RuleType::RuleTypeDoesContainLetterAt)));
                break;
            }
        }

        template<typename T>
        static void CreateLengthRule(int length, T& existingRules) {
            existingRules.push_back(std::shared_ptr<WordleRule>(new WordleRule(0, length, RuleType::RuleHasLength)));
        }

        template<typename T = std::queue<std::shared_ptr<WordleRule>>>
        static void CreateLengthRule(int length, std::queue<std::shared_ptr<WordleRule>>& existingRules) {
            existingRules.push(std::shared_ptr<WordleRule>(new WordleRule(0, length, RuleType::RuleHasLength)));
        }

        template<typename T = std::list<WordleRule>>
        static bool HasIncludingRule(T ruleCollection, char letter) {
            for (auto itr = std::begin(ruleCollection); itr != std::end(ruleCollection); ++itr) {
                std::shared_ptr<WordleRule> rule = *itr;
                if (rule->letter == letter && (rule->ruleType == RuleType::RuleTypeDoesContainLetter || rule->ruleType == RuleType::RuleTypeDoesContainLetterAt)) {
                    return true;
                }
            }
        }

        enum class RuleType {
            RuleTypeDoesNotContainLetter,
            RuleTypeDoesContainLetter,
            RuleTypeDoesNotContainLetterAt,
            RuleTypeDoesContainLetterAt,
            RuleHasLength
        };
        RuleType ruleType;

        bool Validate(const std::string& word) {
            switch (ruleType){
            case RuleType::RuleTypeDoesNotContainLetter:
                return WordleRule::DoesNotContainLetterRuleFunc(word, letter);
                break;
            case RuleType::RuleTypeDoesContainLetter:
                return WordleRule::DoesContainLetterRuleFunc(word, letter);
                break;
            case RuleType::RuleTypeDoesNotContainLetterAt:
                return WordleRule::DoesNotContainLetterAtRuleFunc(word, letter, index);
                break;
            case RuleType::RuleTypeDoesContainLetterAt:
                return WordleRule::DoesContainLetterAtRuleFunc(word, letter, index);
                break;
            case RuleType::RuleHasLength:
                return WordleRule::HasLengthRuleFunc(word, index);
                break;
            }
        }; //function pointer to logic that determines whether or not a given std::string is valid when checked by this rule.

        // for now, rules must be defined at creation.
        WordleRule(char letter, int index, RuleType ruleType)
            :
            letter(letter),
            index(index),
            ruleType(ruleType)
        {
            std::string nameString;
            switch (ruleType) {
            case RuleType::RuleTypeDoesNotContainLetter:
                nameString = std::format(WORDLE_RULE_STRING_DOES_NOT_CONTAIN_LETTER_RULE_FORMAT, letter);
                break;
            case RuleType::RuleTypeDoesContainLetter:
                nameString = std::format(WORDLE_RULE_STRING_DOES_CONTAIN_LETTER_RULE_FORMAT, letter);
                break;
            case RuleType::RuleTypeDoesNotContainLetterAt:
                nameString = std::format(WORDLE_RULE_STRING_DOES_NOT_CONTAIN_LETTER_AT_FORMAT, letter, index);
                break;
            case RuleType::RuleTypeDoesContainLetterAt:
                nameString = std::format(WORDLE_RULE_STRING_DOES_CONTAIN_LETTER_AT_FORMAT, letter, index);
                break;
            case RuleType::RuleHasLength:
                nameString = std::format(WORDLE_RULE_STRING_HAS_LENGTH_FORMAT, index);
                break;
            }
            name = nameString;
        }

        // for now, assume that rules with matching names are equivalent.
        bool operator ==(WordleRule& other) {
            return this->name.compare(other.name) == 0;
        }

        operator std::string() {
            return this->name;
        }
    };
