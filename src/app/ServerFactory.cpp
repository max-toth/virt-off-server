#include "app/ServerFactory.hpp"

#include "app/ServerBuilder.hpp"
#include "v2/app/ServerBuilder.hpp"

#include <memory>

ServerFactory::ServerFactory(asio::io_context& io_ctx, uint16_t port)
    : legacy_(std::make_unique<ServerBuilder>(io_ctx, port))
    , next_(std::make_unique<v2::ServerBuilder>(io_ctx, port)) {}

std::unique_ptr<IServer> ServerFactory::create(ApiVersion version) {
    switch (version) {
        case ApiVersion::V1: {
            return legacy_->build();
        }
        case ApiVersion::V2: {
            return next_->build();
        }
    }
    return nullptr;
}
