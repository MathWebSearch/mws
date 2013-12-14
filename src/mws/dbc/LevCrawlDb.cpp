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
  * @file CrawlDb.cpp
  * @brief Crawl Data LevelDb Database implementation
  * @author Radu Hambasan
  * @date 11 Dec 2013
  */

#include <stdexcept>
#include <string>
using std::string;

#include "common/utils/ToString.hpp"
#include "common/types/Parcelable.hpp"
using common::types::ParcelAllocator;
using common::types::ParcelEncoder;
using common::types::ParcelDecoder;
#include "mws/types/NodeInfo.hpp"
using mws::types::CrawlData;

#include "LevCrawlDb.hpp"


namespace mws { namespace dbc {

LevCrawlDb::LevCrawlDb() : mDatabase(NULL), mNextCrawlId(0) {
}

LevCrawlDb::~LevCrawlDb() {
    delete mDatabase;
}

int LevCrawlDb::open(const char* path) {
    leveldb::Options options;
    options.create_if_missing = false;
    leveldb::Status status =
        leveldb::DB::Open(options, path, &mDatabase);

    return status.ok()? 0 : -1;
}

int LevCrawlDb::create_new(const char* path) {
    leveldb::Options options;
    options.error_if_exists = true;
    options.create_if_missing = true;
    leveldb::Status status =
        leveldb::DB::Open(options, path, &mDatabase);

    return status.ok()? 0 : -1;
}

mws::CrawlId LevCrawlDb::putData(const mws::types::CrawlData& crawlData)
throw (std::exception) {
    CrawlId crawlId = mNextCrawlId++;
    std::string crawlId_str = ToString(crawlId);

    ParcelAllocator allocator;
    allocator.reserve(crawlData);

    ParcelEncoder encoder(allocator);
    encoder.encode(crawlData);

    std::string serial(encoder.getData(), encoder.getSize());
    auto ret =
        mDatabase->Put(leveldb::WriteOptions(), crawlId_str, serial);
    if (!ret.ok()) {
        throw std::runtime_error("Error encountered while inserting: " +
                                 crawlId_str);
    }

    return crawlId;
}

const types::CrawlData LevCrawlDb::getData(const mws::CrawlId& crawlId)
throw (std::exception) {
    std::string crawlId_str = ToString(crawlId);
    std::string retrieved_str;
    mws::types::CrawlData retrieved;

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
