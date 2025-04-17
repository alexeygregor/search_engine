
#ifndef CONVERTERJSON_H
#define CONVERTERJSON_H

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <map>
#include <mutex>
#include <thread>
#include "nlohmann/json.hpp"
using json = nlohmann::json;
using namespace std;

class ConverterJSON
{
public:

    json getConfig();

    void starting();

    void loadConfig();

    void checkConfig();

    void loadRequest();

    vector<string> getRequests();

    vector<string> getTextDocument();

    int getResponsesLimit();

    bool getDBUpdate();

    void putAnswers(vector<vector<pair<int, float>>> answers);

    void getAnswers();

    basic_string<char> getCount(int value);

private:
    float rank = 0;
    size_t doc_id = 0;
    vector<string> documents;
    vector<string> requests;
};


#endif //CONVERTERJSON_H