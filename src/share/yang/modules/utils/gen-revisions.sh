#!/bin/bash
# Copyright (C) 2018-2019 Internet Systems Consortium, Inc. ("ISC")
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# Build module / revision table
#
# This developer script builds the module / revision table, i.e. fill
# the content of the src/lib/yang/yang_revisions.h as explained in
# src/lib/yang/yang_revisions.h.skel
# The keatest-module entry is enabled only when KEATEST_MODULE is defined.

# Allow running from any path.
pushd "$(dirname "$(readlink -f "${0}")")/.." > /dev/null

test=$(basename "$(find . -mindepth 1 -maxdepth 1 -name 'keatest-module*.yang')")
last=$(basename "$(find . -mindepth 1 -maxdepth 1 -name '*.yang' | sort -V | tail -n 1)")

for m in $(find . -mindepth 1 -maxdepth 1 -name '*.yang' | sort -V); do
    m=$(basename "${m}")
    if test "${m}" = "${test}"; then
        echo '#ifdef KEATEST_MODULE'
    fi
    b=$(cut -d '.' -f 1 <<<"${m}")
    r=$(yanglint -f yin "${m}" | grep '<revision date=' | head -n 1 | sed \
      's/.*<revision date="\([0-9][0-9][0-9][0-9]-[0-9][0-9]-[0-9][0-9]\)".*/\1/')
    c=','
    if test "${m}" = "${last}"; then
        c=''
    fi
    printf '    { "%s", "%s" }%s\n' "${b}" "${r}" "${c}"
    if test "${m}" = "${test}"; then
        printf '#endif  // KEATEST_MODULE\n'
    fi
done
