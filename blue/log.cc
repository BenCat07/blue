#include "stdafx.hh"

#include "log.hh"
#include "signature.hh"

auto Log::msg(const char *format, ...) -> void {

    char    buffer[1024];
    va_list vlist;
    va_start(vlist, format);
    vsnprintf(buffer, 1024, format, vlist);
    va_end(vlist);

    using MessageFn = void (*)(const char *format, ...);

    static auto msg_fn = (MessageFn)Signature::resolve_import(Signature::resolve_library("tier0"), "Msg");

    msg_fn("[Blue] %s\n", buffer);

    printf("[Blue] %s\n", buffer);
}
