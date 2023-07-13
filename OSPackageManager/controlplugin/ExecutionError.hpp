/**************************************************************************
 *       Copyright (c) 2023, Cisco Systems, All Rights Reserved
 ***************************************************************************
 *
 *  @file:    ExecutionError.hpp
 *
 ***************************************************************************
 * @desc  Custom exception. It is planned to be thrown in case of error after execv system call.
 ***************************************************************************/
#pragma once
#include <system_error>

class ExecutionError: public std::system_error
{
public:
    ExecutionError(std::error_code ec);
};
