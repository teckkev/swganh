// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE

#include "scene.h"

#include <algorithm>

#include "anh/plugin/plugin_manager.h"
#include "anh/observer/observer_interface.h"

#include "swganh/app/swganh_kernel.h"
#include "swganh/object/object.h"

#include "pub14_core/messages/scene_destroy_object.h"
#include "pub14_core/simulation/quadtree_spatial_provider.h"
#include "pub14_core/simulation/movement_manager.h"
#include "pub14_core/messages/update_transform_message.h"
#include "pub14_core/messages/update_transform_with_parent_message.h"

using namespace std;
using namespace swganh::messages;
using namespace swganh::object;
using namespace swganh::simulation;
using namespace swganh_core::simulation;
using namespace swganh::messages::controllers;
using namespace anh::observer;

class Scene::SceneImpl
{
public:
    SceneImpl(SceneDescription description, swganh::app::SwganhKernel* kernel)
        : kernel_(kernel)
        , description_(move(description))
		
    {
		auto tmp = kernel_->GetPluginManager()->CreateObject<swganh_core::simulation::QuadtreeSpatialProvider>("Simulation::SpatialProvider");
		tmp->SetThis(tmp);
		spatial_index_ = tmp;

		movement_manager_ = make_shared<MovementManager>(kernel);
		movement_manager_->SetSpatialProvider(spatial_index_);
	}

    const SceneDescription& GetDescription() const
    {
        return description_;
    }

    bool HasObject(const shared_ptr<Object>& object)
    {
        return objects_.find(object) != objects_.end();
    }

    void AddObject(shared_ptr<Object> object)
    {
		InsertObject(object);

		spatial_index_->AddObject(nullptr, object);
    }
    
    void RemoveObject(shared_ptr<Object> object)
    {
        if (!HasObject(object))
        {
            return;
        }

		EraseObject(object);             

		spatial_index_->RemoveObject(nullptr, object);
    }

	void InsertObject(const shared_ptr<Object>& object)
	{
		// make sure it's not already there
		auto find_iter = objects_.find(object);
		if (find_iter == end(objects_))
			objects_.insert(find_iter, object);

		auto find_map = object_map_.find(object->GetObjectId());
		if (find_map == end(object_map_))
			object_map_.insert(find_map, ObjectPair(object->GetObjectId(), object));
	}

	void EraseObject(const shared_ptr<Object>& object)
	{        
		objects_.erase(object);
        object_map_.erase(object->GetObjectId());
	}

	void HandleDataTransform(const shared_ptr<Object>& object, DataTransform message)
	{
		movement_manager_->HandleDataTransform(object, message);
	}
	void HandleDataTransformWithParent(const shared_ptr<Object>& object, DataTransformWithParent message)
	{
		movement_manager_->HandleDataTransformWithParent(object, message);
	}

	shared_ptr<swganh::simulation::SpatialProviderInterface> GetSpatialIndex() { return spatial_index_; }
	shared_ptr<swganh::simulation::MovementManagerInterface> GetMovementManager() { return movement_manager_; }

private:

    typedef std::map<
        uint64_t,
        shared_ptr<Object>
    > ObjectMap;
	typedef std::pair<
		uint64_t,
		shared_ptr<Object>
	> ObjectPair;

    typedef std::set<std::shared_ptr<Object>> ObjectSet;

    ObjectSet objects_;
    ObjectMap object_map_;
	
	swganh::app::SwganhKernel* kernel_;
	shared_ptr<swganh::simulation::SpatialProviderInterface> spatial_index_;
	shared_ptr<swganh::simulation::MovementManagerInterface> movement_manager_;

    SceneDescription description_;

};

Scene::Scene(SceneDescription description, swganh::app::SwganhKernel* kernel)
: impl_(new SceneImpl(move(description), move(kernel)))
{}

Scene::Scene(uint32_t scene_id, string name, string label, string description, string terrain, swganh::app::SwganhKernel* kernel) 
{
    SceneDescription scene_description;

    scene_description.id = scene_id;
    scene_description.name = move(name);
    scene_description.label = move(label);
    scene_description.description = move(description);
    scene_description.terrain = move(terrain);

    impl_.reset(new SceneImpl(move(scene_description), move(kernel)));
}

uint32_t Scene::GetSceneId() const
{
    return impl_->GetDescription().id;
}

const std::string& Scene::GetName() const
{
    return impl_->GetDescription().name;
}

const std::string& Scene::GetLabel() const
{
    return impl_->GetDescription().label;
}

const std::string& Scene::GetDescription() const
{
    return impl_->GetDescription().description;
}
const std::string& Scene::GetTerrainMap() const 
{
	return impl_->GetDescription().terrain;
}
void Scene::AddObject(std::shared_ptr<swganh::object::Object> object)
{
    impl_->AddObject(object);
}

void Scene::RemoveObject(std::shared_ptr<swganh::object::Object> object)
{
    impl_->RemoveObject(object);
}

void Scene::HandleDataTransform(const shared_ptr<Object>& object, DataTransform message)
{
	impl_->HandleDataTransform(object, message);
}
void Scene::HandleDataTransformWithParent(const shared_ptr<Object>& object, DataTransformWithParent message)
{
	impl_->HandleDataTransformWithParent(object, message);
}

void Scene::ViewObjects(std::shared_ptr<Object> requester, uint32_t max_depth, bool topDown, std::function<void(std::shared_ptr<Object>)> func)
{
	impl_->GetSpatialIndex()->ViewObjects(requester, max_depth, topDown, func);
}