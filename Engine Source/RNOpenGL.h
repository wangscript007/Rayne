//
//  RNOpenGL.h
//  Rayne
//
//  Copyright 2013 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#ifndef __RAYNE_OPENGL_H__
#define __RAYNE_OPENGL_H__

#include <string>
#include "RNDefines.h"

// OpenGL is a moving target, so is Rayne
// We don't rely on system headers which might be outdated, but we rely on whatever the graphics card tells us
#if RN_TARGET_OPENGL

	#if RN_PLATFORM_MAC_OS
		// This is by far the worst OpenGL hack. Ever.
		// Cocoa.h pulls OpenGL in which fucks up everything from glcorearb.h. So yeah, dirty hacks all around
		#if defined(__gl_h_) || defined(__glext_h_)
			#undef GL_VERSION_1_1
			#undef GL_VERSION_1_2
			#undef GL_VERSION_1_3
			#undef GL_VERSION_1_4
			#undef GL_VERSION_1_5
			#undef GL_VERSION_2_0
			#undef GL_VERSION_2_1
			#undef GL_VERSION_3_0
			#undef GL_VERSION_3_1
			#undef GL_VERSION_3_2
			#undef GL_VERSION_3_3
			#undef GL_VERSION_4_0
			#undef GL_VERSION_4_1
			#undef GL_VERSION_4_2
			#undef GL_VERSION_4_3
			#undef GL_VERSION_4_4

			#undef GL_DRAW_FRAMEBUFFER_BINDING
			#undef GL_GLEXT_VERSION

			#undef __gl_h_
			#undef __glext_h_
		#endif
	#endif

	#if RN_PLATFORM_MAC_OS
		#include "glcorearb.h"
		#include "glext.h"
	#else
		#include "ogl/glcorearb.h"
		#include "ogl/glext.h"
	#endif

	#if RN_PLATFORM_WINDOWS
		#include "ogl/wglext.h"

		#pragma comment(lib, "opengl32.lib")
	#endif
#endif

#ifndef NDEBUG
	#define RN_CHECKOPENGL() RN::gl::CheckForError(__FILE__, __LINE__)
	#define RN_CHECKOPENGL_AGGRESSIVE() RN::gl::CheckForError(__FILE__, __LINE__)
#else
	#define RN_CHECKOPENGL() (void)0
	#define RN_CHECKOPENGL_AGGRESSIVE() (void)0
#endif

namespace RN
{
	namespace gl
	{
		enum class Version : int
		{
#if RN_TARGET_OPENGL_ES
			ES2,
#endif
#if RN_TARGET_OPENGL
			Core3_2,
			Core4_1
#endif
		};
		
		enum class Feature : int
		{
			VertexArrays,
			GeometryShaders,
			TessellationShaders,
			AnisotropicFilter,
			ShaderBinary
		};
		
		Version MaximumVersion();
		Version WantedVersion();
		
		void CheckForError(const char *file, int line);
		bool SupportsFeature(Feature feature);
		bool SupportsExtensions(const std::string& extension);

		// 1.0
		extern PFNGLCULLFACEPROC CullFace;
		extern PFNGLFRONTFACEPROC FrontFace;
		extern PFNGLHINTPROC Hint;
		extern PFNGLLINEWIDTHPROC LineWidth;
		extern PFNGLPOINTSIZEPROC PointSize;
		extern PFNGLPOLYGONMODEPROC PolygonMode;
		extern PFNGLSCISSORPROC Scissor;
		extern PFNGLTEXPARAMETERFPROC TexParameterf;
		extern PFNGLTEXPARAMETERFVPROC TexParameterfv;
		extern PFNGLTEXPARAMETERIPROC TexParameteri;
		extern PFNGLTEXPARAMETERIVPROC TexParameteriv;
		extern PFNGLTEXIMAGE1DPROC TexImage1D;
		extern PFNGLTEXIMAGE2DPROC TexImage2D;
		extern PFNGLDRAWBUFFERPROC DrawBuffer;
		extern PFNGLCLEARPROC Clear;
		extern PFNGLCLEARCOLORPROC ClearColor;
		extern PFNGLCLEARSTENCILPROC ClearStencil;
		extern PFNGLCLEARDEPTHPROC ClearDepth;
		extern PFNGLSTENCILMASKPROC StencilMask;
		extern PFNGLCOLORMASKPROC ColorMask;
		extern PFNGLDEPTHMASKPROC DepthMask;
		extern PFNGLDISABLEPROC Disable;
		extern PFNGLENABLEPROC Enable;
		extern PFNGLFINISHPROC Finish;
		extern PFNGLFLUSHPROC Flush;
		extern PFNGLBLENDFUNCPROC BlendFunc;
		extern PFNGLLOGICOPPROC LogicOp;
		extern PFNGLSTENCILFUNCPROC StencilFunc;
		extern PFNGLSTENCILOPPROC StencilOp;
		extern PFNGLDEPTHFUNCPROC DepthFunc;
		extern PFNGLPIXELSTOREFPROC PixelStoref;
		extern PFNGLPIXELSTOREIPROC PixelStorei;
		extern PFNGLREADBUFFERPROC ReadBuffer;
		extern PFNGLREADPIXELSPROC ReadPixels;
		extern PFNGLGETBOOLEANVPROC GetBooleanv;
		extern PFNGLGETDOUBLEVPROC GetDoublev;
		extern PFNGLGETERRORPROC GetError;
		extern PFNGLGETFLOATVPROC GetFloatv;
		extern PFNGLGETINTEGERVPROC GetIntegerv;
		extern PFNGLGETSTRINGPROC GetString;
		extern PFNGLGETTEXIMAGEPROC GetTexImage;
		extern PFNGLGETTEXPARAMETERFVPROC GetTexParameterfv;
		extern PFNGLGETTEXPARAMETERIVPROC GetTexParameteriv;
		extern PFNGLGETTEXLEVELPARAMETERFVPROC GetTexLevelParameterfv;
		extern PFNGLGETTEXLEVELPARAMETERIVPROC GetTexLevelParameteriv;
		extern PFNGLISENABLEDPROC IsEnabled;
		extern PFNGLDEPTHRANGEPROC DepthRange;
		extern PFNGLVIEWPORTPROC Viewport;

		// 1.1
		extern PFNGLDRAWARRAYSPROC DrawArrays;
		extern PFNGLDRAWELEMENTSPROC DrawElements;
		extern PFNGLGETPOINTERVPROC GetPointerv;
		extern PFNGLPOLYGONOFFSETPROC PolygonOffset;
		extern PFNGLCOPYTEXIMAGE1DPROC CopyTexImage1D;
		extern PFNGLCOPYTEXIMAGE2DPROC CopyTexImage2D;
		extern PFNGLCOPYTEXSUBIMAGE1DPROC CopyTexSubImage1D;
		extern PFNGLCOPYTEXSUBIMAGE2DPROC CopyTexSubImage2D;
		extern PFNGLTEXSUBIMAGE1DPROC TexSubImage1D;
		extern PFNGLTEXSUBIMAGE2DPROC TexSubImage2D;
		extern PFNGLBINDTEXTUREPROC BindTexture;
		extern PFNGLDELETETEXTURESPROC DeleteTextures;
		extern PFNGLGENTEXTURESPROC GenTextures;
		extern PFNGLISTEXTUREPROC IsTexture;

		// 1.2
		extern PFNGLDRAWRANGEELEMENTSPROC DrawRangeElements;
		extern PFNGLTEXIMAGE3DPROC TexImage3D;
		extern PFNGLTEXSUBIMAGE3DPROC TexSubImage3D;
		extern PFNGLCOPYTEXSUBIMAGE3DPROC CopyTexSubImage3D;

		// 1.3
		extern PFNGLACTIVETEXTUREPROC ActiveTexture;
		extern PFNGLSAMPLECOVERAGEPROC SampleCoverage;
		extern PFNGLCOMPRESSEDTEXIMAGE3DPROC CompressedTexImage3D;
		extern PFNGLCOMPRESSEDTEXIMAGE2DPROC CompressedTexImage2D;
		extern PFNGLCOMPRESSEDTEXIMAGE1DPROC CompressedTexImage1D;
		extern PFNGLCOMPRESSEDTEXSUBIMAGE3DPROC CompressedTexSubImage3D;
		extern PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC CompressedTexSubImage2D;
		extern PFNGLCOMPRESSEDTEXSUBIMAGE1DPROC CompressedTexSubImage1D;
		extern PFNGLGETCOMPRESSEDTEXIMAGEPROC GetCompressedTexImage;

		// 1.4
		extern PFNGLBLENDFUNCSEPARATEPROC BlendFuncSeparate;
		extern PFNGLMULTIDRAWARRAYSPROC MultiDrawArrays;
		extern PFNGLMULTIDRAWELEMENTSPROC MultiDrawElements;
		extern PFNGLPOINTPARAMETERFPROC PointParameterf;
		extern PFNGLPOINTPARAMETERFVPROC PointParameterfv;
		extern PFNGLPOINTPARAMETERIPROC PointParameteri;
		extern PFNGLPOINTPARAMETERIVPROC PointParameteriv;
		extern PFNGLBLENDCOLORPROC BlendColor;
		extern PFNGLBLENDEQUATIONPROC BlendEquation;

		// 1.5
		extern PFNGLGENQUERIESPROC GenQueries;
		extern PFNGLDELETEQUERIESPROC DeleteQueries;
		extern PFNGLISQUERYPROC IsQuery;
		extern PFNGLBEGINQUERYPROC BeginQuery;
		extern PFNGLENDQUERYPROC EndQuery;
		extern PFNGLGETQUERYIVPROC GetQueryiv;
		extern PFNGLGETQUERYOBJECTIVPROC GetQueryObjectiv;
		extern PFNGLGETQUERYOBJECTUIVPROC GetQueryObjectuiv;
		extern PFNGLBINDBUFFERPROC BindBuffer;
		extern PFNGLDELETEBUFFERSPROC DeleteBuffers;
		extern PFNGLGENBUFFERSPROC GenBuffers;
		extern PFNGLISBUFFERPROC IsBuffer;
		extern PFNGLBUFFERDATAPROC BufferData;
		extern PFNGLBUFFERSUBDATAPROC BufferSubData;
		extern PFNGLGETBUFFERSUBDATAPROC GetBufferSubData;
		extern PFNGLMAPBUFFERPROC MapBuffer;
		extern PFNGLUNMAPBUFFERPROC UnmapBuffer;
		extern PFNGLGETBUFFERPARAMETERIVPROC GetBufferParameteriv;
		extern PFNGLGETBUFFERPOINTERVPROC GetBufferPointerv;

		// 2.0
		extern PFNGLBLENDEQUATIONSEPARATEPROC BlendEquationSeparate;
		extern PFNGLDRAWBUFFERSPROC DrawBuffers;
		extern PFNGLSTENCILOPSEPARATEPROC StencilOpSeparate;
		extern PFNGLSTENCILFUNCSEPARATEPROC StencilFuncSeparate;
		extern PFNGLSTENCILMASKSEPARATEPROC StencilMaskSeparate;
		extern PFNGLATTACHSHADERPROC AttachShader;
		extern PFNGLBINDATTRIBLOCATIONPROC BindAttribLocation;
		extern PFNGLCOMPILESHADERPROC CompileShader;
		extern PFNGLCREATEPROGRAMPROC CreateProgram;
		extern PFNGLCREATESHADERPROC CreateShader;
		extern PFNGLDELETEPROGRAMPROC DeleteProgram;
		extern PFNGLDELETESHADERPROC DeleteShader;
		extern PFNGLDETACHSHADERPROC DetachShader;
		extern PFNGLDISABLEVERTEXATTRIBARRAYPROC DisableVertexAttribArray;
		extern PFNGLENABLEVERTEXATTRIBARRAYPROC EnableVertexAttribArray;
		extern PFNGLGETACTIVEATTRIBPROC GetActiveAttrib;
		extern PFNGLGETACTIVEUNIFORMPROC GetActiveUniform;
		extern PFNGLGETATTACHEDSHADERSPROC GetAttachedShaders;
		extern PFNGLGETATTRIBLOCATIONPROC GetAttribLocation;
		extern PFNGLGETPROGRAMIVPROC GetProgramiv;
		extern PFNGLGETPROGRAMINFOLOGPROC GetProgramInfoLog;
		extern PFNGLGETSHADERIVPROC GetShaderiv;
		extern PFNGLGETSHADERINFOLOGPROC GetShaderInfoLog;
		extern PFNGLGETSHADERSOURCEPROC GetShaderSource;
		extern PFNGLGETUNIFORMLOCATIONPROC GetUniformLocation;
		extern PFNGLGETUNIFORMFVPROC GetUniformfv;
		extern PFNGLGETUNIFORMIVPROC GetUniformiv;
		extern PFNGLGETVERTEXATTRIBDVPROC GetVertexAttribdv;
		extern PFNGLGETVERTEXATTRIBFVPROC GetVertexAttribfv;
		extern PFNGLGETVERTEXATTRIBIVPROC GetVertexAttribiv;
		extern PFNGLGETVERTEXATTRIBPOINTERVPROC GetVertexAttribPointerv;
		extern PFNGLISPROGRAMPROC IsProgram;
		extern PFNGLISSHADERPROC IsShader;
		extern PFNGLLINKPROGRAMPROC LinkProgram;
		extern PFNGLSHADERSOURCEPROC ShaderSource;
		extern PFNGLUSEPROGRAMPROC UseProgram;
		extern PFNGLUNIFORM1FPROC Uniform1f;
		extern PFNGLUNIFORM2FPROC Uniform2f;
		extern PFNGLUNIFORM3FPROC Uniform3f;
		extern PFNGLUNIFORM4FPROC Uniform4f;
		extern PFNGLUNIFORM1IPROC Uniform1i;
		extern PFNGLUNIFORM2IPROC Uniform2i;
		extern PFNGLUNIFORM3IPROC Uniform3i;
		extern PFNGLUNIFORM4IPROC Uniform4i;
		extern PFNGLUNIFORM1FVPROC Uniform1fv;
		extern PFNGLUNIFORM2FVPROC Uniform2fv;
		extern PFNGLUNIFORM3FVPROC Uniform3fv;
		extern PFNGLUNIFORM4FVPROC Uniform4fv;
		extern PFNGLUNIFORM1IVPROC Uniform1iv;
		extern PFNGLUNIFORM2IVPROC Uniform2iv;
		extern PFNGLUNIFORM3IVPROC Uniform3iv;
		extern PFNGLUNIFORM4IVPROC Uniform4iv;
		extern PFNGLUNIFORMMATRIX2FVPROC UniformMatrix2fv;
		extern PFNGLUNIFORMMATRIX3FVPROC UniformMatrix3fv;
		extern PFNGLUNIFORMMATRIX4FVPROC UniformMatrix4fv;
		extern PFNGLVALIDATEPROGRAMPROC ValidateProgram;
		extern PFNGLVERTEXATTRIB1DPROC VertexAttrib1d;
		extern PFNGLVERTEXATTRIB1DVPROC VertexAttrib1dv;
		extern PFNGLVERTEXATTRIB1FPROC VertexAttrib1f;
		extern PFNGLVERTEXATTRIB1FVPROC VertexAttrib1fv;
		extern PFNGLVERTEXATTRIB1SPROC VertexAttrib1s;
		extern PFNGLVERTEXATTRIB1SVPROC VertexAttrib1sv;
		extern PFNGLVERTEXATTRIB2DPROC VertexAttrib2d;
		extern PFNGLVERTEXATTRIB2DVPROC VertexAttrib2dv;
		extern PFNGLVERTEXATTRIB2FPROC VertexAttrib2f;
		extern PFNGLVERTEXATTRIB2FVPROC VertexAttrib2fv;
		extern PFNGLVERTEXATTRIB2SPROC VertexAttrib2s;
		extern PFNGLVERTEXATTRIB2SVPROC VertexAttrib2sv;
		extern PFNGLVERTEXATTRIB3DPROC VertexAttrib3d;
		extern PFNGLVERTEXATTRIB3DVPROC VertexAttrib3dv;
		extern PFNGLVERTEXATTRIB3FPROC VertexAttrib3f;
		extern PFNGLVERTEXATTRIB3FVPROC VertexAttrib3fv;
		extern PFNGLVERTEXATTRIB3SPROC VertexAttrib3s;
		extern PFNGLVERTEXATTRIB3SVPROC VertexAttrib3sv;
		extern PFNGLVERTEXATTRIB4NBVPROC VertexAttrib4Nbv;
		extern PFNGLVERTEXATTRIB4NIVPROC VertexAttrib4Niv;
		extern PFNGLVERTEXATTRIB4NSVPROC VertexAttrib4Nsv;
		extern PFNGLVERTEXATTRIB4NUBPROC VertexAttrib4Nub;
		extern PFNGLVERTEXATTRIB4NUBVPROC VertexAttrib4Nubv;
		extern PFNGLVERTEXATTRIB4NUIVPROC VertexAttrib4Nuiv;
		extern PFNGLVERTEXATTRIB4NUSVPROC VertexAttrib4Nusv;
		extern PFNGLVERTEXATTRIB4BVPROC VertexAttrib4bv;
		extern PFNGLVERTEXATTRIB4DPROC VertexAttrib4d;
		extern PFNGLVERTEXATTRIB4DVPROC VertexAttrib4dv;
		extern PFNGLVERTEXATTRIB4FPROC VertexAttrib4f;
		extern PFNGLVERTEXATTRIB4FVPROC VertexAttrib4fv;
		extern PFNGLVERTEXATTRIB4IVPROC VertexAttrib4iv;
		extern PFNGLVERTEXATTRIB4SPROC VertexAttrib4s;
		extern PFNGLVERTEXATTRIB4SVPROC VertexAttrib4sv;
		extern PFNGLVERTEXATTRIB4UBVPROC VertexAttrib4ubv;
		extern PFNGLVERTEXATTRIB4UIVPROC VertexAttrib4uiv;
		extern PFNGLVERTEXATTRIB4USVPROC VertexAttrib4usv;
		extern PFNGLVERTEXATTRIBPOINTERPROC VertexAttribPointer;

		// 2.1
		extern PFNGLUNIFORMMATRIX2X3FVPROC UniformMatrix2x3fv;
		extern PFNGLUNIFORMMATRIX3X2FVPROC UniformMatrix3x2fv;
		extern PFNGLUNIFORMMATRIX2X4FVPROC UniformMatrix2x4fv;
		extern PFNGLUNIFORMMATRIX4X2FVPROC UniformMatrix4x2fv;
		extern PFNGLUNIFORMMATRIX3X4FVPROC UniformMatrix3x4fv;
		extern PFNGLUNIFORMMATRIX4X3FVPROC UniformMatrix4x3fv;

		// 3.0
		extern PFNGLCOLORMASKIPROC ColorMaski;
		extern PFNGLGETBOOLEANI_VPROC GetBooleani_v;
		extern PFNGLGETINTEGERI_VPROC GetIntegeri_v;
		extern PFNGLENABLEIPROC Enablei;
		extern PFNGLDISABLEIPROC Disablei;
		extern PFNGLISENABLEDIPROC IsEnabledi;
		extern PFNGLBEGINTRANSFORMFEEDBACKPROC BeginTransformFeedback;
		extern PFNGLENDTRANSFORMFEEDBACKPROC EndTransformFeedback;
		extern PFNGLBINDBUFFERRANGEPROC BindBufferRange;
		extern PFNGLBINDBUFFERBASEPROC BindBufferBase;
		extern PFNGLTRANSFORMFEEDBACKVARYINGSPROC TransformFeedbackVaryings;
		extern PFNGLGETTRANSFORMFEEDBACKVARYINGPROC GetTransformFeedbackVarying;
		extern PFNGLCLAMPCOLORPROC ClampColor;
		extern PFNGLBEGINCONDITIONALRENDERPROC BeginConditionalRender;
		extern PFNGLENDCONDITIONALRENDERPROC EndConditionalRender;
		extern PFNGLVERTEXATTRIBIPOINTERPROC VertexAttribIPointer;
		extern PFNGLGETVERTEXATTRIBIIVPROC GetVertexAttribIiv;
		extern PFNGLGETVERTEXATTRIBIUIVPROC GetVertexAttribIuiv;
		extern PFNGLVERTEXATTRIBI1IPROC VertexAttribI1i;
		extern PFNGLVERTEXATTRIBI2IPROC VertexAttribI2i;
		extern PFNGLVERTEXATTRIBI3IPROC VertexAttribI3i;
		extern PFNGLVERTEXATTRIBI4IPROC VertexAttribI4i;
		extern PFNGLVERTEXATTRIBI1UIPROC VertexAttribI1ui;
		extern PFNGLVERTEXATTRIBI2UIPROC VertexAttribI2ui;
		extern PFNGLVERTEXATTRIBI3UIPROC VertexAttribI3ui;
		extern PFNGLVERTEXATTRIBI4UIPROC VertexAttribI4ui;
		extern PFNGLVERTEXATTRIBI1IVPROC VertexAttribI1iv;
		extern PFNGLVERTEXATTRIBI2IVPROC VertexAttribI2iv;
		extern PFNGLVERTEXATTRIBI3IVPROC VertexAttribI3iv;
		extern PFNGLVERTEXATTRIBI4IVPROC VertexAttribI4iv;
		extern PFNGLVERTEXATTRIBI1UIVPROC VertexAttribI1uiv;
		extern PFNGLVERTEXATTRIBI2UIVPROC VertexAttribI2uiv;
		extern PFNGLVERTEXATTRIBI3UIVPROC VertexAttribI3uiv;
		extern PFNGLVERTEXATTRIBI4UIVPROC VertexAttribI4uiv;
		extern PFNGLVERTEXATTRIBI4BVPROC VertexAttribI4bv;
		extern PFNGLVERTEXATTRIBI4SVPROC VertexAttribI4sv;
		extern PFNGLVERTEXATTRIBI4UBVPROC VertexAttribI4ubv;
		extern PFNGLVERTEXATTRIBI4USVPROC VertexAttribI4usv;
		extern PFNGLGETUNIFORMUIVPROC GetUniformuiv;
		extern PFNGLBINDFRAGDATALOCATIONPROC BindFragDataLocation;
		extern PFNGLGETFRAGDATALOCATIONPROC GetFragDataLocation;
		extern PFNGLUNIFORM1UIPROC Uniform1ui;
		extern PFNGLUNIFORM2UIPROC Uniform2ui;
		extern PFNGLUNIFORM3UIPROC Uniform3ui;
		extern PFNGLUNIFORM4UIPROC Uniform4ui;
		extern PFNGLUNIFORM1UIVPROC Uniform1uiv;
		extern PFNGLUNIFORM2UIVPROC Uniform2uiv;
		extern PFNGLUNIFORM3UIVPROC Uniform3uiv;
		extern PFNGLUNIFORM4UIVPROC Uniform4uiv;
		extern PFNGLTEXPARAMETERIIVPROC TexParameterIiv;
		extern PFNGLTEXPARAMETERIUIVPROC TexParameterIuiv;
		extern PFNGLGETTEXPARAMETERIIVPROC GetTexParameterIiv;
		extern PFNGLGETTEXPARAMETERIUIVPROC GetTexParameterIuiv;
		extern PFNGLCLEARBUFFERIVPROC ClearBufferiv;
		extern PFNGLCLEARBUFFERUIVPROC ClearBufferuiv;
		extern PFNGLCLEARBUFFERFVPROC ClearBufferfv;
		extern PFNGLCLEARBUFFERFIPROC ClearBufferfi;
		extern PFNGLGETSTRINGIPROC GetStringi;
		extern PFNGLISRENDERBUFFERPROC IsRenderbuffer;
		extern PFNGLBINDRENDERBUFFERPROC BindRenderbuffer;
		extern PFNGLDELETERENDERBUFFERSPROC DeleteRenderbuffers;
		extern PFNGLGENRENDERBUFFERSPROC GenRenderbuffers;
		extern PFNGLRENDERBUFFERSTORAGEPROC RenderbufferStorage;
		extern PFNGLGETRENDERBUFFERPARAMETERIVPROC GetRenderbufferParameteriv;
		extern PFNGLISFRAMEBUFFERPROC IsFramebuffer;
		extern PFNGLBINDFRAMEBUFFERPROC BindFramebuffer;
		extern PFNGLDELETEFRAMEBUFFERSPROC DeleteFramebuffers;
		extern PFNGLGENFRAMEBUFFERSPROC GenFramebuffers;
		extern PFNGLCHECKFRAMEBUFFERSTATUSPROC CheckFramebufferStatus;
		extern PFNGLFRAMEBUFFERTEXTURE1DPROC FramebufferTexture1D;
		extern PFNGLFRAMEBUFFERTEXTURE2DPROC FramebufferTexture2D;
		extern PFNGLFRAMEBUFFERTEXTURE3DPROC FramebufferTexture3D;
		extern PFNGLFRAMEBUFFERRENDERBUFFERPROC FramebufferRenderbuffer;
		extern PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC GetFramebufferAttachmentParameteriv;
		extern PFNGLGENERATEMIPMAPPROC GenerateMipmap;
		extern PFNGLBLITFRAMEBUFFERPROC BlitFramebuffer;
		extern PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC RenderbufferStorageMultisample;
		extern PFNGLFRAMEBUFFERTEXTURELAYERPROC FramebufferTextureLayer;
		extern PFNGLMAPBUFFERRANGEPROC MapBufferRange;
		extern PFNGLFLUSHMAPPEDBUFFERRANGEPROC FlushMappedBufferRange;
		extern PFNGLBINDVERTEXARRAYPROC BindVertexArray;
		extern PFNGLDELETEVERTEXARRAYSPROC DeleteVertexArrays;
		extern PFNGLGENVERTEXARRAYSPROC GenVertexArrays;
		extern PFNGLISVERTEXARRAYPROC IsVertexArray;

		// 3.1
		extern PFNGLDRAWARRAYSINSTANCEDPROC DrawArraysInstanced;
		extern PFNGLDRAWELEMENTSINSTANCEDPROC DrawElementsInstanced;
		extern PFNGLTEXBUFFERPROC TexBuffer;
		extern PFNGLPRIMITIVERESTARTINDEXPROC PrimitiveRestartIndex;
		extern PFNGLCOPYBUFFERSUBDATAPROC CopyBufferSubData;
		extern PFNGLGETUNIFORMINDICESPROC GetUniformIndices;
		extern PFNGLGETACTIVEUNIFORMSIVPROC GetActiveUniformsiv;
		extern PFNGLGETACTIVEUNIFORMNAMEPROC GetActiveUniformName;
		extern PFNGLGETUNIFORMBLOCKINDEXPROC GetUniformBlockIndex;
		extern PFNGLGETACTIVEUNIFORMBLOCKIVPROC GetActiveUniformBlockiv;
		extern PFNGLGETACTIVEUNIFORMBLOCKNAMEPROC GetActiveUniformBlockName;
		extern PFNGLUNIFORMBLOCKBINDINGPROC UniformBlockBinding;

		// 3.2
		extern PFNGLDRAWELEMENTSBASEVERTEXPROC DrawElementsBaseVertex;
		extern PFNGLDRAWRANGEELEMENTSBASEVERTEXPROC DrawRangeElementsBaseVertex;
		extern PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXPROC DrawElementsInstancedBaseVertex;
		extern PFNGLMULTIDRAWELEMENTSBASEVERTEXPROC MultiDrawElementsBaseVertex;
		extern PFNGLPROVOKINGVERTEXPROC ProvokingVertex;
		extern PFNGLFENCESYNCPROC FenceSync;
		extern PFNGLISSYNCPROC IsSync;
		extern PFNGLDELETESYNCPROC DeleteSync;
		extern PFNGLCLIENTWAITSYNCPROC ClientWaitSync;
		extern PFNGLWAITSYNCPROC WaitSync;
		extern PFNGLGETINTEGER64VPROC GetInteger64v;
		extern PFNGLGETSYNCIVPROC GetSynciv;
		extern PFNGLGETINTEGER64I_VPROC GetInteger64i_v;
		extern PFNGLGETBUFFERPARAMETERI64VPROC GetBufferParameteri64v;
		extern PFNGLFRAMEBUFFERTEXTUREPROC FramebufferTexture;
		extern PFNGLTEXIMAGE2DMULTISAMPLEPROC TexImage2DMultisample;
		extern PFNGLTEXIMAGE3DMULTISAMPLEPROC TexImage3DMultisample;
		extern PFNGLGETMULTISAMPLEFVPROC GetMultisamplefv;
		extern PFNGLSAMPLEMASKIPROC SampleMaski;

		// 3.3
		extern PFNGLBINDFRAGDATALOCATIONINDEXEDPROC BindFragDataLocationIndexed;
		extern PFNGLGETFRAGDATAINDEXPROC GetFragDataIndex;
		extern PFNGLGENSAMPLERSPROC GenSamplers;
		extern PFNGLDELETESAMPLERSPROC DeleteSamplers;
		extern PFNGLISSAMPLERPROC IsSampler;
		extern PFNGLBINDSAMPLERPROC BindSampler;
		extern PFNGLSAMPLERPARAMETERIPROC SamplerParameteri;
		extern PFNGLSAMPLERPARAMETERIVPROC SamplerParameteriv;
		extern PFNGLSAMPLERPARAMETERFPROC SamplerParameterf;
		extern PFNGLSAMPLERPARAMETERFVPROC SamplerParameterfv;
		extern PFNGLSAMPLERPARAMETERIIVPROC SamplerParameterIiv;
		extern PFNGLSAMPLERPARAMETERIUIVPROC SamplerParameterIuiv;
		extern PFNGLGETSAMPLERPARAMETERIVPROC GetSamplerParameteriv;
		extern PFNGLGETSAMPLERPARAMETERIIVPROC GetSamplerParameterIiv;
		extern PFNGLGETSAMPLERPARAMETERFVPROC GetSamplerParameterfv;
		extern PFNGLGETSAMPLERPARAMETERIUIVPROC GetSamplerParameterIuiv;
		extern PFNGLQUERYCOUNTERPROC QueryCounter;
		extern PFNGLGETQUERYOBJECTI64VPROC GetQueryObjecti64v;
		extern PFNGLGETQUERYOBJECTUI64VPROC GetQueryObjectui64v;
		extern PFNGLVERTEXATTRIBDIVISORPROC VertexAttribDivisor;
		extern PFNGLVERTEXATTRIBP1UIPROC VertexAttribP1ui;
		extern PFNGLVERTEXATTRIBP1UIVPROC VertexAttribP1uiv;
		extern PFNGLVERTEXATTRIBP2UIPROC VertexAttribP2ui;
		extern PFNGLVERTEXATTRIBP2UIVPROC VertexAttribP2uiv;
		extern PFNGLVERTEXATTRIBP3UIPROC VertexAttribP3ui;
		extern PFNGLVERTEXATTRIBP3UIVPROC VertexAttribP3uiv;
		extern PFNGLVERTEXATTRIBP4UIPROC VertexAttribP4ui;
		extern PFNGLVERTEXATTRIBP4UIVPROC VertexAttribP4uiv;

		// 4.0
		extern PFNGLMINSAMPLESHADINGPROC MinSampleShading;
		extern PFNGLBLENDEQUATIONIPROC BlendEquationi;
		extern PFNGLBLENDEQUATIONSEPARATEIPROC BlendEquationSeparatei;
		extern PFNGLBLENDFUNCIPROC BlendFunci;
		extern PFNGLBLENDFUNCSEPARATEIPROC BlendFuncSeparatei;
		extern PFNGLDRAWARRAYSINDIRECTPROC DrawArraysIndirect;
		extern PFNGLDRAWELEMENTSINDIRECTPROC DrawElementsIndirect;
		extern PFNGLUNIFORM1DPROC Uniform1d;
		extern PFNGLUNIFORM2DPROC Uniform2d;
		extern PFNGLUNIFORM3DPROC Uniform3d;
		extern PFNGLUNIFORM4DPROC Uniform4d;
		extern PFNGLUNIFORM1DVPROC Uniform1dv;
		extern PFNGLUNIFORM2DVPROC Uniform2dv;
		extern PFNGLUNIFORM3DVPROC Uniform3dv;
		extern PFNGLUNIFORM4DVPROC Uniform4dv;
		extern PFNGLUNIFORMMATRIX2DVPROC UniformMatrix2dv;
		extern PFNGLUNIFORMMATRIX3DVPROC UniformMatrix3dv;
		extern PFNGLUNIFORMMATRIX4DVPROC UniformMatrix4dv;
		extern PFNGLUNIFORMMATRIX2X3DVPROC UniformMatrix2x3dv;
		extern PFNGLUNIFORMMATRIX2X4DVPROC UniformMatrix2x4dv;
		extern PFNGLUNIFORMMATRIX3X2DVPROC UniformMatrix3x2dv;
		extern PFNGLUNIFORMMATRIX3X4DVPROC UniformMatrix3x4dv;
		extern PFNGLUNIFORMMATRIX4X2DVPROC UniformMatrix4x2dv;
		extern PFNGLUNIFORMMATRIX4X3DVPROC UniformMatrix4x3dv;
		extern PFNGLGETUNIFORMDVPROC GetUniformdv;
		extern PFNGLGETSUBROUTINEUNIFORMLOCATIONPROC GetSubroutineUniformLocation;
		extern PFNGLGETSUBROUTINEINDEXPROC GetSubroutineIndex;
		extern PFNGLGETACTIVESUBROUTINEUNIFORMIVPROC GetActiveSubroutineUniformiv;
		extern PFNGLGETACTIVESUBROUTINEUNIFORMNAMEPROC GetActiveSubroutineUniformName;
		extern PFNGLGETACTIVESUBROUTINENAMEPROC GetActiveSubroutineName;
		extern PFNGLUNIFORMSUBROUTINESUIVPROC UniformSubroutinesuiv;
		extern PFNGLGETUNIFORMSUBROUTINEUIVPROC GetUniformSubroutineuiv;
		extern PFNGLGETPROGRAMSTAGEIVPROC GetProgramStageiv;
		extern PFNGLPATCHPARAMETERIPROC PatchParameteri;
		extern PFNGLPATCHPARAMETERFVPROC PatchParameterfv;
		extern PFNGLBINDTRANSFORMFEEDBACKPROC BindTransformFeedback;
		extern PFNGLDELETETRANSFORMFEEDBACKSPROC DeleteTransformFeedbacks;
		extern PFNGLGENTRANSFORMFEEDBACKSPROC GenTransformFeedbacks;
		extern PFNGLISTRANSFORMFEEDBACKPROC IsTransformFeedback;
		extern PFNGLPAUSETRANSFORMFEEDBACKPROC PauseTransformFeedback;
		extern PFNGLRESUMETRANSFORMFEEDBACKPROC ResumeTransformFeedback;
		extern PFNGLDRAWTRANSFORMFEEDBACKPROC DrawTransformFeedback;
		extern PFNGLDRAWTRANSFORMFEEDBACKSTREAMPROC DrawTransformFeedbackStream;
		extern PFNGLBEGINQUERYINDEXEDPROC BeginQueryIndexed;
		extern PFNGLENDQUERYINDEXEDPROC EndQueryIndexed;
		extern PFNGLGETQUERYINDEXEDIVPROC GetQueryIndexediv;

		// 4.1
		extern PFNGLRELEASESHADERCOMPILERPROC ReleaseShaderCompiler;
		extern PFNGLSHADERBINARYPROC ShaderBinary;
		extern PFNGLGETSHADERPRECISIONFORMATPROC GetShaderPrecisionFormat;
		extern PFNGLDEPTHRANGEFPROC DepthRangef;
		extern PFNGLCLEARDEPTHFPROC ClearDepthf;
		extern PFNGLGETPROGRAMBINARYPROC GetProgramBinary;
		extern PFNGLPROGRAMBINARYPROC ProgramBinary;
		extern PFNGLPROGRAMPARAMETERIPROC ProgramParameteri;
		extern PFNGLUSEPROGRAMSTAGESPROC UseProgramStages;
		extern PFNGLACTIVESHADERPROGRAMPROC ActiveShaderProgram;
		extern PFNGLCREATESHADERPROGRAMVPROC CreateShaderProgramv;
		extern PFNGLBINDPROGRAMPIPELINEPROC BindProgramPipeline;
		extern PFNGLDELETEPROGRAMPIPELINESPROC DeleteProgramPipelines;
		extern PFNGLGENPROGRAMPIPELINESPROC GenProgramPipelines;
		extern PFNGLISPROGRAMPIPELINEPROC IsProgramPipeline;
		extern PFNGLGETPROGRAMPIPELINEIVPROC GetProgramPipelineiv;
		extern PFNGLPROGRAMUNIFORM1IPROC ProgramUniform1i;
		extern PFNGLPROGRAMUNIFORM1IVPROC ProgramUniform1iv;
		extern PFNGLPROGRAMUNIFORM1FPROC ProgramUniform1f;
		extern PFNGLPROGRAMUNIFORM1FVPROC ProgramUniform1fv;
		extern PFNGLPROGRAMUNIFORM1DPROC ProgramUniform1d;
		extern PFNGLPROGRAMUNIFORM1DVPROC ProgramUniform1dv;
		extern PFNGLPROGRAMUNIFORM1UIPROC ProgramUniform1ui;
		extern PFNGLPROGRAMUNIFORM1UIVPROC ProgramUniform1uiv;
		extern PFNGLPROGRAMUNIFORM2IPROC ProgramUniform2i;
		extern PFNGLPROGRAMUNIFORM2IVPROC ProgramUniform2iv;
		extern PFNGLPROGRAMUNIFORM2FPROC ProgramUniform2f;
		extern PFNGLPROGRAMUNIFORM2FVPROC ProgramUniform2fv;
		extern PFNGLPROGRAMUNIFORM2DPROC ProgramUniform2d;
		extern PFNGLPROGRAMUNIFORM2DVPROC ProgramUniform2dv;
		extern PFNGLPROGRAMUNIFORM2UIPROC ProgramUniform2ui;
		extern PFNGLPROGRAMUNIFORM2UIVPROC ProgramUniform2uiv;
		extern PFNGLPROGRAMUNIFORM3IPROC ProgramUniform3i;
		extern PFNGLPROGRAMUNIFORM3IVPROC ProgramUniform3iv;
		extern PFNGLPROGRAMUNIFORM3FPROC ProgramUniform3f;
		extern PFNGLPROGRAMUNIFORM3FVPROC ProgramUniform3fv;
		extern PFNGLPROGRAMUNIFORM3DPROC ProgramUniform3d;
		extern PFNGLPROGRAMUNIFORM3DVPROC ProgramUniform3dv;
		extern PFNGLPROGRAMUNIFORM3UIPROC ProgramUniform3ui;
		extern PFNGLPROGRAMUNIFORM3UIVPROC ProgramUniform3uiv;
		extern PFNGLPROGRAMUNIFORM4IPROC ProgramUniform4i;
		extern PFNGLPROGRAMUNIFORM4IVPROC ProgramUniform4iv;
		extern PFNGLPROGRAMUNIFORM4FPROC ProgramUniform4f;
		extern PFNGLPROGRAMUNIFORM4FVPROC ProgramUniform4fv;
		extern PFNGLPROGRAMUNIFORM4DPROC ProgramUniform4d;
		extern PFNGLPROGRAMUNIFORM4DVPROC ProgramUniform4dv;
		extern PFNGLPROGRAMUNIFORM4UIPROC ProgramUniform4ui;
		extern PFNGLPROGRAMUNIFORM4UIVPROC ProgramUniform4uiv;
		extern PFNGLPROGRAMUNIFORMMATRIX2FVPROC ProgramUniformMatrix2fv;
		extern PFNGLPROGRAMUNIFORMMATRIX3FVPROC ProgramUniformMatrix3fv;
		extern PFNGLPROGRAMUNIFORMMATRIX4FVPROC ProgramUniformMatrix4fv;
		extern PFNGLPROGRAMUNIFORMMATRIX2DVPROC ProgramUniformMatrix2dv;
		extern PFNGLPROGRAMUNIFORMMATRIX3DVPROC ProgramUniformMatrix3dv;
		extern PFNGLPROGRAMUNIFORMMATRIX4DVPROC ProgramUniformMatrix4dv;
		extern PFNGLPROGRAMUNIFORMMATRIX2X3FVPROC ProgramUniformMatrix2x3fv;
		extern PFNGLPROGRAMUNIFORMMATRIX3X2FVPROC ProgramUniformMatrix3x2fv;
		extern PFNGLPROGRAMUNIFORMMATRIX2X4FVPROC ProgramUniformMatrix2x4fv;
		extern PFNGLPROGRAMUNIFORMMATRIX4X2FVPROC ProgramUniformMatrix4x2fv;
		extern PFNGLPROGRAMUNIFORMMATRIX3X4FVPROC ProgramUniformMatrix3x4fv;
		extern PFNGLPROGRAMUNIFORMMATRIX4X3FVPROC ProgramUniformMatrix4x3fv;
		extern PFNGLPROGRAMUNIFORMMATRIX2X3DVPROC ProgramUniformMatrix2x3dv;
		extern PFNGLPROGRAMUNIFORMMATRIX3X2DVPROC ProgramUniformMatrix3x2dv;
		extern PFNGLPROGRAMUNIFORMMATRIX2X4DVPROC ProgramUniformMatrix2x4dv;
		extern PFNGLPROGRAMUNIFORMMATRIX4X2DVPROC ProgramUniformMatrix4x2dv;
		extern PFNGLPROGRAMUNIFORMMATRIX3X4DVPROC ProgramUniformMatrix3x4dv;
		extern PFNGLPROGRAMUNIFORMMATRIX4X3DVPROC ProgramUniformMatrix4x3dv;
		extern PFNGLVALIDATEPROGRAMPIPELINEPROC ValidateProgramPipeline;
		extern PFNGLGETPROGRAMPIPELINEINFOLOGPROC GetProgramPipelineInfoLog;
		extern PFNGLVERTEXATTRIBL1DPROC VertexAttribL1d;
		extern PFNGLVERTEXATTRIBL2DPROC VertexAttribL2d;
		extern PFNGLVERTEXATTRIBL3DPROC VertexAttribL3d;
		extern PFNGLVERTEXATTRIBL4DPROC VertexAttribL4d;
		extern PFNGLVERTEXATTRIBL1DVPROC VertexAttribL1dv;
		extern PFNGLVERTEXATTRIBL2DVPROC VertexAttribL2dv;
		extern PFNGLVERTEXATTRIBL3DVPROC VertexAttribL3dv;
		extern PFNGLVERTEXATTRIBL4DVPROC VertexAttribL4dv;
		extern PFNGLVERTEXATTRIBLPOINTERPROC VertexAttribLPointer;
		extern PFNGLGETVERTEXATTRIBLDVPROC GetVertexAttribLdv;
		extern PFNGLVIEWPORTARRAYVPROC ViewportArrayv;
		extern PFNGLVIEWPORTINDEXEDFPROC ViewportIndexedf;
		extern PFNGLVIEWPORTINDEXEDFVPROC ViewportIndexedfv;
		extern PFNGLSCISSORARRAYVPROC ScissorArrayv;
		extern PFNGLSCISSORINDEXEDPROC ScissorIndexed;
		extern PFNGLSCISSORINDEXEDVPROC ScissorIndexedv;
		extern PFNGLDEPTHRANGEARRAYVPROC DepthRangeArrayv;
		extern PFNGLDEPTHRANGEINDEXEDPROC DepthRangeIndexed;
		extern PFNGLGETFLOATI_VPROC GetFloati_v;
		extern PFNGLGETDOUBLEI_VPROC GetDoublei_v;
	}

#if RN_PLATFORM_WINDOWS
	namespace wgl
	{
		extern PFNWGLCREATECONTEXTATTRIBSARBPROC CreateContextAttribsARB;
		extern PFNWGLCHOOSEPIXELFORMATARBPROC ChoosePixelFormatARB;
		extern PFNWGLSWAPINTERVALEXTPROC SwapIntervalEXT;
		extern PFNWGLGETEXTENSIONSSTRINGARBPROC GetExtensionsStringARB;
	}
#endif
}

#endif /* __RAYNE_OPENGL_H__ */
