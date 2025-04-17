
#include "ConverterJSON.h"

json ConverterJSON::getConfig()
{
    auto config = "{ \"config\": {"
                            "  \"name\": \"SkillBoxSearchEngine\","
                            "  \"version\": 0.1,"
                            "  \"max_responses\": 5,"
                            "  \"db_update\": 10"
                            "  },"
                            "  \"files\": ["
                            "  \"resourses/file001.txt\","
                            "  \"resourses/file002.txt\","
                            "  \"resourses/file003.txt\","
                            "  \"resourses/file004.txt\","
                            "  \"resourses/file005.txt\","
                            "  \"resourses/file006.txt\","
                            "  \"resourses/file007.txt\","
                            "  \"resourses/file008.txt\","
                            "  \"resourses/file009.txt\","
                            "  \"resourses/file010.txt\""
                            "  ]}"_json;
    return config;
}

void ConverterJSON::starting()
{
    ifstream file("config.json");
    json dict;
    file >> dict;
    string name = dict["config"]["name"];
    cout << "Starting " << name << "\n\n";
}


void ConverterJSON::loadConfig()
    {
        ofstream file("config.json");
        file << getConfig();
        file.close();
    }

void ConverterJSON::checkConfig()
{
    if(ifstream check("config.json", ios::in); !check)
        throw invalid_argument("config file is empty");
    else
    {
        json config;
        check >> config;
        if(config["config"].empty())
            throw invalid_argument("config file is missing");
        if(config["config"]["version"] != getConfig()["config"]["version"])
            throw invalid_argument("config.json has incorrect file version");
    }
}

void ConverterJSON::loadRequest()
{
    string  request;
    cout << "Input request:" << endl;
    getline(cin, request);

    json dict;
    basic_string <char> valid;
    ifstream check("requests.json");
    check >> valid;
    check.close();
    if(!valid.empty())
    {
        ifstream out("requests.json");
        out >> dict;
        out.close();
    }

    if(dict["requests"].size() < 1000)
    {
        dict["requests"].push_back(request);
        ofstream in("requests.json");
        in << dict;
        in.close();
    }

    if(requests.empty())
    {
        for(auto& i : dict["requests"])
            requests.push_back(i);
    }
    else requests.push_back(request);
}

vector<string> ConverterJSON::getRequests()
{
    return requests;
}

vector<string> ConverterJSON::getTextDocument()
{
    ifstream file("config.json");
    json dict;
    file >> dict;
    documents.clear();
    for(auto& i : dict["files"])
        documents.push_back(i);
    file.close();
    return documents;
}

int ConverterJSON::getResponsesLimit()
{
    ifstream file("config.json");
    json dict;
    file >> dict;
    int max_responses = 0;
    max_responses = dict["config"]["max_responses"];
    file.close();
    return max_responses;
}

bool ConverterJSON::getDBUpdate()
{
    json dict;
    ifstream config("config.json");
    config >> dict;
    int db_update = 0;
    db_update = dict["config"]["db_update"];
    dict.clear();
    config.close();
    basic_string <char> valid;
    ifstream check("requests.json");
    check >> valid;
    check.close();
    if(!valid.empty())
    {
        if(requests.size() % db_update != 0)
            return false;
    }
    cout << "db_update" << endl;
    return true;
}

void ConverterJSON::putAnswers(vector<vector<pair<int, float>>> answers)
{
    ofstream in("answers.json");
    json dict;

    for(auto i = 0; i < answers.size(); ++i)
    {
        basic_string<char> count = getCount(i + 1);

        if(answers[i].empty())
            dict["answers"]["request" + count]["result"] = "false";

        else
        {
            dict["answers"]["request" + count]["result"] = "true";
            for(auto j = 0; j < answers[i].size(); ++j)
            {
                if(j == getResponsesLimit()) break;
                if(answers[i].size() == 1)
                {
                    dict["answers"]["request" + count]["docid"].push_back(answers[i][j].first);
                    dict["answers"]["request" + count]["rank"].push_back(answers[i][j].second);
                }
                else
                {
                    dict["answers"]["request" + count]["relevance"]["docid"].push_back(answers[i][j].first);
                    dict["answers"]["request" + count]["relevance"]["rank"].push_back(answers[i][j].second);
                }
            }
        }
    }
    in << dict;
    in.close();
}

void ConverterJSON::getAnswers()
{
    ifstream out("answers.json");
    json dict;
    out >> dict;

    if(requests.size() <= 1000)
    {
        basic_string<char> request_count = getCount(dict["answers"].size());

        cout << "Result:" << endl;
        if(dict["answers"]["request" + request_count]["result"] == "false")
            cout << "Not found" << endl;
        else
        {
            int answers_count = 0;
            if(dict["answers"]["request" + request_count]["docid"].size() == 1)
                answers_count = dict["answers"]["request" + request_count]["docid"].size();
            else
                answers_count = dict["answers"]["request" + request_count]["relevance"]["docid"].size();

            for(auto j = 0; j < answers_count; ++j)
            {
                if(j == getResponsesLimit()) break;
                if(dict["answers"]["request" + request_count]["docid"].size() == 1)
                {
                    rank = static_cast<float>(dict["answers"]["request" + request_count]["rank"][j]);
                    doc_id = static_cast<size_t>(dict["answers"]["request" + request_count]["docid"][j]);
                }
                else
                {
                    rank = static_cast<float>(dict["answers"]["request" + request_count]["relevance"]["rank"][j]);
                    doc_id = static_cast<size_t>(dict["answers"]["request" + request_count]["relevance"]["docid"][j]);
                }

                basic_string<char> file_count = getCount(doc_id + 1);
                cout << "file" << file_count  << "   " << rank << endl;
            }
        }
    }
    else cerr << "1000+ requests" << endl;
    out.close();
}

basic_string<char> ConverterJSON::getCount(int value)
{
    basic_string<char> count;
    if(value <= 9)
        count = "00" + to_string(value);
    else if(value <= 99)
        count = "0" + to_string(value);
    else
        count = to_string(value);
    return count;
}