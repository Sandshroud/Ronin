/***
 * Demonstrike Core
 */

#pragma once

namespace VMAP
{
    template<class T> class BIHWrap
    {
        template<class RayCallback> struct MDLCallback
        {
            const T* const* objects;
            RayCallback& _callback;
            G3D::g3d_uint32 objects_size;

            MDLCallback(RayCallback& callback, const T* const* objects_array, G3D::g3d_uint32 objects_size ) : objects(objects_array), _callback(callback), objects_size(objects_size) {}

            bool operator() (const G3D::Ray& ray, G3D::g3d_uint32 Idx, float& MaxDist, bool /*stopAtFirst*/)
            {
                if (Idx >= objects_size)
                    return false;
                if (const T* obj = objects[Idx])
                    return _callback(ray, *obj, MaxDist/*, stopAtFirst*/);
                return false;
            }

            void operator() (const G3D::Vector3& p, G3D::g3d_uint32 Idx)
            {
                if (Idx >= objects_size)
                    return false;
                if (const T* obj = objects[Idx])
                    _callback(p, *obj);
            }
        };

        typedef G3D::Array<const T*> ObjArray;

        BIH m_tree;
        ObjArray m_objects;
        G3D::Table<const T*, G3D::g3d_uint32> m_obj2Idx;
        G3D::Set<const T*> m_objects_to_push;
        int unbalanced_times;
        G3D::GMutex mutex;

    public:
        BIHWrap() : unbalanced_times(0), mutex() { m_objects_to_push.clear(); m_obj2Idx.clear(); m_objects.clear(); }

        void insert(const T& obj)
        {
            mutex.lock();
            ++unbalanced_times;
            m_objects_to_push.insert(&obj);
            mutex.unlock();
        }

        void remove(const T& obj)
        {
            mutex.lock();
            ++unbalanced_times;
            G3D::g3d_uint32 Idx = 0;
            const T * temp;
            if (m_obj2Idx.getRemove(&obj, temp, Idx))
                m_objects[Idx] = NULL;
            else
                m_objects_to_push.remove(&obj);
            mutex.unlock();
        }

        static void getModelBounds(const T *model, G3D::AABox& out) { out = model->getBounds(); }
        void balance()
        {
            mutex.lock();
            if (unbalanced_times == 0)
            {
                mutex.unlock();
                return;
            }

            unbalanced_times = 0;
            m_objects.fastClear();
            m_obj2Idx.getKeys(m_objects);
            m_objects_to_push.getMembers(m_objects);
            //assert that m_obj2Idx has all the keys

            m_tree.build(m_objects, getModelBounds);
            mutex.unlock();
        }

        template<typename RayCallback>
        void intersectRay(const G3D::Ray& ray, RayCallback& intersectCallback, float& maxDist, bool stopAtFirst)
        {
            balance();
            MDLCallback<RayCallback> temp_cb(intersectCallback, m_objects.getCArray(), m_objects.size());
            m_tree.intersectRay(ray, temp_cb, maxDist, stopAtFirst);
        }

        template<typename IsectCallback>
        void intersectPoint(const G3D::Vector3& point, IsectCallback& intersectCallback)
        {
            balance();
            MDLCallback<IsectCallback> callback(intersectCallback, m_objects.getCArray(), m_objects.size());
            m_tree.intersectPoint(point, callback);
        }
    };

}