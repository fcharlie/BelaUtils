#!/bin/bash

# This file is part of hwzip from https://www.hanshq.net/zip.html
# It is put in the public domain; see the LICENSE file for details.

set -eux
set -o pipefail

dir="$PWD"

cd `mktemp -d`

# Create a zip file with Info-ZIP.
echo -n foo > foo
echo -n nanananana > bar
mkdir dir
echo -n baz > dir/baz
echo This is a test comment. | zip test.zip --archive-comment -r foo bar dir

# List it.
diff -su - <("$dir/hwzip" list test.zip) <<END

HWZIP 1.4 -- A very simple ZIP program from https://www.hanshq.net/zip.html

Listing ZIP archive: test.zip

This is a test comment.

foo
bar
dir/
dir/baz

END

# Extract it.
mkdir out
cd out
diff -su - <("$dir/hwzip" extract ../test.zip) <<END

HWZIP 1.4 -- A very simple ZIP program from https://www.hanshq.net/zip.html

Extracting ZIP archive: ../test.zip

This is a test comment.

 Extracting: foo
  Inflating: bar
 (Skipping dir: dir/)
 (Skipping file in dir: dir/baz)

END

cmp ../foo foo
cmp ../bar bar


# Create a ZIP file without comment.
cd ..
diff -su - <("$dir/hwzip" create test2.zip foo bar) <<END

HWZIP 1.4 -- A very simple ZIP program from https://www.hanshq.net/zip.html

Creating ZIP archive: test2.zip

   Stored: foo
 Deflated: bar (50%)

END

diff -su - <(unzip -t test2.zip foo bar) <<END
Archive:  test2.zip
    testing: foo                      OK
    testing: bar                      OK
No errors detected in test2.zip for the 2 files tested.
END


# Create a ZIP file with comment.
diff -su - <("$dir/hwzip" create test3.zip -c "Hello, world!" foo bar) <<END

HWZIP 1.4 -- A very simple ZIP program from https://www.hanshq.net/zip.html

Creating ZIP archive: test3.zip

Hello, world!

   Stored: foo
 Deflated: bar (50%)

END

diff -su - <(unzip -t test3.zip foo bar) <<END
Archive:  test3.zip
Hello, world!
    testing: foo                      OK
    testing: bar                      OK
No errors detected in test3.zip for the 2 files tested.
END


# Create an empty zip file.
diff -su - <("$dir/hwzip" create empty.zip) <<END

HWZIP 1.4 -- A very simple ZIP program from https://www.hanshq.net/zip.html

Creating ZIP archive: empty.zip


END
diff -su - <(unzip -t empty.zip) <<END
Archive:  empty.zip
warning [empty.zip]:  zipfile is empty
END


# Empty with comment.
diff -su - <("$dir/hwzip" create empty2.zip -c "Hello, world!") <<END

HWZIP 1.4 -- A very simple ZIP program from https://www.hanshq.net/zip.html

Creating ZIP archive: empty2.zip

Hello, world!


END
diff -su - <(unzip -t empty2.zip) <<END
Archive:  empty2.zip
Hello, world!
warning [empty2.zip]:  zipfile is empty
END



echo ALL TESTS PASSED
