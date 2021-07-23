// (C) 2020 Deutsche Telekom AG.
//
// Deutsche Telekom AG and all other contributors /
// copyright owners license this file to you under the Apache
// License, Version 2.0 (the "License"); you may not use this
// file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied. See the License for the
// specific language governing permissions and limitations
// under the License.

#include <config.h>

#include <database/timestamp_store.h>

#include <boost/lexical_cast.hpp>

#include <ctime>
#include <fstream>
#include <memory>
#include <random>
#include <regex>

namespace isc {
namespace db {

using boost::bad_lexical_cast;
using boost::lexical_cast;
using std::istringstream;
using std::mt19937_64;
using std::random_device;
using std::regex;
using std::regex_match;
using std::regex_replace;
using std::string;
using std::to_string;
using std::uniform_int_distribution;

namespace {

static string const timestamp_match("(.*?$)");

}  // namespace

void TimestampStore::del(string const& table, string const& key) {
    if (!config_timestamp_) {
        return;
    }

    // Read from the file.
    string content;
    *config_timestamp_ >> content;

    // Check if the line is there.
    istringstream stream(content);
    string line;
    string new_content;
    while (getline(stream, line)) {
        regex r(regex(table + " " + key + " " + timestamp_match));
        if (!regex_match(line, r)) {
            new_content += line + "\n";
        }
    }

    // Write back to the file.
    *config_timestamp_ << new_content;
}

timestamp_t TimestampStore::read(string const& table, string const& key) {
    if (!config_timestamp_) {
        return 0;
    }

    // Read from file.
    string content;
    *config_timestamp_ >> content;

    // Check if the line is there.
    istringstream stream(content);
    string line;
    while (getline(stream, line)) {
        regex r(regex(table + " " + key + " " + timestamp_match));
        if (regex_match(line, r)) {
            // If it is, replace the timestamp.
            return lexical_cast<timestamp_t>(regex_replace(line, r, "$1$2"));
        }
    }
    return 0;
}

void TimestampStore::write(string const& table, string const& key, timestamp_t const& timestamp) {
    if (!config_timestamp_) {
        return;
    }

    // Read from the file.
    string content;
    *config_timestamp_ >> content;

    // Check if the line is there.
    istringstream stream(content);
    string line;
    string new_content;
    bool found(false);
    string new_line(table + " " + key + " " + to_string(timestamp));
    while (getline(stream, line)) {
        regex r(regex(table + " " + key + " " + timestamp_match));
        if (!found && regex_match(line, r)) {
            // If it is, replace the timestamp.
            new_content += regex_replace(line, r, new_line);
            found = true;
        } else {
            new_content += line;
        }
        new_content += "\n";
    }

    // If not found, add it.
    if (!found) {
        new_content += new_line + "\n";
    }

    // Write back to the file.
    *config_timestamp_ << new_content;
}

timestamp_t TimestampStore::generate() {
    mt19937_64 engine(random_device{}());
    uniform_int_distribution<timestamp_t> distribution;
    return distribution(engine);
}

timestamp_t TimestampStore::now() {
    string const& result(to_string(time(nullptr)) + to_string(generate()).substr(0, 9));
    try {
        return lexical_cast<timestamp_t>(result);
    } catch (bad_lexical_cast const& exception) {
        isc_throw(Unexpected,
                  string(exception.what()) + string(" when trying to convert ") + result);
    }
}

}  // namespace db
}  // namespace isc
