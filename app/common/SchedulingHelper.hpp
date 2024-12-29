//
// Created by consti10 on 20.12.20.
//

#ifndef WIFIBROADCAST_SCHEDULINGHELPER_H
#define WIFIBROADCAST_SCHEDULINGHELPER_H

#include <pthread.h>
#include <unistd.h>
#include <string>
#include <iostream>
#include <qdebug.h>
#ifdef __windows__
#include <windows.h>
#endif

namespace SchedulingHelper {

// this thread should run as close to realtime as possible
static void setThreadParamsMaxRealtime(pthread_t target) {

// TODO: windows.h contains THREAD_PRIORITY_TIME_CRITICAL but not working
#ifndef __windows__
    int policy = SCHED_FIFO;
  sched_param param{};
  param.sched_priority = sched_get_priority_max(policy);
  auto result = pthread_setschedparam(target, policy, &param);
  if (result != 0) {
    qDebug()<<"cannot set ThreadParamsMaxRealtime";
  }
#endif
}

static void setThreadParamsMaxRealtime() {
  setThreadParamsMaxRealtime(pthread_self());
}
}
#endif //WIFIBROADCAST_SCHEDULINGHELPER_H
