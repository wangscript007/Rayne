//
//  RNScene.cpp
//  Rayne
//
//  Copyright 2019 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#include "RNSceneBasic.h"
#include "../Debug/RNLogger.h"
#include "../Threads/RNWorkQueue.h"
#include "../Threads/RNWorkGroup.h"
#include "../Objects/RNAutoreleasePool.h"

#define kRNSceneUpdateBatchSize 64
#define kRNSceneRenderBatchSize 32

namespace RN
{
	RNDefineMeta(SceneBasic, Scene)

	SceneBasic::SceneBasic() : _nodesToRemove(new Array())
	{
		
	}
	
	SceneBasic::~SceneBasic()
	{
		_nodesToRemove->Release();
	}

	void SceneBasic::Update(float delta)
	{
		WillUpdate(delta);

		WorkQueue *queue = WorkQueue::GetGlobalQueue(WorkQueue::Priority::Default);

		for(size_t i = 0; i < 3; i ++)
		{
			WorkGroup *group = new WorkGroup();

			IntrusiveList<SceneNode>::Member *member = _nodes[i].GetHead();
			IntrusiveList<SceneNode>::Member *first = member;

			size_t count = 0;

			while(member)
			{
				if(count == kRNSceneUpdateBatchSize)
				{
					group->Perform(queue, [&, member, first] {

						AutoreleasePool pool;
						auto iterator = first;

						while(iterator != member)
						{
							SceneNode *node = iterator->Get();
							UpdateNode(node, delta);
							iterator = iterator->GetNext();
						}

					});

					first = member;
					count = 0;
				}

				member = member->GetNext();
				count ++;
			}

			//Update remaining less than kRNSceneUpdateBatchSize number of nodes
			if(first != member)
			{
				group->Perform(queue, [&, member, first] {

					AutoreleasePool pool;
					auto iterator = first;

					while(iterator != member)
					{
						SceneNode *node = iterator->Get();
						UpdateNode(node, delta);
						iterator = iterator->GetNext();
					}

				});
			}

			group->Wait();
			group->Release();
		}
		
		FlushDeletionQueue();

		Scene::Update(delta);

		DidUpdate(delta);
	}

	void SceneBasic::FlushDeletionQueue()
	{
		_nodesToRemove->Enumerate<SceneNode>([&](SceneNode *node, size_t index, bool &stop) {

			if(node->IsKindOfClass(Camera::GetMetaClass()))
			{
				Camera *camera = static_cast<Camera *>(node);
				_cameras.Erase(camera->_cameraSceneEntry);
			}

			_nodes[static_cast<size_t>(node->GetPriority())].Erase(node->_sceneEntry);

			node->UpdateSceneInfo(nullptr);
			node->Autorelease();
		});

		_nodesToRemove->RemoveAllObjects();
	}

	void SceneBasic::Render(Renderer *renderer)
	{
		WillRender(renderer);

		WorkQueue *queue = WorkQueue::GetGlobalQueue(WorkQueue::Priority::Default);

		for(int cameraPriority = 0; cameraPriority < 3; cameraPriority++)
		{
			IntrusiveList<Camera>::Member *member = _cameras.GetHead();
			while(member)
			{
				Camera *camera = member->Get();
				camera->PostUpdate();

				//Early out if camera is not supposed to render or if this isn't it's priority loop
				if(camera->GetFlags() & Camera::Flags::NoRender || (cameraPriority == 0 && !(camera->GetFlags() & Camera::Flags::RenderEarly)) || (cameraPriority == 1 && (camera->GetFlags() & Camera::Flags::RenderEarly || camera->GetFlags() & Camera::Flags::RenderLate)) || (cameraPriority == 2 && !(camera->GetFlags() & Camera::Flags::RenderLate)))
				{
					member = member->GetNext();
					continue;
				}

				renderer->SubmitCamera(camera, [&] {
					WorkGroup *group = new WorkGroup();

					for(int drawPriority = 0; drawPriority < 5; drawPriority++)
					{
						for(size_t i = 0; i < 3; i ++)
						{
							IntrusiveList<SceneNode>::Member *member = _nodes[i].GetHead();
							IntrusiveList<SceneNode>::Member *first = member;

							size_t count = 0;

							while(member)
							{
								if(count == kRNSceneRenderBatchSize)
								{
									group->Perform(queue, [&, member, first] {

										AutoreleasePool pool;
										auto iterator = first;

										while(iterator != member)
										{
											SceneNode *node = iterator->Get();
											
											//Skip if this is not the nodes draw priority or other reason (node is a light for example)
											if((drawPriority == 0 && node->IsKindOfClass(Light::GetMetaClass())) || (!node->IsKindOfClass(Light::GetMetaClass()) && ((drawPriority == 1 && (node->GetFlags() & SceneNode::Flags::DrawEarly)) || (drawPriority == 2 && !(node->GetFlags() & SceneNode::Flags::DrawEarly || node->GetFlags() & SceneNode::Flags::DrawLate || node->GetFlags() & SceneNode::Flags::DrawLater)) || (drawPriority == 3 && (node->GetFlags() & SceneNode::Flags::DrawLate)) || (drawPriority == 4 && (node->GetFlags() & SceneNode::Flags::DrawLater)))))
											{
												if(node->CanRender(renderer, camera))
													node->Render(renderer, camera);
											}

											iterator = iterator->GetNext();
										}
									});

									first = member;
									count = 0;
								}

								member = member->GetNext();
								count ++;
							}

							//Render remaining less than kRNSceneRenderBatchSize number of nodes
							if(first != member)
							{
								group->Perform(queue, [&, member, first] {

									AutoreleasePool pool;
									auto iterator = first;

									while(iterator != member)
									{
										SceneNode *node = iterator->Get();

										//Skip if this is not the nodes draw priority or other reason (node is a light for example)
										if((drawPriority == 0 && node->IsKindOfClass(Light::GetMetaClass())) || (!node->IsKindOfClass(Light::GetMetaClass()) && ((drawPriority == 1 && (node->GetFlags() & SceneNode::Flags::DrawEarly)) || (drawPriority == 2 && !(node->GetFlags() & SceneNode::Flags::DrawEarly || node->GetFlags() & SceneNode::Flags::DrawLate || node->GetFlags() & SceneNode::Flags::DrawLater)) || (drawPriority == 3 && (node->GetFlags() & SceneNode::Flags::DrawLate)) || (drawPriority == 4 && (node->GetFlags() & SceneNode::Flags::DrawLater)))))
										{
											if(node->CanRender(renderer, camera))
												node->Render(renderer, camera);
										}

										iterator = iterator->GetNext();
									}

								});
							}
						}
						
						group->Wait();
					}
					
					group->Release();
				});

				member = member->GetNext();
			}
		}

		DidRender(renderer);
	}

	void SceneBasic::AddNode(SceneNode *node)
	{
		//Remove from deletion list if scheduled for deletion if the scene didn't change.
		if(node->GetSceneInfo() && node->GetSceneInfo()->GetScene() == this && _nodesToRemove->ContainsObject(node))
		{
			_nodesToRemove->Lock();
			_nodesToRemove->RemoveObject(node);
			_nodesToRemove->Unlock();
			return;
		}
		
		RN_ASSERT(node->GetSceneInfo() == nullptr, "AddNode() must be called on a Node not owned by a scene");
    
		if(node->IsKindOfClass(Camera::GetMetaClass()))
		{
			Camera *camera = static_cast<Camera *>(node);
			_cameras.PushFront(camera->_cameraSceneEntry);
		}
		
        //PushFront to prevent race condition with scene iterating over the nodes.
		_nodes[static_cast<size_t>(node->GetPriority())].PushFront(node->_sceneEntry);
        
		node->Retain();
		SceneInfo *sceneInfo = new SceneInfo(this);
		node->UpdateSceneInfo(sceneInfo->Autorelease());
	}
	
	void SceneBasic::RemoveNode(SceneNode *node)
	{
		RN_ASSERT(node->GetSceneInfo() && node->GetSceneInfo()->GetScene() == this, "RemoveNode() must be called on a Node owned by the scene");
		
		_nodesToRemove->Lock();
		_nodesToRemove->AddObject(node);
		_nodesToRemove->Unlock();
	}
}
