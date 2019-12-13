#pragma once

#include <string>
#include "TkAnalysis.h"
#include "Error.h"
#include "MidCode.h"

void open_file(void);
void read_from_file(void);
void write_into_file(token_info);
void write_into_file(std::string);
void write_into_file(error_info);
void write_into_file(MidCode);
void write_into_mfile(std::string);
void close_file(void);