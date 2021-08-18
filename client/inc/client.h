#pragma once

#include <csignal>
#include <iostream>
#include <sstream>

#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "encryption.h"

bool g_run = true;