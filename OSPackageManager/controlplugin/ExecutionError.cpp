/**
 * @file
 * @copyright (c) 2023 Cisco Systems, Inc. All rights reserved
 */

#include "ExecutionError.hpp"

ExecutionError::ExecutionError(std::error_code ec):
    std::system_error(std::move(ec))
{
}
