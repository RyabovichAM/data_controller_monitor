#ifndef SCHEME_SERVICE_IMPL_H
#define SCHEME_SERVICE_IMPL_H

#include <grpcpp/grpcpp.h>

#include "scheme_service.grpc.pb.h"
#include "storage/scheme_repository.h"

namespace config {

// Implementation of the SchemeService contract. Holds no state of its own: the
// schemes live in the repository.
//
// No watch stream here, unlike ConfigService: nothing reconfigures itself when
// a scheme changes. A browser showing one is the only reader, and it can
// reload.
class SchemeServiceImpl final : public dcm::scheme::v1::SchemeService::Service {
public:
    explicit SchemeServiceImpl(SchemeRepository& repository);

    grpc::Status GetScheme(grpc::ServerContext* context,
                           const dcm::scheme::v1::GetSchemeRequest* request,
                           dcm::scheme::v1::Scheme* response) override;

    grpc::Status SaveScheme(grpc::ServerContext* context,
                            const dcm::scheme::v1::SaveSchemeRequest* request,
                            dcm::scheme::v1::SaveSchemeResponse* response) override;

    grpc::Status ListSchemes(grpc::ServerContext* context,
                             const dcm::scheme::v1::ListSchemesRequest* request,
                             dcm::scheme::v1::ListSchemesResponse* response) override;

    grpc::Status DeleteScheme(grpc::ServerContext* context,
                              const dcm::scheme::v1::DeleteSchemeRequest* request,
                              dcm::scheme::v1::DeleteSchemeResponse* response) override;

private:
    SchemeRepository& repository_;
};

}   //config

#endif // SCHEME_SERVICE_IMPL_H
