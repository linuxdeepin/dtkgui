#!/bin/bash

# SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: LGPL-3.0-or-later

BUILD_DIR=`pwd`/../build/tests/
HTML_DIR=${BUILD_DIR}/html
XML_DIR=${BUILD_DIR}/report

export ASAN_OPTIONS="halt_on_error=0"

# back to project directroy
cd ..

cmake -Bbuild -DCMAKE_BUILD_TYPE=Debug 

cmake --build build --target ut-DtkGui -j$(nproc)

cd $BUILD_DIR

./ut-DtkGui --gtest_output=xml:${XML_DIR}/report_dtkgui.xml

lcov -d ./ -c -o coverage_all.info
#lcov --extract coverage_all.info $EXTRACT_ARGS --output-file coverage.info
lcov --remove coverage_all.info "*/tests/*" "*/usr/include*" "*build-ut/src*" --output-file coverage.info
cd ..
genhtml -o $HTML_DIR $BUILD_DIR/coverage.info && mv ${BUILD_DIR}/html/index.html ${BUILD_DIR}/html/cov_dtkgui.html

test -e ${BUILD_DIR}/asan.log* && mv ${BUILD_DIR}/asan.log* ${BUILD_DIR}/asan_dtkgui.log || touch ${BUILD_DIR}/asan_dtkgui.log

#rm -rf $BUILD_DIR
#rm -rf ../$BUILD_DIR
