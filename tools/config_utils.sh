#!/bin/bash
# (C) 2020 Deutsche Telekom AG.
#
# Deutsche Telekom AG and all other contributors /
# copyright owners license this file to you under the Apache
# License, Version 2.0 (the "License"); you may not use this
# file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# KIND, either express or implied. See the License for the
# specific language governing permissions and limitations
# under the License.

function element-in() {
  local e match="${1}"
  shift
  for e; do [[ "${e}" == "${match}" ]] && return 0; done
  return 1
}

function check-if-library-is-supported() {
  if test -z "${library}"; then
    printf 'ERROR: library not specified\n' 1>&2
    print-help
    exit 3
  fi
  if ! element-in ${library} "${supported_libraries[@]}"; then
    printf 'ERROR: library %s not supported\n' "${library}" 1>&2
    print-help
    exit 4
  fi
}

