/*
 * Copyright (c) 2005 Alexander Gottwald
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE ABOVE LISTED COPYRIGHT HOLDER(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Except as contained in this notice, the name(s) of the above copyright
 * holders shall not be used in advertising or otherwise to promote the sale,
 * use or other dealings in this Software without prior written authorization.
 */
#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <string>
struct CConfig
{
    enum {MultiWindow, Fullscreen, Windowed, Nodecoration} window;
    enum {NoClient, StartProgram, XDMCP} client;
    bool local;
    std::string display;
    std::string protocol;
    std::string program;
    std::string host;
    std::string user;
    bool broadcast;
    bool indirect;
    std::string xdmcp_host;
    bool clipboard;
    std::string extra_params;
#ifdef _DEBUG
    CConfig() : window(MultiWindow), client(StartProgram), local(false), display("1"), 
                protocol("Putty"), program("xterm"), host("lupus"), user("ago"),
                broadcast(false), indirect(false), xdmcp_host("lupus"),
                clipboard(true), extra_params() {};
#else
    CConfig() : window(MultiWindow), client(StartProgram), local(false), display("0"), 
                protocol("Putty"), program("xterm"), host(""), user(""), 
                broadcast(true), indirect(false), xdmcp_host(""),
                clipboard(true), extra_params() {};
#endif
    void Load(const char* filename);
    void Save(const char* filename);
};

#endif
