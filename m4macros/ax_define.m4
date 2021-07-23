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

# AC_DEFINE_SUBST(NAME, VALUE, DESCRIPTION)
# -----------------------------------------
AC_DEFUN([AC_DEFINE_SUBST], [
    # AC_DEFINE adds a $1 define entry with value $2 and comment $3 in the
    # config.h C++ header.
    AC_DEFINE([$1], [$2], [$3])
    # AC_SUBST makes a textual substitution from $1 to $2 *.in files.
    AC_SUBST([$1], ['$2'])
])

# AC_DEFINE_UNQOUTED_SUBST(NAME, VALUE, DESCRIPTION)
# -----------------------------------------
AC_DEFUN([AC_DEFINE_UNQUOTED_SUBST], [
    AC_DEFINE_UNQUOTED([$1], [$2], [$3])
    AC_SUBST([$1], [$2])
])
