#!/bin/bash
# Copyright (C) 2018 Internet Systems Consortium, Inc. ("ISC")
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# Check hashes
#
# This developer script verifies recorded hashes still match the
# result of SHA-256 checksums of the YIN format.
# Requires yanglint to translate YANG to YIN formats and openssl
# for a system independent SHA-256.

# Allow running from any path.
pushd "$(dirname "$(readlink -f "${0}")")/.." > /dev/null

# Parse arguments.
while (( ${#} > 0 )); do
  case "${1}" in
  # -a|--amend                   amend the file with the correct copyright
  '-a'|'--amend') amend=true ;;
  # [-d|--debug]                 Enables debug mode, showing every executed statement.
  '-d'|'--debug') set -x ;;
  # Unrecognized argument
  *) printf "${RED}ERROR: Unrecognized argument '%s'${RESET}\\n" "${1}" 1>&2; exit 1 ;;
  esac; shift
done

[[ -z "${amend+x}" ]] && amend=false

exit_code=0
for m in $(find . -mindepth 1 -maxdepth 1 -name '*.yang'); do
    if ${amend}; then
      yanglint -f yang "${m}" > /tmp/a.yang && mv /tmp/a.yang "${m}"
    fi
    hash1=$(yanglint -f yin "${m}" | openssl dgst -sha256 | sed 's/(stdin)= //')
    h="hashes/$(basename "${m}" .yang).hash"
    if test -f "${h}"; then
        hash2=$(cat "${h}")
        if test "${hash1}" != "${hash2}"; then
            exit_code=1
            printf 'hash mismatch on %s expected %s in %s\n' "${m}" "${hash1}" "${h}"
            if ${amend}; then
                printf '%s\n' "${hash1}" > "${h}"
            fi
        fi
    else
        exit_code=1
        printf 'missing hash file %s for %s\n' "${h}" "${m}"
        if ${amend}; then
            printf '%s\n' "${hash1}" > "${h}"
        fi
    fi
done
exit ${exit_code}
