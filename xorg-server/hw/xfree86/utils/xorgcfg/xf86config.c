/*
 * Copyright (c) 2000 by Conectiva S.A. (http://www.conectiva.com)
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
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * CONECTIVA LINUX BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * 
 * Except as contained in this notice, the name of Conectiva Linux shall
 * not be used in advertising or otherwise to promote the sale, use or other
 * dealings in this Software without prior written authorization from
 * Conectiva Linux.
 *
 * Author: Paulo César Pereira de Andrade <pcpa@conectiva.com.br>
 *
 */

#include "xf86config.h"

/*
 * Implementation
 */
int
xf86removeOption(XF86OptionPtr *options, char *name)
{
    XF86OptionPtr opt = *options, prev = opt;

    while (opt) {
	if (strcasecmp(opt->opt_name, name) == 0) {
	    XtFree(opt->opt_name);
	    XtFree(opt->opt_val);
	    XtFree(opt->opt_comment);
	    if (opt == prev)
		*options = (XF86OptionPtr)(opt->list.next);
	    else
		prev->list.next = opt->list.next;
	    XtFree((XtPointer)opt);

	    return (True);
	}

	prev = opt;
	opt = (XF86OptionPtr)(opt->list.next);
    }

    return (False);
}

int
xf86removeInput(XF86ConfigPtr config, XF86ConfInputPtr input)
{
    XF86ConfInputPtr prev, inp = config->conf_input_lst;
    XF86ConfLayoutPtr lay = config->conf_layout_lst;

    /* remove from main structure */
    prev = inp;
    while (inp != NULL) {
	if (inp == input) {
	    if (inp == prev)
		config->conf_input_lst = (XF86ConfInputPtr)(inp->list.next);
	    else
		prev->list.next = inp->list.next;
	    break;
	}
	prev = inp;
	inp = (XF86ConfInputPtr)(inp->list.next);
    }

    if (inp == NULL)
	return (False);

    /* remove references */
    while (lay != NULL) {
	xf86removeInputRef(lay, inp);
	lay = (XF86ConfLayoutPtr)(lay->list.next);
    }

    XtFree(inp->inp_identifier);
    XtFree(inp->inp_driver);
    XtFree(inp->inp_comment);
    xf86optionListFree(inp->inp_option_lst);
    XtFree((XtPointer)inp);

    return (True);
}

int
xf86removeInputRef(XF86ConfLayoutPtr layout, XF86ConfInputPtr input)
{
    XF86ConfInputrefPtr prev, iref = layout->lay_input_lst;

    prev = iref;
    while (iref != NULL) {
	if (iref->iref_inputdev == input) {
	    XtFree(iref->iref_inputdev_str);
	    xf86optionListFree(iref->iref_option_lst);
	    if (prev == iref)
		layout->lay_input_lst =
		    (XF86ConfInputrefPtr)(iref->list.next);
	    else
		prev->list.next = iref->list.next;
	    XtFree((XtPointer)iref);

	    return (True);
	}
	prev = iref;
	iref = (XF86ConfInputrefPtr)(iref->list.next);
    }

    return (False);
}

int
xf86removeDevice(XF86ConfigPtr config, XF86ConfDevicePtr device)
{
    XF86ConfDevicePtr prev, dev = config->conf_device_lst;
    XF86ConfScreenPtr psc, scr = config->conf_screen_lst;

    /* remove from main structure */
    prev = dev;
    while (dev != NULL) {
	if (dev == device) {
	    if (dev == prev)
		config->conf_device_lst = (XF86ConfDevicePtr)(dev->list.next);
	    else
		prev->list.next = dev->list.next;
	    break;
	}
	prev = dev;
	dev = (XF86ConfDevicePtr)(dev->list.next);
    }

    if (dev == NULL)
	return (False);

    /* remove references */
    psc = scr;
    while (scr != NULL) {
	if (scr->scrn_device == device) {
	    xf86removeScreen(config, scr);
	    if (scr == psc)
		scr = psc = config->conf_screen_lst;
	    else
		scr = psc;
	    continue;
	}
	psc = scr;
	scr = (XF86ConfScreenPtr)(scr->list.next);
    }

    XtFree(dev->dev_identifier);
    XtFree(dev->dev_vendor);
    XtFree(dev->dev_board);
    XtFree(dev->dev_chipset);
    XtFree(dev->dev_busid);
    XtFree(dev->dev_card);
    XtFree(dev->dev_driver);
    XtFree(dev->dev_ramdac);
    XtFree(dev->dev_clockchip);
    XtFree(dev->dev_comment);
    xf86optionListFree(dev->dev_option_lst);
    XtFree((XtPointer)dev);

    return (True);
}

int
xf86removeMonitor(XF86ConfigPtr config, XF86ConfMonitorPtr monitor)
{
    XF86ConfMonitorPtr prev, mon = config->conf_monitor_lst;
    XF86ConfScreenPtr psc, scr = config->conf_screen_lst;

    /* remove from main structure */
    prev = mon;
    while (mon != NULL) {
	if (mon == monitor) {
	    if (mon == prev)
		config->conf_monitor_lst = (XF86ConfMonitorPtr)(mon->list.next);
	    else
		prev->list.next = mon->list.next;
	    break;
	}
	prev = mon;
	mon = (XF86ConfMonitorPtr)(mon->list.next);
    }

    if (mon == NULL)
	return (False);

    /* remove references */
    psc = scr;
    while (scr != NULL) {
	if (scr->scrn_monitor == monitor) {
	    xf86removeScreen(config, scr);
	    if (scr == psc)
	    scr = psc = config->conf_screen_lst;
	    else
		scr = psc;
	    continue;
	}
	psc = scr;
	scr = (XF86ConfScreenPtr)(scr->list.next);
    }

    XtFree(mon->mon_identifier);
    XtFree(mon->mon_vendor);
    XtFree(mon->mon_modelname);
    XtFree(mon->mon_comment);
    xf86optionListFree(mon->mon_option_lst);
    XtFree((XtPointer)mon);

    return (True);
}

int
xf86removeScreen(XF86ConfigPtr config, XF86ConfScreenPtr screen)
{
    XF86ConfScreenPtr prev, scrn;
    XF86ConfLayoutPtr lay;

    if (config == NULL || screen == NULL)
	return (False);

    lay = config->conf_layout_lst;
    prev = scrn = config->conf_screen_lst;

    while (scrn != NULL) {
	if (scrn == screen) {
	    if (scrn == prev)
		config->conf_screen_lst = (XF86ConfScreenPtr)(scrn->list.next);
	    else
		prev->list.next = scrn->list.next;
	    break;
	}
	prev = scrn;
	scrn = (XF86ConfScreenPtr)(scrn->list.next);
    }

    if (scrn == NULL)
	return (False);

    while (lay != NULL) {
	XF86ConfAdjacencyPtr pad, ad = NULL, adj = lay->lay_adjacency_lst;

	pad = adj;
	while (adj) {
	    if (adj->adj_screen == screen)
		ad = adj;
	    else {
		if (adj->adj_top != NULL && adj->adj_top == screen) {
		    XtFree(adj->adj_top_str);
		    adj->adj_top_str = NULL;
		    adj->adj_top = NULL;
		}
		else if (adj->adj_bottom != NULL && adj->adj_bottom == screen) {
		    XtFree(adj->adj_bottom_str);
		    adj->adj_bottom_str = NULL;
		    adj->adj_bottom = NULL;
		}
		else if (adj->adj_left != NULL && adj->adj_left == screen) {
		    XtFree(adj->adj_left_str);
		    adj->adj_left_str = NULL;
		    adj->adj_left = NULL;
		}
		else if (adj->adj_right != NULL && adj->adj_right == screen) {
		    XtFree(adj->adj_right_str);
		    adj->adj_right_str = NULL;
		    adj->adj_right = NULL;
		}
		else if (adj->adj_refscreen != NULL &&
		    strcasecmp(scrn->scrn_identifier,
			       adj->adj_refscreen) == 0) {
		    XtFree(adj->adj_refscreen);
		    adj->adj_refscreen = NULL;
		    adj->adj_where = CONF_ADJ_ABSOLUTE;
		    adj->adj_x = adj->adj_y = 0;
		}
	    }
	    if (ad == NULL)
		pad = adj;
	    adj = (XF86ConfAdjacencyPtr)(adj->list.next);
	}

	if (ad != NULL) {
	    if (ad == lay->lay_adjacency_lst)
		lay->lay_adjacency_lst = (XF86ConfAdjacencyPtr)(ad->list.next);
	    else
		pad->list.next = (XF86ConfAdjacencyPtr)(ad->list.next);
	    XtFree(ad->adj_screen_str);
	    XtFree(ad->adj_top_str);
	    XtFree(ad->adj_bottom_str);
	    XtFree(ad->adj_left_str);
	    XtFree(ad->adj_right_str);
	    XtFree(ad->adj_refscreen);
	    XtFree((XtPointer)ad);
	}

	lay = (XF86ConfLayoutPtr)(lay->list.next);
    }

    xf86freeAdaptorLinkList(screen->scrn_adaptor_lst);
    xf86freeDisplayList(screen->scrn_display_lst);

    XtFree(screen->scrn_identifier);
    XtFree(screen->scrn_monitor_str);
    XtFree(screen->scrn_device_str);
    xf86optionListFree(screen->scrn_option_lst);
    XtFree((XtPointer)screen);

    return (True);
}

int
xf86removeAdjacency(XF86ConfLayoutPtr layout, XF86ConfAdjacencyPtr adjacency)
{
    XF86ConfAdjacencyPtr prev, adj = layout->lay_adjacency_lst;

    if (layout == NULL || adjacency == NULL)
	return (False);

    prev = adj;
    while (adj != NULL) {
	if (adj == adjacency)
	    break;
	prev = adj;
	adj = (XF86ConfAdjacencyPtr)(adj->list.next);
    }
    if (adj == NULL)
	return (False);

    XtFree(adj->adj_screen_str);
    XtFree(adj->adj_top_str);
    XtFree(adj->adj_bottom_str);
    XtFree(adj->adj_left_str);
    XtFree(adj->adj_right_str);
    XtFree(adj->adj_refscreen);
    if (prev == adj)
	layout->lay_adjacency_lst = (XF86ConfAdjacencyPtr)(adj->list.next);
    else
	prev->list.next = adj->list.next;
    XtFree((XtPointer)adj);

    return (True);
}

int
xf86removeInactive(XF86ConfLayoutPtr layout, XF86ConfInactivePtr inactive)
{
    XF86ConfInactivePtr prev, inac = layout->lay_inactive_lst;

    if (layout == NULL || inactive == NULL)
	return (False);

    prev = inac;
    while (inac != NULL) {
	if (inac == inactive)
	    break;
	prev = inac;
	inac = (XF86ConfInactivePtr)(inac->list.next);
    }
    if (inac == NULL)
	return (False);

    XtFree(inac->inactive_device_str);
    if (prev == inac)
	layout->lay_inactive_lst = (XF86ConfInactivePtr)(inac->list.next);
    else
	prev->list.next = inac->list.next;
    XtFree((XtPointer)inac);

    return (True);
}

int
xf86removeLayout(XF86ConfigPtr config, XF86ConfLayoutPtr layout)
{
    XF86ConfLayoutPtr prev, lay = config->conf_layout_lst;
    XF86ConfAdjacencyPtr adj, nadj;
    XF86ConfInactivePtr inac, ninac;
    XF86ConfInputrefPtr iref, niref;

    if (config == NULL || layout == NULL)
	return (False);

    prev = lay;
    while (lay != NULL) {
	if (lay == layout)
	    break;
	prev = lay;
	lay = (XF86ConfLayoutPtr)(lay->list.next);
    }

    if (lay == NULL)
	return (False);

    adj = lay->lay_adjacency_lst;
    while (adj != NULL) {
	nadj = (XF86ConfAdjacencyPtr)(adj->list.next);
	xf86removeAdjacency(lay, adj);
	adj = nadj;
    }

    inac = lay->lay_inactive_lst;
    while (inac != NULL) {
	ninac = (XF86ConfInactivePtr)(inac->list.next);
	xf86removeInactive(lay, inac);
	inac = ninac;
    }

    iref = lay->lay_input_lst;
    while (iref != NULL) {
	niref = (XF86ConfInputrefPtr)(iref->list.next);
	xf86removeInputRef(lay, iref->iref_inputdev);
	iref = niref;
    }

    xf86optionListFree(lay->lay_option_lst);

    if (prev == lay)
	config->conf_layout_lst = (XF86ConfLayoutPtr)(lay->list.next);
    else
	prev->list.next = lay->list.next;
    XtFree(lay->lay_identifier);
    XtFree((XtPointer)lay);

    return (True);
}

int
xf86removeModule(XF86ConfigPtr config, XF86LoadPtr load)
{
    XF86LoadPtr prev, mod;

    if (config == NULL || config->conf_modules == NULL ||
	config->conf_modules->mod_load_lst == NULL)
	return (False);

    for (mod = prev = config->conf_modules->mod_load_lst;
	 mod != NULL; prev = mod, mod = (XF86LoadPtr)(mod->list.next))
	if (load == mod) {
	    if (mod == prev)
		config->conf_modules->mod_load_lst =
		    (XF86LoadPtr)(mod->list.next);
	    else
		prev->list.next = mod->list.next;
	    XtFree(mod->load_name);
	    xf86optionListFree(mod->load_opt);

	    return (True);
	}

    return (False);
}

int
xf86removeModes(XF86ConfigPtr config, XF86ConfModesPtr modes)
{
    XF86ConfModesPtr mod;
    XF86ConfModeLinePtr ml, next;
    XF86ConfMonitorPtr mon;

    if (config == NULL || modes == NULL)
	return (False);

    /* check if modes is in config */
    if ((mod = config->conf_modes_lst) == modes)
	config->conf_modes_lst = (XF86ConfModesPtr)(mod->list.next);
    else
	for (; mod != NULL; mod = (XF86ConfModesPtr)(mod->list.next))
	    if ((XF86ConfModesPtr)(mod->list.next) == modes) {
		mod->list.next = modes->list.next;
		break;
	    }

    if (mod == NULL)
	return (False);

    /* remove references in monitor sections */
    mon = config->conf_monitor_lst;
    while (mon) {
	XF86ConfModesLinkPtr m, p;

	m = p = mon->mon_modes_sect_lst;
	while (m) {
	    if (m->ml_modes == modes) {
		XtFree(m->ml_modes_str);
		if (m == mon->mon_modes_sect_lst)
		    p = mon->mon_modes_sect_lst =
			(XF86ConfModesLinkPtr)(m->list.next);
		else {
		    p->list.next = m->list.next;
		    p = p->list.next;
		}
		XtFree((XtPointer)m);
		m = p;
		continue;
	    }
	    p = m;
	    m = (XF86ConfModesLinkPtr)(m->list.next);
	}
	mon = (XF86ConfMonitorPtr)(mon->list.next);
    }

    /* free modelines */
    ml = modes->mon_modeline_lst;
    while (ml) {
	next = (XF86ConfModeLinePtr)(ml->list.next);
	XtFree(ml->ml_identifier);
	XtFree((XtPointer)ml);
	ml = next;
    }

    /* free mode */
    XtFree(modes->modes_identifier);
    XtFree((XtPointer)modes);

    return (True);
}

int
xf86removeModesModeLine(XF86ConfModesPtr modes, XF86ConfModeLinePtr modeline)
{
    XF86ConfModeLinePtr ml, prev;

    if (modes == NULL || modeline == NULL || modes->mon_modeline_lst == NULL)
	return (False);

    for (ml = prev = modes->mon_modeline_lst; ml;
	 prev = ml, ml = (XF86ConfModeLinePtr)(ml->list.next))
	if (ml == modeline) {
	    if (prev == ml)
		modes->mon_modeline_lst = (XF86ConfModeLinePtr)(ml->list.next);
	    else
		prev->list.next = ml->list.next;
	    XtFree(modeline->ml_identifier);
	    XtFree((XtPointer)modeline);
	    return (True);
	}

    return (False);
}

int
xf86removeMonitorModeLine(XF86ConfMonitorPtr monitor, XF86ConfModeLinePtr modeline)
{
    XF86ConfModeLinePtr ml, prev;

    if (monitor == NULL || modeline == NULL || monitor->mon_modeline_lst == NULL)
	return (False);

    for (ml = prev = monitor->mon_modeline_lst; ml;
	 prev = ml, ml = (XF86ConfModeLinePtr)(ml->list.next))
	if (ml == modeline) {
	    if (prev == ml)
		monitor->mon_modeline_lst = (XF86ConfModeLinePtr)(ml->list.next);
	    else
		prev->list.next = ml->list.next;
	    XtFree(modeline->ml_identifier);
	    XtFree((XtPointer)modeline);
	    return (True);
	}

    return (False);
}

int
xf86removeMonitorModesLink(XF86ConfMonitorPtr monitor, XF86ConfModesLinkPtr link)
{
    XF86ConfModesLinkPtr lnk, prev;

    if (monitor == NULL || link == NULL || monitor->mon_modes_sect_lst == NULL)
	return (False);

    for (lnk = prev = monitor->mon_modes_sect_lst; lnk != NULL;
	 prev = lnk, lnk = (XF86ConfModesLinkPtr)(lnk->list.next))
	if (lnk == link) {
	    if (prev == lnk)
		monitor->mon_modes_sect_lst = (XF86ConfModesLinkPtr)(lnk->list.next);
	    else
		prev->list.next = lnk->list.next;
	    XtFree(link->ml_modes_str);
	    XtFree((XtPointer)link);

	    return (True);
	}

    return (False);
}

int
xf86removeScreenAdaptorLink(XF86ConfScreenPtr scrn, XF86ConfAdaptorLinkPtr link)
{
    XF86ConfAdaptorLinkPtr lnk, prev;

    if (scrn == NULL || link == NULL || scrn->scrn_adaptor_lst == NULL)
	return (False);

    for (lnk = prev = scrn->scrn_adaptor_lst; lnk != NULL;
	 prev = lnk, lnk = (XF86ConfAdaptorLinkPtr)(lnk->list.next))
	if (lnk == link) {
	    if (prev == lnk)
		scrn->scrn_adaptor_lst =
		    (XF86ConfAdaptorLinkPtr)(lnk->list.next);
	    else
		prev->list.next = lnk->list.next;
	    XtFree(link->al_adaptor_str);
	    XtFree((XtPointer)link);

	    return (True);
	}

    return (False);
}

int
xf86removeScreenDisplay(XF86ConfScreenPtr scrn, XF86ConfDisplayPtr display)
{
    XF86ConfDisplayPtr dsp, prev;

    if (scrn == NULL || display == NULL || scrn->scrn_display_lst == NULL)
	return (False);

    for (dsp = prev = scrn->scrn_display_lst; dsp != NULL;
	 prev = dsp, dsp = (XF86ConfDisplayPtr)(dsp->list.next))
	if (dsp == display) {
	    if (prev == dsp)
		scrn->scrn_display_lst =
		    (XF86ConfDisplayPtr)(dsp->list.next);
	    else
		prev->list.next = dsp->list.next;
	    xf86optionListFree(display->disp_option_lst);
	    XtFree((XtPointer)display->disp_visual);
	    xf86freeModeList(display->disp_mode_lst);
	    XtFree((XtPointer)display);

	    return (True);
	}

    return (False);
}

int
xf86removeVideoAdaptor(XF86ConfigPtr config, XF86ConfVideoAdaptorPtr video)
{
    XF86ConfVideoAdaptorPtr vid;
    XF86ConfScreenPtr scrn;
    XF86ConfVideoPortPtr vp, next;

    if (config == NULL || video == NULL)
	return (False);

    /* check if video is in config and update videoadaptor list */
    if ((vid = config->conf_videoadaptor_lst) == video)
	config->conf_videoadaptor_lst = (XF86ConfVideoAdaptorPtr)(vid->list.next);
    else
	for (; vid != NULL; vid = (XF86ConfVideoAdaptorPtr)(vid->list.next))
	    if ((XF86ConfVideoAdaptorPtr)(vid->list.next) == video) {
		vid->list.next = video->list.next;
		break;
	    }

    if (vid == NULL)
	return (False);

    /* remove references in screen sections */
    scrn = config->conf_screen_lst;
    while (scrn) {
	XF86ConfAdaptorLinkPtr v, p;

	v = p = scrn->scrn_adaptor_lst;
	while (v) {
	    if (v->al_adaptor == video) {
		XtFree(v->al_adaptor_str);
		if (v == scrn->scrn_adaptor_lst)
		    p = scrn->scrn_adaptor_lst =
			(XF86ConfAdaptorLinkPtr)(v->list.next);
		else {
		    p->list.next = v->list.next;
		    p = p->list.next;
		}
		XtFree((XtPointer)v);
		v = p;
		continue;
	    }
	    p = v;
	    v = (XF86ConfAdaptorLinkPtr)(v->list.next);
	}
	scrn = (XF86ConfScreenPtr)(scrn->list.next);
    }

    /* free videoports */
    vp = video->va_port_lst;
    while (vp) {
	next = (XF86ConfVideoPortPtr)(vp->list.next);
	XtFree(vp->vp_identifier);
	xf86optionListFree(vp->vp_option_lst);
	XtFree((XtPointer)vp);
	vp = next;
    }

    /* free videoadaptor */
    XtFree(video->va_identifier);
    XtFree(video->va_vendor);
    XtFree(video->va_board);
    XtFree(video->va_busid);
    XtFree(video->va_driver);
    XtFree(video->va_fwdref);
    xf86optionListFree(video->va_option_lst);
    XtFree((XtPointer)video);

    return (True);
}

int
xf86removeVideoPort(XF86ConfVideoAdaptorPtr va, XF86ConfVideoPortPtr vp)
{
    XF86ConfVideoPortPtr prev;

    if (va == NULL || vp == NULL)
	return (False);

    if ((prev = va->va_port_lst) == vp)
	va->va_port_lst = (XF86ConfVideoPortPtr)(va->va_port_lst->list.next);
    else {
	while (prev && (XF86ConfVideoPortPtr)(prev->list.next) != vp)
	    prev = (XF86ConfVideoPortPtr)(prev->list.next);
	if (prev == NULL)
	    return (False);
	prev->list.next = vp->list.next;
    }

    xf86optionListFree(vp->vp_option_lst);
    XtFree((XtPointer)vp);

    return (True);
}

int
xf86removeDisplayMode(XF86ConfDisplayPtr display, XF86ModePtr mode)
{
    XF86ModePtr prev;

    if (display == NULL || mode == NULL)
	return (False);

    if ((prev = display->disp_mode_lst) == mode)
	display->disp_mode_lst = (XF86ModePtr)(display->disp_mode_lst->list.next);
    else {
	while (prev && (XF86ModePtr)(prev->list.next) != mode)
	    prev = (XF86ModePtr)(prev->list.next);
	if (prev == NULL)
	    return (False);
	prev->list.next = mode->list.next;
    }

    XtFree((XtPointer)mode);

    return (True);
}

int
xf86removeVendor(XF86ConfigPtr config, XF86ConfVendorPtr vendor)
{
    XF86ConfVendorPtr prev;

    if (config == NULL || vendor == NULL)
	return (False);

    if ((prev = config->conf_vendor_lst) == vendor)
	config->conf_vendor_lst = (XF86ConfVendorPtr)(config->conf_vendor_lst->list.next);
    else {
	while (prev && (XF86ConfVendorPtr)(prev->list.next) != vendor)
	    prev = (XF86ConfVendorPtr)(prev->list.next);
	if (prev == NULL)
	    return (False);
	prev->list.next = vendor->list.next;
    }

    xf86optionListFree(vendor->vnd_option_lst);
    xf86freeVendorSubList(vendor->vnd_sub_lst);
    XtFree(vendor->vnd_identifier);
    XtFree((XtPointer)vendor);

    return (True);
}

int
xf86removeVendorSub(XF86ConfVendorPtr vendor, XF86ConfVendSubPtr sub)
{
    XF86ConfVendSubPtr prev;

    if (vendor == NULL || sub == NULL)
	return (False);

    if ((prev = vendor->vnd_sub_lst) == sub)
	vendor->vnd_sub_lst = (XF86ConfVendSubPtr)(vendor->vnd_sub_lst->list.next);
    else {
	while (prev && (XF86ConfVendSubPtr)(prev->list.next) != sub)
	    prev = (XF86ConfVendSubPtr)(prev->list.next);
	if (prev == NULL)
	    return (False);
	prev->list.next = sub->list.next;
    }

    xf86optionListFree(sub->vs_option_lst);
    XtFree(sub->vs_name);
    XtFree(sub->vs_identifier);
    XtFree((XtPointer)sub);

    return (True);
}

int
xf86removeBuffers(XF86ConfDRIPtr dri, XF86ConfBuffersPtr buf)
{
    XF86ConfBuffersPtr prev;

    if (dri == NULL || buf == NULL)
	return (False);

    if ((prev = dri->dri_buffers_lst) == buf)
	dri->dri_buffers_lst = (XF86ConfBuffersPtr)(dri->dri_buffers_lst->list.next);
    else {
	while (prev && (XF86ConfBuffersPtr)(prev->list.next) != buf)
	    prev = (XF86ConfBuffersPtr)(prev->list.next);
	if (prev == NULL)
	    return (False);
	prev->list.next = buf->list.next;
    }

    XtFree(buf->buf_flags);
    XtFree((XtPointer)buf);

    return (True);
}

int
xf86renameInput(XF86ConfigPtr config, XF86ConfInputPtr input, char *name)
{
    XF86ConfLayoutPtr lay = config->conf_layout_lst;

    if (config == NULL || input == NULL || name == NULL || *name == '\0')
	return (False);

    while (lay != NULL) {
	XF86ConfInputrefPtr iref = lay->lay_input_lst;

	while (iref != NULL) {
	    if (strcasecmp(input->inp_identifier, iref->iref_inputdev_str) == 0) {
		XtFree(iref->iref_inputdev_str);
		iref->iref_inputdev_str = XtNewString(name);
	    }
	    iref = (XF86ConfInputrefPtr)(iref->list.next);
	}
	lay = (XF86ConfLayoutPtr)(lay->list.next);
    }

    XtFree(input->inp_identifier);
    input->inp_identifier = XtNewString(name);

    return (True);
}

int
xf86renameDevice(XF86ConfigPtr config, XF86ConfDevicePtr dev, char *name)
{
    XF86ConfScreenPtr scr = config->conf_screen_lst;

    if (config == NULL || dev == NULL || name == NULL || *name == '\0')
	return (False);

    while (scr != NULL) {
	if (scr->scrn_device == dev) {
	    XtFree(scr->scrn_device_str);
	    scr->scrn_device_str = XtNewString(name);
	}

	scr = (XF86ConfScreenPtr)(scr->list.next);
    }

    XtFree(dev->dev_identifier);
    dev->dev_identifier = XtNewString(name);

    return (True);
}

int
xf86renameMonitor(XF86ConfigPtr config, XF86ConfMonitorPtr mon, char *name)
{
    XF86ConfScreenPtr scr = config->conf_screen_lst;

    if (config == NULL || mon == NULL || name == NULL || *name == '\0')
	return (False);

    while (scr != NULL) {
	if (scr->scrn_monitor == mon) {
	    XtFree(scr->scrn_monitor_str);
	    scr->scrn_monitor_str = XtNewString(name);
	}

	scr = (XF86ConfScreenPtr)(scr->list.next);
    }

    XtFree(mon->mon_identifier);
    mon->mon_identifier = XtNewString(name);

    return (True);
}

int
xf86renameLayout(XF86ConfigPtr config, XF86ConfLayoutPtr layout, char *name)
{
    if (config == NULL || layout == NULL || name == NULL || *name == '\0')
	return (False);

    XtFree(layout->lay_identifier);
    layout->lay_identifier = XtNewString(name);

    return (True);
}

int
xf86renameScreen(XF86ConfigPtr config, XF86ConfScreenPtr scrn, char *name)
{
    XF86ConfLayoutPtr lay = config->conf_layout_lst;

    if (config == NULL || scrn == NULL || name == NULL || *name == '\0')
	return (False);

    while (lay != NULL) {
	XF86ConfAdjacencyPtr adj = lay->lay_adjacency_lst;

	while (adj != NULL) {
	    if (adj->adj_screen == scrn) {
		XtFree(adj->adj_screen_str);
		adj->adj_screen_str = XtNewString(name);
	    }
	    else if (adj->adj_top == scrn) {
		XtFree(adj->adj_top_str);
		adj->adj_top_str = XtNewString(name);
	    }
	    else if (adj->adj_bottom == scrn) {
		XtFree(adj->adj_bottom_str);
		adj->adj_bottom_str = XtNewString(name);
	    }
	    else if (adj->adj_left == scrn) {
		XtFree(adj->adj_left_str);
		adj->adj_left_str = XtNewString(name);
	    }
	    else if (adj->adj_right == scrn) {
		XtFree(adj->adj_right_str);
		adj->adj_right_str = XtNewString(name);
	    }
	    else if (adj->adj_refscreen != NULL &&
		     strcasecmp(adj->adj_refscreen, name) == 0) {
		XtFree(adj->adj_refscreen);
		adj->adj_refscreen = XtNewString(name);
	    }

	    adj = (XF86ConfAdjacencyPtr)(adj->list.next);
	}
	lay = (XF86ConfLayoutPtr)(lay->list.next);
    }

    XtFree(scrn->scrn_identifier);
    scrn->scrn_identifier = XtNewString(name);

    return (True);
}
