
#include "SearchServer.h"

SearchServer::SearchServer(InvertedIndex& idx, ConverterJSON& cvr) : _index(idx), _convert(cvr){};

vector<vector<RelativeIndex>> SearchServer::search(vector<string> queries_input)
{
    relative_index.clear();

    for(auto& request : queries_input)
    {
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

        RelativeIndex rlv;
        for(auto & i : document)
        {
            if(input_relative.size() == _convert.getResponsesLimit()) break;
            rank = (float)i[0] / (float)max;
            rlv.doc_id = i[1];
            rlv.rank = rank;
            input_relative.push_back({i[1], rank});
        }
        relative_index.push_back(input_relative);
    }
    return relative_index;
}

vector<vector<pair<int, float>>> SearchServer::getAnswers()
{
    for(auto& i : search(_convert.getRequests()))
    {
        vector<pair<int, float>>answer;
        for(auto& j : i)
            answer.push_back({j.doc_id, j.rank});
        answers.push_back(answer);
    }
    return answers;
}