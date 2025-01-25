#pragma once

enum class ARGV_CONSOLE: int {
    ARGV_IP_DEVICE = 1,
    ARGV_FILE_DATA,
    ARGV_TARGET_PROGRAM,
    ARGV_FILE_CONFIG,
    ARGV_MAX
};

#if M_FOUND_LIBIIO
int trx_test(int argc, char *argv[]);
int test_RX(int argc, char *argv[]);
int test_TX(int argc, char *argv[]);
int realtime_RX(int argc, char *argv[]);
#endif










