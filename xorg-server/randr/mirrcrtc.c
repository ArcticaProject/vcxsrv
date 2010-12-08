/*
 * Copyright © 2010 Keith Packard
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting documentation, and
 * that the name of the copyright holders not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  The copyright holders make no representations
 * about the suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THIS SOFTWARE.
 */

#include "randrstr.h"

Bool
miRRSetScreenConfig(ScreenPtr screen,
		    RRScreenConfigPtr screen_config)
{
    RRScreenConfigRec	old_screen_config;

    RRScreenCurrentConfig(screen, &old_screen_config);

    /* Check and see if nothing has changed */
    if (old_screen_config.screen_width == screen_config->screen_width &&
	old_screen_config.screen_height == screen_config->screen_height &&
	old_screen_config.screen_pixmap_width == screen_config->screen_pixmap_width &&
	old_screen_config.screen_pixmap_height == screen_config->screen_pixmap_height &&
	old_screen_config.mm_width == screen_config->mm_width &&
	old_screen_config.mm_height == screen_config->mm_height)
	return TRUE;

    return RRScreenSizeSet(screen,
			   screen_config->screen_width,
			   screen_config->screen_height,
			   screen_config->screen_pixmap_width,
			   screen_config->screen_pixmap_height,
			   screen_config->mm_width,
			   screen_config->mm_height);
}

Bool
miRRSetCrtcConfig(RRCrtcConfigPtr crtc_config)
{
    int	x = crtc_config->x, y = crtc_config->y;

    if (crtc_config->pixmap) {
	x = crtc_config->pixmap_x;
	y = crtc_config->pixmap_y;
    }
    if (!RRCrtcSet(crtc_config->crtc,
		   crtc_config->mode,
		   x,
		   y,
		   crtc_config->rotation,
		   crtc_config->numOutputs,
		   crtc_config->outputs,
		   crtc_config->pixmap))
	return FALSE;
    RRCrtcSpriteTransformSet(crtc_config->crtc,
			     &crtc_config->sprite_position_transform,
			     &crtc_config->sprite_image_transform,
			     &crtc_config->sprite_position_f_transform,
			     &crtc_config->sprite_image_f_transform);
    return TRUE;
}

Bool
miRRDisableCrtc(RRCrtcPtr crtc)
{
    RRCrtcConfigRec	off_config;

    memset(&off_config, '\0', sizeof (RRCrtcConfigRec));
    off_config.crtc = crtc;
    return miRRSetCrtcConfig(&off_config);
}

/*
 * If the current crtc configuration doesn't fit
 * with the new screen config, disable it
 */
Bool
miRRCheckDisableCrtc(RRScreenConfigPtr new_screen_config,
		     RRCrtcConfigPtr old_crtc_config)
{
    RRCrtcPtr crtc = old_crtc_config->crtc;

    /* If it's already disabled, we're done */
    if (!old_crtc_config->mode)
	return TRUE;

    /* If the crtc isn't scanning from the screen pixmap,
     * we're done
     */
    if (old_crtc_config->pixmap)
	return TRUE;

    /* If the new screen configuration covers the existing CRTC space,
     * we're done
     */
    if (RRScreenCoversCrtc(new_screen_config, old_crtc_config,
			   &crtc->client_current_transform, NULL))
	return TRUE;

    /* Disable the crtc and let it get re-enabled */
    return miRRDisableCrtc(crtc);
}

Bool
miRRSetCrtcConfigs(ScreenPtr screen,
		   RRScreenConfigPtr screen_config,
		   RRCrtcConfigPtr crtc_configs,
		   int num_configs)
{
    RRScreenConfigRec	old_screen_config;
    RRCrtcConfigPtr	old_crtc_configs;
    int			i;

    /*
     * Save existing state
     */

    RRScreenCurrentConfig(screen, &old_screen_config);
    old_crtc_configs = calloc(num_configs, sizeof (RRCrtcConfigRec));
    if (!old_crtc_configs)
	return FALSE;

    for (i = 0; i < num_configs; i++)
	if (!RRCrtcCurrentConfig(crtc_configs[i].crtc, &old_crtc_configs[i]))
	    goto fail_save;
    /*
     * Set the new configuration. If anything goes wrong,
     * bail and restore the old configuration
     */
    for (i = 0; i < num_configs; i++)
	if (!miRRCheckDisableCrtc(screen_config, &old_crtc_configs[i]))
	    goto fail_disable;

    if (!miRRSetScreenConfig(screen, screen_config))
	goto fail_set_screen;

    for (i = 0; i < num_configs; i++)
	if (!miRRSetCrtcConfig(&crtc_configs[i]))
	    goto fail_set_crtc;

    RRFreeCrtcConfigs(old_crtc_configs, num_configs);
    return TRUE;

fail_set_crtc:
    /*
     * Restore the previous configuration. Ignore any errors
     * as we just need to hope that the driver can manage to
     * get back to the previous state without trouble.
     */
    for (i = 0; i < num_configs; i++)
	(void) miRRDisableCrtc(old_crtc_configs[i].crtc);
    (void) miRRSetScreenConfig(screen, &old_screen_config);
fail_set_screen:
fail_disable:
    for (i = 0; i < num_configs; i++)
	(void) miRRSetCrtcConfig(&old_crtc_configs[i]);
fail_save:
    RRFreeCrtcConfigs(old_crtc_configs, num_configs);
    return FALSE;
}
