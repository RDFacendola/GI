/// \file deferred_renderer.h
/// \brief Deferred rendering classes.
///
/// \author Raffaele D. Facendola

#pragma once

#include <vector>
#include <memory>

#include "graphics.h"
#include "material.h"
#include "component.h"
#include "scene.h"
#include "observable.h"
#include "mesh.h"

using ::std::vector;
using ::std::shared_ptr;
using ::std::unique_ptr;

namespace gi_lib{

    /// \brief Exposes additional informations for a material used by a deferred renderer.
    /// \author Raffaele D. Facendola
    class DeferredRendererMaterial : public IResource{

    public:

        /// \brief Structure used to compile a deferred material from a file.
        /// This structure is identical to the one used by the base material.
        using CompileFromFile = IMaterial::CompileFromFile;

        /// \brief Virtual destructor.
        virtual ~DeferredRendererMaterial();

        /// \brief Get the base material.
        /// \return Returns the base material.
        virtual ObjectPtr<IMaterial> GetMaterial() = 0;

        /// \brief Get the base material.
        /// \return Returns the base material.
        virtual ObjectPtr<const IMaterial> GetMaterial() const = 0;

        /// \brief Instantiate this material.
        /// \return Returns a new instance of this material.
        virtual ObjectPtr<DeferredRendererMaterial> Instantiate() const = 0;

    };

    /// \brief Renderer with deferred lighting computation.
    /// \author Raffaele D. Facendola
    class DeferredRenderer : public IRenderer{

    public:

        static const int kMIPAuto = 1000;

        /// \brief Create a new tiled deferred renderer.
        /// \param scene Scene assigned to the renderer.
        DeferredRenderer(Scene& scene);

        /// \brief No assignment operator.
        DeferredRenderer& operator=(const DeferredRenderer&) = delete;

        /// \brief Virtual destructor.
        virtual ~DeferredRenderer();

        /// \brief Get the scene this renderer is assigned to.
        /// \return Returns the scene this renderer is assigned to.
        virtual Scene& GetScene() override;

        /// \brief Get the scene this renderer is assigned to.
        /// \return Returns the scene this renderer is assigned to.
        virtual const Scene& GetScene() const override;

        /// \brief Enable or disable the global illumination.
        virtual void EnableGlobalIllumination(bool enable = true) = 0;

        /// \brief Overlay the voxel structure on top of a given image.
        virtual ObjectPtr<ITexture2D> DrawVoxels(const ObjectPtr<ITexture2D>& image, int mip = kMIPAuto) = 0;

        /// \brief Overlay the SH data on top of a given image.
        /// \param alpha_mode Whether to draw voxel's opacity (true) or color (false)
        virtual ObjectPtr<ITexture2D> DrawSH(const ObjectPtr<ITexture2D>& image, bool alpha_mode, int mip = kMIPAuto) = 0;

        /// \brief Lock or unlock the camera. Debug method.
        /// Locking a camera may be useful to analyze the behavior of camera-dependent aspect of the scene, such as frustum culling
        /// LOD switching, multi-resolution voxelization and more.
        virtual void LockCamera(bool lock) = 0;

    private:

        Scene& scene_;		///< \brief Scene this render refers to.

    };

    ////////////// DEFERRED RENDERER MATERIAL ////////////////

    inline DeferredRendererMaterial::~DeferredRendererMaterial(){}

}



