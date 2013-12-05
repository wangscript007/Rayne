//
//  RNShader.h
//  Rayne
//
//  Copyright 2013 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#ifndef __RAYNE_SHADER_H__
#define __RAYNE_SHADER_H__

#include "RNBase.h"
#include "RNObject.h"
#include "RNMutex.h"
#include "RNFile.h"
#include "RNArray.h"
#include "RNShaderLookup.h"
#include "RNShaderUnit.h"

namespace RN
{
	class ShaderProgram
	{
	public:
		enum
		{
			TypeNormal = 0,
			TypeInstanced = (1 << 1),
			TypeAnimated  = (1 << 2),
			TypeLighting = (1 << 3),
			TypeDiscard   = (1 << 4),
			TypeDirectionalShadows = (1 << 5),
			TypePointShadows = (1 << 6),
			TypeSpotShadows = (1 << 7),
			TypeFog = (1 << 8),
			TypeClipPlane = (1 << 9),
			TypeGammaCorrection = (1 << 10)
		};
		
		GLuint program;
		
		GLuint matProj;
		GLuint matProjInverse;
		GLuint matView;
		GLuint matViewInverse;
		GLuint matModel;
		GLuint matModelInverse;
		GLuint matNormal;
		GLuint matNormalInverse;
		GLuint matViewModel;
		GLuint matViewModelInverse;
		GLuint matProjView;
		GLuint matProjViewInverse;
		GLuint matProjViewModel;
		GLuint matProjViewModelInverse;
		GLuint matBones;
		
		GLuint instancingData;
		
		GLuint attPosition;
		GLuint attNormal;
		GLuint attTangent;
		GLuint attTexcoord0;
		GLuint attTexcoord1;
		GLuint attColor0;
		GLuint attColor1;
		GLuint attBoneWeights;
		GLuint attBoneIndices;
		
		GLuint time;
		GLuint frameSize;
		GLuint clipPlanes;
		GLuint discardThreshold;
		
		GLuint fogPlanes;
		GLuint fogColor;
		GLuint clipPlane;
		GLuint cameraAmbient;
		
		GLuint ambient;
		GLuint diffuse;
		GLuint specular;
		GLuint emissive;
		
		GLuint viewPosition;
		GLuint viewNormal;
		
		GLuint lightPointCount;
		GLuint lightPointPosition;
		GLuint lightPointColor;
		std::vector<GLuint> lightPointDepthLocations;
		
		GLuint lightSpotCount;
		GLuint lightSpotPosition;
		GLuint lightSpotColor;
		GLuint lightSpotDirection;
		GLuint lightSpotMatrix;
		std::vector<GLuint> lightSpotDepthLocations;
		
		GLuint lightDirectionalDirection;
		GLuint lightDirectionalColor;
		GLuint lightDirectionalCount;
		GLuint lightDirectionalMatrix;
		GLuint lightDirectionalDepth;
		
		GLuint lightListIndices;
		GLuint lightListOffsetCount;
		GLuint lightListDataPoint;
		GLuint lightListDataSpot;
		
		GLuint lightClusterSize;
		GLuint hdrSettings;
		
		std::vector<GLuint> texlocations;
		std::vector<GLuint> texinfolocations;
		std::vector<GLuint> targetmaplocations;
		std::vector<GLuint> targetmapinfolocations;
		std::vector<GLuint> fraglocations;
		GLuint depthmap;
		GLuint depthmapinfo;
		
		std::unordered_map<std::string, GLuint> _customLocations;
		
		GLuint GetCustomLocation(const std::string& name);
		void ReadLocations();
	};
	
	class Shader : public Object
	{
	public:
		friend class ShaderUnit;
		
		RNAPI Shader();
		RNAPI Shader(const std::string& shader);
		RNAPI virtual ~Shader();
		
		static Shader *WithFile(const std::string& shader);
		
		RNAPI void Define(const std::string& define);
		RNAPI void Define(const std::string& define, const std::string& value);
		RNAPI void Define(const std::string& define, int32 value);
		RNAPI void Define(const std::string& define, float value);
		RNAPI void Undefine(const std::string& define);
		
		RNAPI void SetShaderForType(const std::string& path, ShaderType type);
		RNAPI void SetShaderForType(File *file, ShaderType type);
		
		RNAPI ShaderProgram *GetProgramOfType(uint32 type);
		RNAPI ShaderProgram *GetProgramWithLookup(const ShaderLookup& lookup);
		
		RNAPI bool SupportsProgramOfType(uint32 type);
		RNAPI const std::string& GetShaderSource(ShaderType type);
		
		RNAPI std::string GetFileHash() const;
		
	private:
		struct DebugMarker
		{
			DebugMarker(uint32 tline, uint32 toffset, File *tfile)
			{
				line   = tline;
				offset = toffset;
				file   = tfile->GetName() + "." + tfile->GetExtension();
			}
			
			DebugMarker(uint32 tline, uint32 toffset, const std::string& tfile)
			{
				line   = tline;
				offset = toffset;
				file   = tfile;
			}
			
			uint32 line;
			uint32 offset;
			std::string file;
		};
		
		struct PreProcessedFile
		{
			std::string data;
			std::string fullpath;
			std::vector<DebugMarker> marker;
			uint32 lines;
			uint32 offset;
		};
		
		struct ShaderData
		{
			std::string file;
			std::string shader;
			std::vector<DebugMarker> marker;
		};
		
		enum IncludeMode
		{
			CurrentDir,
			IncludeDir
		};
		
		std::string PreProcessedShaderSource(const std::string& source);
		
		ShaderProgram *CompileProgram(const ShaderLookup& lookup);
		
		void IncludeShader(const std::string& name, IncludeMode mode, File *parent, PreProcessedFile& output);
		void PreProcessFile(File *file, PreProcessedFile& output);
		
		void CompileShader(ShaderType type, const std::string& file, GLuint *outShader);
		void DumpLinkStatusAndDie(ShaderProgram *program);
		bool IsDefined(const std::string& source, const std::string& define);
		
		DebugMarker ResolveFileForLine(ShaderType type, uint32 line);
		
		std::vector<ShaderDefine> _defines;
		
		uint32 _supportedPrograms;
		
		std::unordered_map<ShaderLookup, ShaderProgram *> _programs;
		std::map<ShaderType, ShaderData> _shaderData;
		
		RNDefineMetaWithTraits(Shader, Object, MetaClassTraitCronstructable)
	};
}

#endif /* __RAYNE_SHADER_H__ */
