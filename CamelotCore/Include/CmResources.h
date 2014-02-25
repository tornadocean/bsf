#pragma once

#include "CmPrerequisites.h"
#include "CmModule.h"
#include "CmWorkQueue.h"

namespace CamelotFramework
{
	class CM_EXPORT Resources : public Module<Resources>
	{
	private:
		class CM_EXPORT ResourceRequestHandler : public WorkQueue::RequestHandler
		{
			virtual bool canHandleRequest( const WorkQueue::Request* req, const WorkQueue* srcQ );
			virtual WorkQueue::Response* handleRequest(WorkQueue::Request* req, const WorkQueue* srcQ );
		};

		class CM_EXPORT ResourceResponseHandler : public WorkQueue::ResponseHandler
		{
			virtual bool canHandleResponse( const WorkQueue::Response* res, const WorkQueue* srcQ );
			virtual void handleResponse( const WorkQueue::Response* res, const WorkQueue* srcQ );
		};

		struct CM_EXPORT ResourceLoadRequest
		{
			WString filePath;
			HResource resource;
		};

		struct CM_EXPORT ResourceLoadResponse
		{
			ResourcePtr rawResource;
		};

		struct CM_EXPORT ResourceAsyncOp
		{
			HResource resource;
			WorkQueue::RequestID requestID;
		};

		typedef std::shared_ptr<ResourceLoadRequest> ResourceLoadRequestPtr;
		typedef std::shared_ptr<ResourceLoadResponse> ResourceLoadResponsePtr;

	public:
		/**
		 * @brief	Constructor.
		 *
		 * @param	resMetaPath		Folder where the resource meta-data will be stored. If the folder doesn't exist
		 * 							it will be created.
		 */
		Resources();
		~Resources();

		/**
		 * @brief	Loads the resource from a given path. Returns null if resource can't be loaded.
		 *
		 * @param	filePath					The path of the file to load. The file is searched for in
		 * 										the AssetDatabase first, and if it cannot be found it is
		 * 										loaded as a temporary resource object. You can't save
		 * 										references to temporary resource objects because they won't
		 * 										persist after application shut-down, but otherwise they act
		 * 										the same as normal resources.
		 *
		 * @return	Loaded resource, or null if it cannot be found.
		 */
		HResource load(const WString& filePath);

		/**
		 * @brief	Loads the resource asynchronously. Initially returned resource should not be used
		 * 			until BaseResourceHandle.isLoaded gets set to true.
		 *
		 * @param	filePath	Full pathname of the file.
		 * 						
		 * @return	Resource where the data will eventually be loaded, or null if the file cannot be found.
		 */
		HResource loadAsync(const WString& filePath);

		/**
		 * @brief	Loads the resource with the given uuid.
		 *
		 * @param	uuid	UUID of the resource to load. 
		 *
		 * @return	Loaded resource, or null if it cannot be found.
		 */
		HResource loadFromUUID(const String& uuid);

		/**
		* @brief	Loads the resource with the given UUID asynchronously. Initially returned resource should not be used
		* 			until BaseResourceHandle.isLoaded gets set to true.
		 *
		 * @param	uuid	UUID of the resource to load. 
		 *
		 * @return	Resource where the data will eventually be loaded, or null if the file cannot be found.
		 */
		HResource loadFromUUIDAsync(const String& uuid);

		/**
		 * @brief	Unloads the resource that is referenced by the handle.
		 *
		 * @param	resourceHandle	Handle of the resource.
		 * 							
		 * @note	GPU resources held by the resource will be scheduled to be destroyed on the core thread.
		 * 			Actual resource pointer wont be deleted until all user-held references to it are removed.
		 */
		void unload(HResource resource);

		/**
		 * @brief	Finds all resources that aren't being referenced anywhere and unloads them.
		 */
		void unloadAllUnused();

		/**
		 * @brief	Saves the resource at the specified location.
		 *
		 * @param	resource 	Handle to the resource.
		 * @param	filePath 	Full pathname of the file.
		 * @param	overwrite	(optional) If true, any existing resource at the specified location will
		 * 						be overwritten.
		 */
		void save(HResource resource, const WString& filePath, bool overwrite);

		/**
		 * @brief	Creates a new resource handle from a resource pointer. 
		 * 			You will almost never need to call this manually and should instead use
		 * 			resource-specific methods that return a resource handle in the first place.
		 */
		HResource createResourceHandle(const ResourcePtr& obj);

		/**
		 * @brief	Allows you to set a resource manifest containing UUID <-> file path mapping that is
		 * 			used when resolving resource references.
		 *
		 * @note	If you want objects that reference resources (using ResourceHandles) to be able to
		 * 			find that resource even after application restart, then you must save the resource
		 * 			manifest before closing the application and restore it upon startup.
		 * 			Otherwise resources will be assigned brand new UUIDs and references will be broken.
		 */
		void setResourceManifest(const ResourceManifestPtr& manifest) { mResourceManifest = manifest; }

		/**
		 * @brief	Allows you to retrieve resource manifest containing UUID <-> file path mapping that is
		 * 			used when resolving resource references.
		 *
		 * @see		setResourceManifest
		 */
		ResourceManifestPtr getResourceManifest() const { return mResourceManifest; }

	private:
		ResourceManifestPtr mResourceManifest;

		CM_MUTEX(mInProgressResourcesMutex);
		CM_MUTEX(mLoadedResourceMutex);

		ResourceRequestHandler* mRequestHandler;
		ResourceResponseHandler* mResponseHandler;

		WorkQueue* mWorkQueue;
		UINT16 mWorkQueueChannel;

		UnorderedMap<String, HResource>::type mLoadedResources; 
		UnorderedMap<String, ResourceAsyncOp>::type mInProgressResources; // Resources that are being asynchronously loaded

		HResource loadInternal(const WString& filePath, bool synchronous); 
		ResourcePtr loadFromDiskAndDeserialize(const WString& filePath);

		const WString& getPathFromUUID(const String& uuid) const;
		const String& getUUIDFromPath(const WString& path) const;

		void notifyResourceLoadingFinished(HResource& handle);
		void notifyNewResourceLoaded(HResource& handle);
	};

	CM_EXPORT Resources& gResources();
}