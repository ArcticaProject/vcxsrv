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
#include "config.h"
#include "window/util.h"
#include <msxml2.h>
#include <stdexcept>

const CLSID CLSID_DOMDocument40 = {0x88d969c0,0xf192,0x11d4,0xa6,0x5f,0x00,0x40,0x96,0x32,0x51,0xe5};
const CLSID CLSID_DOMDocument30 = {0xf5078f32,0xc551,0x11d3,0x89,0xb9,0x00,0x00,0xf8,0x1f,0xe2,0x21};
const IID IID_IXMLDOMDocument2 = {0x2933BF95,0x7B36,0x11d2,0xB2,0x0E,0x00,0xC0,0x4F,0x98,0x3E,0x60};

#define HRCALL(x, msg) if (FAILED(x)) { throw std::runtime_error("OLE Error:" msg " failed"); };

char *wcconvert(const wchar_t *wstr)
{
    int chars = WideCharToMultiByte(CP_ACP, 0, wstr, -1, NULL, 0, NULL, NULL);
    if (chars == 0)
	throw win32_error("WideCharToMultiByte");
    char *mbstr = new char[chars];
    chars = WideCharToMultiByte(CP_ACP, 0, wstr, -1, mbstr, chars, NULL, NULL);
    if (chars == 0)
	throw win32_error("WideCharToMultiByte");
    return mbstr;
}

wchar_t *mbconvert(const char *mbstr)
{
    int chars = MultiByteToWideChar(CP_ACP, 0, mbstr, -1, NULL, 0);
    if (chars == 0)
	throw win32_error("MultiByteToWideChar");
    wchar_t *wstr = new wchar_t[chars];
    chars = MultiByteToWideChar(CP_ACP, 0, mbstr, -1, wstr, chars);
    if (chars == 0)
	throw win32_error("MultiByteToWideChar");
    return wstr;
}

VARIANT VariantString(const char *filename)
{

    wchar_t *str = mbconvert(filename);

    VARIANT var;
    VariantInit(&var);
    V_BSTR(&var) = SysAllocString(str);
    V_VT(&var) = VT_BSTR;

    delete [] str;
    return var;
}

VARIANT VariantString(const wchar_t *str)
{
   VARIANT var;
   VariantInit(&var);
   V_BSTR(&var) = SysAllocString(str);
   V_VT(&var) = VT_BSTR;
   return var;
}

IXMLDOMDocument2 *CreateDocument()
{
    IXMLDOMDocument2 *doc = NULL;
    
    CoInitialize(NULL);

    HRCALL(CoCreateInstance(CLSID_DOMDocument40, NULL, CLSCTX_INPROC_SERVER,
                      IID_IXMLDOMDocument2, (void**)&doc), "CoCreateInstance");

    try {
      	HRCALL(doc->put_async(VARIANT_FALSE), "put_async");
	HRCALL(doc->put_validateOnParse(VARIANT_FALSE), "put_validateOnParse");
	HRCALL(doc->put_resolveExternals(VARIANT_FALSE), "put_resolveExternals");

	IXMLDOMProcessingInstruction *pi = NULL;
	IXMLDOMElement *root = NULL;
     	BSTR xml = SysAllocString(L"xml");
	BSTR ver = SysAllocString(L"version='1.0'");
	HRCALL(doc->createProcessingInstruction(xml,ver, &pi), 
		"createProcessingInstruction");
	HRCALL(doc->appendChild(pi, NULL),
		"appendChild");
	pi->Release();
	SysFreeString(xml);
	SysFreeString(ver);

	BSTR elemname = SysAllocString(L"XLaunch");
	HRCALL(doc->createElement(elemname, &root), "createElement");
	HRCALL(doc->appendChild(root, NULL), "appendChild");
	SysFreeString(elemname);
    } catch (...)
    {
	doc->Release();
	throw;
    }
    return doc;
}

void setAttribute(IXMLDOMElement *elem, const wchar_t *name, const wchar_t *value)
{
    BSTR str = SysAllocString(name);
    VARIANT var = VariantString(value);
    HRCALL(elem->setAttribute(str, var), "setAttribute");
    VariantClear(&var);
    SysFreeString(str);
}

void setAttribute(IXMLDOMElement *elem, const wchar_t *name, const char *value)
{
    wchar_t *wstr = mbconvert(value);
    setAttribute(elem, name, wstr);
    delete [] wstr;
    return;
}

void CConfig::Save(const char *filename)
{
    IXMLDOMDocument2 *doc = CreateDocument();
    IXMLDOMElement *root = NULL;

    HRCALL(doc->get_documentElement(&root), "get_documentElement");

    switch (window)
    {
	case MultiWindow:
	    setAttribute(root, L"WindowMode", L"MultiWindow");
	    break;
	case Fullscreen:
	    setAttribute(root, L"WindowMode", L"Fullscreen");
	    break;
	default:
	case Windowed:
	    setAttribute(root, L"WindowMode", L"Windowed");
	    break;
	case Nodecoration:
	    setAttribute(root, L"WindowMode", L"Nodecoration");
	    break;
    }
    switch (client)
    {
	default:
	case NoClient:
	    setAttribute(root, L"ClientMode", L"NoClient");
	    break;
	case StartProgram:
	    setAttribute(root, L"ClientMode", L"StartProgram");
	    break;
	case XDMCP:
	    setAttribute(root, L"ClientMode", L"XDMCP");
	    break;
    }
    setAttribute(root, L"LocalClient", local?L"True":L"False");
    setAttribute(root, L"Display", display.c_str());
    setAttribute(root, L"Program", program.c_str());
    setAttribute(root, L"RemoteProtocol", protocol.c_str());
    setAttribute(root, L"RemoteHost", host.c_str());
    setAttribute(root, L"RemoteUser", user.c_str());
    setAttribute(root, L"XDMCPHost", xdmcp_host.c_str());
    setAttribute(root, L"XDMCPBroadcast", broadcast?L"True":L"False");
    setAttribute(root, L"XDMCPIndirect", indirect?L"True":L"False");
    setAttribute(root, L"Clipboard", clipboard?L"True":L"False");
    setAttribute(root, L"ExtraParams", extra_params.c_str());

    VARIANT var = VariantString(filename);
    HRCALL(doc->save(var), "save");
    VariantClear(&var);


    root->Release();
    doc->Release();
}

BOOL getAttribute(IXMLDOMElement *elem, const wchar_t *name, std::string &ret)
{
    VARIANT var;
    HRCALL(elem->getAttribute((OLECHAR*)name, &var), "getAttribute"); 
    if (V_VT(&var) != VT_NULL && V_VT(&var) == VT_BSTR)
    {
	char *str = wcconvert(V_BSTR(&var));
	ret = str;
	delete [] str;
	return true;
    }
    return false;
}

BOOL getAttributeBool(IXMLDOMElement *elem, const wchar_t *name, bool &ret)
{
    std::string str;
    if (getAttribute(elem, name, str))
    {
	if (str == "True")
	    ret = true;
	else
	    ret = false;
	return true;
    }
    return false;
}


void CConfig::Load(const char *filename)
{
    IXMLDOMDocument2 *doc = CreateDocument();
    IXMLDOMElement *root = NULL;

    VARIANT var = VariantString(filename);
    VARIANT_BOOL status;
    HRCALL(doc->load(var, &status), "load");
    VariantClear(&var);

    if (status == VARIANT_FALSE)
    {
	doc->Release();
	return;
    }

    HRCALL(doc->get_documentElement(&root), "get_documentElement");

    std::string windowMode;
    std::string clientMode;

    if (getAttribute(root, L"WindowMode", windowMode))
    {
	if (windowMode == "MultiWindow")
	    window = MultiWindow;
	else if (windowMode == "Fullscreen")
	    window = Fullscreen;
	else if (windowMode == "Windowed")
	    window = Windowed;
	else if (windowMode == "Nodecoration")
	    window = Nodecoration;
    }
    if (getAttribute(root, L"ClientMode", clientMode))
    {
	if (clientMode == "NoClient")
	    client = NoClient;
	else if (clientMode == "StartProgram")
	    client = StartProgram;
	else if (clientMode == "XDMCP")
	    client = XDMCP;
    }
    
    getAttributeBool(root, L"LocalClient", local);
    getAttribute(root, L"Display", display);
    getAttribute(root, L"Program", program);
    getAttribute(root, L"RemoteProtocol", protocol);
    getAttribute(root, L"RemoteHost", host);
    getAttribute(root, L"RemoteUser", user);
    getAttribute(root, L"XDMCPHost", xdmcp_host);
    getAttributeBool(root, L"XDMCPBroadcast", broadcast);
    getAttributeBool(root, L"XDMCPIndirect", indirect);
    getAttributeBool(root, L"Clipboard", clipboard);
    getAttribute(root, L"ExtraParams", extra_params);
    

    doc->Release();
}

