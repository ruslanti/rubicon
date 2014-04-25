#include <cstdlib>
#include "MongoWrapper.h"

using namespace std;
using namespace mongo;

#define MW  MongoWrapper
#define MWI MongoWrapper::instance()
#define MCC MongoWrapper::instance().getClientConnection()

void run() {
    MCC.connect("localhost");
    unsigned long long cnt = MCC.count(MW::COL1);
    cout << "Found " << cnt << " documents in " << MW::COL1 << endl;

    if (cnt > 0) {
        cout << "Trimming Collection..." << endl;
        MCC.remove(MW::COL1, Query());
    }

    cout << "Inserting... ";
    for (int i = 0; i < 100; i++)
        MCC.insert(MW::COL1, BSON("ip" << "192.168.0.1" << "port" << (1024 + i * 2)));
    cout << "inserted " << MCC.count(MW::COL1) << " documents" << endl;

    cout << "Removing half of collection..." << endl;
    for (int i = 0; i < 100; i = i + 2)
        MCC.remove(MW::COL1, QUERY("port" << (1024 + i * 2)));

    cout << endl << "Documents in collection" << endl;
    auto_ptr<DBClientCursor> cursor =  MCC.query(MW::COL1, Query().sort("port"));
    while (cursor->more())
        cout << cursor->next().toString() << endl;
}

int main() {
    try {
        run();
        std::cout << "processed ok" << std::endl;
    } catch( const mongo::DBException &e ) {
        std::cout << "caught " << e.what() << std::endl;
    }

    return EXIT_SUCCESS;
}
