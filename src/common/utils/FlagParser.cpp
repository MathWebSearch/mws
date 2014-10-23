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
  * @brief File containing the implementation of the FlagParser class.
  *
  * @file FlagParser.cpp
  * @author Corneliu-Claudiu Prodescu
  * @date 24 Jul 2011
  *
  * License: GPL v3
  */

// System includes

#include <assert.h>
#include <getopt.h>  // C getopt function family
#include <map>       // C++ map container
#include <string>    // C string header
#include <vector>    // C++ vector container
#include <cstdio>

// Local includes

#include "common/utils/FlagParser.hpp"  // FlagParser class declaration

// Namespaces

using namespace std;

// Static declarations

int verbose;

namespace common {
namespace utils {

string FlagParser::_progName;
map<char, FlagOpt> FlagParser::_flagOpts;
map<char, vector<string> > FlagParser::_parsedArgs;
vector<string> FlagParser::_params;
int FlagParser::_minNumParams = -1;

void FlagParser::addFlag(const char aShortOpt, const string& aLongOpt,
                         int optMandatory, int argMandatory) {
    FlagOpt currentFlag;

    currentFlag.longOpt = aLongOpt;
    currentFlag.optMandatory = optMandatory;
    currentFlag.argMandatory = argMandatory;
    currentFlag.parsed = false;

    _flagOpts[aShortOpt] = currentFlag;
}

int FlagParser::parse(const int argc, char* const argv[]) {
    map<char, FlagOpt>::iterator it;
    vector<struct option> longOpts;
    string optString;
    signed char ch;
    struct option currentOpt;
    const struct option zeroOpt = {nullptr, 0, nullptr, 0};
    int optIndex;

    // Add help and verbose flags
    addFlag('h', "help", FLAG_OPT, ARG_NONE);
    addFlag('v', "verbose", FLAG_OPT, ARG_NONE);

    // Storing program name
    _progName = argv[0];

    // Building optString and longOpts
    optString = "+";  // Setting POSIXLY_CORRECT ON

    for (it = _flagOpts.begin(); it != _flagOpts.end(); it++) {
        optString += it->first;
        if (it->second.argMandatory == ARG_REQ) {
            optString += ":";
        } else if (it->second.argMandatory == ARG_OPT) {
            optString += "::";
        }

        currentOpt.name = it->second.longOpt.c_str();
        currentOpt.has_arg = it->second.argMandatory;
        currentOpt.flag = nullptr;
        currentOpt.val = it->first;

        longOpts.push_back(currentOpt);
    }

    // Adding a 0 fill element
    longOpts.push_back(zeroOpt);

    while ((ch = getopt_long(argc, argv, optString.c_str(), longOpts.data(),
                             &optIndex)) != -1) {
        // Handle -h / --help
        if (ch == 'h') {
            fprintf(stdout, "%s", getUsage().c_str());
            exit(EXIT_SUCCESS);
        }

        // Handle -v / --verbose
        if (ch == 'v') {
            fprintf(stdout, "%s: verbose mode\n", argv[0]);
            verbose = true;
        }

        // Record flags (and arguments)
        if ((it = _flagOpts.find(ch)) != _flagOpts.end()) {
            it->second.parsed = true;
            switch (it->second.argMandatory) {
            case ARG_NONE:
                _parsedArgs.insert(make_pair(ch, vector<string>()));
                break;
            case ARG_OPT:
                if (optarg == nullptr) continue;
                _parsedArgs[ch].push_back(optarg);
                break;
            case ARG_REQ:
                if (optarg == nullptr) goto failure;
                _parsedArgs[ch].push_back(optarg);
                break;

            default:
                assert(0);
            }
        } else {
            goto failure;
        }
    }

    /* Parse params */
    while (optind < argc) {
        _params.push_back(argv[optind++]);
    }

    // Check if all mandatory arguments were parsed
    for (it = _flagOpts.begin(); it != _flagOpts.end(); it++) {
        if (it->second.optMandatory == FLAG_REQ && !it->second.parsed) {
            fprintf(stderr, "%s: option -%c|--%s is required\n", argv[0],
                    it->first, it->second.longOpt.c_str());
            goto failure;
        }
    }

    // Check if number of params is valid
    if (_minNumParams == -1 && _params.size() != 0) {
        fprintf(stderr, "%s: No params are allowed\n", argv[0]);
        goto failure;
    } else if (_minNumParams > 0 && _params.size() < (size_t)_minNumParams) {
        fprintf(stderr, "%s: At least %d params are required\n", argv[0],
                _minNumParams);
        goto failure;
    }

    return 0;

failure:
    return -1;
}

bool FlagParser::hasArg(char aShortOpt) {
    map<char, vector<string> >::iterator it;

    if (_parsedArgs.find(aShortOpt) != _parsedArgs.end()) {
        return true;
    } else {
        return false;
    }
}

std::string FlagParser::getArg(char aShortOpt) {
    vector<string>& args = _parsedArgs[aShortOpt];

    if (!args.empty())
        return args[0];
    else
        return "";
}

std::vector<std::string> FlagParser::getArgs(char aShortOpt) {
    return _parsedArgs[aShortOpt];
}

std::string FlagParser::getUsage() {
    map<char, FlagOpt>::iterator it;
    string result;
    bool first;
    string begin, sep1, arg, sep2, align;

    align = string(_progName.size(), ' ');
    result = "Usage:\n";
    first = true;

    for (it = _flagOpts.begin(); it != _flagOpts.end(); it++) {
        if (it->second.optMandatory == FLAG_REQ) {
            sep1 = " ";
            sep2 = "";
        } else {
            sep1 = "[";
            sep2 = "]";
        }

        if (it->second.argMandatory == ARG_NONE) {
            arg = "";
        } else if (it->second.argMandatory == ARG_REQ) {
            arg = "arg";
        } else {
            arg = "[arg]";
        }

        if (first) {
            begin = _progName;
            first = false;
        } else {
            begin = align;
        }

        result += begin + " " + sep1 + "-" + it->first + "|--" +
                  it->second.longOpt + " " + arg + " " + sep2 + "\n";
    }

    if (_minNumParams >= 0) {
        if (_minNumParams == 0) {
            sep1 = "[";
            sep2 = "]";
        } else {
            sep1 = " ";
            sep2 = " ";
        }
        result += begin + " " + sep1 + " params... " + sep2 + "\n";
    }

    return result;
}
}
}
