#ifndef ALERT_QUEUE_H
#define ALERT_QUEUE_H

#ifdef __cplusplus

extern "C" {

#endif

void PushAlert(const char *Message);
int  PopAlert(char *Output, int MaxLen);
int  GetAlertQueueSize(void);
void ClearAlertQueue(void);

#ifdef __cplusplus

}

#endif

#endif