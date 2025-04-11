
#ifndef INVERTEDINDEX_H
#define INVERTEDINDEX_H

#include "ConverterJSON.h"

struct Entry
{
    size_t doc_id, count;
};

class InvertedIndex
{
public:

    void readAll();

    void updateDocumentBase(vector<string> input_docs);

    vector<Entry> getWordCount(const string& word);

    void getDocuments();

private:
    bool read_all = false;
    vector<string> input_documents;
    map<string, vector<Entry>>::iterator it;
    map<string, vector<Entry>> freq_dictionary;
};


#endif //INVERTEDINDEX_H
