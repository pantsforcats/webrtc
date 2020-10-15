/*
 *  Copyright 2020 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef PC_CONNECTION_CONTEXT_H_
#define PC_CONNECTION_CONTEXT_H_

#include <memory>
#include <string>

#include "api/media_stream_interface.h"
#include "api/peer_connection_interface.h"
#include "api/scoped_refptr.h"
#include "media/sctp/sctp_transport_internal.h"
#include "p2p/base/basic_packet_socket_factory.h"
#include "pc/channel_manager.h"
#include "rtc_base/rtc_certificate_generator.h"
#include "rtc_base/thread.h"

namespace rtc {
class BasicNetworkManager;
class BasicPacketSocketFactory;
}  // namespace rtc

namespace webrtc {

class RtcEventLog;

// This class contains resources needed by PeerConnection and associated
// objects. A reference to this object is passed to each PeerConnection. The
// methods on this object are assumed not to change the state in any way that
// interferes with the operation of other PeerConnections.
class ConnectionContext : public rtc::RefCountInterface {
 public:
  // Functions called from PeerConnectionFactory
  void SetOptions(const PeerConnectionFactoryInterface::Options& options);

  bool Initialize();

  // Functions called from PeerConnection and friends
  SctpTransportFactoryInterface* sctp_transport_factory() const {
    RTC_DCHECK_RUN_ON(signaling_thread());
    return sctp_factory_.get();
  }

  cricket::ChannelManager* channel_manager() const;

  rtc::Thread* signaling_thread() const { return signaling_thread_; }
  rtc::Thread* worker_thread() const { return worker_thread_; }
  rtc::Thread* network_thread() const { return network_thread_; }

  const PeerConnectionFactoryInterface::Options& options() const {
    return options_;
  }

  const WebRtcKeyValueConfig& trials() const { return *trials_.get(); }

  // Accessors only used from the PeerConnectionFactory class
  rtc::BasicNetworkManager* default_network_manager() const {
    RTC_DCHECK_RUN_ON(signaling_thread());
    return default_network_manager_.get();
  }
  rtc::BasicPacketSocketFactory* default_socket_factory() const {
    RTC_DCHECK_RUN_ON(signaling_thread());
    return default_socket_factory_.get();
  }
  CallFactoryInterface* call_factory() const {
    RTC_DCHECK_RUN_ON(worker_thread());
    return call_factory_.get();
  }

 protected:
  // The Dependencies class allows simple management of all new dependencies
  // being added to the ConnectionContext.
  explicit ConnectionContext(PeerConnectionFactoryDependencies& dependencies);

  virtual ~ConnectionContext();

 private:
  bool wraps_current_thread_;
  // Note: Since owned_network_thread_ and owned_worker_thread_ are used
  // in the initialization of network_thread_ and worker_thread_, they
  // must be declared before them, so that they are initialized first.
  std::unique_ptr<rtc::Thread> owned_network_thread_
      RTC_GUARDED_BY(signaling_thread());
  std::unique_ptr<rtc::Thread> owned_worker_thread_
      RTC_GUARDED_BY(signaling_thread());
  rtc::Thread* const network_thread_;
  rtc::Thread* const worker_thread_;
  rtc::Thread* const signaling_thread_;
  PeerConnectionFactoryInterface::Options options_
      RTC_GUARDED_BY(signaling_thread());
  // Accessed both on signaling thread and worker thread.
  std::unique_ptr<cricket::ChannelManager> channel_manager_;
  std::unique_ptr<rtc::NetworkMonitorFactory> const network_monitor_factory_
      RTC_GUARDED_BY(signaling_thread());
  std::unique_ptr<rtc::BasicNetworkManager> default_network_manager_
      RTC_GUARDED_BY(signaling_thread());
  std::unique_ptr<webrtc::CallFactoryInterface> const call_factory_
      RTC_GUARDED_BY(worker_thread());

  std::unique_ptr<rtc::BasicPacketSocketFactory> default_socket_factory_
      RTC_GUARDED_BY(signaling_thread());
  std::unique_ptr<cricket::MediaEngineInterface> media_engine_
      RTC_GUARDED_BY(signaling_thread());
  std::unique_ptr<SctpTransportFactoryInterface> sctp_factory_
      RTC_GUARDED_BY(signaling_thread());
  // Accessed both on signaling thread and worker thread.
  std::unique_ptr<WebRtcKeyValueConfig> const trials_;
};

}  // namespace webrtc

#endif  // PC_CONNECTION_CONTEXT_H_