#include "SearchServer.h"


int main()
{
    ConverterJSON cvr;
    InvertedIndex idx;
    //cvr.loadConfig();
    //idx.readAll();

    try
    {
        cvr.checkConfig();
    }
    catch(const invalid_argument& e)
    {
        cerr << e.what() << endl;
        return 0;
    }

    cvr.starting();
    idx.updateDocumentBase(cvr.getTextDocument());

    while(true)
    {
        string command;
        cout << "Input command:" << endl;
        getline(cin, command);
        if(command == "search")
        {
            cvr.loadRequest();
            if(cvr.getDBUpdate()) idx.updateDocumentBase(cvr.getTextDocument());
            SearchServer srv(idx);
            cvr.putAnswers(srv.getAnswers(cvr));
            cvr.getAnswers();
        }
        else if(command == "read")
        {
            idx.getDocuments();
        }
        else if(command == "quit")
            break;
        else
            cerr << "incorrect command" << endl;
    }
}