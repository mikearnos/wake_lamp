#include <IPAddress.h>

//Time Zone Selection
//const int TZ_OFFSET = -4; //AST UTC-4
const int TZ_OFFSET = -5; //EST UTC-5
//const int TZ_OFFSET = -6; //CST UTC-6
//const int TZ_OFFSET = -7; //MST UTC-7
//const int TZ_OFFSET = -8; //PST UTC-8
//const int TZ_OFFSET = -9; //AKST UTC-9
//const int TZ_OFFSET = -10; //HST UTC-10

int isDST(time_t);

void startUDP(void);
void stopUDP(void);
void sendNTPpacket(IPAddress&);
uint32_t getTime(void);
void udpLoop(void);

inline int getSeconds(uint32_t);
inline int getMinutes(uint32_t);
inline int getHours(uint32_t);
