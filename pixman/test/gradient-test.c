#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include "pixman.h"

GdkPixbuf *
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
	    guint32 abgr;
	    
	    abgr = (argb & 0xff000000) |
		(argb & 0xff) << 16 |
		(argb & 0x00ff00) |
		(argb & 0xff0000) >> 16;
	    
	    p_bits[h * (p_stride / 4) + w] = abgr;
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
    g_signal_connect (window, "delete_event", G_CALLBACK (gtk_main_quit), NULL);
    
    gtk_widget_show (window);
    
    gtk_main ();
}

int
main (int argc, char **argv)
{
#define WIDTH 200
#define HEIGHT 200
    
    uint32_t *dest = malloc (WIDTH * HEIGHT * 4);
    pixman_image_t *src_img;
    pixman_image_t *dest_img;
    int i;
    pixman_gradient_stop_t stops[2] =
	{
	    { pixman_int_to_fixed (0), { 0xffff, 0x0000, 0x0000, 0xffff } },
	    { pixman_int_to_fixed (1), { 0xffff, 0xffff, 0x0000, 0xffff } }
	};
    pixman_point_fixed_t p1 = { 0, 0 };
    pixman_point_fixed_t p2 = { pixman_int_to_fixed (WIDTH),
				pixman_int_to_fixed (HEIGHT) };
    pixman_transform_t trans = {
	{ { pixman_double_to_fixed (2), pixman_double_to_fixed (0.5), pixman_double_to_fixed (-100), },
	  { pixman_double_to_fixed (0), pixman_double_to_fixed (2), pixman_double_to_fixed (0), },
	  { pixman_double_to_fixed (0), pixman_double_to_fixed (0.004990), pixman_double_to_fixed (1.0) } 
	}
    };

    pixman_transform_t id = {
	{ { pixman_fixed_1, 0, 0 },
	  { 0, pixman_fixed_1, 0 },
	  { 0, 0, pixman_fixed_1 } }
    };

    pixman_point_fixed_t c_inner;
    pixman_point_fixed_t c_outer;
    pixman_fixed_t r_inner;
    pixman_fixed_t r_outer;
    
    gtk_init (&argc, &argv);
    
    for (i = 0; i < WIDTH * HEIGHT; ++i)
	dest[i] = 0x3f0000ff; /* pale blue */
    
    dest_img = pixman_image_create_bits (PIXMAN_a8r8g8b8,
					 WIDTH, HEIGHT, 
					 dest,
					 WIDTH * 4);

    c_inner.x = pixman_double_to_fixed (50.0);
    c_inner.y = pixman_double_to_fixed (50.0);
    c_outer.x = pixman_double_to_fixed (50.0);
    c_outer.y = pixman_double_to_fixed (50.0);
    r_inner = 0;
    r_outer = pixman_double_to_fixed (50.0);
    
    src_img = pixman_image_create_radial_gradient (&c_inner, &c_outer,
						   r_inner, r_outer,
						   stops, 2);
    
#if 0
    src_img = pixman_image_create_linear_gradient  (&p1, &p2,
						    stops, 2);
    
#endif
    pixman_image_set_transform (src_img, &trans);
    
    pixman_image_composite (PIXMAN_OP_OVER, src_img, NULL, dest_img,
			    0, 0, 0, 0, 0, 0, WIDTH, HEIGHT);
    
    printf ("0, 0: %x\n", dest[0]);
    printf ("10, 10: %x\n", dest[10 * 10 + 10]);
    printf ("w, h: %x\n", dest[(HEIGHT - 1) * 100 + (WIDTH - 1)]);
    
    show_window (dest, WIDTH, HEIGHT, WIDTH);
    
    pixman_image_unref (src_img);
    pixman_image_unref (dest_img);
    free (dest);
    
    return 0;
}
