#include "scheme_service_impl.h"

#include <exception>

namespace config {

namespace {

grpc::Status Invalid(const std::string& message) {
    return grpc::Status{grpc::StatusCode::INVALID_ARGUMENT, message};
}

// Rejects what could not be drawn or found again. Everything else — an empty
// canvas, a scheme with no labels — is a legitimate work in progress.
grpc::Status Validate(const dcm::scheme::v1::Scheme& scheme) {
    if (scheme.scheme_id().empty()) {
        return Invalid("scheme_id is empty");
    }
    if (scheme.title().empty()) {
        return Invalid("title is empty");
    }
    if (scheme.width() <= 0 || scheme.height() <= 0) {
        return Invalid("the canvas has no size");
    }

    for (const dcm::scheme::v1::Shape& shape : scheme.shapes()) {
        if (shape.kind() == dcm::scheme::v1::SHAPE_KIND_UNSPECIFIED) {
            return Invalid("a shape has no kind");
        }
        // Two for a rectangle, an ellipse or a line; a polyline needs at least
        // two to be a line rather than a dot.
        if (shape.points_size() < 2) {
            return Invalid("a shape has fewer than two points");
        }
    }

    for (const dcm::scheme::v1::Label& label : scheme.labels()) {
        if (label.parameter().empty()) {
            return Invalid("a label is bound to no parameter");
        }
    }

    return grpc::Status::OK;
}

grpc::Status Failed(const std::exception& error) {
    return grpc::Status{grpc::StatusCode::INTERNAL, error.what()};
}

}   //namespace

SchemeServiceImpl::SchemeServiceImpl(SchemeRepository& repository)
    : repository_{repository} {
}

grpc::Status SchemeServiceImpl::GetScheme(grpc::ServerContext* /*context*/,
                                          const dcm::scheme::v1::GetSchemeRequest* request,
                                          dcm::scheme::v1::Scheme* response) {
    if (request->scheme_id().empty()) {
        return Invalid("scheme_id is empty");
    }

    try {
        std::optional<Scheme> scheme = repository_.Get(request->scheme_id());
        if (!scheme) {
            return grpc::Status{grpc::StatusCode::NOT_FOUND,
                                "no scheme " + request->scheme_id()};
        }

        *response = std::move(*scheme);
        return grpc::Status::OK;
    } catch (const std::exception& error) {
        return Failed(error);
    }
}

grpc::Status SchemeServiceImpl::SaveScheme(grpc::ServerContext* /*context*/,
                                           const dcm::scheme::v1::SaveSchemeRequest* request,
                                           dcm::scheme::v1::SaveSchemeResponse* response) {
    if (grpc::Status status = Validate(request->scheme()); !status.ok()) {
        return status;
    }

    try {
        SchemeRepository::SaveResult result =
            repository_.Save(request->scheme(), request->expected_version());

        if (result.conflict) {
            return grpc::Status{grpc::StatusCode::FAILED_PRECONDITION,
                                "scheme has version " + std::to_string(result.version) +
                                    ", expected " +
                                    std::to_string(request->expected_version())};
        }

        response->set_version(result.version);
        return grpc::Status::OK;
    } catch (const std::exception& error) {
        return Failed(error);
    }
}

grpc::Status SchemeServiceImpl::ListSchemes(grpc::ServerContext* /*context*/,
                                            const dcm::scheme::v1::ListSchemesRequest* /*request*/,
                                            dcm::scheme::v1::ListSchemesResponse* response) {
    try {
        for (SchemeSummary& summary : repository_.List()) {
            *response->add_schemes() = std::move(summary);
        }

        return grpc::Status::OK;
    } catch (const std::exception& error) {
        return Failed(error);
    }
}

grpc::Status SchemeServiceImpl::DeleteScheme(grpc::ServerContext* /*context*/,
                                             const dcm::scheme::v1::DeleteSchemeRequest* request,
                                             dcm::scheme::v1::DeleteSchemeResponse* /*response*/) {
    if (request->scheme_id().empty()) {
        return Invalid("scheme_id is empty");
    }

    try {
        if (!repository_.Delete(request->scheme_id())) {
            return grpc::Status{grpc::StatusCode::NOT_FOUND,
                                "no scheme " + request->scheme_id()};
        }

        return grpc::Status::OK;
    } catch (const std::exception& error) {
        return Failed(error);
    }
}

}   //config
