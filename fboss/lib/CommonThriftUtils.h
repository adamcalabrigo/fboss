/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */
#pragma once

#include "fboss/lib/thrift_service_client/ConnectionOptions.h"

#include <fb303/ThreadCachedServiceData.h>
#include <folly/SocketAddress.h>
#include <folly/coro/AsyncScope.h>

#include <optional>
#include <string>

namespace facebook::fboss {

// helper macro to be used in ReconnectingThriftClient to output connection info
#define STREAM_XLOG(LEVEL) \
  XLOG(LEVEL) << "[STREAM " << connectionLogStr_ << "] "

class ReconnectingThriftClient {
 public:
  enum class State : uint16_t {
    DISCONNECTED,
    CONNECTING,
    CONNECTED,
    CANCELLED
  };

  static std::string connectionStateToString(State state) {
    switch (state) {
      case State::CONNECTING:
        return "CONNECTING";
      case State::CONNECTED:
        return "CONNECTED";
      case State::DISCONNECTED:
        return "DISCONNECTED";
      case State::CANCELLED:
        return "CANCELLED";
    }
    throw std::runtime_error(
        "Unhandled ReconnectingThriftClient::State::" +
        std::to_string(static_cast<int>(state)));
  }

  using StreamStateChangeCb = std::function<void(State, State)>;

  ReconnectingThriftClient(
      const std::string& clientId,
      folly::EventBase* streamEvb,
      folly::EventBase* connRetryEvb,
      const std::string& counterPrefix,
      const std::string& aggCounterPrefix,
      StreamStateChangeCb stateChangeCb,
      uint32_t reconnectTimeout);

  virtual ~ReconnectingThriftClient() {}

  State getState() const {
    return *state_.rlock();
  }

  const std::string& getCounterPrefix() const {
    return counterPrefix_;
  }

  const std::string& clientId() const {
    return clientId_;
  }

  void setConnectionOptions(
      utils::ConnectionOptions options,
      bool allowReset = false /* allow reset for use in tests*/);

  std::string getServer() const {
    if (auto connectionOptions = connectionOptions_.rlock();
        connectionOptions->has_value()) {
      return (*connectionOptions)->getDstAddr().getAddressStr();
    }
    return "";
  }
  void cancel();
  void timeoutExpired() noexcept;
  virtual void resetClient() = 0;
  bool isCancelled() const;
  bool isConnectedToServer() const;
  virtual void onCancellation() = 0;

 protected:
  virtual void setState(State state);
  void scheduleTimeout();
  std::string getConnectedCounterName() {
    return counterPrefix_ + ".connected";
  }
  virtual void connectToServer(const utils::ConnectionOptions& options) = 0;

  void setGracefulServiceLoopCompletion(const std::function<void()>& cb) {
    auto requested = gracefulServiceLoopCompletionCb_.wlock();
    *requested = cb;
  }
  bool isGracefulServiceLoopCompletionRequested() {
    auto requested = gracefulServiceLoopCompletionCb_.rlock();
    return requested->has_value();
  }
  void notifyGracefulServiceLoopCompletion() {
    auto requested = gracefulServiceLoopCompletionCb_.rlock();
    if (requested->has_value()) {
      (*requested).value()();
    }
  }

#if FOLLY_HAS_COROUTINES
  virtual folly::coro::Task<void> serviceLoopWrapper() = 0;
#endif

#if FOLLY_HAS_COROUTINES
  folly::coro::CancellableAsyncScope serviceLoopScope_;
#endif
  std::string clientId_;
  folly::EventBase* streamEvb_;
  folly::EventBase* connRetryEvb_;
  std::string counterPrefix_;
  folly::Synchronized<State> state_{State::DISCONNECTED};
  fb303::TimeseriesWrapper disconnectEvents_;
  fb303::TimeseriesWrapper aggDisconnectEvents_;
  StreamStateChangeCb stateChangeCb_;
  folly::Synchronized<std::optional<utils::ConnectionOptions>>
      connectionOptions_;
  std::unique_ptr<folly::AsyncTimeout> timer_;
  uint32_t reconnectTimeout_;
  std::string connectionLogStr_;
  folly::Synchronized<std::optional<std::function<void()>>>
      gracefulServiceLoopCompletionCb_;
};
}; // namespace facebook::fboss
