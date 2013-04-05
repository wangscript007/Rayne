//
//  RBShape.cpp
//  rayne-bullet
//
//  Copyright 2013 by Überpixel. All rights reserved.
//  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
//  documentation files (the "Software"), to deal in the Software without restriction, including without limitation
//  the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
//  and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
//  INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
//  PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
//  FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
//  ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//

#include "RBShape.h"

namespace RN
{
	namespace bullet
	{
		RNDeclareMeta(Shape)
		RNDeclareMeta(SphereShape)
		RNDeclareMeta(BoxShape)
		RNDeclareMeta(CylinderShape)
		RNDeclareMeta(CapsuleShape)
		RNDeclareMeta(StaticPlaneShape)
		RNDeclareMeta(TriangelMeshShape)
		
		Shape::Shape()
		{
			_shape = 0;
		}
		
		Shape::Shape(btCollisionShape *shape)
		{
			_shape = shape;
		}
		
		Shape::~Shape()
		{
			delete _shape;
		}
		
		
		Vector3 Shape::CalculateLocalInertia(float mass)
		{
			btVector3 inertia;
			_shape->calculateLocalInertia(mass, inertia);
			
			return Vector3(inertia.x(), inertia.y(), inertia.z());
		}
		
		void Shape::SetScale(const Vector3& scale)
		{
			_shape->setLocalScaling(btVector3(scale.x, scale.y, scale.z));
		}
		
		
		SphereShape::SphereShape(float radius)
		{
			_shape = new btSphereShape(radius);
		}
		
		SphereShape *SphereShape::WithRadius(float radius)
		{
			SphereShape *shape = new SphereShape(radius);
			return shape->Autorelease();
		}
		
		
		BoxShape::BoxShape(const Vector3& halfExtents)
		{
			_shape = new btBoxShape(btVector3(halfExtents.x, halfExtents.y, halfExtents.z));
		}
		
		BoxShape *BoxShape::WithHalfExtents(const Vector3& halfExtents)
		{
			BoxShape *shape = new BoxShape(halfExtents);
			return shape->Autorelease();
		}
		
		
		CylinderShape::CylinderShape(const Vector3& halfExtents)
		{
			_shape = new btCylinderShape(btVector3(halfExtents.x, halfExtents.y, halfExtents.z));
		}
		
		CylinderShape *CylinderShape::WithHalfExtents(const Vector3& halfExtents)
		{
			CylinderShape *shape = new CylinderShape(halfExtents);
			return shape->Autorelease();
		}
		
		
		CapsuleShape::CapsuleShape(float radius, float height)
		{
			_shape = new btCapsuleShape(radius, height);
		}
		
		CapsuleShape *CapsuleShape::WithRadius(float radius, float height)
		{
			CapsuleShape *shape = new CapsuleShape(radius, height);
			return shape->Autorelease();
		}
		
		
		StaticPlaneShape::StaticPlaneShape(const Vector3& normal, float constant)
		{
			_shape = new btStaticPlaneShape(btVector3(normal.x, normal.y, normal.z), constant);
		}
		
		StaticPlaneShape *StaticPlaneShape::WithNormal(const Vector3& normal, float constant)
		{
			StaticPlaneShape *shape = new StaticPlaneShape(normal, constant);
			return shape->Autorelease();
		}
		
		
		TriangelMeshShape::TriangelMeshShape(Model *model)
		{
			_triangleMesh = new btTriangleMesh();
			
			uint32 meshes = model->Meshes(0);
			for(uint32 i=0; i<meshes; i++)
			{
				Mesh *mesh = model->MeshAtIndex(0, i);
				AddMesh(mesh);
			}
			
			_shape = new btBvhTriangleMeshShape(_triangleMesh, true);
		}
		
		TriangelMeshShape::TriangelMeshShape(Mesh *mesh)
		{
			_triangleMesh = new btTriangleMesh();
			
			AddMesh(mesh);
			
			_shape = new btBvhTriangleMeshShape(_triangleMesh, true);
		}
		
		TriangelMeshShape::TriangelMeshShape(const Array<Mesh>& meshes)
		{
			_triangleMesh = new btTriangleMesh();
			
			uint32 count = static_cast<uint32>(meshes.Count());
			for(uint32 i=0; i<count; i++)
			{
				Mesh *mesh = meshes.ObjectAtIndex(i);
				AddMesh(mesh);
			}
			
			_shape = new btBvhTriangleMeshShape(_triangleMesh, true);
		}
		
		TriangelMeshShape::~TriangelMeshShape()
		{
			delete _triangleMesh;
		}
		
		void TriangelMeshShape::AddMesh(Mesh *mesh)
		{
			MeshDescriptor *iDescriptor = mesh->Descriptor(kMeshFeatureIndices);
			
			const Vector3 *vertices  = mesh->Data<Vector3>(kMeshFeatureVertices);
			const uint16 *indices    = mesh->Data<uint16>(kMeshFeatureIndices);
			const uint16 *indicesEnd = indices + iDescriptor->elementCount;
			
			while(indices < indicesEnd)
			{
				const Vector3 *vertex0 = vertices + (*indices ++);
				const Vector3 *vertex1 = vertices + (*indices ++);
				const Vector3 *vertex2 = vertices + (*indices ++);
				
				_triangleMesh->addTriangle(btVector3(vertex0->x, vertex0->y, vertex0->z), btVector3(vertex1->x, vertex1->y, vertex1->z), btVector3(vertex2->x, vertex2->y, vertex2->z));
			}
			
			mesh->ReleaseData(kMeshFeatureIndices);
			mesh->ReleaseData(kMeshFeatureVertices);
		}
	}
}