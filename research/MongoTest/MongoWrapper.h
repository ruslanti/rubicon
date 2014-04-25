/*
 * File:   MongoWrapper.h
 * Author: ion
 *
 * Created on April 25, 2014, 3:54 PM
 */

#ifndef MONGOWRAPPER_H
#define	MONGOWRAPPER_H

#include "mongo/client/dbclient.h"

#define DBNAME "rubicon"

class MongoWrapper {
    mongo::DBClientConnection mDbCC;

    MongoWrapper();

public:
    // collections
    static const std::string COL1;

    inline static MongoWrapper& instance() {
        static MongoWrapper staticInstance;
        return staticInstance;
    }

    inline mongo::DBClientConnection& getClientConnection() { return mDbCC; }

    virtual ~MongoWrapper();
};

#endif	/* MONGOWRAPPER_H */

