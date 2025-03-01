// Copyright (C) 2011-2015 Internet Systems Consortium, Inc. ("ISC")
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <config.h>

#include <log/message_dictionary.h>
#include <log/message_initializer.h>
#include <algorithm>
#include <cassert>
#include <cstdlib>


namespace {

using namespace isc::log;

/// @brief Returns the shared pointer to the list of pointers to the
/// log messages defined.
///
/// The returned pointer must be held in the \c MessageInitializer object
/// throughout its lifetime to make sure that the object doesn't outlive
/// the list and may still access it in the destructor. The returned
/// pointer is shared between all \c MessageInitializer instances.
LoggerValuesListPtr
getMutableLoggerValues() {
    static LoggerValuesListPtr logger_values(new LoggerValuesList());
    return (logger_values);
}

/// @brief Returns the pointer to the list of message duplicates.
///
/// The returned pointer must be held in the \c MessageInitializer object
/// throughout its lifetime to make sure that the object doesn't outlive
/// the list and may still access it in the destructor. The returned
/// pointer is shared between all \c MessageInitializer instances.
LoggerDuplicatesListPtr
getMutableDuplicates() {
    static LoggerDuplicatesListPtr duplicates(new LoggerDuplicatesList());
    return (duplicates);
}
} // end unnamed namespace


namespace isc {
namespace log {

MessageInitializer::MessageInitializer(const char* values[])
    : values_(values),
      global_dictionary_(MessageDictionary::globalDictionary()),
      global_logger_values_(getMutableLoggerValues()),
      global_logger_duplicates_(getMutableDuplicates()) {
    global_logger_values_->push_back(values);
}

MessageInitializer::~MessageInitializer() {
    // Search for the pointer to pending messages belonging to our instance.
    LoggerValuesList::iterator my_messages = std::find(global_logger_values_->begin(),
                                                       global_logger_values_->end(),
                                                       values_);
    bool pending = (my_messages != global_logger_values_->end());
    // Our messages are still pending, so let's remove them from the list
    // of pending messages.
    if (pending) {
        global_logger_values_->erase(my_messages);

    } else {
        // Our messages are not pending, so they might have been loaded to
        // the dictionary and/or duplicates.
        int i = 0;
        while (values_[i]) {
            // Check if the unloaded message is registered as duplicate. If it is,
            // remove it from the duplicates list.
            LoggerDuplicatesList::iterator dup =
                std::find(global_logger_duplicates_->begin(),
                          global_logger_duplicates_->end(),
                          values_[i]);
            if (dup != global_logger_duplicates_->end()) {
                global_logger_duplicates_->erase(dup);

            } else {
                global_dictionary_->erase(values_[i], values_[i + 1]);
            }
            i += 2;
        }
    }
}

// Return the number of arrays registered but not yet loaded.

size_t
MessageInitializer::getPendingCount() {
    return (getMutableLoggerValues()->size());
}

// Load the messages in the arrays registered in the logger_values array
// into the global dictionary.

void
MessageInitializer::loadDictionary(bool ignore_duplicates) {
    const MessageDictionaryPtr& global = MessageDictionary::globalDictionary();
    const LoggerValuesListPtr& logger_values = getMutableLoggerValues();

    for (LoggerValuesList::const_iterator values = logger_values->begin();
         values != logger_values->end(); ++values) {
        std::vector<std::string> repeats = global->load(*values);

        // Append the IDs in the list just loaded (the "repeats") to the
        // global list of duplicate IDs.
        if (!ignore_duplicates && !repeats.empty()) {
            const LoggerDuplicatesListPtr& duplicates = getMutableDuplicates();
            duplicates->insert(duplicates->end(), repeats.begin(), repeats.end());
        }
    }

    // ... and mark that the messages have been loaded.  (This avoids a lot
    // of "duplicate message ID" messages in some of the unit tests where the
    // logging initialization code may be called multiple times.)
    logger_values->clear();
}

// Return reference to duplicates vector
const std::list<std::string>&
MessageInitializer::getDuplicates() {
    return (*getMutableDuplicates());
}

// Clear the duplicates vector
void
MessageInitializer::clearDuplicates() {
    getMutableDuplicates()->clear();
}

} // namespace log
} // namespace isc
