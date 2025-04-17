#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <map>
#include <thread>
#include "gtest/gtest.h"
#include "nlohmann/json.hpp"
using namespace std;

class ConverterJSON
{
public:
    nlohmann::json getConfig()
    {
        auto config = "{ \"config\": {"
                              "  \"name\": \"SkillBoxSearchEngine\","
                              "  \"version\": 0.1,"
                              "  \"max_responses\": 5,"
                              "  \"db_update\": 10"
                              "  }}"_json;
        return config;
    }

    int getResponsesLimit()
    {
        max_responses = getConfig()["config"]["max_responses"];
        return max_responses;
    }

private:
    int max_responses = 0;
};


struct Entry
{
    size_t doc_id, count;

    bool operator ==(const Entry& other) const
    {
        return (doc_id == other.doc_id && count == other.count);
    }
};

class InvertedIndex
{
public:

    void updateDocumentBase(vector<string> input_docs)
    {
        freq_dictionary.clear();

        auto Thread_Dict = [this](const string& doc, const size_t& doc_id)
        {
            string word;
            vector<string> texts_input;

            int simbol = 0;
            while(simbol < doc.length())
            {
                if(doc[simbol] != ' ') word += doc[simbol];
                if(!word.empty())
                {
                    if(doc[simbol] == ' ' || simbol == doc.length() - 1)
                    {
                        texts_input.push_back(word);
                        word.clear();
                    }
                }
                ++simbol;
            }

            while(!texts_input.empty())
            {
                size_t count = 0;
                vector<Entry> entry;

                word = texts_input[0];
                for(auto i = 0; i < texts_input.size(); ++i)
                {
                    if(word == texts_input[i])
                    {
                        texts_input.erase(texts_input.begin() + i);
                        ++count;
                        --i;
                    }
                }
                entry.push_back({doc_id, count});

                bool check = true;
                for(it = freq_dictionary.begin(); it != freq_dictionary.end(); ++it)
                {
                    if(it->first == word)
                    {
                        it->second.push_back({doc_id, count});
                        check = false;
                    }
                }
                if(check || freq_dictionary.empty())
                    freq_dictionary.insert(pair(word, entry));
            }
        };

        for(auto doc_id = 0; doc_id < input_docs.size(); ++doc_id)
        {
            thread ThreadDict(Thread_Dict, input_docs[doc_id], doc_id);
            ThreadDict.join();
        }
}

    vector<Entry> getWordCount(const string& word)
    {
        vector<Entry> entry;
        for(it = freq_dictionary.begin(); it != freq_dictionary.end(); ++it)
        {
            if(word == it->first)
            {
                for(auto & i : it->second)
                    entry.push_back({i.doc_id, i.count});
            }
        }
        return entry;
    }

private:
    map<string, vector<Entry>>::iterator it;
    map<string, vector<Entry>> freq_dictionary;
};


void TestInvertedIndexFunctionality(
        const vector<string>& docs,
        const vector<string>& requests,
        const vector<vector<Entry>>& expected
) {
    vector<vector<Entry>> result;
    InvertedIndex idx;
    idx.updateDocumentBase(docs);
    for(auto& request : requests) {
        vector<Entry> word_count = idx.getWordCount(request);
        result.push_back(word_count);
    }
    ASSERT_EQ(result, expected);
}
TEST(TestCaseInvertedIndex, TestBasic) {
    const vector<string> docs = {
            "london is the capital of great britain",
            "big ben is the nickname for the Great bell of the striking clock"
    };
    const vector<string> requests = {"london", "the"};
    const vector<vector<Entry>> expected = {
            {
                    {0, 1}
            }, {
                    {0, 1}, {1, 3}
            }
    };
    TestInvertedIndexFunctionality(docs, requests, expected);
}
TEST(TestCaseInvertedIndex, TestBasic2) {
    const vector<string> docs = {
            "milk milk milk milk water water water",
            "milk water water",
            "milk milk milk milk milk water water water water water",
            "americano cappuccino"
    };
    const vector<string> requests = {"milk", "water", "cappuchino"};
    const vector<vector<Entry>> expected = {
            {
                    {0, 4}, {1, 1}, {2, 5}
            }, {
                    {0, 2}, {1, 2}, {2, 5}
            }, {
                    {3, 1}
            }
    };
    TestInvertedIndexFunctionality(docs, requests, expected);
}
TEST(TestCaseInvertedIndex, TestInvertedIndexMissingWord) {
    const vector<string> docs = {
            "a b c d e f g h i j k l",
            "statement"
    };
    const vector<string> requests = {"m", "statement"};
    const vector<vector<Entry>> expected = {
            {
            }, {
                    {1, 1}
            }
    };
    TestInvertedIndexFunctionality(docs, requests, expected);
}




struct RelativeIndex
{
    size_t doc_id;
    float rank;

    bool operator ==(const RelativeIndex& other) const
    {
        return (doc_id == other.doc_id && rank == other.rank);
    }
};

class SearchServer
{
public:
    SearchServer(InvertedIndex& idx) : _index(idx){};

    vector<vector<RelativeIndex>> search(vector<string> queries_input)
    {
        relative_index.clear();

        for(auto& request : queries_input)
        {
            RelativeIndex rlv;
            ConverterJSON cvr;
            vector<Entry> entry;
            vector<string> input_words;
            vector<vector<size_t>> infrequent, document;
            vector<RelativeIndex> input_relative;
            string word;
            float rank = 0;
            int simbol = 0, max = 0;

            while(simbol < request.length())
            {
                if(request[simbol] != ' ') word += request[simbol];
                if(!word.empty())
                {
                    if(request[simbol] == ' ' || simbol == request.length() - 1)
                    {
                        input_words.push_back(word);
                        word.clear();
                    }
                }
                ++simbol;
            }

            for(auto& i : input_words)
                entry = _index.getWordCount(i);

            for(size_t i = 0; i < input_words.size(); ++i)
            {
                size_t count = 0;
                entry = _index.getWordCount(input_words[i]);

                for(auto& j : entry)
                    count += j.count;
                infrequent.push_back({count, i});
            }
            sort(infrequent.begin(), infrequent.end());

            for(auto& i : infrequent)
            {
                entry = _index.getWordCount(input_words[i[1]]);
                for (auto& j : entry)
                {
                    bool check = true;
                    if(document.empty())
                    {
                        document.push_back({j.count, j.doc_id});
                        check = false;
                    }
                    else
                    {
                        for(auto& k : document)
                        {
                            if(k[1] == j.doc_id)
                            {
                                k[0] += j.count;
                                check = false;
                                break;
                            }
                        }
                    }
                    if(check) document.push_back({j.count, j.doc_id});
                }
            }

            if(!document.empty())
            {
                bool check = true;
                while(check)
                {
                    check = false;
                    for(auto i = 0; i < document.size() - 1; ++i)
                    {
                        if(document[i][0] < document[i + 1][0])
                        {
                            ranges::swap(document[i], document[i + 1]);
                            check = true;
                        }
                    }
                }
                max = document[0][0];
            }

            for(auto & i : document)
            {
                if(input_relative.size() == cvr.getResponsesLimit()) break;
                rank = (float)i[0] / (float)max;
                rlv.doc_id = i[1];
                rlv.rank = rank;
                input_relative.push_back({i[1], rank});
            }
            relative_index.push_back(input_relative);
        }
        return relative_index;
    }

private:
    InvertedIndex _index;
    vector<vector<RelativeIndex>> relative_index;
    vector<vector<pair<int, float>>> answers;
};


TEST(TestCaseSearchServer, TestSimple) {
    const vector<string> docs = {
            "milk milk milk milk water water water",
            "milk water water",
            "milk milk milk milk milk water water water water water",
            "americano cappuccino"
    };
    const vector<string> request = {"milk water", "sugar"};
    const std::vector<vector<RelativeIndex>> expected = {
            {
                    {2, 1},
                    {0, 0.7},
                    {1, 0.3}
            },
            {
            }
    };
    InvertedIndex idx;
    idx.updateDocumentBase(docs);
    SearchServer srv(idx);
    std::vector<vector<RelativeIndex>> result = srv.search(request);
    ASSERT_EQ(result, expected);
}
TEST(TestCaseSearchServer, TestTop5) {
    const vector<string> docs = {
            "london is the capital of great britain",
            "paris is the capital of france",
            "berlin is the capital of germany",
            "rome is the capital of italy",
            "madrid is the capital of spain",
            "lisboa is the capital of portugal",
            "bern is the capital of switzerland",
            "moscow is the capital of russia",
            "kiev is the capital of ukraine",
            "minsk is the capital of belarus",
            "astana is the capital of kazakhstan",
            "beijing is the capital of china",
            "tokyo is the capital of japan",
            "bangkok is the capital of thailand",
            "welcome to moscow the capital of russia the third rome",
            "amsterdam is the capital of netherlands",
            "helsinki is the capital of finland",
            "oslo is the capital of norway",
            "stockholm is the capital of sweden",
            "riga is the capital of latvia",
            "tallinn is the capital of estonia",
            "warsaw is the capital of poland",
    };
    const vector<string> request = {"moscow is the capital of russia"};
    const std::vector<vector<RelativeIndex>> expected = {
            {
                    {7, 1},
                    {14, 1},
                    {0, 0.666666687},
                    {1, 0.666666687},
                    {2, 0.666666687}
            }
    };
    InvertedIndex idx;
    idx.updateDocumentBase(docs);
    SearchServer srv(idx);
    std::vector<vector<RelativeIndex>> result = srv.search(request);
    ASSERT_EQ(result, expected);
}