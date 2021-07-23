#!/bin/bash
# Copyright (C) 2018 Internet Systems Consortium, Inc. ("ISC")
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# Check revisions
#
# This developer script verifies versions in module contents match
# the version in the name.
# Requires yanglint to translate YANG to YIN formats.
# Fixme: use xlstproc to extract the revision.

# Allow running from any path.
pushd "$(dirname "$(readlink -f "${0}")")/.." > /dev/null

exit_code=0
for m in $(find . -mindepth 1 -maxdepth 1 -name '*.yang' | sort -V); do
    rev1=$(yanglint -f yin "${m}" | grep '<revision date=' | head -n 1 | sed \
      's/.*<revision date="\([0-9][0-9][0-9][0-9]-[0-9][0-9]-[0-9][0-9]\)".*/\1/')
    rev2=$(sed 's/.*@\([0-9][0-9][0-9][0-9]-[0-9][0-9]-[0-9][0-9]\)\..*/\1/' <<<"${m}")

    if test "${rev1}" != "${rev2}"; then
        exit_code=1
        printf 'revision mismatch on %s got %s\n' "${m}" "${rev1}"
    fi
done
exit ${exit_code}
