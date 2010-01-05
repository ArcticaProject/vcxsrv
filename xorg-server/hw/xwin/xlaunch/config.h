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
 *
 * Authors:	Alexander Gottwald, Colin Harrison
 */
#ifndef __CONFIG_H__
#define __CONFIG_H__
#define UNICODE
#define _UNICODE

#define PROG_NUMBER 10
#define HOST_NUMBER 10
#define MAX_MESSAGE 256
#define MAX_CAPTION 128

#include <windows.h>
#include <string>
#include <vector>
struct CConfig
{
    enum {MultiWindow, Fullscreen, Windowed, Nodecoration} window;
    enum {NoClient, StartProgram, XDMCP} client;
    enum {NoXClient, Local, PuTTY, OpenSSH} clientstart;
    bool local;
    std::string display;
    std::string protocol;
    std::string protocol_path;
    std::string program;
    std::vector<std::string> progs;
    bool compress;
    std::string host;
    std::string user;
    std::string password;
    bool password_save;
    bool password_start;
    bool broadcast;
    bool indirect;
    std::string xdmcp_host;
    std::vector<std::string> xhosts;
    bool clipboard;
    bool no_access_control;
    std::string font_server;
    std::string extra_params;
    std::string extra_ssh;
#ifdef _DEBUG
    CConfig() : window(MultiWindow), client(NoClient), clientstart(NoXClient), display("1"), 
#else
    CConfig() : window(MultiWindow), client(NoClient), clientstart(NoXClient), display("0"), 
#endif
                local(false), protocol(""),
                protocol_path(""), program("xeyes"), progs(PROG_NUMBER), compress(false), host(""), user("ago"),
                password(""), password_save(false), password_start(false), broadcast(false),
                indirect(false), xdmcp_host(""), xhosts(HOST_NUMBER), clipboard(true), no_access_control(false),
                font_server(), extra_params(), extra_ssh() {};
    void Load(const char * filename);
    void Save(const char * filename);
};

#endif
