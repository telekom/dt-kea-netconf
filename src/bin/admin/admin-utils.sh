#!/bin/sh

# Copyright (C) 2014-2020 Internet Systems Consortium, Inc. ("ISC")
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# This is an utility script that is being included by other scripts.

# There are two ways of calling this method.
# mysql_execute SQL_QUERY - This call is simpler, but requires db_user,
#     db_password and db_name variables to be set.
# mysql_execute SQL_QUERY PARAM1 PARAM2 .. PARAMN - Additional parameters
#     may be specified. They are passed directly to mysql. This one is
#     more convenient to use if the script didn't parse db_user db_password
#     and db_name.
#
# It returns the mysql command exit status to the caller as $?
mysql_execute() {
    QUERY=$1
    shift

    if [ $# -gt 0 ] && [ $1 = '--no-database' ]; then
        mysql $(mysql_extra_arguments) -N -B --user="${db_user}" --password="${db_password}" -e "${QUERY}"
    elif [ $# -gt 0 ]; then
        mysql $(mysql_extra_arguments) -N -B "$@" -e "${QUERY}"
    else
        # Shellcheck complains about variables not being set. They're set in the script that calls this script.
        # shellcheck disable=SC2154
        mysql $(mysql_extra_arguments) -N -B --database="${db_name}" --user="${db_user}" --password="${db_password}" -e "${QUERY}"
    fi

    return $?
}

mysql_execute_script() {
    file=$1
    shift

    if [ $# -gt 0 ] && [ $1 = '--no-database' ]; then
        mysql $(mysql_extra_arguments) -N -B --user="${db_user}" --password="${db_password}" < "${file}"
    elif [ $# -gt 0 ]; then
        mysql $(mysql_extra_arguments) -N -B "$@" < "${file}"
    else
        mysql $(mysql_extra_arguments) -N -B --database="${db_name}" --user="${db_user}" --password="${db_password}" < "${file}"
    fi

    return $?
}

mysql_version() {
    mysql_execute "SELECT CONCAT_WS('.', version, minor) FROM schema_version;" "$@"
    return $?
}

mysql_config_version() {
    VERSION="0.0"

    RESULT=$(mysql_execute "SHOW TABLES;" "$@")
    ERRCODE=$?
    if [ $ERRCODE -ne 0 ]
    then
        printf "mysql_config_version table query failed, mysql status = $ERRCODE"
        exit 1
    fi

    COUNT=$(echo $RESULT | grep config_schema_version | wc -w)
    ERRCODE=$?
    if [ $ERRCODE -ne 0 ]
    then
        printf "mysql_config_version table query failed, mysql status = $ERRCODE"
        exit 1
    fi

    if [ $COUNT -gt 0 ]; then
        VERSION=$(mysql_execute "SELECT CONCAT(version,'.',minor) FROM config_schema_version;" "$@")
    fi
    ERRCODE=$?
    echo $VERSION

    return $ERRCODE
}

mysql_master_version() {
    VERSION="0.0"

    RESULT=`mysql_execute "SHOW TABLES;" "$@"`
    ERRCODE=$?
    if [ $ERRCODE -ne 0 ]
    then
        printf "mysql_master_version table query failed, mysql status = $ERRCODE"
        exit 1
    fi

    COUNT=`echo $RESULT | grep master_schema_version | wc -w`
    ERRCODE=$?
    if [ $ERRCODE -ne 0 ]
    then
        printf "mysql_master_version table query failed, mysql status = $ERRCODE"
        exit 1
    fi

    if [ $COUNT -gt 0 ]; then
        VERSION=$(mysql_execute "SELECT CONCAT(version,'.',minor) FROM master_schema_version;" "$@")
    fi
    ERRCODE=$?
    echo $VERSION

    return $ERRCODE
}

# Submits given SQL text to PostgreSQL
# There are two ways of calling this method.
# pgsql_execute SQL_QUERY - This call is simpler, but requires db_user,
#     db_password and db_name variables to be set.
# pgsql_execute SQL_QUERY PARAM1 PARAM2 .. PARAMN - Additional parameters
#     may be specified. They are passed directly to pgsql. This one is
#     more convenient to use if the script didn't parse db_user db_password
#     and db_name.
#
# It returns the pgsql command exit status to the caller as $?
pgsql_execute() {
    QUERY=$1
    shift

    export PGPASSWORD=${db_password}
    if [ $# -gt 0 ] && [ $1 = '--no-database' ]; then
        echo "${QUERY}" | psql $(pgsql_extra_arguments) --set ON_ERROR_STOP=1 -A -t -h "${db_host}" -q -U "${db_user}"
    elif [ $# -gt 0 ]; then
        echo "${QUERY}" | psql $(pgsql_extra_arguments) --set ON_ERROR_STOP=1 -A -t -h "${db_host}" -q "$@"
    else
        echo "${QUERY}" | psql $(pgsql_extra_arguments) --set ON_ERROR_STOP=1 -A -t -h "${db_host}" -q -U "${db_user}" -d "${db_name}"
    fi

    return $?
}

# Submits SQL in a given file to PostgreSQL
# There are two ways of calling this method.
# pgsql_execute SQL_FILE - This call is simpler, but requires db_user,
#     db_password and db_name variables to be set.
# pgsql_execute SQL_FILE PARAM1 PARAM2 .. PARAMN - Additional parameters
#     may be specified. They are passed directly to pgsql. This one is
#     more convenient to use if the script didn't parse db_user db_password
#     and db_name.
#
# It returns the pgsql command exit status to the caller as $?
pgsql_execute_script() {
    file=$1
    shift

    export PGPASSWORD=${db_password}
    if [ $# -gt 0 ] && [ $1 = '--no-database' ]; then
        psql $(pgsql_extra_arguments) --set ON_ERROR_STOP=1 -A -t -h "${db_host}" -q -U "${db_user}" -f "${file}"
    elif [ $# -gt 0 ]; then
        psql $(pgsql_extra_arguments) --set ON_ERROR_STOP=1 -A -t -h "${db_host}" -q -f "${file}" "$@"
    else
        psql $(pgsql_extra_arguments) --set ON_ERROR_STOP=1 -A -t -h "${db_host}" -q -U "${db_user}" -d "${db_name}" -f "${file}"
    fi

    return $?
}

pgsql_version() {
    pgsql_execute "SELECT version || '.' || minor FROM schema_version" "$@"
    return $?
}

pgsql_config_version() {
    VERSION="0.0"

    RESULT=`pgsql_execute "\d" "$@"`
    ERRCODE=$?
    if [ $ERRCODE -ne 0 ]
    then
        printf "pgsql_config_version table query failed, pgsql status = $ERRCODE"
        exit 1
    fi

    COUNT=`echo $RESULT | grep config_schema_version | wc -w`
    ERRCODE=$?
    if [ $ERRCODE -ne 0 ]
    then
        printf "pgsql_config_version table query failed, pgsql status = $ERRCODE"
        exit 1
    fi

    if [ $COUNT -gt 0 ]; then
        VERSION=`pgsql_execute "SELECT version || '.' || minor FROM config_schema_version" "$@"`
    fi
    ERRCODE=$?
    echo $VERSION

    return $ERRCODE
}

pgsql_master_version() {
    VERSION="0.0"

    RESULT=`pgsql_execute "\d" "$@"`
    ERRCODE=$?
    if [ $ERRCODE -ne 0 ]
    then
        printf "pgsql_master_version table query failed, pgsql status = $ERRCODE"
        exit 1
    fi

    COUNT=`echo $RESULT | grep master_schema_version | wc -w`
    ERRCODE=$?
    if [ $ERRCODE -ne 0 ]
    then
        printf "pgsql_master_version table query failed, pgsql status = $ERRCODE"
        exit 1
    fi

    if [ $COUNT -gt 0 ]; then
        VERSION=`pgsql_execute "SELECT version || '.' || minor FROM master_schema_version" "$@"`
    fi
    ERRCODE=$?
    echo $VERSION

    return $ERRCODE
}

cql_execute() {
    query=$1
    shift

    if [ $# -gt 0 ] && [ $1 = '--no-database' ]; then
        cqlsh $(cql_extra_arguments) -u "${db_user}" -p "${db_password}" -e "${query}"
    elif [ $# -gt 0 ]; then
        cqlsh $(cql_extra_arguments) "$@" -e "$query"
    else
        cqlsh $(cql_extra_arguments) -u "${db_user}" -p "${db_password}" -k "${db_name}" -e "${query}"
    fi

    return $?
}

cql_execute_script() {
    file=$1
    shift

    if [ $# -gt 0 ] && [ $1 = '--no-database' ]; then
        cqlsh $(cql_extra_arguments) -u "${db_user}" -p "${db_password}" -f "${file}"
    elif [ $# -gt 0 ]; then
        cqlsh $(cql_extra_arguments) "$@" -e "$file"
    else
        cqlsh $(cql_extra_arguments) -u "${db_user}" -p "${db_password}" -k "${db_name}" -f "${file}"
    fi

    return $?
}

cql_version() {
    version=$(cql_execute "SELECT version, minor FROM schema_version" "$@")
    error=$?
    version=$(echo "$version" | grep -A 1 "+" | grep -v "+" | tr -d ' ' | cut -d "|" -f 1-2 | tr "|" ".")
    echo $version
    return $error
}

cql_config_version() {
    version="0.0"
    result=$(cql_execute "DESCRIBE tables;" "$@")
    count=$(echo $result | grep config_schema_version | wc -w)
    if [ $count -gt 0 ]; then
        version=$(cql_execute "SELECT version, minor FROM config_schema_version" "$@")
        version=$(echo "$version" | grep -A 1 "+" | grep -v "+" | tr -d ' ' | cut -d "|" -f 1-2 --output-delimiter=".")
    fi
    error=$?
    echo $version
    return $error
}

cql_master_version() {
    version="0.0"
    result=$(cql_execute "DESCRIBE tables;" "$@")
    count=$(echo $result | grep master_schema_version | wc -w)
    if [ $count -gt 0 ]; then
        version=$(cql_execute "SELECT version, minor FROM master_schema_version" "$@")
        version=$(echo "$version" | grep -A 1 "+" | grep -v "+" | tr -d ' ' | cut -d "|" -f 1-2 --output-delimiter=".")
    fi
    error=$?
    echo $version
    return $error
}

mysql_extra_arguments() {
    local line=

    if [ "${db_host}" != 'localhost' ]; then
        line="${line} --host=${db_host}"
    fi

    if [ -n "${db_server_port}" ]; then
        line="${line} --port=${db_server_port}"
    fi

    printf '%s' "${line}" | xargs
}

pgsql_extra_arguments() {
    local line=

    if [ -n "${db_server_port}" ]; then
        line="${line} --port=${db_server_port}"
    fi

    printf '%s' "${line}" | xargs
}

cql_extra_arguments() {
    local line='--connect-timeout 10'

    if [ "${db_host}" != 'localhost' ]; then
        line="${line} ${db_host}"
    fi

    if [ -n "${db_server_port}" ]; then
        line="${line} ${db_server_port}"
    fi

    if [ -n "${db_server_version}" ]; then
        line="${line} --cqlversion=${db_server_version}"
    fi

    if [ -n "${db_use_ssl}" ]; then
        line="${line} --ssl"
    fi

    printf '%s' "${line}" | xargs
}

# recount IPv4 leases from scratch
_RECOUNT4_QUERY=\
"
START TRANSACTION; \
DELETE FROM lease4_stat; \
INSERT INTO lease4_stat (subnet_id, state, leases) \
    SELECT subnet_id, state, COUNT(*) \
    FROM lease4 WHERE state = 0 OR state = 1 \
    GROUP BY subnet_id, state; \
COMMIT;"
export _RECOUNT4_QUERY

# recount IPv6 leases from scratch
_RECOUNT6_QUERY=\
"
START TRANSACTION; \
DELETE FROM lease6_stat; \
INSERT INTO lease6_stat (subnet_id, lease_type, state, leases) \
    SELECT subnet_id, lease_type, state, COUNT(*) \
    FROM lease6 WHERE state = 0 OR state = 1 \
    GROUP BY subnet_id, lease_type, state; \
COMMIT;"
export _RECOUNT6_QUERY
