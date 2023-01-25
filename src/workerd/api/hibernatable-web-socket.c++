// Copyright (c) 2017-2022 Cloudflare, Inc.
// Licensed under the Apache 2.0 license found in the LICENSE file or at:
//     https://opensource.org/licenses/Apache-2.0

#include "hibernatable-web-socket.h"
#include <workerd/jsg/ser.h>

namespace workerd::api {

HibernatableWebSocketEvent::HibernatableWebSocketEvent()
    : ExtendableEvent("webSocketMessage") {};

jsg::Ref<WebSocket> HibernatableWebSocketEvent::getWebSocket(jsg::Lock& lock) {
  // This is just a stub implementation and is to be replaced once the new websocket manager
  // needs it
  return jsg::alloc<WebSocket>(kj::str(""), WebSocket::Locality::LOCAL);
}

kj::Promise<WorkerInterface::CustomEvent::Result> HibernatableWebSocketCustomEventImpl::run(
    kj::Own<IoContext_IncomingRequest> incomingRequest,
    kj::Maybe<kj::StringPtr> entrypointName) {
  // Mark the request as delivered because we're about to run some JS.
  auto& context = incomingRequest->getContext();
  incomingRequest->delivered();
  auto payload = params.getMessage().getPayload();
  EventOutcome outcome = EventOutcome::OK;

  try {
    co_await context.run(
        [entrypointName=entrypointName, &context, payload=payload]
        (Worker::Lock& lock) mutable {
      switch (payload.which()) {
        case rpc::HibernatableWebSocketEvent::Payload::TEXT:
          return lock.getGlobalScope().sendHibernatableWebSocketMessage(
              kj::str(payload.getText()),
              lock,
              lock.getExportedHandler(entrypointName, context.getActor()));

        case rpc::HibernatableWebSocketEvent::Payload::DATA: {
          auto data = payload.getData();
          return lock.getGlobalScope().sendHibernatableWebSocketMessage(
              kj::heapArray(data.begin(), data.size()),
              lock,
              lock.getExportedHandler(entrypointName, context.getActor()));
        }
        case rpc::HibernatableWebSocketEvent::Payload::CLOSE:
          return lock.getGlobalScope().sendHibernatableWebSocketClose(
              kj::str(payload.getClose().getReason()),
              payload.getClose().getCode(),
              lock,
              lock.getExportedHandler(entrypointName, context.getActor()));
        case rpc::HibernatableWebSocketEvent::Payload::ERROR:
          return lock.getGlobalScope().sendHibernatableWebSocketError(
              kj::str(payload.getError()),
              lock,
              lock.getExportedHandler(entrypointName, context.getActor()));

        KJ_UNREACHABLE;
      }
    });
  } catch(kj::Exception e) {
    if (auto desc = e.getDescription();
        !jsg::isTunneledException(desc) && !jsg::isDoNotLogException(desc)) {
      LOG_EXCEPTION("HibernatableWebSocketCustomEventImpl"_kj, e);
    }
    outcome = EventOutcome::EXCEPTION;
  }

  waitUntilTasks.add(incomingRequest->drain());

  co_return Result {
    .outcome = outcome,
  };
}

kj::Promise<WorkerInterface::CustomEvent::Result>
  HibernatableWebSocketCustomEventImpl::sendRpc(
    capnp::HttpOverCapnpFactory& httpOverCapnpFactory,
    capnp::ByteStreamFactory& byteStreamFactory,
    kj::TaskSet& waitUntilTasks,
    rpc::EventDispatcher::Client dispatcher) {
  auto req = dispatcher.hibernatableWebSocketEventRequest();
  req.setMessage(params.getMessage());

  waitUntilTasks.add(req.send().ignoreResult());

  co_return Result {
    .outcome = workerd::EventOutcome::OK,
  };
}

}  // namespace workerd::api
