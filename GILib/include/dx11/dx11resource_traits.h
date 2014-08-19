
/*

/// \brief Get a resource by handle.
/// \tparam Type of resource to get.
/// \param handle The handle of the resource to get.
/// \return Returns a pointer to the given resource if any. Returns null if the specified handle was invalid.
template <typename TResource, typename std::enable_if<std::is_base_of<Resource, TResource>::value>::type* = nullptr>
typename shared_ptr<typename resource_traits<TResource>::type> Get(shared_ptr<TResource> & handle){

	//TODO: Check if the handle has been created inside this manager.

	return static_pointer_cast<typename resource_traits<TResource>::type> (handle.operator->());

}

*/