#!/bin/bash

# Copyright (c) 2014, AllSeen Alliance. All rights reserved.
#
#    Permission to use, copy, modify, and/or distribute this software for any
#    purpose with or without fee is hereby granted, provided that the above
#    copyright notice and this permission notice appear in all copies.
#
#    THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
#    WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
#    MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
#    ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
#    WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
#    ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
#    OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

set -x
set -e
SELF_DIR=$(cd $(dirname $0) > /dev/null; pwd)

if [ -z "${AJ_ROOT}" ]; then
    AJ_ROOT="${SELF_DIR}/../../.."
fi

AJN_DD_PATH=$(cd "${SELF_DIR}/.." > /dev/null; pwd)


AJN_DAEMON_PNAME="alljoyn-daemon"
if [ -z "${VARIANT}" ]; then
    VARIANT="debug"
fi

if [ -d "${AJN_DD_PATH}/build/linux/x86/${VARIANT}" ]; then
    PLATFORM="x86"
elif [ -d "${AJN_DD_PATH}/build/linux/x86_64/${VARIANT}" ]; then
    PLATFORM="x86_64"
else
    printf "Tests are not built yet!\nMake sure to export GTEST_DIR to the right Gtest home path and then build again, e.g.: $> scons CPU=x86 BINDINGS=cpp,c WS=off\n"
    exit 1
fi

PLATFORM_ROOT="${AJN_DD_PATH}/build/linux/${PLATFORM}/${VARIANT}"
OBJ_ROOT="${PLATFORM_ROOT}/obj/datadriven_cpp"

LIB_PATH="${PLATFORM_ROOT}/dist/datadriven_cpp/lib:${PLATFORM_ROOT}/dist/cpp/lib:${PLATFORM_ROOT}/dist/about/lib"

export LD_LIBRARY_PATH="${LIB_PATH}"

# make sure coverage starts clean
if [ ! -z "$(which lcov)" ]; then
    lcov --directory "${PLATFORM_ROOT}" --zerocounters
fi

# running unit tests
# we are doing some magic here to run each test in its own process as we still have some issues to run them in one go (AS-207)
echo "[[ Cleaning old Gtest results if any ]]"
UNITTEST_ROOT="${OBJ_ROOT}/unit_test"

if ! nm "${UNITTEST_ROOT}/ddtest" | grep BundledRouter &> /dev/null; then
    if [ "$(pidof ${AJN_DAEMON_PNAME})" ]; then
         echo "alljoyn-daemon is active...running tests..."
    else
         echo "Please start an alljoyn-daemon to be able to run the tests !"
         exit 1
    fi
fi

rm -rf "${UNITTEST_ROOT}"/gtestresults/
echo "[[ Running unit tests ]]"
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
"${UNITTEST_ROOT}"/ddtest --gtest_list_tests | awk -f "${DIR}/parse_list_tests.awk" | while read line
do
    "${UNITTEST_ROOT}"/ddtest --gtest_output=xml:"${UNITTEST_ROOT}"/gtestresults/ --gtest_filter=$line || exit 1
done

# running system tests
echo "[[ Running system tests ]]"
SYSTEST_ROOT="${OBJ_ROOT}/test"
for dir in $(ls "${SYSTEST_ROOT}"); do
    if [ -x "${SYSTEST_ROOT}/${dir}"/run.sh ]; then
        echo "[[[[ ${dir} ]]]]"
        "${SYSTEST_ROOT}/${dir}"/run.sh || exit 1
    fi
done

echo "[[ Running hybrid test ]]"
RUNTEST=1 "${PLATFORM_ROOT}"/dist/datadriven_cpp/bin/samples/hybriddoor_cons_prov home office school || exit 1


# generate coverage report (lcov 1.10 or better required for --no-external)
if [ ! -z "$(which lcov)" ]; then
    if [ $(lcov --version | cut -d'.' -f2) -ge 10 ]; then
        EXTRA_ARGS="--no-external"
    fi
    COVDIR="${AJN_DD_PATH}"/build/coverage
    mkdir -p "${COVDIR}" > /dev/null 2>&1

    lcov $EXTRA_ARGS --quiet --capture \
         --base-directory "${AJN_DD_PATH}" \
         --directory "${PLATFORM_ROOT}"/obj/datadriven_cpp/lib/ \
         --output-file "${COVDIR}"/ddapi.info
    genhtml --prefix "${PLATFORM_ROOT}" --quiet --output-directory "${COVDIR}" "${COVDIR}"/ddapi.info || true
fi

exit 0
