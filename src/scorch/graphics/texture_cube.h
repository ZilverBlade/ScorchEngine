#pragma once

#include <scorch/graphics/texture.h>

namespace ScorchEngine {
    class SETextureCube : public SETexture {
    public:
        SETextureCube(SEDevice& device, const SETexture::Builder& builder);
        virtual ~SETextureCube();

    protected:
        virtual void createTextureImage(const SETexture::Builder& builder) override;
    };

}