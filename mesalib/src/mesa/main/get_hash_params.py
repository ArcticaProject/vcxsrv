descriptor=[
{ "apis": ["GL", "GLES", "GLES2", "GL_CORE"], "params": [
  [ "ALPHA_BITS", "BUFFER_INT(Visual.alphaBits), extra_new_buffers" ],
  [ "BLEND", "CONTEXT_BIT0(Color.BlendEnabled), NO_EXTRA" ],
  [ "BLEND_SRC", "CONTEXT_ENUM(Color.Blend[0].SrcRGB), NO_EXTRA" ],
  [ "BLUE_BITS", "BUFFER_INT(Visual.blueBits), extra_new_buffers" ],
  [ "COLOR_CLEAR_VALUE", "LOC_CUSTOM, TYPE_FLOATN_4, 0, extra_new_frag_clamp" ],
  [ "COLOR_WRITEMASK", "LOC_CUSTOM, TYPE_INT_4, 0, NO_EXTRA" ],
  [ "CULL_FACE", "CONTEXT_BOOL(Polygon.CullFlag), NO_EXTRA" ],
  [ "CULL_FACE_MODE", "CONTEXT_ENUM(Polygon.CullFaceMode), NO_EXTRA" ],
  [ "DEPTH_BITS", "BUFFER_INT(Visual.depthBits), extra_new_buffers" ],
  [ "DEPTH_CLEAR_VALUE", "CONTEXT_FIELD(Depth.Clear, TYPE_DOUBLEN), NO_EXTRA" ],
  [ "DEPTH_FUNC", "CONTEXT_ENUM(Depth.Func), NO_EXTRA" ],
  [ "DEPTH_RANGE", "LOC_CUSTOM, TYPE_DOUBLEN_2, 0, NO_EXTRA" ],
  [ "DEPTH_TEST", "CONTEXT_BOOL(Depth.Test), NO_EXTRA" ],
  [ "DEPTH_WRITEMASK", "CONTEXT_BOOL(Depth.Mask), NO_EXTRA" ],
  [ "DITHER", "CONTEXT_BOOL(Color.DitherFlag), NO_EXTRA" ],
  [ "FRONT_FACE", "CONTEXT_ENUM(Polygon.FrontFace), NO_EXTRA" ],
  [ "GREEN_BITS", "BUFFER_INT(Visual.greenBits), extra_new_buffers" ],
  [ "LINE_WIDTH", "CONTEXT_FLOAT(Line.Width), NO_EXTRA" ],
  [ "ALIASED_LINE_WIDTH_RANGE", "CONTEXT_FLOAT2(Const.MinLineWidth), NO_EXTRA" ],
  [ "MAX_ELEMENTS_VERTICES", "CONTEXT_INT(Const.MaxArrayLockSize), NO_EXTRA" ],
  [ "MAX_ELEMENTS_INDICES", "CONTEXT_INT(Const.MaxArrayLockSize), NO_EXTRA" ],
  [ "MAX_TEXTURE_SIZE", "LOC_CUSTOM, TYPE_INT, offsetof(struct gl_context, Const.MaxTextureLevels), NO_EXTRA" ],
  [ "MAX_VIEWPORT_DIMS", "CONTEXT_INT2(Const.MaxViewportWidth), NO_EXTRA" ],
  [ "PACK_ALIGNMENT", "CONTEXT_INT(Pack.Alignment), NO_EXTRA" ],
  [ "ALIASED_POINT_SIZE_RANGE", "CONTEXT_FLOAT2(Const.MinPointSize), NO_EXTRA" ],
  [ "POLYGON_OFFSET_FACTOR", "CONTEXT_FLOAT(Polygon.OffsetFactor ), NO_EXTRA" ],
  [ "POLYGON_OFFSET_UNITS", "CONTEXT_FLOAT(Polygon.OffsetUnits ), NO_EXTRA" ],
  [ "POLYGON_OFFSET_FILL", "CONTEXT_BOOL(Polygon.OffsetFill), NO_EXTRA" ],
  [ "RED_BITS", "BUFFER_INT(Visual.redBits), extra_new_buffers" ],
  [ "SCISSOR_BOX", "LOC_CUSTOM, TYPE_INT_4, 0, NO_EXTRA" ],
  [ "SCISSOR_TEST", "LOC_CUSTOM, TYPE_BOOLEAN, NO_OFFSET, NO_EXTRA" ],
  [ "STENCIL_BITS", "BUFFER_INT(Visual.stencilBits), extra_new_buffers" ],
  [ "STENCIL_CLEAR_VALUE", "CONTEXT_INT(Stencil.Clear), NO_EXTRA" ],
  [ "STENCIL_FAIL", "LOC_CUSTOM, TYPE_ENUM, NO_OFFSET, NO_EXTRA" ],
  [ "STENCIL_FUNC", "LOC_CUSTOM, TYPE_ENUM, NO_OFFSET, NO_EXTRA" ],
  [ "STENCIL_PASS_DEPTH_FAIL", "LOC_CUSTOM, TYPE_ENUM, NO_OFFSET, NO_EXTRA" ],
  [ "STENCIL_PASS_DEPTH_PASS", "LOC_CUSTOM, TYPE_ENUM, NO_OFFSET, NO_EXTRA" ],
  [ "STENCIL_REF", "LOC_CUSTOM, TYPE_INT, NO_OFFSET, NO_EXTRA" ],
  [ "STENCIL_TEST", "CONTEXT_BOOL(Stencil.Enabled), NO_EXTRA" ],
  [ "STENCIL_VALUE_MASK", "LOC_CUSTOM, TYPE_INT, NO_OFFSET, NO_EXTRA" ],
  [ "STENCIL_WRITEMASK", "LOC_CUSTOM, TYPE_INT, NO_OFFSET, NO_EXTRA" ],
  [ "SUBPIXEL_BITS", "CONTEXT_INT(Const.SubPixelBits), NO_EXTRA" ],
  [ "TEXTURE_BINDING_2D", "LOC_CUSTOM, TYPE_INT, TEXTURE_2D_INDEX, NO_EXTRA" ],
  [ "UNPACK_ALIGNMENT", "CONTEXT_INT(Unpack.Alignment), NO_EXTRA" ],
  [ "VIEWPORT", "LOC_CUSTOM, TYPE_FLOAT_4, 0, NO_EXTRA" ],

# GL_ARB_multitexture
  [ "ACTIVE_TEXTURE", "LOC_CUSTOM, TYPE_INT, 0, NO_EXTRA" ],

# Note that all the OES_* extensions require that the Mesa "struct
# gl_extensions" include a member with the name of the extension.
# That structure does not yet include OES extensions (and we're
# not sure whether it will).  If it does, all the OES_*
# extensions below should mark the dependency.

# GL_ARB_texture_cube_map
  [ "TEXTURE_BINDING_CUBE_MAP_ARB", "LOC_CUSTOM, TYPE_INT, TEXTURE_CUBE_INDEX, extra_ARB_texture_cube_map" ],
# XXX: OES_texture_cube_map
  [ "MAX_CUBE_MAP_TEXTURE_SIZE_ARB", "LOC_CUSTOM, TYPE_INT, offsetof(struct gl_context, Const.MaxCubeTextureLevels), extra_ARB_texture_cube_map" ],

# XXX: OES_blend_subtract
  [ "BLEND_SRC_RGB", "CONTEXT_ENUM(Color.Blend[0].SrcRGB), NO_EXTRA" ],
  [ "BLEND_DST_RGB", "CONTEXT_ENUM(Color.Blend[0].DstRGB), NO_EXTRA" ],
  [ "BLEND_SRC_ALPHA", "CONTEXT_ENUM(Color.Blend[0].SrcA), NO_EXTRA" ],
  [ "BLEND_DST_ALPHA", "CONTEXT_ENUM(Color.Blend[0].DstA), NO_EXTRA" ],

# GL_BLEND_EQUATION_RGB, which is what we're really after, is
# defined identically to GL_BLEND_EQUATION.
  [ "BLEND_EQUATION", "CONTEXT_ENUM(Color.Blend[0].EquationRGB), NO_EXTRA" ],
  [ "BLEND_EQUATION_ALPHA_EXT", "CONTEXT_ENUM(Color.Blend[0].EquationA), NO_EXTRA" ],

# GL_ARB_texture_compression
  [ "NUM_COMPRESSED_TEXTURE_FORMATS_ARB", "LOC_CUSTOM, TYPE_INT, 0, NO_EXTRA" ],
  [ "COMPRESSED_TEXTURE_FORMATS", "LOC_CUSTOM, TYPE_INT_N, 0, NO_EXTRA" ],

# GL_ARB_multisample
  [ "SAMPLE_ALPHA_TO_COVERAGE_ARB", "CONTEXT_BOOL(Multisample.SampleAlphaToCoverage), NO_EXTRA" ],
  [ "SAMPLE_COVERAGE_ARB", "CONTEXT_BOOL(Multisample.SampleCoverage), NO_EXTRA" ],
  [ "SAMPLE_COVERAGE_VALUE_ARB", "CONTEXT_FLOAT(Multisample.SampleCoverageValue), NO_EXTRA" ],
  [ "SAMPLE_COVERAGE_INVERT_ARB", "CONTEXT_BOOL(Multisample.SampleCoverageInvert), NO_EXTRA" ],
  [ "SAMPLE_BUFFERS_ARB", "BUFFER_INT(Visual.sampleBuffers), extra_new_buffers" ],
  [ "SAMPLES_ARB", "BUFFER_INT(Visual.samples), extra_new_buffers" ],

# GL_ARB_sample_shading
  [ "SAMPLE_SHADING_ARB", "CONTEXT_BOOL(Multisample.SampleShading), extra_gl40_ARB_sample_shading" ],
  [ "MIN_SAMPLE_SHADING_VALUE_ARB", "CONTEXT_FLOAT(Multisample.MinSampleShadingValue), extra_gl40_ARB_sample_shading" ],

# GL_SGIS_generate_mipmap
  [ "GENERATE_MIPMAP_HINT_SGIS", "CONTEXT_ENUM(Hint.GenerateMipmap), NO_EXTRA" ],

# GL_ARB_vertex_buffer_object
  [ "ARRAY_BUFFER_BINDING_ARB", "LOC_CUSTOM, TYPE_INT, 0, NO_EXTRA" ],

# GL_ARB_vertex_buffer_object
# GL_WEIGHT_ARRAY_BUFFER_BINDING_ARB - not supported
  [ "ELEMENT_ARRAY_BUFFER_BINDING_ARB", "LOC_CUSTOM, TYPE_INT, 0, NO_EXTRA" ],

# GL_ARB_color_buffer_float
  [ "CLAMP_VERTEX_COLOR", "CONTEXT_ENUM(Light.ClampVertexColor), extra_ARB_color_buffer_float" ],
  [ "CLAMP_FRAGMENT_COLOR", "CONTEXT_ENUM(Color.ClampFragmentColor), extra_ARB_color_buffer_float" ],
  [ "CLAMP_READ_COLOR", "CONTEXT_ENUM(Color.ClampReadColor), extra_ARB_color_buffer_float_or_glcore" ],

# GL_ARB_copy_buffer
  [ "COPY_READ_BUFFER", "LOC_CUSTOM, TYPE_INT, 0, NO_EXTRA" ],
  [ "COPY_WRITE_BUFFER", "LOC_CUSTOM, TYPE_INT, 0, NO_EXTRA" ],

# GL_OES_read_format
  [ "IMPLEMENTATION_COLOR_READ_TYPE_OES", "LOC_CUSTOM, TYPE_INT, 0, extra_new_buffers" ],
  [ "IMPLEMENTATION_COLOR_READ_FORMAT_OES", "LOC_CUSTOM, TYPE_INT, 0, extra_new_buffers" ],

# GL_EXT_framebuffer_object
  [ "FRAMEBUFFER_BINDING_EXT", "BUFFER_INT(Name), NO_EXTRA" ],
  [ "RENDERBUFFER_BINDING_EXT", "LOC_CUSTOM, TYPE_INT, 0, NO_EXTRA" ],
  [ "MAX_RENDERBUFFER_SIZE_EXT", "CONTEXT_INT(Const.MaxRenderbufferSize), NO_EXTRA" ],

# This entry isn't spec'ed for GLES 2, but is needed for Mesa's
# GLSL:
  [ "MAX_CLIP_PLANES", "CONTEXT_INT(Const.MaxClipPlanes), NO_EXTRA" ],

# GL_{APPLE,ARB,OES}_vertex_array_object
  [ "VERTEX_ARRAY_BINDING_APPLE", "ARRAY_INT(Name), NO_EXTRA" ],

# GL_EXT_texture_filter_anisotropic
  [ "MAX_TEXTURE_MAX_ANISOTROPY_EXT", "CONTEXT_FLOAT(Const.MaxTextureMaxAnisotropy), extra_EXT_texture_filter_anisotropic" ],
]},

# Enums in OpenGL and GLES1
{ "apis": ["GL", "GLES", "GL_CORE"], "params": [
  [ "MAX_LIGHTS", "CONTEXT_INT(Const.MaxLights), NO_EXTRA" ],
  [ "LIGHT0", "CONTEXT_BOOL(Light.Light[0].Enabled), NO_EXTRA" ],
  [ "LIGHT1", "CONTEXT_BOOL(Light.Light[1].Enabled), NO_EXTRA" ],
  [ "LIGHT2", "CONTEXT_BOOL(Light.Light[2].Enabled), NO_EXTRA" ],
  [ "LIGHT3", "CONTEXT_BOOL(Light.Light[3].Enabled), NO_EXTRA" ],
  [ "LIGHT4", "CONTEXT_BOOL(Light.Light[4].Enabled), NO_EXTRA" ],
  [ "LIGHT5", "CONTEXT_BOOL(Light.Light[5].Enabled), NO_EXTRA" ],
  [ "LIGHT6", "CONTEXT_BOOL(Light.Light[6].Enabled), NO_EXTRA" ],
  [ "LIGHT7", "CONTEXT_BOOL(Light.Light[7].Enabled), NO_EXTRA" ],
  [ "LIGHTING", "CONTEXT_BOOL(Light.Enabled), NO_EXTRA" ],
  [ "LIGHT_MODEL_AMBIENT", "CONTEXT_FIELD(Light.Model.Ambient[0], TYPE_FLOATN_4), NO_EXTRA" ],
  [ "LIGHT_MODEL_TWO_SIDE", "CONTEXT_BOOL(Light.Model.TwoSide), NO_EXTRA" ],
  [ "ALPHA_TEST", "CONTEXT_BOOL(Color.AlphaEnabled), NO_EXTRA" ],
  [ "ALPHA_TEST_FUNC", "CONTEXT_ENUM(Color.AlphaFunc), NO_EXTRA" ],
  [ "ALPHA_TEST_REF", "LOC_CUSTOM, TYPE_FLOATN, 0, extra_new_frag_clamp" ],
  [ "BLEND_DST", "CONTEXT_ENUM(Color.Blend[0].DstRGB), NO_EXTRA" ],
  [ "CLIP_DISTANCE0", "CONTEXT_BIT0(Transform.ClipPlanesEnabled), extra_valid_clip_distance" ],
  [ "CLIP_DISTANCE1", "CONTEXT_BIT1(Transform.ClipPlanesEnabled), extra_valid_clip_distance" ],
  [ "CLIP_DISTANCE2", "CONTEXT_BIT2(Transform.ClipPlanesEnabled), extra_valid_clip_distance" ],
  [ "CLIP_DISTANCE3", "CONTEXT_BIT3(Transform.ClipPlanesEnabled), extra_valid_clip_distance" ],
  [ "CLIP_DISTANCE4", "CONTEXT_BIT4(Transform.ClipPlanesEnabled), extra_valid_clip_distance" ],
  [ "CLIP_DISTANCE5", "CONTEXT_BIT5(Transform.ClipPlanesEnabled), extra_valid_clip_distance" ],
  [ "CLIP_DISTANCE6", "CONTEXT_BIT6(Transform.ClipPlanesEnabled), extra_valid_clip_distance" ],
  [ "CLIP_DISTANCE7", "CONTEXT_BIT7(Transform.ClipPlanesEnabled), extra_valid_clip_distance" ],
  [ "COLOR_MATERIAL", "CONTEXT_BOOL(Light.ColorMaterialEnabled), NO_EXTRA" ],
  [ "CURRENT_COLOR", "CONTEXT_FIELD(Current.Attrib[VERT_ATTRIB_COLOR0][0], TYPE_FLOATN_4), extra_flush_current" ],
  [ "CURRENT_NORMAL", "CONTEXT_FIELD(Current.Attrib[VERT_ATTRIB_NORMAL][0], TYPE_FLOATN_3), extra_flush_current" ],
  [ "CURRENT_TEXTURE_COORDS", "LOC_CUSTOM, TYPE_FLOAT_4, 0, extra_flush_current_valid_texture_unit" ],
  [ "POINT_DISTANCE_ATTENUATION", "CONTEXT_FLOAT3(Point.Params[0]), NO_EXTRA" ],
  [ "FOG", "CONTEXT_BOOL(Fog.Enabled), NO_EXTRA" ],
  [ "FOG_COLOR", "LOC_CUSTOM, TYPE_FLOATN_4, 0, extra_new_frag_clamp" ],
  [ "FOG_DENSITY", "CONTEXT_FLOAT(Fog.Density), NO_EXTRA" ],
  [ "FOG_END", "CONTEXT_FLOAT(Fog.End), NO_EXTRA" ],
  [ "FOG_HINT", "CONTEXT_ENUM(Hint.Fog), NO_EXTRA" ],
  [ "FOG_MODE", "CONTEXT_ENUM(Fog.Mode), NO_EXTRA" ],
  [ "FOG_START", "CONTEXT_FLOAT(Fog.Start), NO_EXTRA" ],
  [ "LINE_SMOOTH", "CONTEXT_BOOL(Line.SmoothFlag), NO_EXTRA" ],
  [ "LINE_SMOOTH_HINT", "CONTEXT_ENUM(Hint.LineSmooth), NO_EXTRA" ],
  [ "LINE_WIDTH_RANGE", "CONTEXT_FLOAT2(Const.MinLineWidthAA), NO_EXTRA" ],
  [ "COLOR_LOGIC_OP", "CONTEXT_BOOL(Color.ColorLogicOpEnabled), NO_EXTRA" ],
  [ "LOGIC_OP_MODE", "CONTEXT_ENUM(Color.LogicOp), NO_EXTRA" ],
  [ "MATRIX_MODE", "CONTEXT_ENUM(Transform.MatrixMode), NO_EXTRA" ],
  [ "MAX_MODELVIEW_STACK_DEPTH", "CONST(MAX_MODELVIEW_STACK_DEPTH), NO_EXTRA" ],
  [ "MAX_PROJECTION_STACK_DEPTH", "CONST(MAX_PROJECTION_STACK_DEPTH), NO_EXTRA" ],
  [ "MAX_TEXTURE_STACK_DEPTH", "CONST(MAX_TEXTURE_STACK_DEPTH), NO_EXTRA" ],
  [ "MODELVIEW_MATRIX", "CONTEXT_MATRIX(ModelviewMatrixStack.Top), NO_EXTRA" ],
  [ "MODELVIEW_STACK_DEPTH", "LOC_CUSTOM, TYPE_INT, offsetof(struct gl_context, ModelviewMatrixStack.Depth), NO_EXTRA" ],
  [ "NORMALIZE", "CONTEXT_BOOL(Transform.Normalize), NO_EXTRA" ],
  [ "PACK_SKIP_IMAGES", "CONTEXT_INT(Pack.SkipImages), NO_EXTRA" ],
  [ "PERSPECTIVE_CORRECTION_HINT", "CONTEXT_ENUM(Hint.PerspectiveCorrection), NO_EXTRA" ],
  [ "POINT_SIZE", "CONTEXT_FLOAT(Point.Size), NO_EXTRA" ],
  [ "POINT_SIZE_RANGE", "CONTEXT_FLOAT2(Const.MinPointSizeAA), NO_EXTRA" ],
  [ "POINT_SMOOTH", "CONTEXT_BOOL(Point.SmoothFlag), NO_EXTRA" ],
  [ "POINT_SMOOTH_HINT", "CONTEXT_ENUM(Hint.PointSmooth), NO_EXTRA" ],
  [ "POINT_SIZE_MIN_EXT", "CONTEXT_FLOAT(Point.MinSize), NO_EXTRA" ],
  [ "POINT_SIZE_MAX_EXT", "CONTEXT_FLOAT(Point.MaxSize), NO_EXTRA" ],
  [ "POINT_FADE_THRESHOLD_SIZE_EXT", "CONTEXT_FLOAT(Point.Threshold), NO_EXTRA" ],
  [ "PROJECTION_MATRIX", "CONTEXT_MATRIX(ProjectionMatrixStack.Top), NO_EXTRA" ],
  [ "PROJECTION_STACK_DEPTH", "LOC_CUSTOM, TYPE_INT, offsetof(struct gl_context, ProjectionMatrixStack.Depth), NO_EXTRA" ],
  [ "RESCALE_NORMAL", "CONTEXT_BOOL(Transform.RescaleNormals), NO_EXTRA" ],
  [ "SHADE_MODEL", "CONTEXT_ENUM(Light.ShadeModel), NO_EXTRA" ],
  [ "TEXTURE_2D", "LOC_CUSTOM, TYPE_BOOLEAN, 0, NO_EXTRA" ],
  [ "TEXTURE_MATRIX", "LOC_CUSTOM, TYPE_MATRIX, 0, extra_valid_texture_unit" ],
  [ "TEXTURE_STACK_DEPTH", "LOC_CUSTOM, TYPE_INT, 0, extra_valid_texture_unit" ],
  [ "VERTEX_ARRAY", "ARRAY_BOOL(VertexAttrib[VERT_ATTRIB_POS].Enabled), NO_EXTRA" ],
  [ "VERTEX_ARRAY_SIZE", "ARRAY_INT(VertexAttrib[VERT_ATTRIB_POS].Size), NO_EXTRA" ],
  [ "VERTEX_ARRAY_TYPE", "ARRAY_ENUM(VertexAttrib[VERT_ATTRIB_POS].Type), NO_EXTRA" ],
  [ "VERTEX_ARRAY_STRIDE", "ARRAY_INT(VertexAttrib[VERT_ATTRIB_POS].Stride), NO_EXTRA" ],
  [ "NORMAL_ARRAY", "ARRAY_BOOL(VertexAttrib[VERT_ATTRIB_NORMAL].Enabled), NO_EXTRA" ],
  [ "NORMAL_ARRAY_TYPE", "ARRAY_ENUM(VertexAttrib[VERT_ATTRIB_NORMAL].Type), NO_EXTRA" ],
  [ "NORMAL_ARRAY_STRIDE", "ARRAY_INT(VertexAttrib[VERT_ATTRIB_NORMAL].Stride), NO_EXTRA" ],
  [ "COLOR_ARRAY", "ARRAY_BOOL(VertexAttrib[VERT_ATTRIB_COLOR0].Enabled), NO_EXTRA" ],
  [ "COLOR_ARRAY_SIZE", "LOC_CUSTOM, TYPE_INT, 0, NO_EXTRA" ],
  [ "COLOR_ARRAY_TYPE", "ARRAY_ENUM(VertexAttrib[VERT_ATTRIB_COLOR0].Type), NO_EXTRA" ],
  [ "COLOR_ARRAY_STRIDE", "ARRAY_INT(VertexAttrib[VERT_ATTRIB_COLOR0].Stride), NO_EXTRA" ],
  [ "TEXTURE_COORD_ARRAY", "LOC_CUSTOM, TYPE_BOOLEAN, offsetof(struct gl_vertex_attrib_array, Enabled), NO_EXTRA" ],
  [ "TEXTURE_COORD_ARRAY_SIZE", "LOC_CUSTOM, TYPE_INT, offsetof(struct gl_vertex_attrib_array, Size), NO_EXTRA" ],
  [ "TEXTURE_COORD_ARRAY_TYPE", "LOC_CUSTOM, TYPE_ENUM, offsetof(struct gl_vertex_attrib_array, Type), NO_EXTRA" ],
  [ "TEXTURE_COORD_ARRAY_STRIDE", "LOC_CUSTOM, TYPE_INT, offsetof(struct gl_vertex_attrib_array, Stride), NO_EXTRA" ],

# GL_ARB_multitexture
  [ "MAX_TEXTURE_UNITS", "CONTEXT_INT(Const.MaxTextureUnits), NO_EXTRA" ],
  [ "CLIENT_ACTIVE_TEXTURE", "LOC_CUSTOM, TYPE_INT, 0, NO_EXTRA" ],

# GL_ARB_texture_cube_map
  [ "TEXTURE_CUBE_MAP_ARB", "LOC_CUSTOM, TYPE_BOOLEAN, 0, NO_EXTRA" ],
# S, T, and R are always set at the same time
  [ "TEXTURE_GEN_STR_OES", "LOC_TEXUNIT, TYPE_BIT_0, offsetof(struct gl_texture_unit, TexGenEnabled), NO_EXTRA" ],

# GL_ARB_multisample
  [ "MULTISAMPLE_ARB", "CONTEXT_BOOL(Multisample.Enabled), NO_EXTRA" ],
  [ "SAMPLE_ALPHA_TO_ONE_ARB", "CONTEXT_BOOL(Multisample.SampleAlphaToOne), NO_EXTRA" ],

# GL_ARB_vertex_buffer_object
  [ "VERTEX_ARRAY_BUFFER_BINDING_ARB", "LOC_CUSTOM, TYPE_INT, offsetof(struct gl_vertex_array_object, VertexBinding[VERT_ATTRIB_POS].BufferObj), NO_EXTRA" ],
  [ "NORMAL_ARRAY_BUFFER_BINDING_ARB", "LOC_CUSTOM, TYPE_INT, offsetof(struct gl_vertex_array_object, VertexBinding[VERT_ATTRIB_NORMAL].BufferObj), NO_EXTRA" ],
  [ "COLOR_ARRAY_BUFFER_BINDING_ARB", "LOC_CUSTOM, TYPE_INT, offsetof(struct gl_vertex_array_object, VertexBinding[VERT_ATTRIB_COLOR0].BufferObj), NO_EXTRA" ],
  [ "TEXTURE_COORD_ARRAY_BUFFER_BINDING_ARB", "LOC_CUSTOM, TYPE_INT, NO_OFFSET, NO_EXTRA" ],

# GL_OES_point_sprite
  [ "POINT_SPRITE_NV", "CONTEXT_BOOL(Point.PointSprite), extra_NV_point_sprite_ARB_point_sprite" ],
]},


{ "apis": ["GLES"], "params": [
# OES_point_size_array
  [ "POINT_SIZE_ARRAY_OES", "ARRAY_FIELD(VertexAttrib[VERT_ATTRIB_POINT_SIZE].Enabled, TYPE_BOOLEAN)" ],
  [ "POINT_SIZE_ARRAY_TYPE_OES", "ARRAY_FIELD(VertexAttrib[VERT_ATTRIB_POINT_SIZE].Type, TYPE_ENUM)" ],
  [ "POINT_SIZE_ARRAY_STRIDE_OES", "ARRAY_FIELD(VertexAttrib[VERT_ATTRIB_POINT_SIZE].Stride, TYPE_INT)" ],
  [ "POINT_SIZE_ARRAY_BUFFER_BINDING_OES", "LOC_CUSTOM, TYPE_INT, 0" ],
]},

{ "apis": ["GL", "GL_CORE", "GLES2"], "params": [
# == GL_MAX_TEXTURE_COORDS_NV
  [ "MAX_TEXTURE_COORDS_ARB", "CONTEXT_INT(Const.MaxTextureCoordUnits), extra_ARB_fragment_program" ],
  [ "PACK_IMAGE_HEIGHT", "CONTEXT_INT(Pack.ImageHeight), NO_EXTRA" ],
  [ "PACK_ROW_LENGTH", "CONTEXT_INT(Pack.RowLength), NO_EXTRA" ],
  [ "PACK_SKIP_PIXELS", "CONTEXT_INT(Pack.SkipPixels), NO_EXTRA" ],
  [ "PACK_SKIP_ROWS", "CONTEXT_INT(Pack.SkipRows), NO_EXTRA" ],
  [ "UNPACK_ROW_LENGTH", "CONTEXT_INT(Unpack.RowLength), NO_EXTRA" ],
  [ "UNPACK_SKIP_PIXELS", "CONTEXT_INT(Unpack.SkipPixels), NO_EXTRA" ],
  [ "UNPACK_SKIP_ROWS", "CONTEXT_INT(Unpack.SkipRows), NO_EXTRA" ],
  [ "UNPACK_SKIP_IMAGES", "CONTEXT_INT(Unpack.SkipImages), NO_EXTRA" ],
  [ "UNPACK_IMAGE_HEIGHT", "CONTEXT_INT(Unpack.ImageHeight), NO_EXTRA" ],

# GL_ARB_draw_buffers
  [ "MAX_DRAW_BUFFERS_ARB", "CONTEXT_INT(Const.MaxDrawBuffers), NO_EXTRA" ],

# GL_EXT_framebuffer_object / GL_NV_fbo_color_attachments
  [ "MAX_COLOR_ATTACHMENTS", "CONTEXT_INT(Const.MaxColorAttachments), NO_EXTRA" ],

# GL_ARB_draw_buffers / GL_NV_draw_buffers (for ES 2.0)
  [ "DRAW_BUFFER0_ARB", "BUFFER_ENUM(ColorDrawBuffer[0]), NO_EXTRA" ],
  [ "DRAW_BUFFER1_ARB", "BUFFER_ENUM(ColorDrawBuffer[1]), extra_valid_draw_buffer" ],
  [ "DRAW_BUFFER2_ARB", "BUFFER_ENUM(ColorDrawBuffer[2]), extra_valid_draw_buffer" ],
  [ "DRAW_BUFFER3_ARB", "BUFFER_ENUM(ColorDrawBuffer[3]), extra_valid_draw_buffer" ],
  [ "DRAW_BUFFER4_ARB", "BUFFER_ENUM(ColorDrawBuffer[4]), extra_valid_draw_buffer" ],
  [ "DRAW_BUFFER5_ARB", "BUFFER_ENUM(ColorDrawBuffer[5]), extra_valid_draw_buffer" ],
  [ "DRAW_BUFFER6_ARB", "BUFFER_ENUM(ColorDrawBuffer[6]), extra_valid_draw_buffer" ],
  [ "DRAW_BUFFER7_ARB", "BUFFER_ENUM(ColorDrawBuffer[7]), extra_valid_draw_buffer" ],
  [ "BLEND_COLOR_EXT", "LOC_CUSTOM, TYPE_FLOATN_4, 0, extra_new_frag_clamp" ],

# GL_ARB_fragment_program
# == GL_MAX_TEXTURE_IMAGE_UNITS_NV
  [ "MAX_TEXTURE_IMAGE_UNITS_ARB", "CONTEXT_INT(Const.Program[MESA_SHADER_FRAGMENT].MaxTextureImageUnits), extra_ARB_fragment_program" ],
  [ "MAX_VERTEX_TEXTURE_IMAGE_UNITS_ARB", "CONTEXT_INT(Const.Program[MESA_SHADER_VERTEX].MaxTextureImageUnits), extra_ARB_vertex_shader" ],
  [ "MAX_COMBINED_TEXTURE_IMAGE_UNITS_ARB", "CONTEXT_INT(Const.MaxCombinedTextureImageUnits), extra_ARB_vertex_shader" ],

# GL_ARB_shader_objects
# Actually, this token isn't part of GL_ARB_shader_objects, but is
# close enough for now.
  [ "CURRENT_PROGRAM", "LOC_CUSTOM, TYPE_INT, 0, NO_EXTRA" ],

# OpenGL 2.0
  [ "STENCIL_BACK_FUNC", "CONTEXT_ENUM(Stencil.Function[1]), NO_EXTRA" ],
  [ "STENCIL_BACK_VALUE_MASK", "CONTEXT_INT(Stencil.ValueMask[1]), NO_EXTRA" ],
  [ "STENCIL_BACK_WRITEMASK", "CONTEXT_INT(Stencil.WriteMask[1]), NO_EXTRA" ],
  [ "STENCIL_BACK_REF", "LOC_CUSTOM, TYPE_INT, NO_OFFSET, NO_EXTRA" ],
  [ "STENCIL_BACK_FAIL", "CONTEXT_ENUM(Stencil.FailFunc[1]), NO_EXTRA" ],
  [ "STENCIL_BACK_PASS_DEPTH_FAIL", "CONTEXT_ENUM(Stencil.ZFailFunc[1]), NO_EXTRA" ],
  [ "STENCIL_BACK_PASS_DEPTH_PASS", "CONTEXT_ENUM(Stencil.ZPassFunc[1]), NO_EXTRA" ],
  [ "MAX_VERTEX_ATTRIBS_ARB", "CONTEXT_INT(Const.Program[MESA_SHADER_VERTEX].MaxAttribs), extra_ARB_vertex_program_api_es2" ],

# OES_texture_3D
  [ "TEXTURE_BINDING_3D", "LOC_CUSTOM, TYPE_INT, TEXTURE_3D_INDEX, NO_EXTRA" ],
  [ "MAX_3D_TEXTURE_SIZE", "LOC_CUSTOM, TYPE_INT, offsetof(struct gl_context, Const.Max3DTextureLevels), NO_EXTRA" ],

# GL_ARB_fragment_program/OES_standard_derivatives
  [ "FRAGMENT_SHADER_DERIVATIVE_HINT", "CONTEXT_ENUM(Hint.FragmentShaderDerivative), extra_ARB_fragment_shader" ],

# GL_NV_read_buffer
  [ "READ_BUFFER", "LOC_CUSTOM, TYPE_ENUM, NO_OFFSET, extra_NV_read_buffer_api_gl" ],

# GL_ARB_ES2_compatibility
  [ "SHADER_COMPILER", "CONST(1), extra_ARB_ES2_compatibility_api_es2" ],
  [ "MAX_VARYING_VECTORS", "CONTEXT_INT(Const.MaxVarying), extra_ARB_ES2_compatibility_api_es2" ],
  [ "MAX_VERTEX_UNIFORM_VECTORS", "LOC_CUSTOM, TYPE_INT, 0, extra_ARB_ES2_compatibility_api_es2" ],
  [ "MAX_FRAGMENT_UNIFORM_VECTORS", "LOC_CUSTOM, TYPE_INT, 0, extra_ARB_ES2_compatibility_api_es2" ],
  [ "NUM_SHADER_BINARY_FORMATS", "CONST(0), extra_ARB_ES2_compatibility_api_es2" ],
  [ "SHADER_BINARY_FORMATS", "LOC_CUSTOM, TYPE_INVALID, 0, extra_ARB_ES2_compatibility_api_es2" ],

# GL_ARB_get_program_binary / GL_OES_get_program_binary
  [ "NUM_PROGRAM_BINARY_FORMATS", "CONST(0), NO_EXTRA" ],
  [ "PROGRAM_BINARY_FORMATS", "LOC_CUSTOM, TYPE_INVALID, 0, NO_EXTRA" ],

# GL_INTEL_performance_query
  [ "PERFQUERY_QUERY_NAME_LENGTH_MAX_INTEL", "CONST(MAX_PERFQUERY_QUERY_NAME_LENGTH), extra_INTEL_performance_query" ],
  [ "PERFQUERY_COUNTER_NAME_LENGTH_MAX_INTEL", "CONST(MAX_PERFQUERY_COUNTER_NAME_LENGTH), extra_INTEL_performance_query" ],
  [ "PERFQUERY_COUNTER_DESC_LENGTH_MAX_INTEL", "CONST(MAX_PERFQUERY_COUNTER_DESC_LENGTH), extra_INTEL_performance_query" ],
  [ "PERFQUERY_GPA_EXTENDED_COUNTERS_INTEL", "CONST(PERFQUERY_HAVE_GPA_EXTENDED_COUNTERS), extra_INTEL_performance_query" ],

# GL_KHR_context_flush_control
  [ "CONTEXT_RELEASE_BEHAVIOR", "CONTEXT_ENUM(Const.ContextReleaseBehavior), NO_EXTRA" ],
]},

# GLES3 is not a typo.
{ "apis": ["GL", "GLES", "GLES3", "GL_CORE"], "params": [
# GL_EXT_texture_lod_bias
  [ "MAX_TEXTURE_LOD_BIAS_EXT", "CONTEXT_FLOAT(Const.MaxTextureLodBias), NO_EXTRA" ],
]},


# Enums in  OpenGL and ES 3.0
{ "apis": ["GL", "GL_CORE", "GLES3"], "params": [
# GL 3.0 / GLES3
  [ "NUM_EXTENSIONS", "LOC_CUSTOM, TYPE_INT, 0, extra_gl30_es3" ],
  [ "MAJOR_VERSION", "LOC_CUSTOM, TYPE_INT, 0, extra_gl30_es3" ],
  [ "MINOR_VERSION", "LOC_CUSTOM, TYPE_INT, 0, extra_gl30_es3" ],

  # GL 3.2 / GLES3
  [ "MAX_VERTEX_OUTPUT_COMPONENTS", "CONTEXT_INT(Const.Program[MESA_SHADER_VERTEX].MaxOutputComponents), extra_gl32_es3" ],
  [ "MAX_FRAGMENT_INPUT_COMPONENTS", "CONTEXT_INT(Const.Program[MESA_SHADER_FRAGMENT].MaxInputComponents), extra_gl32_es3" ],

# GL_ARB_ES3_compatibility
  [ "MAX_ELEMENT_INDEX", "CONTEXT_INT64(Const.MaxElementIndex), extra_ARB_ES3_compatibility_api_es3"],
  [ "PRIMITIVE_RESTART_FIXED_INDEX", "CONTEXT_BOOL(Array.PrimitiveRestartFixedIndex), extra_ARB_ES3_compatibility_api_es3" ],

# GL_ARB_fragment_shader
  [ "MAX_FRAGMENT_UNIFORM_COMPONENTS_ARB", "CONTEXT_INT(Const.Program[MESA_SHADER_FRAGMENT].MaxUniformComponents), extra_ARB_fragment_shader" ],

# GL_ARB_framebuffer_object
  [ "MAX_SAMPLES", "CONTEXT_INT(Const.MaxSamples), extra_ARB_framebuffer_object_EXT_framebuffer_multisample" ],

# GL_ARB_sync
  [ "MAX_SERVER_WAIT_TIMEOUT", "CONTEXT_INT64(Const.MaxServerWaitTimeout), extra_ARB_sync" ],

# GL_ARB_transform_feedback2
  [ "TRANSFORM_FEEDBACK_BUFFER_PAUSED", "LOC_CUSTOM, TYPE_BOOLEAN, 0, extra_ARB_transform_feedback2_api_es3" ],
  [ "TRANSFORM_FEEDBACK_BUFFER_ACTIVE", "LOC_CUSTOM, TYPE_BOOLEAN, 0, extra_ARB_transform_feedback2_api_es3" ],
  [ "TRANSFORM_FEEDBACK_BINDING", "LOC_CUSTOM, TYPE_INT, 0, extra_ARB_transform_feedback2_api_es3" ],

# GL_ARB_uniform_buffer_object
  [ "MAX_VERTEX_UNIFORM_BLOCKS", "CONTEXT_INT(Const.Program[MESA_SHADER_VERTEX].MaxUniformBlocks), extra_ARB_uniform_buffer_object" ],
  [ "MAX_FRAGMENT_UNIFORM_BLOCKS", "CONTEXT_INT(Const.Program[MESA_SHADER_FRAGMENT].MaxUniformBlocks), extra_ARB_uniform_buffer_object" ],
  [ "MAX_COMBINED_UNIFORM_BLOCKS", "CONTEXT_INT(Const.MaxCombinedUniformBlocks), extra_ARB_uniform_buffer_object" ],
  [ "MAX_UNIFORM_BLOCK_SIZE", "CONTEXT_INT(Const.MaxUniformBlockSize), extra_ARB_uniform_buffer_object" ],
  [ "MAX_UNIFORM_BUFFER_BINDINGS", "CONTEXT_INT(Const.MaxUniformBufferBindings), extra_ARB_uniform_buffer_object" ],
  [ "MAX_COMBINED_VERTEX_UNIFORM_COMPONENTS", "CONTEXT_INT(Const.Program[MESA_SHADER_VERTEX].MaxCombinedUniformComponents), extra_ARB_uniform_buffer_object" ],
  [ "MAX_COMBINED_FRAGMENT_UNIFORM_COMPONENTS", "CONTEXT_INT(Const.Program[MESA_SHADER_FRAGMENT].MaxCombinedUniformComponents), extra_ARB_uniform_buffer_object" ],
  [ "UNIFORM_BUFFER_OFFSET_ALIGNMENT", "CONTEXT_INT(Const.UniformBufferOffsetAlignment), extra_ARB_uniform_buffer_object" ],
  [ "UNIFORM_BUFFER_BINDING", "LOC_CUSTOM, TYPE_INT, 0, extra_ARB_uniform_buffer_object" ],

# GL_ARB_vertex_shader
  [ "MAX_VERTEX_UNIFORM_COMPONENTS_ARB", "CONTEXT_INT(Const.Program[MESA_SHADER_VERTEX].MaxUniformComponents), extra_ARB_vertex_shader" ],
  [ "MAX_VARYING_FLOATS_ARB", "LOC_CUSTOM, TYPE_INT, 0, extra_ARB_vertex_shader" ],

# GL_EXT_framebuffer_blit
# NOTE: GL_DRAW_FRAMEBUFFER_BINDING_EXT == GL_FRAMEBUFFER_BINDING_EXT
  [ "READ_FRAMEBUFFER_BINDING_EXT", "LOC_CUSTOM, TYPE_INT, 0, NO_EXTRA" ],

# GL_EXT_gpu_shader4 / GLSL 1.30
  [ "MIN_PROGRAM_TEXEL_OFFSET", "CONTEXT_INT(Const.MinProgramTexelOffset), extra_GLSL_130_es3" ],
  [ "MAX_PROGRAM_TEXEL_OFFSET", "CONTEXT_INT(Const.MaxProgramTexelOffset), extra_GLSL_130_es3" ],

# GL_EXT_pixel_buffer_object
  [ "PIXEL_PACK_BUFFER_BINDING_EXT", "LOC_CUSTOM, TYPE_INT, 0, extra_EXT_pixel_buffer_object" ],
  [ "PIXEL_UNPACK_BUFFER_BINDING_EXT", "LOC_CUSTOM, TYPE_INT, 0, extra_EXT_pixel_buffer_object" ],

  # GL_EXT_texture_array
  [ "TEXTURE_BINDING_2D_ARRAY", "LOC_CUSTOM, TYPE_INT, TEXTURE_2D_ARRAY_INDEX, extra_EXT_texture_array_es3" ],
  [ "MAX_ARRAY_TEXTURE_LAYERS_EXT", "CONTEXT_INT(Const.MaxArrayTextureLayers), extra_EXT_texture_array_es3" ],

# GL_EXT_transform_feedback
  [ "TRANSFORM_FEEDBACK_BUFFER_BINDING", "LOC_CUSTOM, TYPE_INT, 0, extra_EXT_transform_feedback" ],
  [ "RASTERIZER_DISCARD", "CONTEXT_BOOL(RasterDiscard), extra_EXT_transform_feedback" ],
  [ "MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS", "CONTEXT_INT(Const.MaxTransformFeedbackInterleavedComponents), extra_EXT_transform_feedback" ],
  [ "MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS", "CONTEXT_INT(Const.MaxTransformFeedbackBuffers), extra_EXT_transform_feedback" ],
  [ "MAX_TRANSFORM_FEEDBACK_SEPARATE_COMPONENTS", "CONTEXT_INT(Const.MaxTransformFeedbackSeparateComponents), extra_EXT_transform_feedback" ],
]},

{ "apis": ["GLES", "GLES2"], "params": [
# GL_OES_EGL_image_external
  [ "TEXTURE_BINDING_EXTERNAL_OES", "LOC_CUSTOM, TYPE_INT, TEXTURE_EXTERNAL_INDEX, extra_OES_EGL_image_external" ],
  [ "TEXTURE_EXTERNAL_OES", "LOC_CUSTOM, TYPE_BOOLEAN, 0, extra_OES_EGL_image_external" ],
]},

{ "apis": ["GL", "GL_CORE", "GLES3"], "params": [
# GL_ARB_sampler_objects / GL 3.3 / GLES 3.0
  [ "SAMPLER_BINDING", "LOC_CUSTOM, TYPE_INT, GL_SAMPLER_BINDING, NO_EXTRA" ],
]},

# Remaining enums are only in OpenGL
{ "apis": ["GL", "GL_CORE"], "params": [
  [ "ACCUM_RED_BITS", "BUFFER_INT(Visual.accumRedBits), NO_EXTRA" ],
  [ "ACCUM_GREEN_BITS", "BUFFER_INT(Visual.accumGreenBits), NO_EXTRA" ],
  [ "ACCUM_BLUE_BITS", "BUFFER_INT(Visual.accumBlueBits), NO_EXTRA" ],
  [ "ACCUM_ALPHA_BITS", "BUFFER_INT(Visual.accumAlphaBits), NO_EXTRA" ],
  [ "ACCUM_CLEAR_VALUE", "CONTEXT_FIELD(Accum.ClearColor[0], TYPE_FLOATN_4), NO_EXTRA" ],
  [ "ALPHA_BIAS", "CONTEXT_FLOAT(Pixel.AlphaBias), NO_EXTRA" ],
  [ "ALPHA_SCALE", "CONTEXT_FLOAT(Pixel.AlphaScale), NO_EXTRA" ],
  [ "ATTRIB_STACK_DEPTH", "CONTEXT_INT(AttribStackDepth), NO_EXTRA" ],
  [ "AUTO_NORMAL", "CONTEXT_BOOL(Eval.AutoNormal), NO_EXTRA" ],
  [ "AUX_BUFFERS", "BUFFER_INT(Visual.numAuxBuffers), NO_EXTRA" ],
  [ "BLUE_BIAS", "CONTEXT_FLOAT(Pixel.BlueBias), NO_EXTRA" ],
  [ "BLUE_SCALE", "CONTEXT_FLOAT(Pixel.BlueScale), NO_EXTRA" ],
  [ "CLIP_DEPTH_MODE", "CONTEXT_ENUM(Transform.ClipDepthMode), extra_ARB_clip_control" ],
  [ "CLIP_ORIGIN", "CONTEXT_ENUM(Transform.ClipOrigin), extra_ARB_clip_control" ],
  [ "CLIENT_ATTRIB_STACK_DEPTH", "CONTEXT_INT(ClientAttribStackDepth), NO_EXTRA" ],
  [ "COLOR_MATERIAL_FACE", "CONTEXT_ENUM(Light.ColorMaterialFace), NO_EXTRA" ],
  [ "COLOR_MATERIAL_PARAMETER", "CONTEXT_ENUM(Light.ColorMaterialMode), NO_EXTRA" ],
  [ "CURRENT_INDEX", "CONTEXT_FLOAT(Current.Attrib[VERT_ATTRIB_COLOR_INDEX][0]), extra_flush_current" ],
  [ "CURRENT_RASTER_COLOR", "CONTEXT_FIELD(Current.RasterColor[0], TYPE_FLOATN_4), NO_EXTRA" ],
  [ "CURRENT_RASTER_DISTANCE", "CONTEXT_FLOAT(Current.RasterDistance), NO_EXTRA" ],
  [ "CURRENT_RASTER_INDEX", "CONST(1), NO_EXTRA" ],
  [ "CURRENT_RASTER_POSITION", "CONTEXT_FLOAT4(Current.RasterPos[0]), NO_EXTRA" ],
  [ "CURRENT_RASTER_SECONDARY_COLOR", "CONTEXT_FIELD(Current.RasterSecondaryColor[0], TYPE_FLOATN_4), NO_EXTRA" ],
  [ "CURRENT_RASTER_TEXTURE_COORDS", "LOC_CUSTOM, TYPE_FLOAT_4, 0, extra_valid_texture_unit" ],
  [ "CURRENT_RASTER_POSITION_VALID", "CONTEXT_BOOL(Current.RasterPosValid), NO_EXTRA" ],
  [ "DEPTH_BIAS", "CONTEXT_FLOAT(Pixel.DepthBias), NO_EXTRA" ],
  [ "DEPTH_SCALE", "CONTEXT_FLOAT(Pixel.DepthScale), NO_EXTRA" ],
  [ "DOUBLEBUFFER", "BUFFER_INT(Visual.doubleBufferMode), NO_EXTRA" ],
  [ "DRAW_BUFFER", "BUFFER_ENUM(ColorDrawBuffer[0]), NO_EXTRA" ],
  [ "EDGE_FLAG", "LOC_CUSTOM, TYPE_BOOLEAN, 0, extra_flush_current" ],
  [ "FEEDBACK_BUFFER_SIZE", "CONTEXT_INT(Feedback.BufferSize), NO_EXTRA" ],
  [ "FEEDBACK_BUFFER_TYPE", "CONTEXT_ENUM(Feedback.Type), NO_EXTRA" ],
  [ "FOG_INDEX", "CONTEXT_FLOAT(Fog.Index), NO_EXTRA" ],
  [ "GREEN_BIAS", "CONTEXT_FLOAT(Pixel.GreenBias), NO_EXTRA" ],
  [ "GREEN_SCALE", "CONTEXT_FLOAT(Pixel.GreenScale), NO_EXTRA" ],
  [ "INDEX_BITS", "BUFFER_INT(Visual.indexBits), extra_new_buffers" ],
  [ "INDEX_CLEAR_VALUE", "CONTEXT_INT(Color.ClearIndex), NO_EXTRA" ],
  [ "INDEX_MODE", "CONST(0) , NO_EXTRA" ],
  [ "INDEX_OFFSET", "CONTEXT_INT(Pixel.IndexOffset), NO_EXTRA" ],
  [ "INDEX_SHIFT", "CONTEXT_INT(Pixel.IndexShift), NO_EXTRA" ],
  [ "INDEX_WRITEMASK", "CONTEXT_INT(Color.IndexMask), NO_EXTRA" ],
  [ "LIGHT_MODEL_COLOR_CONTROL", "CONTEXT_ENUM(Light.Model.ColorControl), NO_EXTRA" ],
  [ "LIGHT_MODEL_LOCAL_VIEWER", "CONTEXT_BOOL(Light.Model.LocalViewer), NO_EXTRA" ],
  [ "LINE_STIPPLE", "CONTEXT_BOOL(Line.StippleFlag), NO_EXTRA" ],
  [ "LINE_STIPPLE_PATTERN", "LOC_CUSTOM, TYPE_INT, 0, NO_EXTRA" ],
  [ "LINE_STIPPLE_REPEAT", "CONTEXT_INT(Line.StippleFactor), NO_EXTRA" ],
  [ "LINE_WIDTH_GRANULARITY", "CONTEXT_FLOAT(Const.LineWidthGranularity), NO_EXTRA" ],
  [ "LIST_BASE", "CONTEXT_INT(List.ListBase), NO_EXTRA" ],
  [ "LIST_INDEX", "LOC_CUSTOM, TYPE_INT, 0, NO_EXTRA" ],
  [ "LIST_MODE", "LOC_CUSTOM, TYPE_ENUM, 0, NO_EXTRA" ],
  [ "INDEX_LOGIC_OP", "CONTEXT_BOOL(Color.IndexLogicOpEnabled), NO_EXTRA" ],
  [ "MAP1_COLOR_4", "CONTEXT_BOOL(Eval.Map1Color4), NO_EXTRA" ],
  [ "MAP1_GRID_DOMAIN", "CONTEXT_FLOAT2(Eval.MapGrid1u1), NO_EXTRA" ],
  [ "MAP1_GRID_SEGMENTS", "CONTEXT_INT(Eval.MapGrid1un), NO_EXTRA" ],
  [ "MAP1_INDEX", "CONTEXT_BOOL(Eval.Map1Index), NO_EXTRA" ],
  [ "MAP1_NORMAL", "CONTEXT_BOOL(Eval.Map1Normal), NO_EXTRA" ],
  [ "MAP1_TEXTURE_COORD_1", "CONTEXT_BOOL(Eval.Map1TextureCoord1), NO_EXTRA" ],
  [ "MAP1_TEXTURE_COORD_2", "CONTEXT_BOOL(Eval.Map1TextureCoord2), NO_EXTRA" ],
  [ "MAP1_TEXTURE_COORD_3", "CONTEXT_BOOL(Eval.Map1TextureCoord3), NO_EXTRA" ],
  [ "MAP1_TEXTURE_COORD_4", "CONTEXT_BOOL(Eval.Map1TextureCoord4), NO_EXTRA" ],
  [ "MAP1_VERTEX_3", "CONTEXT_BOOL(Eval.Map1Vertex3), NO_EXTRA" ],
  [ "MAP1_VERTEX_4", "CONTEXT_BOOL(Eval.Map1Vertex4), NO_EXTRA" ],
  [ "MAP2_COLOR_4", "CONTEXT_BOOL(Eval.Map2Color4), NO_EXTRA" ],
  [ "MAP2_GRID_DOMAIN", "LOC_CUSTOM, TYPE_FLOAT_4, 0, NO_EXTRA" ],
  [ "MAP2_GRID_SEGMENTS", "CONTEXT_INT2(Eval.MapGrid2un), NO_EXTRA" ],
  [ "MAP2_INDEX", "CONTEXT_BOOL(Eval.Map2Index), NO_EXTRA" ],
  [ "MAP2_NORMAL", "CONTEXT_BOOL(Eval.Map2Normal), NO_EXTRA" ],
  [ "MAP2_TEXTURE_COORD_1", "CONTEXT_BOOL(Eval.Map2TextureCoord1), NO_EXTRA" ],
  [ "MAP2_TEXTURE_COORD_2", "CONTEXT_BOOL(Eval.Map2TextureCoord2), NO_EXTRA" ],
  [ "MAP2_TEXTURE_COORD_3", "CONTEXT_BOOL(Eval.Map2TextureCoord3), NO_EXTRA" ],
  [ "MAP2_TEXTURE_COORD_4", "CONTEXT_BOOL(Eval.Map2TextureCoord4), NO_EXTRA" ],
  [ "MAP2_VERTEX_3", "CONTEXT_BOOL(Eval.Map2Vertex3), NO_EXTRA" ],
  [ "MAP2_VERTEX_4", "CONTEXT_BOOL(Eval.Map2Vertex4), NO_EXTRA" ],
  [ "MAP_COLOR", "CONTEXT_BOOL(Pixel.MapColorFlag), NO_EXTRA" ],
  [ "MAP_STENCIL", "CONTEXT_BOOL(Pixel.MapStencilFlag), NO_EXTRA" ],
  [ "MAX_ATTRIB_STACK_DEPTH", "CONST(MAX_ATTRIB_STACK_DEPTH), NO_EXTRA" ],
  [ "MAX_CLIENT_ATTRIB_STACK_DEPTH", "CONST(MAX_CLIENT_ATTRIB_STACK_DEPTH), NO_EXTRA" ],
  [ "MAX_EVAL_ORDER", "CONST(MAX_EVAL_ORDER), NO_EXTRA" ],
  [ "MAX_LIST_NESTING", "CONST(MAX_LIST_NESTING), NO_EXTRA" ],
  [ "MAX_NAME_STACK_DEPTH", "CONST(MAX_NAME_STACK_DEPTH), NO_EXTRA" ],
  [ "MAX_PIXEL_MAP_TABLE", "CONST(MAX_PIXEL_MAP_TABLE), NO_EXTRA" ],
  [ "MAX_UNIFORM_LOCATIONS", "CONTEXT_INT(Const.MaxUserAssignableUniformLocations), extra_ARB_explicit_uniform_location" ],
  [ "NAME_STACK_DEPTH", "CONTEXT_INT(Select.NameStackDepth), NO_EXTRA" ],
  [ "PACK_LSB_FIRST", "CONTEXT_BOOL(Pack.LsbFirst), NO_EXTRA" ],
  [ "PACK_SWAP_BYTES", "CONTEXT_BOOL(Pack.SwapBytes), NO_EXTRA" ],
  [ "PACK_INVERT_MESA", "CONTEXT_BOOL(Pack.Invert), NO_EXTRA" ],
  [ "PIXEL_MAP_A_TO_A_SIZE", "CONTEXT_INT(PixelMaps.AtoA.Size), NO_EXTRA" ],
  [ "PIXEL_MAP_B_TO_B_SIZE", "CONTEXT_INT(PixelMaps.BtoB.Size), NO_EXTRA" ],
  [ "PIXEL_MAP_G_TO_G_SIZE", "CONTEXT_INT(PixelMaps.GtoG.Size), NO_EXTRA" ],
  [ "PIXEL_MAP_I_TO_A_SIZE", "CONTEXT_INT(PixelMaps.ItoA.Size), NO_EXTRA" ],
  [ "PIXEL_MAP_I_TO_B_SIZE", "CONTEXT_INT(PixelMaps.ItoB.Size), NO_EXTRA" ],
  [ "PIXEL_MAP_I_TO_G_SIZE", "CONTEXT_INT(PixelMaps.ItoG.Size), NO_EXTRA" ],
  [ "PIXEL_MAP_I_TO_I_SIZE", "CONTEXT_INT(PixelMaps.ItoI.Size), NO_EXTRA" ],
  [ "PIXEL_MAP_I_TO_R_SIZE", "CONTEXT_INT(PixelMaps.ItoR.Size), NO_EXTRA" ],
  [ "PIXEL_MAP_R_TO_R_SIZE", "CONTEXT_INT(PixelMaps.RtoR.Size), NO_EXTRA" ],
  [ "PIXEL_MAP_S_TO_S_SIZE", "CONTEXT_INT(PixelMaps.StoS.Size), NO_EXTRA" ],
  [ "POINT_SIZE_GRANULARITY", "CONTEXT_FLOAT(Const.PointSizeGranularity), NO_EXTRA" ],
  [ "POLYGON_MODE", "CONTEXT_ENUM2(Polygon.FrontMode), NO_EXTRA" ],
  [ "POLYGON_OFFSET_BIAS_EXT", "CONTEXT_FLOAT(Polygon.OffsetUnits), NO_EXTRA" ],
  [ "POLYGON_OFFSET_POINT", "CONTEXT_BOOL(Polygon.OffsetPoint), NO_EXTRA" ],
  [ "POLYGON_OFFSET_LINE", "CONTEXT_BOOL(Polygon.OffsetLine), NO_EXTRA" ],
  [ "POLYGON_SMOOTH", "CONTEXT_BOOL(Polygon.SmoothFlag), NO_EXTRA" ],
  [ "POLYGON_SMOOTH_HINT", "CONTEXT_ENUM(Hint.PolygonSmooth), NO_EXTRA" ],
  [ "POLYGON_STIPPLE", "CONTEXT_BOOL(Polygon.StippleFlag), NO_EXTRA" ],
  [ "RED_BIAS", "CONTEXT_FLOAT(Pixel.RedBias), NO_EXTRA" ],
  [ "RED_SCALE", "CONTEXT_FLOAT(Pixel.RedScale), NO_EXTRA" ],
  [ "RENDER_MODE", "CONTEXT_ENUM(RenderMode), NO_EXTRA" ],
  [ "RGBA_MODE", "CONST(1), NO_EXTRA" ],
  [ "SELECTION_BUFFER_SIZE", "CONTEXT_INT(Select.BufferSize), NO_EXTRA" ],
  [ "STEREO", "BUFFER_INT(Visual.stereoMode), NO_EXTRA" ],
  [ "TEXTURE_1D", "LOC_CUSTOM, TYPE_BOOLEAN, NO_OFFSET, NO_EXTRA" ],
  [ "TEXTURE_3D", "LOC_CUSTOM, TYPE_BOOLEAN, NO_OFFSET, NO_EXTRA" ],
  [ "TEXTURE_BINDING_1D", "LOC_CUSTOM, TYPE_INT, TEXTURE_1D_INDEX, NO_EXTRA" ],
  [ "TEXTURE_BINDING_1D_ARRAY", "LOC_CUSTOM, TYPE_INT, TEXTURE_1D_ARRAY_INDEX, extra_EXT_texture_array" ],
  [ "TEXTURE_GEN_S", "LOC_TEXUNIT, TYPE_BIT_0, offsetof(struct gl_texture_unit, TexGenEnabled), NO_EXTRA" ],
  [ "TEXTURE_GEN_T", "LOC_TEXUNIT, TYPE_BIT_1, offsetof(struct gl_texture_unit, TexGenEnabled), NO_EXTRA" ],
  [ "TEXTURE_GEN_R", "LOC_TEXUNIT, TYPE_BIT_2, offsetof(struct gl_texture_unit, TexGenEnabled), NO_EXTRA" ],
  [ "TEXTURE_GEN_Q", "LOC_TEXUNIT, TYPE_BIT_3, offsetof(struct gl_texture_unit, TexGenEnabled), NO_EXTRA" ],
  [ "UNPACK_LSB_FIRST", "CONTEXT_BOOL(Unpack.LsbFirst), NO_EXTRA" ],
  [ "UNPACK_SWAP_BYTES", "CONTEXT_BOOL(Unpack.SwapBytes), NO_EXTRA" ],
  [ "ZOOM_X", "CONTEXT_FLOAT(Pixel.ZoomX), NO_EXTRA" ],
  [ "ZOOM_Y", "CONTEXT_FLOAT(Pixel.ZoomY), NO_EXTRA" ],

# Vertex arrays
  [ "VERTEX_ARRAY_COUNT_EXT", "CONST(0), NO_EXTRA" ],
  [ "NORMAL_ARRAY_COUNT_EXT", "CONST(0), NO_EXTRA" ],
  [ "COLOR_ARRAY_COUNT_EXT", "CONST(0), NO_EXTRA" ],
  [ "INDEX_ARRAY", "ARRAY_BOOL(VertexAttrib[VERT_ATTRIB_COLOR_INDEX].Enabled), NO_EXTRA" ],
  [ "INDEX_ARRAY_TYPE", "ARRAY_ENUM(VertexAttrib[VERT_ATTRIB_COLOR_INDEX].Type), NO_EXTRA" ],
  [ "INDEX_ARRAY_STRIDE", "ARRAY_INT(VertexAttrib[VERT_ATTRIB_COLOR_INDEX].Stride), NO_EXTRA" ],
  [ "INDEX_ARRAY_COUNT_EXT", "CONST(0), NO_EXTRA" ],
  [ "TEXTURE_COORD_ARRAY_COUNT_EXT", "CONST(0), NO_EXTRA" ],
  [ "EDGE_FLAG_ARRAY", "ARRAY_BOOL(VertexAttrib[VERT_ATTRIB_EDGEFLAG].Enabled), NO_EXTRA" ],
  [ "EDGE_FLAG_ARRAY_STRIDE", "ARRAY_INT(VertexAttrib[VERT_ATTRIB_EDGEFLAG].Stride), NO_EXTRA" ],
  [ "EDGE_FLAG_ARRAY_COUNT_EXT", "CONST(0), NO_EXTRA" ],

# GL_ARB_texture_compression
  [ "TEXTURE_COMPRESSION_HINT_ARB", "CONTEXT_INT(Hint.TextureCompression), NO_EXTRA" ],

# GL_EXT_compiled_vertex_array
  [ "ARRAY_ELEMENT_LOCK_FIRST_EXT", "CONTEXT_INT(Array.LockFirst), NO_EXTRA" ],
  [ "ARRAY_ELEMENT_LOCK_COUNT_EXT", "CONTEXT_INT(Array.LockCount), NO_EXTRA" ],

# GL_ARB_compressed_texture_pixel_storage
  [ "UNPACK_COMPRESSED_BLOCK_WIDTH", "CONTEXT_INT(Unpack.CompressedBlockWidth), NO_EXTRA" ],
  [ "UNPACK_COMPRESSED_BLOCK_HEIGHT", "CONTEXT_INT(Unpack.CompressedBlockHeight), NO_EXTRA" ],
  [ "UNPACK_COMPRESSED_BLOCK_DEPTH", "CONTEXT_INT(Unpack.CompressedBlockDepth), NO_EXTRA" ],
  [ "UNPACK_COMPRESSED_BLOCK_SIZE", "CONTEXT_INT(Unpack.CompressedBlockSize), NO_EXTRA" ],
  [ "PACK_COMPRESSED_BLOCK_WIDTH", "CONTEXT_INT(Pack.CompressedBlockWidth), NO_EXTRA" ],
  [ "PACK_COMPRESSED_BLOCK_HEIGHT", "CONTEXT_INT(Pack.CompressedBlockHeight), NO_EXTRA" ],
  [ "PACK_COMPRESSED_BLOCK_DEPTH", "CONTEXT_INT(Pack.CompressedBlockDepth), NO_EXTRA" ],
  [ "PACK_COMPRESSED_BLOCK_SIZE", "CONTEXT_INT(Pack.CompressedBlockSize), NO_EXTRA" ],

# GL_ARB_transpose_matrix
  [ "TRANSPOSE_MODELVIEW_MATRIX_ARB", "CONTEXT_MATRIX_T(ModelviewMatrixStack), NO_EXTRA" ],
  [ "TRANSPOSE_PROJECTION_MATRIX_ARB", "CONTEXT_MATRIX_T(ProjectionMatrixStack.Top), NO_EXTRA" ],
  [ "TRANSPOSE_TEXTURE_MATRIX_ARB", "CONTEXT_MATRIX_T(TextureMatrixStack), NO_EXTRA" ],

# GL_EXT_secondary_color
  [ "COLOR_SUM", "CONTEXT_BOOL(Fog.ColorSumEnabled), NO_EXTRA" ],
  [ "CURRENT_SECONDARY_COLOR", "CONTEXT_FIELD(Current.Attrib[VERT_ATTRIB_COLOR1][0], TYPE_FLOATN_4), extra_flush_current" ],
  [ "SECONDARY_COLOR_ARRAY", "ARRAY_BOOL(VertexAttrib[VERT_ATTRIB_COLOR1].Enabled), NO_EXTRA" ],
  [ "SECONDARY_COLOR_ARRAY_TYPE", "ARRAY_ENUM(VertexAttrib[VERT_ATTRIB_COLOR1].Type), NO_EXTRA" ],
  [ "SECONDARY_COLOR_ARRAY_STRIDE", "ARRAY_INT(VertexAttrib[VERT_ATTRIB_COLOR1].Stride), NO_EXTRA" ],
  [ "SECONDARY_COLOR_ARRAY_SIZE", "LOC_CUSTOM, TYPE_INT, 0, NO_EXTRA" ],

# GL_EXT_fog_coord
  [ "CURRENT_FOG_COORDINATE", "CONTEXT_FLOAT(Current.Attrib[VERT_ATTRIB_FOG][0]), extra_flush_current" ],
  [ "FOG_COORDINATE_ARRAY", "ARRAY_BOOL(VertexAttrib[VERT_ATTRIB_FOG].Enabled), NO_EXTRA" ],
  [ "FOG_COORDINATE_ARRAY_TYPE", "ARRAY_ENUM(VertexAttrib[VERT_ATTRIB_FOG].Type), NO_EXTRA" ],
  [ "FOG_COORDINATE_ARRAY_STRIDE", "ARRAY_INT(VertexAttrib[VERT_ATTRIB_FOG].Stride), NO_EXTRA" ],
  [ "FOG_COORDINATE_SOURCE", "CONTEXT_ENUM(Fog.FogCoordinateSource), NO_EXTRA" ],

# GL_NV_fog_distance
  [ "FOG_DISTANCE_MODE_NV", "CONTEXT_ENUM(Fog.FogDistanceMode), extra_NV_fog_distance" ],

# GL_IBM_rasterpos_clip
  [ "RASTER_POSITION_UNCLIPPED_IBM", "CONTEXT_BOOL(Transform.RasterPositionUnclipped), NO_EXTRA" ],

# GL_NV_point_sprite
  [ "POINT_SPRITE_R_MODE_NV", "CONTEXT_ENUM(Point.SpriteRMode), extra_NV_point_sprite" ],
  [ "POINT_SPRITE_COORD_ORIGIN", "CONTEXT_ENUM(Point.SpriteOrigin), extra_NV_point_sprite_ARB_point_sprite" ],

# GL_NV_texture_rectangle
  [ "TEXTURE_RECTANGLE_NV", "LOC_CUSTOM, TYPE_BOOLEAN, 0, extra_NV_texture_rectangle" ],
  [ "TEXTURE_BINDING_RECTANGLE_NV", "LOC_CUSTOM, TYPE_INT, TEXTURE_RECT_INDEX, extra_NV_texture_rectangle" ],
  [ "MAX_RECTANGLE_TEXTURE_SIZE_NV", "CONTEXT_INT(Const.MaxTextureRectSize), extra_NV_texture_rectangle" ],

# GL_EXT_stencil_two_side
  [ "STENCIL_TEST_TWO_SIDE_EXT", "CONTEXT_BOOL(Stencil.TestTwoSide), extra_EXT_stencil_two_side" ],
  [ "ACTIVE_STENCIL_FACE_EXT", "LOC_CUSTOM, TYPE_ENUM, NO_OFFSET, NO_EXTRA" ],

# GL_NV_light_max_exponent
  [ "MAX_SHININESS_NV", "CONTEXT_FLOAT(Const.MaxShininess), NO_EXTRA" ],
  [ "MAX_SPOT_EXPONENT_NV", "CONTEXT_FLOAT(Const.MaxSpotExponent), NO_EXTRA" ],

# GL_NV_primitive_restart
  [ "PRIMITIVE_RESTART_NV", "CONTEXT_BOOL(Array.PrimitiveRestart), extra_NV_primitive_restart" ],
  [ "PRIMITIVE_RESTART_INDEX_NV", "CONTEXT_INT(Array.RestartIndex), extra_NV_primitive_restart" ],

# GL_ARB_vertex_buffer_object
  [ "INDEX_ARRAY_BUFFER_BINDING_ARB", "LOC_CUSTOM, TYPE_INT, offsetof(struct gl_vertex_array_object, VertexBinding[VERT_ATTRIB_COLOR_INDEX].BufferObj), NO_EXTRA" ],
  [ "EDGE_FLAG_ARRAY_BUFFER_BINDING_ARB", "LOC_CUSTOM, TYPE_INT, offsetof(struct gl_vertex_array_object, VertexBinding[VERT_ATTRIB_EDGEFLAG].BufferObj), NO_EXTRA" ],
  [ "SECONDARY_COLOR_ARRAY_BUFFER_BINDING_ARB", "LOC_CUSTOM, TYPE_INT, offsetof(struct gl_vertex_array_object, VertexBinding[VERT_ATTRIB_COLOR1].BufferObj), NO_EXTRA" ],
  [ "FOG_COORDINATE_ARRAY_BUFFER_BINDING_ARB", "LOC_CUSTOM, TYPE_INT, offsetof(struct gl_vertex_array_object, VertexBinding[VERT_ATTRIB_FOG].BufferObj), NO_EXTRA" ],

# GL_ARB_vertex_program
# == GL_VERTEX_PROGRAM_NV
  [ "VERTEX_PROGRAM_ARB", "CONTEXT_BOOL(VertexProgram.Enabled), extra_ARB_vertex_program" ],
# == GL_VERTEX_PROGRAM_POINT_SIZE_NV
  [ "VERTEX_PROGRAM_POINT_SIZE_ARB", "CONTEXT_BOOL(VertexProgram.PointSizeEnabled), extra_ARB_vertex_program" ],
# == GL_VERTEX_PROGRAM_TWO_SIDE_NV
  [ "VERTEX_PROGRAM_TWO_SIDE_ARB", "CONTEXT_BOOL(VertexProgram.TwoSideEnabled), extra_ARB_vertex_program" ],
# == GL_MAX_TRACK_MATRIX_STACK_DEPTH_NV
  [ "MAX_PROGRAM_MATRIX_STACK_DEPTH_ARB", "CONTEXT_INT(Const.MaxProgramMatrixStackDepth), extra_ARB_vertex_program_ARB_fragment_program" ],
# == GL_MAX_TRACK_MATRICES_NV
  [ "MAX_PROGRAM_MATRICES_ARB", "CONTEXT_INT(Const.MaxProgramMatrices), extra_ARB_vertex_program_ARB_fragment_program" ],
# == GL_CURRENT_MATRIX_STACK_DEPTH_NV
  [ "CURRENT_MATRIX_STACK_DEPTH_ARB", "LOC_CUSTOM, TYPE_INT, 0, extra_ARB_vertex_program_ARB_fragment_program" ],
# == GL_CURRENT_MATRIX_NV
  [ "CURRENT_MATRIX_ARB", "LOC_CUSTOM, TYPE_MATRIX, 0, extra_ARB_vertex_program_ARB_fragment_program" ],
# == GL_CURRENT_MATRIX_NV
  [ "TRANSPOSE_CURRENT_MATRIX_ARB", "LOC_CUSTOM, TYPE_MATRIX_T, 0, extra_ARB_vertex_program_ARB_fragment_program" ],
# == GL_PROGRAM_ERROR_POSITION_NV
  [ "PROGRAM_ERROR_POSITION_ARB", "CONTEXT_INT(Program.ErrorPos), extra_ARB_vertex_program_ARB_fragment_program" ],

# GL_ARB_fragment_program
  [ "FRAGMENT_PROGRAM_ARB", "CONTEXT_BOOL(FragmentProgram.Enabled), extra_ARB_fragment_program" ],

# GL_EXT_packed_float
  [ "RGBA_SIGNED_COMPONENTS_EXT", "LOC_CUSTOM, TYPE_INT_4, 0, extra_EXT_packed_float" ],

# GL_EXT_depth_bounds_test
  [ "DEPTH_BOUNDS_TEST_EXT", "CONTEXT_BOOL(Depth.BoundsTest), extra_EXT_depth_bounds_test" ],
  [ "DEPTH_BOUNDS_EXT", "CONTEXT_FLOAT2(Depth.BoundsMin), extra_EXT_depth_bounds_test" ],

# GL_ARB_depth_clamp
  [ "DEPTH_CLAMP", "CONTEXT_BOOL(Transform.DepthClamp), extra_ARB_depth_clamp" ],

# GL_ATI_fragment_shader
  [ "NUM_FRAGMENT_REGISTERS_ATI", "CONST(6), extra_ATI_fragment_shader" ],
  [ "NUM_FRAGMENT_CONSTANTS_ATI", "CONST(8), extra_ATI_fragment_shader" ],
  [ "NUM_PASSES_ATI", "CONST(2), extra_ATI_fragment_shader" ],
  [ "NUM_INSTRUCTIONS_PER_PASS_ATI", "CONST(8), extra_ATI_fragment_shader" ],
  [ "NUM_INSTRUCTIONS_TOTAL_ATI", "CONST(16), extra_ATI_fragment_shader" ],
  [ "COLOR_ALPHA_PAIRING_ATI", "CONST(GL_TRUE), extra_ATI_fragment_shader" ],
  [ "NUM_LOOPBACK_COMPONENTS_ATI", "CONST(3), extra_ATI_fragment_shader" ],
  [ "NUM_INPUT_INTERPOLATOR_COMPONENTS_ATI", "CONST(3), extra_ATI_fragment_shader" ],

# GL_EXT_provoking_vertex
  [ "PROVOKING_VERTEX_EXT", "CONTEXT_ENUM(Light.ProvokingVertex), extra_EXT_provoking_vertex" ],
  [ "QUADS_FOLLOW_PROVOKING_VERTEX_CONVENTION_EXT", "CONTEXT_BOOL(Const.QuadsFollowProvokingVertexConvention), extra_EXT_provoking_vertex" ],

# GL_ARB_seamless_cube_map
  [ "TEXTURE_CUBE_MAP_SEAMLESS", "CONTEXT_BOOL(Texture.CubeMapSeamless), extra_ARB_seamless_cube_map" ],

# GL_EXT_texture_integer
  [ "RGBA_INTEGER_MODE_EXT", "BUFFER_BOOL(_IntegerColor), extra_EXT_texture_integer_and_new_buffers" ],

# GL_ARB_transform_feedback3
  [ "MAX_TRANSFORM_FEEDBACK_BUFFERS", "CONTEXT_INT(Const.MaxTransformFeedbackBuffers), extra_ARB_transform_feedback3" ],
  [ "MAX_VERTEX_STREAMS", "CONTEXT_INT(Const.MaxVertexStreams), extra_ARB_transform_feedback3_ARB_gpu_shader5" ],

# GL_ARB_geometry_shader4
  [ "MAX_GEOMETRY_TEXTURE_IMAGE_UNITS_ARB", "CONTEXT_INT(Const.Program[MESA_SHADER_GEOMETRY].MaxTextureImageUnits), extra_gl32_ARB_geometry_shader4" ],
  [ "MAX_GEOMETRY_OUTPUT_VERTICES_ARB", "CONTEXT_INT(Const.MaxGeometryOutputVertices), extra_gl32_ARB_geometry_shader4" ],
  [ "MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS_ARB", "CONTEXT_INT(Const.MaxGeometryTotalOutputComponents), extra_gl32_ARB_geometry_shader4" ],
  [ "MAX_GEOMETRY_UNIFORM_COMPONENTS_ARB", "CONTEXT_INT(Const.Program[MESA_SHADER_GEOMETRY].MaxUniformComponents), extra_gl32_ARB_geometry_shader4" ],
  [ "MAX_GEOMETRY_VARYING_COMPONENTS_ARB", "CONTEXT_INT(Const.Program[MESA_SHADER_GEOMETRY].MaxOutputComponents), extra_ARB_geometry_shader4" ],
  [ "MAX_VERTEX_VARYING_COMPONENTS_ARB", "CONTEXT_INT(Const.Program[MESA_SHADER_VERTEX].MaxOutputComponents), extra_ARB_geometry_shader4" ],

# GL_ARB_color_buffer_float
  [ "RGBA_FLOAT_MODE_ARB", "BUFFER_FIELD(Visual.floatMode, TYPE_BOOLEAN), extra_core_ARB_color_buffer_float_and_new_buffers" ],

# GL_ARB_texture_buffer_object
  [ "MAX_TEXTURE_BUFFER_SIZE_ARB", "CONTEXT_INT(Const.MaxTextureBufferSize), extra_texture_buffer_object" ],
  [ "TEXTURE_BINDING_BUFFER_ARB", "LOC_CUSTOM, TYPE_INT, 0, extra_texture_buffer_object" ],
  [ "TEXTURE_BUFFER_DATA_STORE_BINDING_ARB", "LOC_CUSTOM, TYPE_INT, TEXTURE_BUFFER_INDEX, extra_texture_buffer_object" ],
  [ "TEXTURE_BUFFER_FORMAT_ARB", "LOC_CUSTOM, TYPE_INT, 0, extra_texture_buffer_object" ],
  [ "TEXTURE_BUFFER_ARB", "LOC_CUSTOM, TYPE_INT, 0, extra_texture_buffer_object" ],

# GL_ARB_texture_multisample / GL 3.2
  [ "TEXTURE_BINDING_2D_MULTISAMPLE", "LOC_CUSTOM, TYPE_INT, TEXTURE_2D_MULTISAMPLE_INDEX, extra_ARB_texture_multisample" ],
  [ "TEXTURE_BINDING_2D_MULTISAMPLE_ARRAY", "LOC_CUSTOM, TYPE_INT, TEXTURE_2D_MULTISAMPLE_ARRAY_INDEX, extra_ARB_texture_multisample" ],
  [ "MAX_COLOR_TEXTURE_SAMPLES", "CONTEXT_INT(Const.MaxColorTextureSamples), extra_ARB_texture_multisample" ],
  [ "MAX_DEPTH_TEXTURE_SAMPLES", "CONTEXT_INT(Const.MaxDepthTextureSamples), extra_ARB_texture_multisample" ],
  [ "MAX_INTEGER_SAMPLES", "CONTEXT_INT(Const.MaxIntegerSamples), extra_ARB_texture_multisample" ],
  [ "SAMPLE_MASK", "CONTEXT_BOOL(Multisample.SampleMask), extra_ARB_texture_multisample" ],
  [ "MAX_SAMPLE_MASK_WORDS", "CONST(1), extra_ARB_texture_multisample" ],

# GL 3.0
  [ "CONTEXT_FLAGS", "CONTEXT_INT(Const.ContextFlags), extra_version_30" ],

# GL3.0 / GL_EXT_framebuffer_sRGB
  [ "FRAMEBUFFER_SRGB_EXT", "CONTEXT_BOOL(Color.sRGBEnabled), extra_EXT_framebuffer_sRGB" ],
  [ "FRAMEBUFFER_SRGB_CAPABLE_EXT", "BUFFER_INT(Visual.sRGBCapable), extra_EXT_framebuffer_sRGB_and_new_buffers" ],

# GL 3.1
# NOTE: different enum values for GL_PRIMITIVE_RESTART_NV
# vs. GL_PRIMITIVE_RESTART!
  [ "PRIMITIVE_RESTART", "CONTEXT_BOOL(Array.PrimitiveRestart), extra_version_31" ],
  [ "PRIMITIVE_RESTART_INDEX", "CONTEXT_INT(Array.RestartIndex), extra_version_31" ],

# GL 3.2
  [ "CONTEXT_PROFILE_MASK", "CONTEXT_INT(Const.ProfileMask), extra_version_32" ],
  [ "MAX_GEOMETRY_INPUT_COMPONENTS", "CONTEXT_INT(Const.Program[MESA_SHADER_GEOMETRY].MaxInputComponents), extra_version_32" ],
  [ "MAX_GEOMETRY_OUTPUT_COMPONENTS", "CONTEXT_INT(Const.Program[MESA_SHADER_GEOMETRY].MaxOutputComponents), extra_version_32" ],

# GL 4.4
  [ "MAX_VERTEX_ATTRIB_STRIDE", "CONTEXT_ENUM(Const.MaxVertexAttribStride), NO_EXTRA" ],

# GL_ARB_robustness
  [ "RESET_NOTIFICATION_STRATEGY_ARB", "CONTEXT_ENUM(Const.ResetStrategy), NO_EXTRA" ],

# GL_KHR_debug (GL 4.3)/ GL_ARB_debug_output
  [ "DEBUG_LOGGED_MESSAGES", "LOC_CUSTOM, TYPE_INT, 0, NO_EXTRA" ],
  [ "DEBUG_NEXT_LOGGED_MESSAGE_LENGTH", "LOC_CUSTOM, TYPE_INT, 0, NO_EXTRA" ],
  [ "MAX_DEBUG_LOGGED_MESSAGES", "CONST(MAX_DEBUG_LOGGED_MESSAGES), NO_EXTRA" ],
  [ "MAX_DEBUG_MESSAGE_LENGTH", "CONST(MAX_DEBUG_MESSAGE_LENGTH), NO_EXTRA" ],
  [ "MAX_LABEL_LENGTH", "CONST(MAX_LABEL_LENGTH), NO_EXTRA" ],
  [ "MAX_DEBUG_GROUP_STACK_DEPTH", "CONST(MAX_DEBUG_GROUP_STACK_DEPTH), NO_EXTRA" ],
  [ "DEBUG_GROUP_STACK_DEPTH", "LOC_CUSTOM, TYPE_INT, 0, NO_EXTRA" ],

  [ "MAX_DUAL_SOURCE_DRAW_BUFFERS", "CONTEXT_INT(Const.MaxDualSourceDrawBuffers), extra_ARB_blend_func_extended" ],

# GL_ARB_uniform_buffer_object
  [ "MAX_GEOMETRY_UNIFORM_BLOCKS", "CONTEXT_INT(Const.Program[MESA_SHADER_GEOMETRY].MaxUniformBlocks), extra_ARB_uniform_buffer_object_and_geometry_shader" ],
  [ "MAX_COMBINED_GEOMETRY_UNIFORM_COMPONENTS", "CONTEXT_INT(Const.Program[MESA_SHADER_GEOMETRY].MaxCombinedUniformComponents), extra_ARB_uniform_buffer_object_and_geometry_shader" ],

# GL_ARB_timer_query
  [ "TIMESTAMP", "LOC_CUSTOM, TYPE_INT64, 0, extra_ARB_timer_query" ],

# GL_ARB_map_buffer_alignment
  [ "MIN_MAP_BUFFER_ALIGNMENT", "CONTEXT_INT(Const.MinMapBufferAlignment), NO_EXTRA" ],

# GL_ARB_texture_cube_map_array
  [ "TEXTURE_BINDING_CUBE_MAP_ARRAY_ARB", "LOC_CUSTOM, TYPE_INT, TEXTURE_CUBE_ARRAY_INDEX, extra_ARB_texture_cube_map_array" ],

# GL_ARB_texture_gather
  [ "MIN_PROGRAM_TEXTURE_GATHER_OFFSET", "CONTEXT_INT(Const.MinProgramTextureGatherOffset), extra_ARB_texture_gather"],
  [ "MAX_PROGRAM_TEXTURE_GATHER_OFFSET", "CONTEXT_INT(Const.MaxProgramTextureGatherOffset), extra_ARB_texture_gather"],
  [ "MAX_PROGRAM_TEXTURE_GATHER_COMPONENTS_ARB", "CONTEXT_INT(Const.MaxProgramTextureGatherComponents), extra_ARB_texture_gather"],

# GL_ARB_separate_shader_objects
  [ "PROGRAM_PIPELINE_BINDING", "LOC_CUSTOM, TYPE_INT, GL_PROGRAM_PIPELINE_BINDING, NO_EXTRA" ],

# GL_ARB_shader_atomic_counters
  [ "ATOMIC_COUNTER_BUFFER_BINDING", "LOC_CUSTOM, TYPE_INT, 0, extra_ARB_shader_atomic_counters" ],
  [ "MAX_ATOMIC_COUNTER_BUFFER_BINDINGS", "CONTEXT_INT(Const.MaxAtomicBufferBindings), extra_ARB_shader_atomic_counters" ],
  [ "MAX_ATOMIC_COUNTER_BUFFER_SIZE", "CONTEXT_INT(Const.MaxAtomicBufferSize), extra_ARB_shader_atomic_counters" ],
  [ "MAX_VERTEX_ATOMIC_COUNTER_BUFFERS", "CONTEXT_INT(Const.Program[MESA_SHADER_VERTEX].MaxAtomicBuffers), extra_ARB_shader_atomic_counters" ],
  [ "MAX_VERTEX_ATOMIC_COUNTERS", "CONTEXT_INT(Const.Program[MESA_SHADER_VERTEX].MaxAtomicCounters), extra_ARB_shader_atomic_counters" ],
  [ "MAX_FRAGMENT_ATOMIC_COUNTER_BUFFERS", "CONTEXT_INT(Const.Program[MESA_SHADER_FRAGMENT].MaxAtomicBuffers), extra_ARB_shader_atomic_counters" ],
  [ "MAX_FRAGMENT_ATOMIC_COUNTERS", "CONTEXT_INT(Const.Program[MESA_SHADER_FRAGMENT].MaxAtomicCounters), extra_ARB_shader_atomic_counters" ],
  [ "MAX_GEOMETRY_ATOMIC_COUNTER_BUFFERS", "CONTEXT_INT(Const.Program[MESA_SHADER_GEOMETRY].MaxAtomicBuffers), extra_ARB_shader_atomic_counters_and_geometry_shader" ],
  [ "MAX_GEOMETRY_ATOMIC_COUNTERS", "CONTEXT_INT(Const.Program[MESA_SHADER_GEOMETRY].MaxAtomicCounters), extra_ARB_shader_atomic_counters_and_geometry_shader" ],
  [ "MAX_COMBINED_ATOMIC_COUNTER_BUFFERS", "CONTEXT_INT(Const.MaxCombinedAtomicBuffers), extra_ARB_shader_atomic_counters" ],
  [ "MAX_COMBINED_ATOMIC_COUNTERS", "CONTEXT_INT(Const.MaxCombinedAtomicCounters), extra_ARB_shader_atomic_counters" ],

# GL_ARB_vertex_attrib_binding
  [ "MAX_VERTEX_ATTRIB_RELATIVE_OFFSET", "CONTEXT_ENUM(Const.MaxVertexAttribRelativeOffset), NO_EXTRA" ],
  [ "MAX_VERTEX_ATTRIB_BINDINGS", "CONTEXT_ENUM(Const.MaxVertexAttribBindings), NO_EXTRA" ],

# GL_ARB_shader_image_load_store
  [ "MAX_IMAGE_UNITS", "CONTEXT_INT(Const.MaxImageUnits), extra_ARB_shader_image_load_store"],
  [ "MAX_COMBINED_IMAGE_UNITS_AND_FRAGMENT_OUTPUTS", "CONTEXT_INT(Const.MaxCombinedImageUnitsAndFragmentOutputs), extra_ARB_shader_image_load_store"],
  [ "MAX_IMAGE_SAMPLES", "CONTEXT_INT(Const.MaxImageSamples), extra_ARB_shader_image_load_store"],
  [ "MAX_VERTEX_IMAGE_UNIFORMS", "CONTEXT_INT(Const.Program[MESA_SHADER_VERTEX].MaxImageUniforms), extra_ARB_shader_image_load_store"],
  [ "MAX_GEOMETRY_IMAGE_UNIFORMS", "CONTEXT_INT(Const.Program[MESA_SHADER_GEOMETRY].MaxImageUniforms), extra_ARB_shader_image_load_store_and_geometry_shader"],
  [ "MAX_FRAGMENT_IMAGE_UNIFORMS", "CONTEXT_INT(Const.Program[MESA_SHADER_FRAGMENT].MaxImageUniforms), extra_ARB_shader_image_load_store"],
  [ "MAX_COMBINED_IMAGE_UNIFORMS", "CONTEXT_INT(Const.MaxCombinedImageUniforms), extra_ARB_shader_image_load_store"],

# GL_ARB_compute_shader
  [ "MAX_COMPUTE_WORK_GROUP_INVOCATIONS", "CONTEXT_INT(Const.MaxComputeWorkGroupInvocations), extra_ARB_compute_shader" ],
  [ "MAX_COMPUTE_UNIFORM_BLOCKS", "CONST(MAX_COMPUTE_UNIFORM_BLOCKS), extra_ARB_compute_shader" ],
  [ "MAX_COMPUTE_TEXTURE_IMAGE_UNITS", "CONST(MAX_COMPUTE_TEXTURE_IMAGE_UNITS), extra_ARB_compute_shader" ],
  [ "MAX_COMPUTE_ATOMIC_COUNTER_BUFFERS", "CONST(MAX_COMPUTE_ATOMIC_COUNTER_BUFFERS), extra_ARB_compute_shader" ],
  [ "MAX_COMPUTE_ATOMIC_COUNTERS", "CONST(MAX_COMPUTE_ATOMIC_COUNTERS), extra_ARB_compute_shader" ],
  [ "MAX_COMPUTE_SHARED_MEMORY_SIZE", "CONST(MAX_COMPUTE_SHARED_MEMORY_SIZE), extra_ARB_compute_shader" ],
  [ "MAX_COMPUTE_UNIFORM_COMPONENTS", "CONST(MAX_COMPUTE_UNIFORM_COMPONENTS), extra_ARB_compute_shader" ],
  [ "MAX_COMPUTE_IMAGE_UNIFORMS", "CONST(MAX_COMPUTE_IMAGE_UNIFORMS), extra_ARB_compute_shader" ],

# GL_ARB_gpu_shader5
  [ "MAX_GEOMETRY_SHADER_INVOCATIONS", "CONST(MAX_GEOMETRY_SHADER_INVOCATIONS), extra_ARB_gpu_shader5" ],
  [ "MIN_FRAGMENT_INTERPOLATION_OFFSET", "CONTEXT_FLOAT(Const.MinFragmentInterpolationOffset), extra_ARB_gpu_shader5" ],
  [ "MAX_FRAGMENT_INTERPOLATION_OFFSET", "CONTEXT_FLOAT(Const.MaxFragmentInterpolationOffset), extra_ARB_gpu_shader5" ],
  [ "FRAGMENT_INTERPOLATION_OFFSET_BITS", "CONST(FRAGMENT_INTERPOLATION_OFFSET_BITS), extra_ARB_gpu_shader5" ],
]},

# Enums restricted to OpenGL Core profile
{ "apis": ["GL_CORE"], "params": [
# GL_ARB_texture_buffer_range
  [ "TEXTURE_BUFFER_OFFSET_ALIGNMENT", "CONTEXT_INT(Const.TextureBufferOffsetAlignment), extra_ARB_texture_buffer_range" ],
# GL_ARB_draw_indirect
  [ "DRAW_INDIRECT_BUFFER_BINDING", "LOC_CUSTOM, TYPE_INT, 0, extra_ARB_draw_indirect" ],

# GL_ARB_viewport_array
  [ "MAX_VIEWPORTS", "CONTEXT_INT(Const.MaxViewports), extra_ARB_viewport_array" ],
  [ "VIEWPORT_SUBPIXEL_BITS", "CONTEXT_INT(Const.ViewportSubpixelBits), extra_ARB_viewport_array" ],
  [ "VIEWPORT_BOUNDS_RANGE", "CONTEXT_FLOAT2(Const.ViewportBounds), extra_ARB_viewport_array" ],
  [ "LAYER_PROVOKING_VERTEX", "CONTEXT_ENUM(Light.ProvokingVertex), extra_ARB_viewport_array" ],
  [ "VIEWPORT_INDEX_PROVOKING_VERTEX", "CONTEXT_ENUM(Light.ProvokingVertex), extra_ARB_viewport_array" ],

# GL_EXT_polygon_offset_clamp
  [ "POLYGON_OFFSET_CLAMP_EXT", "CONTEXT_FLOAT(Polygon.OffsetClamp), extra_EXT_polygon_offset_clamp" ],
]}

]
