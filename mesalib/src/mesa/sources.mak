### Lists of source files, included by Makefiles

# this is part of MAIN_SOURCES
MAIN_ES_SOURCES = \
	main/api_exec_es1.c \
	main/api_exec_es2.c

MAIN_SOURCES = \
	main/api_arrayelt.c \
	main/api_exec.c \
	main/api_loopback.c \
	main/api_noop.c \
	main/api_validate.c \
	main/accum.c \
	main/arbprogram.c \
	main/atifragshader.c \
	main/attrib.c \
	main/arrayobj.c \
	main/blend.c \
	main/bufferobj.c \
	main/buffers.c \
	main/clear.c \
	main/clip.c \
	main/colortab.c \
	main/condrender.c \
	main/context.c \
	main/convolve.c \
	main/cpuinfo.c \
	main/debug.c \
	main/depth.c \
	main/depthstencil.c \
	main/dlist.c \
	main/dlopen.c \
	main/drawpix.c \
	main/drawtex.c \
	main/enable.c \
	main/enums.c \
	main/eval.c \
	main/execmem.c \
	main/extensions.c \
	main/fbobject.c \
	main/feedback.c \
	main/ffvertex_prog.c \
	main/fog.c \
	main/formats.c \
	main/framebuffer.c \
	main/get.c \
	main/getstring.c \
	main/hash.c \
	main/hint.c \
	main/histogram.c \
	main/image.c \
	main/imports.c \
	main/light.c \
	main/lines.c \
	main/matrix.c \
	main/mipmap.c \
	main/mm.c \
	main/multisample.c \
	main/nvprogram.c \
	main/pixel.c \
	main/pixelstore.c \
	main/points.c \
	main/polygon.c \
	main/queryobj.c \
	main/querymatrix.c \
	main/rastpos.c \
	main/readpix.c \
	main/remap.c \
	main/renderbuffer.c \
	main/scissor.c \
	main/shaderapi.c \
	main/shaderobj.c \
	main/shared.c \
	main/state.c \
	main/stencil.c \
	main/syncobj.c \
	main/texcompress.c \
	main/texcompress_s3tc.c \
	main/texcompress_fxt1.c \
	main/texenv.c \
	main/texenvprogram.c \
	main/texfetch.c \
	main/texformat.c \
	main/texgen.c \
	main/texgetimage.c \
	main/teximage.c \
	main/texobj.c \
	main/texpal.c \
	main/texparam.c \
	main/texrender.c \
	main/texstate.c \
	main/texstore.c \
	main/transformfeedback.c \
	main/uniforms.c \
	main/varray.c \
	main/version.c \
	main/viewport.c \
	main/vtxfmt.c \
	$(MAIN_ES_SOURCES)

MATH_SOURCES = \
	math/m_debug_clip.c \
	math/m_debug_norm.c \
	math/m_debug_xform.c \
	math/m_eval.c \
	math/m_matrix.c \
	math/m_translate.c \
	math/m_vector.c

MATH_XFORM_SOURCES = \
	math/m_xform.c

SWRAST_SOURCES = \
	swrast/s_aaline.c \
	swrast/s_aatriangle.c \
	swrast/s_accum.c \
	swrast/s_alpha.c \
	swrast/s_atifragshader.c \
	swrast/s_bitmap.c \
	swrast/s_blend.c \
	swrast/s_blit.c \
	swrast/s_clear.c \
	swrast/s_copypix.c \
	swrast/s_context.c \
	swrast/s_depth.c \
	swrast/s_drawpix.c \
	swrast/s_feedback.c \
	swrast/s_fog.c \
	swrast/s_fragprog.c \
	swrast/s_lines.c \
	swrast/s_logic.c \
	swrast/s_masking.c \
	swrast/s_points.c \
	swrast/s_readpix.c \
	swrast/s_span.c \
	swrast/s_stencil.c \
	swrast/s_texcombine.c \
	swrast/s_texfilter.c \
	swrast/s_triangle.c \
	swrast/s_zoom.c

SWRAST_SETUP_SOURCES = \
	swrast_setup/ss_context.c \
	swrast_setup/ss_triangle.c 

TNL_SOURCES = \
	tnl/t_context.c \
	tnl/t_pipeline.c \
	tnl/t_draw.c \
	tnl/t_rasterpos.c \
	tnl/t_vb_program.c \
	tnl/t_vb_render.c \
	tnl/t_vb_texgen.c \
	tnl/t_vb_texmat.c \
	tnl/t_vb_vertex.c \
	tnl/t_vb_cull.c \
	tnl/t_vb_fog.c \
	tnl/t_vb_light.c \
	tnl/t_vb_normals.c \
	tnl/t_vb_points.c \
	tnl/t_vp_build.c \
	tnl/t_vertex.c \
	tnl/t_vertex_sse.c \
	tnl/t_vertex_generic.c 

VBO_SOURCES = \
	vbo/vbo_context.c \
	vbo/vbo_exec.c \
	vbo/vbo_exec_api.c \
	vbo/vbo_exec_array.c \
	vbo/vbo_exec_draw.c \
	vbo/vbo_exec_eval.c \
	vbo/vbo_rebase.c \
	vbo/vbo_split.c \
	vbo/vbo_split_copy.c \
	vbo/vbo_split_inplace.c \
	vbo/vbo_save.c \
	vbo/vbo_save_api.c \
	vbo/vbo_save_draw.c \
	vbo/vbo_save_loopback.c 

STATETRACKER_SOURCES = \
	state_tracker/st_atom.c \
	state_tracker/st_atom_blend.c \
	state_tracker/st_atom_clip.c \
	state_tracker/st_atom_constbuf.c \
	state_tracker/st_atom_depth.c \
	state_tracker/st_atom_framebuffer.c \
	state_tracker/st_atom_msaa.c \
	state_tracker/st_atom_pixeltransfer.c \
	state_tracker/st_atom_sampler.c \
	state_tracker/st_atom_scissor.c \
	state_tracker/st_atom_shader.c \
	state_tracker/st_atom_rasterizer.c \
	state_tracker/st_atom_stipple.c \
	state_tracker/st_atom_texture.c \
	state_tracker/st_atom_viewport.c \
	state_tracker/st_cb_accum.c \
	state_tracker/st_cb_bitmap.c \
	state_tracker/st_cb_blit.c \
	state_tracker/st_cb_bufferobjects.c \
	state_tracker/st_cb_clear.c \
	state_tracker/st_cb_condrender.c \
	state_tracker/st_cb_flush.c \
	state_tracker/st_cb_drawpixels.c \
	state_tracker/st_cb_drawtex.c \
	state_tracker/st_cb_eglimage.c \
	state_tracker/st_cb_fbo.c \
	state_tracker/st_cb_feedback.c \
	state_tracker/st_cb_program.c \
	state_tracker/st_cb_queryobj.c \
	state_tracker/st_cb_rasterpos.c \
	state_tracker/st_cb_readpixels.c \
	state_tracker/st_cb_strings.c \
	state_tracker/st_cb_texture.c \
	state_tracker/st_cb_viewport.c \
	state_tracker/st_cb_xformfb.c \
	state_tracker/st_context.c \
	state_tracker/st_debug.c \
	state_tracker/st_draw.c \
	state_tracker/st_draw_feedback.c \
	state_tracker/st_extensions.c \
	state_tracker/st_format.c \
	state_tracker/st_gen_mipmap.c \
	state_tracker/st_manager.c \
	state_tracker/st_mesa_to_tgsi.c \
	state_tracker/st_program.c \
	state_tracker/st_texture.c

PROGRAM_SOURCES = \
	program/arbprogparse.c \
	program/hash_table.c \
	program/lex.yy.c \
	program/nvfragparse.c \
	program/nvvertparse.c \
	program/program.c \
	program/program_parse.tab.c \
	program/program_parse_extra.c \
	program/prog_cache.c \
	program/prog_execute.c \
	program/prog_instruction.c \
	program/prog_noise.c \
	program/prog_optimize.c \
	program/prog_parameter.c \
	program/prog_parameter_layout.c \
	program/prog_print.c \
	program/prog_statevars.c \
	program/prog_uniform.c \
	program/programopt.c \
	program/symbol_table.c

SHADER_CXX_SOURCES = \
	program/ir_to_mesa.cpp

ASM_C_SOURCES =	\
	x86/common_x86.c \
	x86/x86_xform.c \
	x86/3dnow.c \
	x86/sse.c \
	x86/rtasm/x86sse.c \
	sparc/sparc.c \
	ppc/common_ppc.c \
	x86-64/x86-64.c

X86_SOURCES =			\
	x86/common_x86_asm.S	\
	x86/x86_xform2.S	\
	x86/x86_xform3.S	\
	x86/x86_xform4.S	\
	x86/x86_cliptest.S	\
	x86/mmx_blend.S		\
	x86/3dnow_xform1.S	\
	x86/3dnow_xform2.S	\
	x86/3dnow_xform3.S	\
	x86/3dnow_xform4.S	\
	x86/3dnow_normal.S	\
	x86/sse_xform1.S	\
	x86/sse_xform2.S	\
	x86/sse_xform3.S	\
	x86/sse_xform4.S	\
	x86/sse_normal.S	\
	x86/read_rgba_span_x86.S

X86-64_SOURCES =		\
	x86-64/xform4.S

SPARC_SOURCES =			\
	sparc/clip.S		\
	sparc/norm.S		\
	sparc/xform.S

COMMON_DRIVER_SOURCES =			\
	drivers/common/driverfuncs.c	\
	drivers/common/meta.c


# Sources for building non-Gallium drivers
MESA_SOURCES = \
	$(MAIN_SOURCES)		\
	$(MATH_SOURCES)		\
	$(MATH_XFORM_SOURCES)	\
	$(VBO_SOURCES)		\
	$(TNL_SOURCES)		\
	$(PROGRAM_SOURCES)	\
	$(SWRAST_SOURCES)	\
	$(SWRAST_SETUP_SOURCES)	\
	$(COMMON_DRIVER_SOURCES)\
	$(ASM_C_SOURCES)

MESA_CXX_SOURCES = \
	 $(SHADER_CXX_SOURCES)

# Sources for building Gallium drivers
MESA_GALLIUM_SOURCES = \
	$(MAIN_SOURCES)		\
	$(MATH_SOURCES)		\
	$(VBO_SOURCES)		\
	$(STATETRACKER_SOURCES)	\
	$(PROGRAM_SOURCES)	\
	ppc/common_ppc.c	\
	x86/common_x86.c

MESA_GALLIUM_CXX_SOURCES = \
	 $(SHADER_CXX_SOURCES)

# All the core C sources, for dependency checking
ALL_SOURCES = \
	$(MESA_SOURCES)		\
	$(MESA_CXX_SOURCES)	\
	$(MESA_ASM_SOURCES)	\
	$(STATETRACKER_SOURCES)


### Object files

MESA_OBJECTS = \
	$(MESA_SOURCES:.c=.o) \
	$(MESA_CXX_SOURCES:.cpp=.o) \
	$(MESA_ASM_SOURCES:.S=.o)

MESA_GALLIUM_OBJECTS = \
	$(MESA_GALLIUM_SOURCES:.c=.o) \
	$(MESA_GALLIUM_CXX_SOURCES:.cpp=.o) \
	$(MESA_ASM_SOURCES:.S=.o)


COMMON_DRIVER_OBJECTS = $(COMMON_DRIVER_SOURCES:.c=.o)


### Other archives/libraries

GLSL_LIBS = \
	$(TOP)/src/glsl/libglsl.a


### Include directories

INCLUDE_DIRS = \
	-I$(TOP)/include \
	-I$(TOP)/src/glsl \
	-I$(TOP)/src/mesa \
	-I$(TOP)/src/mapi \
	-I$(TOP)/src/gallium/include \
	-I$(TOP)/src/gallium/auxiliary
