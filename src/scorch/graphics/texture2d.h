#pragma once

#include <scorch/graphics/texture.h>

namespace ScorchEngine {
    class SETexture2D : public SETexture {
    public:
        SETexture2D(SEDevice& device, const SETexture::Builder& builder);
        virtual ~SETexture2D();

    protected:
        virtual void createTextureImage(const SETexture::Builder& builder) override;
    };

}