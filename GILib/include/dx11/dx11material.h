/// \file dx11material.h
/// \brief ???
///
/// \author Raffaele D. Facendola

#pragma once
#include <memory>

#include "material.h"

#include "dx11/dx11.h"

#include "windows/win_os.h"

namespace gi_lib{

	namespace dx11{

		using ::std::unique_ptr;
		using ::std::shared_ptr;

		/// \brief DirectX11 material.
		/// \author Raffaele D. Facendola
		class DX11Material : public IMaterial{

			friend class DX11MaterialVariable;
			friend class DX11MaterialResource;

		public:

			/// \brief Create a new DirectX11 material from shader code.
			/// \param device The device used to load the graphical resources.
			/// \param bundle Bundle used to load the material.
			DX11Material(const CompileFromFile& args);

			/// \brief Instantiate a DirectX11 material from another one.
			/// \param device The device used to load the graphical resources.
			/// \param bundle Bundle used to instantiate the material.
			DX11Material(const Instantiate& args);

			/// \brief Default destructor.
			virtual ~DX11Material();

			virtual size_t GetSize() const override;

			virtual ObjectPtr<IMaterialParameter> GetParameter(const string& name) override;

			virtual ObjectPtr<IMaterialResource> GetResource(const string& name) override;

			/// \brief Commit all the constant buffers and bind the material to the pipeline.
			void Commit(ID3D11DeviceContext& context);

		private:

			/// \brief Holds the properties shared among material instances.
			struct MaterialImpl;

			/// \brief Holds the private properties of this material instance.
			struct InstanceImpl;

			shared_ptr<MaterialImpl> shared_impl_;		///< \brief Properties shared among material instances.

			unique_ptr<InstanceImpl> private_impl_;		///< \brief Private properties of this material instance.

		};

		/// \brief Material variable.
		/// \author Raffaele D. Facendola
		class DX11MaterialVariable : public IMaterialParameter{

		public:

			DX11MaterialVariable(DX11Material::InstanceImpl& instance_impl, size_t buffer_index, size_t variable_size, size_t variable_offset);

			virtual ~DX11MaterialVariable(){};

			virtual void Set(const void* value_ptr, size_t size) override;

		private:

			DX11Material::InstanceImpl* instance_impl_;

			size_t buffer_index_;

			size_t variable_offset_;		///< \brief Offset of the variable from the beginning of the buffer in bytes.

			size_t variable_size_;			///< \brief Size of the variable in bytes.

		};

		/// \brief Material resource, used to bind shader resource views.
		/// \author Raffaele D. Facendola
		class DX11MaterialResource : public IMaterialResource{

		public:

			DX11MaterialResource(DX11Material::InstanceImpl& instance_impl, size_t resource_index);

			virtual ~DX11MaterialResource(){}

			virtual void Set(ObjectPtr<IResourceView> resource) override;

		private:

			DX11Material::InstanceImpl* instance_impl_;

			size_t resource_index_;

		};

	}

}