/**************************************************************************
 *       Copyright (c) 2023, Cisco Systems, All Rights Reserved
 ***************************************************************************
 *
 *  @file:    ScopedGuard.h
 *
 ***************************************************************************
 * @desc Calls in a destructor lambda,passed in constructor.
 ***************************************************************************/
#pragma once

namespace util
{
template<typename TFunc>
class scoped_guard
{
public:
    explicit scoped_guard(const TFunc& fn):
        fn_(fn)
    {}
    ~scoped_guard()
    {
        fn_();
    }
    scoped_guard(const scoped_guard&) = delete;
    scoped_guard(scoped_guard&&) = delete;
    scoped_guard& operator =(const scoped_guard&) =  delete;
    scoped_guard& operator =(scoped_guard&&) =  delete;
private:
    TFunc fn_;
};
} //namespace util
