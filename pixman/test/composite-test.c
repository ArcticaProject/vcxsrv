#include <stdlib.h>
#include <stdio.h>
#include "pixman.h"

#include <gtk/gtk.h>

static GdkPixbuf *
pixbuf_from_argb32 (uint32_t *bits,
		    int width,
		    int height,
		    int stride)
{
    GdkPixbuf *pixbuf = gdk_pixbuf_new (GDK_COLORSPACE_RGB, TRUE,
					8, width, height);
    int p_stride = gdk_pixbuf_get_rowstride (pixbuf);
    guint32 *p_bits = (guint32 *)gdk_pixbuf_get_pixels (pixbuf);
    int w, h;

    for (h = 0; h < height; ++h)
    {
	for (w = 0; w < width; ++w)
	{
	    uint32_t argb = bits[h * stride + w];
	    guint32 rgba;

	    rgba = (argb << 8) | (argb >> 24);

	    p_bits[h * (p_stride / 4) + w] = rgba;
	}
    }

    return pixbuf;
}

static gboolean
on_expose (GtkWidget *widget, GdkEventExpose *expose, gpointer data)
{
    GdkPixbuf *pixbuf = data;
    
    gdk_draw_pixbuf (widget->window, NULL,
		     pixbuf, 0, 0, 0, 0,
		     gdk_pixbuf_get_width (pixbuf),
		     gdk_pixbuf_get_height (pixbuf),
		     GDK_RGB_DITHER_NONE,
		     0, 0);

    return TRUE;
}

static void
show_window (uint32_t *bits, int w, int h, int stride)
{
    GdkPixbuf *pixbuf;

    GtkWidget *window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

    pixbuf = pixbuf_from_argb32 (bits, w, h, stride);

    g_signal_connect (window, "expose_event", G_CALLBACK (on_expose), pixbuf);
    
    gtk_widget_show (window);

    gtk_main ();
}

#define WIDTH	100
#define HEIGHT	100

static uint32_t
reader (const void *src, int size)
{
    switch (size)
    {
    case 1:
	return *(uint8_t *)src;
    case 2:
	return *(uint16_t *)src;
    case 4:
	return *(uint32_t *)src;
    default:
	g_assert_not_reached();
    }
}

static void
writer (void *src, uint32_t value, int size)
{
    switch (size)
    {
    case 1:
	*(uint8_t *)src = value;
	break;

    case 2:
	*(uint16_t *)src = value;
	break;

    case 4:
	*(uint32_t *)src = value;
	break;
    }
}

int
main (int argc, char **argv)
{
    uint32_t *src = malloc (WIDTH * HEIGHT * 4);
    uint32_t *dest = malloc (WIDTH * HEIGHT * 4);
    pixman_image_t *src_img;
    pixman_image_t *dest_img;
    int i, j;
    gtk_init (&argc, &argv);

    for (i = 0; i < WIDTH * HEIGHT; ++i)
	src[i] = 0x7f7f0000; /* red */

    for (i = 0; i < WIDTH * HEIGHT; ++i)
	dest[i] = 0x7f0000ff; /* blue */
    
    src_img = pixman_image_create_bits (PIXMAN_a8r8g8b8,
					WIDTH, HEIGHT,
					src,
					WIDTH * 4);

    dest_img = pixman_image_create_bits (PIXMAN_a8r8g8b8,
					 WIDTH, HEIGHT,
					 dest,
					 WIDTH * 4);

    pixman_image_set_accessors (src_img, reader, writer);
    pixman_image_set_accessors (dest_img, reader, writer);
    
    pixman_image_composite (PIXMAN_OP_OVER, src_img, NULL, dest_img,
			    0, 0, 0, 0, 0, 0, WIDTH, HEIGHT);

#if 0
    for (i = 0; i < WIDTH; ++i)
    {
	for (j = 0; j < HEIGHT; ++j)
	    g_print ("%x, ", dest[i * WIDTH + j]);
	g_print ("\n");
    }
#endif
    
    show_window (dest, WIDTH, HEIGHT, WIDTH);
    
    pixman_image_unref (src_img);
    pixman_image_unref (dest_img);
    free (src);
    free (dest);


    
    return 0;
}
