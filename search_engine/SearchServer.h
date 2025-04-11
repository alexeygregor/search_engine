

#ifndef SEARCHSERVER_H
#define SEARCHSERVER_H

#include "InvertedIndex.h"

struct RelativeIndex
{
    size_t doc_id;
    float rank;
};

class SearchServer
{
public:

    SearchServer(InvertedIndex &idx);

    vector<vector<RelativeIndex>> search(vector<string> queries_input);

    vector<vector<pair<int, float>>> getAnswers(ConverterJSON& cvr);

private:
    InvertedIndex _index;
    vector<vector<RelativeIndex>> relative_index;
    vector<vector<pair<int, float>>> answers;
};


#endif //SEARCHSERVER_H
