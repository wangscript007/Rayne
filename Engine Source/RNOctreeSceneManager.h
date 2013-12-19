//
//  RNOctreeSceneManager.h
//  Rayne
//
//  Copyright 2013 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#ifndef __RAYNE_OCTREESCENEMANAGER_H__
#define __RAYNE_OCTREESCENEMANAGER_H__

#include "RNBase.h"
#include "RNSceneNode.h"
#include "RNCamera.h"
#include "RNEntity.h"
#include "RNRenderer.h"
#include "RNHit.h"
#include "RNSceneManager.h"

namespace RN
{
	class OctreeNode
	{
	public:
		OctreeNode(const Vector3 &min, const Vector3 &max)
		{
			
		}
		
		~OctreeNode()
		{
			
		}
		
		void Insert(const Vector3 &min, const Vector3 &max, const SceneNode *node)
		{
			
		}
		
		void Remove(const Vector3 &min, const Vector3 &max, const SceneNode *node)
		{
			
		}
		
		void Get(const Vector3 &min, const Vector3 &max) const
		{
			
		}
		
	private:
		Vector3 _min;
		Vector3 _max;
		
		std::vector<SceneNode*> _nodes;
		OctreeNode *_children[8];
	};
	
	class Octree
	{
	public:
		Octree()
		{
			
		}
		
		~Octree()
		{
			
		}
		
		void Insert(const Vector3 &min, const Vector3 &max, const SceneNode *node)
		{
			
		}
		
		void Remove(const Vector3 &min, const Vector3 &max, const SceneNode *node)
		{
			
		}
		
		void Get(const Vector3 &min, const Vector3 &max) const
		{
			
		}
		
	private:
		OctreeNode *_root;
	};
	
	class OctreeSceneManager : public SceneManager
	{
	public:
		OctreeSceneManager();
		~OctreeSceneManager() override;
		
		void AddSceneNode(SceneNode *node) override;
		void RemoveSceneNode(SceneNode *node) override;
		void UpdateSceneNode(SceneNode *node, uint32 changes) override;
		
		void RenderScene(Camera *camera) override;
		
		Hit CastRay(const Vector3 &position, const Vector3 &direction, uint32 mask = 0xffff, Hit::HitMode mode = Hit::HitMode::IgnoreNone) override;
		
	private:
		void RenderSceneNode(Camera *camera, SceneNode *node);
		
		std::vector<SceneNode *> _nodes;
		
		Octree _octree;
		
		RNDefineMetaWithTraits(OctreeSceneManager, SceneManager, MetaClassTraitCronstructable);
	};
}

#endif /* __RAYNE_OCTREESCENEMANAGER_H__ */