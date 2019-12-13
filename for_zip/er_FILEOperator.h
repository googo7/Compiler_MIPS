#pragma once

#include <string>
#include "er_TkAnalysis.h"
#include "er_Error.h"
void er_open_file(void);
void er_read_from_file(void);
void er_write_into_file(er_token_info&);
void er_write_into_file(std::string);
void er_write_into_file(er_error_info&);
void er_close_file(void);