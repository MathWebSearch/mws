/*

Copyright (C) 2010-2013 KWARC Group <kwarc.info>

This file is part of MathWebSearch.

MathWebSearch is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

MathWebSearch is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with MathWebSearch.  If not, see <http://www.gnu.org/licenses/>.

*/

/**
  * @file LevCrawlDb.cpp
  * @brief LevelDb Crawl Database implementation
  * @author Radu Hambasan
  * @date 11 Dec 2013
  */


#include <sys/stat.h>
#include <sys/types.h>

#include <stdexcept>
using std::runtime_error;
#include <string>
using std::string;
#include <leveldb/db.h>
using leveldb::DB;
using leveldb::Options;
using leveldb::Status;
using leveldb::DestroyDB;

#include "common/types/Parcelable.hpp"
using common::types::ParcelAllocator;
using common::types::ParcelEncoder;
using common::types::ParcelDecoder;
#include "mws/dbc/CrawlDb.hpp"
#include "mws/dbc/LevCrawlDb.hpp"


namespace mws { namespace dbc {

LevCrawlDb::LevCrawlDb() : mDatabase(NULL), mNextCrawlId(CRAWLID_NULL) {
}

LevCrawlDb::~LevCrawlDb() {
    delete mDatabase;
}

void LevCrawlDb::open(const char* path) throw (runtime_error) {
    leveldb::Options options;
    options.create_if_missing = false;
    leveldb::Status status =
        leveldb::DB::Open(options, path, &mDatabase);

    if (!status.ok()) {
        throw std::runtime_error(string(path) + " not found.");
    }
}

void LevCrawlDb::create_new(const char* path, bool deleteIfExists)
throw (runtime_error) {
    if (deleteIfExists) {
        (void) DestroyDB(path, Options());
    }

    Options options;
    options.error_if_exists = true;
    options.create_if_missing = true;
    Status status = DB::Open(options, path, &mDatabase);
    if (!status.ok()) {
        throw std::runtime_error(status.ToString());
    }
}

CrawlId LevCrawlDb::putData(const CrawlData& crawlData)
throw (std::exception) {
    CrawlId crawlId = ++mNextCrawlId;
    string crawlId_str = std::to_string(crawlId);

    ParcelAllocator allocator;
    allocator.reserve(crawlData);

    ParcelEncoder encoder(allocator);
    encoder.encode(crawlData);

    string serial(encoder.getData(), encoder.getSize());
    auto ret =
        mDatabase->Put(leveldb::WriteOptions(), crawlId_str, serial);
    if (!ret.ok()) {
        throw std::runtime_error("Error encountered while inserting: " +
                                 crawlId_str);
    }

    return crawlId;
}

const CrawlData LevCrawlDb::getData(const CrawlId& crawlId)
throw (std::exception) {
    string crawlId_str = std::to_string(crawlId);
    string retrieved_str;
    CrawlData retrieved;

    auto status =
        mDatabase->Get(leveldb::ReadOptions(), crawlId_str, &retrieved_str);
    if (status.ok()) {
        ParcelDecoder decoder(retrieved_str.data(), retrieved_str.size());
        decoder.decode(&retrieved);
        return retrieved;
    } else {
        throw std::runtime_error("No data corresponding to crawlId = " +
                            crawlId_str);
    }

    return retrieved;
}


}  // namespace dbc
}  // namespace mws
