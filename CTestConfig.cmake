set(CTEST_NIGHTLY_START_TIME "02:00:00 CET")
set(CTEST_DROP_METHOD "http")
set(CTEST_DROP_SITE "my.cdash.org")
set(CTEST_DROP_LOCATION "/submit.php?project=Deska")
set(CTEST_DROP_SITE_CDASH TRUE)

find_program(HOSTNAME_CMD NAMES hostname)
exec_program(${HOSTNAME_CMD} ARGS --fqdn OUTPUT_VARIABLE HOSTNAME)
set(CTEST_SITE "${HOSTNAME}")

find_program(LSB_RELEASE_CMD NAMES lsb_release)
exec_program(${LSB_RELEASE_CMD} ARGS -d -s OUTPUT_VARIABLE LSB_RELEASE_DESC)
set(CTEST_BUILD_NAME "${LSB_RELEASE_DESC}")