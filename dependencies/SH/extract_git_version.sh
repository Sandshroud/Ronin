#!/bin/sh
version=`git log -1 --pretty=format:"%h"`
timestamp=`git log -1 --pretty=format:"%ct"`
tag=`git log -1 --pretty=format:"%d" | cut -d ' ' -f3 | cut -d ')' -f1  | cut -d ',' -f1 | cut -d '/' -f2`
count=`git rev-list --count HEAD`
hostname=`hostname`
username=`whoami | cut -d\\\ -f2`
now=$(date +"%s")

if [ -z "$version" ]
then
	version="0"
fi

if [ -z "$timestamp" ]
then
	timestamp="0"
fi

echo "" >> "../../src/hearthstone-shared/git_version.h"
echo "// Do not alter, automatically extracted." > "../../src/hearthstone-shared/git_version.h"
echo "#pragma once" >> "../../src/hearthstone-shared/git_version.h"
echo "" >> "../../src/hearthstone-shared/git_version.h"
echo "// Our cmake generation time" >> "../../src/hearthstone-shared/git_version.h"
echo "#define CMAKE_GENERATED \"$now\"" >> "../../src/hearthstone-shared/git_version.h"
echo "" >> "../../src/hearthstone-shared/git_version.h"
echo "// Branch info" >> "../../src/hearthstone-shared/git_version.h"
echo "#define BUILD_TAG \"$tag\"" >> "../../src/hearthstone-shared/git_version.h"
echo "#define BUILD_REVISION $count" >> "../../src/hearthstone-shared/git_version.h"
echo "" >> "../../src/hearthstone-shared/git_version.h"
echo "// Commit info" >> "../../src/hearthstone-shared/git_version.h"
echo "#define BUILD_HASH 0x$version" >> "../../src/hearthstone-shared/git_version.h"
echo "#define BUILD_HASH_STR \"$version\"" >> "../../src/hearthstone-shared/git_version.h"
echo "#define BUILD_USER_STR \"$username\"" >> "../../src/hearthstone-shared/git_version.h"
echo "#define BUILD_HOST_STR \"$hostname\"" >> "../../src/hearthstone-shared/git_version.h"
echo "#define COMMIT_TIMESTAMP $timestamp" >> "../../src/hearthstone-shared/git_version.h"
