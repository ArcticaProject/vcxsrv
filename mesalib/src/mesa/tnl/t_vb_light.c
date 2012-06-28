/*
 * Mesa 3-D graphics library
 * Version:  6.5
 *
 * Copyright (C) 1999-2006  Brian Paul   All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * BRIAN PAUL BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
 * AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */



#include "main/glheader.h"
#include "main/colormac.h"
#include "main/light.h"
#include "main/macros.h"
#include "main/imports.h"
#include "main/simple_list.h"
#include "main/mtypes.h"

#include "math/m_translate.h"

#include "t_context.h"
#include "t_pipeline.h"
#include "tnl.h"

#define LIGHT_TWOSIDE       0x1
#define LIGHT_MATERIAL      0x2
#define MAX_LIGHT_FUNC      0x4

typedef void (*light_func)( struct gl_context *ctx,
			    struct vertex_buffer *VB,
			    struct tnl_pipeline_stage *stage,
			    GLvector4f *input );

/**
 * Information for updating current material attributes from vertex color,
 * for GL_COLOR_MATERIAL.
 */
struct material_cursor {
   const GLfloat *ptr;    /* points to src vertex color (in VB array) */
   GLuint stride;         /* stride to next vertex color (bytes) */
   GLfloat *current;      /* points to material attribute to update */
   GLuint size;           /* vertex/color size: 1, 2, 3 or 4 */
};

/**
 * Data private to this pipeline stage.
 */
struct light_stage_data {
   GLvector4f Input;
   GLvector4f LitColor[2];
   GLvector4f LitSecondary[2];
   light_func *light_func_tab;

   struct material_cursor mat[MAT_ATTRIB_MAX];
   GLuint mat_count;
   GLuint mat_bitmask;
};


#define LIGHT_STAGE_DATA(stage) ((struct light_stage_data *)(stage->privatePtr))



/**********************************************************************/
/*****                  Lighting computation                      *****/
/**********************************************************************/


/*
 * Notes:
 *   When two-sided lighting is enabled we compute the color (or index)
 *   for both the front and back side of the primitive.  Then, when the
 *   orientation of the facet is later learned, we can determine which
 *   color (or index) to use for rendering.
 *
 *   KW: We now know orientation in advance and only shade for
 *       the side or sides which are actually required.
 *
 * Variables:
 *   n = normal vector
 *   V = vertex position
 *   P = light source position
 *   Pe = (0,0,0,1)
 *
 * Precomputed:
 *   IF P[3]==0 THEN
 *       // light at infinity
 *       IF local_viewer THEN
 *           _VP_inf_norm = unit vector from V to P      // Precompute
 *       ELSE
 *           // eye at infinity
 *           _h_inf_norm = Normalize( VP + <0,0,1> )     // Precompute
 *       ENDIF
 *   ENDIF
 *
 * Functions:
 *   Normalize( v ) = normalized vector v
 *   Magnitude( v ) = length of vector v
 */



static void
validate_shine_table( struct gl_context *ctx, GLuint side, GLfloat shininess )
{
   TNLcontext *tnl = TNL_CONTEXT(ctx);
   struct tnl_shine_tab *list = tnl->_ShineTabList;
   struct tnl_shine_tab *s;

   ASSERT(side < 2);

   foreach(s, list)
      if ( s->shininess == shininess )
	 break;

   if (s == list) {
      GLint j;
      GLfloat *m;

      foreach(s, list)
	 if (s->refcount == 0)
	    break;

      m = s->tab;
      m[0] = 0.0;
      if (shininess == 0.0) {
	 for (j = 1 ; j <= SHINE_TABLE_SIZE ; j++)
	    m[j] = 1.0;
      }
      else {
	 for (j = 1 ; j < SHINE_TABLE_SIZE ; j++) {
            GLdouble t, x = j / (GLfloat) (SHINE_TABLE_SIZE - 1);
            if (x < 0.005) /* underflow check */
               x = 0.005;
            t = pow(x, shininess);
	    if (t > 1e-20)
	       m[j] = (GLfloat) t;
	    else
	       m[j] = 0.0;
	 }
	 m[SHINE_TABLE_SIZE] = 1.0;
      }

      s->shininess = shininess;
   }

   if (tnl->_ShineTable[side])
      tnl->_ShineTable[side]->refcount--;

   tnl->_ShineTable[side] = s;
   move_to_tail( list, s );
   s->refcount++;
}


void
_tnl_validate_shine_tables( struct gl_context *ctx )
{
   TNLcontext *tnl = TNL_CONTEXT(ctx);
   GLfloat shininess;
   
   shininess = ctx->Light.Material.Attrib[MAT_ATTRIB_FRONT_SHININESS][0];
   if (!tnl->_ShineTable[0] || tnl->_ShineTable[0]->shininess != shininess)
      validate_shine_table( ctx, 0, shininess );

   shininess = ctx->Light.Material.Attrib[MAT_ATTRIB_BACK_SHININESS][0];
   if (!tnl->_ShineTable[1] || tnl->_ShineTable[1]->shininess != shininess)
      validate_shine_table( ctx, 1, shininess );
}


/**
 * In the case of colormaterial, the effected material attributes
 * should already have been bound to point to the incoming color data,
 * prior to running the pipeline.
 * This function copies the vertex's color to the material attributes
 * which are tracking glColor.
 * It's called per-vertex in the lighting loop.
 */
static void
update_materials(struct gl_context *ctx, struct light_stage_data *store)
{
   GLuint i;

   for (i = 0 ; i < store->mat_count ; i++) {
      /* update the material */
      COPY_CLEAN_4V(store->mat[i].current, store->mat[i].size, store->mat[i].ptr);
      /* increment src vertex color pointer */
      STRIDE_F(store->mat[i].ptr, store->mat[i].stride);
   }
      
   /* recompute derived light/material values */
   _mesa_update_material( ctx, store->mat_bitmask );
   /* XXX we should only call this if we're tracking/changing the specular
    * exponent.
    */
   _tnl_validate_shine_tables( ctx );
}


/**
 * Prepare things prior to running the lighting stage.
 * Return number of material attributes which will track vertex color.
 */
static GLuint
prepare_materials(struct gl_context *ctx,
                  struct vertex_buffer *VB, struct light_stage_data *store)
{
   GLuint i;
   
   store->mat_count = 0;
   store->mat_bitmask = 0;

   /* Examine the _ColorMaterialBitmask to determine which materials
    * track vertex color.  Override the material attribute's pointer
    * with the color pointer for each one.
    */
   if (ctx->Light.ColorMaterialEnabled) {
      const GLuint bitmask = ctx->Light._ColorMaterialBitmask;
      for (i = 0 ; i < MAT_ATTRIB_MAX ; i++)
	 if (bitmask & (1<<i))
	    VB->AttribPtr[_TNL_ATTRIB_MAT_FRONT_AMBIENT + i] = VB->AttribPtr[_TNL_ATTRIB_COLOR0];
   }

   /* Now, for each material attribute that's tracking vertex color, save
    * some values (ptr, stride, size, current) that we'll need in
    * update_materials(), above, that'll actually copy the vertex color to
    * the material attribute(s).
    */
   for (i = _TNL_FIRST_MAT; i <= _TNL_LAST_MAT; i++) {
      if (VB->AttribPtr[i]->stride) {
	 const GLuint j = store->mat_count++;
	 const GLuint attr = i - _TNL_ATTRIB_MAT_FRONT_AMBIENT;
	 store->mat[j].ptr    = VB->AttribPtr[i]->start;
	 store->mat[j].stride = VB->AttribPtr[i]->stride;
	 store->mat[j].size   = VB->AttribPtr[i]->size;
	 store->mat[j].current = ctx->Light.Material.Attrib[attr];
	 store->mat_bitmask |= (1<<attr);
      }
   }

   /* FIXME: Is this already done?
    */
   _mesa_update_material( ctx, ~0 );

   _tnl_validate_shine_tables( ctx );

   return store->mat_count;
}

/*
 * Compute dp ^ SpecularExponent.
 * Lerp between adjacent values in the f(x) lookup table, giving a
 * continuous function, with adequate overall accuracy.  (Though still
 * pretty good compared to a straight lookup).
 */
static inline GLfloat
lookup_shininess(const struct gl_context *ctx, GLuint face, GLfloat dp)
{
   TNLcontext *tnl = TNL_CONTEXT(ctx);
   const struct tnl_shine_tab *tab = tnl->_ShineTable[face];
   float f = dp * (SHINE_TABLE_SIZE - 1);
   int k = (int) f;
   if (k < 0 /* gcc may cast an overflow float value to negative int value */
	|| k > SHINE_TABLE_SIZE - 2)
      return powf(dp, tab->shininess);
   else
      return tab->tab[k] + (f - k) * (tab->tab[k+1] - tab->tab[k]);
}

/* Tables for all the shading functions.
 */
static light_func _tnl_light_tab[MAX_LIGHT_FUNC];
static light_func _tnl_light_fast_tab[MAX_LIGHT_FUNC];
static light_func _tnl_light_fast_single_tab[MAX_LIGHT_FUNC];
static light_func _tnl_light_spec_tab[MAX_LIGHT_FUNC];

#define TAG(x)           x
#define IDX              (0)
#include "t_vb_lighttmp.h"

#define TAG(x)           x##_twoside
#define IDX              (LIGHT_TWOSIDE)
#include "t_vb_lighttmp.h"

#define TAG(x)           x##_material
#define IDX              (LIGHT_MATERIAL)
#include "t_vb_lighttmp.h"

#define TAG(x)           x##_twoside_material
#define IDX              (LIGHT_TWOSIDE|LIGHT_MATERIAL)
#include "t_vb_lighttmp.h"


static void init_lighting_tables( void )
{
   static int done;

   if (!done) {
      init_light_tab();
      init_light_tab_twoside();
      init_light_tab_material();
      init_light_tab_twoside_material();
      done = 1;
   }
}


static GLboolean run_lighting( struct gl_context *ctx, 
			       struct tnl_pipeline_stage *stage )
{
   struct light_stage_data *store = LIGHT_STAGE_DATA(stage);
   TNLcontext *tnl = TNL_CONTEXT(ctx);
   struct vertex_buffer *VB = &tnl->vb;
   GLvector4f *input = ctx->_NeedEyeCoords ? VB->EyePtr : VB->AttribPtr[_TNL_ATTRIB_POS];
   GLuint idx;

   if (!ctx->Light.Enabled || ctx->VertexProgram._Current)
      return GL_TRUE;

   /* Make sure we can talk about position x,y and z:
    */
   if (input->size <= 2 && input == VB->AttribPtr[_TNL_ATTRIB_POS]) {

      _math_trans_4f( store->Input.data,
		      VB->AttribPtr[_TNL_ATTRIB_POS]->data,
		      VB->AttribPtr[_TNL_ATTRIB_POS]->stride,
		      GL_FLOAT,
		      VB->AttribPtr[_TNL_ATTRIB_POS]->size,
		      0,
		      VB->Count );

      if (input->size <= 2) {
	 /* Clean z.
	  */
	 _mesa_vector4f_clean_elem(&store->Input, VB->Count, 2);
      }
	 
      if (input->size <= 1) {
	 /* Clean y.
	  */
	 _mesa_vector4f_clean_elem(&store->Input, VB->Count, 1);
      }

      input = &store->Input;
   }
   
   idx = 0;

   if (prepare_materials( ctx, VB, store ))
      idx |= LIGHT_MATERIAL;

   if (ctx->Light.Model.TwoSide)
      idx |= LIGHT_TWOSIDE;

   /* The individual functions know about replaying side-effects
    * vs. full re-execution. 
    */
   store->light_func_tab[idx]( ctx, VB, stage, input );

   return GL_TRUE;
}


/* Called in place of do_lighting when the light table may have changed.
 */
static void validate_lighting( struct gl_context *ctx,
					struct tnl_pipeline_stage *stage )
{
   light_func *tab;

   if (!ctx->Light.Enabled || ctx->VertexProgram._Current)
      return;

   if (ctx->Light._NeedVertices) {
      if (ctx->Light.Model.ColorControl == GL_SEPARATE_SPECULAR_COLOR)
	 tab = _tnl_light_spec_tab;
      else
	 tab = _tnl_light_tab;
   }
   else {
      if (ctx->Light.EnabledList.next == ctx->Light.EnabledList.prev)
	 tab = _tnl_light_fast_single_tab;
      else
	 tab = _tnl_light_fast_tab;
   }


   LIGHT_STAGE_DATA(stage)->light_func_tab = tab;

   /* This and the above should only be done on _NEW_LIGHT:
    */
   TNL_CONTEXT(ctx)->Driver.NotifyMaterialChange( ctx );
}



/* Called the first time stage->run is called.  In effect, don't
 * allocate data until the first time the stage is run.
 */
static GLboolean init_lighting( struct gl_context *ctx,
				struct tnl_pipeline_stage *stage )
{
   TNLcontext *tnl = TNL_CONTEXT(ctx);
   struct light_stage_data *store;
   GLuint size = tnl->vb.Size;

   stage->privatePtr = MALLOC(sizeof(*store));
   store = LIGHT_STAGE_DATA(stage);
   if (!store)
      return GL_FALSE;

   /* Do onetime init.
    */
   init_lighting_tables();

   _mesa_vector4f_alloc( &store->Input, 0, size, 32 );
   _mesa_vector4f_alloc( &store->LitColor[0], 0, size, 32 );
   _mesa_vector4f_alloc( &store->LitColor[1], 0, size, 32 );
   _mesa_vector4f_alloc( &store->LitSecondary[0], 0, size, 32 );
   _mesa_vector4f_alloc( &store->LitSecondary[1], 0, size, 32 );

   store->LitColor[0].size = 4;
   store->LitColor[1].size = 4;
   store->LitSecondary[0].size = 3;
   store->LitSecondary[1].size = 3;

   return GL_TRUE;
}




static void dtr( struct tnl_pipeline_stage *stage )
{
   struct light_stage_data *store = LIGHT_STAGE_DATA(stage);

   if (store) {
      _mesa_vector4f_free( &store->Input );
      _mesa_vector4f_free( &store->LitColor[0] );
      _mesa_vector4f_free( &store->LitColor[1] );
      _mesa_vector4f_free( &store->LitSecondary[0] );
      _mesa_vector4f_free( &store->LitSecondary[1] );
      FREE( store );
      stage->privatePtr = NULL;
   }
}

const struct tnl_pipeline_stage _tnl_lighting_stage =
{
   "lighting",			/* name */
   NULL,			/* private_data */
   init_lighting,
   dtr,				/* destroy */
   validate_lighting,
   run_lighting
};
