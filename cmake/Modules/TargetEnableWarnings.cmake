# - target_enable_warnings(<target>)
#
# © 2022 Cisco Systems, Inc. All rights reserved.

# enable all warnings for Visual Studio or makefiles
#
# NOTE
#  -Wno-missing-field-initializers and -Wno-missing-braces is added
#  because of a bug in gcc that's fixed in gcc4.7.0 and later
#
# See: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=36750
#
# Also see: https://stackoverflow.com/a/1539162
#
# Summary: C99 standards (and older C90 as well) will in fact zero
#          a whole struct when initialized with { 0 }.  Older versions
#          of gcc is erroneously issuing a warning when that syntax
#          is used.
#
macro(target_enable_warnings target)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W3 /WX)
    else(MSVC)
        target_compile_options(${target}
            PRIVATE
                -Wall
                -Wextra
                -Werror
                -Wno-missing-field-initializers
                -Wno-missing-braces
                -fstack-protector
        )
    endif(MSVC)
endmacro(target_enable_warnings target)
