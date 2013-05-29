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
  * @brief File containing the implementation of the loadMwsHarvestFromDirectory
  * function
  *
  * @file loadMwsHarvestFromDirectory.cpp
  * @date 30 Apr 2012
  *
  * License: GPL v3
  *
  */

// System includes

#include <utility>
#include <algorithm>
#include <stdio.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

// Local includes

#include "mws/index/MwsIndexNode.hpp"
#include "common/utils/Path.hpp"
#include "loadMwsHarvestFromFd.hpp"


// Configuration files

#include "MwsDaemonConf.hpp"

// Macro handling

#ifndef MWS_HARVEST_EXT
#define MWS_HARVEST_EXT DEFAULT_MWS_HARVEST_EXT
#endif

// Namespaces

using namespace std;

namespace mws {

int
loadMwsHarvestFromDirectory(mws::AbsPath const& dirPath,
                            IndexContext* ctxt)
{
    DIR*           directory;
    struct dirent* currEntry;
    struct dirent  tmpEntry;
    int            ret;
    size_t         extenSize;
    size_t         entrySize;
    int            fd;
    int            totalLoaded;
    pair<int,int>  loadReturn;
    vector<string> files;
    AbsPath        fullPath;
    vector<string> :: iterator it;

    totalLoaded = 0;

    extenSize = strlen(DEFAULT_MWS_HARVEST_EXT);

    directory = opendir(dirPath.get());
    if (!directory)
    {
        perror("Bad data directory provided");
        return -1;
    }

    printf("Indexing harvest files...\n");

    while ((ret = readdir_r(directory, &tmpEntry, &currEntry)) == 0 &&
            currEntry != NULL)
    {
        entrySize = strlen(currEntry->d_name);

        if ( currEntry->d_name[0] == '.' )      // Skipping if hidden file
        {
            printf("Skipping hidden entry \"%s\"\n", currEntry->d_name);
        }
        else if ( strcmp(currEntry->d_name + entrySize - extenSize,
                         MWS_HARVEST_EXT) != 0 ) // Skipping if bad file extension
        {
            printf("Skipping bad extension file \"%s\"\n", currEntry->d_name);
        }
        else
        {
            files.push_back(currEntry->d_name);
        }
    }
    if (ret != 0)
    {
        perror("readdir:");
    }

    // Closing the directory
    closedir(directory);

    // Sorting the entries
    sort(files.begin(), files.end());

    printf("Loading harvest files...\n");

    // Loading the harvests from the respective entries
    for (it = files.begin(); it != files.end(); it++)
    {
        fullPath.set(dirPath.get());
        fullPath.append(*it);
        fd = open(fullPath.get(), O_RDONLY);
        if (fd < 0)
        {
            fprintf(stderr, "Error while opening \"%s\"\n", fullPath.get());
            continue;
        }

        printf("Loading %s... ", fullPath.get());
        fflush(stdout);
        loadReturn = loadMwsHarvestFromFd(fd, ctxt);
        if (loadReturn.first == 0)
        {
            printf("%d loaded\n", loadReturn.second);
        }
        else
        {
            printf("%d loaded (with errors)\n", loadReturn.second);
        }

        totalLoaded += loadReturn.second;

        close(fd);

        printf("Total %d\n", totalLoaded);
    }

    return totalLoaded;
}

}
